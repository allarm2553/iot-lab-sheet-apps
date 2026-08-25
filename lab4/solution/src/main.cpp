/**
 * Lab 4: Local WebSockets (Complete Code Solution)
 * Features:
 *  - Serves static files via WebServer (Port 80) from LittleFS.
 *  - Handles real-time WebSockets communication (Port 81) using ArduinoJson.
 *  - Receives button events from client (toggle_fan -> GPIO 13, toggle_mist -> GPIO 14).
 *  - Challenge: Receives custom temperature threshold via Slider (e.g. {"threshold": 32.5}).
 *  - Uses the dynamic threshold to execute Hysteresis logic for Fan Control:
 *    - Temp > Threshold: Fan ON
 *    - Temp < Threshold - 0.5: Fan OFF
 *  - Broadcasts temperature, humidity, soil moisture, and current states to all clients every 2 seconds.
 */

#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#endif
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <DHT.h>

const char* ssid = "iot_512";
const char* password = "iot123456";

#if defined(ESP8266)
ESP8266WebServer server(80);
#else
WebServer server(80);
#endif
WebSocketsServer webSocket = WebSocketsServer(81);

#if defined(ESP8266)
#define DHTPIN 2            // D3/GPIO 0 สำหรับ AX-WiFi
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

// Dynamic control parameters
float tempThreshold = 30.0; // Default temperature threshold (changeable via slider)
float humThreshold = 50.0;  // Default humidity threshold for mist pump (changeable via slider)
bool fanState = false;
bool mistState = false;
int toggleCount = 0;        // Physical button press counter (from Lab 2)
bool autoMode = true;       // Fan Mode flag: true = Auto (temp control), false = Manual
bool autoMistMode = true;   // Mist Mode flag: true = Auto (humidity control), false = Manual

// Broadcaster helper
void broadcastSensorData(float temp, float hum, float soil, bool fan, bool mist, float threshold, float hum_threshold, int press, bool mode, bool mist_mode) {
  JsonDocument doc;
  doc["temp"] = isnan(temp) ? 0.0 : (round(temp * 10.0) / 10.0);
  doc["humidity"] = isnan(hum) ? 0.0 : (round(hum * 10.0) / 10.0);
  doc["soil"] = round(soil * 10.0) / 10.0;
  doc["fan"] = fan;
  doc["mist"] = mist;
  doc["threshold"] = round(threshold * 10.0) / 10.0;
  doc["hum_threshold"] = round(hum_threshold * 10.0) / 10.0;
  doc["press"] = press;
  doc["mode"] = mode;
  doc["mist_mode"] = mist_mode;
  
  String output;
  serializeJson(doc, output);
  
  // ส่งข้อมูลกระจายไปยังหน้าเว็บทุกหน้าจอที่มีการเชื่อมต่อท่อ Socket อยู่ (broadcastTXT)
  webSocket.broadcastTXT(output);
}

void broadcastState() {
  broadcastSensorData(temperature, humidity, analogPercent, fanState, mistState, tempThreshold, humThreshold, toggleCount, autoMode, autoMistMode);
}

