/**
 * LAB6_Perform: Advanced Modular IoT Controller with Separate WebConfig & Auto MAC Topics
 * 
 * Features:
 *  - Fully decoupled WebConfig Portal (/config.html) from Main Dashboard (/index.html).
 *  - Dynamic Hardware & Network configuration saved to LittleFS (/config.json):
 *      * WiFi SSID & Password
 *      * MQTT Server, Port, User, Password
 *      * DHT Sensor Type (DHT11/DHT22) & Data Pin
 *      * Analog (ADC) Pin
 *      * Fan Relay Pin & Mist Relay Pin
 *      * Physical Button Pin
 *  - Automatically generates unique MQTT Sub/Pub Topics using device MAC address:
 *      * Sub Topic: esp-node/<MAC>/control/cmd
 *      * Pub Topic: esp-node/<MAC>/state
 *  - Fallback Captive Portal AP Mode (192.168.4.1) if WiFi connection fails or unconfigured.
 *  - Hybrid Dual-Protocol communication (WebSockets Port 81 + Cloud MQTT).
 *  - Full 2-way state synchronization across Local WS, Cloud MQTT, OLED and Hardware.
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <WebSocketsServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <DHT.h>

// --- Configuration Structure ---
struct DeviceConfig {
  String ssid = "iot_512";
  String password = "iot123456";
  String mqttServer = "broker.emqx.io";
  int mqttPort = 1883;
  String mqttUser = "elec";
  String mqttPassword = "elec1234";
  int dhtType = 11;       // 11 = DHT11, 22 = DHT22
  int dhtPin = 33;        // Default ESP32 IPST-WiFi
  int analogPin = 36;     // ADC1_CH0 (VP)
  int fanRelayPin = 5;    // GPIO 5
  int mistRelayPin = 23;  // GPIO 23
  int buttonPin = 0;      // GPIO 0 (SW1)
} config;

// Dynamic System Variables
String deviceMac = "";
String cleanMac = "";
String subTopic = "";
String pubTopic = "";
String clientId = "";

// Global Servers & Clients
WebServer server(80);
DNSServer dnsServer;
WebSocketsServer webSocket(81);
WiFiClient espClient;
PubSubClient mqttClient(espClient);
DHT* dht = nullptr;

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool oledAvailable = false;

// Hardware & State Variables
float temperature = 0.0;
float humidity = 0.0;
float analogPercent = 0.0;
float tempThreshold = 30.0;
bool fanState = false;
bool mistState = false;
int toggleCount = 0;
bool autoMode = true;

// AP / Network Flags & Timing
bool isAPMode = false;
bool rebootPending = false;
unsigned long rebootTime = 0;
unsigned long lastReadTime = 0;
unsigned long lastMqttRetry = 0;
int wsClientCount = 0;

// Button Debounce
bool lastButtonReading = HIGH;
bool currentButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// Forward Declarations
void loadConfiguration();
void saveConfiguration();
void initHardware();
void setupWebServer();
void broadcastAndPublishState();
void updateOledDisplay(const char* statusMsg = "");
void handleIncomingCommand(JsonDocument& doc, const char* source);
void mqttCallback(char* topic, byte* payload, unsigned int length);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);

// --- LittleFS Config Helpers ---
void loadConfiguration() {
  if (LittleFS.exists("/config.json")) {
    File file = LittleFS.open("/config.json", "r");
    if (file) {
      JsonDocument doc;
      DeserializationError err = deserializeJson(doc, file);
      file.close();
      if (!err) {
        if (doc.containsKey("ssid")) config.ssid = doc["ssid"].as<String>();
        if (doc.containsKey("password")) config.password = doc["password"].as<String>();
        if (doc.containsKey("mqttServer")) config.mqttServer = doc["mqttServer"].as<String>();
        if (doc.containsKey("mqttPort")) config.mqttPort = doc["mqttPort"].as<int>();
        if (doc.containsKey("mqttUser")) config.mqttUser = doc["mqttUser"].as<String>();
        if (doc.containsKey("mqttPassword")) config.mqttPassword = doc["mqttPassword"].as<String>();
        if (doc.containsKey("dhtType")) config.dhtType = doc["dhtType"].as<int>();
        if (doc.containsKey("dhtPin")) config.dhtPin = doc["dhtPin"].as<int>();
        if (doc.containsKey("analogPin")) config.analogPin = doc["analogPin"].as<int>();
        if (doc.containsKey("fanRelayPin")) config.fanRelayPin = doc["fanRelayPin"].as<int>();
        if (doc.containsKey("mistRelayPin")) config.mistRelayPin = doc["mistRelayPin"].as<int>();
        if (doc.containsKey("buttonPin")) config.buttonPin = doc["buttonPin"].as<int>();
        Serial.println("✓ Custom configuration loaded from /config.json");
        return;
      }
    }
  }
  Serial.println("! Using default hardware and network configuration.");
}

void saveConfiguration() {
  File file = LittleFS.open("/config.json", "w");
  if (file) {
    JsonDocument doc;
    doc["ssid"] = config.ssid;
    doc["password"] = config.password;
    doc["mqttServer"] = config.mqttServer;
    doc["mqttPort"] = config.mqttPort;
    doc["mqttUser"] = config.mqttUser;
    doc["mqttPassword"] = config.mqttPassword;
    doc["dhtType"] = config.dhtType;
    doc["dhtPin"] = config.dhtPin;
    doc["analogPin"] = config.analogPin;
    doc["fanRelayPin"] = config.fanRelayPin;
    doc["mistRelayPin"] = config.mistRelayPin;
    doc["buttonPin"] = config.buttonPin;
    serializeJson(doc, file);
    file.close();
    Serial.println("✓ Configuration successfully saved to /config.json");
  } else {
    Serial.println("✕ Failed to open /config.json for writing.");
  }
}

// --- Setup Hardware Pins & Peripherals ---
void initHardware() {
  pinMode(config.fanRelayPin, OUTPUT);
  pinMode(config.mistRelayPin, OUTPUT);
  pinMode(config.buttonPin, INPUT_PULLUP);

  digitalWrite(config.fanRelayPin, LOW);
  digitalWrite(config.mistRelayPin, LOW);

  // Initialize DHT Sensor dynamically
  if (dht != nullptr) delete dht;
  dht = new DHT(config.dhtPin, (config.dhtType == 22) ? DHT22 : DHT11);
  dht->begin();
  Serial.printf("✓ DHT Sensor initialized (Type: DHT%d, Pin: GPIO %d)\n", config.dhtType, config.dhtPin);
}

// --- Content-Type Helper ---
String getContentType(String filename) {
  if (filename.endsWith(".html") || filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".json")) return "application/json";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

bool handleFileRead(String path) {
  if (path.endsWith("/")) path += "index.html";
  if (path == "/config") path = "/config.html";
  
  String contentType = getContentType(path);
  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

// --- Web Server Setup ---
void setupWebServer() {
  // Static Web Dashboard and Config Pages
  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "404: File Not Found");
    }
  });

  // REST API: Get Current Configuration & Auto MAC Topics
  server.on("/api/config", HTTP_GET, []() {
    JsonDocument doc;
    doc["mac"] = deviceMac;
    doc["cleanMac"] = cleanMac;
    doc["subTopic"] = subTopic;
    doc["pubTopic"] = pubTopic;
    doc["clientId"] = clientId;
    doc["ssid"] = config.ssid;
    doc["mqttServer"] = config.mqttServer;
    doc["mqttPort"] = config.mqttPort;
    doc["mqttUser"] = config.mqttUser;
    doc["dhtType"] = config.dhtType;
    doc["dhtPin"] = config.dhtPin;
    doc["analogPin"] = config.analogPin;
    doc["fanRelayPin"] = config.fanRelayPin;
    doc["mistRelayPin"] = config.mistRelayPin;
    doc["buttonPin"] = config.buttonPin;
    doc["isAPMode"] = isAPMode;
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });

  // REST API: Save Configuration & Trigger Reboot
  server.on("/api/config", HTTP_POST, []() {
    String postBody = server.arg("plain");
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, postBody);
    
    if (err && server.args() == 0) {
      server.send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON format\"}");
      return;
    }

    if (!err) {
      if (doc.containsKey("ssid")) config.ssid = doc["ssid"].as<String>();
      if (doc.containsKey("password") && doc["password"].as<String>().length() > 0) config.password = doc["password"].as<String>();
      if (doc.containsKey("mqttServer")) config.mqttServer = doc["mqttServer"].as<String>();
      if (doc.containsKey("mqttPort")) config.mqttPort = doc["mqttPort"].as<int>();
      if (doc.containsKey("mqttUser")) config.mqttUser = doc["mqttUser"].as<String>();
      if (doc.containsKey("mqttPassword") && doc["mqttPassword"].as<String>().length() > 0) config.mqttPassword = doc["mqttPassword"].as<String>();
      if (doc.containsKey("dhtType")) config.dhtType = doc["dhtType"].as<int>();
      if (doc.containsKey("dhtPin")) config.dhtPin = doc["dhtPin"].as<int>();
      if (doc.containsKey("analogPin")) config.analogPin = doc["analogPin"].as<int>();
      if (doc.containsKey("fanRelayPin")) config.fanRelayPin = doc["fanRelayPin"].as<int>();
      if (doc.containsKey("mistRelayPin")) config.mistRelayPin = doc["mistRelayPin"].as<int>();
      if (doc.containsKey("buttonPin")) config.buttonPin = doc["buttonPin"].as<int>();
    } else {
      // Fallback form post
      if (server.hasArg("ssid")) config.ssid = server.arg("ssid");
      if (server.hasArg("password") && server.arg("password").length() > 0) config.password = server.arg("password");
      if (server.hasArg("mqttServer")) config.mqttServer = server.arg("mqttServer");
      if (server.hasArg("mqttPort")) config.mqttPort = server.arg("mqttPort").toInt();
      if (server.hasArg("mqttUser")) config.mqttUser = server.arg("mqttUser");
      if (server.hasArg("mqttPassword") && server.arg("mqttPassword").length() > 0) config.mqttPassword = server.arg("mqttPassword");
      if (server.hasArg("dhtType")) config.dhtType = server.arg("dhtType").toInt();
      if (server.hasArg("dhtPin")) config.dhtPin = server.arg("dhtPin").toInt();
      if (server.hasArg("analogPin")) config.analogPin = server.arg("analogPin").toInt();
      if (server.hasArg("fanRelayPin")) config.fanRelayPin = server.arg("fanRelayPin").toInt();
      if (server.hasArg("mistRelayPin")) config.mistRelayPin = server.arg("mistRelayPin").toInt();
      if (server.hasArg("buttonPin")) config.buttonPin = server.arg("buttonPin").toInt();
    }

    saveConfiguration();
    
    server.send(200, "application/json", "{\"success\":true,\"message\":\"บันทึกการตั้งค่าเรียบร้อย! กำลังรีบูตบอร์ดใน 2 วินาที...\"}");
    
    rebootPending = true;
    rebootTime = millis() + 2000;
  });

  // REST API: Factory Reset Configuration
  server.on("/api/reset", HTTP_POST, []() {
    if (LittleFS.exists("/config.json")) {
      LittleFS.remove("/config.json");
    }
    server.send(200, "application/json", "{\"success\":true,\"message\":\"คืนค่าเริ่มต้นเรียบร้อย กำลังรีบูต...\"}");
    rebootPending = true;
    rebootTime = millis() + 2000;
  });

  // REST API: Current State
  server.on("/api/state", HTTP_GET, []() {
    JsonDocument doc;
    doc["mac"] = cleanMac;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["analogPercent"] = analogPercent;
    doc["fanState"] = fanState;
    doc["mistState"] = mistState;
    doc["tempThreshold"] = tempThreshold;
    doc["autoMode"] = autoMode;
    doc["toggleCount"] = toggleCount;
    doc["wsClients"] = wsClientCount;
    doc["mqttConnected"] = mqttClient.connected();
    String res;
    serializeJson(doc, res);
    server.send(200, "application/json", res);
  });

  server.begin();
  Serial.println("✓ HTTP WebServer started on Port 80");
}

// --- OLED Update Routine ---
void updateOledDisplay(const char* statusMsg) {
  if (!oledAvailable) return;
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Line 1: Header + Network Info
  display.setCursor(0, 0);
  if (isAPMode) {
    display.print("AP: 192.168.4.1");
  } else {
    display.printf("IP: %s", WiFi.localIP().toString().c_str());
  }

  // Line 2: MAC & WS/MQTT Badges
  display.setCursor(0, 12);
  display.printf("ID:%s W:%d M:%s", cleanMac.substring(cleanMac.length() > 6 ? cleanMac.length() - 6 : 0).c_str(), wsClientCount, mqttClient.connected() ? "ON" : "OFF");

  // Line 3: Divider
  display.drawLine(0, 22, 128, 22, SSD1306_WHITE);

  // Line 4: Temperature & Humidity
  display.setCursor(0, 26);
  if (isnan(temperature) || isnan(humidity)) {
    display.print("Sensor: DHT Error");
  } else {
    display.printf("T:%.1fC H:%.1f%%", temperature, humidity);
  }

  // Line 5: Analog & Threshold
  display.setCursor(0, 38);
  display.printf("Pot:%.1f%% Set:%.1fC", analogPercent, tempThreshold);

  // Line 6: Actuator States & Button Count
  display.setCursor(0, 50);
  display.printf("FAN:%s MST:%s SW:%d", fanState ? "ON" : "OFF", mistState ? "ON" : "OFF", toggleCount);

  if (strlen(statusMsg) > 0) {
    display.fillRect(0, 48, 128, 16, SSD1306_BLACK);
    display.setCursor(0, 52);
    display.print(statusMsg);
  }

  display.display();
}

// --- Broadcast State to Local WebSockets & Cloud MQTT ---
void broadcastAndPublishState() {
  JsonDocument doc;
  doc["mac"] = cleanMac;
  doc["temperature"] = serialized(String(temperature, 1));
  doc["humidity"] = serialized(String(humidity, 1));
  doc["analogPercent"] = serialized(String(analogPercent, 1));
  doc["fanState"] = fanState;
  doc["mistState"] = mistState;
  doc["tempThreshold"] = serialized(String(tempThreshold, 1));
  doc["autoMode"] = autoMode;
  doc["toggleCount"] = toggleCount;
  doc["wsClients"] = wsClientCount;
  doc["mqttConnected"] = mqttClient.connected();
  doc["source"] = "esp-node";
  doc["timestamp"] = millis();

  String jsonString;
  serializeJson(doc, jsonString);

  // 1. Broadcast via WebSockets to all connected browsers on LAN
  webSocket.broadcastTXT(jsonString);

  // 2. Publish to Cloud MQTT Broker with Dynamic Unique Topic
  if (mqttClient.connected()) {
    mqttClient.publish(pubTopic.c_str(), jsonString.c_str());
  }

  updateOledDisplay();
}

// --- Centralized Command Handler ---
void handleIncomingCommand(JsonDocument& doc, const char* source) {
  bool stateChanged = false;
  String cmd = doc["command"].as<String>();

  Serial.printf("[%s CMD] Command: %s\n", source, cmd.c_str());

  if (cmd == "toggle_fan") {
    fanState = !fanState;
    digitalWrite(config.fanRelayPin, fanState ? HIGH : LOW);
    autoMode = false;
    stateChanged = true;
  } else if (cmd == "toggle_mist") {
    mistState = !mistState;
    digitalWrite(config.mistRelayPin, mistState ? HIGH : LOW);
    stateChanged = true;
  } else if (cmd == "set_fan") {
    fanState = doc["state"].as<bool>();
    digitalWrite(config.fanRelayPin, fanState ? HIGH : LOW);
    autoMode = false;
    stateChanged = true;
  } else if (cmd == "set_mist") {
    mistState = doc["state"].as<bool>();
    digitalWrite(config.mistRelayPin, mistState ? HIGH : LOW);
    stateChanged = true;
  } else if (cmd == "set_threshold") {
    tempThreshold = doc["threshold"].as<float>();
    stateChanged = true;
  } else if (cmd == "set_mode") {
    autoMode = doc["autoMode"].as<bool>();
    stateChanged = true;
  } else if (cmd == "ping") {
    stateChanged = true;
  }

  if (stateChanged) {
    broadcastAndPublishState();
  }
}

// --- WebSocket Event Handler ---
void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WS] Client #%u Disconnected.\n", num);
      wsClientCount = webSocket.connectedClients();
      updateOledDisplay();
      break;

    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[WS] Client #%u Connected from %s\n", num, ip.toString().c_str());
      wsClientCount = webSocket.connectedClients();
      
      // Immediately send current state and config metadata
      broadcastAndPublishState();
      break;
    }

    case WStype_TEXT: {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload, length);
      if (!error) {
        handleIncomingCommand(doc, "WebSocket");
      }
      break;
    }

    default:
      break;
  }
}

// --- MQTT Message Callback ---
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, message);
  if (!error) {
    handleIncomingCommand(doc, "MQTT");
  }
}

// --- Non-blocking MQTT Reconnect ---
void reconnectMQTT() {
  if (isAPMode) return;
  if (!WiFi.isConnected()) return;
  if (mqttClient.connected()) return;

  unsigned long now = millis();
  if (now - lastMqttRetry > 5000) {
    lastMqttRetry = now;
    Serial.print("[MQTT] Connecting to broker: ");
    Serial.print(config.mqttServer);
    Serial.print(" as ");
    Serial.println(clientId);

    bool connected = false;
    if (config.mqttUser.length() > 0) {
      connected = mqttClient.connect(clientId.c_str(), config.mqttUser.c_str(), config.mqttPassword.c_str());
    } else {
      connected = mqttClient.connect(clientId.c_str());
    }

    if (connected) {
      Serial.println("✓ [MQTT] Connected successfully!");
      mqttClient.subscribe(subTopic.c_str());
      Serial.printf("✓ [MQTT] Subscribed to unique topic: %s\n", subTopic.c_str());
      Serial.printf("✓ [MQTT] Publishing to unique topic: %s\n", pubTopic.c_str());
      broadcastAndPublishState();
    } else {
      Serial.printf("✕ [MQTT] Failed, rc=%d (will retry in 5s)\n", mqttClient.state());
    }
  }
}

// --- Arduino Setup ---
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\n==================================================");
  Serial.println("   LAB6_Perform: Dual-Protocol Modular IoT Controller");
  Serial.println("==================================================");

  // 1. Initialize LittleFS
  if (!LittleFS.begin(true)) {
    Serial.println("✕ LittleFS mount failed! Formatting...");
    LittleFS.format();
    LittleFS.begin(true);
  }
  Serial.println("✓ LittleFS Mounted Successfully");

  // 2. Load Configuration
  loadConfiguration();

  // 3. Obtain MAC Address & Build Unique Topics
  deviceMac = WiFi.macAddress();
  cleanMac = deviceMac;
  cleanMac.replace(":", "");
  cleanMac.toUpperCase();
  subTopic = "esp-node/" + cleanMac + "/control/cmd";
  pubTopic = "esp-node/" + cleanMac + "/state";
  clientId = "ESP32_" + cleanMac;

  Serial.println("--------------------------------------------------");
  Serial.printf("  Device MAC:        %s\n", deviceMac.c_str());
  Serial.printf("  Unique Clean ID:   %s\n", cleanMac.c_str());
  Serial.printf("  MQTT Sub Topic:    %s\n", subTopic.c_str());
  Serial.printf("  MQTT Pub Topic:    %s\n", pubTopic.c_str());
  Serial.println("--------------------------------------------------");

  // 4. Initialize OLED I2C Display
  Wire.begin(21, 22);
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    oledAvailable = true;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.println(" LAB6_Perform IoT");
    display.println(" Starting System...");
    display.printf(" ID: %s\n", cleanMac.c_str());
    display.display();
  }

  // 5. Initialize Hardware Pins
  initHardware();

  // 6. Connect to WiFi or fallback to AP Mode
  Serial.printf("Connecting to Wi-Fi SSID: %s ...\n", config.ssid.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(config.ssid.c_str(), config.password.c_str());

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
    delay(300);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    isAPMode = false;
    Serial.println("\n✓ Wi-Fi Connected!");
    Serial.print("  IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.printf("  Open Dashboard: http://%s/\n", WiFi.localIP().toString().c_str());
    Serial.printf("  Open Config:    http://%s/config.html\n", WiFi.localIP().toString().c_str());
  } else {
    // Start Fallback AP Mode
    isAPMode = true;
    String apName = "ESP-Config-" + cleanMac.substring(cleanMac.length() > 4 ? cleanMac.length() - 4 : 0);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apName.c_str(), "12345678");
    IPAddress apIP(192, 168, 4, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    dnsServer.start(53, "*", apIP);

    Serial.println("\n! WiFi Connection Failed. Starting Fallback AP Mode.");
    Serial.printf("  SSID: %s (Password: 12345678)\n", apName.c_str());
    Serial.println("  IP:   http://192.168.4.1/config.html");
  }

  // 7. Setup Web Server & WebSockets
  setupWebServer();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("✓ WebSockets Server started on Port 81");

  // 8. Setup MQTT Client
  mqttClient.setServer(config.mqttServer.c_str(), config.mqttPort);
  mqttClient.setCallback(mqttCallback);

  updateOledDisplay("System Ready");
}

// --- Main Execution Loop ---
void loop() {
  // Handle Scheduled Reboot
  if (rebootPending && millis() > rebootTime) {
    Serial.println("Rebooting MCU now...");
    ESP.restart();
  }

  // DNS Server for Captive Portal in AP Mode
  if (isAPMode) {
    dnsServer.processNextRequest();
  }

  // Handle Web Server & WebSocket Clients
  server.handleClient();
  webSocket.loop();

  // Handle MQTT Connection & Incoming Messages
  if (!isAPMode) {
    if (!mqttClient.connected()) {
      reconnectMQTT();
    } else {
      mqttClient.loop();
    }
  }

  // Periodic Sensor Reading & Automation Control (Every 2000ms)
  unsigned long currentMillis = millis();
  if (currentMillis - lastReadTime >= 2000) {
    lastReadTime = currentMillis;

    // Read DHT Sensor
    if (dht != nullptr) {
      float t = dht->readTemperature();
      float h = dht->readHumidity();
      if (!isnan(t) && !isnan(h)) {
        temperature = t;
        humidity = h;
      }
    }

    // Read Analog Potentiometer
    int rawAnalog = analogRead(config.analogPin);
    analogPercent = (rawAnalog / 4095.0) * 100.0;

    // Auto Fan Control with Hysteresis (0.5 degC)
    if (autoMode) {
      if (temperature > (tempThreshold + 0.5) && !fanState) {
        fanState = true;
        digitalWrite(config.fanRelayPin, HIGH);
        Serial.println("[AUTO] Temperature exceeded threshold -> Fan ON");
      } else if (temperature < (tempThreshold - 0.5) && fanState) {
        fanState = false;
        digitalWrite(config.fanRelayPin, LOW);
        Serial.println("[AUTO] Temperature dropped below threshold -> Fan OFF");
      }
    }

    // Synchronize state across all channels
    broadcastAndPublishState();
  }

  // Physical Button Debounce Handling (GPIO 0 / SW1)
  int reading = digitalRead(config.buttonPin);
  if (reading != lastButtonReading) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != currentButtonState) {
      currentButtonState = reading;
      if (currentButtonState == LOW) { // Button Pressed
        toggleCount++;
        fanState = !fanState;
        digitalWrite(config.fanRelayPin, fanState ? HIGH : LOW);
        autoMode = false;
        Serial.printf("[BUTTON] Pressed! Count: %d, Fan toggled to %s\n", toggleCount, fanState ? "ON" : "OFF");
        broadcastAndPublishState();
      }
    }
  }
  lastButtonReading = reading;
}
