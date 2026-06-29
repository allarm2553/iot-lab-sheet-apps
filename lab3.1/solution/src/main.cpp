/**
 * =====================================================================
 *  Lab 3.1 — Solution Code (PlatformIO / Arduino C++)
 *  ชื่อ: การเชื่อมต่อ WiFi เบื้องต้น (WiFi Scan & Station Mode)
 * =====================================================================
 *
 *  โปรแกรมนี้รวมการทดลองทั้ง 2 ส่วน + โจทย์ท้าทายไว้ในไฟล์เดียว:
 *
 *  ► STEP 1 — WiFi Scan
 *      สแกนหาเครือข่ายทั้งหมด แสดง SSID / RSSI / Channel / Security
 *      พร้อมค้นหาเครือข่ายเป้าหมาย TARGET_SSID จากรายการที่สแกนได้
 *
 *  ► STEP 2 — WiFi Station Connect
 *      เชื่อมต่อ ACCESS POINT ด้วย begin(ssid, password)
 *      รอจนกว่า WL_CONNECTED แสดง IP / RSSI / MAC
 *
 *  ► CHALLENGE — LED Blink Indicator
 *      เมื่อเชื่อมต่อสำเร็จ ให้ LED บนบอร์ดกะพริบ 3 ครั้ง
 *      (GPIO 2 ทั้ง ESP32 และ ESP8266)
 *
 *  ► LOOP — Monitor Connection
 *      ตรวจสอบสถานะการเชื่อมต่อทุก 5 วินาที
 *      ถ้าหลุดให้ reconnect อัตโนมัติ
 *
 *  Board Support:
 *      - ESP32   : IPST-WiFi, ESP32 DevKitC  (env: ipst_wifi)
 *      - ESP8266 : AX-WiFi, NodeMCU v2       (env: ax_wifi)
 * =====================================================================
 */

#include <Arduino.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #define LED_PIN       2       // GPIO 2 (SLED บน AX-WiFi, Active LOW)
  #define LED_ON        LOW
  #define LED_OFF       HIGH
  #define ENC_OPEN      ENC_TYPE_NONE
#elif defined(ESP32)
  #include <WiFi.h>
  #define LED_PIN       2       // GPIO 2 (LED Built-in บน ESP32 DevKit, Active HIGH)
  #define LED_ON        HIGH
  #define LED_OFF       LOW
  #define ENC_OPEN      WIFI_AUTH_OPEN
#endif

// ─── WiFi Credentials ────────────────────────────────────────────────────────
const char* TARGET_SSID = "iot_512";    // SSID เป้าหมายที่ต้องการค้นหาและเชื่อมต่อ
const char* PASSWORD     = "iot123456"; // Password ของ ACCESS POINT

// ─── Timing ──────────────────────────────────────────────────────────────────
const unsigned long CHECK_INTERVAL_MS = 5000;  // ตรวจสอบสถานะทุก 5 วินาที
unsigned long lastCheckTime = 0;

// ─── Forward Declarations ────────────────────────────────────────────────────
void stepScan();
void stepConnect();
void blinkLED(int times, int onMs = 150, int offMs = 150);
void printDivider(char ch = '-', int len = 55);

// =============================================================================
//  SETUP
// =============================================================================
void setup() {
  Serial.begin(115200);
  delay(500);

  // ตั้งค่า LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF);

  // ─── STEP 1: WiFi Scan ───────────────────────────────────────────────────
  stepScan();

  // ─── STEP 2: WiFi Connect ────────────────────────────────────────────────
  stepConnect();
}

// =============================================================================
//  LOOP — Connection Monitor
// =============================================================================
void loop() {
  unsigned long now = millis();

  if (now - lastCheckTime >= CHECK_INTERVAL_MS) {
    lastCheckTime = now;

    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("[OK] WiFi ยังคงเชื่อมต่ออยู่ | IP: %s | RSSI: %d dBm\n",
                    WiFi.localIP().toString().c_str(),
                    WiFi.RSSI());
    } else {
      Serial.println("[!!] WiFi หลุดการเชื่อมต่อ! กำลังพยายามเชื่อมต่อใหม่...");
      WiFi.reconnect();

      // รอสักครู่
      int retry = 0;
      while (WiFi.status() != WL_CONNECTED && retry < 20) {
        delay(500);
        Serial.print(".");
        retry++;
      }

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[OK] เชื่อมต่อใหม่สำเร็จ!");
        blinkLED(2);  // กะพริบ 2 ครั้งแสดงการ reconnect
      } else {
        Serial.println("\n[FAIL] ไม่สามารถเชื่อมต่อใหม่ได้");
      }
    }
  }
}

