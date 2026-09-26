/**
 * Lab 1: ESP32 Smart Greenhouse Simulation (Starter Template)
 * PlatformIO + Wokwi for VS Code Template
 * 
 * คำแนะนำสำหรับผู้เรียน:
 * 1. ศึกษาโครงสร้างโค้ดและทดสอบอ่านค่าเซ็นเซอร์
 * 2. เติมโค้ดตรรกะควบคุมในส่วน TODO 1 - TODO 4 ให้สมบูรณ์ตามโจทย์ท้าทาย
 * 3. กด Build / Run Simulation บน VS Code เพื่อทดสอบการทำงาน
 */

#include <Arduino.h>
#include <DHT.h>

// ==========================================
// 1. Sensor & Pin Configuration
// ==========================================
#define USE_DHT22    // เลือกระหว่าง USE_DHT22 หรือ USE_DHT11
#if defined(USE_DHT22)
  #define DHTTYPE DHT22
#else
  #define DHTTYPE DHT11
#endif

#define DHTPIN 33
#define ANALOG_PIN 36
#define FAN_RELAY_PIN 5
#define MIST_RELAY_PIN 23
#define ALERT_LED_PIN 18

// ==========================================
// 2. Relay Active Logic Configuration
// ==========================================
// กำหนดระดับลอจิกการทำงานของโมดูลรีเลย์ (Active-HIGH vs Active-LOW):
// - สำหรับโมดูลแบบ Active-HIGH (Wokwi default): RELAY_ACTIVE_LOW = false (HIGH = ทำงาน, LOW = ดับ)
// - สำหรับโมดูลแบบ Active-LOW (โมดูล Optocoupler ทั่วไป): RELAY_ACTIVE_LOW = true (LOW = ทำงาน, HIGH = ดับ)
#define RELAY_ACTIVE_LOW false  // กำหนดเป็น true เมื่อต่อใช้งานจริงกับบอร์ด Active-LOW

#if RELAY_ACTIVE_LOW
  #define RELAY_ON   LOW
  #define RELAY_OFF  HIGH
#else
  #define RELAY_ON   HIGH
  #define RELAY_OFF  LOW
#endif

// ฟังก์ชันสำหรับกำหนดสถานะรีเลย์ (Relay Control Helper Function)
void setRelay(uint8_t pin, bool state) {
  digitalWrite(pin, state ? RELAY_ON : RELAY_OFF);
}

DHT dht(DHTPIN, DHTTYPE);

float temperature = 0;
float humidity = 0;
int rawAnalog = 0;
float analogPercent = 0;
bool fanState = false;
bool mistState = false;

unsigned long lastReadTime = 0;
const unsigned long READ_INTERVAL = 2000;

void setup() {
  Serial.begin(115200);
  
  // กำหนดโหมดขาพิน
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(MIST_RELAY_PIN, OUTPUT);
  pinMode(ALERT_LED_PIN, OUTPUT);
  
  // กำหนดสถานะเริ่มต้นที่ปลอดภัย (Fail-Safe State: ปิดโหลดทั้งหมด)
  setRelay(FAN_RELAY_PIN, false);
  setRelay(MIST_RELAY_PIN, false);
  digitalWrite(ALERT_LED_PIN, LOW);

  pinMode(ANALOG_PIN, INPUT);
  #if defined(ESP32)
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);
  #endif

  dht.begin();
  Serial.println("=================================================");
  Serial.println(" Wokwi for VS Code Simulation: Lab 1 ESP32");
  Serial.printf(" Relay Mode: %s\n", RELAY_ACTIVE_LOW ? "Active-LOW" : "Active-HIGH");
  Serial.println("=================================================");
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastReadTime >= READ_INTERVAL) {
    lastReadTime = currentMillis;

    // อ่านค่าจากเซ็นเซอร์
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    rawAnalog = analogRead(ANALOG_PIN);

    // -------------------------------------------------------------
    // TODO 1: ตรวจสอบความถูกต้องของเซ็นเซอร์ (Fail-Safe Protection)
    // -------------------------------------------------------------
    // คำสั่งแนะนำ: ใช้ isnan(temperature) || isnan(humidity)
    // เงื่อนไข: หากอ่านค่าผิดพลาด ให้เปิดไฟ ALERT_LED, ปิดรีเลย์ทั้ง 2 ตัวเพื่อความปลอดภัย และ return;
    /*
    if ( ... ) {
      // เขียนโค้ด Fail-Safe ที่นี่
      return;
    }
    */

    // -------------------------------------------------------------
    // TODO 2: แปลงค่าแอนะล็อกดิบ (0-4095) ให้เป็นเปอร์เซ็นต์ (0.0 - 100.0%)
    // -------------------------------------------------------------
    // ระวัง: ปัญหา Integer Division ในภาษา C/C++
    analogPercent = 0.0f; // แก้ไขสูตรคำนวณที่นี่

    // -------------------------------------------------------------
    // TODO 3: ควบคุมพัดลมระบายความร้อนแบบ Hysteresis
    // -------------------------------------------------------------
    // เงื่อนไข:
    // - หาก temperature >= 30.0 C และพัดลมยังปิดอยู่ -> เปิดพัดลม (fanState = true)
    // - หาก temperature <= 29.5 C และพัดลมเปิดอยู่ -> ปิดพัดลม (fanState = false)
    // สั่งงานรีเลย์ด้วยคำสั่ง: setRelay(FAN_RELAY_PIN, fanState);
    

    // -------------------------------------------------------------
    // TODO 4: ควบคุมปั๊มพ่นหมอกตามระดับความชื้นสัมพัทธ์
    // -------------------------------------------------------------
    // เงื่อนไข:
    // - หาก humidity <= 50.0% และปั๊มยังปิดอยู่ -> เปิดปั๊มพ่นหมอก (mistState = true)
    // - หาก humidity >= 60.0% และปั๊มเปิดอยู่ -> ปิดปั๊มพ่นหมอก (mistState = false)
    // สั่งงานรีเลย์ด้วยคำสั่ง: setRelay(MIST_RELAY_PIN, mistState);
    

    // แสดงผลข้อมูลทาง Serial Monitor
    Serial.printf("[STATUS] Temp: %.1f C | Hum: %.1f%% | ADC: %.1f%% (Raw: %d) | Fan: %s | Mist: %s\n",
                  temperature, humidity, analogPercent, rawAnalog,
                  fanState ? "ON" : "OFF", mistState ? "ON" : "OFF");
  }
}
