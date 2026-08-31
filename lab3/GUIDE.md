# 🌐 ใบงานที่ 3: เว็บเซิร์ฟเวอร์บนบอร์ด ESP32 / ESP8266 (LittleFS Web Server)

คู่มือการทดลองสร้าง **Embedded HTTP Web Server (Port 80)** บนบอร์ดไมโครคอนโทรลเลอร์ ESP32 และ ESP8266 เพื่อให้บริการไฟล์หน้าเว็บ (HTML, CSS, JavaScript, Images) จากระบบแฟ้มข้อมูล **LittleFS Flash Memory**

---

## 🎯 วัตถุประสงค์การเรียนรู้ (Objectives)

1. เข้าใจหลักการทำงานของ **HTTP Protocol** (Port 80) และวงจรคำขอ-ตอบกลับ (Stateless Request-Response Cycle)
2. เข้าใจโครงสร้างพาร์ติชัน Flash Memory และการใช้งานระบบไฟล์ **LittleFS** (เปรียบเทียบกับ SPIFFS)
3. สามารถอัปโหลดไฟล์เว็บ (`index.html`, `styles.css`, `app.js`) ขึ้นหน่วยความจำ Flash ผ่าน PlatformIO / Arduino IDE
4. รู้วิธีการใช้ฟังก์ชัน `server.streamFile()` เพื่อสตรีมไฟล์ขนาดใหญ่ไปยังเว็บเบราว์เซอร์โดยตรงโดยไม่สิ้นเปลืองหน่วยความจำ RAM (ป้องกัน Heap Overflow)
5. สามารถจัดการ Content-Type (MIME Type) และการตอบกลับกรณีไม่พบไฟล์ (404 File Not Found) ผ่าน `server.onNotFound()`

---

## 📁 โครงสร้างไฟล์ในระบบ LittleFS (`data/`)

ไฟล์หน้าเว็บทั้งหมดจะถูกจัดเก็บไว้ในโฟลเดอร์ `data/` ภายในโปรเจกต์:

```text
lab3/
 ├── platformio.ini         (การตั้งค่าโปรเจกต์ PlatformIO พร้อม board_build.filesystem = littlefs)
 ├── index.html             (เว็บแอปพลิเคชันใบงานออนไลน์พร้อมระบบ Auto-Grader)
 ├── Code.gs                (สคริปต์ Google Apps Script ตรวจคะแนนและบันทึก Google Sheets)
 ├── GUIDE.md               (คู่มือและเฉลยการทดลอง)
 └── solution/
      ├── platformio.ini
      ├── lab3_solution.ino (ไฟล์โปรแกรมฉบับสมบูรณ์สำหรับ Arduino IDE)
      ├── src/
      │    └── main.cpp     (ไฟล์โปรแกรมฉบับสมบูรณ์สำหรับ PlatformIO)
      └── data/
           └── index.html   (หน้าเว็บ UI ตัวอย่างที่บันทึกใน LittleFS Flash Memory)
```

---

## 💻 ขั้นตอนการอัปโหลดไฟล์ขึ้น LittleFS Flash Memory

### วิธีที่ 1: การใช้งานผ่าน PlatformIO (แนะนำ)

1. วางไฟล์เว็บทั้งหมด (`index.html`, `styles.css`, ฯลฯ) ไว้ในโฟลเดอร์ `solution/data/`
2. ตรวจสอบว่าใน `platformio.ini` มีการกำหนด `board_build.filesystem = littlefs`
3. เปิดหน้าต่าง Terminal ใน VS Code แล้วรันคำสั่ง:
   ```bash
   # อัปโหลดระบบไฟล์ LittleFS ไปยังบอร์ด ESP32
   pio run -e ipst_wifi -t uploadfs

   # จากนั้น Build และ Flash เฟิร์มแวร์หลัก
   pio run -e ipst_wifi -t upload
   ```
4. หรือคลิกที่ไอคอน **PlatformIO** แถบด้านข้าง -> เลือก Environment `ipst_wifi` หรือ `ax_wifi` -> ขยายเมนู **Platform** -> คลิก **Upload Filesystem Image**

### วิธีที่ 2: การใช้งานผ่าน Arduino IDE 2.x

