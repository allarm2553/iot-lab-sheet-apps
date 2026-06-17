/**
 * Lab 6: Grand Challenge - Integrated Hybrid Node (Complete Code Solution)
 * 
 * ── Dashboard Compatibility Update ──
 * สอดคล้องกับ Dashboard ที่อัปเดตใหม่ ดังนี้:
 *  - MQTT Topic ทุกตัวเปลี่ยนเป็น prefix "esp32-climate-node/..."
 *    ตาม MQTT Topic Map ที่แสดงบน dashboard
 *  - WebSocket JSON payload เพิ่มฟิลด์ "mqtt": true/false
 *    เพื่ออัปเดต Connection Status Bar บน dashboard
 *  - Subscribe แยกสำหรับ Fan command และ Mist command
 *  - Subscribe Threshold command จาก dashboard slider
 *  - Publish แยก topic ให้ทุก sensor และ relay state
 * 
 * Features:
 *  - Reads DHT11 (GPIO 33) and Potentiometer/Soil sensor (GPIO 34).
 *  - Drives OLED Display via I2C (SDA=21, SCL=22).
 *  - Hosts WebServer (Port 80) from LittleFS files.
 *  - Hosts WebSocket Server (Port 81) for local dashboard.
 *  - Connects to EMQX MQTT Broker for cloud control.
 *  - Hysteresis Fan Control: ON if temp > threshold, OFF if < threshold - 0.5.
 *  - Hysteresis Mist Control: ON if soil < 40%, OFF if soil > 50%.
 *  - Non-blocking MQTT Reconnection via millis() timer (every 10s).
 */

#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#endif
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <LittleFS.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ─────────────────────────────────────────────
// Wi-Fi Credentials
// ─────────────────────────────────────────────
const char* ssid     = "iot_512";
const char* password = "iot123456";

// ─────────────────────────────────────────────
// MQTT Broker
// ─────────────────────────────────────────────
const char* mqttServer = "broker.emqx.io";
const int   mqttPort   = 1883;

// MQTT Topic prefix (ตรงกับ Topic Map ใน dashboard)
const char* TOPIC_PREFIX = "esp32-climate-node";

// ── Subscriptions (รับคำสั่งจาก dashboard / Home Assistant) ──
const char* TOPIC_SUB_FAN_CMD       = "esp32-climate-node/switch/cooling_fan_relay/command";
const char* TOPIC_SUB_MIST_CMD      = "esp32-climate-node/switch/mist_pump_relay/command";
const char* TOPIC_SUB_THRESHOLD_SET = "esp32-climate-node/climate/temp_threshold/set";

// ── Publications (ส่งค่าขึ้น broker) ──
const char* TOPIC_PUB_TEMP          = "esp32-climate-node/sensor/room_temperature/state";
const char* TOPIC_PUB_HUMIDITY      = "esp32-climate-node/sensor/room_humidity/state";
const char* TOPIC_PUB_SOIL          = "esp32-climate-node/sensor/analog/state";
const char* TOPIC_PUB_FAN_STATE     = "esp32-climate-node/switch/cooling_fan_relay/state";
const char* TOPIC_PUB_MIST_STATE    = "esp32-climate-node/switch/mist_pump_relay/state";

// ─────────────────────────────────────────────
// Hardware Pins
// ─────────────────────────────────────────────
#if defined(ESP8266)
#define DHTPIN         0       // D3/GPIO 0
#define ANALOG_PIN     A0      // A0
#define FAN_RELAY_PIN  13      // D7/GPIO 13
#define MIST_RELAY_PIN 14      // D5/GPIO 14
#define ADC_RESOLUTION 1023.0
#elif defined(ESP32)
#define DHTPIN         33
#define ANALOG_PIN     34
#define FAN_RELAY_PIN  13
#define MIST_RELAY_PIN 14
#define ADC_RESOLUTION 4095.0
#endif
#define DHTTYPE DHT11

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET   -1

// ─────────────────────────────────────────────
// Object Instances
// ─────────────────────────────────────────────
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#if defined(ESP8266)
ESP8266WebServer server(80);
#else
WebServer server(80);
#endif
WebSocketsServer webSocket = WebSocketsServer(81);
WiFiClient   espClient;
PubSubClient mqttClient(espClient);

// ─────────────────────────────────────────────
// Application State
// ─────────────────────────────────────────────
float temperature   = 0.0;
float humidity      = 0.0;
float analogPercent = 0.0;
float tempThreshold = 30.0;
bool  fanState      = false;
bool  mistState     = false;

