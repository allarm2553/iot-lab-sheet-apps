#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// Screen Dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

// Pin Definitions
#define PIN_OLED_SDA 21
#define PIN_OLED_SCL 22
#define PIN_DHT      33
#define PIN_POT      36
#define PIN_SW1      0
#define PIN_RELAY1   5
#define PIN_RELAY2   23
#define PIN_LED      18

// Sensor & Display Objects
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(PIN_DHT, DHT22);

// Application State Variables
int pressCount = 0;
bool relay1State = false;
bool relay2State = false;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

unsigned long lastDisplayTime = 0;
const unsigned long displayInterval = 250; // 4 Hz refresh rate

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n[LAB 2] ESP32 OLED SSD1306 & Multi-Sensor Dashboard Initializing...");

  // Initialize GPIO Pins
  pinMode(PIN_SW1, INPUT_PULLUP);
  pinMode(PIN_RELAY1, OUTPUT);
  pinMode(PIN_RELAY2, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_RELAY1, LOW);
  digitalWrite(PIN_RELAY2, LOW);
  digitalWrite(PIN_LED, LOW);

  // Initialize I2C and OLED
  Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("[ERROR] SSD1306 allocation failed. Check I2C wiring and address (0x3C)!"));
    for(;;); // Don't proceed, loop forever
  }

  // Initialize DHT Sensor
  dht.begin();

  // Show Initial Splash
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 20);
  display.println(F("IoT SMART DASH"));
  display.setCursor(20, 35);
  display.println(F("OLED SSD1306 Ready"));
  display.display();
  delay(1500);

  Serial.println("[SYSTEM] System Ready!");
}

void loop() {
  // 1. Button Debounce & Toggle Handling
  int reading = digitalRead(PIN_SW1);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    static int stableButtonState = HIGH;
    if (reading != stableButtonState) {
      stableButtonState = reading;
      if (stableButtonState == LOW) { // Button Pressed
        pressCount++;
        relay1State = !relay1State;
        digitalWrite(PIN_RELAY1, relay1State ? HIGH : LOW);
        digitalWrite(PIN_LED, relay1State ? HIGH : LOW);
        
        // Auto mist trigger every 3 presses
        if (pressCount % 3 == 0) {
          relay2State = true;
          digitalWrite(PIN_RELAY2, HIGH);
        } else {
          relay2State = false;
          digitalWrite(PIN_RELAY2, LOW);
        }

        Serial.printf("[BUTTON] Count: %d | Relay1(Fan): %s | Relay2(Mist): %s\n",
                      pressCount, relay1State ? "ON" : "OFF", relay2State ? "ON" : "OFF");
      }
    }
  }
  lastButtonState = reading;

  // 2. Non-blocking OLED Display Refresh (4 Hz)
  if (millis() - lastDisplayTime >= displayInterval) {
    lastDisplayTime = millis();

    // Read Sensors
    float temp = dht.readTemperature();
    float humid = dht.readHumidity();
    int potRaw = analogRead(PIN_POT);
    int potPercent = map(potRaw, 0, 4095, 0, 100);

    // Clear Frame Buffer
    display.clearDisplay();

    // -------------------------------------------------------------
    // SECTION 1: INVERTED HEADER BAR (128x10 px)
    // -------------------------------------------------------------
    display.fillRect(0, 0, 128, 10, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Inverted text
    display.setTextSize(1);
    display.setCursor(14, 1);
    display.print(F("IoT SMART DASHBOARD"));

    // -------------------------------------------------------------
    // SECTION 2: DHT SENSOR READINGS (Y: 14..23)
    // -------------------------------------------------------------
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 14);
    if (isnan(temp) || isnan(humid)) {
      display.print(F("DHT: Sensor Error!"));
    } else {
      display.printf("T:%.1fC  H:%.1f%%", temp, humid);
    }

    // -------------------------------------------------------------
    // SECTION 3: ADC GRAPHIC PROGRESS BAR (Y: 27..37)
    // -------------------------------------------------------------
    display.setCursor(0, 27);
    display.printf("ADC: %3d%%", potPercent);
    // Draw Progress Bar Border & Fill
    display.drawRect(55, 27, 72, 8, SSD1306_WHITE);
    int barWidth = map(potPercent, 0, 100, 0, 68);
    if (barWidth > 0) {
      display.fillRect(57, 29, barWidth, 4, SSD1306_WHITE);
    }

    // Separator Line
    display.drawLine(0, 39, 128, 39, SSD1306_WHITE);

    // -------------------------------------------------------------
    // SECTION 4: RELAY & TOGGLE STATUS (Y: 43..54)
    // -------------------------------------------------------------
    display.setCursor(0, 43);
    display.printf("FAN : [%s]", relay1State ? "ON " : "OFF");
    display.setCursor(68, 43);
    display.printf("MIST: [%s]", relay2State ? "ON " : "OFF");

    display.setCursor(0, 54);
    display.printf("BTN SW1 : %d Presses", pressCount);

    // -------------------------------------------------------------
    // FLUSH FRAME BUFFER TO PHYSICAL OLED SCREEN
    // -------------------------------------------------------------
    display.display();

    // Serial Telemetry
    Serial.printf("[TELEMETRY] Temp: %.1f C, Humid: %.1f %%, ADC: %d%% (%d), Fan: %d, Mist: %d, Press: %d\n",
                  temp, humid, potPercent, potRaw, relay1State, relay2State, pressCount);
  }
}
