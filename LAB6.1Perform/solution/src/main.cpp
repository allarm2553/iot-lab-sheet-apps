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

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #define WebServer ESP8266WebServer
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  #include <esp_mac.h>
#endif

#include <DNSServer.h>
#include <WebSocketsServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <DHT.h>
#include <Adafruit_BME280.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// --- Configuration Structure ---
struct DeviceConfig {
  String ssid = "iot_512";
  String password = "iot123456";
  String mqttServer = "broker.emqx.io";
  int mqttPort = 1883;
  String mqttUser = "elec";
  String mqttPassword = "elec1234";
  String topicPrefix = "allarm-iot"; // Namespace prefix for MQTT isolation
  int dhtType = 11;       // 11 = DHT11, 22 = DHT22
  int mqType = 2;         // 2 = MQ-2, 135 = MQ-135, 0 = Generic Potentiometer/ADC
  float gasThreshold = 40.0; // Gas/Smoke alarm threshold (%)
  int adcMin = 3500;         // ADC Raw for 0% (Dry Soil or Min ADC)
  int adcMax = 1200;         // ADC Raw for 100% (Wet Soil or Max ADC)

  // Module & Pin Enable/Disable Flags
  bool enableDht = true;
  bool enableGas = true;
  bool enableFan = true;
  bool enableMist = true;
  bool enableFanBtn = true;
  bool enableMistBtn = true;
  bool enableResetBtn = true;

  // New Sensors
  bool enableSoil = false;
  int soilPin = 34;           // ADC1_CH6 (GPIO 34)
  bool enableBme = false;
  int bmeAddress = 0x76;      // I2C 0x76 or 0x77
  bool enableDs18b20 = false;
  int ds18b20Pin = 4;         // GPIO 4 (1-Wire)
  bool enableLdr = false;
  int ldrPin = 35;            // ADC1_CH7 (GPIO 35)
  bool enablePir = false;
  int pirPin = 27;            // GPIO 27
  bool enableUltrasonic = false;
  int trigPin = 12;           // GPIO 12
  int echoPin = 14;           // GPIO 14
  // Custom Relay & Switch Names
  String fanName = "พัดลมระบายอากาศ (Fan)";
  String mistName = "หัวพ่นหมอก (Mist)";
  String fanBtnName = "สวิตช์พัดลม (SW_FAN)";
  String mistBtnName = "สวิตช์พ่นหมอก (SW_MIST)";
  String resetBtnName = "สวิตช์ Reset ค่าโรงงาน";

  // Dynamic Automation Rule Bindings
  String fanSensor = "temp";       // "temp", "hum", "soil", "gas", "ldr", "ds18b20", "bme_temp", "bme_hum", "bme_press", "none"
  String fanOperator = ">";        // ">" or "<"
  float fanThreshold = 30.0;
  float fanHysteresis = 0.5;

  String mistSensor = "soil";      // "temp", "hum", "soil", "gas", "ldr", "ds18b20", "bme_temp", "bme_hum", "bme_press", "none"
  String mistOperator = "<";       // ">" or "<"
  float mistThreshold = 40.0;
  float mistHysteresis = 2.0;

#if defined(ESP8266)
  int dhtPin = 2;         // GPIO 2 (D4)
  int analogPin = A0;      // A0 (MQ Gas Sensor / ADC Pin)
  int fanRelayPin = 14;   // GPIO 14 (D5)
  int mistRelayPin = 12;  // GPIO 12 (D6)
  int fanButtonPin = 13;  // GPIO 13 (D7)
  int mistButtonPin = 15; // GPIO 15 (D8)
  int resetButtonPin = 0; // GPIO 0 (Flash button / D3)
#else
  int dhtPin = 33;        // Default ESP32 IPST-WiFi
  int analogPin = 36;     // ADC1_CH0 (VP) / MQ Gas Sensor Pin
  int fanRelayPin = 5;    // GPIO 5
  int mistRelayPin = 23;  // GPIO 23
  int fanButtonPin = 18;  // GPIO 18 (Dedicated Fan Button)
  int mistButtonPin = 19; // GPIO 19 (Dedicated Mist Button)
  int resetButtonPin = 0; // GPIO 0 (Long-press >= 3s for Factory Reset ONLY)
#endif
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
Adafruit_BME280* bme = nullptr;
OneWire* oneWire = nullptr;
DallasTemperature* ds18b20 = nullptr;

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
float gasPercent = 0.0;
int gasRaw = 0;
bool gasAlarm = false;

// New Sensor Readings
float soilMoisture = 0.0;
float bmeTemp = 0.0;
float bmeHum = 0.0;
float bmePress = 0.0;
float waterTemp = 0.0;
float lightPercent = 0.0;
bool motionDetected = false;
float distanceCm = 0.0;
float tankPercent = 0.0;

float tempThreshold = 30.0;
float mistThreshold = 60.0;
float gasThreshold = 40.0;
bool fanState = false;
bool mistState = false;
bool autoMode = true;

// AP / Network Flags & Timing
bool isAPMode = false;
bool rebootPending = false;
unsigned long rebootTime = 0;
unsigned long lastReadTime = 0;
unsigned long lastMqttRetry = 0;
int wsClientCount = 0;

// Button Debounce & Timing
bool lastFanBtnReading = HIGH;
bool currentFanBtnState = HIGH;
unsigned long lastFanDebounceTime = 0;

bool lastMistBtnReading = HIGH;
bool currentMistBtnState = HIGH;
unsigned long lastMistDebounceTime = 0;

const unsigned long debounceDelay = 50;

// Long-press detection for Reset Button (GPIO 0)
unsigned long resetPressStartTime = 0;
bool resetHolding = false;
bool resetExecuted = false;

// Forward Declarations
void loadConfiguration();
void saveConfiguration();
void initHardware();
void setupWebServer();
void broadcastAndPublishState();
void publishActuatorState(const char* reason = "event");
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
        if (doc.containsKey("ssid")) {
          String loadedSsid = doc["ssid"].as<String>();
          if (loadedSsid.length() > 0) config.ssid = loadedSsid;
        }
        if (doc.containsKey("password")) config.password = doc["password"].as<String>();
        if (doc.containsKey("mqttServer")) config.mqttServer = doc["mqttServer"].as<String>();
        if (doc.containsKey("mqttPort")) config.mqttPort = doc["mqttPort"].as<int>();
        if (doc.containsKey("mqttUser")) config.mqttUser = doc["mqttUser"].as<String>();
        if (doc.containsKey("mqttPassword")) config.mqttPassword = doc["mqttPassword"].as<String>();
        if (doc.containsKey("topicPrefix")) config.topicPrefix = doc["topicPrefix"].as<String>();
        if (doc.containsKey("dhtType")) config.dhtType = doc["dhtType"].as<int>();
        if (doc.containsKey("mqType")) config.mqType = doc["mqType"].as<int>();
        if (doc.containsKey("gasThreshold")) {
          config.gasThreshold = doc["gasThreshold"].as<float>();
          gasThreshold = config.gasThreshold;
        }
        if (doc.containsKey("dhtPin")) config.dhtPin = doc["dhtPin"].as<int>();
        if (doc.containsKey("analogPin")) config.analogPin = doc["analogPin"].as<int>();
        if (doc.containsKey("adcMin")) config.adcMin = doc["adcMin"].as<int>();
        if (doc.containsKey("adcMax")) config.adcMax = doc["adcMax"].as<int>();
        if (doc.containsKey("fanRelayPin")) config.fanRelayPin = doc["fanRelayPin"].as<int>();
        if (doc.containsKey("mistRelayPin")) config.mistRelayPin = doc["mistRelayPin"].as<int>();
        if (doc.containsKey("fanButtonPin")) config.fanButtonPin = doc["fanButtonPin"].as<int>();
        if (doc.containsKey("mistButtonPin")) config.mistButtonPin = doc["mistButtonPin"].as<int>();
        if (doc.containsKey("resetButtonPin")) config.resetButtonPin = doc["resetButtonPin"].as<int>();

        if (doc.containsKey("enableDht")) config.enableDht = doc["enableDht"].as<bool>();
        if (doc.containsKey("enableGas")) config.enableGas = doc["enableGas"].as<bool>();
        if (doc.containsKey("enableFan")) config.enableFan = doc["enableFan"].as<bool>();
        if (doc.containsKey("enableMist")) config.enableMist = doc["enableMist"].as<bool>();
        if (doc.containsKey("enableFanBtn")) config.enableFanBtn = doc["enableFanBtn"].as<bool>();
        if (doc.containsKey("enableMistBtn")) config.enableMistBtn = doc["enableMistBtn"].as<bool>();
        if (doc.containsKey("enableResetBtn")) config.enableResetBtn = doc["enableResetBtn"].as<bool>();

