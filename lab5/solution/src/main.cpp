/**
 * Lab 5: Cloud MQTT (Complete Code Solution)
 * Features:
 *  - Serves dynamic MQTT communication (Port 1883) using ArduinoJson.
 *  - Subscribes to "esp-node/control/cmd" to receive remote controls (action/value in JSON).
 *  - Toggles GPIO 13 (Fan) upon receiving {"action":"toggle_fan", "value":true/false}.
 *  - Publishes full sensor data in JSON format to "esp-node/state" every 5 seconds.
 *  - Includes Temperature, Humidity, Soil Moisture (Analog Sensor %), Fan Relay status, and Button press count.
 */

#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

const char* ssid = "iot_512";
const char* password = "iot123456";

// MQTT settings
const char* mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
const char* subTopic = "esp-node/control/cmd";
const char* pubTopic = "esp-node/state";
const char* mqttUser = "elec";
const char* mqttPassword = "elec1234";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

#if defined(ESP8266)
#define DHTPIN 0            // D3/GPIO 0 สำหรับ AX-WiFi
#define ANALOG_PIN A0       // A0 (ตัวต้านทานปรับค่าได้ VR สำหรับ AX-WiFi)
#define FAN_RELAY_PIN 13    // D7/GPIO 13 สำหรับ AX-WiFi
#define MIST_RELAY_PIN 16   // D0/GPIO 16 สำหรับ AX-WiFi
#define BUTTON_PIN 0        // D3/GPIO 0 (ปุ่ม FLASH บนบอร์ด AX-WiFi)
#define ADC_RESOLUTION 1023.0
#elif defined(ESP32)
#define DHTPIN 33           // พอร์ต 33 สำหรับ IPST-WiFi
#define ANALOG_PIN 36       // GPIO 36 / KNOB-S สำหรับ IPST-WiFi
#define FAN_RELAY_PIN 5     // พอร์ต 5 สำหรับ IPST-WiFi
#define MIST_RELAY_PIN 23    // พอร์ต 23 สำหรับ IPST-WiFi
#define BUTTON_PIN 0        // GPIO 0 (ปุ่ม SW1 บนบอร์ด IPST-WiFi)
#define ADC_RESOLUTION 4095.0
#endif
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// Environmental data
float temperature = 0;
float humidity = 0;
float analogPercent = 0;

// Device states
bool fanState = false;
int toggleCount = 0;

// MQTT Callback Function
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: [");
  Serial.print(topic);
  Serial.print("] ");
  
  // Parse incoming JSON payload
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  
  // Handle control actions
  if (String(topic) == subTopic) {
    if (doc.containsKey("action")) {
      String action = doc["action"];
      if (action == "toggle_fan") {
        fanState = doc["value"];
        digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
        Serial.printf("MQTT Control: Fan toggled to %s\n", fanState ? "ON" : "OFF");
      }
    }
  }
}

// Reconnect helper
void reconnectMqtt() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    
    // Generate unique Client ID from Chip ID
    #if defined(ESP8266)
    String clientId = "ESP8266-" + String(ESP.getChipId(), HEX);
    #elif defined(ESP32)
    String clientId = "ESP32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    #endif
    
    if (mqttClient.connect(clientId.c_str(), mqttUser, mqttPassword)) {
      Serial.println("connected");
      
      // Subscribe to control topic
      mqttClient.subscribe(subTopic);
      Serial.printf("Subscribed to: %s\n", subTopic);
    } else {
      Serial.printf("failed, rc=%d. Trying again in 5 seconds...\n", mqttClient.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // Set Pin Modes
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(FAN_RELAY_PIN, LOW);
  
  #if defined(ESP32)
  analogSetPinAttenuation(ANALOG_PIN, ADC_11db);
  #endif
  
  // Wi-Fi Connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected.");
  
  // Setup MQTT client
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(mqttCallback);
}

void loop() {
  if (!mqttClient.connected()) {
    reconnectMqtt();
  }
  mqttClient.loop();
  
  // Physical Button Press handling
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
        toggleCount++;
        digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
        Serial.printf("Button Pressed. Fan toggled to: %d, Count: %d\n", fanState, toggleCount);
        
        // Immediately publish updated state
        JsonDocument doc;
        doc["temp"] = temperature;
        doc["humidity"] = humidity;
        doc["soil"] = analogPercent;
        doc["fan"] = fanState;
        doc["press"] = toggleCount;
        
        String output;
        serializeJson(doc, output);
        mqttClient.publish(pubTopic, output.c_str());
      }
    }
  }
  lastButtonState = reading;
  
  // Periodic sensor publishing
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 5000) {
    lastUpdate = millis();
    
    // Read sensor values
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    int rawAnalog = analogRead(ANALOG_PIN);
    analogPercent = (rawAnalog / ADC_RESOLUTION) * 100.0;
    
    if (!isnan(temperature) && !isnan(humidity)) {
      // Pack into JSON payload
      JsonDocument doc;
      doc["temp"] = temperature;
      doc["humidity"] = humidity;
      doc["soil"] = analogPercent;
      doc["fan"] = fanState;
      doc["press"] = toggleCount;
      
      String output;
      serializeJson(doc, output);
      mqttClient.publish(pubTopic, output.c_str());
      Serial.printf("MQTT Published: %s -> Topic: %s\n", output.c_str(), pubTopic);
    } else {
      Serial.println("DHT11 read failed, skipping MQTT publish");
    }
  }
  
  delay(1);
}
