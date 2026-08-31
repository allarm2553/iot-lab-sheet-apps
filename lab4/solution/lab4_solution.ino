/**
 * Lab 4: Local WebSockets Full-Duplex Server (Arduino IDE Sketch)
 * Features:
 *  - Serves HTML/CSS/JS dashboard from LittleFS on Port 80.
 *  - Handles real-time WebSockets communication on Port 81.
 *  - Real-time Fan/Mist relay control, DHT11 & Potentiometer broadcast, and Slider Hysteresis threshold.
 */

#include <Arduino.h>
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  ESP8266WebServer server(80);
  #define DHTPIN 2
  #define ANALOG_PIN A0
  #define FAN_RELAY_PIN 13
  #define MIST_RELAY_PIN 16
  #define BUTTON_PIN 0
  #define ADC_RESOLUTION 1023.0
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  WebServer server(80);
  #define DHTPIN 33
  #define ANALOG_PIN 36
  #define FAN_RELAY_PIN 5
  #define MIST_RELAY_PIN 23
  #define BUTTON_PIN 0
  #define ADC_RESOLUTION 4095.0
#endif

#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <DHT.h>

const char* ssid = "iot_512";
const char* password = "iot123456";

WebSocketsServer webSocket = WebSocketsServer(81);
DHT dht(DHTPIN, DHT11);

float temperature = 0;
float humidity = 0;
float analogPercent = 0;

float tempThreshold = 30.0;
float humThreshold = 50.0;
bool fanState = false;
bool mistState = false;
int toggleCount = 0;
bool autoMode = true;
bool autoMistMode = true;

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
  webSocket.broadcastTXT(output);
}

void broadcastState() {
  broadcastSensorData(temperature, humidity, analogPercent, fanState, mistState, tempThreshold, humThreshold, toggleCount, autoMode, autoMistMode);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_CONNECTED) {
    IPAddress ip = webSocket.remoteIP(num);
    Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
    broadcastState();
  } else if (type == WStype_TEXT) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error) return;
    
    if (doc["action"].is<JsonVariant>()) {
      String action = doc["action"];
      if (action == "toggle_fan") {
        fanState = doc["value"];
        autoMode = false;
        digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
        broadcastState();
      } else if (action == "toggle_mist") {
        mistState = doc["value"];
        autoMistMode = false;
        digitalWrite(MIST_RELAY_PIN, mistState ? HIGH : LOW);
        broadcastState();
      } else if (action == "toggle_mode") {
        autoMode = doc["value"];
        broadcastState();
      } else if (action == "set_threshold") {
        tempThreshold = doc["value"];
        autoMode = true;
        broadcastState();
      }
    }
    
    if (doc["threshold"].is<JsonVariant>()) {
      tempThreshold = doc["threshold"];
      autoMode = true;
      broadcastState();
    }
  }
}

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
  
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(MIST_RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(FAN_RELAY_PIN, LOW);
  digitalWrite(MIST_RELAY_PIN, LOW);
  
  #if defined(ESP32)
  analogSetPinAttenuation(ANALOG_PIN, ADC_11db);
  #endif
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected: " + WiFi.localIP().toString());
  
  #if defined(ESP8266)
  LittleFS.begin();
  #elif defined(ESP32)
  LittleFS.begin(true);
  #endif
  
  server.onNotFound(handleFileRequest);
  server.begin();
  
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSockets server running on Port 81.");
}

void loop() {
  server.handleClient();
  webSocket.loop();
  
  int reading = digitalRead(BUTTON_PIN);
  static bool lastButtonState = HIGH;
  static bool currentButtonState = HIGH;
  static unsigned long lastDebounceTime = 0;
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > 50) {
    if (reading != currentButtonState) {
      currentButtonState = reading;
      if (currentButtonState == LOW) {
        fanState = !fanState;
        autoMode = false;
        toggleCount++;
        digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
        broadcastState();
      }
    }
  }
  lastButtonState = reading;
  
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 2000) {
    lastUpdate = millis();
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    int rawAnalog = analogRead(ANALOG_PIN);
    analogPercent = (rawAnalog / ADC_RESOLUTION) * 100.0;
    
    if (autoMode && !isnan(temperature) && !isnan(humidity)) {
      if (temperature > tempThreshold) {
        fanState = true;
      } else if (temperature < (tempThreshold - 0.5)) {
        fanState = false;
      }
      digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
    }
    
    broadcastState();
  }
  delay(1);
}
