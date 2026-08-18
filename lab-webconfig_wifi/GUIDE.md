# 🌐 ใบงานเสริม: การตั้งค่า Wi-Fi ผ่านหน้าเว็บ Web Configuration (Captive Portal)

คู่มือการเรียนรู้และทดลองสร้างระบบ **Web Wi-Fi Configurator** พร้อม **Captive Portal** และการบันทึกค่าลงใน **LittleFS Flash Memory** บนบอร์ด **ESP32** (เช่น IPST-WiFi) และ **ESP8266** (เช่น AX-WiFi / NodeMCU)

---

## 🎯 วัตถุประสงค์ (Objectives)

1. เข้าใจหลักการทำงานของ **Captive Portal** และ **DNS Server (Port 53)** ในการดักจับและเปลี่ยนเส้นทาง (Redirect) เว็บบราวเซอร์ไปยังหน้าคอนฟิก `192.168.4.1` อัตโนมัติ
2. สามารถสร้าง **Embedded Web Server (Port 80)** เพื่อแสดงผลหน้าเว็บกำหนดค่า Wi-Fi และรับข้อมูลผ่าน HTTP POST
3. สามารถจัดเก็บและโหลดข้อมูลการเชื่อมต่อเครือข่าย (`ssid`, `pass`) ในรูปแบบ JSON ลงใน **LittleFS Flash Memory (`/config.json`)** ได้อย่างถาวรโดยไม่ต้องแก้โค้ดใหม่
4. เข้าใจกลไก **Fallback AP Mode** เมื่อไม่พบคอนฟิกหรือเชื่อมต่อ Wi-Fi ไม่สำเร็จ และการกดปุ่มบนบอร์ด (GPIO 0) ค้าง 3 วินาทีเพื่อบังคับเข้าโหมด AP

---

## 🏗️ โครงสร้างและการทำงานของระบบ (System Workflow)

```
                       ┌────────────────────────────────┐
                       │           ESP Boot Up          │
                       └───────────────┬────────────────┘
                                       │
                                       ▼
                       ┌────────────────────────────────┐
                       │  LittleFS.exists(/config.json) │
                       └───────┬────────────────┬───────┘
                     YES (พบ)  │                │ NO (ไม่พบไฟล์)
                               ▼                │
                 ┌───────────────────────────┐  │
                 │ WiFi.begin(ssid, pass)    │  │
                 └─────────────┬─────────────┘  │
                   สำเร็จ       │   ล้มเหลว (>15s)│
         ┌─────────────────────┴────────┐       │
         ▼                              ▼       ▼
┌─────────────────────────┐   ┌────────────────────────────────┐
│   STA Mode Connected    │   │      Fallback AP Mode          │
│   (เชื่อมต่อเน็ตสำเร็จ)    │   │  SSID: "ESP-Config-AP"         │
│   - รันโปรแกรมหลัก       │   │  IP: 192.168.4.1               │
│   - หากกด SW1 ค้าง 3 วิ  │   │  - DNSServer (Port 53)         │
│     จะสลับเข้า AP Mode   │   │  - WebServer (Port 80)         │
└─────────────────────────┘   └───────────────┬────────────────┘
                                              │ ผู้ใช้กรอก Wi-Fi ผ่านเว็บ
                                              ▼
                              ┌────────────────────────────────┐
                              │  บันทึกค่าลง /config.json      │
                              │  และ ESP.restart()             │
                              └────────────────────────────────┘
```

---

## 📁 โครงสร้างไฟล์ในโฟลเดอร์ `lab-webconfig_wifi`

