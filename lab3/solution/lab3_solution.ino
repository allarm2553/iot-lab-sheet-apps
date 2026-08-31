/**
 * =====================================================================
 *  Lab 3 — Solution Code (Arduino IDE Sketch .ino)
 *  ชื่อ: การเชื่อมต่อ WiFi เบื้องต้น (WiFi Scan & Station Mode with Event-Driven Auto-Reconnect)
 * =====================================================================
 *
 *  คุณสมบัติเด่น (Key Features):
 *  ► STEP 1 — WiFi Scan & Parameters Analysis
 *      สแกนหาเครือข่ายทั้งหมด แสดง SSID, RSSI (dBm), Channel, Security
 *      พร้อมค้นหาเครือข่ายเป้าหมาย TARGET_SSID
 *
 *  ► STEP 2 — WiFi Parameters & Station Connect
 *      ทำความเข้าใจสถานะการเชื่อมต่อ (WL_CONNECTED, WL_DISCONNECTED, ฯลฯ)
 *      แสดง IP, Subnet, Gateway, DNS, RSSI, BSSID และ MAC Address
 *
 *  ► STEP 3 — Event-Driven Auto-Reconnect (ฟังก์ชันพิเศษไม่ใช้การ Polling ใน loop)
 *      ใช้สถาปัตยกรรม Callback / Event Handler:
 *        - ESP32  : WiFi.onEvent() ดักจับ ARDUINO_EVENT_WIFI_STA_DISCONNECTED & GOT_IP
 *        - ESP8266: WiFi.onStationModeDisconnected() & onStationModeGotIP()
 *      ระบบจะเชื่อมต่อใหม่โดยอัตโนมัติในระดับ System Task ทันทีที่สัญญาณหลุด
 *      ทำให้ loop() ทำงานได้อิสระ ไม่ต้องคอยเช็ค if (WiFi.status() != WL_CONNECTED)
 *
 *  Board Support:
 *      - ESP32   : IPST-WiFi, ESP32 DevKitC
 *      - ESP8266 : AX-WiFi, NodeMCU v2
 * =====================================================================
 */

#include <Arduino.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #define LED_PIN       2       // GPIO 2 (SLED บน AX-WiFi, Active LOW)
  #define LED_ON        LOW
  #define LED_OFF       HIGH
  #define ENC_OPEN      ENC_TYPE_NONE
  WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;
#elif defined(ESP32)
  #include <WiFi.h>
  #define LED_PIN       2       // GPIO 2 (LED Built-in บน ESP32 DevKit, Active HIGH)
  #define LED_ON        HIGH
  #define LED_OFF       LOW
  #define ENC_OPEN      WIFI_AUTH_OPEN
#endif

// ─── WiFi Credentials (เปลี่ยนเป็น "Wokwi-GUEST" สำหรับ Wokwi Simulation) ─────
const char* TARGET_SSID = "iot_512";    // หรือ "Wokwi-GUEST"
const char* PASSWORD    = "iot123456";   // หรือ "" (Wokwi ไม่มีรหัสผ่าน)

// ─── Forward Declarations ────────────────────────────────────────────────────
void stepScan();
void stepConnect();
void initWiFiEvents();
void blinkLED(int times, int onMs = 150, int offMs = 150);
void printDivider(char ch = '-', int len = 60);
String getStatusDescription(wl_status_t status);