        // New Sensors Configuration
        if (doc.containsKey("enableSoil")) config.enableSoil = doc["enableSoil"].as<bool>();
        if (doc.containsKey("soilPin")) config.soilPin = doc["soilPin"].as<int>();
        if (doc.containsKey("enableBme")) config.enableBme = doc["enableBme"].as<bool>();
        if (doc.containsKey("bmeAddress")) config.bmeAddress = doc["bmeAddress"].as<int>();
        if (doc.containsKey("enableDs18b20")) config.enableDs18b20 = doc["enableDs18b20"].as<bool>();
        if (doc.containsKey("ds18b20Pin")) config.ds18b20Pin = doc["ds18b20Pin"].as<int>();
        if (doc.containsKey("enableLdr")) config.enableLdr = doc["enableLdr"].as<bool>();
        if (doc.containsKey("ldrPin")) config.ldrPin = doc["ldrPin"].as<int>();
        if (doc.containsKey("enablePir")) config.enablePir = doc["enablePir"].as<bool>();
        if (doc.containsKey("pirPin")) config.pirPin = doc["pirPin"].as<int>();
        if (doc.containsKey("enableUltrasonic")) config.enableUltrasonic = doc["enableUltrasonic"].as<bool>();
        if (doc.containsKey("trigPin")) config.trigPin = doc["trigPin"].as<int>();
        if (doc.containsKey("echoPin")) config.echoPin = doc["echoPin"].as<int>();

        // Custom Names
        if (doc.containsKey("fanName")) config.fanName = doc["fanName"].as<String>();
        if (doc.containsKey("mistName")) config.mistName = doc["mistName"].as<String>();
        if (doc.containsKey("fanBtnName")) config.fanBtnName = doc["fanBtnName"].as<String>();
        if (doc.containsKey("mistBtnName")) config.mistBtnName = doc["mistBtnName"].as<String>();
        if (doc.containsKey("resetBtnName")) config.resetBtnName = doc["resetBtnName"].as<String>();

        // Dynamic Automation Rule Bindings
        if (doc.containsKey("fanSensor")) config.fanSensor = doc["fanSensor"].as<String>();
        if (doc.containsKey("fanOperator")) config.fanOperator = doc["fanOperator"].as<String>();
        if (doc.containsKey("fanThreshold")) config.fanThreshold = doc["fanThreshold"].as<float>();
        if (doc.containsKey("fanHysteresis")) config.fanHysteresis = doc["fanHysteresis"].as<float>();

        if (doc.containsKey("mistSensor")) config.mistSensor = doc["mistSensor"].as<String>();
        if (doc.containsKey("mistOperator")) config.mistOperator = doc["mistOperator"].as<String>();
        if (doc.containsKey("mistThreshold")) config.mistThreshold = doc["mistThreshold"].as<float>();
        if (doc.containsKey("mistHysteresis")) config.mistHysteresis = doc["mistHysteresis"].as<float>();

        // Sync legacy thresholds
        tempThreshold = config.fanThreshold;
        mistThreshold = config.mistThreshold;

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
    doc["topicPrefix"] = config.topicPrefix;
    doc["dhtType"] = config.dhtType;
    doc["mqType"] = config.mqType;
    doc["gasThreshold"] = config.gasThreshold;
    doc["dhtPin"] = config.dhtPin;
    doc["analogPin"] = config.analogPin;
    doc["adcMin"] = config.adcMin;
    doc["adcMax"] = config.adcMax;
    doc["fanRelayPin"] = config.fanRelayPin;
    doc["mistRelayPin"] = config.mistRelayPin;
    doc["fanButtonPin"] = config.fanButtonPin;
    doc["mistButtonPin"] = config.mistButtonPin;
    doc["resetButtonPin"] = config.resetButtonPin;

    doc["enableDht"] = config.enableDht;
    doc["enableGas"] = config.enableGas;
    doc["enableFan"] = config.enableFan;
    doc["enableMist"] = config.enableMist;
    doc["enableFanBtn"] = config.enableFanBtn;
    doc["enableMistBtn"] = config.enableMistBtn;
    doc["enableResetBtn"] = config.enableResetBtn;

    // New Sensors Configuration
    doc["enableSoil"] = config.enableSoil;
    doc["soilPin"] = config.soilPin;
    doc["enableBme"] = config.enableBme;
    doc["bmeAddress"] = config.bmeAddress;
    doc["enableDs18b20"] = config.enableDs18b20;
    doc["ds18b20Pin"] = config.ds18b20Pin;
    doc["enableLdr"] = config.enableLdr;
    doc["ldrPin"] = config.ldrPin;
    doc["enablePir"] = config.enablePir;
    doc["pirPin"] = config.pirPin;
    doc["enableUltrasonic"] = config.enableUltrasonic;
    doc["trigPin"] = config.trigPin;
    doc["echoPin"] = config.echoPin;

    // Custom Names
    doc["fanName"] = config.fanName;
    doc["mistName"] = config.mistName;
    doc["fanBtnName"] = config.fanBtnName;
    doc["mistBtnName"] = config.mistBtnName;
    doc["resetBtnName"] = config.resetBtnName;

    // Dynamic Automation Rule Bindings
    doc["fanSensor"] = config.fanSensor;
    doc["fanOperator"] = config.fanOperator;
    doc["fanThreshold"] = config.fanThreshold;
    doc["fanHysteresis"] = config.fanHysteresis;

    doc["mistSensor"] = config.mistSensor;
    doc["mistOperator"] = config.mistOperator;
    doc["mistThreshold"] = config.mistThreshold;
    doc["mistHysteresis"] = config.mistHysteresis;

    serializeJson(doc, file);
    file.close();
    Serial.println("✓ Configuration successfully saved to /config.json");
  } else {
    Serial.println("✕ Failed to open /config.json for writing.");
  }
}

// --- Setup Hardware Pins & Peripherals ---
void initHardware() {
  if (config.enableFan) {
    pinMode(config.fanRelayPin, OUTPUT);
    digitalWrite(config.fanRelayPin, LOW);
  }
  if (config.enableMist) {
    pinMode(config.mistRelayPin, OUTPUT);
    digitalWrite(config.mistRelayPin, LOW);
  }
  if (config.enableFanBtn) {
    pinMode(config.fanButtonPin, INPUT_PULLUP);
  }
  if (config.enableMistBtn) {
    pinMode(config.mistButtonPin, INPUT_PULLUP);
  }
  if (config.enableResetBtn) {
    pinMode(config.resetButtonPin, INPUT_PULLUP);
  }

  // 1. Initialize DHT Sensor dynamically if enabled
  if (dht != nullptr) {
    delete dht;
    dht = nullptr;
  }
  if (config.enableDht) {
    dht = new DHT(config.dhtPin, (config.dhtType == 22) ? DHT22 : DHT11);
    dht->begin();
    Serial.printf("✓ DHT Sensor initialized (Type: DHT%d, Pin: GPIO %d)\n", config.dhtType, config.dhtPin);
  } else {
    Serial.println("- DHT Sensor DISABLED");
  }

  // 2. Gas/Smoke Sensor
  if (config.enableGas) {
    Serial.printf("✓ Gas/Smoke Sensor initialized (Type: MQ-%d/ADC, Pin: GPIO/ADC %d, Threshold: %.1f%%)\n", config.mqType, config.analogPin, config.gasThreshold);
  } else {
    Serial.println("- Gas/Smoke Sensor DISABLED");
  }

  // 3. Soil Moisture Sensor
  if (config.enableSoil) {
    Serial.printf("✓ Soil Moisture Sensor initialized on GPIO/ADC %d\n", config.soilPin);
  }

  // 4. BME280 I2C Sensor
  if (config.enableBme) {
    if (bme == nullptr) bme = new Adafruit_BME280();
    if (bme->begin(config.bmeAddress)) {
      Serial.printf("✓ BME280 Sensor initialized (I2C Addr: 0x%X)\n", config.bmeAddress);
    } else {
      Serial.printf("✕ BME280 not detected at 0x%X (Check SDA/SCL wiring)\n", config.bmeAddress);
    }
  }

  // 5. DS18B20 1-Wire Waterproof Temperature Sensor
  if (ds18b20 != nullptr) {
    delete ds18b20;
    ds18b20 = nullptr;
  }
  if (oneWire != nullptr) {
    delete oneWire;
    oneWire = nullptr;
  }
  if (config.enableDs18b20) {
    oneWire = new OneWire(config.ds18b20Pin);
    ds18b20 = new DallasTemperature(oneWire);
    ds18b20->begin();
    ds18b20->setWaitForConversion(false);
    Serial.printf("✓ DS18B20 Sensor initialized on GPIO %d\n", config.ds18b20Pin);
  }

  // 6. LDR Light Sensor
  if (config.enableLdr) {
    Serial.printf("✓ LDR Light Sensor initialized on GPIO/ADC %d\n", config.ldrPin);
  }

  // 7. PIR Motion Sensor
  if (config.enablePir) {
    pinMode(config.pirPin, INPUT);
    Serial.printf("✓ PIR Motion Sensor initialized on GPIO %d\n", config.pirPin);
  }

  // 8. HC-SR04 Ultrasonic Distance Sensor
  if (config.enableUltrasonic) {
    pinMode(config.trigPin, OUTPUT);
    pinMode(config.echoPin, INPUT);
    digitalWrite(config.trigPin, LOW);
    Serial.printf("✓ HC-SR04 Ultrasonic Sensor initialized (Trig: %d, Echo: %d)\n", config.trigPin, config.echoPin);
  }
}

// --- Content-Type Helper for Web & PWA ---
String getContentType(String filename) {
  if (filename.endsWith(".html") || filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".json") || filename.endsWith(".webmanifest")) return "application/manifest+json";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".svg")) return "image/svg+xml";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

