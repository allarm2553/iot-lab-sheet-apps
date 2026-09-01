/**
 * =====================================================================
 *  Lab 3.1 — Solution Code (PlatformIO / C++)
 *  ชื่อ: เว็บเซิร์ฟเวอร์บนบอร์ด ESP32 / ESP8266 (LittleFS Web Server)
 * =====================================================================
 * 
 *  คุณสมบัติเด่น (Key Features):
 *   1. เชื่อมต่อ Wi-Fi SSID "iot_512", Password "iot123456"
 *   2. เมานต์ระบบไฟล์ LittleFS Flash Memory
 *   3. สตรีมไฟล์ Static (.html, .css, .js, .png, .ico) ด้วย server.streamFile()
 *   4. ดักจับและตอบกลับกรณีไม่พบไฟล์ (404 Not Found)
 *   5. ทำงานแบบประหยัด RAM (Heap Memory Safe)
 * 
 *  รองรับบอร์ด:
 *   - ESP32   : IPST-WiFi, ESP32 DevKitC
 *   - ESP8266 : AX-WiFi, NodeMCU v2
 * =====================================================================
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

// ─── กำหนดข้อมูลการเชื่อมต่อ Wi-Fi ──────────────────────────────────────────
const char* ssid     = "ALLARM_HOME_WiFi";
const char* password = "arm123456";

// ─── ฟังก์ชันช่วยตรวจสอบ Content-Type (MIME Type) ───────────────────────────
String getContentType(String path) {
  if (path.endsWith(".html") || path.endsWith(".htm")) return "text/html";
  else if (path.endsWith(".css"))                      return "text/css";
  else if (path.endsWith(".js"))                       return "application/javascript";
  else if (path.endsWith(".json"))                     return "application/json";
  else if (path.endsWith(".png"))                      return "image/png";
  else if (path.endsWith(".jpg") || path.endsWith(".jpeg")) return "image/jpeg";
  else if (path.endsWith(".gif"))                      return "image/gif";
  else if (path.endsWith(".ico"))                      return "image/x-icon";
  else if (path.endsWith(".svg"))                      return "image/svg+xml";
  return "text/plain";
}

// ─── ฟังก์ชันอ่านและสตรีมไฟล์จาก LittleFS ไปยัง Client ───────────────────────
void handleFileRequest() {
  String path = server.uri();
  
  // หากเข้าถึง Path หลัก ("/") ให้เรียกไฟล์ index.html เป็นค่าเริ่มต้น
  if (path.endsWith("/")) {
    path += "index.html";
  }
  
  String dataType = getContentType(path);
  
  // ตรวจสอบว่ามีไฟล์ดังกล่าวอยู่ใน LittleFS หรือไม่
  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    // สตรีมไฟล์ตรงจาก Flash Memory ไปยัง Client โดยไม่เปลือง RAM
    server.streamFile(file, dataType);
    file.close();
    Serial.printf("[200 OK] ให้บริการไฟล์: %s (Content-Type: %s)\n", path.c_str(), dataType.c_str());
  } else {
    // กรณีไม่พบไฟล์ ให้ตอบกลับ Error 404
    String notFoundMsg = "404: File Not Found\n\nURI: " + path + "\n";
    server.send(404, "text/plain", notFoundMsg);
    Serial.printf("[404 NOT FOUND] ไม่พบไฟล์: %s\n", path.c_str());
  }
}

// =============================================================================
//  SETUP
// =============================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n==========================================");
  Serial.println("  Lab 3.1: LittleFS Embedded Web Server");
  Serial.println("==========================================");

  // 1. เชื่อมต่อ Wi-Fi ในโหมด Station
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.printf("กำลังเชื่อมต่อ Wi-Fi \"%s\"", ssid);
  
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 40) {
    delay(500);
    Serial.print(".");
    retry++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[OK] เชื่อมต่อ Wi-Fi สำเร็จ!");
    Serial.print("  IP Address: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[WARN] ไม่สามารถเชื่อมต่อ Wi-Fi ได้ (จะทำงานในโหมดออฟไลน์/ทดสอบระบบไฟล์)");
  }

  // 2. เริ่มต้นเมานต์ระบบไฟล์ LittleFS
  #if defined(ESP8266)
  if (!LittleFS.begin()) {
  #else
  if (!LittleFS.begin(true)) { // true = formatOnFail ถ้าเมานต์ไม่สำเร็จให้ฟอร์แมต
  #endif
    Serial.println("[FAIL] การเมานต์ระบบไฟล์ LittleFS ล้มเหลว!");
    return;
  }
  Serial.println("[OK] เมานต์ระบบไฟล์ LittleFS สำเร็จ");

  // 3. กำหนด Route สำหรับจัดการคำขอไฟล์ทั้งหมด (Catch-All / onNotFound)
  server.onNotFound(handleFileRequest);

  // 4. เริ่มต้นการทำงานของ HTTP Web Server
  server.begin();
  Serial.println("[OK] HTTP Web Server เริ่มทำงานที่พอร์ต 80 เรียบร้อยแล้ว");
  Serial.println("พิมพ์ IP Address ของบอร์ดลงในเว็บบราวเซอร์เพื่อเปิดหน้าเว็บ");
}

// =============================================================================
//  LOOP
// =============================================================================
void loop() {
  // ประมวลผลคำขอจาก Client ที่เข้ามายัง Web Server
  server.handleClient();
  
  // หน่วงเวลาเล็กน้อยเพื่อให้ Background Tasks (Wi-Fi stack) ทำงานได้อย่างราบรื่น
  delay(2);
}
