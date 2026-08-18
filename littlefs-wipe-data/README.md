# 🧹 LittleFS Wipe Data Utility

คู่มือการใช้งานเครื่องมือล้างและฟอร์แมตหน่วยความจำภายใน (Flash Memory พาร์ติชัน LittleFS) สำหรับบอร์ด **ESP32** (เช่น IPST-WiFi, ESP32 Dev Module) และ **ESP8266** (เช่น AX-WiFi, NodeMCU V2)

---

## 🎯 เมื่อไหร่ที่ควรใช้เครื่องมือนี้?

1. **ต้องการล้างการตั้งค่าเดิมทั้งหมด (Factory Reset / Clean Slate):**
   - ลบไฟล์ `config.json` (ข้อมูล Wi-Fi SSID/Password, MQTT Broker, พอร์ต, และพารามิเตอร์อื่นๆ ที่เคยบันทึกไว้)
   - ลบไฟล์เว็บ UI (`index.html`, `styles.css`, etc.) ที่เคยอัพโหลดค้างไว้
2. **แก้ปัญหา LittleFS เสียหาย (Corrupted Filesystem):**
   - บอร์ดแจ้งเตือน `LittleFS Mount Failed`
   - เกิดปัญหา Bootloop / รีสตาร์ตวนซ้ำจากการอ่านไฟล์คอนฟิกไม่สำเร็จ
3. **เตรียมพร้อมก่อนอัพโหลด Data ชุดใหม่:**
   - มั่นใจว่าไม่มีไฟล์ขยะหรือโครงสร้างไดเรกทอรีเก่าตกค้างใน Flash

---

## 🚀 วิธีการใช้งาน

คุณสามารถเลือกอัพโหลดได้ 2 ช่องทางตามความสะดวก:

### วิธีที่ 1: ผ่าน PlatformIO (VS Code) — *แนะนำ*

1. เปิดโฟลเดอร์ `littlefs-wipe-data` ด้วย VS Code
2. เลือก Environment บอร์ดที่แถบด้านล่าง หรือในเมนู PlatformIO:
   - **ESP32 (IPST-WiFi / ESP32 Dev):** เลือก `env:ipst_wifi`
   - **ESP8266 (AX-WiFi / NodeMCU):** เลือก `env:ax_wifi`
3. กดปุ่ม **Upload** (เครื่องหมายลูกศรขวา `→`)
4. เปิด **Serial Monitor** ที่ baud rate `115200`

---

### วิธีที่ 2: ผ่าน Arduino IDE

1. เปิดไฟล์ `littlefs-wipe-data.ino` ด้วย Arduino IDE
2. เลือกบอร์ดและพอร์ตให้ถูกต้อง:
   - **ESP32:** `Tools` > `Board` > `ESP32 Arduino` > `ESP32 Dev Module`
   - **ESP8266:** `Tools` > `Board` > `ESP8266 Boards` > `NodeMCU 1.0 (ESP-12E Module)`
3. กดปุ่ม **Upload**
4. เปิด **Serial Monitor** (ตั้งความเร็วเป็น `115200 baud`)

---

## 📟 ตัวอย่างผลลัพธ์บน Serial Monitor

เมื่อโปรแกรมทำงานสำเร็จ จะแสดงข้อความลักษณะนี้:

```text
====================================
--- LittleFS Force Format Utility ---
====================================
[INFO] LittleFS mounted successfully.
[INFO] Formatting LittleFS. Please wait, this may take a few seconds...
[SUCCESS] LittleFS formatted successfully in 1420 ms!
====================================
```

> **หมายเหตุ:** ขั้นตอนการฟอร์แมตอาจใช้เวลาประมาณ 1–5 วินาที ขึ้นอยู่กับขนาดพาร์ติชัน Flash Memory ของบอร์ด

---

## 📋 ขั้นตอนถัดไปหลังจากล้างข้อมูลแล้ว
หลังจากฟอร์แมต LittleFS เรียบร้อยแล้ว:
1. แฟลชโปรเจกต์หลักของคุณ (เช่น LAB ต่างๆ หรือโค้ด IoT) ลงในบอร์ด
2. หากโปรเจกต์ต้องใช้ไฟล์ Web UI หรือ Config ให้รันคำสั่ง **Upload Filesystem Image** / **Upload LittleFS** ใน PlatformIO หรือ Arduino IDE Plugin เพื่ออัพโหลดไฟล์ใหม่เข้าสู่บอร์ด
