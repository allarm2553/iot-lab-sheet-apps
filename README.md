# คู่มือการติดตั้งและใช้งานระบบส่งใบงานออนไลน์ (Google Sheets & Local Server Simulation)

โฟลเดอร์นี้ประกอบด้วยเว็บแอปพลิเคชัน (Web Apps) ทั้งหมด **11 ชุด** สำหรับระบบส่งใบงานปฏิบัติการในหลักสูตร **Hybrid Local/Cloud IoT Node ด้วย ESP32** และ **หลักสูตรระดับสูง (Advanced IoT)** โดยคุณครูผู้สอนสามารถเลือกใช้งานได้ **2 ระบบ** ตามความเหมาะสมของสภาพแวดล้อมเครือข่ายอินเทอร์เน็ต:
1. **ระบบคลาวด์ออนไลน์ (Google Sheets + Google Drive)** - สำหรับห้องเรียนที่มีเน็ตต่อออกภายนอกได้ปกติ
2. **ระบบจำลองเซิร์ฟเวอร์โลคอล (Local Simulation Server)** - สำหรับห้องเรียนแบบจำลองเครือข่ายปิด (LAN/Offline) รันเซิร์ฟเวอร์บนคอมพิวเตอร์ของคุณครู

---

## 📁 โครงสร้างโปรเจกต์
- [lab-basic/](file:///C:/Users/terd2/.gemini/antigravity/scratch/lab-sheet-apps/lab-basic) - ใบงานพื้นฐาน: การติดตั้ง Arduino IDE และคุณลักษณะของ ESP32
- [lab1/](file:///C:/Users/terd2/.gemini/antigravity/scratch/lab-sheet-apps/lab1) - ใบงานที่ 1: การอ่านค่าเซ็นเซอร์และการควบคุมเอาต์พุต (GPIO, ADC & Relays)
- [lab2/](file:///C:/Users/terd2/.gemini/antigravity/scratch/lab-sheet-apps/lab2) - ใบงานที่ 2: การแสดงผลผ่านจอ OLED SSD1306 (I2C)
- [lab3/](file:///C:/Users/terd2/.gemini/antigravity/scratch/lab-sheet-apps/lab3) - ใบงานที่ 3: เว็บเซิร์ฟเวอร์บนบอร์ด ESP32 (LittleFS Web Server)
- [lab4/](file:///C:/Users/terd2/.gemini/antigravity/scratch/lab-sheet-apps/lab4) - ใบงานที่ 4: การส่งข้อมูลสองทางแบบเรียลไทม์ (Local WebSockets)
- [lab5/](file:///C:/Users/terd2/.gemini/antigravity/scratch/lab-sheet-apps/lab5) - ใบงานที่ 5: การส่งข้อมูลระดับคลาวด์ผ่าน Cloud MQTT
- [lab6/](file:///C:/Users/terd2/.gemini/antigravity/scratch/lab-sheet-apps/lab6) - ใบงานที่ 6: การบูรณาการโครงงาน Hybrid Node ขั้นสูง (Grand Challenge)
- [lab-extra/](file:///C:/Users/terd2/.gemini/antigravity/scratch/lab-sheet-apps/lab-extra) - ใบงานเสริมปฏิบัติการ: การพัฒนา Web Dashboard ด้วย HTML, CSS และ JavaScript
- [lab7/](file:///C:/Users/terd2/.gemini/antigravity/scratch/lab-sheet-apps/lab7) - ใบงานที่ 7: การเชื่อมโยงข้อมูลและการสร้างแดชบอร์ดด้วย Node-RED
- [lab8/](file:///C:/Users/terd2/.gemini/antigravity/scratch/lab-sheet-apps/lab8) - ใบงานที่ 8: การบันทึกสถิติมิติตามเวลาด้วย InfluxDB (Data Logging)
- [lab9/](file:///C:/Users/terd2/.gemini/antigravity/scratch/lab-sheet-apps/lab9) - ใบงานที่ 9: การวิเคราะห์และแสดงผลข้อมูลย้อนหลังด้วย Grafana

---

## 🛠️ ระบบที่ 1: การใช้งานผ่านเซิร์ฟเวอร์จำลองโลคอล (Local Server - ออฟไลน์ 100%)
ระบบนี้เหมาะอย่างยิ่งสำหรับการจัดเตรียมห้องเรียนคอมพิวเตอร์แบบออฟไลน์ หรือการจำลองบนเครือข่ายภายในโรงเรียน (LAN) โดยข้อมูลใบงานและไฟล์แนบรูปภาพ/ซอร์สโค้ดของนักเรียนจะถูกเขียนบันทึกลงในดิสก์คอมพิวเตอร์ของคุณครูโดยตรง

### ขั้นตอนการเริ่มรันเซิร์ฟเวอร์:
1. ติดตั้ง **Node.js** ลงบนเครื่องคอมพิวเตอร์ของคุณครู (ดาวน์โหลดเวอร์ชัน LTS จากหน้าเว็บ [nodejs.org](https://nodejs.org))
2. เปิดโปรแกรม terminal หรือ PowerShell แล้วชี้ตำแหน่งโฟลเดอร์มาที่โปรเจกต์นี้ จากนั้นรันคำสั่งติดตั้ง Dependencies:
   ```bash
   npm install
   ```
3. รันสั่งเริ่มทำงานเซิร์ฟเวอร์:
   ```bash
   npm start
   ```
4. เซิร์ฟเวอร์จะเปิดพอร์ตการทำงานขึ้นมาที่พอร์ต `3000` โดยคุณครูสามารถแจกจ่ายไอพีของคอมพิวเตอร์เครื่องหลักเพื่อให้นักเรียนเข้าทำใบงานได้ เช่น:
   - **ใบงานพื้นฐาน**: `http://<IP_เครื่องครู>:3000/lab-basic`
   - **ใบงานที่ 1**: `http://<IP_เครื่องครู>:3000/lab1`
   - **ใบงานที่ 7**: `http://<IP_เครื่องครู>:3000/lab7`
5. **แผงตรวจคะแนนของคุณครู (Teacher Dashboard)**: 
   คุณครูสามารถเปิดดูและดาวน์โหลดไฟล์งานนักเรียนทั้งหมดแยกห้องเรียนได้ผ่านลิงก์:
   - `http://localhost:3000/dashboard`

### โครงสร้างข้อมูลที่เก็บลงดิสก์เครื่องหลัก (โฟลเดอร์ `submissions/`):
- `submissions/data/` - เก็บไฟล์คำตอบแบบตัวหนังสือของนักเรียนในรูปแบบไฟล์ JSON แยกตามใบงาน (เช่น `lab1.json`)
- `submissions/uploads/` - เก็บไฟล์แนบทางกายภาพ รูปภาพผลลัพธ์หน้าจอ และไฟล์โค้ด `.ino` ของนักเรียน แยกโฟลเดอร์แต่ละใบงานอย่างเป็นระเบียบ

---

## ☁️ ระบบที่ 2: ขั้นตอนการติดตั้งและ Deploy บน Google Sheets (ออนไลน์)
สำหรับห้องเรียนที่มีอินเทอร์เน็ตใช้งานปกติ และต้องการประมวลผลข้อมูลส่งขึ้นคลาวด์ของ Google ไดรฟ์และชีตโดยตรง (ทำแยกกันทีละใบงาน):

### ขั้นตอนการทำงาน:
1. **สร้างชีต**: สร้าง Google Sheet เปล่าขึ้นมาและเปิดแถบ **ส่วนขยาย (Extensions) &rarr; Apps Script**
2. **ใส่โค้ด Backend**: คัดลอกโค้ดจากไฟล์ `Code.gs` ในโฟลเดอร์ใบงานนั้นๆ (เช่นจาก `lab1/Code.gs`) ไปวางทับลงในแท็บ Apps Script ของไฟล์ `Code.gs` แล้วกดบันทึก
3. **ใส่โค้ด Frontend**: คลิกเครื่องหมายบวกเพื่อเพิ่มไฟล์ประเภท **HTML** ตั้งชื่อว่า `index` (จะได้ไฟล์ `index.html`) จากนั้นลบโค้ดเริ่มต้นออก นำสคริปต์ทั้งหมดจาก `index.html` ของโฟลเดอร์ใบงานนั้นๆ ไปวางและกดบันทึก
4. **Deploy การใช้งานจริง**: คลิกปุ่ม **Deploy (การใช้การที่ใช้งานได้จริงใหม่)** 
   - เลือกประเภทบริการเป็น **เว็บแอป (Web App)**
   - ตั้งค่า **เรียกใช้งานในฐานะ (Execute as)** เป็น: **ฉัน (Me / อีเมลครู)**
   - ตั้งค่า **ผู้มีสิทธิ์เข้าถึง (Who has access)** เป็น: **ทุกคน (Anyone)**
   - กดปุ่ม **Deploy** และทำการ **อนุมัติสิทธิ์การเข้าถึง (Authorize Access)** ไปตามบัญชี Google ของท่าน
5. ระบบจะคืนค่า **URL ของเว็บแอป (Web App URL)** นำลิงก์นี้ไปแจกจ่ายให้นักเรียนเริ่มตอบคำถามและส่งผลงานได้ทันที!
