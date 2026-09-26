#include <Arduino.h>

// =========================================================================
// 🔘 ใบงานที่ 1.1: การควบคุมเอาต์พุตด้วยอินพุตสวิตช์ปุ่มกด (Digital Inputs & Debouncing)
// หลักสูตรการพัฒนาระบบ Hybrid Local/Cloud IoT Node ด้วย ESP32 / ESP8266
// =========================================================================

// --- Pin Definitions ---
#if defined(ESP8266)
  #define BUTTON_PIN      0   // D3 (ปุ่ม FLASH บนบอร์ด AX-WiFi)
  #define FAN_RELAY_PIN  13   // D7 (Relay 1 - พัดลม)
  #define MIST_RELAY_PIN 16   // D0 (Relay 2 - ปั๊มพ่นหมอก)
  #define ONBOARD_LED     2   // D4 (LED Built-in)
#else
  #define BUTTON_PIN      0   // GPIO 0 (ปุ่ม BOOT / SW1 บนบอร์ด ESP32)
  #define FAN_RELAY_PIN   5   // GPIO 5 (Relay 1 - พัดลม)
  #define MIST_RELAY_PIN 23   // GPIO 23 (Relay 2 - ปั๊มพ่นหมอก)
  #define ONBOARD_LED    18   // GPIO 18 (LED Built-in / Indicator)
#endif

// --- Relay Active Mode Configuration ---
// กำหนดเป็น false หากใช้บอร์ดจำลอง Wokwi หรือ Relay Active-HIGH
// กำหนดเป็น true หากใช้บอร์ด Relay Optocoupler ทั่วไปที่เป็น Active-LOW
#define RELAY_ACTIVE_LOW false

#if RELAY_ACTIVE_LOW
  #define RELAY_ON   LOW
  #define RELAY_OFF  HIGH
#else
  #define RELAY_ON   HIGH
  #define RELAY_OFF  LOW
#endif

void setRelay(uint8_t pin, bool state) {
  digitalWrite(pin, state ? RELAY_ON : RELAY_OFF);
}

// --- Global Variables for State Machine & Debounce ---
bool fanRelayState = false;
bool mistRelayState = false;
int toggleCount = 0;

int lastButtonState = HIGH;      // สถานะปุ่มรอบก่อนหน้า (สำหรับตรวจจับการสั่น)
int currentButtonState = HIGH;   // สถานะปุ่มที่ผ่านการกรอง Debounce แล้ว

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;         // เวลากรองสัญญาณกระดอน (50 ms)

unsigned long buttonPressTime = 0;              // เวลาที่เริ่มกดปุ่มลง
const unsigned long longPressDuration = 2000;   // เวลากดค้างเพื่อ Reset (2,000 ms = 2 วินาที)
bool longPressTriggered = false;                // แฟล็กป้องกันการประมวลผลคำสั่ง Reset ซ้ำ

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n=======================================================");
  Serial.println("  ESP32 Lab 1.1: Digital Inputs & Debouncing Started");
  Serial.println("=======================================================");

  // 1. ตั้งค่าโหมดพินสวิตช์เป็น INPUT_PULLUP (Active-LOW)
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // 2. ตั้งค่าโหมดพินรีเลย์และ LED เป็น OUTPUT
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(MIST_RELAY_PIN, OUTPUT);
  pinMode(ONBOARD_LED, OUTPUT);

  // 3. เริ่มต้นด้วยการปิดโหลดทั้งหมดอย่างปลอดภัย
  setRelay(FAN_RELAY_PIN, false);
  setRelay(MIST_RELAY_PIN, false);
  digitalWrite(ONBOARD_LED, LOW);
}

void loop() {
  int reading = digitalRead(BUTTON_PIN);

  // ตรวจสอบว่าสัญญาณเกิดการเปลี่ยนสถานะหรือไม่
  if (reading != lastButtonState) {
    lastDebounceTime = millis(); // บันทึกเวลาที่เกิดการเปลี่ยนแปลง
  }

  // หากสัญญาณนิ่งเกินช่วงเวลา debounceDelay (50ms) ให้ยอมรับค่า
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != currentButtonState) {
      currentButtonState = reading;

      // ตรวจพบอีเวนต์การกดปุ่มลง (Falling Edge: HIGH -> LOW)
      if (currentButtonState == LOW) {
        buttonPressTime = millis();
        longPressTriggered = false;

        // สลับสถานะ Relay 1 (Fan)
        fanRelayState = !fanRelayState;
        setRelay(FAN_RELAY_PIN, fanRelayState);
        digitalWrite(ONBOARD_LED, fanRelayState ? HIGH : LOW);

        // -------------------------------------------------------------
        // 🏆 TODO 1 (Grand Challenge): นับจำนวนครั้งการกดสลับสถานะ
        // toggleCount++;
        // -------------------------------------------------------------

        // -------------------------------------------------------------
        // 🏆 TODO 2 (Grand Challenge): หากกดครบ 3 ครั้ง ให้เปิด Relay 2 (Mist)
        // if (toggleCount >= 3) {
        //   mistRelayState = true;
        //   setRelay(MIST_RELAY_PIN, true);
        // }
        // -------------------------------------------------------------

        // ส่ง Telemetry ไปยังหน้าเว็บ
        Serial.printf("[STATUS] Button: LOW | PressCount: %d | Relay1: %s | Relay2: %s\n",
                      toggleCount,
                      fanRelayState ? "ON" : "OFF",
                      mistRelayState ? "ON" : "OFF");
      } else {
        // เมื่อปล่อยปุ่ม (Rising Edge: LOW -> HIGH)
        Serial.printf("[STATUS] Button: HIGH | PressCount: %d | Relay1: %s | Relay2: %s\n",
                      toggleCount,
                      fanRelayState ? "ON" : "OFF",
                      mistRelayState ? "ON" : "OFF");
      }
    }
  }

  // -----------------------------------------------------------------
  // 🏆 TODO 3 (Grand Challenge): ตรวจจับการกดปุ่มค้าง > 2 วินาที (Long Press Reset)
  // หากผู้ใช้กดปุ่มค้างไว้เกิน 2 วินาที (และยังไม่เคยสั่ง Reset ในรอบการกดนี้):
  // 1. ตั้งค่า longPressTriggered = true;
  // 2. ปิด Relay 1 และ Relay 2 ทั้งหมด (Safe State)
  // 3. รีเซ็ตตัวนับ toggleCount = 0;
  // 4. ส่งข้อความแจ้งเตือนทาง Serial: "[EVENT] Long Press Reset Activated!"
  // -----------------------------------------------------------------
  if (currentButtonState == LOW && !longPressTriggered) {
    if ((millis() - buttonPressTime) > longPressDuration) {
      longPressTriggered = true;
      fanRelayState = false;
      mistRelayState = false;
      setRelay(FAN_RELAY_PIN, false);
      setRelay(MIST_RELAY_PIN, false);
      digitalWrite(ONBOARD_LED, LOW);
      toggleCount = 0;

      Serial.println("\n[EVENT] >>> Safe State Reset via Long Press (2.0s) <<<");
      Serial.printf("[STATUS] Button: HELD | PressCount: %d | Relay1: OFF | Relay2: OFF\n", toggleCount);
    }
  }

  lastButtonState = reading;
  delay(10); // หน่วงเวลาเล็กน้อยเพื่อลดภาระ CPU
}
