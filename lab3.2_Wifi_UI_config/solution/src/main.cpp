/**
 * Lab 3.2: Web Wi-Fi Configurator & GPIO 0 Reset Button (Complete Solution)
 * Features:
 *  - Persistent Wi-Fi credentials storage using LittleFS (/config.json).
 *  - Automatic fallback to Access Point (AP) mode (SSID: ESP_WiFi_Config, IP: 192.168.4.1) with DNS Captive Portal.
 *  - Embedded Web Server serving Glassmorphic Wi-Fi Setup Dashboard from LittleFS (or inline fallback).
 *  - REST API Endpoints:
 *      GET  /api/scan -> Scans nearby Wi-Fi networks and returns JSON array.
 *      POST /api/save -> Receives SSID/Password, saves to LittleFS, and restarts board.
 *  - Physical Button Reset (GPIO 0 Long-Press >= 3s):
 *      Erases /config.json, blinks LED rapidly (5 times), and enters Config Mode.
 */

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <DNSServer.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  ESP8266WebServer server(80);
  #define BUTTON_PIN 0    // D3/GPIO 0 (ปุ่ม FLASH บนบอร์ด AX-WiFi)
  #define LED_PIN    2    // D4/GPIO 2 (LED บนบอร์ด AX-WiFi / Active LOW)
  #define LED_ON     LOW
  #define LED_OFF    HIGH
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  WebServer server(80);
  #define BUTTON_PIN 0    // GPIO 0 (ปุ่ม SW1 บนบอร์ด IPST-WiFi)
  #define LED_PIN    2    // GPIO 2 (Built-in LED บนบอร์ด ESP32 / Active HIGH)
  #define LED_ON     HIGH
  #define LED_OFF    LOW
#endif

// Access Point & DNS Captive Portal Settings
const char* apSSID = "ESP_WiFi_Config";
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;

bool isAPMode = false;
String wifiSSID = "";
String wifiPass = "";

// ── LED Feedback Helper ──
void blinkStatusLED(int times, int delayMs = 120) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, LED_ON);
    delay(delayMs);
    digitalWrite(LED_PIN, LED_OFF);
    if (i < times - 1) delay(delayMs);
  }
}

// ── Load Credentials from LittleFS ──
bool loadWifiConfig() {
  if (!LittleFS.exists("/config.json")) {
    Serial.println("[LittleFS] No /config.json found.");
    return false;
  }
  File file = LittleFS.open("/config.json", "r");
  if (!file) {
    Serial.println("[LittleFS] Failed to open /config.json for reading.");
    return false;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, file);
  file.close();

  if (err) {
    Serial.println("[LittleFS] JSON parse error in config file.");
    return false;
  }

  wifiSSID = doc["ssid"].as<String>();
  wifiPass = doc["pass"].as<String>();
  Serial.printf("[LittleFS] Loaded SSID: %s\n", wifiSSID.c_str());
  return (wifiSSID.length() > 0);
}

// ── Save Credentials to LittleFS ──
bool saveWifiConfig(String ssid, String pass) {
  File file = LittleFS.open("/config.json", "w");
  if (!file) {
    Serial.println("[LittleFS] Failed to open /config.json for writing.");
    return false;
  }
  JsonDocument doc;
  doc["ssid"] = ssid;
  doc["pass"] = pass;
  serializeJson(doc, file);
  file.close();
  Serial.println("[LittleFS] Credentials saved successfully.");
  return true;
}

// ── Factory Reset (Erase /config.json) ──
void factoryResetWifi() {
  Serial.println("\n[FACTORY RESET] Holding GPIO 0 for 3s detected! Erasing Wi-Fi config...");
  if (LittleFS.exists("/config.json")) {
    LittleFS.remove("/config.json");
    Serial.println("[FACTORY RESET] /config.json deleted successfully.");
  }
  blinkStatusLED(5, 80); // Rapid blink 5 times
  delay(500);
  Serial.println("[FACTORY RESET] Restarting system...");
  #if defined(ESP8266) || defined(ESP32)
  ESP.restart();
  #endif
}

// ── Fallback HTML if LittleFS data files are not uploaded ──
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

// ── Web Server API Handlers ──
void handleRoot() {
  if (LittleFS.exists("/index.html")) {
    File f = LittleFS.open("/index.html", "r");
    server.streamFile(f, "text/html");
    f.close();
  } else {
    server.send_P(200, "text/html", FALLBACK_HTML);
  }
}

