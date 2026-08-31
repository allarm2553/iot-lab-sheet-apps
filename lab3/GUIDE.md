# 📶 ใบงานที่ 3: การตรวจหา WiFi, โหมดการทำงาน, พารามิเตอร์สถานะ และระบบ Auto-Reconnect แบบ Event-Driven

คู่มือการทดลองสั่งงานโมดูล Wi-Fi ของ ESP32 และ ESP8266 ในการทำความเข้าใจโหมดการทำงาน (**STA**, **AP**, **AP_STA**), การสแกนหาเครือข่ายไร้สาย (WiFi Scanner), การวิเคราะห์พารามิเตอร์เครือข่าย (IP, MAC, RSSI, Status Enum), และการใช้ฟังก์ชันพิเศษ **Event-Driven Auto-Reconnect** ดักจับเหตุการณ์สัญญาณหลุดโดยไม่ต้องวนลูปตรวจสอบ

---

## 🎯 1. วัตถุประสงค์การเรียนรู้ (Objectives)

1. เข้าใจโหมดการทำงานของ Wi-Fi ได้แก่ **Station Mode (`WIFI_STA`)**, **Access Point Mode (`WIFI_AP`)**, **Dual Mode (`WIFI_AP_STA`)** และ **(`WIFI_OFF`)**
2. เข้าใจและสามารถอ่านค่า **รหัสสถานะการเชื่อมต่อ Wi-Fi (`wl_status_t`)** เช่น `WL_CONNECTED`, `WL_DISCONNECTED`, `WL_CONNECT_FAILED`
3. สามารถสแกนหาเครือข่ายไร้สาย `WiFi.scanNetworks()` และดึงค่าความแรงสัญญาณ **RSSI (dBm)**, หมายเลขช่องสัญญาณ (Channel 1–13) และมาตรฐานความปลอดภัย
4. สามารถดึงพารามิเตอร์เครือข่ายสำคัญ ได้แก่ **Local IP, Subnet Mask, Gateway, DNS, MAC Address, BSSID**
5. เข้าใจข้อจำกัดของการ Polling สถานะใน `loop()` และสามารถประยุกต์ใช้ **Event-Driven Callback Architecture**:
   - **ESP32:** `WiFi.onEvent()` ดักจับ `ARDUINO_EVENT_WIFI_STA_DISCONNECTED` และ `GOT_IP`
   - **ESP8266:** `WiFiEventHandler` ดักจับ `onStationModeDisconnected()` และ `onStationModeGotIP()`
   เพื่อเชื่อมต่อเครือข่ายใหม่อัตโนมัติในระดับ System Task โดยไม่บล็อกการทำงานของ Main Loop

---

## 🌐 2. ทฤษฎีโหมดการทำงานของ Wi-Fi (Wi-Fi Operational Modes)

ชิปไมโครคอนโทรลเลอร์ ESP32 และ ESP8266 บรรจุตัวรับส่งสัญญาณคลื่นวิทยุมาตรฐาน IEEE 802.11 b/g/n ย่านความถี่ 2.4 GHz โดยสามารถกำหนดโหมดการทำงานได้ผ่าน `WiFi.mode()`:

1. **Station Mode (`WIFI_STA`):**
   - บอร์ดทำหน้าที่เป็น **Client (ลูกข่าย)** เชื่อมต่อไปยัง Access Point หรือ Wi-Fi Router หลัก
   - ร้องขอหมายเลข IP ผ่านโพรโทคอล DHCP เพื่อรับส่งข้อมูลกับเครื่องเซิร์ฟเวอร์บน Local LAN หรือ Cloud Internet
2. **Access Point Mode (`WIFI_AP`):**
   - บอร์ดทำหน้าที่เป็น **Host (ศูนย์กลางกระจายสัญญาณ)** ปล่อย Wi-Fi Hotspot ของตัวเองออกมา
   - สมาร์ตโฟนหรือคอมพิวเตอร์สามารถเชื่อมต่อเข้ามายังบอร์ดได้โดยตรง โดยบอร์ดจะแจก IP ให้ Client (เช่น กำหนด IP ตัวเองเป็น `192.168.4.1`)
3. **Dual Mode (`WIFI_AP_STA`):**
   - บอร์ดเปิดใช้งานทั้งโหมด AP และ STA พร้อมกัน
   - นิยมใช้ในระบบ **Wi-Fi Provisioning / Captive Portal** เพื่อให้ผู้ใช้เข้ามาตั้งค่ารหัสผ่าน Wi-Fi ผ่านหน้าเว็บ ในขณะที่บอร์ดยังสแกนหาเครือข่ายภายนอกได้
4. **Wi-Fi Off (`WIFI_OFF`):**
   - ปิดภาควิทยุทั้งหมดเพื่อประหยัดพลังงานแบตเตอรี่สูงสุด เหมาะกับงาน Deep Sleep

---

