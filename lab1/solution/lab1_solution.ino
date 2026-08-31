/**
 * Lab 1: GPIO, ADC & Relays (Arduino IDE Sketch Solution)
 * 
 * Features:
 *  - Configurable DHT Sensor Selection (DHT11 / DHT22)
 *  - Non-blocking Sensor Read Timer using millis()
 *  - Reading DHT temperature and humidity with isnan() check
 *  - Reading Analog Voltage via ADC with floating-point calculation
 *  - Relay Macro abstraction (RELAY_ON / RELAY_OFF)
 *  - Hysteresis control for Fan (Turn ON >= 30.0 C, Turn OFF <= 29.5 C)
 *  - Humidity control for Mist Pump (Turn ON <= 50%, Turn OFF >= 60%)
 *  - Fail-Safe Action: Safe-state shutdown on sensor disconnection/error
 */

#include <Arduino.h>
#include <DHT.h>

// ================================================================
// ⚙️ 1. เลือกชนิดเซ็นเซอร์ DHT ที่ใช้งาน (DHT11 / DHT22 Selection)
// ================================================================
#define USE_DHT22    // [เลือกใช้ DHT22] สำหรับ Wokwi Simulation หรือโมดูลสีขาว AM2302
// #define USE_DHT11 // [เลือกใช้ DHT11] สำหรับชุดทดลองบอร์ดจริงที่มีโมดูลสีฟ้า

#if defined(USE_DHT22)
  #define DHTTYPE      DHT22
  #define SENSOR_NAME  "DHT22"
#elif defined(USE_DHT11)
  #define DHTTYPE      DHT11
  #define SENSOR_NAME  "DHT11"
#else
  #define DHTTYPE      DHT22
  #define SENSOR_NAME  "DHT22 (Default)"
#endif

// ================================================================
// 🔌 2. การกำหนดขาพินฮาร์ดแวร์ตามชนิดบอร์ด (Pin Definitions)
// ================================================================
#if defined(ESP8266)
#define BOARD_NAME      "ESP8266 (AX-WiFi / NodeMCU)"
#define DHTPIN          2            // D3 / GPIO 0 สำหรับ AX-WiFi
#define ANALOG_PIN      A0           // A0 (ตัวต้านทานปรับค่าได้ VR สำหรับ AX-WiFi)
#define FAN_RELAY_PIN   13           // D7 / GPIO 13 สำหรับ AX-WiFi
#define MIST_RELAY_PIN  16           // D0 / GPIO 16 สำหรับ AX-WiFi
#define ALERT_LED_PIN   2            // Onboard LED (GPIO 2 / D4)
#define ADC_RESOLUTION  1023.0f
#elif defined(ESP32)
#define BOARD_NAME      "ESP32 (IPST-WiFi / DevKit)"
#define DHTPIN          33           // พอร์ต 33 สำหรับ IPST-WiFi
#define ANALOG_PIN      36           // GPIO 36 / KNOB-S สำหรับ IPST-WiFi
#define FAN_RELAY_PIN   5            // พอร์ต 5 สำหรับ IPST-WiFi
#define MIST_RELAY_PIN  23           // พอร์ต 23 สำหรับ IPST-WiFi
#define ALERT_LED_PIN   18           // Onboard LED (GPIO 18)
#define ADC_RESOLUTION  4095.0f
#endif

// Macro ป้องกันความสับสนเรื่อง Active-HIGH vs Active-LOW
#define RELAY_ON        HIGH
#define RELAY_OFF       LOW

DHT dht(DHTPIN, DHTTYPE);

// ตัวแปรสถานะ
float temperature = 0.0f;
float humidity = 0.0f;
int rawAnalog = 0;
float analogPercent = 0.0f;

bool fanState = false;
bool mistState = false;
int sensorErrorCounter = 0;
const int MAX_SENSOR_ERRORS = 3;

unsigned long lastSampleTime = 0;
const unsigned long SAMPLE_INTERVAL = 2000; // อ่านค่าทุก 2 วินาที

void setSafeState(const char* reason) {
  fanState = false;
  mistState = false;
  digitalWrite(FAN_RELAY_PIN, RELAY_OFF);
  digitalWrite(MIST_RELAY_PIN, RELAY_OFF);
  digitalWrite(ALERT_LED_PIN, HIGH); // แจ้งเตือนผ่าน LED
  Serial.printf("[FAIL-SAFE ACTIVATED] %s -> โหลดทั้งหมดถูกสั่งปิดเพื่อความปลอดภัย!\n", reason);
}

