# 📶 ใบงานที่ 3: การตรวจหา WiFi, พารามิเตอร์สถานะ และระบบ Auto-Reconnect แบบ Event-Driven

คู่มือการทดลองสั่งงานโมดูล Wi-Fi ของ ESP32 และ ESP8266 ในการสแกนหาเครือข่ายไร้สาย (WiFi Scanner), การวิเคราะห์พารามิเตอร์เครือข่าย (IP, MAC, RSSI, Status Constants), และการใช้ฟังก์ชันพิเศษ **Event-Driven Auto-Reconnect** ดักจับเหตุการณ์สัญญาณหลุดโดยไม่ต้องวนลูปตรวจสอบ

---

## 🎯 1. วัตถุประสงค์การเรียนรู้ (Objectives)

1. เข้าใจโหมดการทำงานของ Wi-Fi ได้แก่ **Station Mode (WIFI_STA)**, **Access Point Mode (WIFI_AP)** และ **AP+STA Mode (WIFI_AP_STA)**
2. เข้าใจและสามารถอ่านค่า **รหัสสถานะการเชื่อมต่อ Wi-Fi (`wl_status_t`)** เช่น `WL_CONNECTED`, `WL_DISCONNECTED`, `WL_CONNECT_FAILED`
3. สามารถดึงพารามิเตอร์เครือข่ายสำคัญ ได้แก่ **Local IP, Subnet Mask, Gateway, DNS, MAC Address, BSSID และ RSSI (dBm)**
4. เข้าใจข้อจำกัดของการ Polling สถานะใน `loop()` และสามารถประยุกต์ใช้ **Event-Driven Callback Architecture**:
   - **ESP32:** `WiFi.onEvent()` ดักจับ `ARDUINO_EVENT_WIFI_STA_DISCONNECTED` และ `GOT_IP`
   - **ESP8266:** `WiFiEventHandler` ดักจับ `onStationModeDisconnected()` และ `onStationModeGotIP()`
   เพื่อเชื่อมต่อเครือข่ายใหม่อัตโนมัติในระดับ System Task โดยไม่บล็อกการทำงานของ Main Loop

---

## 📊 2. ทฤษฎีพารามิเตอร์และการแปลความหมายสถานะ Wi-Fi

### 2.1 ตารางรหัสสถานะ `WiFi.status()` (`wl_status_t`)

ฟังก์ชัน `WiFi.status()` จะคืนค่าเป็นตัวเลข Enumeration เพื่อบอกสถานะปัจจุบันของโมดูล Wi-Fi:

| รหัสตัวเลข | ค่าคงที่ Enum | ความหมาย | สถานะการทำงาน |
| :---: | :--- | :--- | :--- |
| **3** | `WL_CONNECTED` | เชื่อมต่อสำเร็จและได้รับ IP Address จากเราเตอร์แล้ว | ใช้งานเครือข่าย/ส่งข้อมูลได้ทันที |
| **6** | `WL_DISCONNECTED` | หลุดการเชื่อมต่อ หรือถูกสั่งตัดการเชื่อมต่อจาก AP | เกิดขึ้นเมื่อ Access Point ปิดตัว หรือสัญญาณหลุด |
| **4** | `WL_CONNECT_FAILED` | การเชื่อมต่อล้มเหลว | มักเกิดจาก **รหัสผ่าน Wi-Fi ผิด** หรือ AP ปฏิเสธ |
| **1** | `WL_NO_SSID_AVAIL` | ไม่พบชื่อ SSID เป้าหมาย | เกิดขึ้นเมื่อตั้งชื่อ Wi-Fi ผิด หรืออยู่นอกระยะสัญญาณ |
| **5** | `WL_CONNECTION_LOST` | สัญญาณขาดหายระหว่างการเชื่อมต่อ | สัญญาณอ่อนมาก (RSSI ต่ำกว่า -85 dBm) |
| **0** | `WL_IDLE_STATUS` | อยู่ในสถานะพัก / กำลังสลับโหมด | ค่าชั่วคราวระหว่างรอคำสั่งใหม่ |
| **2** | `WL_SCAN_COMPLETED` | สแกนหาเครือข่ายเสร็จสมบูรณ์ | พร้อมเรียกดูรายการเครือข่ายที่พบ |
| **255** | `WL_NO_SHIELD` | ไม่พบชิป Wi-Fi | ฮาร์ดแวร์ขัดข้องหรือไม่รองรับ |

