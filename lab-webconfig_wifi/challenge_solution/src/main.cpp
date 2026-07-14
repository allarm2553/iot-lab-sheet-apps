/**
 * Lab WebConfig Wi-Fi: Web Wi-Fi Configurator (Challenge Solution with Scanning)
 * Features:
 *  - Initializes LittleFS to load/save JSON Wi-Fi configs.
 *  - Spawns AP (SSID: ESP-Config-AP) in WIFI_AP_STA mode so scanning STA networks is supported.
 *  - Redirects DNS requests on port 53 to 192.168.4.1 (Captive Portal).
 *  - Serves dynamic configuration portal with a "Scan" button.
 *  - GET /scan -> Executes WiFi.scanNetworks() and returns a JSON list of SSIDs and signal strength.
 *  - Saves selected SSID/Password into /config.json and restarts.
 *  - Supports 3-second BUTTON_PIN (GPIO0) hold to enter AP mode.
 */

#include <Arduino.h>
#include <Wire.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <DNSServer.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#else
#include <WiFi.h>
#include <WebServer.h>
#endif

// Hardware Pins
#if defined(ESP8266)
#define BUTTON_PIN 0        // D3/GPIO 0 (ปุ่ม FLASH บนบอร์ด AX-WiFi)
#else
#define BUTTON_PIN 0        // GPIO 0 (ปุ่ม SW1 บนบอร์ด IPST-WiFi)
#endif

// Access Point Settings
const char* apSSID = "ESP-Config-AP";
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);

DNSServer dnsServer;
#if defined(ESP8266)
ESP8266WebServer server(80);
#else
WebServer server(80);
#endif

bool apMode = false;
String wifiSSID = "";
String wifiPassword = "";

// Load config from LittleFS JSON file
bool loadWifiConfig() {
  if (!LittleFS.exists("/config.json")) {
    Serial.println("Config file /config.json not found.");
    return false;
  }
  
  File file = LittleFS.open("/config.json", "r");
  if (!file) {
    Serial.println("Failed to open /config.json");
    return false;
  }
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    Serial.println("Failed to parse config JSON");
    return false;
  }
  
  wifiSSID = doc["ssid"].as<String>();
  wifiPassword = doc["pass"].as<String>();
  return true;
}

// Save config to LittleFS JSON file
void saveWifiConfig(String ssid, String pass) {
  File file = LittleFS.open("/config.json", "w");
  if (file) {
    JsonDocument doc;
    doc["ssid"] = ssid;
    doc["pass"] = pass;
    serializeJson(doc, file);
    file.close();
    Serial.println("Config saved successfully.");
  } else {
    Serial.println("Failed to open config file for writing.");
  }
}

