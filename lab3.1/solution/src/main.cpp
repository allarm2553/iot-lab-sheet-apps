/**
 * =====================================================================
 *  Lab 3.1 — Solution Code (PlatformIO / C++)
 *  ชื่อ: การพัฒนา IoT Web Dashboard ด้วย LittleFS และ REST API
 * =====================================================================
 * 
 *  คุณสมบัติเด่น (Key Features):
 *   1. เชื่อมต่อ Wi-Fi SSID "iot_512", Password "iot123456"
 *   2. เมานต์ระบบไฟล์ LittleFS Flash Memory
 *   3. สตรีมไฟล์ Static (.html, .css, .js) ด้วย server.streamFile()
 *   4. REST API Endpoint:
 *      - GET /api/data    : ส่งค่าเซ็นเซอร์ (raw, temp) และสถานะรีเลย์ (JSON)
 *      - GET /api/control : ควบคุม Relay พัดลม (GPIO 5) และปั๊มหมอก (GPIO 23)
 *   5. Safety Automation: ตรวจสอบอุณหภูมิ > 35°C สั่งเปิดพัดลมอัตโนมัติ
 * 
 *  วงจรฮาร์ดแวร์ (ใช้วงจรเดิมจาก Lab 1):
 *   - Analog Sensor (Potentiometer) : GPIO 36 (VP / ADC1)
 *   - Fan Relay (พัดลม)            : GPIO 5 (Active-LOW)
 *   - Mist Relay (ปั๊มพ่นหมอก)      : GPIO 23 (Active-LOW)
 *   - Status LED                  : GPIO 2
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

// ─── ข้อมูลการเชื่อมต่อ Wi-Fi ──────────────────────────────────────────
const char* ssid     = "iot_512";
const char* password = "iot123456";

// ─── นิยามขาใช้งาน (Pin Definition) ──────────────────────────────────
#define SENSOR_PIN     36   // Analog Input (ADC1_CH0 / VP)
#define FAN_RELAY_PIN   5   // Fan Relay (Active-LOW)
#define MIST_RELAY_PIN 23   // Mist Relay (Active-LOW)
#define STATUS_LED_PIN  2   // Onboard LED

// ─── ตัวแปรสถานะระบบ (System State) ───────────────────────────────────
bool fanState   = false;
bool mistState  = false;
bool autoAlert  = false;

// ─── ฟังก์ชันช่วยตรวจสอบ Content-Type (MIME Type) ───────────────────────────
String getContentType(String path) {
  if (path.endsWith(".html") || path.endsWith(".htm")) return "text/html";
  else if (path.endsWith(".css"))                      return "text/css";
  else if (path.endsWith(".js"))                       return "application/javascript";
  else if (path.endsWith(".json"))                     return "application/json";
  else if (path.endsWith(".png"))                      return "image/png";
  else if (path.endsWith(".jpg") || path.endsWith(".jpeg")) return "image/jpeg";
  else if (path.endsWith(".ico"))                      return "image/x-icon";
  else if (path.endsWith(".svg"))                      return "image/svg+xml";
  return "text/plain";
}

// ─── 1. REST API: ส่งค่าเซ็นเซอร์และสถานะรีเลย์แบบ JSON ────────────────────
void handleApiData() {
  int rawAnalog = analogRead(SENSOR_PIN);
  float tempC = (rawAnalog / 4095.0) * 100.0; // คำนวณเป็นอุณหภูมิจำลอง 0.0 - 100.0 °C
  
  String json = "{";
  json += "\"raw\":" + String(rawAnalog) + ",";
  json += "\"temp\":" + String(tempC, 1) + ",";
  json += "\"fan\":" + String(fanState ? "true" : "false") + ",";
  json += "\"mist\":" + String(mistState ? "true" : "false") + ",";
  json += "\"alert\":" + String(autoAlert ? "true" : "false");
  json += "}";
  
  server.send(200, "application/json", json);
}

// ─── 2. REST API: ควบคุมการเปิด-ปิด Relay จากคำสั่งบนเว็บ ──────────────────
void handleApiControl() {
  if (server.hasArg("relay") && server.hasArg("state")) {
    String relay = server.arg("relay");
    bool state = (server.arg("state") == "1" || server.arg("state") == "true");

    if (relay == "fan") {
      fanState = state;
      digitalWrite(FAN_RELAY_PIN, fanState ? LOW : HIGH); // Active-LOW
      Serial.printf("[CONTROL] Fan Relay -> %s\n", fanState ? "ON" : "OFF");
    } else if (relay == "mist") {
      mistState = state;
      digitalWrite(MIST_RELAY_PIN, mistState ? LOW : HIGH);
      Serial.printf("[CONTROL] Mist Relay -> %s\n", mistState ? "ON" : "OFF");
    }
    
    server.send(200, "application/json", "{\"success\":true}");
  } else {
    server.send(400, "application/json", "{\"error\":\"Missing relay or state parameter\"}");
  }
}

// ─── 3. ฟังก์ชันอ่านและสตรีมไฟล์จาก LittleFS ไปยัง Client ───────────────────────
void handleFileRequest() {
  String path = server.uri();
  if (path.endsWith("/")) path += "index.html";
  
  String dataType = getContentType(path);
  
  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    server.streamFile(file, dataType);
    file.close();
  } else {
    String notFoundMsg = "404: File Not Found\nURI: " + path;
    server.send(404, "text/plain", notFoundMsg);
  }
}

// =============================================================================
//  SETUP
// =============================================================================
void setup() {
  Serial.begin(115200);
  delay(500);

  // กำหนดโหมดของขาเอาต์พุต
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(MIST_RELAY_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);

  // ปิดรีเลย์เริ่มต้น (Active-LOW: HIGH = OFF)
  digitalWrite(FAN_RELAY_PIN, HIGH);
  digitalWrite(MIST_RELAY_PIN, HIGH);
  digitalWrite(STATUS_LED_PIN, LOW);

  Serial.println("\n========================================================");
  Serial.println("  Lab 3.1: IoT Web Dashboard & REST API Server (Port 80)");
  Serial.println("========================================================");

  // เชื่อมต่อ Wi-Fi
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
    digitalWrite(STATUS_LED_PIN, HIGH);
    Serial.println("\n[OK] เชื่อมต่อ Wi-Fi สำเร็จ!");
    Serial.print("  Dashboard URL: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[WARN] ไม่สามารถเชื่อมต่อ Wi-Fi ได้");
  }

  // เมานต์ระบบไฟล์ LittleFS
  #if defined(ESP8266)
  if (!LittleFS.begin()) {
  #else
  if (!LittleFS.begin(true)) {
  #endif
    Serial.println("[FAIL] การเมานต์ LittleFS ล้มเหลว!");
    return;
  }
  Serial.println("[OK] เมานต์ระบบไฟล์ LittleFS สำเร็จ");

  // ลงทะเบียน REST API Endpoints
  server.on("/api/data", HTTP_GET, handleApiData);
  server.on("/api/control", HTTP_GET, handleApiControl);

  // กำหนด Catch-All สำหรับสตรีมไฟล์หน้าเว็บจาก LittleFS
  server.onNotFound(handleFileRequest);

  // เริ่มต้น Web Server
  server.begin();
  Serial.println("[OK] HTTP Web Server พร้อมให้บริการ!");
}

// =============================================================================
//  LOOP
// =============================================================================
void loop() {
  // ประมวลผลคำขอ HTTP Request
  server.handleClient();

  // Safety Automation: ตรวจสอบอุณหภูมิเซ็นเซอร์จำลอง (ADC GPIO 36)
  int raw = analogRead(SENSOR_PIN);
  float currentTemp = (raw / 4095.0) * 100.0;

  if (currentTemp > 35.0) {
    if (!fanState) {
      fanState = true;
      digitalWrite(FAN_RELAY_PIN, LOW); // เปิดพัดลมระบายความร้อน
      Serial.println("[SAFETY ALERT] อุณหภูมิเกิน 35°C -> เปิดพัดลมอัตโนมัติ!");
    }
    autoAlert = true;
  } else {
    autoAlert = false;
  }

  delay(2);
}