---

### 2.2 ตารางพารามิเตอร์เครือข่ายและการใช้งาน (Network Parameters)

| คำสั่ง API | เลเยอร์ (OSI Model) | ตัวอย่างค่าที่ได้รับ | ประโยชน์ในการพัฒนาระบบ IoT |
| :--- | :--- | :--- | :--- |
| `WiFi.localIP()` | Network (Layer 3) | `192.168.1.145` | หมายเลข IP บนวง LAN สำหรับเปิดหน้าเว็บหรือรับคำสั่ง |
| `WiFi.subnetMask()` | Network (Layer 3) | `255.255.255.0` | กำหนดขอบเขตขนาดของเครือข่าย Local Subnet |
| `WiFi.gatewayIP()` | Network (Layer 3) | `192.168.1.1` | IP ของเราเตอร์หลักที่ใช้ส่งข้อมูลออกสู่อินเทอร์เน็ต |
| `WiFi.dnsIP()` | Application (Layer 7) | `8.8.8.8` | เซิร์ฟเวอร์แปลงชื่อโดเมน (เช่น Google DNS) |
| `WiFi.RSSI()` | Physical (Layer 1) | `-58 dBm` | ความแรงสัญญาณวิทยุ (ยิ่งเข้าใกล้ 0 ยิ่งแรง) |
| `WiFi.macAddress()` | Data Link (Layer 2) | `24:0A:C4:58:3B:1C` | Hardware ID ไม่ซ้ำกันในโลก ใช้เป็น Device UUID ใน Cloud |
| `WiFi.BSSIDstr()` | Data Link (Layer 2) | `A4:2B:B0:E3:71:00` | MAC Address ของ Access Point ที่เราเกาะอยู่ |
| `WiFi.channel()` | Physical (Layer 1) | `6` (ช่อง 1–13) | ช่องความถี่วิทยุ 2.4 GHz เพื่อหลีกเลี่ยงสัญญาณรบกวน |

---

## ⚡ 3. สถาปัตยกรรม Event-Driven vs Status Polling

```text
[วิธีแบบเดิม: Status Polling ใน loop]
loop() ──> เช็ค if (WiFi.status() != WL_CONNECTED) ──> สั่ง delay(500) ──> สั่ง reconnect ──> [CPU เสียรอบการทำงาน]

[วิธีขั้นสูง: Event-Driven Callback Handler]
WiFi Stack (เบื้องหลัง) ── [เกิด Event หลุดสัญญาณ] ──> เรียก Interrupt Callback อัตโนมัติ ──> reconnect ทันที
loop()                  ── [ทำงานเซนเซอร์ / หน้าจอ / ควบคุม Relay ได้ต่อเนื่อง ไม่ต้องเสียเวลาเช็คสถานะ]
```

### การเขียนโค้ด Event-Driven:
1. **ESP32:**
   ```cpp
   WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
     if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
       Serial.println("[Event] สัญญาณหลุด! เชื่อมต่อใหม่ใน Background...");
       WiFi.reconnect();
     } else if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
       Serial.print("[Event] ได้รับ IP: ");
       Serial.println(WiFi.localIP());
     }
   });
   ```
2. **ESP8266:**
   ```cpp
   WiFiEventHandler disconnectHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& evt) {
     Serial.println("[Event] Disconnected! Reconnecting...");
     WiFi.reconnect();
   });
   ```

---

## 📁 4. โครงสร้างโปรเจกต์ (Project Structure)

```text
lab3/
 ├── platformio.ini         (การตั้งค่า PlatformIO)
 ├── diagram.json           (ไดอะแกรมวงจรจำลอง Wokwi)
 ├── wokwi.toml             (การตั้งค่า Wokwi Simulator Runner)
 ├── index.html             (เว็บแอปพลิเคชันใบงานออนไลน์พร้อมระบบ Auto-Grader)
 ├── Code.gs                (สคริปต์ Google Apps Script ตรวจคะแนนและบันทึก Google Sheets)
 ├── GUIDE.md               (คู่มือและเฉลยการทดลอง)
 └── solution/
      ├── platformio.ini
      ├── diagram.json
      ├── wokwi.toml
      ├── lab3_solution.ino (ไฟล์โปรแกรมฉบับสมบูรณ์สำหรับ Arduino IDE)
      └── src/
           └── main.cpp     (ไฟล์โปรแกรมฉบับสมบูรณ์สำหรับ PlatformIO)
```

