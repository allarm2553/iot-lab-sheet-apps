/**
 * Lab WebConfig Wi-Fi: Web Wi-Fi Configurator (Asynchronous Version)
 * Features:
 *  - Uses ESPAsyncWebServer for non-blocking asynchronous request handling.
 *  - Stores SSID and Password in separate text files (/ssid.txt, /pass.txt) in LittleFS.
 *  - Spawns DNS Captive Portal on port 53 (redirecting to 192.168.4.1).
 *  - Supports 3-second BUTTON_PIN (GPIO0) hold to enter AP Config Mode dynamically.
 */

#include <Arduino.h>
#include <Wire.h>
#include <LittleFS.h>
#include <DNSServer.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#else
#include <WiFi.h>
#include <AsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

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
AsyncWebServer server(80);

bool apMode = false;
String wifiSSID = "";
String wifiPassword = "";

// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";

// Helper to read file from LittleFS
String readFile(fs::FS &fs, const char * path) {
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()) {
    return String();
  }
  String fileContent;
  while(file.available()) {
    fileContent = file.readStringUntil('\n');
    break;     
  }
  fileContent.trim();
  return fileContent;
}

// Helper to write file to LittleFS
void writeFile(fs::FS &fs, const char * path, const char * message) {
  File file = fs.open(path, "w");
  if(!file) {
    Serial.println("Failed to open file for writing.");
    return;
  }
  file.print(message);
  file.close();
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

// Start Access Point & Captive Portal
void startConfigPortal() {
  apMode = true;
  Serial.println("Starting Access Point Mode for Wi-Fi configuration...");
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID);
  
  // Start DNS Server
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);
  
  Serial.printf("AP Started! SSID: %s\n", apSSID);
  Serial.printf("Config Portal URL: http://%s/\n", apIP.toString().c_str());
}

// Initialize Wi-Fi
bool initWiFi() {
  if(wifiSSID == "") {
    Serial.println("SSID is empty. Cannot connect.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
  Serial.println("Connecting to WiFi...");

  unsigned long startMillis = millis();
  while(WiFi.status() != WL_CONNECTED && millis() - startMillis < 15000) {
    delay(500);
    Serial.print(".");
  }

  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected successfully!");
    Serial.print("Local IP Address: ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println("\nConnection timed out.");
    return false;
  }
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
    if (!LittleFS.begin()) {
      Serial.println("LittleFS hard error! Halt.");
      while (true) delay(1000);
    }
  }

  // Load values saved in LittleFS
  wifiSSID = readFile(LittleFS, ssidPath);
  wifiPassword = readFile(LittleFS, passPath);
  
  Serial.print("SSID loaded: "); Serial.println(wifiSSID);

  // Set up AsyncWebServer Routes
  
  // GET / -> Serves the portal index.html if in AP Mode
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (apMode) {
      request->send(LittleFS, "/index.html", "text/html");
    } else {
      request->send(200, "text/plain", "Welcome to IoT Device Home Page!");
    }
  });

  // Serve static resources from LittleFS
  server.serveStatic("/", LittleFS, "/");

  // POST /save -> Handles Wi-Fi setup submission
  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {
    String newSSID = "";
    String newPass = "";

    if (request->hasParam("ssid", true)) {
      newSSID = request->getParam("ssid", true)->value();
      writeFile(LittleFS, ssidPath, newSSID.c_str());
    }
    if (request->hasParam("password", true)) {
      newPass = request->getParam("password", true)->value();
      writeFile(LittleFS, passPath, newPass.c_str());
    }

    Serial.printf("SSID Configured: %s\n", newSSID.c_str());

    String html = "<!DOCTYPE html><html lang='th'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>บันทึกสำเร็จ</title>";
    html += "<style>body{background:#0f172a;color:#f8fafc;font-family:sans-serif;text-align:center;padding-top:10%;}div{background:rgba(30,41,59,0.7);padding:2rem;border-radius:12px;display:inline-block;border:1px solid rgba(255,255,255,0.1);}</style></head>";
    html += "<body><div><h2>บันทึกข้อมูล (Async) เรียบร้อยแล้ว!</h2><p>บอร์ดจะทำการรีบูตเพื่อเชื่อมต่อกับ Wi-Fi ใหม่...</p></div></body></html>";
    request->send(200, "text/html", html);

    // Reboot after 2 seconds
    // Since we are inside async handler, we shouldn't block the thread too long.
    // ESP8266 or ESP32 will restart.
    delay(2000);
    #if defined(ESP8266)
    ESP.reset();
    #else
    ESP.restart();
    #endif
  });

  // Captive Portal Redirect for not found requests
  server.onNotFound([](AsyncWebServerRequest *request) {
    if (apMode && !isIpAddress(request->host()) && request->host() != "192.168.4.1") {
      request->redirect("http://192.168.4.1/");
    } else {
      request->send(404, "text/plain", "Not Found");
    }
  });

  server.begin();

  if (initWiFi()) {
    // Normal operation running in STA mode starts here.
    return;
  } else {
    // Connection failed -> Start AP portal
    startConfigPortal();
  }
}

void loop() {
  // Check if button (GPIO0) is held down for 3 seconds to trigger config mode
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
        buttonPressed = false;
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
    // DNSServer needs to process requests synchronously
    dnsServer.processNextRequest();
  } else {
    // Normal operation STA loop
    static unsigned long lastLog = 0;
    if (millis() - lastLog > 5000) {
      lastLog = millis();
      Serial.printf("System Online (STA Mode). IP: %s\n", WiFi.localIP().toString().c_str());
    }
  }
  delay(1);
}
