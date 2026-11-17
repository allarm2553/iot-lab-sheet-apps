# IoT Lab Worksheets Workspace Rules & Gold Standard Template Reference

## 🎯 Official Benchmark Standard (Template ต้นแบบมาตรฐาน)
- **`lab2/` (`lab2/index.html`, `lab2/vscode-wokwi-template/`, `lab2/Code.gs`, `lab2/GUIDE.md`)** และ **`lab1/`** ได้รับการกำหนดให้เป็น **Gold Standard Worksheet Template (ต้นแบบใบงานปฏิบัติการมาตรฐาน)** สำหรับใบงานทั้งหมดในโปรเจกต์นี้
- **`Master_Code.gs`** ร่วมกับ **`config.js`**, **`index.html`** และ **`dashboard.html`** เป็น **Gold Standard Universal Gradebook & Roster Lookup Template** สำหรับระบบรวมศูนย์คะแนนทุกใบงานลงใน Google Sheet เดียว

เมื่อมีการพัฒนาใบงานใหม่ หรือปรับปรุงใบงานเดิม ต้องยึดถือโครงสร้าง สถาปัตยกรรม และมาตรฐานทางเทคนิคจากต้นแบบนี้ทุกประการ

---

## 📐 Core Architectural & Technical Standards

### 1. มาตรฐานพินฮาร์ดแวร์ (Hardware Pinout Standard)
- **บอร์ด ESP32 (IPST-WiFi):**
  - **On-board LED / Status LED:** `GPIO 18` เท่านั้น (ห้ามระบุเป็น `GPIO 2` และห้ามนำ `GPIO 18` ไปต่อสวิตช์ปุ่มกด)
  - **Push Button (SW1 / BOOT):** `GPIO 0` (`INPUT_PULLUP`, Active LOW)
  - **เซ็นเซอร์ DHT11 / DHT22 (DATA):** `GPIO 33`
  - **Analog Input (KNOB-S / VR):** `GPIO 36 (VP)` (12-bit: `0–4095`)
  - **Relay 1 (พัดลม) / Relay 2 (พ่นหมอก):** `GPIO 5` / `GPIO 23`
  - **จอแสดงผล OLED SSD1306 (I2C `0x3C`):** `SDA = GPIO 21`, `SCL = GPIO 22`
- **บอร์ด ESP8266 (NodeMCU 1.0 / AX-WiFi):**
  - **Push Button (FLASH บนบอร์ด):** `GPIO 0 / D3` (`INPUT_PULLUP`, Active LOW — ห้ามนำมาต่อเซ็นเซอร์ DHT เด็ดขาด)
  - **เซ็นเซอร์ DHT11 / DHT22 (DATA):** `GPIO 2 / D4` เสมอ (`#define DHTPIN 2`)
  - **Analog Input (VR):** `A0 (ADC0)` (10-bit: `0–1023`)
  - **Relay 1 / Relay 2:** `GPIO 13 (D7)` / `GPIO 16 (D0)`
  - **จอแสดงผล OLED SSD1306 (I2C `0x3C`):** `SDA = GPIO 4 (D2)`, `SCL = GPIO 5 (D1)`

---

### 2. มาตรฐาน VS Code + Wokwi Direct Launch (`openProjectInVsCode`)
- ปุ่ม **`</> VS Code Wokwi`** ในทุกใบงานที่มีวงจรจำลอง Wokwi ต้องเรียกใช้ฟังก์ชัน `openProjectInVsCode('<labFolder>')`:
  1. เรียกโปรโตคอล `window.location.href = 'vscode://file/Users/allarmmac/myjob_folder/MyLaB/IoT/<labFolder>/<subDir>';` โดยตรง (ไม่มี Modal กั้น)
  2. ส่งคำขอ `POST /api/open-vscode` ไปยัง `server.js` (`localhost:4000`) เพื่อสั่งเปิดโฟลเดอร์โปรเจกต์พร้อมเปิดไฟล์ `src/main.cpp` และ `diagram.json` ขึ้นมาในโปรแกรม Visual Studio Code ทันที