---

## 🧠 5. เฉลยแบบทดสอบปรนัย (Quiz Answer Keys)

* **ข้อที่ 1 (ตอบ ก - 1a):** โหมด `WIFI_STA` ทำหน้าที่เป็น Client เชื่อมต่อเข้ากับ Access Point ที่มีอยู่แล้วเพื่อออกสู่อินเทอร์เน็ต
* **ข้อที่ 2 (ตอบ ค - 2c):** สัญญาณ `-50 dBm` มีความแรงสูงกว่า `-85 dBm` (เนื่องจากค่า dBm ยิ่งมีค่าติดลบน้อย จะยิ่งมีกำลังส่งสูง)
* **ข้อที่ 3 (ตอบ ข - 3b):** ค่าคงที่ `WL_CONNECTED` มีค่าเป็นตัวเลขเท่ากับ 3 ซึ่งหมายความว่าบอร์ดเชื่อมต่อสำเร็จและได้รับ IP จาก DHCP แล้ว
* **ข้อที่ 4 (ตอบ ง - 4d):** สถาปัตยกรรมแบบ **Event-Driven (Callback)** ช่วยให้ระบบดักจับการหลุดของสัญญาณและเชื่อมต่อใหม่ได้ในระดับเบื้องหลังทันที โดยไม่ต้องให้ฟังก์ชัน `loop()` เสียเวลาคอยเช็คสถานะ
* **ข้อที่ 5 (ตอบ ข - 5b):** `WiFi.macAddress()` คือหมายเลขประจำตัวฮาร์ดแวร์ระดับ Layer 2 (Data Link) ไม่ซ้ำกันในโลก นิยมใช้เป็น Device ID ระบุตัวตนบอร์ด IoT บน Cloud

---

## ✍️ 6. เฉลยคำถามวิเคราะห์เชิงลึก

1. **ค่า RSSI คืออะไร และมีผลอย่างไรต่อการติดตั้งอุปกรณ์ IoT?**
   > RSSI (Received Signal Strength Indication) มีหน่วยเป็น dBm เป็นค่ากำลังของคลื่นวิทยุที่บอร์ดรับได้ ยิ่งติดลบน้อย (เช่น -40 ถึง -60 dBm) ยิ่งเสถียร หากค่าต่ำกว่า -80 dBm แพ็กเก็ตข้อมูลจะเริ่มสูญหาย (Packet Loss) ทำให้อุปกรณ์หลุดบ่อย จึงต้องติดตั้งอุปกรณ์ในจุดที่มี RSSI ไม่ต่ำกว่า -70 dBm
2. **ทำไมระบบ Event-Driven Auto-Reconnect จึงมีประสิทธิภาพสูงกว่าการเขียนคำสั่ง polling ตรวจสอบสถานะใน loop()?**
   > การ Polling ใน `loop()` ทำให้สูญเสียรอบประมวลผล CPU และหากมีคำสั่ง `delay()` อื่นๆ ขวางอยู่ จะทำให้การตรวจจับสัญญาณหลุดล่าช้า ขณะที่ Event-Driven ทำงานแบบ Asynchronous ขับเคลื่อนด้วย System Event Callback ทันทีที่เกิดเหตุการณ์ ทำให้กู้คืนสัญญาณได้เร็วกว่าและไม่กระทบการทำงานหลักของโปรแกรม
3. **การดึง MAC Address มีประโยชน์อย่างไรในการพัฒนาระบบ IoT เชิงพาณิชย์?**
   > MAC Address เป็นค่าประจำตัวฮาร์ดแวร์ชิปที่ถูกกำหนดจากโรงงานและไม่ซ้ำกันในโลก ทำให้สามารถนำมาใช้เป็น **Unique Client ID / Device Serial Number** ในการลงทะเบียนบนระบบคลาวด์ (เช่น MQTT Client ID, Firebase Node ID) โดยไม่ต้องสร้างรหัสขึ้นมาเอง