| ไฟล์ / โฟลเดอร์ | รายละเอียด |
| :--- | :--- |
| [`student_starter.cpp`](file:///c:/Users/terd2/.gemini/antigravity/scratch/sheets-webapps/iot-lab-sheet-apps/lab-webconfig_wifi/student_starter.cpp) | โค้ดโครงร่างสำหรับนักเรียน มีช่องว่าง `/* เติมคำสั่งที่นี่ */` ให้นักเรียนฝึกเขียน |
| [`anser.txt`](file:///c:/Users/terd2/.gemini/antigravity/scratch/sheets-webapps/iot-lab-sheet-apps/lab-webconfig_wifi/anser.txt) | เฉลยคำตอบของช่องว่างใน `student_starter.cpp` |
| [`solution/`](file:///c:/Users/terd2/.gemini/antigravity/scratch/sheets-webapps/iot-lab-sheet-apps/lab-webconfig_wifi/solution) | โปรเจกต์ PlatformIO ฉบับสมบูรณ์ (พร้อมไฟล์ `data/index.html` หน้าฟอร์มแบบ Glassmorphism) |
| [`challenge_solution/`](file:///c:/Users/terd2/.gemini/antigravity/scratch/sheets-webapps/iot-lab-sheet-apps/lab-webconfig_wifi/challenge_solution) | โปรเจกต์ฉบับท้าทาย (เพิ่มฟีเจอร์ปุ่ม **Scan Wi-Fi** ให้เลือก SSID จาก Dropdown ได้ทันที) |
| [`index.html`](file:///c:/Users/terd2/.gemini/antigravity/scratch/sheets-webapps/iot-lab-sheet-apps/lab-webconfig_wifi/index.html) | หน้าเว็บ Web App สำหรับส่งใบงานและตรวจคะแนนอัตโนมัติ (Google Apps Script UI) |
| [`Code.gs`](file:///c:/Users/terd2/.gemini/antigravity/scratch/sheets-webapps/iot-lab-sheet-apps/lab-webconfig_wifi/Code.gs) | โค้ด Backend ตรวจคะแนนอัตโนมัติบน Google Apps Script |

---

## 🔌 การต่อวงจรและพินฮาร์ดแวร์ (Hardware Pinouts)

| บอร์ด | ฟังก์ชัน | ขา GPIO | คำอธิบาย |
| :--- | :--- | :--- | :--- |
| **ESP32 (IPST-WiFi)** | Force AP Button | `GPIO 0` | ปุ่ม **SW1** บนบอร์ด (กดค้าง 3 วิ เพื่อเข้าโหมด AP) |
| **ESP8266 (AX-WiFi / NodeMCU)** | Force AP Button | `GPIO 0` (`D3`) | ปุ่ม **FLASH** บนบอร์ด (กดค้าง 3 วิ เพื่อเข้าโหมด AP) |

---

## ✍️ เฉลยคำตอบในใบงาน (Worksheet Answers)

### 1. โค้ดเติมช่องว่าง (Code Blanks)
* **ช่องว่างที่ 1 (`loop()` ขณะอยู่ในโหมด AP):**
  ```cpp
  dnsServer.processNextRequest();
  ```
  *(ทำหน้าที่ประมวลผลคำขอ DNS เพื่อทำ Captive Portal)*

* **ช่องว่างที่ 2 (`saveWifiConfig()`):**
  ```cpp
  LittleFS.open("/config.json", "w");
  ```
  *(เปิดไฟล์ `config.json` ในโหมด Write เพื่อเขียนค่าการตั้งค่าทับลง Flash)*

---

### 2. คำถามสรุปผลการทดลอง (Review Questions)

**คำถามที่ 1: ระบบ Captive Portal ใช้หลักการใดในการนำผู้ใช้ไปยังหน้าเว็บตั้งค่าอัตโนมัติ?**
> **แนวคำตอบ:** ใช้ `DNSServer` บนพอร์ต 53 ดักจับคำขอ Domain Name ทุกคำขอ (Wildcard `*`) ที่ส่งมาจากอุปกรณ์ที่เชื่อมต่อกับ Access Point แล้วตอบกลับด้วยหมายเลข IP ของตัว ESP เอง (`192.168.4.1`) ส่งผลให้เว็บบราวเซอร์หรือระบบปฏิบัติการของมือถือ/คอมพิวเตอร์เปิดหน้าล็อกอินหรือหน้าคอนฟิกขึ้นมาโดยอัตโนมัติ

**คำถามที่ 2: เหตุใดการบันทึกการตั้งค่าลง LittleFS จึงดีกว่าการ Hardcode ค่าลงในซอร์สโค้ดโดยตรง?**
> **แนวคำตอบ:** การบันทึกลง LittleFS ทำให้สามารถเปลี่ยนเครือข่าย Wi-Fi ได้ตลอดเวลาผ่านหน้าเว็บโดยไม่ต้องต่อสาย Flash โค้ดใหม่ และข้อมูลยังคงถูกจัดเก็บอย่างถาวรแม้ปิดไฟหรือรีสตาร์ตบอร์ด นอกจากนี้ยังช่วยให้สามารถแจกจ่ายเฟิร์มแวร์ตัวเดียวกันให้อุปกรณ์หลายตัวได้ทันที

---

## 🚀 วิธีการทดสอบระบบ (Testing & Verification)

1. **อัพโหลดโค้ด:**
   - เปิดโฟลเดอร์ [`solution/`](file:///c:/Users/terd2/.gemini/antigravity/scratch/sheets-webapps/iot-lab-sheet-apps/lab-webconfig_wifi/solution) หรือ [`challenge_solution/`](file:///c:/Users/terd2/.gemini/antigravity/scratch/sheets-webapps/iot-lab-sheet-apps/lab-webconfig_wifi/challenge_solution) ใน PlatformIO
   - แฟลชโค้ดลงบอร์ด ESP32 หรือ ESP8266
2. **เข้าสู่หน้าตั้งค่า:**
   - หากยังไม่เคยตั้งค่า บอร์ดจะปล่อย Wi-Fi ชื่อ **`ESP-Config-AP`**
   - ใช้อุปกรณ์เชื่อมต่อ Wi-Fi ดังกล่าว จากนั้นเปิดบราวเซอร์ไปที่ `http://192.168.4.1`
3. **กำหนดค่าและรีสตาร์ต:**
   - เลือก/กรอกชื่อ Wi-Fi (SSID) และรหัสผ่าน (Password)
   - กด **Save & Connect** บอร์ดจะบันทึกข้อมูลลง LittleFS และรีบูตเข้าสู่โหมดเชื่อมต่อเครือข่ายจริงทันที