## 📊 3. ทฤษฎีการสแกน, ค่า RSSI, พารามิเตอร์เครือข่าย และรหัสสถานะ

### 3.1 การสแกนหาเครือข่ายและค่าความแรงสัญญาณ RSSI (dBm)
ฟังก์ชัน `WiFi.scanNetworks()` ค้นหาคลื่นวิทยุรอบตัวทั้ง 13 ช่องสัญญาณ (Channels 1–13 ย่าน 2412–2472 MHz) เพื่อดึงข้อมูล:
- **SSID:** ชื่อเครือข่าย
- **RSSI (Received Signal Strength Indicator):** หน่วยเป็น **dBm (Decibel-milliwatts)** ซึ่งเป็นค่ากำลังงานที่ภาครับตรวจจับได้เทียบกับ 1 mW ตามสเกลลอการิทึม เนื่องจากสัญญาณวิทยุมีกำลังต่ำมากในระดับไมโครวัตต์ จึงแสดงผลเป็นค่าติดลบเสมอ:
  - $\ge -50\text{ dBm}$: สัญญาณดีเยี่ยม (Excellent Signal - ใกล้ Router มาก)
  - $\ge -65\text{ dBm}$: สัญญาณดี เหมาะสำหรับงาน IoT ส่งข้อมูลต่อเนื่อง (Good Signal)
  - $\ge -75\text{ dBm}$: สัญญาณระดับปานกลาง/พอใช้ (Fair Signal)
  - $< -80\text{ dBm}$: สัญญาณอ่อนมาก อาจเกิด Packet Loss หรือหลุดบ่อย (Poor Signal)
- **Security Type:** การเข้ารหัสความปลอดภัย เช่น `OPEN`, `WEP`, `WPA_PSK`, `WPA2_PSK`, `WPA3_SAE`

---

### 3.2 ตารางรหัสสถานะ `WiFi.status()` (`wl_status_t`)

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

### 3.3 ตารางพารามิเตอร์เครือข่ายและการใช้งาน (Network Parameters)

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

## ⚡ 4. สถาปัตยกรรม Event-Driven vs Status Polling

```text
[วิธีแบบเดิม: Status Polling ใน loop]
loop() ──> เช็ค if (WiFi.status() != WL_CONNECTED) ──> สั่ง delay(500) ──> สั่ง reconnect ──> [CPU เสียรอบการทำงาน]

[วิธีขั้นสูง: Event-Driven Callback Handler]
WiFi Stack (เบื้องหลัง) ── [เกิด Event หลุดสัญญาณ] ──> เรียก Interrupt Callback อัตโนมัติ ──> reconnect ทันที
loop()                  ── [ทำงานเซนเซอร์ / หน้าจอ / ควบคุม Relay ได้ต่อเนื่อง ไม่ต้องเสียเวลาเช็คสถานะ]
```

### ตารางเปรียบเทียบเชิงลึก:

| มิติการเปรียบเทียบ | 🔴 Status Polling ใน loop() | 🟢 Event-Driven Callback |
| :--- | :--- | :--- |
| **สถาปัตยกรรม** | **Synchronous / Polling** (ถาม-ตอบเป็นรอบ) | **Asynchronous / Interrupt-driven** (แจ้งเตือนเมื่อเกิดเหตุ) |
| **ภาระ CPU (Overhead)** | ❌ **สิ้นเปลืองรอบ CPU** ในการเรียก `WiFi.status()` ซ้ำๆ | ✅ **Zero CPU Overhead** ใน Main Thread (ทำงานเฉพาะเมื่อสถานะเปลี่ยนจริง) |
| **ความเร็วในการกู้คืนสัญญาณ** | ⚠️ **ช้ากว่า** ขึ้นกับรอบเวลา (เช่น เช็คทุก 5-10s) หรือติด Delay | ⚡ **รวดเร็วทันที (Real-time)** สั่งต่อใหม่ในหลักมิลลิวินาที |
| **ความเสี่ยงที่ระบบหลักจะค้าง** | ❌ **เสี่ยงสูง** หากติด `while` หรือ `delay` งานควบคุมรีเลย์จะหยุดชะงัก | ✅ **ปลอดภัย 100% (Non-blocking)** ทำงานในระดับ OS/Driver Background |
| **ความสะอาดของโค้ด** | ⚠️ โค้ดใน `loop()` ปะปนระหว่างงาน Application กับ Network | ✅ โค้ดใน `loop()` สะอาดมาก แยก Network Handler ไว้ใน `setup()` |
| **การนำไปใช้งานจริง** | เหมาะสำหรับผู้เริ่มต้นศึกษาหลักการทำงาน | **มาตรฐานสำหรับอุปกรณ์ IoT เชิงพาณิชย์ (Production Grade)** |

