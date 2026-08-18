# 🌐 ใบงานที่ 3: เว็บเซิร์ฟเวอร์บนบอร์ด ESP32 / ESP8266 (LittleFS Web Server)

คู่มือการทดลองสร้าง **Embedded HTTP Web Server (Port 80)** บนบอร์ด ESP32 และ ESP8266 เพื่อให้บริการไฟล์หน้าเว็บ (HTML, CSS, JavaScript) จากระบบแฟ้มข้อมูล **LittleFS Flash Memory**

---

## 🎯 วัตถุประสงค์ (Objectives)

1. เข้าใจหลักการทำงานของ **HTTP Protocol** และการทำงานของไมโครคอนโทรลเลอร์ในฐานะ **Web Server**
2. สามารถจัดเก็บไฟล์เว็บแอปพลิเคชัน (`index.html`, `styles.css`, `app.js`) ลงในพาร์ติชัน **LittleFS Flash Memory**
3. เข้าใจการใช้ฟังก์ชัน `server.streamFile()` เพื่อสตรีมไฟล์ขนาดใหญ่ไปยังบราวเซอร์โดยตรงโดยไม่สิ้นเปลืองหน่วยความจำ RAM
4. สามารถจัดการ Content-Type (MIME Type) และการตอบกลับกรณีไม่พบไฟล์ (404 Not Found)

---

## 📁 โครงสร้างไฟล์ใน LittleFS (`data/`)

```text
data/
 ├── index.html     (โครงสร้างหน้าเว็บ UI)
 ├── styles.css     (การตกแต่งธีม Glassmorphism)
 └── app.js         (การทำงานและ Logic ฝั่ง Client)
```

---

## ✍️ เฉลยคำตอบในใบงาน (Worksheet Answers)

### 1. โค้ดเติมช่องว่าง (Code Blanks)
* การกำหนดฟังก์ชันดักรับคำขอทุกพาธ (Catch-All NotFound Handler):
  ```cpp
  server.onNotFound(handleFileRequest);
  ```
* การประมวลผลคำขอเว็บเซิร์ฟเวอร์ใน `loop()`:
  ```cpp
  server.handleClient();
  ```

---

### 2. คำถามสรุปผลการทดลอง (Review Questions)

**คำถามที่ 1: ระบบแฟ้มข้อมูล LittleFS มีข้อดีอย่างไรเมื่อเทียบกับระบบ SPIFFS รุ่นเก่า?**
> **แนวคำตอบ:** LittleFS เป็นระบบไฟล์ที่ออกแบบมาเฉพาะสำหรับ Flash Memory ที่มีทรัพยากรจำกัด มีความทนทานต่อไฟดับกะทันหัน (Power-loss Resilient) ไม่ทำให้ระบบไฟล์พัง มีระบบ Wear Leveling ช่วยกระจายการเขียนยืดอายุการใช้งานชิป Flash และรองรับการจัดการโครงสร้างไดเรกทอรีที่เสถียรกว่า SPIFFS

**คำถามที่ 2: เหตุใดการใช้คำสั่ง `server.streamFile()` จึงมีประสิทธิภาพดีกว่าการอ่านไฟล์ทั้งหมดเข้ามาเก็บในตัวแปร `String` แล้วสั่ง `server.send()`?**
> **แนวคำตอบ:** เนื่องจากไมโครคอนโทรลเลอร์มีหน่วยความจำ RAM จำกัดมาก หากใช้ `String` เก็บไฟล์เว็บขนาดใหญ่ อาจทำให้หน่วยความจำ RAM หมด (Heap Overflow / Out of Memory) และทำให้บอร์ดรีสตาร์ต การใช้ `server.streamFile()` จะอ่านข้อมูลจาก Flash Memory แล้วส่งทีละก้อน (Chunk) ออกทางเครือข่ายโดยตรง จึงประหยัด RAM และทำงานได้รวดเร็วมาก
