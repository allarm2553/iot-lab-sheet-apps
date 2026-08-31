/**
 * Lab 3.2: Web Wi-Fi Configurator & GPIO 0 Reset Button
 * Standalone Arduino IDE Sketch (.ino)
 * 
 * Instructions:
 * 1. Install ArduinoJson library (version 7.x) from Library Manager.
 * 2. Select Board: "ESP32 Dev Module" or "NodeMCU 1.0 (ESP-12E Module)".
 * 3. Upload sketch.
 * 4. To upload data folder to LittleFS, use "ESP32 LittleFS Data Upload" plugin.
 */

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <DNSServer.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  ESP8266WebServer server(80);
  #define BUTTON_PIN 0
  #define LED_PIN    2
  #define LED_ON     LOW
  #define LED_OFF    HIGH
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  WebServer server(80);
  #define BUTTON_PIN 0
  #define LED_PIN    2
  #define LED_ON     HIGH
  #define LED_OFF    LOW
#endif

const char* apSSID = "ESP_WiFi_Config";
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;

bool isAPMode = false;
String wifiSSID = "";
String wifiPass = "";

void blinkStatusLED(int times, int delayMs = 120) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, LED_ON);
    delay(delayMs);
    digitalWrite(LED_PIN, LED_OFF);
    if (i < times - 1) delay(delayMs);
  }
}

bool loadWifiConfig() {
  if (!LittleFS.exists("/config.json")) return false;
  File file = LittleFS.open("/config.json", "r");
  if (!file) return false;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, file);
  file.close();

  if (err) return false;
  wifiSSID = doc["ssid"].as<String>();
  wifiPass = doc["pass"].as<String>();
  return (wifiSSID.length() > 0);
}

bool saveWifiConfig(String ssid, String pass) {
  File file = LittleFS.open("/config.json", "w");
  if (!file) return false;
  JsonDocument doc;
  doc["ssid"] = ssid;
  doc["pass"] = pass;
  serializeJson(doc, file);
  file.close();
  return true;
}

void factoryResetWifi() {
  Serial.println("\n[FACTORY RESET] GPIO 0 Long-Press detected! Clearing Wi-Fi config...");
  if (LittleFS.exists("/config.json")) {
    LittleFS.remove("/config.json");
  }
  blinkStatusLED(5, 80);
  delay(500);
  ESP.restart();
}

const char FALLBACK_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="th">
<head>
  <meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP Wi-Fi Setup</title>
  <style>
    body{background:#0f172a;color:#f8fafc;font-family:sans-serif;display:flex;align-items:center;justify-content:center;min-height:100vh;margin:0;padding:1rem;}
    .card{background:rgba(30,41,59,0.85);border:1px solid rgba(255,255,255,0.1);border-radius:16px;padding:2rem;max-width:400px;width:100%;text-align:center;}
    h2{margin-top:0;color:#818cf8;}input{width:100%;padding:0.75rem;margin:0.5rem 0 1rem;background:#1e293b;border:1px solid #475569;border-radius:8px;color:#fff;box-sizing:border-box;}
    button{width:100%;padding:0.85rem;background:#6366f1;color:#fff;border:none;border-radius:8px;font-size:1rem;font-weight:600;cursor:pointer;}
  </style>
</head>
<body>
  <div class="card">
    <h2>⚡ Wi-Fi Config Portal</h2>
    <p style="font-size:0.85rem;color:#94a3b8;">กรุณากรอกชื่อและรหัสผ่าน Wi-Fi เพื่อเชื่อมต่อ</p>
    <form action="/api/save" method="POST">
      <input type="text" name="ssid" placeholder="ชื่อ Wi-Fi (SSID)" required>
      <input type="password" name="pass" placeholder="รหัสผ่าน (Password)">
      <button type="submit">บันทึกและเชื่อมต่อ</button>
    </form>
  </div>
</body>
</html>
)rawliteral";

void handleRoot() {
  if (LittleFS.exists("/index.html")) {
    File f = LittleFS.open("/index.html", "r");
    server.streamFile(f, "text/html");
    f.close();
  } else {
    server.send_P(200, "text/html", FALLBACK_HTML);
  }
}

void handleScan() {
  int n = WiFi.scanNetworks();
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();
  for (int i = 0; i < n; i++) {
    JsonObject obj = arr.add<JsonObject>();
    obj["ssid"] = WiFi.SSID(i);
    obj["rssi"] = WiFi.RSSI(i);
    #if defined(ESP8266)
    obj["enc"] = (WiFi.encryptionType(i) != ENC_TYPE_NONE);
    #elif defined(ESP32)
    obj["enc"] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
    #endif
  }
  String res;
  serializeJson(doc, res);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", res);
}

void handleSave() {
  String reqSSID = "";
  String reqPass = "";
  if (server.hasArg("plain")) {
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    reqSSID = doc["ssid"].as<String>();
    reqPass = doc["pass"].as<String>();
  } else if (server.hasArg("ssid")) {
    reqSSID = server.arg("ssid");
    reqPass = server.arg("pass");
  }

  if (reqSSID.length() > 0) {
    saveWifiConfig(reqSSID, reqPass);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", "{\"status\":\"success\"}");
    delay(1000);
    ESP.restart();
  } else {
    server.send(400, "application/json", "{\"status\":\"error\"}");
  }
}

void startConfigPortal() {
  isAPMode = true;
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID);
  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/scan", HTTP_GET, handleScan);
  server.on("/api/save", HTTP_POST, handleSave);
  server.onNotFound([]() {
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
  });

  server.begin();
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF);

  #if defined(ESP8266)
  LittleFS.begin();
  #elif defined(ESP32)
  LittleFS.begin(true);
  #endif

  if (loadWifiConfig()) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID.c_str(), wifiPass.c_str());
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 25) {
      delay(500);
      retries++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      blinkStatusLED(3, 150);
      return;
    }
  }

  startConfigPortal();
}

void loop() {
  static unsigned long pressStartTime = 0;
  static bool isPressed = false;

  int btnState = digitalRead(BUTTON_PIN);
  if (btnState == LOW) {
    if (!isPressed) {
      pressStartTime = millis();
      isPressed = true;
    } else {
      if (millis() - pressStartTime >= 3000) {
        factoryResetWifi();
      }
    }
  } else {
    isPressed = false;
  }

  if (isAPMode) {
    dnsServer.processNextRequest();
    server.handleClient();
  }
  delay(2);
}