// ─────────────────────────────────────────────
// broadcastLocalDashboard()
// ส่ง JSON ไปยัง WebSocket clients ทุกตัว
// เพิ่มฟิลด์ "mqtt" เพื่ออัปเดต Connection Status Bar
// ─────────────────────────────────────────────
void broadcastLocalDashboard() {
  JsonDocument doc;
  doc["temp"]      = temperature;
  doc["humidity"]  = humidity;
  doc["soil"]      = analogPercent;
  doc["fan"]       = fanState;
  doc["mist"]      = mistState;
  doc["threshold"] = tempThreshold;
  doc["mqtt"]      = mqttClient.connected();  // ← อัปเดต MQTT status pill บน dashboard

  String output;
  serializeJson(doc, output);
  webSocket.broadcastTXT(output);
}

// ─────────────────────────────────────────────
// updateOledDisplay()
// ─────────────────────────────────────────────
void updateOledDisplay() {
  display.clearDisplay();

  // Header
  display.setCursor(15, 0);
  display.setTextSize(1);
  display.print("HYBRID IOT NODE");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  // IP Address
  display.setCursor(0, 12);
  display.printf("IP: %s", WiFi.localIP().toString().c_str());

  // Sensor readings
  display.setCursor(0, 24);
  display.printf("T: %.1fC | H: %.1f%%", temperature, humidity);

  display.setCursor(0, 36);
  display.printf("Soil/Analog: %.1f%%", analogPercent);

  // Relay states
  display.setCursor(0, 48);
  display.printf("Fan:%s | Mist:%s",
                 fanState  ? "ON" : "OFF",
                 mistState ? "ON" : "OFF");

  // MQTT + Threshold footer
  display.drawLine(0, 57, 128, 57, SSD1306_WHITE);
  display.setCursor(0, 59);
  display.printf("MQTT:%s Thr:%.1f",
                 mqttClient.connected() ? "OK" : "NO",
                 tempThreshold);

  display.display();
}

// ─────────────────────────────────────────────
// publishAllToMqtt()
// Publish สถานะทุก sensor และ relay ขึ้น broker
// เรียกทุก 10 วินาทีเมื่อ MQTT เชื่อมต่ออยู่
// ─────────────────────────────────────────────
void publishAllToMqtt() {
  if (!mqttClient.connected()) return;

  // Sensor values (ทศนิยม 1 ตำแหน่ง)
  char buf[12];

  dtostrf(temperature,   4, 1, buf);
  mqttClient.publish(TOPIC_PUB_TEMP, buf);

  dtostrf(humidity,      4, 1, buf);
  mqttClient.publish(TOPIC_PUB_HUMIDITY, buf);

  dtostrf(analogPercent, 4, 1, buf);
  mqttClient.publish(TOPIC_PUB_SOIL, buf);

  // Relay states (ตาม HA convention: "ON" / "OFF")
  mqttClient.publish(TOPIC_PUB_FAN_STATE,  fanState  ? "ON" : "OFF");
  mqttClient.publish(TOPIC_PUB_MIST_STATE, mistState ? "ON" : "OFF");

  Serial.printf("[MQTT] Published → T:%.1f H:%.1f Soil:%.1f Fan:%s Mist:%s\n",
                temperature, humidity, analogPercent,
                fanState ? "ON" : "OFF", mistState ? "ON" : "OFF");
}

// ─────────────────────────────────────────────
// mqttCallback()
// รับคำสั่งจาก broker แยกตาม topic
// ─────────────────────────────────────────────
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  message.trim();
  Serial.printf("[MQTT] Received [%s]: %s\n", topic, message.c_str());

  // ── Fan Command ──
  if (String(topic) == TOPIC_SUB_FAN_CMD) {
    if (message.equalsIgnoreCase("ON") || message == "1") {
      fanState = true;
      digitalWrite(FAN_RELAY_PIN, HIGH);
    } else if (message.equalsIgnoreCase("OFF") || message == "0") {
      fanState = false;
      digitalWrite(FAN_RELAY_PIN, LOW);
    }
    // Publish state feedback
    mqttClient.publish(TOPIC_PUB_FAN_STATE, fanState ? "ON" : "OFF");
  }

  // ── Mist Command ──
  else if (String(topic) == TOPIC_SUB_MIST_CMD) {
    if (message.equalsIgnoreCase("ON") || message == "1") {
      mistState = true;
      digitalWrite(MIST_RELAY_PIN, HIGH);
    } else if (message.equalsIgnoreCase("OFF") || message == "0") {
      mistState = false;
      digitalWrite(MIST_RELAY_PIN, LOW);
    }
    mqttClient.publish(TOPIC_PUB_MIST_STATE, mistState ? "ON" : "OFF");
  }

  // ── Threshold Set ──
  else if (String(topic) == TOPIC_SUB_THRESHOLD_SET) {
    float newThreshold = message.toFloat();
    if (newThreshold >= 25.0 && newThreshold <= 35.0) {
      tempThreshold = newThreshold;
      Serial.printf("[MQTT] Threshold updated to %.1f C\n", tempThreshold);
    }
  }

  // Sync all clients after any MQTT command
  broadcastLocalDashboard();
  updateOledDisplay();
}

