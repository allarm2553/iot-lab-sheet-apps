/**
 * Lab 2: OLED SSD1306 Display (Complete Code Solution)
 * Features:
 *  - Interfacing SSD1306 OLED via I2C (SDA -> GPIO 21, SCL -> GPIO 22).
 *  - Displays header "MONITORING NODE" with a horizontal divider line at Y=10.
 *  - Displays real-time Temperature (C) and Soil Moisture (%).
 *  - Challenge: Displays the real-time status of Relay 1 (Fan) and Relay 2 (Pump/Mist)
 *    on the bottom row of the screen (e.g., "Fan: ON | Mist: OFF").
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#if defined(ESP8266)
#define DHTPIN 0            // D3/GPIO 0
#define ANALOG_PIN A0       // A0
#define FAN_RELAY_PIN 13    // D7/GPIO 13
#define MIST_RELAY_PIN 14   // D5/GPIO 14
#define ADC_RESOLUTION 1023.0
#elif defined(ESP32)
#define DHTPIN 33
#define ANALOG_PIN 34
#define FAN_RELAY_PIN 13
#define MIST_RELAY_PIN 14
#define ADC_RESOLUTION 4095.0
#endif
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

float temperature = 0;
float humidity = 0;
int rawAnalog = 0;
float analogPercent = 0;

// Relays states
bool fanState = false;
bool mistState = false;

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // Initialize I2C and OLED Display
  #if defined(ESP8266)
  Wire.begin(4, 5); // SDA = GPIO 4 (D2), SCL = GPIO 5 (D1) สำหรับ ESP8266
  #else
  Wire.begin(21, 22); // SDA = GPIO 21, SCL = GPIO 22 สำหรับ ESP32
  #endif
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  // Configure Relays
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(MIST_RELAY_PIN, OUTPUT);
  digitalWrite(FAN_RELAY_PIN, LOW);
  digitalWrite(MIST_RELAY_PIN, LOW);
  
  // Set ADC attenuation
  #if defined(ESP32)
  analogSetPinAttenuation(ANALOG_PIN, ADC_11db);
  #endif

  // Initial screen display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.print("System Booting...");
  display.display();
  delay(1500);
}

void loop() {
  // Read sensors
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  rawAnalog = analogRead(ANALOG_PIN);
  analogPercent = (rawAnalog / ADC_RESOLUTION) * 100.0;
  
  // Fan Control with Hysteresis (from Lab 1)
  if (temperature > 30.0) {
    fanState = true;
  } else if (temperature < 29.5) {
    fanState = false;
  }
  digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
  
  // Mist/Pump Control based on Analog Sensor threshold
  if (analogPercent < 40.0) {
    mistState = true;
  } else if (analogPercent > 60.0) {
    mistState = false;
  }
  digitalWrite(MIST_RELAY_PIN, mistState ? HIGH : LOW);
  
  // Draw OLED interface
  display.clearDisplay();
  
  // 1. เขียนข้อความพาดหัว (Header) ไว้ที่แถวบนสุด
  display.setCursor(15, 0);
  display.print("MONITORING NODE");
  
  // 2. วาดเส้นแบ่งกึ่งกลางหน้าจอแนวนอนที่พิกเซล Y = 10
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  // 3. แสดงค่าเซ็นเซอร์อุณหภูมิและความชื้นที่อ่านได้จริง
  display.setCursor(0, 16);
  if (isnan(temperature)) {
    display.print("Temp: -- C");
  } else {
    display.printf("Temp: %.1f C", temperature);
  }
  
  display.setCursor(0, 28);
  if (isnan(humidity)) {
    display.print("Humid: -- %");
  } else {
    display.printf("Humid: %.1f %%", humidity);
  }
  
  display.setCursor(0, 40);
  display.printf("Analog: %.1f %%", analogPercent);
  
  // 4. โจทย์ท้าทาย: แสดงสถานะเปิด-ปิดของพัดลมและปั๊มน้ำด้านล่างสุด
  display.drawLine(0, 52, 128, 52, SSD1306_WHITE);
  display.setCursor(0, 55);
  display.printf("Fan:%s | Mist:%s", fanState ? "ON" : "OFF", mistState ? "ON" : "OFF");
  
  // 5. แสดงผลข้อความส่งคำสั่งออกจอภาพจริง
  display.display();
  
  delay(1000);
}
