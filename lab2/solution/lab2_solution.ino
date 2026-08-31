/**
 * Lab 2: OLED SSD1306 Display & Multi-Sensor Dashboard (Arduino Sketch Solution)
 * 
 * Features:
 *  - Interfacing SSD1306 0.96" OLED via I2C (128x64 Pixels, Address 0x3C).
 *  - Configurable DHT Sensor Selection (DHT11 / DHT22).
 *  - Non-blocking Sensor Read & OLED Screen Refresh via millis().
 *  - Potentiometer ADC reading with floating-point math & graphical Gauge Bar.
 *  - Switch Button Input with Software Debounce & State Toggle.
 *  - Output Relay & Alert LED control.
 *  - Multi-Sensor Dashboard Layout with Fail-Safe NaN detection.
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
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
  #define SENSOR_NAME  "DHT22"
#endif

// ================================================================
// 📟 2. กำหนดค่าหน้าจอ OLED SSD1306 (I2C)
// ================================================================
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ================================================================
// 🔌 3. การกำหนดขาพินฮาร์ดแวร์ตามชนิดบอร์ด (Pin Definitions)
// ================================================================
#if defined(ESP8266)
#define BOARD_NAME      "ESP8266 (AX-WiFi / NodeMCU)"
#define I2C_SDA_PIN     4            // D2 / GPIO 4 สำหรับ AX-WiFi
#define I2C_SCL_PIN     5            // D1 / GPIO 5 สำหรับ AX-WiFi
#define DHTPIN          0            // D3 / GPIO 0 สำหรับ AX-WiFi
#define ANALOG_PIN      A0           // A0 (ตัวต้านทานปรับค่าได้ VR สำหรับ AX-WiFi)
#define BUTTON_PIN      0            // D3 / GPIO 0 (ปุ่ม FLASH บนบอร์ด)
#define FAN_RELAY_PIN   14           // D5 / GPIO 14 สำหรับ AX-WiFi
#define ALERT_LED_PIN   2            // Onboard LED (GPIO 2 / D4)
#define ADC_RESOLUTION  1023.0f
#elif defined(ESP32)
#define BOARD_NAME      "ESP32 (IPST-WiFi / DevKit)"
#define I2C_SDA_PIN     21           // GPIO 21 สำหรับ IPST-WiFi
#define I2C_SCL_PIN     22           // GPIO 22 สำหรับ IPST-WiFi
#define DHTPIN          33           // พอร์ต 33 สำหรับ IPST-WiFi
#define ANALOG_PIN      36           // GPIO 36 / KNOB-S สำหรับ IPST-WiFi
#define BUTTON_PIN      0            // GPIO 0 (ปุ่ม SW1 บนบอร์ด)
#define FAN_RELAY_PIN   5            // พอร์ต 5 สำหรับ IPST-WiFi
#define ALERT_LED_PIN   18           // Onboard LED (GPIO 18)
#define ADC_RESOLUTION  4095.0f
#endif

#define RELAY_ON        HIGH
#define RELAY_OFF       LOW

DHT dht(DHTPIN, DHTTYPE);

// ตัวแปรข้อมูลเซ็นเซอร์และสถานะ
float temperature = 0.0f;
float humidity = 0.0f;
int rawAnalog = 0;
float analogPercent = 0.0f;
bool sensorError = false;

// Button Debounce & State Variables
bool lastButtonState = HIGH;
bool currentButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 50;
bool fanState = false;
int toggleCount = 0;

// OLED Non-blocking Timer
unsigned long lastDisplayTime = 0;
const unsigned long DISPLAY_INTERVAL = 500; // รีเฟรชจอทุก 500ms

void updateOLED() {
  display.clearDisplay();

  // 1. วาดแถบ Header ด้านบน (Inverted Bar)
  display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(4, 2);
  display.print("IoT SMART DASHBOARD");

  display.setTextColor(SSD1306_WHITE);

  // 2. แสดงค่าอุณหภูมิและความชื้น
  display.setCursor(0, 16);
  if (!sensorError) {
    display.printf("T: %.1f C  H: %.0f %%", temperature, humidity);
  } else {
    display.print("DHT: SENSOR ERROR!");
  }

  // 3. แสดงค่าแอนะล็อกและหลอดเกจกราฟิก (Progress Bar)
  display.setCursor(0, 29);
  display.printf("VR: %.0f%% (%d)", analogPercent, rawAnalog);
  
  // วาดกรอบเกจ (X=0, Y=40, W=128, H=7)
  display.drawRect(0, 40, 128, 7, SSD1306_WHITE);
  int barWidth = (int)((analogPercent / 100.0f) * 124.0f);
  if (barWidth > 124) barWidth = 124;
  if (barWidth > 0) {
    display.fillRect(2, 42, barWidth, 3, SSD1306_WHITE);
  }

  // 4. แสดงสถานะรีเลย์และจำนวนครั้งที่กดสวิตช์
  display.setCursor(0, 52);
  display.printf("Fan: %s (#%d) | SW:%s", 
                 fanState ? "ON" : "OFF", 
                 toggleCount,
                 currentButtonState == LOW ? "DN" : "UP");

  display.display();
}

void setup() {
  Serial.begin(115200);
  delay(500);

  // 1. ตั้งค่าขาพิน
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(ALERT_LED_PIN, OUTPUT);
  pinMode(ANALOG_PIN, INPUT);

  digitalWrite(FAN_RELAY_PIN, RELAY_OFF);
  digitalWrite(ALERT_LED_PIN, LOW);

  #if defined(ESP32)
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);
  #endif

  dht.begin();

  // 2. เริ่มต้นบัส I2C และจอ OLED
  #if defined(ESP8266)
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  #else
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  #endif

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("[ERROR] ไม่พบจอ OLED SSD1306 หรือจัดสรรหน่วยความจำล้มเหลว!"));
    for (;;);
  }

  // แสดงหน้าจอต้อนรับ (Splash Screen)
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(18, 15);
  display.print("LAB 2: OLED I2C");
  display.setCursor(10, 30);
  display.print("System Initialized");
  display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
  display.display();
  delay(1200);

  Serial.println("=================================================");
  Serial.println(" Lab 2: OLED Display & Multi-Sensor Initialized");
  Serial.printf(" Board : %s\n", BOARD_NAME);
  Serial.printf(" OLED  : SSD1306 (128x64) I2C Address 0x%02X\n", SCREEN_ADDRESS);
  Serial.printf(" Sensor: %s (Pin %d)\n", SENSOR_NAME, DHTPIN);
  Serial.println("=================================================");
}

void loop() {
  unsigned long now = millis();

  // 1. จัดการอินพุตสวิตช์ปุ่มกด (Debounce & State Toggle)
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = now;
  }

  if ((now - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != currentButtonState) {
      currentButtonState = reading;
      if (currentButtonState == LOW) {
        fanState = !fanState;
        digitalWrite(FAN_RELAY_PIN, fanState ? RELAY_ON : RELAY_OFF);
        toggleCount++;
        Serial.printf(">> [BUTTON CLICK] Toggle #%d | Fan: %s\n", 
                      toggleCount, fanState ? "ON" : "OFF");
        updateOLED(); // อัพเดทหน้าจอทันทีเมื่อมีการกดปุ่ม
      }
    }
  }
  lastButtonState = reading;

  // 2. รอบการอ่านเซ็นเซอร์และรีเฟรชหน้าจอ OLED (Non-blocking Timer)
  if (now - lastDisplayTime >= DISPLAY_INTERVAL) {
    lastDisplayTime = now;

    // อ่านค่าเซ็นเซอร์
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    rawAnalog = analogRead(ANALOG_PIN);

    if (isnan(t) || isnan(h)) {
      sensorError = true;
      digitalWrite(ALERT_LED_PIN, HIGH);
    } else {
      sensorError = false;
      temperature = t;
      humidity = h;
      digitalWrite(ALERT_LED_PIN, LOW);
    }

    analogPercent = ((float)rawAnalog / ADC_RESOLUTION) * 100.0f;

    // รีเฟรชหน้าจอ OLED
    updateOLED();
  }
}
