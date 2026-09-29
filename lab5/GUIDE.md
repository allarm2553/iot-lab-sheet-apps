# คู่มือการเรียนรู้ Lab 5: Cloud MQTT Protocol & Bidirectional Testing with MQTTBox / MQTT Explorer

คู่มือนี้จะอธิบายขั้นตอนการพัฒนาระบบ IoT โดยใช้โปรโตคอล **Cloud MQTT (Message Queuing Telemetry Transport)** ตั้งแต่การเขียนโค้ดบอร์ดไมโครคอนโทรลเลอร์ (ESP32 / ESP8266) การเชื่อมต่อกับ Cloud Broker สาธารณะ (`broker.emqx.io`) ไปจนถึงการทดสอบการรับ-ส่งข้อมูลสองทิศทาง (Bidirectional Pub/Sub) ผ่านโปรแกรมทดสอบมาตรฐาน เช่น **MQTTBox, MQTT Explorer หรือ EMQX Online Client**

---

## สารบัญ
1. [ภาพรวมของสถาปัตยกรรมและโปรโตคอล MQTT](#1-ภาพรวมของสถาปัตยกรรมและโปรโตคอล-mqtt)
2. [การเชื่อมต่อวงจรฮาร์ดแวร์ (ESP32, DHT11 & OLED)](#2-การเชื่อมต่อวงจรฮาร์ดแวร์)
3. [ขั้นตอนที่ 1: การเขียนโปรแกรมฝั่งบอร์ด (C++ / PlatformIO)](#3-ขั้นตอนที่-1-การเขียนโปรแกรมฝั่งบอร์ด)
4. [ขั้นตอนที่ 2: การติดตั้งและใช้งานโปรแกรมทดสอบ MQTTBox](#4-ขั้นตอนที่-2-การติดตั้งและใช้งานโปรแกรมทดสอบ-mqttbox)
5. [ขั้นตอนที่ 3: การทดสอบรับส่งข้อมูลและควบคุมโหลดจริง](#5-ขั้นตอนที่-3-การทดสอบรับส่งข้อมูลและควบคุมโหลดจริง)
6. [การวิเคราะห์ปัญหาและการประเมินผล](#6-การวิเคราะห์ปัญหาและการประเมินผล)

---

## 1. ภาพรวมของสถาปัตยกรรมและโปรโตคอล MQTT

ในการทดลองนี้ อุปกรณ์ฮาร์ดแวร์จะส่งค่าเซ็นเซอร์ (อุณหภูมิ, ความชื้น, แอนะล็อกเปอร์เซ็นต์, จำนวนครั้งกดสวิตช์) ขึ้นไปยังอินเทอร์เน็ตผ่าน **Cloud MQTT Broker (`broker.emqx.io:1883`)** ในรูปแบบของข้อความโครงสร้าง JSON และรับข้อสั่งการจากโปรแกรมทดสอบภายนอกกลับไปควบคุมรีเลย์และ LED

```mermaid
flowchart LR
    ESP[ESP32 / ESP8266 Node] -->|Publish Telemetry\nesp-node/state (JSON)| Broker((Cloud MQTT Broker\nbroker.emqx.io:1883))
    Broker -->|Subscribe State| Tester[MQTT Testing Tool\nMQTTBox / MQTT Explorer]
    Tester -->|Publish Command\nesp-node/control/cmd| Broker
    Broker -->|Callback Message| ESP
```

### การตั้งค่าการสื่อสาร (MQTT Topics & Schemas)
* **บอร์ดส่งรายงานสถานะ (Telemetry Publish):**
  * Topic: `esp-node/state` (หรือ `kku/iot/lab5/state`)
  * Interval: ทุก 5 วินาที
  * Payload ตัวอย่าง:
    ```json
    {
      "temp": 28.5,
      "hum": 65.0,
      "fan": 1,
      "mist": 0,
      "soil": 45.2,
      "press": 3,
      "mode": true
    }
    ```
* **บอร์ดรับคำสั่งควบคุม (Command Subscribe):**
  * Topic: `esp-node/control/cmd` (หรือ `kku/iot/lab5/control`)
  * Payload ตัวอย่าง:
    ```json
    {"action": "toggle_fan", "value": true}
    ```

---

## 2. การเชื่อมต่อวงจรฮาร์ดแวร์

| อุปกรณ์ | ขาพิน ESP32 (IPST-WiFi) | ขาพิน ESP8266 (NodeMCU) | รายละเอียด |
| :--- | :--- | :--- | :--- |
| **DHT11 Sensor** | GPIO 33 (หรือ GPIO 0) | GPIO 0 (D3) | เซ็นเซอร์วัดอุณหภูมิและความชื้น |
| **Analog VR (Knob)** | GPIO 36 (Sensor VP) | ADC0 (A0) | ตัวต้านทานปรับค่าได้จำลองเซ็นเซอร์ดิน |
| **Relay 1 (พัดลม)** | GPIO 18 | GPIO 14 (D5) | รีเลย์ควบคุมโหลด 1 |
| **Relay 2 (พ่นหมอก)** | GPIO 19 | GPIO 12 (D6) | รีเลย์ควบคุมโหลด 2 |
| **OLED Display (I2C)** | SDA: GPIO 21 / SCL: GPIO 22 | SDA: GPIO 4 / SCL: GPIO 5 | จอแสดงผล I2C Address 0x3C |

---

## 3. ขั้นตอนที่ 1: การเขียนโปรแกรมฝั่งบอร์ด

### การคอนฟิกการเชื่อมต่อในโค้ด:
```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

const char* mqttServer = "broker.emqx.io";
const int mqttPort = 1883;
const char* pubTopic = "esp-node/state";
const char* subTopic = "esp-node/control/cmd";

WiFiClient espClient;
PubSubClient mqttClient(espClient);
```

### การทำงานของฟังก์ชัน Reconnect ป้องกัน Client ID ชนกัน:
```cpp
void reconnectMqtt() {
  while (!mqttClient.connected()) {
    // ใช้ MAC Address เป็น Client ID เพื่อป้องกันการหลุดจากการใช้ ID ซ้ำ
    String clientId = "ESP32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("MQTT Connected!");
      mqttClient.subscribe(subTopic);
    } else {
      delay(2000);
    }
  }
}
```

---

## 4. ขั้นตอนที่ 2: การติดตั้งและใช้งานโปรแกรมทดสอบ MQTTBox

1. **ดาวน์โหลดและติดตั้ง MQTTBox:** (หรือใช้ MQTT Explorer / EMQX Online WebSocket Client)
2. **สร้างการเชื่อมต่อ Client ใหม่ (Create MQTT Client):**
   * **Client Name:** `Lab5-Tester`
   * **Protocol:** `mqtt / tcp`
   * **Host:** `broker.emqx.io:1883`
   * **Client Id:** `Tester-Student-512` (ตั้งชื่อไม่ให้ซ้ำกับผู้อื่น)
3. **กดปุ่ม Save และ Connect:** ตรวจสอบให้สถานะขึ้นเป็นแถบสีเขียว **Connected**

---

## 5. ขั้นตอนที่ 3: การทดสอบรับส่งข้อมูลและควบคุมโหลดจริง

### 5.1 การ Subscribe ตรวจสอบข้อมูลเซ็นเซอร์
1. ในกล่อง **Add subscriber** กรอก Topic `esp-node/state`
2. เลือก QoS `0 - Almost once` แล้วกดปุ่ม **Subscribe**
3. ข้อความ JSON ที่บอร์ด ESP32 ส่งมาจะแสดงผลทุก 5 วินาที พร้อมค่าอุณหภูมิและความชื้น

### 5.2 การ Publish สั่งการเปิด-ปิดรีเลย์บนบอร์ด
1. ในกล่อง **Publish to topic** กรอก Topic `esp-node/control/cmd`
2. ใส่ Payload ข้อความ JSON:
   ```json
   {"action": "toggle_fan", "value": true}
   ```
3. กดปุ่ม **Publish** สังเกตรีเลย์พัดลมและหลอดไฟบนบอร์ดจริงทำงานทันที
4. ทดสอบส่งคำสั่งปิด:
   ```json
   {"action": "toggle_fan", "value": false}
   ```
5. แคปเจอร์ภาพหน้าจอผลการทดสอบทั้งสองส่วนเพื่อแนบในรายงานการทดลอง

---

## 6. การวิเคราะห์ปัญหาและการประเมินผล

* **ปัญหา Client Disconnected วนลูป:** ตรวจสอบว่าไม่ได้ตั้ง Client ID ซ้ำกับผู้เรียนคนอื่น
* **ไม่ได้รับ Callback:** ตรวจสอบว่าได้เรียกคำสั่ง `mqttClient.loop()` ในฟังก์ชัน `loop()` อย่างต่อเนื่องโดยไม่มีคำสั่ง `delay()` มาบล็อกการทำงาน