// =============================================================================
//  HELPER: แปลงรหัสสถานะ WiFi.status() เป็นข้อความอธิบายความหมาย
// =============================================================================
String getStatusDescription(wl_status_t status) {
  switch (status) {
    case WL_CONNECTED:       return "WL_CONNECTED (3): เชื่อมต่อเครือข่ายสำเร็จและได้รับ IP";
    case WL_NO_SHIELD:       return "WL_NO_SHIELD (255): ไม่พบโมดูล Wi-Fi หรือฮาร์ดแวร์ขัดข้อง";
    case WL_IDLE_STATUS:     return "WL_IDLE_STATUS (0): อยู่ในสถานะพัก/กำลังเปลี่ยนโหมด";
    case WL_NO_SSID_AVAIL:   return "WL_NO_SSID_AVAIL (1): ไม่พบชื่อ SSID ที่กำหนดในระยะสัญญาณ";
    case WL_SCAN_COMPLETED:  return "WL_SCAN_COMPLETED (2): การสแกนหาเครือข่ายเสร็จสมบูรณ์";
    case WL_CONNECT_FAILED:  return "WL_CONNECT_FAILED (4): การเชื่อมต่อล้มเหลว (เช่น รหัสผ่านผิด)";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST (5): สัญญาณเครือข่ายขาดหาย";
    case WL_DISCONNECTED:    return "WL_DISCONNECTED (6): ตัดการเชื่อมต่อจาก Access Point";
    default:                 return "UNKNOWN_STATUS: ไม่ทราบสถานะ (" + String((int)status) + ")";
  }
}

// =============================================================================
//  LED HELPER
// =============================================================================
void blinkLED(int times, int onMs, int offMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, LED_ON);
    delay(onMs);
    digitalWrite(LED_PIN, LED_OFF);
    if (i < times - 1) delay(offMs);
  }
}

void printDivider(char ch, int len) {
  for (int i = 0; i < len; i++) Serial.print(ch);
  Serial.println();
}

// =============================================================================
//  EVENT-DRIVEN WIFI HANDLER (ระบบตอบสนองอัตโนมัติเมื่อเกิดเหตุการณ์)
// =============================================================================
#if defined(ESP32)
void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_START:
      Serial.println("[WiFi Event] Station Interface Started (พร้อมทำงาน)");
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.println("[WiFi Event] เชื่อมต่อกับ Access Point สำเร็จ! กำลังขอ IP ผ่าน DHCP...");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.printf("[WiFi Event] ได้รับหมายเลข IP Address: %s\n", WiFi.localIP().toString().c_str());
      blinkLED(3);
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("[WiFi Event ⚠️] สัญญาณ Wi-Fi หลุด! (Event-Driven Auto-Reconnect กำลังเชื่อมต่อใหม่ใน Background...)");
      WiFi.reconnect();
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
    Serial.printf("[WiFi Event] ได้รับหมายเลข IP Address: %s\n", WiFi.localIP().toString().c_str());
    blinkLED(3);
  });

  disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event) {
    Serial.println("[WiFi Event ⚠️] สัญญาณ Wi-Fi หลุด! (Event-Driven Auto-Reconnect กำลังเชื่อมต่อใหม่ใน Background...)");
    WiFi.reconnect();
  });
  #endif

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}

// =============================================================================
//  STEP 1: WIFI SCAN
// =============================================================================
void stepScan() {
  printDivider('=', 60);
  Serial.println("  STEP 1: การสแกนหาเครือข่าย Wi-Fi (WiFi Network Scanner)");
  printDivider('=', 60);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("[SCAN] กำลังเริ่มการสแกนหาเครือข่ายไร้สาย...");
  int n = WiFi.scanNetworks();

  if (n == 0) {
    Serial.println("[SCAN] ไม่พบเครือข่าย Wi-Fi ใดๆ ในระยะสัญญาณ");
  } else {
    Serial.printf("[SCAN] สแกนเสร็จสิ้น! พบทั้งหมด %d เครือข่าย:\n\n", n);
    Serial.printf("%-4s | %-24s | %-8s | %-4s | %-12s\n", "No.", "SSID", "RSSI", "CH", "Security");
    printDivider('-', 60);

    bool foundTarget = false;
    for (int i = 0; i < n; i++) {
      String secType = "OPEN";
      #if defined(ESP8266)
      if (WiFi.encryptionType(i) != ENC_TYPE_NONE) secType = "ENCRYPTED";
      #elif defined(ESP32)
      if (WiFi.encryptionType(i) != WIFI_AUTH_OPEN) secType = "ENCRYPTED";
      #endif

      Serial.printf("[%02d] | %-24s | %4d dBm | %-4d | %-12s\n",
                    i + 1,
                    WiFi.SSID(i).c_str(),
                    WiFi.RSSI(i),
                    WiFi.channel(i),
                    secType.c_str());

      if (WiFi.SSID(i) == TARGET_SSID) foundTarget = true;
    }
    printDivider('-', 60);

    if (foundTarget) {
      Serial.printf("[SCAN] ✓ พบเครือข่ายเป้าหมาย \"%s\" พร้อมทำการเชื่อมต่อ\n", TARGET_SSID);
    } else {
      Serial.printf("[SCAN] ⚠️ ไม่พบเครือข่ายเป้าหมาย \"%s\"\n", TARGET_SSID);
    }
  }
  Serial.println();
}