// ─────────────────────────────────────────────
// reconnectMqttNonBlocking()
// Subscribe ทุก topic ใหม่เมื่อ reconnect สำเร็จ
// ─────────────────────────────────────────────
void reconnectMqttNonBlocking() {
  static unsigned long lastMqttRetry = 0;

  if (!mqttClient.connected()) {
    unsigned long now = millis();
    if (now - lastMqttRetry > 10000 || lastMqttRetry == 0) {
      lastMqttRetry = now;
      Serial.print("[MQTT] Connecting (non-blocking)...");

      String clientId = "ESP-Resilient-" + String(random(0xffff), HEX);
      if (mqttClient.connect(clientId.c_str())) {
        Serial.println(" Connected.");

        // Subscribe ทุก command topic
        mqttClient.subscribe(TOPIC_SUB_FAN_CMD);
        mqttClient.subscribe(TOPIC_SUB_MIST_CMD);
        mqttClient.subscribe(TOPIC_SUB_THRESHOLD_SET);

        Serial.println("[MQTT] Subscribed to command topics.");

        // Notify dashboard ว่า MQTT กลับมา online แล้ว
        broadcastLocalDashboard();
      } else {
        Serial.printf(" Failed (state %d). Retry in 10s.\n", mqttClient.state());
      }
    }
  }
}

// ─────────────────────────────────────────────
// webSocketEvent()
// รับคำสั่งจาก dashboard ผ่าน WebSocket
// ─────────────────────────────────────────────
void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_CONNECTED) {
    // ส่งสถานะปัจจุบันให้ client ใหม่ทันที
    JsonDocument doc;
    doc["temp"]      = temperature;
    doc["humidity"]  = humidity;
    doc["soil"]      = analogPercent;
    doc["fan"]       = fanState;
    doc["mist"]      = mistState;
    doc["threshold"] = tempThreshold;
    doc["mqtt"]      = mqttClient.connected();
    String output;
    serializeJson(doc, output);
    webSocket.sendTXT(num, output);
    Serial.printf("[WS] Client #%d connected. Sent current state.\n", num);
  }

  if (type == WStype_TEXT) {
    JsonDocument doc;
    deserializeJson(doc, payload, length);

    bool changed = false;

    // ── Manual override: Fan ──
    if (doc.containsKey("action")) {
      String action = doc["action"];
      if (action == "toggle_fan") {
        fanState = doc["value"].as<bool>();
        digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
        mqttClient.publish(TOPIC_PUB_FAN_STATE, fanState ? "ON" : "OFF");
        Serial.printf("[WS] Fan override → %s\n", fanState ? "ON" : "OFF");
        changed = true;
      } else if (action == "toggle_mist") {
        mistState = doc["value"].as<bool>();
        digitalWrite(MIST_RELAY_PIN, mistState ? HIGH : LOW);
        mqttClient.publish(TOPIC_PUB_MIST_STATE, mistState ? "ON" : "OFF");
        Serial.printf("[WS] Mist override → %s\n", mistState ? "ON" : "OFF");
        changed = true;
      }
    }

    // ── Threshold slider ──
    if (doc.containsKey("threshold")) {
      float newVal = doc["threshold"].as<float>();
      if (newVal >= 25.0 && newVal <= 35.0) {
        tempThreshold = newVal;
        Serial.printf("[WS] Threshold → %.1f C\n", tempThreshold);

        // Publish threshold change ไปยัง broker ด้วย
        char buf[8];
        dtostrf(tempThreshold, 4, 1, buf);
        mqttClient.publish(TOPIC_SUB_THRESHOLD_SET, buf);
        changed = true;
      }
    }

    if (changed) {
      broadcastLocalDashboard();
      updateOledDisplay();
    }
  }
}

