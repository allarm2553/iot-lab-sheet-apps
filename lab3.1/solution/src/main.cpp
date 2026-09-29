/**
 * =====================================================================
 *  Lab 3.1 — Solution Code (PlatformIO / C++)
 *  ชื่อ: การพัฒนา IoT Web Dashboard ด้วย LittleFS และ REST API
 * =====================================================================
 * 
 *  คุณสมบัติเด่น (Key Features):
 *   1. เชื่อมต่อ Wi-Fi SSID "iot_512", Password "iot123456"
 *   2. อ่านค่าอุณหภูมิและความชื้นจาก DHT11 (GPIO 33) และสัญญาณอนาล็อก (GPIO 36)
 *   3. เมานต์ระบบไฟล์ LittleFS Flash Memory
 *   4. สตรีมไฟล์ Static (.html, .css, .js) ด้วย server.streamFile()
 *   5. REST API Endpoint:
 *      - GET /api/data    : ส่งค่าเซ็นเซอร์ (temp, hum, analog) และสถานะรีเลย์ (JSON)
 *      - GET /api/control : ควบคุม Relay 1 พัดลม (GPIO 5) และ Relay 2 ปั๊มหมอก (GPIO 23)
 *   6. Dual Automation Thresholds:
 *      - อุณหภูมิ DHT11 >= 35.0 °C -> เปิด Relay 1 พัดลม (GPIO 5) อัตโนมัติ (ปิดเมื่อ < 32.0 °C)
 *      - สัญญาณอนาล็อก > 2500 -> เปิด Relay 2 ปั๊มหมอก (GPIO 23) อัตโนมัติ (ปิดเมื่อ <= 2000)
 * 
 *  วงจรฮาร์ดแวร์ (ใช้วงจรเดิมจาก Lab 1):
 *   - DHT11 Sensor                : GPIO 33 (Data)
 *   - Analog Sensor (VR / LDR)    : GPIO 36 (VP / ADC1)
 *   - Relay 1 (พัดลมระบายความร้อน) : GPIO 5 (Active-LOW)
 *   - Relay 2 (ปั๊มพ่นหมอก / รดน้ำ) : GPIO 23 (Active-LOW)
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
#include <DHT.h>

// ─── ข้อมูลการเชื่อมต่อ Wi-Fi ──────────────────────────────────────────
const char* ssid     = "iot_512";
const char* password = "iot123456";

// ─── นิยามขาใช้งาน (Pin Definition) ──────────────────────────────────
#define DHTPIN         33   // ขาข้อมูล DHT11 จากวงจร Lab 1
#define DHTTYPE        DHT11
#define ANALOG_PIN     36   // Analog Input (ADC1_CH0 / VP)
#define RELAY1_PIN      5   // Relay 1 พัดลม (Active-LOW)
#define RELAY2_PIN     23   // Relay 2 ปั๊มพ่นหมอก (Active-LOW)
#define STATUS_LED_PIN  2   // Onboard LED

DHT dht(DHTPIN, DHTTYPE);

// ─── ตัวแปรสถานะระบบ (System State) ───────────────────────────────────
bool relay1State = false;
bool relay2State = false;
bool autoAlert1  = false;
bool autoAlert2  = false;

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
  float tempC = dht.readTemperature();
  float hum = dht.readHumidity();
  int rawAnalog = analogRead(ANALOG_PIN);
  
  if (isnan(tempC)) tempC = 0.0;
  if (isnan(hum)) hum = 0.0;
  
  String json = "{";
  json += "\"temp\":" + String(tempC, 1) + ",";
  json += "\"hum\":" + String(hum, 1) + ",";
  json += "\"analog\":" + String(rawAnalog) + ",";
  json += "\"relay1\":" + String(relay1State ? "true" : "false") + ",";
  json += "\"relay2\":" + String(relay2State ? "true" : "false") + ",";
  json += "\"alert1\":" + String(autoAlert1 ? "true" : "false") + ",";
  json += "\"alert2\":" + String(autoAlert2 ? "true" : "false");
  json += "}";
  
  server.send(200, "application/json", json);
}

// ─── 2. REST API: ควบคุมการเปิด-ปิด Relay จากคำสั่งบนเว็บ ──────────────────
void handleApiControl() {
  if (server.hasArg("relay") && server.hasArg("state")) {
    String relay = server.arg("relay");
    bool state = (server.arg("state") == "1" || server.arg("state") == "true");

    if (relay == "1" || relay == "fan") {
      relay1State = state;
      digitalWrite(RELAY1_PIN, relay1State ? LOW : HIGH); // Active-LOW
      Serial.printf("[CONTROL] Relay 1 (Fan) -> %s\n", relay1State ? "ON" : "OFF");
    } else if (relay == "2" || relay == "mist") {
      relay2State = state;
      digitalWrite(RELAY2_PIN, relay2State ? LOW : HIGH);
      Serial.printf("[CONTROL] Relay 2 (Mist) -> %s\n", relay2State ? "ON" : "OFF");
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

  // เริ่มต้นเซ็นเซอร์ DHT11
  dht.begin();

  // กำหนดโหมดของขาเอาต์พุต
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);

  // ปิดรีเลย์เริ่มต้น (Active-LOW: HIGH = OFF)
  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);
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

  // 1. เงื่อนไขอุณหภูมิควบคุม Relay 1 (พัดลม)
  float tempC = dht.readTemperature();
  if (!isnan(tempC)) {
    if (tempC >= 35.0) {
      if (!relay1State) {
        relay1State = true;
        digitalWrite(RELAY1_PIN, LOW); // เปิดพัดลมระบายความร้อน
        Serial.println("[SAFETY ALERT] อุณหภูมิ >= 35°C -> เปิดพัดลมอัตโนมัติ!");
      }
      autoAlert1 = true;
    } else if (tempC < 32.0) {
      if (relay1State) {
        relay1State = false;
        digitalWrite(RELAY1_PIN, HIGH); // ปิดพัดลม
        Serial.println("[INFO] อุณหภูมิต่ำกว่า 32°C -> ปิดพัดลมอัตโนมัติ");
      }
      autoAlert1 = false;
    }
  }

  // 2. เงื่อนไขสัญญาณอนาล็อกควบคุม Relay 2 (ปั๊มหมอก/รดน้ำ)
  int rawAnalog = analogRead(ANALOG_PIN);
  if (rawAnalog > 2500) {
    if (!relay2State) {
      relay2State = true;
      digitalWrite(RELAY2_PIN, LOW); // เปิดปั๊มหมอก
      Serial.println("[ANALOG ALERT] สัญญาณอนาล็อก > 2500 -> เปิดปั๊มหมอกอัตโนมัติ!");
    }
    autoAlert2 = true;
  } else if (rawAnalog <= 2000) {
    if (relay2State) {
      relay2State = false;
      digitalWrite(RELAY2_PIN, HIGH); // ปิดปั๊มหมอก
      Serial.println("[INFO] สัญญาณอนาล็อก <= 2000 -> ปิดปั๊มหมอก");
    }
    autoAlert2 = false;
  }

  delay(2);
}
