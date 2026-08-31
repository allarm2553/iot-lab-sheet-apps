/**
 * Lab 1.1: Digital Inputs, Software Debounce & Relays (Complete Code Solution)
 * 
 * Features:
 *  - Interfacing Push Button switch with software debounce (non-blocking millis).
 *  - Single-click toggle Relay 1 (Fan) on button press.
 *  - Challenge: 
 *    - Keeps count of button toggles. On the 3rd press, turns ON Relay 2 (Pump/Mist).
 *    - Detects Long Press (held down > 2000ms) to shut down and reset all loads.
 *  - Macro abstraction (RELAY_ON / RELAY_OFF) for Active-HIGH / Active-LOW compatibility.
 *  - Cross-platform support (ESP32 IPST-WiFi & ESP8266 AX-WiFi).
 */

#include <Arduino.h>

// ================================================================
// 🔌 1. การกำหนดขาพินฮาร์ดแวร์ตามชนิดบอร์ด (Pin Definitions)
// ================================================================
#if defined(ESP8266)
#define BOARD_NAME      "ESP8266 (AX-WiFi / NodeMCU)"
#define BUTTON_PIN      0            // D3 / GPIO 0 (ปุ่ม FLASH บนบอร์ด)
#define FAN_RELAY_PIN   13           // D7 / GPIO 13 สำหรับ AX-WiFi
#define MIST_RELAY_PIN  16           // D0 / GPIO 16 สำหรับ AX-WiFi
#define ALERT_LED_PIN   2            // Onboard LED (GPIO 2 / D4)
#elif defined(ESP32)
#define BOARD_NAME      "ESP32 (IPST-WiFi / DevKit)"
#define BUTTON_PIN      0            // GPIO 0 (ปุ่ม SW1 บนบอร์ด หรือ Push Button)
#define FAN_RELAY_PIN   5            // พอร์ต 5 สำหรับ IPST-WiFi
#define MIST_RELAY_PIN  23           // พอร์ต 23 สำหรับ IPST-WiFi
#define ALERT_LED_PIN   18           // Onboard LED (GPIO 18)
#endif

// Macro ป้องกันความสับสนเรื่อง Active-HIGH vs Active-LOW
#define RELAY_ON        HIGH
#define RELAY_OFF       LOW

// Button Debounce & State Variables
bool lastButtonState = HIGH;
bool currentButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 50; // 50ms debounce time

// Long Press Timing
unsigned long buttonPressTime = 0;
const unsigned long LONG_PRESS_DURATION = 2000; // 2 วินาที
bool longPressTriggered = false;

// Relay and count states
bool fanState = false;
bool mistState = false;
int toggleCount = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  
  // 1. ตั้งค่า INPUT_PULLUP สำหรับสวิตช์ปุ่มกด และ OUTPUT สำหรับขารีเลย์
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(MIST_RELAY_PIN, OUTPUT);
  pinMode(ALERT_LED_PIN, OUTPUT);
  
  // กำหนดสถานะเริ่มต้นที่ปลอดภัย (Safe Initial State)
  digitalWrite(FAN_RELAY_PIN, RELAY_OFF);
  digitalWrite(MIST_RELAY_PIN, RELAY_OFF);
  digitalWrite(ALERT_LED_PIN, LOW);
  
  Serial.println("=================================================");
  Serial.println(" Lab 1.1: Digital Inputs & Debouncing Initialized");
  Serial.printf(" Board : %s\n", BOARD_NAME);
  Serial.printf(" Button: GPIO %d (INPUT_PULLUP)\n", BUTTON_PIN);
  Serial.printf(" Relays: Fan=GPIO %d, Mist=GPIO %d\n", FAN_RELAY_PIN, MIST_RELAY_PIN);
  Serial.println("=================================================");
}

void loop() {
  int reading = digitalRead(BUTTON_PIN);
  
  // 1. ตรวจสอบการสั่นหน้าสัมผัส (Debounce Check)
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    // หากผ่านพ้น DEBOUNCE_DELAY ไปแล้ว และสถานะปุ่มเปลี่ยนไปจริง
    if (reading != currentButtonState) {
      currentButtonState = reading;
      
      // ตรวจพบอีเวนต์ "กดสวิตช์ลง" (Falling Edge / Active LOW)
      if (currentButtonState == LOW) {
        buttonPressTime = millis();
        longPressTriggered = false;
        
        // 2. การสลับสถานะของ Relay 1 (Fan Toggle)
        fanState = !fanState;
        digitalWrite(FAN_RELAY_PIN, fanState ? RELAY_ON : RELAY_OFF);
        
        // เพิ่มจำนวนครั้งการกดสวิตช์
        toggleCount++;
        Serial.printf(">> [BUTTON CLICK] Toggle #%d | Fan Relay: %s\n", 
                      toggleCount, fanState ? "ON" : "OFF");
        
        // โจทย์ท้าทาย: เมื่อกด Toggle ครบ 3 ครั้ง ให้เปิดการทำงานของ Relay 2 (Mist Pump)
        if (toggleCount == 3) {
          mistState = true;
          digitalWrite(MIST_RELAY_PIN, RELAY_ON);
          Serial.println(">> [CHALLENGE TRIGGERED] กดครบ 3 ครั้ง -> เปิดปั๊มพ่นหมอก (Mist Relay ON)");
        }
      }
    }
  }
  
  // 3. ตรวจจับการกดปุ่มค้าง (Long Press Detection > 2 วินาที)
  if (currentButtonState == LOW && !longPressTriggered) {
    if ((millis() - buttonPressTime) > LONG_PRESS_DURATION) {
      longPressTriggered = true; // ป้องกันการทำงานซ้ำจนกว่าจะปล่อยปุ่ม
      
      // เมื่อกดค้างครบ 2 วินาที: สั่ง Safe Reset ปิดรีเลย์ทุกตัว และล้างตัวนับ
      fanState = false;
      mistState = false;
      toggleCount = 0;
      
      digitalWrite(FAN_RELAY_PIN, RELAY_OFF);
      digitalWrite(MIST_RELAY_PIN, RELAY_OFF);
      
      // กระพริบไฟเตือน Reset
      digitalWrite(ALERT_LED_PIN, HIGH);
      delay(150);
      digitalWrite(ALERT_LED_PIN, LOW);
      
      Serial.println(">> [LONG PRESS DETECTED] ระบบถูกรีเซ็ต: ปิดโหลดทั้งหมด และรีเซ็ตตัวนับเป็น 0");
    }
  }
  
  lastButtonState = reading;
}
