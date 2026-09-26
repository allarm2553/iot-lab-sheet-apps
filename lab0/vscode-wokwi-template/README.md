# 🛠️ Lab 0: VS Code + PlatformIO + Wokwi Template Project
## การติดตั้งสภาพแวดล้อมการพัฒนาและการทดสอบอัปโหลดโปรแกรมไฟกระพริบ (Blink LED)

โปรเจกต์ต้นแบบสำหรับเริ่มต้นการพัฒนาไมโครคอนโทรลเลอร์ ESP32 / IPST-WiFi ด้วย Visual Studio Code, PlatformIO IDE และการจำลองวงจรเสมือนจริงด้วย Wokwi Simulator

---

### 📦 โครงสร้างไฟล์ในโปรเจกต์
- `platformio.ini`: ไฟล์คอนฟิกบอร์ดและสภาพแวดล้อม PlatformIO (ESP32, 115200 baud)
- `wokwi.toml`: ไฟล์เชื่อมโยงเฟิร์มแวร์เข้ากับโปรแกรมจำลอง Wokwi
- `diagram.json`: แผนภาพวงจรจำลอง (ESP32, LED สีแดง/น้ำเงิน, ตัวต้านทาน 220Ω, ปุ่มกด SW1)
- `src/main.cpp`: ซอร์สโค้ดโปรแกรมภาษา C++ สำหรับทดสอบไฟกระพริบและการอ่านค่า Serial Print
- `.vscode/`: ค่าคอนฟิก Extensions และ Tasks สำหรับ VS Code

---

### 🚀 ขั้นตอนการใช้งาน

#### 1. เปิดโปรเจกต์ใน Visual Studio Code
1. แตกไฟล์ ZIP และเปิดโฟลเดอร์นี้ใน VS Code (`File` -> `Open Folder...`)
2. ตรวจสอบว่าได้ติดตั้งส่วนขยาย **PlatformIO IDE** และ **Wokwi Simulator** แล้ว

#### 2. รันการจำลองบน Wokwi Simulator ใน VS Code
1. กดปุ่ม `Ctrl + Shift + P` (หรือ `Cmd + Shift + P` บน Mac)
2. พิมพ์คำสั่ง `Wokwi: Start Simulator` แล้วกด Enter
3. หรือกดเปิดไฟล์ `diagram.json` แล้วกดปุ่ม **Play (▶️)** ที่มุมขวาบนของหน้าต่างวงจร

#### 3. คอมไพล์และอัปโหลดลงบอร์ดฮาร์ดแวร์จริง (Physical Board)
1. เชื่อมต่อบอร์ด ESP32 หรือ IPST-WiFi เข้ากับเครื่องคอมพิวเตอร์ผ่านสาย USB
2. กดปุ่มไอคอน **Build (✓)** ที่แถบด้านล่างซ้ายของ VS Code เพื่อคอมไพล์โค้ด
3. กดปุ่มไอคอน **Upload (→)** เพื่ออัปโหลดเฟิร์มแวร์ลงบอร์ด
4. กดปุ่มไอคอน **Serial Monitor (🔌)** เพื่อดูข้อความ Log ที่ส่งมาจากบอร์ดที่ความเร็ว `115200` Baud
