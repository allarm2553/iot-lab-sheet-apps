/**
 * Lab 1: GPIO, ADC & Relays (Complete Code Solution)
 * Features:
 *  - Reading DHT11 temperature and humidity on GPIO 33.
 *  - Reading Analog Voltage via ADC on GPIO 34.
 *  - Relays control on GPIO 13 (Fan) and GPIO 14 (Pump/Mist).
 *  - Hysteresis control for Fan to prevent relay clicking:
 *    - Turn ON when temperature > 30.0 C
 *    - Turn OFF when temperature < 29.5 C
 */

#include <Arduino.h>
#include <DHT.h>

#if defined(ESP8266)
#define DHTPIN 0            // D3/GPIO 0 สำหรับ AX-WiFi
#define ANALOG_PIN A0       // A0 (ตัวต้านทานปรับค่าได้ VR สำหรับ AX-WiFi)
#define FAN_RELAY_PIN 13    // D7/GPIO 13สำหรับ AX-WiFi
#define MIST_RELAY_PIN 16   // D0/GPIO 16 สำหรับ AX-WiFi
#define ADC_RESOLUTION 1023.0
#elif defined(ESP32)
#define DHTPIN 33           // พอร์ต 33 สำหรับ IPST-WiFi
#define ANALOG_PIN 36       // GPIO 36 / KNOB-S สำหรับ IPST-WiFi
#define FAN_RELAY_PIN 5     // พอร์ต 5 สำหรับ IPST-WiFi
#define MIST_RELAY_PIN 23    // พอร์ต 23 สำหรับ IPST-WiFi
#define ADC_RESOLUTION 4095.0
#endif
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

float temperature = 0;
float humidity = 0;
int rawAnalog = 0;
float analogPercent = 0;

// State variable for Fan Hysteresis Control
bool fanState = false;

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // 1. ตั้งค่าโหมดของขารีเลย์ให้เป็นเอาต์พุต
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(MIST_RELAY_PIN, OUTPUT);
  
  // เริ่มต้นสถานะรีเลย์ให้ปิดอยู่ (Active HIGH)
  digitalWrite(FAN_RELAY_PIN, LOW);
  digitalWrite(MIST_RELAY_PIN, LOW);
  
  // 2. ตั้งค่าลดทอนสัญญาณของ ADC เป็น 11dB (ช่วงแรงดัน 0-3.3V)
  #if defined(ESP32)
  analogSetPinAttenuation(ANALOG_PIN, ADC_11db);
  #endif
  
  Serial.println("Lab 1 Complete Solution Initialized.");
}

void loop() {
  // อ่านค่าจากเซ็นเซอร์ DHT11
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  
  // อ่านค่าแรงดันอะนาล็อกจากเซ็นเซอร์
  rawAnalog = analogRead(ANALOG_PIN);
  
  // 3. แปลงค่า rawAnalog (0-4095) ให้เป็นเปอร์เซ็นต์ (0-100%)
  analogPercent = (rawAnalog / ADC_RESOLUTION) * 100.0;
  
  // ตรวจสอบค่าความถูกต้องในการอ่านเซ็นเซอร์
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.printf("Temp: %.1f C | Humidity: %.1f%% | Analog: %.1f%% (Raw: %d)\n", 
                  temperature, humidity, analogPercent, rawAnalog);
  }
  
  // 4. เขียนเงื่อนไขควบคุมพัดลมด้วยระบบ Hysteresis (โจทย์ท้าทาย)
  // พัดลมเปิดเมื่ออุณหภูมิ > 30.0 C และปิดเมื่ออุณหภูมิลดต่ำกว่า 29.5 C
  if (temperature > 30.0) {
    fanState = true;
  } else if (temperature < 29.5) {
    fanState = false;
  }
  
  // สั่งงานขา GPIO 13 ตามสถานะที่คำนวณได้
  digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
  
  // การควบคุมปั๊มน้ำ/พ่นหมอกแบบง่ายตามค่าความชื้นดิน/ตัวแปลงอะนาล็อก (ตัวอย่างเพิ่มเติม)
  // หากเปอร์เซ็นต์น้อยกว่า 40% ให้เปิดปั๊มน้ำ, เกิน 60% ให้ปิดปั๊มน้ำ
  if (analogPercent < 40.0) {
    digitalWrite(MIST_RELAY_PIN, HIGH);
  } else if (analogPercent > 60.0) {
    digitalWrite(MIST_RELAY_PIN, LOW);
  }
  
  delay(2000);
}
