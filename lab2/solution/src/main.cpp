/**
 * Lab 2: OLED SSD1306 Display (Complete Code Solution with Integrated Lab 1 & Lab 1.1)
 * Features:
 *  - Interfacing SSD1306 OLED via I2C.
 *  - Reading DHT11 Temperature & Humidity (Lab 1).
 *  - Reading Analog Voltage via potentiometer (Lab 1).
 *  - Switch button input (BUTTON_PIN) with software debounce (Lab 1.1).
 *  - Toggling Relay/LED output (RELAY_PIN) and counting button presses (Lab 1.1).
 *  - Displays all sensor readings and the current button-controlled relay status on OLED.
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
#define DHTPIN 0            // D3/GPIO 0 สำหรับ AX-WiFi
#define ANALOG_PIN A0       // A0 (ตัวต้านทานปรับค่าได้ VR สำหรับ AX-WiFi)
#define BUTTON_PIN 0        // D3/GPIO 0 (ปุ่ม FLASH บนบอร์ด AX-WiFi)
#define RELAY_PIN 14        // D5/GPIO 14 สำหรับ AX-WiFi (Onboard SLED)
#define ADC_RESOLUTION 1023.0
#elif defined(ESP32)
#define DHTPIN 33           // พอร์ต 33 สำหรับ IPST-WiFi
#define ANALOG_PIN 36       // GPIO 36 / KNOB-S สำหรับ IPST-WiFi
#define BUTTON_PIN 0        // GPIO 0 (ปุ่ม SW1 บนบอร์ด IPST-WiFi)
#define RELAY_PIN 18        // พอร์ต 18 สำหรับ IPST-WiFi (Onboard LED)
#define ADC_RESOLUTION 4095.0
#endif
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

float temperature = 0;
float humidity = 0;
int rawAnalog = 0;
float analogPercent = 0;

// Button Debounce & State Variables (from Lab 1.1)
bool lastButtonState = HIGH;
bool currentButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50; 
bool relayState = false;
int toggleCount = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // Initialize I2C and OLED Display
  #if defined(ESP8266)
  Wire.begin(4, 5); // SDA = GPIO 4 (D2), SCL = GPIO 5 (D1) สำหรับ AX-WiFi
  #else
  Wire.begin(21, 22); // SDA = GPIO 21, SCL = GPIO 22 สำหรับ IPST-WiFi
  #endif
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  // Configure Switch and Relay/LED
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  
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
  // Read sensors (from Lab 1)
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  rawAnalog = analogRead(ANALOG_PIN);
  analogPercent = (rawAnalog / ADC_RESOLUTION) * 100.0;
  
  // Button Debounce & Toggle Control (from Lab 1.1)
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != currentButtonState) {
      currentButtonState = reading;
      if (currentButtonState == LOW) {
        relayState = !relayState;
        toggleCount++;
        Serial.printf("Button Pressed. Relay toggled to: %d, Count: %d\n", relayState, toggleCount);
      }
    }
  }
  digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
  lastButtonState = reading;
  
  // Draw OLED interface
  display.clearDisplay();
  
  // 1. เขียนข้อความพาดหัว (Header) ไว้ที่แถวบนสุด
  display.setCursor(15, 0);
  display.print("MONITORING NODE");
  
  // 2. วาดเส้นแบ่งกึ่งกลางหน้าจอแนวนอนที่พิกเซล Y = 10
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  // 3. แสดงค่าเซ็นเซอร์อุณหภูมิและความชื้นที่อ่านได้จริง (จากใบงานที่ 1)
  display.setCursor(0, 16);
  if (isnan(temperature)) {
    display.print("Temp: -- C");
  } else {
    display.printf("Temp: %.1f C", temperature);
  }
  
  display.setCursor(0, 28);
  if (isnan(humidity)) {
    display.print("Humid: -- %%");
  } else {
    display.printf("Humid: %.1f %%", humidity);
  }
  
  display.setCursor(0, 40);
  display.printf("Analog: %.1f %%", analogPercent);
  
  // 4. โจทย์ท้าทาย: แสดงสถานะการเปิด-ปิดและจำนวนครั้งการกดปุ่มของรีเลย์ (จากใบงานที่ 1.1)
  display.drawLine(0, 52, 128, 52, SSD1306_WHITE);
  display.setCursor(0, 55);
  display.printf("Relay:%s | Press:%d", relayState ? "ON" : "OFF", toggleCount);
  
  // 5. แสดงผลข้อความส่งคำสั่งออกจอภาพจริง
  display.display();
  
  delay(50); // Small delay for loop stability, responsive button checking
}
