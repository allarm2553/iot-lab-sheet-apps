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
#define DHTPIN 0            // D3/GPIO 0 สำหรับ AX-WiFi
#define ANALOG_PIN A0       // A0 (ตัวต้านทานปรับค่าได้ VR สำหรับ AX-WiFi)
#define FAN_RELAY_PIN 13    // D7/GPIO 13 สำหรับ AX-WiFi
#define MIST_RELAY_PIN 16   // D0/GPIO 16 สำหรับ AX-WiFi
#define ADC_RESOLUTION 1023.0
#elif defined(ESP32)
#define DHTPIN 33           // พอร์ต 33 สำหรับ IPST-WiFi
#define ANALOG_PIN 36       // GPIO 36 / KNOB-S สำหรับ IPST-WiFi
#define FAN_RELAY_PIN 5     // พอร์ต 5 สำหรับ IPST-WiFi
#define MIST_RELAY_PIN 23    // พอร์ต 23 สำหรับ IPST-WiFi
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
bool fanState = false;
bool mistState = false;

// Broadcaster helper
void broadcastSensorData(float temp, float hum, float soil, bool fan, float threshold) {
  JsonDocument doc;
  doc["temp"] = temp;
  doc["humidity"] = hum;
  doc["soil"] = soil;
  doc["fan"] = fan;
  doc["threshold"] = threshold;
  
  String output;
  serializeJson(doc, output);
  
  // 2. ส่งข้อมูลกระจายไปยังหน้าเว็บทุกหน้าจอที่มีการเชื่อมต่อท่อ Socket อยู่ (broadcastTXT)
  webSocket.broadcastTXT(output);
}

// WebSocket Event Handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    
    // 1. ตรวจสอบและรับสถานะคำสั่งควบคุมรีเลย์จากปุ่มบนหน้าเว็บ
    if(doc.containsKey("action")) {
       String action = doc["action"];
       if(action == "toggle_fan") {
          fanState = doc["value"];
          digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
          Serial.printf("WS: Fan toggled manually to %s\n", fanState ? "ON" : "OFF");
       } else if (action == "toggle_mist") {
          mistState = doc["value"];
          digitalWrite(MIST_RELAY_PIN, mistState ? HIGH : LOW);
          Serial.printf("WS: Mist toggled manually to %s\n", mistState ? "ON" : "OFF");
       }
    }
    
    // 2. โจทย์ท้าทาย: ตรวจสอบและอัปเดตเกณฑ์อุณหภูมิ (Temperature Threshold) จากสไลเดอร์
    if (doc.containsKey("threshold")) {
      tempThreshold = doc["threshold"];
      Serial.printf("WS: Temperature Threshold updated to %.1f C\n", tempThreshold);
      // สะท้อนค่ากลับเพื่อแจ้งเครื่องอื่นๆ
      broadcastSensorData(temperature, humidity, analogPercent, fanState, tempThreshold);
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
  
  // Periodic sensor readings and decision logic
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 2000) {
    lastUpdate = millis();
    
    // Read sensor values
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    int rawAnalog = analogRead(ANALOG_PIN);
    analogPercent = (rawAnalog / ADC_RESOLUTION) * 100.0;
    
    // Check if DHT is functioning
    if (!isnan(temperature) && !isnan(humidity)) {
      // Hysteresis Control Logic based on dynamic tempThreshold
      if (temperature > tempThreshold) {
        fanState = true;
      } else if (temperature < (tempThreshold - 0.5)) {
        fanState = false;
      }
      digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
    }
    
    // Broadcast data to all connected WebSocket clients
    broadcastSensorData(temperature, humidity, analogPercent, fanState, tempThreshold);
  }
  
  delay(1);
}