1. ติดตั้งส่วนขยาย **Arduino ESP32 LittleFS Filesystem Uploader**
2. กด `Ctrl + Shift + P` (หรือ `Cmd + Shift + P` บน Mac) -> พิมพ์ `Upload LittleFS to ESP32`
3. อัปโหลดโค้ดสเก็ตช์ `lab3_solution.ino` เข้าสู่บอร์ดตามปกติ

---

## ✍️ เฉลยคำตอบและแนวคิดในใบงาน (Worksheet Answers)

### 1. เฉลยโค้ดเติมคำตอบ (Skeleton Code Blanks)

| ช่องที่ | ฟังก์ชัน / คำตอบ | คำอธิบาย |
| :---: | :--- | :--- |
| **ช่องที่ 1** | `LittleFS.begin(true)` หรือ `LittleFS.begin()` | เมานต์ระบบไฟล์ LittleFS (พารามิเตอร์ `true` ใน ESP32 สั่งให้ Auto-Format หากเมานต์ครั้งแรกไม่สำเร็จ) |
| **ช่องที่ 2** | `server.onNotFound(handleFileRequest)` | กำหนดฟังก์ชันดักรับคำขอไฟล์ที่ไม่ตรงกับ Route เจาะจงใดๆ (Catch-All Handler) |
| **ช่องที่ 3** | `LittleFS.exists(path)` | ตรวจสอบว่าไฟล์ตาม URI ที่ร้องขอมีอยู่ในระบบไฟล์หรือไม่ |
| **ช่องที่ 4** | `server.streamFile(file, dataType)` | สตรีมไฟล์ตรงจาก Flash Memory ไปยัง Client ทีละ Chunk เพื่อประหยัด RAM |
| **ช่องที่ 5** | `server.handleClient()` | ตรวจสอบและประมวลผลคำขอ HTTP ที่เข้ามายังเซิร์ฟเวอร์ในฟังก์ชัน `loop()` |

---

### 2. เฉลยแบบทดสอบปรนัย 5 ข้อ (Multiple Choice Quiz Keys)

| ข้อที่ | หัวข้อคำถาม | คำตอบที่ถูกต้อง | เหตุผลทางเทคนิค |
| :---: | :--- | :---: | :--- |
| **1** | หมายเลขพอร์ต 80 และ HTTP Protocol | **ข (1b)** | พอร์ต 80 เป็นพอร์ตมาตรฐานสากลสำหรับ HTTP ในการรับส่งข้อมูลแบบ Stateless Request-Response |
| **2** | จุดเด่นของระบบไฟล์ LittleFS | **ค (2c)** | LittleFS ออกแบบมาสำหรับ Flash Memory มี Wear Leveling ยืดอายุการใช้งาน และ Power-loss Resilience ทนทานต่อไฟดับ |
| **3** | เหตุใด `server.streamFile()` จึงปลอดภัยต่อ RAM | **ก (3a)** | `streamFile()` ส่งข้อมูลทีละก้อน (Chunk) ตรงจาก Flash โดยไม่ต้องโหลดไฟล์ทั้งก้อนเข้ามาเก็บใน RAM จึงไม่เกิด Heap Overflow |
| **4** | ความสำคัญของ MIME Content-Type | **ง (4d)** | เพื่อแจ้งให้เว็บเบราว์เซอร์ทราบชนิดของไฟล์ และทำการเรนเดอร์สไตล์หรือรันสคริปต์ได้ถูกต้องตามมาตรฐานเว็บ |
| **5** | หน้าที่ของ `server.onNotFound()` | **ก (5a)** | ทำหน้าที่เป็น Catch-all Handler จัดการคำขอ URI ที่ไม่มี Route หรือส่งคืนรหัส 404 เมื่อไม่พบไฟล์ |

---

### 3. เฉลยคำถามวิเคราะห์เชิงลึก (Analytical Questions)

