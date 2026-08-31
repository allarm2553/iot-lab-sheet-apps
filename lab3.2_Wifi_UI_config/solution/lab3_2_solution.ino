/**
 * Lab 3.2: Web Wi-Fi Configurator & GPIO 0 Reset Button with OLED Debug Mode
 * Standalone Arduino IDE Sketch (.ino) with Event-Driven Auto-Reconnect & 30s AP Fallback
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
  #define BUTTON_PIN 0
  #define LED_PIN    2
  #define LED_ON     LOW
  #define LED_OFF    HIGH
  #define SDA_PIN    4
  #define SCL_PIN    5
  WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  WebServer server(80);
  #define BUTTON_PIN 0
  #define LED_PIN    2
  #define LED_ON     HIGH
  #define LED_OFF    LOW
  #define SDA_PIN    21
  #define SCL_PIN    22
#endif

const char* apSSID = "ESP_WiFi_Config";
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;

bool isAPMode = false;
String wifiSSID = "";
String wifiPass = "";
unsigned long disconnectStartTime = 0;
const unsigned long AP_FALLBACK_TIMEOUT_MS = 30000;

void startConfigPortal();

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
  if (ssid.length() > 14) display.println(ssid.substring(0, 13) + ".");
  else display.println(ssid);
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

void blinkStatusLED(int times, int delayMs = 120) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, LED_ON);
    delay(delayMs);
    digitalWrite(LED_PIN, LED_OFF);
    if (i < times - 1) delay(delayMs);
  }
}

#if defined(ESP32)
void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      disconnectStartTime = 0;
      isAPMode = false;
      blinkStatusLED(3, 150);
      showOledSTAMode(wifiSSID, WiFi.localIP().toString(), WiFi.RSSI());
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      if (!isAPMode) {
        if (disconnectStartTime == 0) disconnectStartTime = millis();
        WiFi.reconnect();
      }
      break;
    default: break;
  }
}
#endif

void initWiFiEvents() {
  #if defined(ESP32)
  WiFi.onEvent(onWiFiEvent);
  #elif defined(ESP8266)
  gotIpEventHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event) {
    disconnectStartTime = 0;
    isAPMode = false;
    blinkStatusLED(3, 150);
    showOledSTAMode(wifiSSID, WiFi.localIP().toString(), WiFi.RSSI());
  });
  disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event) {
    if (!isAPMode) {
      if (disconnectStartTime == 0) disconnectStartTime = millis();
      WiFi.reconnect();
    }
  });
  #endif
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
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
  showOledReset();
  if (LittleFS.exists("/config.json")) {
    LittleFS.remove("/config.json");
  }
  blinkStatusLED(5, 80);
  delay(1000);
  ESP.restart();
}

void handleRoot() {
  server.send(200, "text/html", "<h2>Wi-Fi Setup Portal</h2><p>Saved! Rebooting...</p>");
}

void handleScan() {
  int n = WiFi.scanNetworks();
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();
  for (int i = 0; i < n; i++) {
    JsonObject obj = arr.add<JsonObject>();
    obj["ssid"] = WiFi.SSID(i);
    obj["rssi"] = WiFi.RSSI(i);
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
  disconnectStartTime = 0;
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID);
  
  showOledAPMode();
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

  initOLED();
  initWiFiEvents();

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
  } else {
    if (WiFi.status() != WL_CONNECTED) {
      if (disconnectStartTime == 0) disconnectStartTime = millis();
      unsigned long elapsed = millis() - disconnectStartTime;
      int remainingSec = (elapsed < AP_FALLBACK_TIMEOUT_MS) ? (AP_FALLBACK_TIMEOUT_MS - elapsed) / 1000 : 0;
      
      static unsigned long lastOled = 0;
      if (millis() - lastOled >= 1000) {
        lastOled = millis();
        showOledReconnecting(wifiSSID, remainingSec);
      }

      if (elapsed >= AP_FALLBACK_TIMEOUT_MS) {
        startConfigPortal();
      }
    } else {
      static unsigned long lastHeartbeat = 0;
      if (millis() - lastHeartbeat >= 5000) {
        lastHeartbeat = millis();
        showOledSTAMode(wifiSSID, WiFi.localIP().toString(), WiFi.RSSI());
      }
    }
  }
  delay(2);
}