bool handleFileRead(String path) {
  if (path.endsWith("/")) path += "index.html";
  if (path == "/config") path = "/config.html";
  
  String contentType = getContentType(path);
  if (LittleFS.exists(path)) {
    // Special headers for Service Worker and PWA Manifest
    if (path.endsWith("/sw.js")) {
      server.sendHeader("Service-Worker-Allowed", "/");
      server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    } else if (path.endsWith(".html")) {
      server.sendHeader("Cache-Control", "no-cache");
    }

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
    doc["topicPrefix"] = config.topicPrefix;
    doc["dhtType"] = config.dhtType;
    doc["mqType"] = config.mqType;
    doc["gasThreshold"] = config.gasThreshold;
    doc["dhtPin"] = config.dhtPin;
    doc["analogPin"] = config.analogPin;
    doc["adcMin"] = config.adcMin;
    doc["adcMax"] = config.adcMax;
    doc["fanRelayPin"] = config.fanRelayPin;
    doc["mistRelayPin"] = config.mistRelayPin;
    doc["fanButtonPin"] = config.fanButtonPin;
    doc["mistButtonPin"] = config.mistButtonPin;
    doc["resetButtonPin"] = config.resetButtonPin;

    doc["enableDht"] = config.enableDht;
    doc["enableGas"] = config.enableGas;
    doc["enableFan"] = config.enableFan;
    doc["enableMist"] = config.enableMist;
    doc["enableFanBtn"] = config.enableFanBtn;
    doc["enableMistBtn"] = config.enableMistBtn;
    doc["enableResetBtn"] = config.enableResetBtn;

    // New Sensors
    doc["enableSoil"] = config.enableSoil;
    doc["soilPin"] = config.soilPin;
    doc["enableBme"] = config.enableBme;
    doc["bmeAddress"] = config.bmeAddress;
    doc["enableDs18b20"] = config.enableDs18b20;
    doc["ds18b20Pin"] = config.ds18b20Pin;
    doc["enableLdr"] = config.enableLdr;
    doc["ldrPin"] = config.ldrPin;
    doc["enablePir"] = config.enablePir;
    doc["pirPin"] = config.pirPin;
    doc["enableUltrasonic"] = config.enableUltrasonic;
    doc["trigPin"] = config.trigPin;
    doc["echoPin"] = config.echoPin;

    // Custom Names
    doc["fanName"] = config.fanName;
    doc["mistName"] = config.mistName;
    doc["fanBtnName"] = config.fanBtnName;
    doc["mistBtnName"] = config.mistBtnName;
    doc["resetBtnName"] = config.resetBtnName;

    // Dynamic Automation Rule Bindings
    doc["fanSensor"] = config.fanSensor;
    doc["fanOperator"] = config.fanOperator;
    doc["fanThreshold"] = config.fanThreshold;
    doc["fanHysteresis"] = config.fanHysteresis;

    doc["mistSensor"] = config.mistSensor;
    doc["mistOperator"] = config.mistOperator;
    doc["mistThreshold"] = config.mistThreshold;
    doc["mistHysteresis"] = config.mistHysteresis;

    doc["isAPMode"] = isAPMode;
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });

  // REST API: Save Configuration & Smart Live Hot-Reload / Reboot
  server.on("/api/config", HTTP_POST, []() {
    String postBody = server.arg("plain");
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, postBody);
    
    if (err && server.args() == 0) {
      server.send(400, "application/json", "{\"success\":false,\"error\":\"Invalid JSON format\"}");
      return;
    }

    String oldSsid = config.ssid;
    String oldPass = config.password;

    if (!err) {
      if (doc["ssid"].is<String>()) config.ssid = doc["ssid"].as<String>();
      if (doc["password"].is<String>() && doc["password"].as<String>().length() > 0) config.password = doc["password"].as<String>();
      if (doc["mqttServer"].is<String>()) config.mqttServer = doc["mqttServer"].as<String>();
      if (doc["mqttPort"].is<int>()) config.mqttPort = doc["mqttPort"].as<int>();
      if (doc["mqttUser"].is<String>()) config.mqttUser = doc["mqttUser"].as<String>();
      if (doc["mqttPassword"].is<String>() && doc["mqttPassword"].as<String>().length() > 0) config.mqttPassword = doc["mqttPassword"].as<String>();
      if (doc["topicPrefix"].is<String>()) config.topicPrefix = doc["topicPrefix"].as<String>();
      if (doc["dhtType"].is<int>()) config.dhtType = doc["dhtType"].as<int>();
      if (doc["mqType"].is<int>()) config.mqType = doc["mqType"].as<int>();
      if (doc["gasThreshold"].is<float>()) {
        config.gasThreshold = doc["gasThreshold"].as<float>();
        gasThreshold = config.gasThreshold;
      }
      if (doc["dhtPin"].is<int>()) config.dhtPin = doc["dhtPin"].as<int>();
      if (doc["analogPin"].is<int>()) config.analogPin = doc["analogPin"].as<int>();
      if (doc["adcMin"].is<int>()) config.adcMin = doc["adcMin"].as<int>();
      if (doc["adcMax"].is<int>()) config.adcMax = doc["adcMax"].as<int>();
      if (doc["fanRelayPin"].is<int>()) config.fanRelayPin = doc["fanRelayPin"].as<int>();
      if (doc["mistRelayPin"].is<int>()) config.mistRelayPin = doc["mistRelayPin"].as<int>();
      if (doc["fanButtonPin"].is<int>()) config.fanButtonPin = doc["fanButtonPin"].as<int>();
      if (doc["mistButtonPin"].is<int>()) config.mistButtonPin = doc["mistButtonPin"].as<int>();
      if (doc["resetButtonPin"].is<int>()) config.resetButtonPin = doc["resetButtonPin"].as<int>();

      if (doc["enableDht"].is<bool>()) config.enableDht = doc["enableDht"].as<bool>();
      if (doc["enableGas"].is<bool>()) config.enableGas = doc["enableGas"].as<bool>();
      if (doc["enableFan"].is<bool>()) config.enableFan = doc["enableFan"].as<bool>();
      if (doc["enableMist"].is<bool>()) config.enableMist = doc["enableMist"].as<bool>();
      if (doc["enableFanBtn"].is<bool>()) config.enableFanBtn = doc["enableFanBtn"].as<bool>();
      if (doc["enableMistBtn"].is<bool>()) config.enableMistBtn = doc["enableMistBtn"].as<bool>();
      if (doc["enableResetBtn"].is<bool>()) config.enableResetBtn = doc["enableResetBtn"].as<bool>();

      // New Sensors
      if (doc["enableSoil"].is<bool>()) config.enableSoil = doc["enableSoil"].as<bool>();
      if (doc["soilPin"].is<int>()) config.soilPin = doc["soilPin"].as<int>();
      if (doc["enableBme"].is<bool>()) config.enableBme = doc["enableBme"].as<bool>();
      if (doc["bmeAddress"].is<int>()) config.bmeAddress = doc["bmeAddress"].as<int>();
      if (doc["enableDs18b20"].is<bool>()) config.enableDs18b20 = doc["enableDs18b20"].as<bool>();
      if (doc["ds18b20Pin"].is<int>()) config.ds18b20Pin = doc["ds18b20Pin"].as<int>();
      if (doc["enableLdr"].is<bool>()) config.enableLdr = doc["enableLdr"].as<bool>();
      if (doc["ldrPin"].is<int>()) config.ldrPin = doc["ldrPin"].as<int>();
      if (doc["enablePir"].is<bool>()) config.enablePir = doc["enablePir"].as<bool>();
      if (doc["pirPin"].is<int>()) config.pirPin = doc["pirPin"].as<int>();
      if (doc["enableUltrasonic"].is<bool>()) config.enableUltrasonic = doc["enableUltrasonic"].as<bool>();
      if (doc["trigPin"].is<int>()) config.trigPin = doc["trigPin"].as<int>();
      if (doc["echoPin"].is<int>()) config.echoPin = doc["echoPin"].as<int>();

      // Custom Names
      if (doc["fanName"].is<String>()) config.fanName = doc["fanName"].as<String>();
      if (doc["mistName"].is<String>()) config.mistName = doc["mistName"].as<String>();
      if (doc["fanBtnName"].is<String>()) config.fanBtnName = doc["fanBtnName"].as<String>();
      if (doc["mistBtnName"].is<String>()) config.mistBtnName = doc["mistBtnName"].as<String>();
      if (doc["resetBtnName"].is<String>()) config.resetBtnName = doc["resetBtnName"].as<String>();

      // Dynamic Automation Rule Bindings
      if (doc["fanSensor"].is<String>()) config.fanSensor = doc["fanSensor"].as<String>();
      if (doc["fanOperator"].is<String>()) config.fanOperator = doc["fanOperator"].as<String>();
      if (doc["fanThreshold"].is<float>()) {
        config.fanThreshold = doc["fanThreshold"].as<float>();
        tempThreshold = config.fanThreshold;
      }
      if (doc["fanHysteresis"].is<float>()) config.fanHysteresis = doc["fanHysteresis"].as<float>();

      if (doc["mistSensor"].is<String>()) config.mistSensor = doc["mistSensor"].as<String>();
      if (doc["mistOperator"].is<String>()) config.mistOperator = doc["mistOperator"].as<String>();
      if (doc["mistThreshold"].is<float>()) {
        config.mistThreshold = doc["mistThreshold"].as<float>();
        mistThreshold = config.mistThreshold;
      }
      if (doc["mistHysteresis"].is<float>()) config.mistHysteresis = doc["mistHysteresis"].as<float>();
    } else {
      // Fallback form post
      if (server.hasArg("ssid")) config.ssid = server.arg("ssid");
      if (server.hasArg("password") && server.arg("password").length() > 0) config.password = server.arg("password");
      if (server.hasArg("mqttServer")) config.mqttServer = server.arg("mqttServer");
      if (server.hasArg("mqttPort")) config.mqttPort = server.arg("mqttPort").toInt();
      if (server.hasArg("mqttUser")) config.mqttUser = server.arg("mqttUser");
      if (server.hasArg("mqttPassword") && server.arg("mqttPassword").length() > 0) config.mqttPassword = server.arg("mqttPassword");
      if (server.hasArg("topicPrefix")) config.topicPrefix = server.arg("topicPrefix");
      if (server.hasArg("dhtType")) config.dhtType = server.arg("dhtType").toInt();
      if (server.hasArg("mqType")) config.mqType = server.arg("mqType").toInt();
      if (server.hasArg("gasThreshold")) {
        config.gasThreshold = server.arg("gasThreshold").toFloat();
        gasThreshold = config.gasThreshold;
      }
      if (server.hasArg("dhtPin")) config.dhtPin = server.arg("dhtPin").toInt();
      if (server.hasArg("analogPin")) config.analogPin = server.arg("analogPin").toInt();
      if (server.hasArg("adcMin")) config.adcMin = server.arg("adcMin").toInt();
      if (server.hasArg("adcMax")) config.adcMax = server.arg("adcMax").toInt();
      if (server.hasArg("fanRelayPin")) config.fanRelayPin = server.arg("fanRelayPin").toInt();
      if (server.hasArg("mistRelayPin")) config.mistRelayPin = server.arg("mistRelayPin").toInt();
      if (server.hasArg("fanButtonPin")) config.fanButtonPin = server.arg("fanButtonPin").toInt();
      if (server.hasArg("mistButtonPin")) config.mistButtonPin = server.arg("mistButtonPin").toInt();
      if (server.hasArg("resetButtonPin")) config.resetButtonPin = server.arg("resetButtonPin").toInt();

      if (server.hasArg("fanName")) config.fanName = server.arg("fanName");
      if (server.hasArg("mistName")) config.mistName = server.arg("mistName");
      if (server.hasArg("fanBtnName")) config.fanBtnName = server.arg("fanBtnName");
      if (server.hasArg("mistBtnName")) config.mistBtnName = server.arg("mistBtnName");
      if (server.hasArg("resetBtnName")) config.resetBtnName = server.arg("resetBtnName");

      if (server.hasArg("enableDht")) config.enableDht = (server.arg("enableDht") == "1" || server.arg("enableDht") == "true");
      if (server.hasArg("enableGas")) config.enableGas = (server.arg("enableGas") == "1" || server.arg("enableGas") == "true");
      if (server.hasArg("enableFan")) config.enableFan = (server.arg("enableFan") == "1" || server.arg("enableFan") == "true");
      if (server.hasArg("enableMist")) config.enableMist = (server.arg("enableMist") == "1" || server.arg("enableMist") == "true");
      if (server.hasArg("enableFanBtn")) config.enableFanBtn = (server.arg("enableFanBtn") == "1" || server.arg("enableFanBtn") == "true");
      if (server.hasArg("enableMistBtn")) config.enableMistBtn = (server.arg("enableMistBtn") == "1" || server.arg("enableMistBtn") == "true");
      if (server.hasArg("enableResetBtn")) config.enableResetBtn = (server.arg("enableResetBtn") == "1" || server.arg("enableResetBtn") == "true");

      if (server.hasArg("enableSoil")) config.enableSoil = (server.arg("enableSoil") == "1" || server.arg("enableSoil") == "true");
      if (server.hasArg("soilPin")) config.soilPin = server.arg("soilPin").toInt();
      if (server.hasArg("enableBme")) config.enableBme = (server.arg("enableBme") == "1" || server.arg("enableBme") == "true");
      if (server.hasArg("bmeAddress")) config.bmeAddress = (server.arg("bmeAddress").startsWith("0x") || server.arg("bmeAddress").startsWith("0X")) ? strtol(server.arg("bmeAddress").c_str(), NULL, 16) : server.arg("bmeAddress").toInt();
      if (server.hasArg("enableDs18b20")) config.enableDs18b20 = (server.arg("enableDs18b20") == "1" || server.arg("enableDs18b20") == "true");
      if (server.hasArg("ds18b20Pin")) config.ds18b20Pin = server.arg("ds18b20Pin").toInt();
      if (server.hasArg("enableLdr")) config.enableLdr = (server.arg("enableLdr") == "1" || server.arg("enableLdr") == "true");
      if (server.hasArg("ldrPin")) config.ldrPin = server.arg("ldrPin").toInt();
      if (server.hasArg("enablePir")) config.enablePir = (server.arg("enablePir") == "1" || server.arg("enablePir") == "true");
      if (server.hasArg("pirPin")) config.pirPin = server.arg("pirPin").toInt();
      if (server.hasArg("enableUltrasonic")) config.enableUltrasonic = (server.arg("enableUltrasonic") == "1" || server.arg("enableUltrasonic") == "true");
      if (server.hasArg("trigPin")) config.trigPin = server.arg("trigPin").toInt();
      if (server.hasArg("echoPin")) config.echoPin = server.arg("echoPin").toInt();
    }

    saveConfiguration();

    bool wifiChanged = (config.ssid != oldSsid) || (config.password != oldPass);
    if (wifiChanged) {
      server.send(200, "application/json", "{\"success\":true,\"reboot\":true,\"message\":\"เปลี่ยน Wi-Fi สำเร็จ! บอร์ดกำลังรีบูตเพื่อเชื่อมต่อเครือข่ายใหม่...\"}");
      rebootPending = true;
      rebootTime = millis() + 1500;
    } else {
      // Live Hot-Reload in memory without rebooting!
      initHardware();
      gasThreshold = config.gasThreshold;

      String pfx = config.topicPrefix;
      pfx.trim();
      while (pfx.endsWith("/")) pfx = pfx.substring(0, pfx.length() - 1);
      if (pfx.length() > 0) pfx += "/";

      subTopic = pfx + "esp-node/" + cleanMac + "/control/cmd";
      pubTopic = pfx + "esp-node/" + cleanMac + "/state";

      mqttClient.setServer(config.mqttServer.c_str(), config.mqttPort);
      if (mqttClient.connected()) {
        mqttClient.disconnect(); // Triggers non-blocking reconnect with new topics/broker
      }

      broadcastAndPublishState();
      server.send(200, "application/json", "{\"success\":true,\"reboot\":false,\"message\":\"✓ บันทึกและอัปเดตการตั้งค่าทันที (Live Updated) โดยไม่ต้องรีบูตบอร์ด!\"}");
    }
  });

  // REST API: Factory Reset Configuration
  server.on("/api/reset", HTTP_POST, []() {
    if (LittleFS.exists("/config.json")) {
      LittleFS.remove("/config.json");
    }
    server.send(200, "application/json", "{\"success\":true,\"reboot\":true,\"message\":\"คืนค่าเริ่มต้นเรียบร้อย กำลังรีบูต...\"}");
    rebootPending = true;
    rebootTime = millis() + 1500;
  });

  // REST API: Current State
  server.on("/api/state", HTTP_GET, []() {
    JsonDocument doc;
    doc["mac"] = cleanMac;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["analogPercent"] = analogPercent;
    doc["gasPercent"] = gasPercent;
    doc["gasRaw"] = gasRaw;
    doc["adcRaw"] = gasRaw;
    doc["gasAlarm"] = gasAlarm;
    doc["mqType"] = config.mqType;
    doc["gasThreshold"] = gasThreshold;
    doc["adcMin"] = config.adcMin;
    doc["adcMax"] = config.adcMax;
    doc["fanState"] = fanState;
    doc["mistState"] = mistState;
    doc["tempThreshold"] = tempThreshold;
    doc["mistThreshold"] = mistThreshold;
    doc["autoMode"] = autoMode;
    doc["wsClients"] = wsClientCount;
    doc["mqttConnected"] = mqttClient.connected();

    doc["enableDht"] = config.enableDht;
    doc["enableGas"] = config.enableGas;
    doc["enableFan"] = config.enableFan;
    doc["enableMist"] = config.enableMist;
    doc["enableFanBtn"] = config.enableFanBtn;
    doc["enableMistBtn"] = config.enableMistBtn;
    doc["enableResetBtn"] = config.enableResetBtn;

    // Dynamic Automation Rule Bindings
    doc["fanSensor"] = config.fanSensor;
    doc["fanOperator"] = config.fanOperator;
    doc["fanThreshold"] = config.fanThreshold;
    doc["fanHysteresis"] = config.fanHysteresis;

    doc["mistSensor"] = config.mistSensor;
    doc["mistOperator"] = config.mistOperator;
    doc["mistThreshold"] = config.mistThreshold;
    doc["mistHysteresis"] = config.mistHysteresis;

    if (!isnan(soilMoisture)) doc["soilMoisture"] = soilMoisture;
    if (!isnan(lightPercent)) doc["lightPercent"] = lightPercent;
    if (!isnan(waterTemp)) doc["waterTemp"] = waterTemp;
    if (!isnan(bmeTemp)) doc["bmeTemp"] = bmeTemp;
    if (!isnan(bmeHum)) doc["bmeHum"] = bmeHum;
    if (!isnan(bmePress)) doc["bmePress"] = bmePress;

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

  // Line 1: Network IP
  display.setCursor(0, 0);
  if (isAPMode) {
    display.print("AP: 192.168.4.1");
  } else {
    display.printf("IP: %s", WiFi.localIP().toString().c_str());
  }

  // Line 2: Full Hardware MAC Address
  display.setCursor(0, 10);
  display.printf("MAC: %s", deviceMac.c_str());

  // Line 3: Protocol Badges (WS Clients & MQTT Connection)
  display.setCursor(0, 20);
  display.printf("WS:%d Cli | MQTT:%s", wsClientCount, mqttClient.connected() ? "ON" : "OFF");

  // Line 4: Divider
  display.drawLine(0, 29, 128, 29, SSD1306_WHITE);

  // Line 5: Temperature & Humidity Sensors (if enabled)
  display.setCursor(0, 33);
  if (!config.enableDht) {
    display.print("DHT: [Disabled]");
  } else if (isnan(temperature) || isnan(humidity)) {
    display.print("Sensor: DHT Error");
  } else {
    display.printf("T:%.1fC  H:%.1f%%", temperature, humidity);
  }

  // Line 6: Analog Sensor Reading (if enabled)
  display.setCursor(0, 43);
  if (!config.enableGas) {
    display.print("ADC: [Disabled]");
  } else if (config.mqType == 2) {
    display.printf("MQ2:%.0f%% Set:%.0f%s", gasPercent, gasThreshold, gasAlarm ? "!ALARM" : "");
  } else if (config.mqType == 135) {
    display.printf("MQ135:%.0f%% Set:%.0f%s", gasPercent, gasThreshold, gasAlarm ? "!ALARM" : "");
  } else if (config.mqType == 10) {
    display.printf("Soil:%.0f%% ADC:%d", soilMoisture, gasRaw);
  } else if (config.mqType == 20) {
    display.printf("Light:%.0f%% ADC:%d", lightPercent, gasRaw);
  } else {
    display.printf("ADC:%.0f%% Raw:%d", analogPercent, gasRaw);
  }

  // Line 7: Actuator Relays & Auto/Manual Mode
  display.setCursor(0, 54);
  String fStr = config.enableFan ? (fanState ? "ON" : "OFF") : "X";
  String mStr = config.enableMist ? (mistState ? "ON" : "OFF") : "X";
  display.printf("F:%s M:%s [%s]", fStr.c_str(), mStr.c_str(), autoMode ? "AUT" : "MAN");

  if (strlen(statusMsg) > 0) {
    display.fillRect(0, 48, 128, 16, SSD1306_BLACK);
    display.setCursor(0, 52);
    display.print(statusMsg);
  }

  display.display();
}

// --- Broadcast State to Local WebSockets & Cloud MQTT (Dynamic Lean Payload) ---
void broadcastAndPublishState() {
  JsonDocument doc;
  doc["mac"] = cleanMac;
  doc["autoMode"] = autoMode;
  doc["mqttConnected"] = mqttClient.connected();
  doc["wsClients"] = wsClientCount;
  doc["source"] = "esp-node";
  doc["timestamp"] = millis();

  // Module Enable Flags for Adaptive Dashboard Layout Sync
  doc["enableDht"] = config.enableDht;
  doc["enableGas"] = config.enableGas;
  doc["enableFan"] = config.enableFan;
  doc["enableMist"] = config.enableMist;
  doc["enableFanBtn"] = config.enableFanBtn;
  doc["enableMistBtn"] = config.enableMistBtn;
  doc["enableResetBtn"] = config.enableResetBtn;

  doc["enableSoil"] = config.enableSoil;
  doc["enableBme"] = config.enableBme;
  doc["enableDs18b20"] = config.enableDs18b20;
  doc["enableLdr"] = config.enableLdr;
  doc["enablePir"] = config.enablePir;
  doc["enableUltrasonic"] = config.enableUltrasonic;

  // 1. DHT Sensor Telemetry (Included ONLY if DHT is enabled)
  if (config.enableDht) {
    if (!isnan(temperature)) doc["temperature"] = serialized(String(temperature, 1));
    if (!isnan(humidity)) doc["humidity"] = serialized(String(humidity, 1));
    doc["tempThreshold"] = serialized(String(tempThreshold, 1));
    doc["mistThreshold"] = serialized(String(mistThreshold, 1));
  }

  // 2. Analog / ADC Sensor Telemetry (Included ONLY if Analog Sensor is enabled)
  if (config.enableGas) {
    doc["gasPercent"] = serialized(String(gasPercent, 1));
    doc["gasRaw"] = gasRaw;
    doc["adcRaw"] = gasRaw;
    doc["gasAlarm"] = gasAlarm;
    doc["mqType"] = config.mqType;
    doc["gasThreshold"] = serialized(String(gasThreshold, 1));
    doc["adcMin"] = config.adcMin;
    doc["adcMax"] = config.adcMax;
    doc["analogPercent"] = serialized(String(analogPercent, 1));
    if (config.mqType == 10 && !isnan(soilMoisture)) {
      doc["soilMoisture"] = serialized(String(soilMoisture, 1));
    }
    if (config.mqType == 20 && !isnan(lightPercent)) {
      doc["lightPercent"] = serialized(String(lightPercent, 1));
    }
  }

  // 3. Fan Actuator Metadata (State is handled exclusively via publishActuatorState)
  if (config.enableFan) {
    doc["fanName"] = config.fanName;
    if (config.enableFanBtn) {
      doc["fanBtnName"] = config.fanBtnName;
    }
  }

  // 4. Mist Actuator Metadata (State is handled exclusively via publishActuatorState)
  if (config.enableMist) {
    doc["mistName"] = config.mistName;
    if (config.enableMistBtn) {
      doc["mistBtnName"] = config.mistBtnName;
    }
  }

  // Dynamic Automation Rule Bindings
  doc["fanSensor"] = config.fanSensor;
  doc["fanOperator"] = config.fanOperator;
  doc["fanThreshold"] = serialized(String(config.fanThreshold, 1));
  doc["fanHysteresis"] = serialized(String(config.fanHysteresis, 1));

  doc["mistSensor"] = config.mistSensor;
  doc["mistOperator"] = config.mistOperator;
  doc["mistThreshold"] = serialized(String(config.mistThreshold, 1));
  doc["mistHysteresis"] = serialized(String(config.mistHysteresis, 1));

  // 5. Soil Moisture Sensor (Included ONLY if Soil Moisture is enabled)
  if (config.enableSoil && !isnan(soilMoisture)) {
    doc["soilMoisture"] = serialized(String(soilMoisture, 1));
  }

  // 6. BME280 Environmental Sensor (Included ONLY if BME280 is enabled)
  if (config.enableBme) {
    if (!isnan(bmeTemp)) doc["bmeTemp"] = serialized(String(bmeTemp, 1));
    if (!isnan(bmeHum)) doc["bmeHum"] = serialized(String(bmeHum, 1));
    if (!isnan(bmePress)) doc["bmePress"] = serialized(String(bmePress, 1));
  }

  // 7. DS18B20 Waterproof Temperature Sensor (Included ONLY if DS18B20 is enabled)
  if (config.enableDs18b20 && !isnan(waterTemp)) {
    doc["waterTemp"] = serialized(String(waterTemp, 1));
  }

  // 8. LDR Light Sensor (Included ONLY if LDR is enabled)
  if (config.enableLdr && !isnan(lightPercent)) {
    doc["lightPercent"] = serialized(String(lightPercent, 1));
  }

  // 9. PIR Motion Sensor (Included ONLY if PIR is enabled)
  if (config.enablePir) {
    doc["motionDetected"] = motionDetected;
  }

  // 10. HC-SR04 Ultrasonic Distance Sensor (Included ONLY if Ultrasonic is enabled)
  if (config.enableUltrasonic) {
    if (!isnan(distanceCm)) doc["distanceCm"] = serialized(String(distanceCm, 1));
    if (!isnan(tankPercent)) doc["tankPercent"] = serialized(String(tankPercent, 1));
  }

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

// --- Dedicated Instant Actuator / Switch Event Publisher (<5ms Lean Payload) ---
void publishActuatorState(const char* reason) {
  JsonDocument doc;
  doc["event"] = "actuator_change";
  doc["reason"] = reason;
  doc["mac"] = cleanMac;
  doc["autoMode"] = autoMode;
  doc["fanState"] = fanState;
  doc["mistState"] = mistState;
  doc["timestamp"] = millis();

  String jsonString;
  serializeJson(doc, jsonString);

  // 1. Instant WebSocket broadcast to LAN clients
  webSocket.broadcastTXT(jsonString);

  // 2. Instant MQTT publish to Cloud broker
  if (mqttClient.connected()) {
    mqttClient.publish(pubTopic.c_str(), jsonString.c_str());
  }

  updateOledDisplay();
}

// --- Dynamic Sensor-to-Actuator Rule Engine ---
float getSensorValue(const String& source) {
  if (source == "temp")      return temperature;
  if (source == "hum")       return humidity;
  if (source == "soil")      return soilMoisture;
  if (source == "gas")       return gasPercent;
  if (source == "ldr")       return lightPercent;
  if (source == "ds18b20")   return waterTemp;
  if (source == "bme_temp")  return bmeTemp;
  if (source == "bme_hum")   return bmeHum;
  if (source == "bme_press") return bmePress;
  return NAN;
}

bool evaluateRule(float val, const String& op, float threshold, float hysteresis, bool currentState) {
  if (isnan(val)) return currentState;
  if (op == "<") {
    if (val <= threshold) return true;
    else if (val > (threshold + hysteresis)) return false;
  } else { // default ">"
    if (val >= threshold) return true;
    else if (val < (threshold - hysteresis)) return false;
  }
  return currentState;
}

// --- Centralized Command Handler (Event-Driven Real-time Execution) ---
void handleIncomingCommand(JsonDocument& doc, const char* source) {
  String cmd = doc["command"].as<String>();

  Serial.printf("[%s CMD] Command: %s\n", source, cmd.c_str());

  if (cmd == "toggle_fan") {
    if (config.enableFan) {
      fanState = !fanState;
      digitalWrite(config.fanRelayPin, fanState ? HIGH : LOW);
      autoMode = false;
      publishActuatorState("cmd_toggle_fan");
      return;
    }
  } else if (cmd == "toggle_mist") {
    if (config.enableMist) {
      mistState = !mistState;
      digitalWrite(config.mistRelayPin, mistState ? HIGH : LOW);
      autoMode = false;
      publishActuatorState("cmd_toggle_mist");
      return;
    }
  } else if (cmd == "set_fan") {
    if (config.enableFan) {
      if (doc.containsKey("state")) {
        if (doc["state"].is<bool>()) fanState = doc["state"].as<bool>();
        else if (doc["state"].is<int>()) fanState = (doc["state"].as<int>() == 1);
        else if (doc["state"].is<String>()) {
          String s = doc["state"].as<String>();
          s.toLowerCase();
          fanState = (s == "true" || s == "1" || s == "on");
        }
      } else if (doc.containsKey("fanState")) {
        if (doc["fanState"].is<bool>()) fanState = doc["fanState"].as<bool>();
        else if (doc["fanState"].is<int>()) fanState = (doc["fanState"].as<int>() == 1);
      }
      digitalWrite(config.fanRelayPin, fanState ? HIGH : LOW);
      autoMode = false;
      Serial.printf("[CMD] Set Fan: %s (Auto Mode disabled -> Manual)\n", fanState ? "ON" : "OFF");
      publishActuatorState("cmd_set_fan");
      return;
    }
  } else if (cmd == "set_mist") {
    if (config.enableMist) {
      if (doc.containsKey("state")) {
        if (doc["state"].is<bool>()) mistState = doc["state"].as<bool>();
        else if (doc["state"].is<int>()) mistState = (doc["state"].as<int>() == 1);
        else if (doc["state"].is<String>()) {
          String s = doc["state"].as<String>();
          s.toLowerCase();
          mistState = (s == "true" || s == "1" || s == "on");
        }
      } else if (doc.containsKey("mistState")) {
        if (doc["mistState"].is<bool>()) mistState = doc["mistState"].as<bool>();
        else if (doc["mistState"].is<int>()) mistState = (doc["mistState"].as<int>() == 1);
      }
      digitalWrite(config.mistRelayPin, mistState ? HIGH : LOW);
      autoMode = false;
      Serial.printf("[CMD] Set Mist: %s (Auto Mode disabled -> Manual)\n", mistState ? "ON" : "OFF");
      publishActuatorState("cmd_set_mist");
      return;
    }
  } else if (cmd == "set_threshold" || cmd == "set_fan_threshold") {
    if (doc["threshold"].is<float>()) config.fanThreshold = doc["threshold"].as<float>();
    if (doc["fanThreshold"].is<float>()) config.fanThreshold = doc["fanThreshold"].as<float>();
    if (doc["tempThreshold"].is<float>()) config.fanThreshold = doc["tempThreshold"].as<float>();
    tempThreshold = config.fanThreshold;
    broadcastAndPublishState();
    return;
  } else if (cmd == "set_mist_threshold") {
    if (doc["threshold"].is<float>()) config.mistThreshold = doc["threshold"].as<float>();
    if (doc["mistThreshold"].is<float>()) config.mistThreshold = doc["mistThreshold"].as<float>();
    mistThreshold = config.mistThreshold;
    broadcastAndPublishState();
    return;
  } else if (cmd == "set_gas_threshold") {
    if (doc["threshold"].is<float>()) gasThreshold = doc["threshold"].as<float>();
    if (doc["gasThreshold"].is<float>()) gasThreshold = doc["gasThreshold"].as<float>();
    config.gasThreshold = gasThreshold;
    broadcastAndPublishState();
    return;
  } else if (cmd == "set_rule") {
    int relay = doc["relay"].as<int>();
    if (relay == 1) {
      if (doc["sensor"].is<String>()) config.fanSensor = doc["sensor"].as<String>();
      if (doc["operator"].is<String>()) config.fanOperator = doc["operator"].as<String>();
      if (doc["threshold"].is<float>()) config.fanThreshold = doc["threshold"].as<float>();
      if (doc["hysteresis"].is<float>()) config.fanHysteresis = doc["hysteresis"].as<float>();
      tempThreshold = config.fanThreshold;
    } else if (relay == 2) {
      if (doc["sensor"].is<String>()) config.mistSensor = doc["sensor"].as<String>();
      if (doc["operator"].is<String>()) config.mistOperator = doc["operator"].as<String>();
      if (doc["threshold"].is<float>()) config.mistThreshold = doc["threshold"].as<float>();
      if (doc["hysteresis"].is<float>()) config.mistHysteresis = doc["hysteresis"].as<float>();
      mistThreshold = config.mistThreshold;
    }
    broadcastAndPublishState();
    return;
  } else if (cmd == "set_mode") {
    if (doc.containsKey("autoMode")) {
      if (doc["autoMode"].is<bool>()) autoMode = doc["autoMode"].as<bool>();
      else if (doc["autoMode"].is<int>()) autoMode = (doc["autoMode"].as<int>() == 1);
      else if (doc["autoMode"].is<String>()) {
        String m = doc["autoMode"].as<String>();
        m.toLowerCase();
        autoMode = (m == "true" || m == "auto" || m == "1");
      }
    } else if (doc.containsKey("mode")) {
      String m = doc["mode"].as<String>();
      m.toLowerCase();
      autoMode = (m == "auto" || m == "true" || m == "1");
    } else if (doc.containsKey("state")) {
      autoMode = doc["state"].as<bool>();
    }
    
    Serial.printf("[CMD] Mode switch: %s\n", autoMode ? "AUTO" : "MANUAL");

    // When switching to Auto mode, immediately evaluate dynamic rules
    if (autoMode) {
      if (config.enableFan) {
        bool isGasHazard = config.enableGas && gasAlarm && (config.mqType == 2 || config.mqType == 135);
        bool shouldFan = fanState;
        if (isGasHazard) {
          shouldFan = true;
        } else if (config.fanSensor != "none") {
          float val = getSensorValue(config.fanSensor);
          shouldFan = evaluateRule(val, config.fanOperator, config.fanThreshold, config.fanHysteresis, fanState);
        }
        fanState = shouldFan;
        digitalWrite(config.fanRelayPin, fanState ? HIGH : LOW);
      }
      if (config.enableMist && config.mistSensor != "none") {
        float val = getSensorValue(config.mistSensor);
        mistState = evaluateRule(val, config.mistOperator, config.mistThreshold, config.mistHysteresis, mistState);
        digitalWrite(config.mistRelayPin, mistState ? HIGH : LOW);
      }
    }
    publishActuatorState("cmd_set_mode");
    return;
  } else if (cmd == "toggle_mode") {
    autoMode = !autoMode;
    if (autoMode) {
      if (config.enableFan) {
        bool isGasHazard = config.enableGas && gasAlarm && (config.mqType == 2 || config.mqType == 135);
        bool shouldFan = fanState;
        if (isGasHazard) {
          shouldFan = true;
        } else if (config.fanSensor != "none") {
          float val = getSensorValue(config.fanSensor);
          shouldFan = evaluateRule(val, config.fanOperator, config.fanThreshold, config.fanHysteresis, fanState);
        }
        fanState = shouldFan;
        digitalWrite(config.fanRelayPin, fanState ? HIGH : LOW);
      }
      if (config.enableMist && config.mistSensor != "none") {
        float val = getSensorValue(config.mistSensor);
        mistState = evaluateRule(val, config.mistOperator, config.mistThreshold, config.mistHysteresis, mistState);
        digitalWrite(config.mistRelayPin, mistState ? HIGH : LOW);
      }
    }
    publishActuatorState("cmd_toggle_mode");
    return;
  } else if (cmd == "ping" || cmd == "get_state") {
    publishActuatorState("sync");
    broadcastAndPublishState();
    return;
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
      
      // Immediately send current actuator state and sensor telemetry
      publishActuatorState("connect_sync");
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
  if (WiFi.status() != WL_CONNECTED) return;
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
      if (!connected) {
        // Fallback to anonymous connection on public brokers
        connected = mqttClient.connect(clientId.c_str());
      }
    } else {
      connected = mqttClient.connect(clientId.c_str());
    }

    if (connected) {
      Serial.println("✓ [MQTT] Connected successfully!");
      mqttClient.subscribe(subTopic.c_str());
      Serial.printf("✓ [MQTT] Subscribed to unique topic: %s\n", subTopic.c_str());
      Serial.printf("✓ [MQTT] Publishing to unique topic: %s\n", pubTopic.c_str());
      publishActuatorState("connect_sync");
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
#if defined(ESP8266)
  if (!LittleFS.begin()) {
    Serial.println("✕ LittleFS mount failed! Formatting...");
    LittleFS.format();
    LittleFS.begin();
  }
#else
  if (!LittleFS.begin(true)) {
    Serial.println("✕ LittleFS mount failed! Formatting...");
    LittleFS.format();
    LittleFS.begin(true);
  }
#endif
  Serial.println("✓ LittleFS Mounted Successfully");

  // 2. Load Configuration
  loadConfiguration();

  // 3. Obtain Hardware MAC Address & Build Unique Topics
  WiFi.mode(WIFI_STA);
  uint8_t baseMac[6] = {0};
#if defined(ESP8266)
  WiFi.macAddress(baseMac);
#else
  esp_err_t ret = esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  if (ret != ESP_OK || (baseMac[0] == 0 && baseMac[1] == 0 && baseMac[2] == 0 && baseMac[3] == 0 && baseMac[4] == 0 && baseMac[5] == 0)) {
    esp_read_mac(baseMac, ESP_MAC_EFUSE_FACTORY);
  }
#endif
  if (baseMac[0] == 0 && baseMac[1] == 0 && baseMac[2] == 0 && baseMac[3] == 0 && baseMac[4] == 0 && baseMac[5] == 0) {
    String wm = WiFi.macAddress();
    if (wm.length() > 0 && wm != "00:00:00:00:00:00") {
      deviceMac = wm;
    } else {
      deviceMac = "00:00:00:00:00:01";
    }
  } else {
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
    deviceMac = String(macStr);
  }
  deviceMac.toUpperCase();
  cleanMac = deviceMac;
  cleanMac.replace(":", "");
  cleanMac.toUpperCase();

  String pfx = config.topicPrefix;
  pfx.trim();
  while (pfx.endsWith("/")) pfx = pfx.substring(0, pfx.length() - 1);
  if (pfx.length() > 0) pfx += "/";

  subTopic = pfx + "esp-node/" + cleanMac + "/control/cmd";
  pubTopic = pfx + "esp-node/" + cleanMac + "/state";
#if defined(ESP8266)
  clientId = "ESP8266_" + cleanMac;
#else
  clientId = "ESP32_" + cleanMac;
#endif

  Serial.println("--------------------------------------------------");
  Serial.printf("  Device MAC:        %s\n", deviceMac.c_str());
  Serial.printf("  Unique Clean ID:   %s\n", cleanMac.c_str());
  Serial.printf("  Topic Prefix:      %s\n", config.topicPrefix.c_str());
  Serial.printf("  MQTT Sub Topic:    %s\n", subTopic.c_str());
  Serial.printf("  MQTT Pub Topic:    %s\n", pubTopic.c_str());
  Serial.println("--------------------------------------------------");

  // 4. Initialize OLED I2C Display
#if defined(ESP8266)
  Wire.begin(4, 5); // SDA=GPIO4 (D2), SCL=GPIO5 (D1)
  Wire.setClock(400000);
#else
  Wire.begin(21, 22);
  Wire.setClock(400000);
#endif
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    oledAvailable = true;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 4);
    display.println(" LAB6_Perform IoT");
    display.println(" Starting System...");
    display.printf(" MAC: %s\n", deviceMac.c_str());
    display.printf(" ID:  %s\n", cleanMac.c_str());
    display.display();
  }

  // 5. Initialize Hardware Pins
  initHardware();

  // 6. Connect to WiFi or fallback to AP Mode
  if (config.ssid.length() > 0) {
    Serial.printf("Connecting to Wi-Fi SSID: %s ...\n", config.ssid.c_str());
    WiFi.mode(WIFI_STA);
#if defined(ESP32)
    WiFi.setSleep(false); // Disable WiFi Modem Sleep for ultra-low latency (<5ms)
#endif
    WiFi.begin(config.ssid.c_str(), config.password.c_str());

    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
      delay(300);
      Serial.print(".");
    }
  }

  if (config.ssid.length() > 0 && WiFi.status() == WL_CONNECTED) {
    isAPMode = false;
#if defined(ESP32)
    WiFi.setSleep(false); // Ensure WiFi Sleep remains disabled
#endif
    Serial.println("\n✓ Wi-Fi Connected!");
    Serial.print("  IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.printf("  Open Dashboard: http://%s/\n", WiFi.localIP().toString().c_str());
    Serial.printf("  Open Config:    http://%s/config.html\n", WiFi.localIP().toString().c_str());
  } else {
    // Start Fallback / Factory Default AP Mode
    isAPMode = true;
    String apName = "ESP-Config-" + cleanMac.substring(cleanMac.length() > 4 ? cleanMac.length() - 4 : 0);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apName.c_str(), "12345678");
    IPAddress apIP(192, 168, 4, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    dnsServer.start(53, "*", apIP);

    Serial.println("\n! WiFi Unconfigured. Starting AP Mode (Captive Portal).");
    Serial.printf("  SSID: %s (Password: 12345678)\n", apName.c_str());
    Serial.println("  IP:   http://192.168.4.1/config.html");
    updateOledDisplay("AP Mode: 192.168.4.1");
  }

  // 7. Setup Web Server & WebSockets
  setupWebServer();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("✓ WebSockets Server started on Port 81");

  // 8. Setup MQTT Client
  mqttClient.setServer(config.mqttServer.c_str(), config.mqttPort);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(2048);
  mqttClient.setSocketTimeout(2);  // Non-blocking 2-second timeout (avoids 15s freeze)
  mqttClient.setKeepAlive(30);

  updateOledDisplay("System Ready");
}

// --- Main Execution Loop ---
void loop() {
  // Handle Scheduled Clean Reboot (Graceful Teardown)
  if (rebootPending && millis() > rebootTime) {
    Serial.println("\n[SYSTEM] Gracefully shutting down services before MCU restart...");
    // 1. Turn off relays to ensure safe strapping pin levels
    if (config.enableFan) digitalWrite(config.fanRelayPin, LOW);
    if (config.enableMist) digitalWrite(config.mistRelayPin, LOW);
    
    // 2. Disconnect network services
    webSocket.close();
    if (mqttClient.connected()) mqttClient.disconnect();
    server.stop();
    
    // 3. Unmount LittleFS
    LittleFS.end();
    
    // 4. Disconnect Wi-Fi cleanly
    WiFi.disconnect(true);
    delay(200);
    
    Serial.println("[SYSTEM] Rebooting MCU now...");
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

    // 1. Read DHT Sensor (if enabled)
    if (config.enableDht && dht != nullptr) {
      float t = dht->readTemperature();
      float h = dht->readHumidity();
      if (!isnan(t) && !isnan(h)) {
        temperature = t;
        humidity = h;
      }
    } else if (!config.enableDht) {
      temperature = NAN;
      humidity = NAN;
    }

    // 2. Read Analog / ADC Sensor (MQ-2, MQ-135, Soil Moisture, LDR, VR)
    if (config.enableGas) {
#if defined(ESP8266)
      int rawAnalog = analogRead(A0);
#else
      int rawAnalog = analogRead(config.analogPin);
#endif
      gasRaw = rawAnalog;

      if (config.mqType == 10) {
        // Soil Moisture Sensor with Calibration
        int dryVal = config.adcMin;
        int wetVal = config.adcMax;
        if (dryVal == wetVal) {
#if defined(ESP8266)
          soilMoisture = constrain((1.0 - (rawAnalog / 1023.0)) * 100.0, 0.0, 100.0);
#else
          soilMoisture = constrain((1.0 - (rawAnalog / 4095.0)) * 100.0, 0.0, 100.0);
#endif
        } else {
          float pct = ((float)(rawAnalog - dryVal) / (float)(wetVal - dryVal)) * 100.0;
          soilMoisture = constrain(pct, 0.0, 100.0);
        }
        gasPercent = soilMoisture;
        analogPercent = soilMoisture;
        gasAlarm = false;
      } else {
        // Generic ADC / LDR / Gas with Calibration
        int minVal = config.adcMin;
        int maxVal = config.adcMax;
        if (minVal == maxVal) {
#if defined(ESP8266)
          analogPercent = (rawAnalog / 1023.0) * 100.0;
#else
          analogPercent = (rawAnalog / 4095.0) * 100.0;
#endif
        } else {
          float pct = ((float)(rawAnalog - minVal) / (float)(maxVal - minVal)) * 100.0;
          analogPercent = constrain(pct, 0.0, 100.0);
        }
        gasPercent = analogPercent;

        if (config.mqType == 20) {
          // LDR Light Sensor
          lightPercent = analogPercent;
          gasAlarm = false;
        } else if (config.mqType == 2 || config.mqType == 135) {
          gasAlarm = (gasPercent >= gasThreshold);
        } else {
          // Generic ADC 0-100%
          gasAlarm = false;
        }
      }
    } else {
      gasRaw = 0;
      gasPercent = 0.0;
      analogPercent = 0.0;
      if (config.mqType == 10) soilMoisture = NAN;
      if (config.mqType == 20) lightPercent = NAN;
      gasAlarm = false;
    }



    // 4. Read Soil Moisture Sensor (if legacy enableSoil is used independently)
    if (config.enableSoil && config.mqType != 10) {
#if defined(ESP8266)
      int rawSoil = analogRead(A0);
      soilMoisture = constrain((1.0 - (rawSoil / 1023.0)) * 100.0, 0.0, 100.0);
#else
      int rawSoil = analogRead(config.soilPin);
      soilMoisture = constrain((1.0 - (rawSoil / 4095.0)) * 100.0, 0.0, 100.0);
#endif
    } else if (!config.enableSoil && config.mqType != 10) {
      soilMoisture = NAN;
    }

    // 5. Read BME280 Environmental Sensor (if enabled)
    if (config.enableBme && bme != nullptr) {
      float bt = bme->readTemperature();
      float bh = bme->readHumidity();
      float bp = bme->readPressure() / 100.0F; // Pa to hPa
      if (!isnan(bt) && !isnan(bh)) {
        bmeTemp = bt;
        bmeHum = bh;
        bmePress = bp;
      }
    } else if (!config.enableBme) {
      bmeTemp = NAN;
      bmeHum = NAN;
      bmePress = NAN;
    }

    // 6. Read DS18B20 1-Wire Waterproof Temperature Sensor (if enabled)
    if (config.enableDs18b20 && ds18b20 != nullptr) {
      ds18b20->requestTemperatures();
      float wt = ds18b20->getTempCByIndex(0);
      if (wt > -55.0 && wt < 125.0) {
        waterTemp = wt;
      }
    } else if (!config.enableDs18b20) {
      waterTemp = NAN;
    }

    // 7. Read LDR Light Sensor (if legacy enableLdr is used independently)
    if (config.enableLdr && config.mqType != 20) {
#if defined(ESP8266)
      int rawLdr = analogRead(A0);
      lightPercent = (rawLdr / 1023.0) * 100.0;
#else
      int rawLdr = analogRead(config.ldrPin);
      lightPercent = (rawLdr / 4095.0) * 100.0;
#endif
    } else if (!config.enableLdr && config.mqType != 20) {
      lightPercent = NAN;
    }

    // 8. Read PIR Motion Sensor (if enabled)
    if (config.enablePir) {
      motionDetected = (digitalRead(config.pirPin) == HIGH);
    } else {
      motionDetected = false;
    }

    // 9. Read HC-SR04 Ultrasonic Distance Sensor (if enabled)
    if (config.enableUltrasonic) {
      digitalWrite(config.trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(config.trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(config.trigPin, LOW);
      long duration = pulseIn(config.echoPin, HIGH, 25000); // 25ms timeout (~4.3m max)
      if (duration > 0) {
        distanceCm = (duration * 0.0343) / 2.0;
        // Map distance to water tank level % (assuming 100cm tank depth)
        tankPercent = constrain((1.0 - (distanceCm / 100.0)) * 100.0, 0.0, 100.0);
      } else {
        distanceCm = NAN;
        tankPercent = NAN;
      }
    } else {
      distanceCm = NAN;
      tankPercent = NAN;
    }

    // 10. Dynamic Relay Automation Rule Evaluator (Executed after all sensors are updated)
    if (autoMode) {
      // Relay 1 (Fan) Automation
      if (config.enableFan) {
        bool isGasHazard = config.enableGas && gasAlarm && (config.mqType == 2 || config.mqType == 135);
        bool shouldFan = fanState;
        if (isGasHazard) {
          shouldFan = true;
        } else if (config.fanSensor != "none") {
          float val = getSensorValue(config.fanSensor);
          shouldFan = evaluateRule(val, config.fanOperator, config.fanThreshold, config.fanHysteresis, fanState);
        }

        if (shouldFan != fanState) {
          fanState = shouldFan;
          digitalWrite(config.fanRelayPin, fanState ? HIGH : LOW);
          publishActuatorState(fanState ? "auto_fan_on" : "auto_fan_off");
          if (isGasHazard) {
            Serial.printf("[AUTO ALERT] Gas/Smoke level exceeded (%.1f%% >= %.1f%%) -> Emergency Fan ON\n", gasPercent, gasThreshold);
          } else {
            Serial.printf("[AUTO] Fan Relay %s (Sensor: %s, Val: %.1f, Set: %.1f)\n", fanState ? "ON" : "OFF", config.fanSensor.c_str(), getSensorValue(config.fanSensor), config.fanThreshold);
          }
        }
      }

      // Relay 2 (Mist) Automation
      if (config.enableMist && config.mistSensor != "none") {
        float val = getSensorValue(config.mistSensor);
        bool shouldMist = evaluateRule(val, config.mistOperator, config.mistThreshold, config.mistHysteresis, mistState);
        if (shouldMist != mistState) {
          mistState = shouldMist;
          digitalWrite(config.mistRelayPin, mistState ? HIGH : LOW);
          publishActuatorState(mistState ? "auto_mist_on" : "auto_mist_off");
          Serial.printf("[AUTO] Mist Relay %s (Sensor: %s, Val: %.1f, Set: %.1f)\n", mistState ? "ON" : "OFF", config.mistSensor.c_str(), val, config.mistThreshold);
        }
      }
    }

    // Synchronize state across all channels
    broadcastAndPublishState();
  }

  // 1. Factory Reset Button Handling (Long-press >= 3s on resetButtonPin GPIO 0)
  if (config.enableResetBtn) {
    int resetReading = digitalRead(config.resetButtonPin);
    if (resetReading == LOW) {
      if (!resetHolding) {
        resetHolding = true;
        resetPressStartTime = millis();
        resetExecuted = false;
        Serial.println("[BTN] Reset button (GPIO 0) pressed. Hold for 3s to enter AP Mode...");
      } else {
        unsigned long holdDuration = millis() - resetPressStartTime;
        if (holdDuration < 3000) {
          int secLeft = 3 - (holdDuration / 1000);
          char buf[32];
          snprintf(buf, sizeof(buf), "HOLD RESET: %ds", secLeft);
          updateOledDisplay(buf);
        } else if (!resetExecuted) {
          resetExecuted = true;
          Serial.println("\n[RESET] Factory Reset 3s hold detected! Erasing /config.json and entering AP Mode...");
          updateOledDisplay("RESETTING TO AP...");

          if (LittleFS.exists("/config.json")) {
            LittleFS.remove("/config.json");
          }

          delay(400);
          // Graceful teardown
          if (config.enableFan) digitalWrite(config.fanRelayPin, LOW);
          if (config.enableMist) digitalWrite(config.mistRelayPin, LOW);
          webSocket.close();
          if (mqttClient.connected()) mqttClient.disconnect();
          server.stop();
          LittleFS.end();
          WiFi.disconnect(true);
          delay(200);
          ESP.restart();
        }
      }
    } else {
      if (resetHolding && !resetExecuted) {
        updateOledDisplay("Ready");
      }
      resetHolding = false;
    }
  }

  // 2. Physical Fan Toggle Button Debounce
  if (config.enableFan && config.enableFanBtn) {
    int fanBtnReading = digitalRead(config.fanButtonPin);
    if (fanBtnReading != lastFanBtnReading) lastFanDebounceTime = millis();
    if ((millis() - lastFanDebounceTime) > debounceDelay) {
      if (fanBtnReading != currentFanBtnState) {
        currentFanBtnState = fanBtnReading;
        if (currentFanBtnState == LOW) {
          fanState = !fanState;
          digitalWrite(config.fanRelayPin, fanState ? HIGH : LOW);
          autoMode = false;
          publishActuatorState("btn_fan_press");
        }
      }
    }
    lastFanBtnReading = fanBtnReading;
  }

  // 3. Physical Mist Toggle Button Debounce
  if (config.enableMist && config.enableMistBtn) {
    int mistBtnReading = digitalRead(config.mistButtonPin);
    if (mistBtnReading != lastMistBtnReading) lastMistDebounceTime = millis();
    if ((millis() - lastMistDebounceTime) > debounceDelay) {
      if (mistBtnReading != currentMistBtnState) {
        currentMistBtnState = mistBtnReading;
        if (currentMistBtnState == LOW) {
          mistState = !mistState;
          digitalWrite(config.mistRelayPin, mistState ? HIGH : LOW);
          autoMode = false;
          publishActuatorState("btn_mist_press");
        }
      }
    }
    lastMistBtnReading = mistBtnReading;
  }
}
