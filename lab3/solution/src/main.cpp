/**
 * Lab 3: Web Server on ESP32 (LittleFS Web Server - Complete Code Solution)
 * Features:
 *  - Connecting to Wi-Fi SSID "iot_512", password "iot123456".
 *  - Initializing LittleFS filesystem.
 *  - Streaming requested files (.html, .css, .js) dynamically.
 *  - Handling 404 File Not Found.
 *  - Challenge: Serving index.html containing "Hello, World from ESP32 LittleFS!"
 */

#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
#elif defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
WebServer server(80);
#endif
#include <LittleFS.h>

const char* ssid = "iot_512";
const char* password = "iot123456";

// Helper function to serve static files from LittleFS
void handleFileRequest() {
  String path = server.uri();
  
  // If requesting root path, append default index.html
  if (path.endsWith("/")) {
    path += "index.html";
  }
  
  // Determine Content-Type based on file extension
  String dataType = "text/plain";
  if (path.endsWith(".html")) {
    dataType = "text/html";
  } else if (path.endsWith(".css")) {
    dataType = "text/css";
  } else if (path.endsWith(".js")) {
    dataType = "application/javascript";
  } else if (path.endsWith(".png")) {
    dataType = "image/png";
  } else if (path.endsWith(".jpg") || path.endsWith(".jpeg")) {
    dataType = "image/jpeg";
  } else if (path.endsWith(".ico")) {
    dataType = "image/x-icon";
  }
  
  // Check if file exists in LittleFS and stream it
  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    server.streamFile(file, dataType);
    file.close();
    Serial.printf("Served file: %s (Type: %s)\n", path.c_str(), dataType.c_str());
  } else {
    server.send(404, "text/plain", "File Not Found");
    Serial.printf("File not found: %s\n", path.c_str());
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // 1. เชื่อมต่อ Wi-Fi
  Serial.printf("Connecting to Wi-Fi %s...", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // 2. เริ่มต้นเมานต์ระบบไฟล์ LittleFS
  #if defined(ESP8266)
  if(!LittleFS.begin()){
  #else
  if(!LittleFS.begin(true)){
  #endif
     Serial.println("LittleFS Mount Failed");
     return;
  }
  Serial.println("LittleFS mounted successfully.");
  
  // 3. ตั้งเส้นทางเซิร์ฟเวอร์ให้ส่งไฟล์แบบ Dynamic เมื่อมีการเรียกหน้านอกเหนือจากที่ระบุ (onNotFound)
  server.onNotFound(handleFileRequest);
  
  // เริ่มต้นทำงาน Web Server
  server.begin();
  Serial.println("HTTP Web Server started on port 80.");
}

void loop() {
  // 4. จัดการประมวลผลคำสั่งฝั่งไคลเอนต์ที่ส่งเข้ามาที่เว็บเซิร์ฟเวอร์
  server.handleClient();
  
  // Yield execution to allow background Wi-Fi tasks to run smoothly
  delay(2);
}