**คำถามที่ 1: ระบบแฟ้มข้อมูล LittleFS มีข้อดีอย่างไรเมื่อเทียบกับระบบ SPIFFS รุ่นเก่า และช่วยยืดอายุ Flash Memory ได้อย่างไร?**
> **แนวคำตอบ:** LittleFS ถูกพัฒนาขึ้นมาเพื่อแก้ข้อจำกัดของ SPIFFS โดยมีกลไก **Dynamic Wear Leveling** ที่ช่วยกระจายการเขียนข้อมูลลงในบล็อกของ Flash Memory อย่างสม่ำเสมอ ไม่เขียนซ้ำที่เดิมจนเซลล์ความจำเสื่อมเร็ว มีระบบ **Power-loss Resilience** ที่ใช้โครงสร้างข้อมูลแบบ CoW (Copy-on-Write) ทำให้เมื่อเกิดไฟดับกะทันหันขณะบันทึกข้อมูล ระบบไฟล์จะไม่เสียหาย และยังรองรับโครงสร้างโฟลเดอร์ย่อย (Directory Hierarchy) อย่างแท้จริง

**คำถามที่ 2: เหตุใดการใช้คำสั่ง `server.streamFile()` จึงป้องกันปัญหา Heap Overflow เมื่อให้บริการไฟล์ขนาดใหญ่ได้?**
> **แนวคำตอบ:** เนื่องจากไมโครคอนโทรลเลอร์ ESP32/ESP8266 มีหน่วยความจำ RAM (SRAM/Heap) จำกัดมาก (ไม่กี่สิบถึงไม่กี่ร้อยกิโลไบต์) หากใช้การอ่านไฟล์เข้ามาเก็บในตัวแปร `String` แล้วสั่ง `server.send()` ไฟล์ขนาดใหญ่ (เช่น รูปภาพหรือ CSS ขนาด 50KB+) จะทำให้ RAM เต็มและบอร์ดค้าง/รีสตาร์ต การใช้ `server.streamFile()` จะใช้วิธีอ่านข้อมูลจาก Flash เป็นก้อนย่อย (Chunk Buffer ขนาดเล็ก เช่น 256-512 ไบต์) แล้วส่งออกทาง TCP Socket ทันทีวนไปจนจบไฟล์ จึงใช้ RAM น้อยมากคงที่ตลอดเวลา

**คำถามที่ 3: หากเบราว์เซอร์ร้องขอไฟล์ `styles.css` แต่เซิร์ฟเวอร์ส่ง Content-Type เป็น `text/plain` จะเกิดผลกระทบอย่างไรต่อหน้าเว็บ?**
> **แนวคำตอบ:** เว็บเบราว์เซอร์สมัยใหม่จะมีระบบความปลอดภัยและปฏิบัติตามมาตรฐาน MIME Type Checking อย่างเคร่งครัด หากได้รับไฟล์ `.css` ที่มี Header ระบุเป็น `text/plain` เบราว์เซอร์จะปฏิเสธการประมวลผลไฟล์นั้นเป็น Cascading Style Sheets (CSS MIME Type Mismatch / Strict MIME Type Checking) ทำให้หน้าเว็บแสดงผลเป็นเพียงข้อความดิบสีขาวดำที่ไม่มีการจัดรูปแบบสไตล์ สวยงาม หรือตำแหน่งเลย

---

## 🔍 การแก้ไขปัญหาที่พบบ่อย (Troubleshooting Guide)

1. **ขึ้นข้อความ `LittleFS Mount Failed` บน Serial Monitor:**
   - ตรวจสอบว่าได้รันคำสั่ง `Upload Filesystem Image` (หรือ `pio run -t uploadfs`) เพื่อฟอร์แมตและสร้างพาร์ติชัน LittleFS แล้วหรือยัง
   - สำหรับ ESP32 ให้ระบุ `LittleFS.begin(true)` เพื่อให้ทำการ Auto-format อัตโนมัติในกรณีที่ยังไม่มีพาร์ติชัน
2. **เปิดหน้าเว็บผ่านเบราว์เซอร์แล้วขึ้น `404: File Not Found`:**
   - ตรวจสอบว่าในโฟลเดอร์ `data/` มีไฟล์ชื่อ `index.html` (สะกดตัวพิมพ์เล็กตรงกันทั้งหมด)
   - ตรวจสอบว่าได้เรียก URL ถูกต้อง เช่น `http://192.168.1.XXX/`
3. **หน้าเว็บเปิดได้แต่ไม่มีสไตล์ CSS หรือรูปภาพไม่ขึ้น:**
   - ตรวจสอบฟังก์ชัน `getContentType()` ว่ามีเงื่อนไขตรวจสอบ `.css`, `.js`, `.png` ครบถ้วนหรือไม่
