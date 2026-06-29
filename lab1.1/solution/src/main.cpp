/**
 * Lab 1.1: Switch Input (Digital Inputs & Relays - Complete Code Solution)
 * Features:
 *  - Interfacing Push Button switch with software debounce.
 *  - Toggle Relay 1 (Fan) on button press.
 *  - Challenge: 
 *    - Keeps count of button toggles. On the 3rd press, turns ON Relay 2 (Pump/Mist).
 *    - Detects Long Press (held down > 2000ms) to shut down and reset both relays.
 *  - Compatible with both ESP32 and ESP8266 platforms.
 */

#include <Arduino.h>

#if defined(ESP8266)
#define BUTTON_PIN 0       // D3/GPIO 0 (ปุ่ม FLASH บนบอร์ด AX-WiFi)
#define FAN_RELAY_PIN 13   // D7/GPIO 13 สำหรับ AX-WiFi
#define MIST_RELAY_PIN 16  // D0/GPIO 16 สำหรับ AX-WiFi
#elif defined(ESP32)
#define BUTTON_PIN 0       // GPIO 0 (ปุ่ม SW1 บนบอร์ด IPST-WiFi)
#define FAN_RELAY_PIN 5    // พอร์ต 5 สำหรับ IPST-WiFi
#define MIST_RELAY_PIN 23   // พอร์ต 23 สำหรับ IPST-WiFi
#endif

// Button Debounce & State Variables
bool lastButtonState = HIGH;
bool currentButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50; // 50ms debounce time

// Long Press Timing
unsigned long buttonPressTime = 0;
unsigned long longPressDuration = 2000; // 2 seconds
bool longPressTriggered = false;

// Relay and count states
bool relayState = false;
bool mistRelayState = false;
int toggleCount = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // 1. ตั้งค่า INPUT_PULLUP สำหรับสลับสวิตช์ปุ่มกด และ OUTPUT สำหรับขารีเลย์ทั้งสองขา
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(MIST_RELAY_PIN, OUTPUT);
  
  // ปิดรีเลย์เริ่มต้น
  digitalWrite(FAN_RELAY_PIN, LOW);
  digitalWrite(MIST_RELAY_PIN, LOW);
  
  Serial.println("Lab 1.1 Switch Input Solution Booted.");
}

void loop() {
  int reading = digitalRead(BUTTON_PIN);
  
  // ตรวจสอบการสั่นหน้าสัมผัส (Debounce Check)
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // หากผ่านพ้น debounceDelay ไปแล้ว และสถานะปุ่มเปลี่ยนไปจริง
    if (reading != currentButtonState) {
      currentButtonState = reading;
      
      // ตรวจพบอีเวนต์ "กดสวิตช์ลง" (Active LOW เนื่องจากเชื่อมต่อภายในแบบ Pull-up)
      if (currentButtonState == LOW) {
        buttonPressTime = millis();
        longPressTriggered = false;
        
        // 2. การทดลองแบบ Switch Toggle (สลับสถานะของ Relay 1)
        relayState = !relayState;
        digitalWrite(FAN_RELAY_PIN, relayState ? HIGH : LOW);
        
        // เพิ่มจำนวนการสลับสถานะ (Toggle Count) สำหรับโจทย์ท้าทาย
        toggleCount++;
        Serial.printf("Button Pressed! Toggle Count = %d, Fan State = %s\n", 
                      toggleCount, relayState ? "ON" : "OFF");
        
        // โจทย์ท้าทาย: เมื่อกด Toggle ครบ 3 ครั้ง ให้เปิดการทำงานของ Relay 2 (GPIO 14)
        if (toggleCount == 3) {
          mistRelayState = true;
          digitalWrite(MIST_RELAY_PIN, HIGH);
          Serial.println("Challenge Triggered: Mist Relay 2 turned ON!");
        }
      }
    }
  }
  
  // 3. ตรวจจับการกดปุ่มแช่/ค้าง (Press Long Switch) มากกว่า 2 วินาที
  if (currentButtonState == LOW && !longPressTriggered) {
    if ((millis() - buttonPressTime) > longPressDuration) {
      longPressTriggered = true; // ตั้งสถานะป้องกันการทำงานซ้ำในขอบเขตการกดรอบนี้
      
      // โจทย์ท้าทาย: เมื่อกดค้างครบ 2 วินาที ให้สั่ง Reset ปิดรีเลย์ทุกตัว และล้างค่าจำนวนกด
      relayState = false;
      mistRelayState = false;
      toggleCount = 0;
      
      digitalWrite(FAN_RELAY_PIN, LOW);
      digitalWrite(MIST_RELAY_PIN, LOW);
      
      Serial.println("System Reset via Long Press! Both Relays turned OFF. Counter cleared.");
    }
  }
  
  lastButtonState = reading;
}
