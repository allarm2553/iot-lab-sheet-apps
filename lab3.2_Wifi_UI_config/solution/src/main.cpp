/**
 * Lab 3.2: Web Wi-Fi Configurator & GPIO 0 Reset Button with OLED Debug Mode
 * Features:
 *  - Persistent Wi-Fi credentials storage using LittleFS (/config.json).
 *  - Event-Driven Auto-Reconnect:
 *      * Uses WiFi.onEvent() (ESP32) / WiFiEventHandler (ESP8266) for real-time background reconnect.
 *      * Auto-Fallback to AP Portal: If disconnected for > 30 seconds, automatically launches AP Portal!
 *  - Automatic fallback to Access Point (AP) mode (SSID: ESP_WiFi_Config, IP: 192.168.4.1) with DNS Captive Portal.
 *  - Embedded Web Server serving Glassmorphic Wi-Fi Setup Dashboard from LittleFS (or inline fallback).
 *  - OLED Debug Display (SSD1306 128x64 I2C 0x3C):
 *      * AP Mode : Displays "[AP Mode] Setup", AP SSID ("ESP_WiFi_Config"), and IP ("192.168.4.1").
 *      * STA Mode: Displays "[STA Mode] Online", Connected SSID, Assigned IP, and Signal RSSI (dBm).
 *      * Reconnect Mode: Displays "[RECONNECTING]" with countdown to AP fallback.
 *      * Reset   : Displays "[RESET] Cleared!" notification on factory reset.
 *  - REST API Endpoints:
 *      GET  /api/scan -> Scans nearby Wi-Fi networks and returns JSON array.
 *      POST /api/save -> Receives SSID/Password, saves to LittleFS, and restarts board.
 *  - Physical Button Reset (GPIO 0 Long-Press >= 3s):
 *      Erases /config.json, blinks LED rapidly (5 times), and enters Config Mode.
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <DNSServer.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDR     0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool hasOLED = false;

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  ESP8266WebServer server(80);
  #define BUTTON_PIN 0    // D3/GPIO 0 (ปุ่ม FLASH บนบอร์ด AX-WiFi)
  #define LED_PIN    2    // D4/GPIO 2 (LED บนบอร์ด AX-WiFi / Active LOW)
  #define LED_ON     LOW
  #define LED_OFF    HIGH
  #define SDA_PIN    4    // D2/GPIO 4
  #define SCL_PIN    5    // D1/GPIO 5
  WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  WebServer server(80);
  #define BUTTON_PIN 0    // GPIO 0 (ปุ่ม SW1 บนบอร์ด IPST-WiFi)
  #define LED_PIN    2    // GPIO 2 (Built-in LED บนบอร์ด ESP32 / Active HIGH)
  #define LED_ON     HIGH
  #define LED_OFF    LOW
  #define SDA_PIN    21   // GPIO 21
  #define SCL_PIN    22   // GPIO 22
#endif

// Access Point & DNS Captive Portal Settings
const char* apSSID = "ESP_WiFi_Config";
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;

bool isAPMode = false;
String wifiSSID = "";
String wifiPass = "";
unsigned long disconnectStartTime = 0;
const unsigned long AP_FALLBACK_TIMEOUT_MS = 30000; // 30 seconds

// ── Forward Declarations ──
void startConfigPortal();
void blinkStatusLED(int times, int delayMs = 120);

// ── OLED Display Helper Functions ──
void initOLED() {
  Wire.begin(SDA_PIN, SCL_PIN);

  if (display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    hasOLED = true;
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(F("IoT Lab 3.2: WiFi UI"));
    display.println(F("Initializing..."));
    display.display();
    delay(500);
  } else {
    Serial.println(F("[OLED] SSD1306 not detected at 0x3C. Continuing without display."));
  }
}

void showOledAPMode() {
  if (!hasOLED) return;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("=== [AP MODE] ==="));
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
  
  display.setCursor(0, 15);
  display.print(F("SSID: "));
  display.println(apSSID);
  
  display.setCursor(0, 28);
  display.print(F("IP  : "));
  display.println(apIP.toString());
  
  display.setCursor(0, 42);
  display.println(F("Portal: http://"));
  display.setCursor(0, 53);
  display.println(F("192.168.4.1 (Port 80)"));
  
  display.display();
}

void showOledSTAMode(String ssid, String ip, int rssi) {
  if (!hasOLED) return;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("=== [STA ONLINE] ==="));
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
  
  display.setCursor(0, 15);
  display.print(F("SSID: "));
  if (ssid.length() > 14) {
    display.println(ssid.substring(0, 13) + ".");
  } else {
    display.println(ssid);
  }
  
  display.setCursor(0, 28);
  display.print(F("IP  : "));
  display.println(ip);
  
  display.setCursor(0, 42);
  display.print(F("RSSI: "));
  display.print(rssi);
  display.println(F(" dBm"));
  
  display.setCursor(0, 54);
  if (rssi >= -60) display.println(F("Signal: Excellent (++)"));
  else if (rssi >= -75) display.println(F("Signal: Good (+)"));
  else display.println(F("Signal: Fair/Weak (-)"));
  
  display.display();
}

void showOledReconnecting(String ssid, int remainingSec) {
  if (!hasOLED) return;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F(">> RECONNECTING <<"));
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
  
  display.setCursor(0, 15);
  display.print(F("Target: "));
  display.println(ssid);
  
  display.setCursor(0, 30);
  display.println(F("Signal Lost..."));
  
  display.setCursor(0, 45);
  display.printf("AP Fallback: %ds\n", remainingSec);
  
  display.display();
}

void showOledReset() {
  if (!hasOLED) return;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.println(F(">> FACTORY RESET <<"));
  display.drawLine(0, 22, 127, 22, SSD1306_WHITE);
  display.setCursor(0, 30);
  display.println(F("WiFi config cleared!"));
  display.setCursor(0, 46);
  display.println(F("Rebooting to AP..."));
  display.display();
}

// ── LED Feedback Helper ──
void blinkStatusLED(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, LED_ON);
    delay(delayMs);
    digitalWrite(LED_PIN, LED_OFF);
    if (i < times - 1) delay(delayMs);
  }
}

// ── Event-Driven Auto-Reconnect Handler ──
#if defined(ESP32)
void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.printf("[Event ✓] Connected! IP: %s | RSSI: %d dBm\n",
                    WiFi.localIP().toString().c_str(), WiFi.RSSI());
      disconnectStartTime = 0;
      isAPMode = false;
      blinkStatusLED(3, 150);
      showOledSTAMode(wifiSSID, WiFi.localIP().toString(), WiFi.RSSI());
      break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      if (!isAPMode) {
        if (disconnectStartTime == 0) {
          disconnectStartTime = millis();
        }
        Serial.println("[Event ⚠️] Wi-Fi Disconnected! Auto-reconnecting in Background...");
        WiFi.reconnect();
      }
      break;

    default:
      break;
  }
}
#endif

void initWiFiEvents() {
  #if defined(ESP32)
  WiFi.onEvent(onWiFiEvent);
  #elif defined(ESP8266)
  gotIpEventHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event) {
    Serial.printf("[Event ✓] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
    disconnectStartTime = 0;
    isAPMode = false;
    blinkStatusLED(3, 150);
    showOledSTAMode(wifiSSID, WiFi.localIP().toString(), WiFi.RSSI());
  });

  disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event) {
    if (!isAPMode) {
      if (disconnectStartTime == 0) {
        disconnectStartTime = millis();
      }
      Serial.println("[Event ⚠️] Wi-Fi Disconnected! Auto-reconnecting in Background...");
      WiFi.reconnect();
    }
  });
  #endif

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
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
  showOledReset();
  if (LittleFS.exists("/config.json")) {
    LittleFS.remove("/config.json");
    Serial.println("[FACTORY RESET] /config.json deleted successfully.");
  }
  blinkStatusLED(5, 80);
  delay(1000);
  Serial.println("[FACTORY RESET] Restarting system...");
  ESP.restart();
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
  disconnectStartTime = 0;

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

  showOledAPMode();

  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/styles.css", HTTP_GET, []() { handleStaticFile("/styles.css", "text/css"); });
  server.on("/app.js", HTTP_GET, []() { handleStaticFile("/app.js", "application/javascript"); });
  server.on("/api/scan", HTTP_GET, handleScan);
  server.on("/api/save", HTTP_POST, handleSave);

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

  Serial.println("\n\n--- IoT Lab 3.2: Web Wi-Fi Configurator with Event-Driven Auto-Reconnect ---");

  initOLED();
  initWiFiEvents();

  #if defined(ESP8266)
  if (!LittleFS.begin()) Serial.println("[LittleFS] Mount Failed!");
  #elif defined(ESP32)
  if (!LittleFS.begin(true)) Serial.println("[LittleFS] Mount Failed!");
  #endif

  if (loadWifiConfig()) {
    Serial.printf("[STA] Connecting to saved Wi-Fi: %s ...\n", wifiSSID.c_str());
    if (hasOLED) {
      display.clearDisplay();
      display.setCursor(0, 10);
      display.println(F("Connecting to:"));
      display.println(wifiSSID);
      display.display();
    }

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
      return;
    } else {
      Serial.println("\n[STA] Connection timed out. Switching to Config Mode...");
    }
  }

  startConfigPortal();
}

void loop() {
  // ── 1. GPIO 0 Button Long-Press Detection (>= 3 Seconds) ──
  static unsigned long pressStartTime = 0;
  static bool isPressed = false;

  int btnState = digitalRead(BUTTON_PIN);
  if (btnState == LOW) {
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

  // ── 2. AP Mode vs STA Mode Process Handling ──
  if (isAPMode) {
    dnsServer.processNextRequest();
    server.handleClient();
  } else {
    // Check if disconnected and calculate fallback timeout to AP Portal
    if (WiFi.status() != WL_CONNECTED) {
      if (disconnectStartTime == 0) {
        disconnectStartTime = millis();
      }

      unsigned long elapsed = millis() - disconnectStartTime;
      int remainingSec = (elapsed < AP_FALLBACK_TIMEOUT_MS) ? (AP_FALLBACK_TIMEOUT_MS - elapsed) / 1000 : 0;
      
      static unsigned long lastOledUpdate = 0;
      if (millis() - lastOledUpdate >= 1000) {
        lastOledUpdate = millis();
        showOledReconnecting(wifiSSID, remainingSec);
      }

      // If disconnected for more than 30s -> Auto Fallback to AP Config Portal
      if (elapsed >= AP_FALLBACK_TIMEOUT_MS) {
        Serial.println("\n[FALLBACK ⚠️] Wi-Fi lost for >30s! Auto-launching Config AP Portal...");
        startConfigPortal();
      }
    } else {
      // Normal STA Mode Monitor
      static unsigned long lastHeartbeat = 0;
      if (millis() - lastHeartbeat >= 5000) {
        lastHeartbeat = millis();
        showOledSTAMode(wifiSSID, WiFi.localIP().toString(), WiFi.RSSI());
      }
    }
  }

  delay(2);
}