// =============================================================================
//  STEP 2: WIFI CONNECT & PARAMETERS DISPLAY
// =============================================================================
void stepConnect() {
  printDivider('=', 60);
  Serial.println("  STEP 2: การเชื่อมต่อ Wi-Fi และแสดงพารามิเตอร์เครือข่าย");
  printDivider('=', 60);

  Serial.printf("[CONNECT] กำลังเชื่อมต่อ SSID: \"%s\" ...\n", TARGET_SSID);
  Serial.printf("[STATUS ก่อนต่อ] %s\n", getStatusDescription(WiFi.status()).c_str());

  WiFi.begin(TARGET_SSID, PASSWORD);

  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 30) {
    delay(500);
    Serial.print(".");
    timeout++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[CONNECT] ✓ เชื่อมต่อ Wi-Fi สำเร็จ!");
    printDivider('-', 60);
    Serial.printf("  1. สถานะเครือข่าย (Status)      : %s\n", getStatusDescription(WiFi.status()).c_str());
    Serial.printf("  2. หมายเลข IP (Local IP)        : %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("  3. ซับเน็ตมาสก์ (Subnet Mask)    : %s\n", WiFi.subnetMask().toString().c_str());
    Serial.printf("  4. เกตเวย์ (Gateway IP)          : %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("  5. เซิร์ฟเวอร์ DNS (Primary DNS) : %s\n", WiFi.dnsIP().toString().c_str());
    Serial.printf("  6. ความแรงสัญญาณ (RSSI)         : %d dBm\n", WiFi.RSSI());
    Serial.printf("  7. หมายเลข MAC (MAC Address)    : %s\n", WiFi.macAddress().c_str());
    Serial.printf("  8. Access Point BSSID           : %s\n", WiFi.BSSIDstr().c_str());
    printDivider('-', 60);
  } else {
    Serial.printf("\n[CONNECT] ✗ เชื่อมต่อไม่สำเร็จ! สถานะ: %s\n", getStatusDescription(WiFi.status()).c_str());
  }
}

// =============================================================================
//  SETUP
// =============================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF);

  Serial.println("\n============================================================");
  Serial.println("  IoT Lab 3: WiFi Parameters & Event-Driven Auto-Reconnect");
  Serial.println("============================================================");

  // 1. ลงทะเบียน Event-Driven Auto-Reconnect
  initWiFiEvents();

  // 2. ดำเนินการ Step 1: Scan
  stepScan();

  // 3. ดำเนินการ Step 2: Connect
  stepConnect();
}

// =============================================================================
//  LOOP
// =============================================================================
void loop() {
  static unsigned long lastLog = 0;
  if (millis() - lastLog >= 10000) {
    lastLog = millis();
    Serial.printf("[MAIN TASK Running...] Free Heap: %d Bytes | WiFi Status: %d (%s)\n",
                  ESP.getFreeHeap(),
                  (int)WiFi.status(),
                  WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
  }
  delay(10);
}