void setup() {
  Serial.begin(115200);
  
  // 1. ตั้งค่าโหมดของขารีเลย์และไฟเตือน
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(MIST_RELAY_PIN, OUTPUT);
  pinMode(ALERT_LED_PIN, OUTPUT);
  
  // กำหนดสถานะเริ่มต้นที่ปลอดภัย (Safe Initial State)
  digitalWrite(FAN_RELAY_PIN, RELAY_OFF);
  digitalWrite(MIST_RELAY_PIN, RELAY_OFF);
  digitalWrite(ALERT_LED_PIN, LOW);
  
  // 2. ตั้งค่าขา Analog ADC (ช่วงแรงดัน 0-3.3V)
  pinMode(ANALOG_PIN, INPUT);
  #if defined(ESP32)
  analogSetAttenuation(ADC_11db); // ตั้งค่าความไวสัญญาณ 0-3.3V (รองรับ ESP32 Core 3.x)
  analogReadResolution(12);       // ความละเอียด 12-bit (0-4095)
  #endif
  
  dht.begin();
  Serial.println("=================================================");
  Serial.println(" Lab 1: Fault-Tolerant IoT Node Initialized");
  Serial.printf(" Board : %s\n", BOARD_NAME);
  Serial.printf(" Sensor: %s (Pin GPIO %d)\n", SENSOR_NAME, DHTPIN);
  Serial.println("=================================================");
}

void loop() {
  unsigned long now = millis();

  // Non-blocking timer: อ่านค่าทุก 2 วินาที
  if (now - lastSampleTime >= SAMPLE_INTERVAL) {
    lastSampleTime = now;

    // อ่านค่าจากเซ็นเซอร์ DHT และ ADC
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    rawAnalog = analogRead(ANALOG_PIN);

    // ตรวจสอบความถูกต้องในการอ่านเซ็นเซอร์ (Error Handling)
    if (isnan(temperature) || isnan(humidity)) {
      sensorErrorCounter++;
      Serial.printf("[WARN #%d] ไม่สามารถอ่านค่าจาก %s ได้! กรุณาตรวจสายสัญญาณ\n", sensorErrorCounter, SENSOR_NAME);
      
      if (sensorErrorCounter >= MAX_SENSOR_ERRORS) {
        setSafeState("Sensor Read Failure (NaN detected >= 3 times)");
      }
      return; // ข้ามการประมวลผลรอบนี้
    }

    // เมื่ออ่านค่าเซ็นเซอร์ได้ปกติ ให้รีเซ็ตข้อผิดพลาด
    sensorErrorCounter = 0;
    digitalWrite(ALERT_LED_PIN, LOW);

    // 3. แปลงค่าดิบ ADC เป็นเปอร์เซ็นต์ด้วย Floating-point division ป้องกัน integer division bug
    analogPercent = ((float)rawAnalog / ADC_RESOLUTION) * 100.0f;

    // 4. ระบบควบคุมพัดลมด้วย Hysteresis (ป้องกัน Relay Chatter)
    // เปิดเมื่อ Temp >= 30.0 C, ปิดเมื่อ Temp <= 29.5 C
    if (temperature >= 30.0f && !fanState) {
      fanState = true;
      digitalWrite(FAN_RELAY_PIN, RELAY_ON);
      Serial.println(">> [CONTROL] อุณหภูมิสูง (>= 30.0 C) -> เปิดพัดลมระบายความร้อน");
    } else if (temperature <= 29.5f && fanState) {
      fanState = false;
      digitalWrite(FAN_RELAY_PIN, RELAY_OFF);
      Serial.println(">> [CONTROL] อุณหภูมิลดสู่เกณฑ์ปลอดภัย (<= 29.5 C) -> ปิดพัดลม");
    }

    // 5. ระบบควบคุมปั๊มพ่นหมอกตามความชื้น (Humidity Hysteresis)
    // เปิดเมื่อ Humidity <= 50.0%, ปิดเมื่อ Humidity >= 60.0%
    if (humidity <= 50.0f && !mistState) {
      mistState = true;
      digitalWrite(MIST_RELAY_PIN, RELAY_ON);
      Serial.println(">> [CONTROL] ความชื้นต่ำ (<= 50.0%) -> เปิดปั๊มพ่นหมอก");
    } else if (humidity >= 60.0f && mistState) {
      mistState = false;
      digitalWrite(MIST_RELAY_PIN, RELAY_OFF);
      Serial.println(">> [CONTROL] ความชื้นเหมาะสม (>= 60.0%) -> ปิดปั๊มพ่นหมอก");
    }

    // พิมพ์รายงานสถานะ
    Serial.printf("[STATUS] %s: Temp=%.1f C | Hum=%.1f%% | ADC=%.1f%% (Raw: %d) | Fan=%s | Mist=%s\n",
                  SENSOR_NAME, temperature, humidity, analogPercent, rawAnalog,
                  fanState ? "ON" : "OFF", mistState ? "ON" : "OFF");
  }
}
