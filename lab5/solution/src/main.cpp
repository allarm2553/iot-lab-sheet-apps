/**
 * Lab 5: Cloud MQTT (Complete Code Solution with OLED Display)
 * Features:
 *  - Interfacing SSD1306 OLED via I2C to show states.
 *  - Connecting to Wi-Fi and displaying local IP.
 *  - Connecting to public EMQX MQTT Broker (broker.emqx.io, port 1883).
 *  - Subscribing to "esp-node/control/cmd" to receive remote controls (action/value in JSON).
 *  - Toggles GPIO 13 (Fan) upon receiving {"action":"toggle_fan", "value":true/false}.
 *  - Publishes full sensor data in JSON format to "esp-node/state" every 5 seconds.
 *  - Includes Temperature, Humidity, Soil Moisture (Analog Sensor %), Fan Relay status, and Button press count.
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

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#if defined(ESP8266)
#define DHTPIN 0            // D3/GPIO 0 สำหรับ AX-WiFi
#define ANALOG_PIN A0       // A0 (ตัวต้านทานปรับค่าได้ VR สำหรับ AX-WiFi)
#define FAN_RELAY_PIN 13    // D7/GPIO 13 สำหรับ AX-WiFi
#define BUTTON_PIN 0        // D3/GPIO 0 (ปุ่ม FLASH บนบอร์ด AX-WiFi)
#define ADC_RESOLUTION 1023.0
#elif defined(ESP32)
#define DHTPIN 33           // พอร์ต 33 สำหรับ IPST-WiFi
#define ANALOG_PIN 36       // GPIO 36 / KNOB-S สำหรับ IPST-WiFi
#define FAN_RELAY_PIN 5     // พอร์ต 5 สำหรับ IPST-WiFi
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

// Helper to draw OLED interface
void updateOledDisplay(const char* statusMsg = "") {
  display.clearDisplay();
  
  // 1. Header
  display.setCursor(15, 0);
  display.print("CLOUD MQTT NODE");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  // 2. Sensor Values
  display.setCursor(0, 14);
  if (isnan(temperature)) {
    display.print("Temp: -- C");
  } else {
    display.printf("Temp: %.1f C", temperature);
  }
  
  display.setCursor(0, 24);
  if (isnan(humidity)) {
    display.print("Humid: -- %%");
  } else {
    display.printf("Humid: %.1f %%", humidity);
  }
  
  display.setCursor(0, 34);
  display.printf("Analog: %.1f %%", analogPercent);
  
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
  display.printf("Fan:%s | Press:%d", fanState ? "ON" : "OFF", toggleCount);
  
  display.display();
}

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
        updateOledDisplay("Command Recv!");
      }
    }
  }
}

// Reconnect helper
void reconnectMqtt() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    display.clearDisplay();
    display.setCursor(10, 10);
    display.print("MQTT Connecting...");
    display.display();
    
    // Generate unique Client ID from Chip ID
    #if defined(ESP8266)
    String clientId = "ESP8266-" + String(ESP.getChipId(), HEX);
    #elif defined(ESP32)
    String clientId = "ESP32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    #endif
    
    if (mqttClient.connect(clientId.c_str(), mqttUser, mqttPassword)) {
      Serial.println("connected");
      display.clearDisplay();
      display.setCursor(10, 10);
      display.print("MQTT Connected!");
      display.display();
      delay(1000);
      
      // Subscribe to control topic
      mqttClient.subscribe(subTopic);
      Serial.printf("Subscribed to: %s\n", subTopic);
      updateOledDisplay("Broker Connected");
    } else {
      Serial.printf("failed, rc=%d. Trying again in 5 seconds...\n", mqttClient.state());
      display.clearDisplay();
      display.setCursor(10, 10);
      display.print("MQTT Failed!");
      display.setCursor(10, 25);
      display.printf("State: %d", mqttClient.state());
      display.display();
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

  // Initialize I2C and OLED Display
  #if defined(ESP8266)
  Wire.begin(4, 5); // SDA = GPIO 4 (D2), SCL = GPIO 5 (D1) สำหรับ AX-WiFi
  #else
  Wire.begin(21, 22); // SDA = GPIO 21, SCL = GPIO 22 สำหรับ IPST-WiFi
  #endif
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 20);
    display.print("System Booting...");
    display.display();
    delay(1500);
  }
  
  // Wi-Fi Connection
  Serial.printf("Connecting to %s...", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    display.clearDisplay();
    display.setCursor(10, 10);
    display.print("WiFi Connecting...");
    display.display();
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected.");
  display.clearDisplay();
  display.setCursor(10, 10);
  display.print("WiFi Connected!");
  display.setCursor(10, 25);
  display.print(WiFi.localIP().toString());
  display.display();
  delay(1500);
  
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
        
        // Update OLED immediately
        updateOledDisplay("Button Pressed!");
        
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
      // Update OLED screen
      updateOledDisplay();

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
