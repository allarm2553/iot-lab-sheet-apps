/**
 * Lab 4.1: HTTP GET / POST Web Server & REST API (Arduino IDE Sketch)
 * Features:
 *  - REST API on Port 80 (GET /api/data, POST /api/relay).
 *  - SSD1306 OLED Display (I2C 0x3C).
 *  - DHT11 temperature/humidity, Fan Relay, and GPIO 0 debounced button.
 *  - CORS support for cross-domain web app calls.
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  ESP8266WebServer server(80);
  #define DHTPIN 2
  #define FAN_RELAY_PIN 13
  #define BUTTON_PIN 0
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  WebServer server(80);
  #define DHTPIN 33
  #define FAN_RELAY_PIN 5
  #define BUTTON_PIN 0
#endif
#include <ArduinoJson.h>
#include <DHT.h>
#include <LittleFS.h>

const char* ssid = "iot_512";
const char* password = "iot123456";

Adafruit_SSD1306 display(128, 64, &Wire, -1);
DHT dht(DHTPIN, DHT11);

float temperature = 0.0;
float humidity = 0.0;
bool relayState = false;
int toggleCount = 0;
bool oledAvailable = false;

void updateOledDisplay(const char* statusMsg = "") {
  if (!oledAvailable) return;
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(8, 0);
  display.print("HTTP REST SERVER");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  display.setCursor(0, 14);
  if (isnan(temperature)) display.print("Temp: -- C");
  else display.printf("Temp: %.1f C", temperature);
  
  display.setCursor(0, 24);
  if (isnan(humidity)) display.print("Humid: -- %%");
  else display.printf("Humid: %.1f %%", humidity);
  
  display.setCursor(0, 35);
  if (statusMsg && strlen(statusMsg) > 0) display.print(statusMsg);
  else { display.print("IP: "); display.print(WiFi.localIP()); }
  
  display.drawLine(0, 48, 128, 48, SSD1306_WHITE);
  display.setCursor(0, 52);
  display.printf("Fan:%s | Cnt:%d", relayState ? "ON" : "OFF", toggleCount);
  display.display();
}

void handleGetData() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  JsonDocument doc;
  doc["temp"] = isnan(temperature) ? 0.0 : (round(temperature * 10.0) / 10.0);
  doc["humidity"] = isnan(humidity) ? 0.0 : (round(humidity * 10.0) / 10.0);
  doc["relay"] = relayState;
  doc["press"] = toggleCount;
  doc["ip"] = WiFi.localIP().toString();
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  server.send(200, "application/json", jsonStr);
}

void handleSetRelay() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\": \"Missing Request Body\"}");
    return;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  if (error) {
    server.send(400, "application/json", "{\"error\": \"Invalid JSON\"}");
    return;
  }

  if (doc["relay"].is<JsonVariant>()) {
    relayState = doc["relay"];
  } else if (doc["action"].is<JsonVariant>() && doc["action"] == "toggle") {
    relayState = !relayState;
  }

  digitalWrite(FAN_RELAY_PIN, relayState ? HIGH : LOW);
  updateOledDisplay("Relay Updated!");

  JsonDocument resDoc;
  resDoc["success"] = true;
  resDoc["relay"] = relayState;
  resDoc["message"] = relayState ? "Fan Relay Turned ON" : "Fan Relay Turned OFF";

  String resStr;
  serializeJson(resDoc, resStr);
  server.send(200, "application/json", resStr);
}

void handleOptions() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(204);
}

void handleFileRequest() {
  String path = server.uri();
  if (path.endsWith("/")) path += "index.html";

  String contentType = "text/plain";
  if (path.endsWith(".html")) contentType = "text/html";
  else if (path.endsWith(".css")) contentType = "text/css";
  else if (path.endsWith(".js")) contentType = "application/javascript";
  else if (path.endsWith(".json")) contentType = "application/json";

  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return;
  }

  if (server.uri() == "/") {
    String html = "<!DOCTYPE html><html lang='th'><head><meta charset='UTF-8'>"
                  "<title>ESP32 HTTP REST Dashboard</title></head><body>"
                  "<h2>ESP REST API Server is Online</h2></body></html>";
    server.send(200, "text/html; charset=utf-8", html);
    return;
  }
  server.send(404, "text/plain", "404 Not Found");
}

void setup() {
  Serial.begin(115200);
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(FAN_RELAY_PIN, LOW);
  
  dht.begin();
  
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    oledAvailable = true;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 20);
    display.println("Connecting WiFi...");
    display.display();
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected: " + WiFi.localIP().toString());

  #if defined(ESP8266)
  LittleFS.begin();
  #elif defined(ESP32)
  LittleFS.begin(true);
  #endif

  server.on("/api/data", HTTP_GET, handleGetData);
  server.on("/api/relay", HTTP_POST, handleSetRelay);
  server.on("/api/data", HTTP_OPTIONS, handleOptions);
  server.on("/api/relay", HTTP_OPTIONS, handleOptions);
  server.onNotFound(handleFileRequest);
  server.begin();

  updateOledDisplay("Server Ready");
}

void loop() {
  server.handleClient();

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
        relayState = !relayState;
        toggleCount++;
        digitalWrite(FAN_RELAY_PIN, relayState ? HIGH : LOW);
        updateOledDisplay("Button Toggled!");
      }
    }
  }
  lastButtonState = reading;

  static unsigned long lastSensorRead = 0;
  if (millis() - lastSensorRead > 2000) {
    lastSensorRead = millis();
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    updateOledDisplay();
  }
  delay(1);
}