// Check if string is an IP address
bool isIpAddress(String src) {
  for (size_t i = 0; i < src.length(); i++) {
    char c = src[i];
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

// Captive Portal Redirect
bool redirectToConfig() {
  if (!isIpAddress(server.hostHeader()) && server.hostHeader() != "192.168.4.1") {
    Serial.println("Redirecting to captive portal");
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
    return true;
  }
  return false;
}

// Handle GET /
void handlePortal() {
  if (redirectToConfig()) {
    return;
  }
  
  // Serve the configuration index.html from LittleFS
  if (LittleFS.exists("/index.html")) {
    File file = LittleFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  } else {
    server.send(404, "text/plain", "HTML Portal File Not Found. Please upload files to LittleFS.");
  }
}

// Handle POST /save
void handleSave() {
  if (server.hasArg("ssid") && server.hasArg("password")) {
    String newSSID = server.arg("ssid");
    String newPass = server.arg("password");
    
    Serial.printf("SSID Received: %s\n", newSSID.c_str());
    
    // Save to LittleFS
    saveWifiConfig(newSSID, newPass);
    
    // Respond with a success page
    String html = "<!DOCTYPE html><html lang='th'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>บันทึกสำเร็จ</title>";
    html += "<style>body{background:#0f172a;color:#f8fafc;font-family:sans-serif;text-align:center;padding-top:10%;}div{background:rgba(30,41,59,0.7);padding:2rem;border-radius:12px;display:inline-block;border:1px solid rgba(255,255,255,0.1);}</style></head>";
    html += "<body><div><h2>บันทึกข้อมูลเรียบร้อยแล้ว!</h2><p>บอร์ดจะทำการรีบูตภายใน 3 วินาทีเพื่อทำการเชื่อมต่อกับ Wi-Fi ใหม่...</p></div></body></html>";
    server.send(200, "text/html", html);
    
    delay(2000);
    #if defined(ESP8266)
    ESP.reset();
    #else
    ESP.restart();
    #endif
  } else {
    server.send(400, "text/plain", "Bad Request: SSID and Password are required.");
  }
}

// Handle GET /scan (Challenge Task: Scans for Wi-Fi networks and returns JSON list)
void handleScan() {
  Serial.println("Scanning nearby networks...");
  int n = WiFi.scanNetworks();
  Serial.printf("Scan completed. Found %d networks.\n", n);

  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();

  // Limit list to avoid exceeding RAM limits (max 15 networks)
  int limit = (n > 15) ? 15 : n;
  for (int i = 0; i < limit; ++i) {
    JsonObject net = arr.add<JsonObject>();
    net["ssid"] = WiFi.SSID(i);
    net["rssi"] = WiFi.RSSI(i);
    #if defined(ESP8266)
    net["secure"] = (WiFi.encryptionType(i) != ENC_TYPE_NONE);
    #else
    net["secure"] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
    #endif
  }

  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
  
  // Clear scan results from memory
  WiFi.scanDelete();
}

// Setup Access Point for Captive Portal Config Mode
void startConfigPortal() {
  apMode = true;
  Serial.println("Starting Access Point Mode for Wi-Fi configuration...");
  
  // WIFI_AP_STA mode is REQUIRED on ESP32/ESP8266 to allow scanning STA channels while serving AP.
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID);
  
  // Start DNS server redirecting all requests to apIP
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);
  
  // Set up endpoints
  server.on("/", HTTP_GET, handlePortal);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/scan", HTTP_GET, handleScan); // Challenge Scan Endpoint
  server.onNotFound(handlePortal); // Catch-all for Captive Portal redirects
  server.begin();
  
  Serial.printf("AP Started! SSID: %s\n", apSSID);
  Serial.printf("Config Portal URL: http://%s/\n", apIP.toString().c_str());
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  Serial.println("\nInitializing LittleFS...");
  
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed. Formatting...");
    #if defined(ESP8266)
    LittleFS.format();
    #endif
    // Retry
    if (!LittleFS.begin()) {
      Serial.println("LittleFS hard error! Halt.");
      while (true) delay(1000);
    }
  }
  
  // 1. Try loading Wi-Fi credentials from LittleFS
  if (loadWifiConfig()) {
    Serial.printf("Wi-Fi credentials loaded. SSID: %s\n", wifiSSID.c_str());
    Serial.println("Connecting to Wi-Fi...");
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    
    // Wait for connection (15 seconds timeout)
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 30) {
      delay(500);
      Serial.print(".");
      retries++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected successfully!");
      Serial.print("Local IP Address: ");
      Serial.println(WiFi.localIP());
      return;
    } else {
      Serial.println("\nConnection timed out. Launching Config Portal...");
    }
  } else {
    Serial.println("No saved Wi-Fi credentials found.");
  }
  
  // 2. Launch Captive Portal if connection failed or no configuration was found
  startConfigPortal();
}

void loop() {
  // Check if button (GPIO0) is held down for 3 seconds (3000ms) to trigger config mode
  int buttonVal = digitalRead(BUTTON_PIN);
  static unsigned long buttonPressTime = 0;
  static bool buttonPressed = false;
  
  if (buttonVal == LOW) {
    if (!buttonPressed) {
      buttonPressTime = millis();
      buttonPressed = true;
    } else {
      if (millis() - buttonPressTime >= 3000) {
        Serial.println("\nButton held for 3 seconds! Entering Config Portal...");
        buttonPressed = false; // Reset state
        if (!apMode) {
          WiFi.disconnect();
          startConfigPortal();
        }
      }
    }
  } else {
    buttonPressed = false;
  }

  if (apMode) {
    dnsServer.processNextRequest();
    server.handleClient();
  } else {
    // Normal operation loop
    static unsigned long lastLog = 0;
    if (millis() - lastLog > 5000) {
      lastLog = millis();
      Serial.printf("System Online (STA Mode). IP: %s\n", WiFi.localIP().toString().c_str());
    }
  }
  delay(1);
}
