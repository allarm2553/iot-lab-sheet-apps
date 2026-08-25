/**
 * Lab 3.2: HTTP GET / POST Web Server & REST API (Complete Solution)
 * Features:
 *  - Serves REST API via WebServer (Port 80)
 *  - Handles HTTP GET "/api/data" -> returns Temperature, Humidity, Relay State, and Press Count in JSON
 *  - Handles HTTP POST "/api/relay" -> receives JSON payload {"relay": true/false} to control Relay 1 (Fan)
 *  - Interfaces DHT11 Sensor, Relay 1 (GPIO 5 / GPIO 13), Push Button (GPIO 0), and SSD1306 OLED (I2C)
 *  - Fully compatible with ESP32 (IPST-WiFi) and ESP8266 (AX-WiFi)
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#endif
#include <ArduinoJson.h>
#include <DHT.h>

// Wi-Fi Credentials
const char* ssid = "iot_512";
const char* password = "iot123456";

// Web Server on Port 80
#if defined(ESP8266)
ESP8266WebServer server(80);
#else
WebServer server(80);
#endif

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool oledAvailable = false;

// Hardware Pin Definitions
#if defined(ESP8266)
#define DHTPIN 2            // D4/GPIO 2 สำหรับ AX-WiFi (หรือ GPIO 0 / D3)
#define FAN_RELAY_PIN 13    // D7/GPIO 13 สำหรับ AX-WiFi
#define BUTTON_PIN 0        // D3/GPIO 0 (ปุ่ม FLASH บนบอร์ด AX-WiFi)
#elif defined(ESP32)
#define DHTPIN 33           // พอร์ต 33 สำหรับ IPST-WiFi
#define FAN_RELAY_PIN 5     // พอร์ต 5 สำหรับ IPST-WiFi
#define BUTTON_PIN 0        // GPIO 0 (ปุ่ม SW1 บนบอร์ด IPST-WiFi)
#endif
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// Environmental data & States
float temperature = 0.0;
float humidity = 0.0;
bool relayState = false;
int toggleCount = 0;

// Helper to update OLED Display
void updateOledDisplay(const char* statusMsg = "") {
  if (!oledAvailable) return;
  display.clearDisplay();
  
  // 1. Header
  display.setCursor(8, 0);
  display.print("HTTP REST SERVER");
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
  
  // 3. IP Address or Status
  display.setCursor(0, 35);
  if (statusMsg && strlen(statusMsg) > 0) {
    display.print(statusMsg);
  } else {
    display.print("IP: ");
    display.print(WiFi.localIP());
  }
  
  // 4. Relay & Button Press Counter
  display.drawLine(0, 48, 128, 48, SSD1306_WHITE);
  display.setCursor(0, 52);
  display.printf("Fan:%s | Cnt:%d", relayState ? "ON" : "OFF", toggleCount);
  
  display.display();
}

// -------------------------------------------------------------
// REST API Handlers
// -------------------------------------------------------------

// 1. HTTP GET /api/data : ส่งข้อมูลเซ็นเซอร์และสถานะรีเลย์กลับเป็น JSON
void handleGetData() {
  // CORS Headers เพื่อให้เว็บอื่นสามารถ fetch ข้ามโดเมนได้
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
  Serial.printf("[HTTP GET /api/data] Response: %s\n", jsonStr.c_str());
}

// 2. HTTP POST /api/relay : รับ JSON Payload เพื่อสั่งเปิด-ปิดรีเลย์
void handleSetRelay() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\": \"Missing Request Body\"}");
    return;
  }

  String payload = server.arg("plain");
  Serial.printf("[HTTP POST /api/relay] Body: %s\n", payload.c_str());

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload);
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

// 3. HTTP GET /api/toggle : สำหรับทดสอบสลับสถานะผ่านเบราว์เซอร์โดยตรง
void handleToggleRelay() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  relayState = !relayState;
  digitalWrite(FAN_RELAY_PIN, relayState ? HIGH : LOW);
  updateOledDisplay("Relay Toggled!");

  JsonDocument resDoc;
  resDoc["success"] = true;
  resDoc["relay"] = relayState;
  resDoc["message"] = relayState ? "Fan Relay Turned ON" : "Fan Relay Turned OFF";

  String resStr;
  serializeJson(resDoc, resStr);
  server.send(200, "application/json", resStr);
}

// CORS Pre-flight Options Handler
void handleOptions() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(204);
}

// Static File Request Handler (LittleFS)
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

  // Fallback Inline Web Dashboard if file doesn't exist on LittleFS
  if (server.uri() == "/") {
    String html = "<!DOCTYPE html><html lang='th'><head><meta charset='UTF-8'>"
                  "<meta name='viewport' content='width=device-width,initial-scale=1.0'>"
                  "<title>ESP32 HTTP REST Dashboard</title>"
                  "<style>body{font-family:sans-serif;background:#0f172a;color:#f8fafc;padding:20px;text-align:center;}"
                  ".card{background:rgba(30,41,59,0.8);padding:20px;border-radius:12px;max-width:400px;margin:20px auto;border:1px solid rgba(255,255,255,0.1);}"
                  ".val{font-size:2rem;font-weight:bold;color:#38bdf8;margin:10px 0;}"
                  "button{background:#06b6d4;color:#fff;border:none;padding:12px 24px;border-radius:8px;font-size:1.1rem;cursor:pointer;font-weight:bold;}"
                  "</style></head><body>"
                  "<h1>ESP32 HTTP REST API Dashboard</h1>"
                  "<div class='card'><h2>อุณหภูมิและความชื้น</h2>"
                  "<div class='val'><span id='temp'>--</span> °C | <span id='hum'>--</span> %</div>"
                  "<p>สถานะรีเลย์: <b id='relay-status'>--</b></p>"
                  "<button onclick='toggleRelay()'>สลับสถานะรีเลย์ (POST)</button></div>"
                  "<script>"
                  "function fetchData(){fetch('/api/data').then(r=>r.json()).then(d=>{"
                  "document.getElementById('temp').textContent=d.temp.toFixed(1);"
                  "document.getElementById('hum').textContent=d.humidity.toFixed(1);"
                  "document.getElementById('relay-status').textContent=d.relay?'ON':'OFF';"
                  "document.getElementById('relay-status').style.color=d.relay?'#10b981':'#ef4444';"
                  "});}"
                  "function toggleRelay(){"
                  "const cur = document.getElementById('relay-status').textContent==='ON';"
                  "fetch('/api/relay',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({relay:!cur})})"
                  ".then(r=>r.json()).then(()=>fetchData());}"
                  "setInterval(fetchData,2000);fetchData();"
                  "</script></body></html>";
    server.send(200, "text/html; charset=utf-8", html);
    return;
  }

  server.send(404, "text/plain", "404 Not Found");
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(FAN_RELAY_PIN, OUTPUT);
  digitalWrite(FAN_RELAY_PIN, LOW);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialize I2C OLED
  #if defined(ESP8266)
  Wire.begin(4, 5); // SDA: GPIO 4 (D2), SCL: GPIO 5 (D1)
  #elif defined(ESP32)
  Wire.begin(21, 22); // SDA: GPIO 21, SCL: GPIO 22
  #endif

  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    oledAvailable = true;
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    updateOledDisplay("Connecting WiFi...");
  }

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected!");
  Serial.print("Web Server IP: ");
  Serial.println(WiFi.localIP());

  // Mount LittleFS
  #if defined(ESP8266)
  LittleFS.begin();
  #else
  LittleFS.begin(true);
  #endif

  // Register REST API Endpoints
  server.on("/api/data", HTTP_GET, handleGetData);
  server.on("/api/relay", HTTP_POST, handleSetRelay);
  server.on("/api/relay", HTTP_OPTIONS, handleOptions);
  server.on("/api/toggle", HTTP_GET, handleToggleRelay);
  server.onNotFound(handleFileRequest);

  server.begin();
  Serial.println("HTTP Web Server Started on Port 80.");
  updateOledDisplay();
}

void loop() {
  server.handleClient();

  // 1. Physical Button Debounce
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
        relayState = !relayState;
        toggleCount++;
        digitalWrite(FAN_RELAY_PIN, relayState ? HIGH : LOW);
        Serial.printf("Physical Button Pressed! Relay: %s, Count: %d\n", relayState ? "ON" : "OFF", toggleCount);
        updateOledDisplay("Button Pressed!");
      }
    }
  }
  lastButtonState = reading;

  // 2. Periodic Sensor Readings (every 2 seconds)
  static unsigned long lastSensorRead = 0;
  if (millis() - lastSensorRead > 2000) {
    lastSensorRead = millis();
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t)) temperature = t;
    if (!isnan(h)) humidity = h;
    updateOledDisplay();
  }

  delay(1);
}