// ─────────────────────────────────────────────
// handleFileRequest()
// Serve LittleFS files สำหรับ Web Dashboard
// ─────────────────────────────────────────────
void handleFileRequest() {
  String path = server.uri();
  if (path.endsWith("/")) path += "index.html";

  String dataType = "text/plain";
  if      (path.endsWith(".html")) dataType = "text/html";
  else if (path.endsWith(".css"))  dataType = "text/css";
  else if (path.endsWith(".js"))   dataType = "application/javascript";
  else if (path.endsWith(".ico"))  dataType = "image/x-icon";

  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    server.streamFile(file, dataType);
    file.close();
  } else {
    server.send(404, "text/plain", "404: File Not Found");
  }
}

// ─────────────────────────────────────────────
// setup()
// ─────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  dht.begin();

  // Relay outputs
  pinMode(FAN_RELAY_PIN,  OUTPUT);
  pinMode(MIST_RELAY_PIN, OUTPUT);
  digitalWrite(FAN_RELAY_PIN,  LOW);
  digitalWrite(MIST_RELAY_PIN, LOW);

  // ADC attenuation for ESP32 (11dB = full 0–3.3V range)
#if defined(ESP32)
  analogSetPinAttenuation(ANALOG_PIN, ADC_11db);
#endif

  // OLED init
#if defined(ESP8266)
  Wire.begin(4, 5);
#else
  Wire.begin(21, 22);
#endif
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("[OLED] SSD1306 allocation failed"));
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.print("Initializing...");
  display.display();

  // Wi-Fi
  Serial.printf("[WiFi] Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\n[WiFi] Connected. IP: %s\n", WiFi.localIP().toString().c_str());

  // OLED: show IP
  display.clearDisplay();
  display.setCursor(0, 0);
  display.printf("WiFi: %s\n", WiFi.localIP().toString().c_str());
  display.display();

  // LittleFS
#if defined(ESP8266)
  if (LittleFS.begin()) {
#else
  if (LittleFS.begin(true)) {
#endif
    Serial.println("[FS] LittleFS mounted.");
  } else {
    Serial.println("[FS] LittleFS mount FAILED.");
  }

  // HTTP server
  server.onNotFound(handleFileRequest);
  server.begin();
  Serial.println("[HTTP] Web server started on port 80.");

  // WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("[WS] WebSocket server started on port 81.");

  // MQTT client
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setKeepAlive(60);

  Serial.println("[SYS] Hybrid Node ready. Dashboard: http://" + WiFi.localIP().toString());
}

// ─────────────────────────────────────────────
// loop()
// ─────────────────────────────────────────────
void loop() {
  server.handleClient();
  webSocket.loop();

  // Non-blocking MQTT maintenance
  reconnectMqttNonBlocking();
  if (mqttClient.connected()) {
    mqttClient.loop();
  }

  // ── Sensor & control loop (ทุก 2 วินาที) ──
  static unsigned long lastReading = 0;
  if (millis() - lastReading > 2000) {
    lastReading = millis();

    // Read sensors
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    int rawAnalog = analogRead(ANALOG_PIN);

    if (!isnan(t)) temperature = t;
    if (!isnan(h)) humidity    = h;
    analogPercent = (rawAnalog / ADC_RESOLUTION) * 100.0;

    // ── Hysteresis: Fan (temperature threshold) ──
    if (!isnan(temperature)) {
      if (temperature > tempThreshold) {
        fanState = true;
      } else if (temperature < (tempThreshold - 0.5f)) {
        fanState = false;
      }
      digitalWrite(FAN_RELAY_PIN, fanState ? HIGH : LOW);
    }

    // ── Hysteresis: Mist pump (soil moisture) ──
    if (analogPercent < 40.0f) {
      mistState = true;
    } else if (analogPercent > 50.0f) {
      mistState = false;
    }
    digitalWrite(MIST_RELAY_PIN, mistState ? HIGH : LOW);

    // Update OLED and broadcast to all WS clients
    updateOledDisplay();
    broadcastLocalDashboard();

    // ── MQTT Publish (ทุก 10 วินาที) ──
    static unsigned long lastMqttPub = 0;
    if (millis() - lastMqttPub > 10000) {
      lastMqttPub = millis();
      publishAllToMqtt();
    }
  }

  delay(1);
}