- โฟลเดอร์ `vscode-wokwi-template/` ต้นแบบ (`lab2/vscode-wokwi-template/`):
  - `wokwi.toml`: ต้องระบุพาธในเครื่องหมายอัญประกาศเสมอ (`firmware = '.pio/build/esp32dev/firmware.bin'`, `elf = '.pio/build/esp32dev/firmware.elf'`)
  - `diagram.json`: ใช้ `"board-ssd1306"` สำหรับจอ OLED SSD1306 และมี `"wokwi-logic-analyzer"` สำหรับส่งออกไฟล์ `.vcd`
  - `.vscode/extensions.json`: แนะนำ `"platformio.platformio-ide"` และ `"wokwi.wokwi-vscode"`
  - `.pio/build/esp32dev/firmware.bin` & `firmware.elf`: มีไฟล์ Firmware คอมไพล์ไว้ล่วงหน้าเพื่อให้กดรันจำลองได้ทันที

---

### 3. มาตรฐานการล็อกใบงาน ปุ่มนักศึกษาคนใหม่ และการยืนยันรหัสผ่านครูผู้สอน
- **กล่องแจ้งเตือนสีเขียวด้านบน (`#submittedNoticeBanner`):** แสดงเวลาที่ส่งงาน และมีเฉพาะปุ่ม **`[ 🔓 ครูผู้สอน: ปลดล็อกส่งใหม่ ]`**
- **ปุ่มเริ่มทำสำหรับนักศึกษาคนใหม่ (`#newStudentTopBar` & `#newStudentBtn`):**
  - วาง `#newStudentTopBar` ไว้ **ใต้กล่อง `.gas-connect-box` (ก่อนหัวข้อ ข้อมูลผู้ส่งรายงาน)** เป็นปุ่มสีน้ำเงินเต็มความกว้าง `[ 👤+ เริ่มทำสำหรับ นศ. คนใหม่ (ล้างฟอร์มเริ่มใหม่) ]`
  - และแสดงปุ่ม `#newStudentBtn` ที่ท้ายฟอร์มด้านล่างสุดด้วย
- **รหัสผ่านครูผู้สอน (`verifyTeacherUnlockPassword`):**
  - การกดปุ่ม `[ 🔓 ครูผู้สอน: ปลดล็อกส่งใหม่ ]` หรือ `[ 🔓 เคลียร์แคชส่งงาน ]` ต้องแสดงหน้าต่าง **`🔒 ยืนยันสิทธิ์ครูผู้สอน`** ให้กรอกรหัสผ่าน (`1234`, `teacher`, `iot512` หรือค่าใน `TEACHER_UNLOCK_PASSWORD` ใน `config.js`) ก่อนเสมอ เพื่อป้องกันนักศึกษากดปลดล็อกเอง

---

### 4. มาตรฐาน Universal Master Gradebook & Roster Lookup (`Master_Code.gs`)
- **รวมศูนย์ทุกใบงาน (19 แล็บ) ใน Google Sheet เดียว:** แยกแท็บตามรายใบงานอัตโนมัติ และมีแท็บแรกสุดชื่อ **`📊 สรุปคะแนนรวม`**
- **Master Roster Protection & Student ID Lookup (`syncAllScoresToMasterRoster`):**
  - ครูสามารถวางใบรายชื่อ (`รหัสนักศึกษา | ชื่อ-นามสกุล | กลุ่ม/ห้อง`) ลงในแท็บ `📊 สรุปคะแนนรวม` ล่วงหน้าได้
  - ยึด **`รหัสนักศึกษา` (คอลัมน์ A)** เป็น Primary Key ในการดึงคะแนนจากทุกแท็บใบงานย่อย
  - **ห้ามนำชื่อหรือห้องที่นักศึกษาพิมพ์เองมาเขียนทับคอลัมน์ B (`ชื่อ-นามสกุล`) และคอลัมน์ C (`กลุ่ม/ห้อง`) ที่ครูวางไว้ล่วงหน้า**
  - บังคับฟอร์แมตคอลัมน์ A และ C เป็น Plain Text (`setNumberFormat("@")`) และใช้ `sanitizeGroupValue()` ป้องกันไม่ให้ Google Sheets แปลงห้องเรียน เช่น `4/5`, `4/6` เป็นวันที่ (`Mon May 04 2026...GMT+0700`)
  - เมื่อครูสั่งปลดล็อกส่งงานใหม่ (`clearStudentSubmissionFromAllSheets`) ให้ลบเฉพาะแถวในแท็บใบงานย่อย และรีเซ็ตเฉพาะช่องคะแนนของแล็บนั้นในหน้า `📊 สรุปคะแนนรวม` เป็น `"-"` โดย **ห้ามลบแถวรายชื่อของนักศึกษาออกจากหน้าสรุปคะแนนรวม**
