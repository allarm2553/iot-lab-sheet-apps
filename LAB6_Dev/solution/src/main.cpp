/**
 * Lab 6: Hybrid Dual-Protocol (WebSockets + Cloud MQTT) IoT Dashboard
 * Solution Source Code (Complete Dual-Mode implementation)
 * 
 * Features:
 *  - Serves static Web Dashboard files via WebServer (Port 80) from LittleFS.
 *  - Real-time local WebSockets communication on Port 81 (low latency, LAN).
 *  - Cloud MQTT communication via EMQX Broker (broker.emqx.io:1883) for global remote access.
 *  - Full state synchronization: Any command from WS or MQTT updates hardware and broadcasts to BOTH WS and MQTT.
 *  - SSD1306 OLED I2C Display showing Network IP, WS Clients Count, MQTT Status, Sensors & Relays.
 *  - Dynamic Temperature Threshold (Slider) & Hysteresis Logic for Auto Fan Control.
 *  - Physical Switch Debouncing & Counter (GPIO 0).
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#endif

#include <WebSocketsServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <DHT.h>

// --- Network & MQTT Configuration ---
const char* ssid = "iot_512";
const char* password = "iot123456";

const char* mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
const char* subTopic = "esp-node/control/cmd";
const char* pubTopic = "esp-node/state";
const char* mqttUser = "elec";
const char* mqttPassword = "elec1234";

// --- Hardware Pin Definitions ---
#if defined(ESP8266)
#define DHTPIN 2            // D4/GPIO 2 สำหรับ AX-WiFi
#define ANALOG_PIN A0       // A0 VR Potentiometer
#define FAN_RELAY_PIN 13    // D7/GPIO 13
#define MIST_RELAY_PIN 16   // D0/GPIO 16
#define BUTTON_PIN 0        // D3/GPIO 0 (FLASH Button)
#define ADC_RESOLUTION 1023.0
#elif defined(ESP32)
#define DHTPIN 33           // GPIO 33 สำหรับ IPST-WiFi
#define ANALOG_PIN 36       // GPIO 36 / KNOB-S
#define FAN_RELAY_PIN 5     // GPIO 5
#define MIST_RELAY_PIN 23   // GPIO 23
#define BUTTON_PIN 0        // GPIO 0 / SW1
#define ADC_RESOLUTION 4095.0
#endif
#define DHTTYPE DHT11

// --- Global Objects ---
#if defined(ESP8266)
ESP8266WebServer server(80);
#else
WebServer server(80);
#endif

WebSocketsServer webSocket = WebSocketsServer(81);
WiFiClient espClient;
PubSubClient mqttClient(espClient);
DHT dht(DHTPIN, DHTTYPE);

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool oledAvailable = false;

// --- State Variables ---
float temperature = 0.0;
float humidity = 0.0;
float analogPercent = 0.0;

float tempThreshold = 30.0;
bool fanState = false;
bool mistState = false;
int toggleCount = 0;
bool autoMode = true;

// Timing & State tracking
unsigned long lastReadTime = 0;
unsigned long lastMqttRetry = 0;
int wsClientCount = 0;

// Debounce for physical button
bool lastButtonReading = HIGH;
bool currentButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// Forward declarations
void broadcastAndPublishState();
void updateOledDisplay(const char* statusMsg = "");
void handleIncomingCommand(JsonDocument& doc, const char* source);

// --- LittleFS File Helper ---
String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".json")) return "application/json";
  return "text/plain";
}

bool handleFileRead(String path) {
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

// --- Display OLED Interface ---
void updateOledDisplay(const char* statusMsg) {
  if (!oledAvailable) return;
  display.clearDisplay();
  
  // 1. Header
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 0);
  display.print("HYBRID WS+MQTT IOT");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);
  
  // 2. Sensor Information
  display.setCursor(0, 12);
  if (isnan(temperature)) {
    display.print("Temp: -- C");
  } else {
    display.printf("Temp:%.1fC Th:%.1f", temperature, tempThreshold);
  }
  
  display.setCursor(0, 22);
  if (isnan(humidity)) {
    display.print("Humid: -- %%");
  } else {
    display.printf("Humid:%.1f%% KNOB:%.0f%%", humidity, analogPercent);
  }
  
  // 3. Dual Connection Information
  display.setCursor(0, 32);
  display.printf("IP: %s", WiFi.localIP().toString().c_str());
  display.setCursor(0, 42);
  display.printf("WS:%d | MQTT:%s", wsClientCount, mqttClient.connected() ? "ON" : "OFF");
  
  // 4. Relay & Button Counter Status
  display.drawLine(0, 52, 128, 52, SSD1306_WHITE);
  display.setCursor(0, 55);
  display.printf("Fan:%s Mist:%s %s", fanState ? "ON" : "OFF", mistState ? "ON" : "OFF", autoMode ? "AUTO" : "MANU");
  
  display.display();
}

// --- Synchronized Broadcast and Publish ---
void broadcastAndPublishState() {
  JsonDocument doc;
  doc["temp"] = isnan(temperature) ? 0.0 : (round(temperature * 10.0) / 10.0);
  doc["humidity"] = isnan(humidity) ? 0.0 : (round(humidity * 10.0) / 10.0);
  doc["soil"] = round(analogPercent * 10.0) / 10.0;
  doc["fan"] = fanState;
  doc["mist"] = mistState;
  doc["threshold"] = round(tempThreshold * 10.0) / 10.0;
  doc["press"] = toggleCount;
  doc["mode"] = autoMode;
  doc["ws_clients"] = wsClientCount;
  doc["mqtt_conn"] = mqttClient.connected();
  
  String output;
  serializeJson(doc, output);
  
  // 1. Broadcast to all local WebSocket clients (Port 81)
  webSocket.broadcastTXT(output);
  
  // 2. Publish to Cloud MQTT Broker (esp-node/state)
  if (mqttClient.connected()) {
    mqttClient.publish(pubTopic, output.c_str());
  }

  updateOledDisplay();
}

// --- Unified Command Processing ---
void handleIncomingCommand(JsonDocument& doc, const char* source) {
  bool stateChanged = false;

  // Process "action" commands
  if (doc["action"].is<JsonVariant>()) {
    String action = doc["action"];
    if (action == "toggle_fan") {
      fanState = doc["value"].as<bool>();
      autoMode = false; // Switch to manual override
      digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
      Serial.printf("[%s] Fan toggled to %s (Manual Mode)\n", source, fanState ? "ON" : "OFF");
      stateChanged = true;
    } else if (action == "toggle_mist") {
      mistState = doc["value"].as<bool>();
      digitalWrite(MIST_RELAY_PIN, mistState ? HIGH : LOW);
      Serial.printf("[%s] Mist toggled to %s\n", source, mistState ? "ON" : "OFF");
      stateChanged = true;
    } else if (action == "toggle_mode") {
      autoMode = doc["value"].as<bool>();
      Serial.printf("[%s] Mode toggled to %s\n", source, autoMode ? "AUTO" : "MANUAL");
      if (autoMode && !isnan(temperature)) {
        if (temperature > tempThreshold) {
          fanState = true;
        } else if (temperature < (tempThreshold - 0.5)) {
          fanState = false;
        }
        digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
      }
      stateChanged = true;
    } else if (action == "set_threshold") {
      float newTh = doc["value"].as<float>();
      if (newTh >= 10.0 && newTh <= 50.0) {
        tempThreshold = newTh;
        Serial.printf("[%s] Threshold updated to %.1f C\n", source, tempThreshold);
        if (autoMode && !isnan(temperature)) {
          if (temperature > tempThreshold) {
            fanState = true;
          } else if (temperature < (tempThreshold - 0.5)) {
            fanState = false;
          }
          digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
        }
        stateChanged = true;
      }
    }
  }

  // Direct threshold property check
  if (doc["threshold"].is<JsonVariant>()) {
    float newTh = doc["threshold"].as<float>();
    if (newTh >= 10.0 && newTh <= 50.0) {
      tempThreshold = newTh;
      Serial.printf("[%s] Threshold updated via field to %.1f C\n", source, tempThreshold);
      if (autoMode && !isnan(temperature)) {
        if (temperature > tempThreshold) {
          fanState = true;
        } else if (temperature < (tempThreshold - 0.5)) {
          fanState = false;
        }
        digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
      }
      stateChanged = true;
    }
  }

  if (stateChanged) {
    broadcastAndPublishState();
  }
}

// --- WebSocket Event Callback ---
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("WS Client #%u Disconnected\n", num);
      if (wsClientCount > 0) wsClientCount--;
      updateOledDisplay();
      break;

    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("WS Client #%u Connected from %s\n", num, ip.toString().c_str());
      wsClientCount++;
      // Immediately send current state to newly connected WS client
      broadcastAndPublishState();
      break;
    }

    case WStype_TEXT: {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload, length);
      if (!error) {
        handleIncomingCommand(doc, "WebSocket");
      } else {
        Serial.print(F("WS deserializeJson failed: "));
        Serial.println(error.f_str());
      }
      break;
    }
    default:
      break;
  }
}

// --- MQTT Incoming Message Callback ---
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.printf("MQTT Message arrived on [%s]\n", topic);
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload, length);
  if (!error) {
    handleIncomingCommand(doc, "MQTT");
  } else {
    Serial.print(F("MQTT deserializeJson failed: "));
    Serial.println(error.f_str());
  }
}

// --- Non-blocking MQTT Reconnection ---
void reconnectMqtt() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (mqttClient.connected()) return;

  unsigned long now = millis();
  if (now - lastMqttRetry > 5000) {
    lastMqttRetry = now;
    Serial.print("Connecting to MQTT Broker...");
    
    String clientId = "ESP-HybridNode-";
    clientId += String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str(), mqttUser, mqttPassword)) {
      Serial.println(" connected!");
      mqttClient.subscribe(subTopic);
      broadcastAndPublishState();
    } else {
      Serial.printf(" failed, rc=%d. Will retry in 5s\n", mqttClient.state());
    }
    updateOledDisplay();
  }
}

// --- SETUP ---
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println(F("\n--- Starting Lab 6 Hybrid WS+MQTT Node ---"));

  // 1. Initialize Hardware Pins
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(MIST_RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(FAN_RELAY_PIN, LOW);
  digitalWrite(MIST_RELAY_PIN, LOW);

  // 2. Initialize OLED SSD1306
  Wire.begin();
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    oledAvailable = true;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(15, 20);
    display.print("Lab 6: Hybrid IoT");
    display.setCursor(10, 35);
    display.print("Initializing...");
    display.display();
  } else {
    Serial.println(F("OLED SSD1306 allocation failed or not found"));
  }

  // 3. Initialize LittleFS
  if (!LittleFS.begin()) {
    Serial.println(F("LittleFS Mount Failed!"));
  } else {
    Serial.println(F("LittleFS Mounted Successfully."));
  }

  // 4. Initialize DHT Sensor
  dht.begin();

  // 5. Connect to Wi-Fi
  Serial.printf("Connecting to Wi-Fi: %s", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(F("\nWi-Fi Connected!"));
  Serial.print(F("IP Address: "));
  Serial.println(WiFi.localIP());

  // 6. Setup WebServer (Port 80)
  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "404: File Not Found");
    }
  });
  server.begin();
  Serial.println(F("HTTP WebServer started on port 80"));

  // 7. Setup WebSocketsServer (Port 81)
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println(F("WebSockets Server started on port 81"));

  // 8. Setup MQTT Client
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(mqttCallback);
  reconnectMqtt();

  updateOledDisplay("System Ready");
}

// --- MAIN LOOP ---
void loop() {
  // 1. Maintain WebServer & WebSockets
  server.handleClient();
  webSocket.loop();

  // 2. Maintain MQTT Client connection & loop
  if (!mqttClient.connected()) {
    reconnectMqtt();
  } else {
    mqttClient.loop();
  }

  // 3. Debounce Physical Button (GPIO 0)
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonReading) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != currentButtonState) {
      currentButtonState = reading;
      if (currentButtonState == LOW) { // Button pressed
        toggleCount++;
        fanState = !fanState;
        autoMode = false; // Manual override
        digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
        Serial.printf("[Physical Button] Toggled Fan to %s | Count: %d\n", fanState ? "ON" : "OFF", toggleCount);
        broadcastAndPublishState();
      }
    }
  }
  lastButtonReading = reading;

  // 4. Periodic Sensor Sampling & Hysteresis Logic (Every 2 Seconds)
  unsigned long currentMillis = millis();
  if (currentMillis - lastReadTime >= 2000) {
    lastReadTime = currentMillis;

    // Read Sensors
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    int rawAnalog = analogRead(ANALOG_PIN);
    analogPercent = (rawAnalog / ADC_RESOLUTION) * 100.0;
    if (analogPercent > 100.0) analogPercent = 100.0;

    // Execute Hysteresis Logic in Auto Mode
    if (autoMode && !isnan(temperature)) {
      bool previousFanState = fanState;
      if (temperature > tempThreshold) {
        fanState = true;
      } else if (temperature < (tempThreshold - 0.5)) {
        fanState = false;
      }
      if (previousFanState != fanState) {
        digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
        Serial.printf("[Auto Logic] Temperature %.1f C -> Fan toggled to %s\n", temperature, fanState ? "ON" : "OFF");
      }
    }

    // Broadcast update across WS and MQTT
    broadcastAndPublishState();
  }
}