// =============================================================================
//  STEP 1: WiFi Scan — สแกนหาเครือข่าย WiFi ในบริเวณใกล้เคียง
// =============================================================================
void stepScan() {
  printDivider('=');
  Serial.println("  STEP 1: WiFi Network Scanner");
  printDivider('=');

  // 1. ตั้งค่าโหมด WiFi เป็น Station (STA) Mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();  // ล้างการเชื่อมต่อเดิมก่อนสแกน
  delay(100);

  Serial.println("กำลังสแกนหาเครือข่าย WiFi...");

  // 2. สแกนเครือข่าย (blocking scan)
  int n = WiFi.scanNetworks();

  if (n == 0) {
    Serial.println("ไม่พบเครือข่าย WiFi ในบริเวณนี้");
  } else {
    Serial.printf("\nพบเครือข่ายทั้งหมด: %d เครือข่าย\n\n", n);
    Serial.printf("%-4s %-32s %-6s %-8s %s\n",
                  "#", "SSID", "CH", "RSSI", "Security");
    printDivider();

    bool targetFound = false;
    int  targetRSSI  = 0;

    for (int i = 0; i < n; i++) {
      // 3. อ่านชื่อ SSID ของเครือข่ายลำดับที่ i
      String ssid = WiFi.SSID(i);

      // 4. อ่านความแรงสัญญาณ RSSI (dBm)
      int rssi = WiFi.RSSI(i);

      // 5. อ่านหมายเลขช่องสัญญาณ (Channel)
      int ch = WiFi.channel(i);

      // 6. ตรวจสอบประเภทความปลอดภัย
      String security = (WiFi.encryptionType(i) == ENC_OPEN) ? "OPEN" : "WPA/WPA2";

      // แสดงผลพร้อม marker ถ้าเจอเครือข่ายเป้าหมาย
      bool isTarget = (ssid == TARGET_SSID);
      if (isTarget) {
        targetFound = true;
        targetRSSI  = rssi;
        Serial.printf("%-4d %-32s %-6d %-8d %s  ◄ TARGET\n",
                      i + 1, ssid.c_str(), ch, rssi, security.c_str());
      } else {
        Serial.printf("%-4d %-32s %-6d %-8d %s\n",
                      i + 1, ssid.c_str(), ch, rssi, security.c_str());
      }
    }

    printDivider();

    // แสดงผลการค้นหาเครือข่ายเป้าหมาย (Challenge Requirement)
    Serial.println();
    if (targetFound) {
      Serial.printf("[FOUND] พบเครือข่ายเป้าหมาย \"%s\" | RSSI = %d dBm\n",
                    TARGET_SSID, targetRSSI);
    } else {
      Serial.printf("[NOT FOUND] ไม่พบเครือข่าย \"%s\" ในบริเวณนี้\n", TARGET_SSID);
    }
  }

  // ล้างผลการสแกนออกจากหน่วยความจำ
  WiFi.scanDelete();
  Serial.println();
}

// =============================================================================
//  STEP 2: WiFi Connect — เชื่อมต่อ WiFi ในโหมด Station (STA)
// =============================================================================
void stepConnect() {
  printDivider('=');
  Serial.println("  STEP 2: WiFi Station Mode — Connect");
  printDivider('=');

  Serial.printf("กำลังเชื่อมต่อกับ \"%s\"", TARGET_SSID);

  // 1. เริ่มต้นการเชื่อมต่อ WiFi ด้วย SSID และ Password
  WiFi.begin(TARGET_SSID, PASSWORD);

  // 2. รอจนกว่าสถานะ WiFi จะเท่ากับ WL_CONNECTED
  int dots = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    dots++;
    // Timeout หลังจาก 30 วินาที
    if (dots > 60) {
      Serial.println("\n[TIMEOUT] ไม่สามารถเชื่อมต่อได้ภายใน 30 วินาที");
      Serial.println("ตรวจสอบ SSID / Password และลองใหม่อีกครั้ง");
      return;
    }
  }

  Serial.println("\n");
  Serial.println("╔══════════════════════════════════════╗");
  Serial.println("║   ✓  เชื่อมต่อ WiFi สำเร็จ!          ║");
  Serial.println("╚══════════════════════════════════════╝");

  // 3. แสดง IP Address ที่ได้รับจาก DHCP Server
  Serial.printf("  SSID      : %s\n", WiFi.SSID().c_str());
  Serial.printf("  IP Address : %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("  Gateway    : %s\n", WiFi.gatewayIP().toString().c_str());
  Serial.printf("  Subnet     : %s\n", WiFi.subnetMask().toString().c_str());

  // 4. แสดงความแรงของสัญญาณ RSSI (dBm)
  Serial.printf("  RSSI       : %d dBm", WiFi.RSSI());
  int rssi = WiFi.RSSI();
  if      (rssi >= -50) Serial.println("  (ดีเยี่ยม)");
  else if (rssi >= -65) Serial.println("  (ดี)");
  else if (rssi >= -75) Serial.println("  (พอใช้)");
  else                  Serial.println("  (อ่อน)");

  // 5. แสดง MAC Address ของบอร์ด
  Serial.printf("  MAC Addr   : %s\n", WiFi.macAddress().c_str());

  printDivider();
  Serial.println();

  // ── CHALLENGE: LED Blink 3 ครั้ง เมื่อเชื่อมต่อสำเร็จ ───────────────────
  Serial.println("[CHALLENGE] เชื่อมต่อสำเร็จ — LED กะพริบ 3 ครั้ง...");
  blinkLED(3, 200, 200);
  Serial.println("[CHALLENGE] LED Blink เสร็จสิ้น");
  Serial.println();

  lastCheckTime = millis();
}

// =============================================================================
//  Utility: LED Blink
//    times  — จำนวนครั้งที่กะพริบ
//    onMs   — ระยะเวลาที่ LED ติด (มิลลิวินาที)
//    offMs  — ระยะเวลาที่ LED ดับ (มิลลิวินาที)
// =============================================================================
void blinkLED(int times, int onMs, int offMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, LED_ON);
    delay(onMs);
    digitalWrite(LED_PIN, LED_OFF);
    if (i < times - 1) delay(offMs);
  }
}

// =============================================================================
//  Utility: Print Divider Line
// =============================================================================
void printDivider(char ch, int len) {
  for (int i = 0; i < len; i++) Serial.print(ch);
  Serial.println();
}
