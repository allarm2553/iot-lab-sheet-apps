/**
 * Lab 5: Cloud MQTT Connection (Complete Code Solution)
 * Features:
 *  - Connecting to Wi-Fi.
 *  - Connecting to public EMQX MQTT Broker (broker.emqx.io, port 1883).
 *  - Subscribing to "esp32-node/fan/cmd" to receive remote toggle commands (ON/OFF).
 *  - Controlling GPIO 13 (Fan) in the MQTT callback.
 *  - Publishing temperature sensor data to "esp32-node/temp/state" every 5 seconds.
 *  - Implementing automated reconnect logic.
 */

#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <PubSubClient.h>
#include <DHT.h>

const char* ssid = "iot_512";
const char* password = "iot123456";

// MQTT settings
const char* mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
const char* subTopic = "esp32-node/fan/cmd";
const char* pubTopic = "esp32-node/temp/state";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

#if defined(ESP8266)
#define DHTPIN 0            // D3/GPIO 0 สำหรับ AX-WiFi
#define FAN_RELAY_PIN 13    // D7/GPIO 13 สำหรับ AX-WiFi
#elif defined(ESP32)
#define DHTPIN 33           // พอร์ต 33 สำหรับ IPST-WiFi
#define FAN_RELAY_PIN 5     // พอร์ต 5 สำหรับ IPST-WiFi
#endif
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
float temperature = 0.0;

// MQTT Callback Function
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: [");
  Serial.print(topic);
  Serial.print("] ");
  
  // Convert payload to string
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  
  // Control fan based on command message payload (ON / OFF)
  if (String(topic) == subTopic) {
    if (message == "ON" || message == "on" || message == "1") {
      digitalWrite(FAN_RELAY_PIN, HIGH);
      Serial.println("MQTT COMMAND: Turned Fan ON");
    } else if (message == "OFF" || message == "off" || message == "0") {
      digitalWrite(FAN_RELAY_PIN, LOW);
      Serial.println("MQTT COMMAND: Turned Fan OFF");
    }
  }
}

// Reconnect helper
void reconnectMqtt() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    
    // Generate random Client ID to avoid duplication conflicts on public broker
    String clientId = "ESPClient-" + String(random(0xffff), HEX);
    
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
      
      // 1. สมัครรับข้อมูลคำสั่งควบคุมพัดลม (Subscribe) จากคลาวด์ภายนอก
      mqttClient.subscribe(subTopic);
      Serial.printf("Subscribed to: %s\n", subTopic);
    } else {
      Serial.printf("failed, rc=%d. Trying again in 5 seconds...\n", mqttClient.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // Set output GPIO 13
  pinMode(FAN_RELAY_PIN, OUTPUT);
  digitalWrite(FAN_RELAY_PIN, LOW); // Start with fan off
  
  // Connect Wi-Fi
  Serial.printf("Connecting to %s...", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected.");
  
  // Setup MQTT client
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(mqttCallback);
}

void loop() {
  // Re-establish MQTT connection if dropped
  if (!mqttClient.connected()) {
    reconnectMqtt();
  }
  mqttClient.loop();
  
  // 2. เผยแพร่ (Publish) ข้อมูลรายงานค่าอุณหภูมิขึ้นไปทุกๆ 5 วินาที
  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 5000) {
    lastMsg = millis();
    
    // Read temperature
    temperature = dht.readTemperature();
    
    if (!isnan(temperature)) {
      // Publish raw temperature string to MQTT broker
      String tempStr = String(temperature, 1);
      mqttClient.publish(pubTopic, tempStr.c_str());
      Serial.printf("MQTT Published: %s C -> Topic: %s\n", tempStr.c_str(), pubTopic);
    } else {
      Serial.println("DHT11 read failed, skipping MQTT publish");
    }
  }
  
  delay(10);
}
