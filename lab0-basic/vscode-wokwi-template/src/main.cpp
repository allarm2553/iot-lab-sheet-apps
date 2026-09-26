/**
 * ============================================================================
 * Lab 0: Environment Setup (VS Code, PlatformIO, Wokwi) & LED Blink Test
 * Course: Hybrid Local/Cloud IoT Node with ESP32 / IPST-WiFi
 * ============================================================================
 */

#include <Arduino.h>

// --- Pin Definitions ---
#define ONBOARD_LED_PIN   2   // ESP32 On-board Built-in LED (GPIO 2)
#define EXT_LED1_PIN      23  // External Red LED (GPIO 23)
#define EXT_LED2_PIN      19  // External Blue LED (GPIO 19)
#define BUTTON_PIN        18  // Push Button Switch (GPIO 18, INPUT_PULLUP)

// --- Timing Variables for Non-blocking Blink ---
unsigned long previousMillis = 0;
const long interval = 500;    // Blink interval: 500 ms (1 Hz cycle)
bool ledState = LOW;
int blinkCount = 0;

void setup() {
  // 1. Initialize Serial Communication at 115200 baud
  Serial.begin(115200);
  delay(500); // Allow hardware to stabilize

  Serial.println("\n==================================================");
  Serial.println("🚀 ESP32 IoT Lab 0: Environment & Blink Test");
  Serial.println("🎓 Visual Studio Code + PlatformIO + Wokwi Ready!");
  Serial.println("==================================================");

  // 2. Configure GPIO Pin Modes
  pinMode(ONBOARD_LED_PIN, OUTPUT);
  pinMode(EXT_LED1_PIN, OUTPUT);
  pinMode(EXT_LED2_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initial state: Turn all LEDs OFF
  digitalWrite(ONBOARD_LED_PIN, LOW);
  digitalWrite(EXT_LED1_PIN, LOW);
  digitalWrite(EXT_LED2_PIN, LOW);

  Serial.println("✅ GPIO Pins configured successfully.");
  Serial.println("💡 Onboard LED: GPIO 2, Ext LED1: GPIO 23, Ext LED2: GPIO 19, Button: GPIO 18\n");
}

void loop() {
  // Read push button state (LOW when pressed due to INPUT_PULLUP)
  bool buttonPressed = (digitalRead(BUTTON_PIN) == LOW);

  // Non-blocking timer using millis()
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= (buttonPressed ? 150 : interval)) {
    previousMillis = currentMillis;
    ledState = !ledState;
    blinkCount++;

    // Toggle LEDs
    digitalWrite(ONBOARD_LED_PIN, ledState);
    digitalWrite(EXT_LED1_PIN, ledState);
    digitalWrite(EXT_LED2_PIN, !ledState); // Alternating blink between LED1 and LED2

    // Print status over Serial Monitor
    Serial.printf("[T=%lums | #%d] LED State: %s | Mode: %s | CPU Temp: %.1f°C\n",
                  currentMillis,
                  blinkCount,
                  ledState ? "⚡ ON (HIGH)" : "💤 OFF (LOW)",
                  buttonPressed ? "🔥 FAST BLINK (Button Pressed)" : "🟢 NORMAL (500ms)",
                  temperatureRead());
  }

  // Small delay to prevent tight loop in simulation
  delay(10);
}