void handleStaticFile(String path, String contentType) {
  if (LittleFS.exists(path)) {
    File f = LittleFS.open(path, "r");
    server.streamFile(f, contentType);
    f.close();
  } else {
    server.send(404, "text/plain", "File Not Found");
  }
}

void handleScan() {
  Serial.println("[API] Scanning Wi-Fi networks...");
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
  Serial.printf("[API] Scan complete. Found %d networks.\n", n);
}

void handleSave() {
  String reqSSID = "";
  String reqPass = "";

  if (server.hasArg("plain")) {
    // JSON Payload
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    reqSSID = doc["ssid"].as<String>();
    reqPass = doc["pass"].as<String>();
  } else if (server.hasArg("ssid")) {
    // Form POST Payload
    reqSSID = server.arg("ssid");
    reqPass = server.arg("pass");
  }

  if (reqSSID.length() > 0) {
    saveWifiConfig(reqSSID, reqPass);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Saved! Rebooting...\"}");
    delay(1000);
    ESP.restart();
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing SSID\"}");
  }
}

// ── Start Access Point Config Portal ──
void startConfigPortal() {
  isAPMode = true;
  Serial.println("\n==========================================");
  Serial.println("   LAUNCHING WI-FI CONFIG AP PORTAL");
  Serial.println("==========================================");

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID);

  Serial.print("[AP] SSID: ");
  Serial.println(apSSID);
  Serial.print("[AP] IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Start DNS Server for Captive Portal (Redirect all domains to 192.168.4.1)
  dnsServer.start(DNS_PORT, "*", apIP);

  // Web Server Routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/styles.css", HTTP_GET, []() { handleStaticFile("/styles.css", "text/css"); });
  server.on("/app.js", HTTP_GET, []() { handleStaticFile("/app.js", "application/javascript"); });
  server.on("/api/scan", HTTP_GET, handleScan);
  server.on("/api/save", HTTP_POST, handleSave);

  // Captive portal detection routes
  server.onNotFound([]() {
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
  });

  server.begin();
  Serial.println("[HTTP] Config Server started on port 80.");
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF);

  Serial.println("\n\n--- IoT Lab 3.2: Web Wi-Fi Configurator ---");

  // Mount LittleFS
  #if defined(ESP8266)
  if (!LittleFS.begin()) {
    Serial.println("[LittleFS] Mount Failed!");
  }
  #elif defined(ESP32)
  if (!LittleFS.begin(true)) {
    Serial.println("[LittleFS] Mount Failed!");
  }
  #endif

  // Check saved Wi-Fi configuration
  if (loadWifiConfig()) {
    Serial.printf("[STA] Connecting to saved Wi-Fi: %s ...\n", wifiSSID.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID.c_str(), wifiPass.c_str());

    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 25) { // 12.5 seconds
      delay(500);
      Serial.print(".");
      retries++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n[STA] Connected successfully!");
      Serial.print("[STA] Local IP Address: ");
      Serial.println(WiFi.localIP());
      blinkStatusLED(3, 150); // Blink 3 times when connected
      return;
    } else {
      Serial.println("\n[STA] Connection failed/timed out. Switching to Config Mode...");
    }
  }

  // If no config or connection failed -> Start AP Config Portal
  startConfigPortal();
}

void loop() {
  // ── 1. GPIO 0 Button Long-Press Detection (>= 3 Seconds) ──
  static unsigned long pressStartTime = 0;
  static bool isPressed = false;

  int btnState = digitalRead(BUTTON_PIN);
  if (btnState == LOW) { // Button active LOW
    if (!isPressed) {
      pressStartTime = millis();
      isPressed = true;
    } else {
      unsigned long duration = millis() - pressStartTime;
      if (duration >= 3000) {
        factoryResetWifi();
      }
    }
  } else {
    isPressed = false;
  }

  // ── 2. AP Mode Process Handling ──
  if (isAPMode) {
    dnsServer.processNextRequest();
    server.handleClient();
  } else {
    // STA Mode Heartbeat Logger
    static unsigned long lastHeartbeat = 0;
    if (millis() - lastHeartbeat >= 10000) {
      lastHeartbeat = millis();
      Serial.printf("[STA Monitor] Connected to %s | IP: %s | RSSI: %d dBm\n",
        wifiSSID.c_str(), WiFi.localIP().toString().c_str(), WiFi.RSSI());
    }
  }

  delay(2);
}