// WebSocket Event Handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_CONNECTED) {
    IPAddress ip = webSocket.remoteIP(num);
    Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
    broadcastState();
  } else if (type == WStype_TEXT) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    
    // 1. ตรวจสอบและรับสถานะคำสั่งควบคุมรีเลย์จากปุ่มบนหน้าเว็บ
    if (doc["action"].is<JsonVariant>()) {
       String action = doc["action"];
       if(action == "toggle_fan") {
          fanState = doc["value"];
          autoMode = false; // Switch to Manual mode upon user manual override
          digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
          Serial.printf("WS: Fan toggled manually to %s (Manual Mode)\n", fanState ? "ON" : "OFF");
          broadcastState();
       } else if (action == "toggle_mist") {
          mistState = doc["value"];
          autoMistMode = false; // Switch to Manual mode upon user manual override
          digitalWrite(MIST_RELAY_PIN, mistState ? HIGH : LOW);
          Serial.printf("WS: Mist toggled manually to %s (Manual Mode)\n", mistState ? "ON" : "OFF");
          broadcastState();
       } else if (action == "toggle_mode") {
          autoMode = doc["value"];
          Serial.printf("WS: Fan Mode toggled manually to %s\n", autoMode ? "Auto" : "Manual");
          if (autoMode && !isnan(temperature) && !isnan(humidity)) {
            if (temperature > tempThreshold) {
              fanState = true;
            } else if (temperature < (tempThreshold - 0.5)) {
              fanState = false;
            }
            digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
          }
          broadcastState();
       } else if (action == "toggle_mist_mode") {
          autoMistMode = doc["value"];
          Serial.printf("WS: Mist Mode toggled manually to %s\n", autoMistMode ? "Auto" : "Manual");
          if (autoMistMode && !isnan(humidity)) {
            if (humidity < humThreshold || analogPercent < 40.0) {
              mistState = true;
            } else if (humidity > (humThreshold + 5.0) && analogPercent > 50.0) {
              mistState = false;
            }
            digitalWrite(MIST_RELAY_PIN, mistState ? HIGH : LOW);
          }
          broadcastState();
       } else if (action == "set_threshold") {
          tempThreshold = doc["value"];
          autoMode = true; // Auto mode active
          Serial.printf("WS: Temperature Threshold updated to %.1f C (Auto Mode)\n", tempThreshold);
          if (!isnan(temperature) && !isnan(humidity)) {
            if (temperature > tempThreshold) {
              fanState = true;
            } else if (temperature < (tempThreshold - 0.5)) {
              fanState = false;
            }
            digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
          }
          broadcastState();
       } else if (action == "set_hum_threshold") {
          humThreshold = doc["value"];
          autoMistMode = true; // Auto mode active
          Serial.printf("WS: Humidity Threshold updated to %.1f %% (Auto Mode)\n", humThreshold);
          if (!isnan(humidity)) {
            if (humidity < humThreshold || analogPercent < 40.0) {
              mistState = true;
            } else if (humidity > (humThreshold + 5.0) && analogPercent > 50.0) {
              mistState = false;
            }
            digitalWrite(MIST_RELAY_PIN, mistState ? HIGH : LOW);
          }
          broadcastState();
       }
    }
    
    // 2. รับค่า threshold จากสไลเดอร์โดยตรง
    if (doc["threshold"].is<JsonVariant>()) {
      tempThreshold = doc["threshold"];
      autoMode = true;
      Serial.printf("WS: Temperature Threshold updated to %.1f C (Auto Mode)\n", tempThreshold);
      if (!isnan(temperature) && !isnan(humidity)) {
        if (temperature > tempThreshold) {
          fanState = true;
        } else if (temperature < (tempThreshold - 0.5)) {
          fanState = false;
        }
        digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
      }
      broadcastState();
    }

    // 2.1 รับค่า hum_threshold จากสไลเดอร์โดยตรง
    if (doc["hum_threshold"].is<JsonVariant>()) {
      humThreshold = doc["hum_threshold"];
      autoMistMode = true;
      Serial.printf("WS: Humidity Threshold updated to %.1f %% (Auto Mode)\n", humThreshold);
      if (!isnan(humidity)) {
        if (humidity < humThreshold || analogPercent < 40.0) {
          mistState = true;
        } else if (humidity > (humThreshold + 5.0) && analogPercent > 50.0) {
          mistState = false;
        }
        digitalWrite(MIST_RELAY_PIN, mistState ? HIGH : LOW);
      }
      broadcastState();
    }
  }
}

// Static web server handler
void handleFileRequest() {
  String path = server.uri();
  if (path.endsWith("/")) path += "index.html";
  
  String dataType = "text/plain";
  if (path.endsWith(".html")) dataType = "text/html";
  else if (path.endsWith(".css")) dataType = "text/css";
  else if (path.endsWith(".js")) dataType = "application/javascript";
  
  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    server.streamFile(file, dataType);
    file.close();
  } else {
    server.send(404, "text/plain", "File Not Found");
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
  
  // Wi-Fi Connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected.");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  
  // LittleFS
  #if defined(ESP8266)
  if(!LittleFS.begin()){
  #else
  if(!LittleFS.begin(true)){
  #endif
     Serial.println("LittleFS Mount Failed");
     return;
  }
  
  // Server Setup
  server.onNotFound(handleFileRequest);
  server.begin();
  
  // WebSocket Server Setup
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
  Serial.println("System initialized. Server running on port 80, WebSockets on port 81.");
}

void loop() {
  server.handleClient();
  webSocket.loop();
  
  // Physical Button Debounce & Toggle Control (consistent with Lab 2)
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
        Serial.printf("Button Pressed. Fan toggled to: %d, Count: %d (Manual Mode)\n", fanState, toggleCount);
        // Broadcast immediately
        broadcastState();
      }
    }
  }
  lastButtonState = reading;
  
  // Periodic sensor readings and decision logic
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 2000) {
    lastUpdate = millis();
    
    // Read sensor values
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    int rawAnalog = analogRead(ANALOG_PIN);
    analogPercent = (rawAnalog / ADC_RESOLUTION) * 100.0;
    
    // Check if DHT is functioning and fan is in Auto Mode
    if (autoMode && !isnan(temperature) && !isnan(humidity)) {
      // Hysteresis Control Logic based on dynamic tempThreshold
      if (temperature > tempThreshold) {
        fanState = true;
      } else if (temperature < (tempThreshold - 0.5)) {
        fanState = false;
      }
      digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
    }

    // Auto Mode Mist Control Logic based on dynamic humThreshold & soil moisture
    if (autoMistMode && !isnan(humidity)) {
      if (humidity < humThreshold || analogPercent < 40.0) {
        mistState = true;
      } else if (humidity > (humThreshold + 5.0) && analogPercent > 50.0) {
        mistState = false;
      }
      digitalWrite(MIST_RELAY_PIN, mistState ? HIGH : LOW);
    }
    
    // Broadcast data to all connected WebSocket clients
    broadcastState();
  }
  
  delay(1);
}
