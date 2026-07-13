/**
 * LittleFS Force Format / Wipe Data Utility
 * Supports: ESP32 and ESP8266
 * 
 * Note: If using Arduino IDE, open this file directly.
 * Ensure you have selected the correct Board (ESP32 or ESP8266) under Tools > Board.
 */

#include <LittleFS.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n====================================");
  Serial.println("--- LittleFS Force Format Utility ---");
  Serial.println("====================================");

  // 1. Mount LittleFS to check status
  bool mounted = false;
  #if defined(ESP8266)
    mounted = LittleFS.begin();
  #elif defined(ESP32)
    mounted = LittleFS.begin(true);
  #endif

  if (mounted) {
    Serial.println("[INFO] LittleFS mounted successfully.");
  } else {
    Serial.println("[WARN] LittleFS mount failed. Proceeding to format anyway...");
  }

  // 2. Format the filesystem
  Serial.println("[INFO] Formatting LittleFS. Please wait, this may take a few seconds...");
  
  unsigned long startTime = millis();
  bool formatted = LittleFS.format();
  unsigned long duration = millis() - startTime;
  
  if (formatted) {
    Serial.print("[SUCCESS] LittleFS formatted successfully in ");
    Serial.print(duration);
    Serial.println(" ms!");
  } else {
    Serial.println("[ERROR] LittleFS format failed!");
  }
  
  Serial.println("====================================");
}

void loop() {
  // Do nothing
}