### การเขียนโค้ด Event-Driven:
1. **ESP32:**
   ```cpp
   WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
     if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
       Serial.println("[Event] สัญญาณหลุด! เชื่อมต่อใหม่ใน Background...");
       WiFi.reconnect();
     } else if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
       Serial.printf("[Event] ได้รับ IP: %s (RSSI: %d dBm)\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
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

## 📁 5. โครงสร้างโปรเจกต์ (Project Structure)

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

## 🧠 6. เฉลยแบบทดสอบปรนัย (Quiz Answer Keys)

* **ข้อที่ 1 (ตอบ ค - 1c):** ค่า RSSI มีหน่วยเป็น dBm เป็นค่าติดลบเทียบกับ 1 mW ตามสเกลลอการิทึม โดยสัญญาณที่เข้าใกล้ 0 มากกว่า (เช่น -45 dBm) จะแรงกว่าค่าที่ลบมาก (เช่น -85 dBm)
* **ข้อที่ 2 (ตอบ ข - 2b):** โหมด `WIFI_STA` ทำให้อุปกรณ์ทำหน้าที่เป็นลูกข่าย (Client) เกาะ Wi-Fi Router ส่วนโหมด `WIFI_AP` ทำให้อุปกรณ์ปล่อยสัญญาณ Wi-Fi ให้เครื่องอื่นเข้ามาเชื่อมต่อ
* **ข้อที่ 3 (ตอบ ก - 3a):** ฟังก์ชัน `WiFi.scanNetworks()` สแกนหาคลื่น Wi-Fi ในบริเวณใกล้เคียงและส่งค่ากลับเป็นจำนวนเครือข่ายที่ค้นพบ (int)
* **ข้อที่ 4 (ตอบ ค - 4c):** ฟังก์ชัน `WiFi.status()` ส่งคืนค่าคงที่ `WL_CONNECTED` มีค่าตัวเลขคือ 3 หมายถึงเชื่อมต่อสำเร็จและได้รับ IP เรียบร้อย
* **ข้อที่ 5 (ตอบ ข - 5b):** แนวทางปฏิบัติที่ดีที่สุดคือใช้สถาปัตยกรรม **Event-Driven Callback** (เช่น `WiFi.onEvent()` บน ESP32 หรือ `WiFiEventHandler` บน ESP8266) เพื่อให้ระบบเชื่อมต่อใหม่อัตโนมัติในระดับ Background Task

---

## ✍️ 7. เฉลยคำถามวิเคราะห์เชิงลึก

1. **ค่า RSSI คืออะไร และมีผลอย่างไรต่อการติดตั้งอุปกรณ์ IoT?**
   > RSSI (Received Signal Strength Indication) มีหน่วยเป็น dBm เป็นค่ากำลังของคลื่นวิทยุที่บอร์ดรับได้ ยิ่งติดลบน้อย (เช่น -40 ถึง -60 dBm) ยิ่งเสถียร หากค่าต่ำกว่า -80 dBm แพ็กเก็ตข้อมูลจะเริ่มสูญหาย (Packet Loss) ทำให้อุปกรณ์หลุดบ่อย จึงต้องติดตั้งอุปกรณ์ในจุดที่มี RSSI ไม่ต่ำกว่า -70 dBm
2. **โหมด Station (STA), Access Point (AP) และ AP+STA แตกต่างกันอย่างไรในทางสถาปัตยกรรม และยกตัวอย่างการใช้งานจริง?**
   > - **STA (Station):** อุปกรณ์ทำหน้าที่เป็น Client ไปเกาะเราเตอร์หลัก เช่น ส่งข้อมูลอุณหภูมิขึ้น Cloud Dashboard
   > - **AP (Access Point):** อุปกรณ์ปล่อย Hotspot ของตัวเอง เช่น กล้องติดรถยนต์หรือสมาร์ตสวิตช์ในโรงงานที่ไม่มีเราเตอร์
   > - **AP+STA (Dual Mode):** ทำงานทั้งสองโหมดพร้อมกัน เช่น ระบบ Wi-Fi Provisioning / Captive Portal ให้ผู้ใช้เกาะ Hotspot เพื่อตั้งค่ารหัสผ่านบ้าน
3. **เหตุใดสถาปัตยกรรมแบบ Event-Driven Callback (เช่น WiFi.onEvent()) จึงมีประสิทธิภาพและความเสถียรสูงกว่าการเขียนคำสั่ง Status Polling ใน loop() เพื่อตรวจจับ Wi-Fi หลุด?**
   > การ Polling ใน `loop()` ทำให้สูญเสียรอบประมวลผล CPU และหากมีคำสั่ง `delay()` อื่นๆ ขวางอยู่ จะทำให้การตรวจจับสัญญาณหลุดล่าช้า ขณะที่ Event-Driven ทำงานแบบ Asynchronous ขับเคลื่อนด้วย System Event Callback ทันทีที่เกิดเหตุการณ์ ทำให้กู้คืนสัญญาณได้เร็วกว่าและไม่กระทบการทำงานหลักของโปรแกรม
