/**
 * Lab WebConfig Wi-Fi: Web Wi-Fi Configurator (Solution Code)
 * Features:
 *  - Initializes LittleFS to load/save JSON-based Wi-Fi config.
 *  - If no credentials exist or connection fails, starts an Access Point (SSID: ESP-Config-AP).
 *  - Spawns a DNS Server (Port 53) to redirect all traffic to 192.168.4.1 (Captive Portal).
 *  - Spawns a Web Server (Port 80) serving a beautiful glassmorphism configuration form.
 *  - Saves submitted SSID/Password into /config.json and restarts.
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
  // BLANK 2: LittleFS.open("/config.json", "w")
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

// Setup Access Point for Captive Portal Config Mode
void startConfigPortal() {
  apMode = true;
  Serial.println("Starting Access Point Mode for Wi-Fi configuration...");
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID);
  
  // Start DNS server redirecting all requests to apIP
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);
  
  // Set up endpoints
  server.on("/", HTTP_GET, handlePortal);
  server.on("/save", HTTP_POST, handleSave);
  server.onNotFound(handlePortal); // Catch-all for Captive Portal redirects
  server.begin();
  
  Serial.printf("AP Started! SSID: %s\n", apSSID);
  Serial.printf("Config Portal URL: http://%s/\n", apIP.toString().c_str());
}

void setup() {
  Serial.begin(115200);
  delay(1000);
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
      // Normal application running in station mode can start here.
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
  if (apMode) {
    // BLANK 1: dnsServer.processNextRequest()
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
