/**
 * Lab 5: Cloud MQTT & Cross-Platform Dashboard (Complete Code Solution)
 * Features:
 *  - Interfacing SSD1306 OLED via I2C to show real-time states and network info.
 *  - Connecting to Wi-Fi and EMQX Public Cloud MQTT Broker (broker.emqx.io:1883).
 *  - Subscribing to "esp-node/control/cmd" to receive remote controls (action/value/threshold in JSON).
 *  - Full feature parity with Lab 4:
 *    - Temp, Humidity (DHT11), Analog % (KNOB / Soil moisture sensor).
 *    - Fan Relay (GPIO 5 on ESP32 / GPIO 13 on ESP8266).
 *    - Mist Relay (GPIO 23 on ESP32 / GPIO 16 on ESP8266).
 *    - Dynamic Temperature Threshold (Slider) & Hysteresis Logic.
 *    - Auto / Manual mode toggling.
 *    - Physical button debounce & press counter (GPIO 0).
 *  - Publishes full state in JSON format to "esp-node/state" periodically and upon state changes.
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// Wi-Fi Credentials
const char* ssid = "iot_512";
const char* password = "iot123456";

// MQTT Broker Settings
const char* mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
const char* subTopic = "esp-node/control/cmd";
const char* pubTopic = "esp-node/state";
const char* mqttUser = "elec";
const char* mqttPassword = "elec1234";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool oledAvailable = false;

// Hardware Pin Definitions
#if defined(ESP8266)
#define DHTPIN 2            // D4/GPIO 2 สำหรับ AX-WiFi (หรือ GPIO 0 / D3)
#define ANALOG_PIN A0       // A0 (ตัวต้านทานปรับค่าได้ VR สำหรับ AX-WiFi)
#define FAN_RELAY_PIN 13    // D7/GPIO 13 สำหรับ AX-WiFi
#define MIST_RELAY_PIN 16   // D0/GPIO 16 สำหรับ AX-WiFi
#define BUTTON_PIN 0        // D3/GPIO 0 (ปุ่ม FLASH บนบอร์ด AX-WiFi)
#define ADC_RESOLUTION 1023.0
#elif defined(ESP32)
#define DHTPIN 33           // พอร์ต 33 สำหรับ IPST-WiFi
#define ANALOG_PIN 36       // GPIO 36 / KNOB-S สำหรับ IPST-WiFi
#define FAN_RELAY_PIN 5     // พอร์ต 5 สำหรับ IPST-WiFi
#define MIST_RELAY_PIN 23   // พอร์ต 23 สำหรับ IPST-WiFi
#define BUTTON_PIN 0        // GPIO 0 (ปุ่ม SW1 บนบอร์ด IPST-WiFi)
#define ADC_RESOLUTION 4095.0
#endif
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// Environmental data
float temperature = 0;
float humidity = 0;
float analogPercent = 0;

// Dynamic control parameters
float tempThreshold = 30.0; // Default temperature threshold (changeable via dashboard slider)
bool fanState = false;
bool mistState = false;
int toggleCount = 0;        // Physical button press counter
bool autoMode = true;       // Mode flag: true = Auto (hysteresis control), false = Manual

// Helper to draw OLED interface
void updateOledDisplay(const char* statusMsg = "") {
  if (!oledAvailable) return;
  display.clearDisplay();
  
  // 1. Header
  display.setCursor(10, 0);
  display.print("MQTT IOT DASHBOARD");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  // 2. Sensor Values
  display.setCursor(0, 14);
  if (isnan(temperature)) {
    display.print("Temp: -- C");
  } else {
    display.printf("Temp: %.1f C  Th:%.1f", temperature, tempThreshold);
  }
  
  display.setCursor(0, 24);
  if (isnan(humidity)) {
    display.print("Humid: -- %%");
  } else {
    display.printf("Humid: %.1f %%", humidity);
  }
  
  display.setCursor(0, 34);
  display.printf("KNOB: %.1f %% | %s", analogPercent, autoMode ? "AUTO" : "MANU");
  
  // 3. Network Connection Status
  display.setCursor(0, 44);
  if (statusMsg && strlen(statusMsg) > 0) {
    display.print(statusMsg);
  } else {
    display.printf("MQTT: %s", mqttClient.connected() ? "Connected" : "Offline");
  }
  
  // 4. Relay and Button Press Count
  display.drawLine(0, 53, 128, 53, SSD1306_WHITE);
  display.setCursor(0, 56);
  display.printf("Fan:%s Mist:%s Cnt:%d", fanState ? "ON" : "OFF", mistState ? "ON" : "OFF", toggleCount);
  
  display.display();
}

// Publish full sensor data to MQTT state topic
void publishSensorState() {
  if (!mqttClient.connected()) return;

  JsonDocument doc;
  doc["temp"] = isnan(temperature) ? 0.0 : (round(temperature * 10.0) / 10.0);
  doc["humidity"] = isnan(humidity) ? 0.0 : (round(humidity * 10.0) / 10.0);
  doc["soil"] = round(analogPercent * 10.0) / 10.0;
  doc["fan"] = fanState;
  doc["mist"] = mistState;
  doc["threshold"] = round(tempThreshold * 10.0) / 10.0;
  doc["press"] = toggleCount;
  doc["mode"] = autoMode;
  
  String output;
  serializeJson(doc, output);
  
  mqttClient.publish(pubTopic, output.c_str());
  Serial.printf("[MQTT PUB -> %s]: %s\n", pubTopic, output.c_str());
}

// MQTT Callback Function (Receives commands from Web Dashboard)
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.printf("[MQTT SUB <- %s]: ", topic);
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  
  bool stateChanged = false;

  // 1. รับคำสั่ง Action ต่างๆ
  if (doc["action"].is<JsonVariant>()) {
    String action = doc["action"];
    if (action == "toggle_fan") {
      fanState = doc["value"];
      autoMode = false; // Switch to Manual mode upon user manual override
      digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
      Serial.printf("MQTT: Fan toggled manually to %s (Manual Mode)\n", fanState ? "ON" : "OFF");
      stateChanged = true;
    } else if (action == "toggle_mist") {
      mistState = doc["value"];
      digitalWrite(MIST_RELAY_PIN, mistState ? HIGH : LOW);
      Serial.printf("MQTT: Mist toggled manually to %s\n", mistState ? "ON" : "OFF");
      stateChanged = true;
    } else if (action == "toggle_mode") {
      autoMode = doc["value"];
      Serial.printf("MQTT: Mode toggled to %s\n", autoMode ? "Auto" : "Manual");
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
      tempThreshold = doc["value"];
      autoMode = true; // Auto mode active
      Serial.printf("MQTT: Temperature Threshold updated to %.1f C\n", tempThreshold);
      if (!isnan(temperature)) {
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
  
  // 2. รับค่า threshold จากสไลเดอร์โดยตรง (เช่น {"threshold": 32.5})
  if (doc["threshold"].is<JsonVariant>()) {
    tempThreshold = doc["threshold"];
    autoMode = true; // Switch back to Auto mode
    Serial.printf("MQTT: Temperature Threshold updated to %.1f C (Auto Mode)\n", tempThreshold);
    if (!isnan(temperature)) {
      if (temperature > tempThreshold) {
        fanState = true;
      } else if (temperature < (tempThreshold - 0.5)) {
        fanState = false;
      }
      digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
    }
    stateChanged = true;
  }

  if (stateChanged) {
    updateOledDisplay("Command Recv!");
    publishSensorState();
  }
}

// Reconnect to MQTT Broker
void reconnectMqtt() {
  static unsigned long lastReconnectAttempt = 0;
  unsigned long now = millis();
  
  if (now - lastReconnectAttempt > 5000) {
    lastReconnectAttempt = now;
    Serial.print("Attempting MQTT connection...");
    
    String clientId = "ESPNode-" + String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str(), mqttUser, mqttPassword)) {
      Serial.println("connected to MQTT broker!");
      mqttClient.subscribe(subTopic);
      Serial.printf("Subscribed to: %s\n", subTopic);
      updateOledDisplay("MQTT Connected");
      publishSensorState();
    } else {
      Serial.printf("failed, rc=%d. Will try again in 5s\n", mqttClient.state());
      updateOledDisplay("MQTT Retrying...");
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // Set Pin Modes
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(MIST_RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(FAN_RELAY_PIN, LOW);
  digitalWrite(MIST_RELAY_PIN, LOW);
  
  #if defined(ESP32)
  analogSetPinAttenuation(ANALOG_PIN, ADC_11db);
  #endif
  
  // Initialize OLED (Address 0x3C)
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    oledAvailable = true;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 15);
    display.println("Connecting Wi-Fi...");
    display.display();
  }
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected!");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  
  if (oledAvailable) {
    display.clearDisplay();
    display.setCursor(10, 15);
    display.println("Wi-Fi Connected!");
    display.setCursor(0, 30);
    display.println(WiFi.localIP().toString());
    display.display();
    delay(1000);
  }
  
  // Configure MQTT
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(512);
  
  updateOledDisplay("System Ready");
  Serial.println("System initialized. Running MQTT communication loop.");
}

void loop() {
  // 1. Maintain MQTT connection
  if (!mqttClient.connected()) {
    reconnectMqtt();
  } else {
    mqttClient.loop();
  }
  
  // 2. Physical Button Debounce & Toggle Control
  int reading = digitalRead(BUTTON_PIN);
  static bool lastButtonState = HIGH;
  static bool currentButtonState = HIGH;
  static unsigned long lastDebounceTime = 0;
  static unsigned long debounceDelay = 50;
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != currentButtonState) {
      currentButtonState = reading;
      if (currentButtonState == LOW) {
        fanState = !fanState;
        autoMode = false; // Switch to Manual mode upon physical override
        toggleCount++;
        digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
        Serial.printf("Physical Button Pressed! Fan: %s, Count: %d (Manual Mode)\n", fanState ? "ON" : "OFF", toggleCount);
        updateOledDisplay("Btn Pressed!");
        publishSensorState();
      }
    }
  }
  lastButtonState = reading;
  
  // 3. Periodic Sensor Readings and Auto Hysteresis Logic
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 3000) {
    lastUpdate = millis();
    
    // Read sensors
    float newT = dht.readTemperature();
    float newH = dht.readHumidity();
    if (!isnan(newT)) temperature = newT;
    if (!isnan(newH)) humidity = newH;
    
    int rawAnalog = analogRead(ANALOG_PIN);
    analogPercent = (rawAnalog / ADC_RESOLUTION) * 100.0;
    
    // Auto Mode Hysteresis Logic based on dynamic tempThreshold
    if (autoMode && !isnan(temperature)) {
      if (temperature > tempThreshold) {
        fanState = true;
      } else if (temperature < (tempThreshold - 0.5)) {
        fanState = false;
      }
      digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
    }
    
    updateOledDisplay();
    publishSensorState();
  }
  
  delay(10);
}
