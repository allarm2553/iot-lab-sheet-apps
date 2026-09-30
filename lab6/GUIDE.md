# คู่มือการเรียนรู้ Lab 6: การบูรณาการระบบ Hybrid Dual-Mode IoT Node (Local WebSockets & Cloud MQTT)

คู่มือนี้อธิบายขั้นตอนการพัฒนาและบูรณาการระบบ IoT ขั้นสูง โดยรวมจุดเด่นของ **Local WebSockets (Lab 4.1)** ที่ทำงานได้รวดเร็วแบบ 0-Latency ภายในวง LAN เข้ากับ **Cloud MQTT (LAB5_Dev)** ที่สามารถควบคุมข้ามเครือข่ายอินเทอร์เน็ตได้จากทุกที่ทั่วโลก ให้อยู่ร่วมกันบนบอร์ดไมโครคอนโทรลเลอร์ (ESP32 / ESP8266) พร้อมการควบคุมผ่านหน้าเว็บแดชบอร์ด **Progressive Web App (PWA)** แบบ Multi-Protocol

---

## สารบัญ
1. [ภาพรวมสถาปัตยกรรมระบบ Dual-Mode Coexistence](#1-ภาพรวมสถาปัตยกรรมระบบ-dual-mode-coexistence)
2. [การเชื่อมต่อวงจรฮาร์ดแวร์ (Hardware Pinouts)](#2-การเชื่อมต่อวงจรฮาร์ดแวร์)
3. [ขั้นตอนที่ 1: การเขียนโปรแกรมเฟิร์มแวร์ฝั่งบอร์ด (C++ / PlatformIO)](#3-ขั้นตอนที่-1-การเขียนโปรแกรมเฟิร์มแวร์ฝั่งบอร์ด)
4. [ขั้นตอนที่ 2: กลไก Bidirectional State Synchronization](#4-ขั้นตอนที่-2-กลไก-bidirectional-state-synchronization)
5. [ขั้นตอนที่ 3: การพัฒนาหน้าเว็บแดชบอร์ดและการติดตั้ง PWA](#5-ขั้นตอนที่-3-การพัฒนาหน้าเว็บแดชบอร์ดและการติดตั้ง-pwa)
6. [การทดสอบและการวิเคราะห์ผลการทำงาน](#6-การทดสอบและการวิเคราะห์ผลการทำงาน)

---

## 1. ภาพรวมสถาปัตยกรรมระบบ Dual-Mode Coexistence

ระบบ Hybrid IoT Node ทำหน้าที่เปิดบริการสื่อสาร 2 ช่องทางขนานกันบนอุปกรณ์ตัวเดียว:
1. **Local WebSockets Server (Port 81):** สื่อสารตรงกับเครื่องลูกข่าย (Clients) ในวง Wi-Fi เดียวกัน ตอบสนองทันทีแบบ 0-Latency แม้อินเทอร์เน็ตภายนอกจะล่ม
2. **Cloud MQTT Client (Port 1883 / WSS:8084):** เชื่อมต่อกับ Public MQTT Broker (`broker.emqx.io`) เพื่อรายงานค่าเซ็นเซอร์และรับคำสั่งจากระยะไกล

```mermaid
flowchart TD
    subgraph LAN [Local Area Network (ภายในบ้าน / โรงงาน)]
        DashboardLocal[Web Dashboard Local\nPort 80 / WS :81]
        ESP32[ESP32 Hybrid Node\nDHT11 + Relays + OLED]
    end

    subgraph WAN [Cloud & Internet (ภายนอก)]
        Broker((Cloud MQTT Broker\nbroker.emqx.io:1883))
        DashboardCloud[Web Dashboard Cloud\nWSS :8084 PWA App]
    end

    DashboardLocal <-->|WebSocket :81\n0-Latency| ESP32
    ESP32 <-->|MQTT :1883\nTelemetry & Control| Broker
    Broker <-->|MQTT over WSS :8084| DashboardCloud
```

---

## 2. การเชื่อมต่อวงจรฮาร์ดแวร์

| อุปกรณ์ | ขาพิน ESP32 (IPST-WiFi) | ขาพิน ESP8266 (AX-WiFi) | หน้าที่การทำงาน |
| :--- | :--- | :--- | :--- |
| **DHT11 Sensor** | GPIO 33 | GPIO 0 (D3) | วัดอุณหภูมิและความชื้น |
| **Analog VR (Soil)** | GPIO 36 (VP) | A0 (ADC0) | จำลองเซ็นเซอร์ความชื้นในดิน |
| **Relay 1 (พัดลม)** | GPIO 18 (หรือ GPIO 5) | GPIO 14 (D5) | รีเลย์ควบคุมพัดลมระบายอากาศ |
| **Relay 2 (พ่นหมอก)** | GPIO 19 (หรือ GPIO 13) | GPIO 12 (D6) | รีเลย์ควบคุมปั๊มน้ำพ่นหมอก |
| **OLED Display** | SDA: 21, SCL: 22 | SDA: 4, SCL: 5 | แสดงผลค่าและสถานะการเชื่อมต่อ |

---

## 3. ขั้นตอนที่ 1: การเขียนโปรแกรมเฟิร์มแวร์ฝั่งบอร์ด

### การประกาศตัวแปรและการจัดการพอร์ต:
```cpp
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <DHT.h>

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
WiFiClient espClient;
PubSubClient mqttClient(espClient);
```

### การจัดการ Non-blocking Reconnect Loop ใน `loop()`:
```cpp
void loop() {
  server.handleClient();
  webSocket.loop();
  
  // ตรวจสอบและเชื่อมต่อ MQTT ใหม่แบบ Non-blocking (ห้ามใช้ while / delay)
  static unsigned long lastMqttCheck = 0;
  if (!mqttClient.connected() && (millis() - lastMqttCheck > 5000)) {
    lastMqttCheck = millis();
    String clientId = "ESP32-Hybrid-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (mqttClient.connect(clientId.c_str())) {
      mqttClient.subscribe("esp32-climate-node/control/cmd");
    }
  }
  if (mqttClient.connected()) {
    mqttClient.loop();
  }

  // Periodic Heartbeat ส่งค่าทุก 5 วินาที
  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 5000) {
    lastMsg = millis();
    broadcastHybridState();
  }
}
```

---

## 4. ขั้นตอนที่ 2: กลไก Bidirectional State Synchronization

เมื่อมีคำสั่งเปลี่ยนสถานะรีเลย์ (ไม่ว่าจะสั่งผ่าน Local WebSocket หรือ Cloud MQTT) ฟังก์ชัน `broadcastHybridState()` จะถูกเรียกใช้งานทันที เพื่อส่งสถานะใหม่กระจายไปให้ทุกช่องทาง:

```cpp
void broadcastHybridState() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  
  JsonDocument doc;
  doc["temp"] = isnan(t) ? 0 : t;
  doc["humidity"] = isnan(h) ? 0 : h;
  doc["fan"] = fanState ? 1 : 0;
  doc["mist"] = mistState ? 1 : 0;
  doc["mqtt"] = mqttClient.connected();
  
  String payload;
  serializeJson(doc, payload);
  
  // 1. Broadcast ส่งหา Client ทุกเครื่องบนวง LAN
  webSocket.broadcastTXT(payload);
  
  // 2. Publish ส่งขึ้น Cloud MQTT Broker
  if (mqttClient.connected()) {
    mqttClient.publish("esp32-climate-node/state", payload.c_str());
  }
}
```

---

## 5. ขั้นตอนที่ 3: การพัฒนาหน้าเว็บแดชบอร์ดและการติดตั้ง PWA

1. **ดาวน์โหลดชุดไฟล์ Web UI:** กดปุ่ม **"โหลด Web UI (data.zip)"** บนหน้าใบงาน
2. **แตกไฟล์ลงในโฟลเดอร์ `data/`:** จะประกอบด้วย `index.html`, `styles.css`, `manifest.json`, และ `sw.js`
3. **การติดตั้งใช้งาน PWA (Progressive Web App):**
   * บน **Google Chrome / Edge (Desktop):** กดไอคอนรูปคอมพิวเตอร์พร้อมลูกศรดาวน์โหลดบนแถบ Address Bar เพื่อติดตั้งเป็น Standalone App
   * บน **สมาร์ตโฟน (Android / iOS):** กดเมนูตัวเลือก (3 จุด) -> เลือก **"เพิ่มลงในหน้าจอหลัก (Add to Home Screen)"** หรือ **"ติดตั้งแอป (Install App)"**

---

## 6. การทดสอบและการวิเคราะห์ผลการทำงาน

* **การทดสอบ Zero-Latency:** สั่งงานผ่าน Local WebSocket (Port 81) จะพบว่ารีเลย์ทำงานทันทีโดยไม่ต้องพึ่งพาอินเทอร์เน็ต
* **การทดสอบ Fault Tolerance (WAN Offline):** เมื่อถอดสายอินเทอร์เน็ตของ Router ออก บอร์ดจะยังคงทำงานในโหมด Local WebSocket ต่อไปได้ 100% โดยไม่เกิดอาการค้าง เนื่องจากใช้ Non-blocking Reconnect Timer
