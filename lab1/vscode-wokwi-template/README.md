# คู่มือการรัน Wokwi Simulation บน VS Code (Local Simulation)

เทมเพลตนี้ถูกเตรียมไว้สำหรับการจำลองการทำงานของวงจร ESP32 Lab 1 แบบออฟไลน์ 100% บนเครื่องคอมพิวเตอร์ของคุณเองผ่าน VS Code ไม่ต้องพึ่งพา Server ของ Wokwi.com

---

## 🛠️ ขั้นตอนการใช้งาน

### 1. ติดตั้ง Extensions ที่จำเป็นใน VS Code
- **PlatformIO IDE** (`platformio.platformio-ide`)
- **Wokwi for VS Code** (`Wokwi.wokwi-vscode`)

### 2. ขั้นตอนการคอมไพล์และรันจำลอง
1. เปิดโฟลเดอร์นี้ใน VS Code
2. รอให้ PlatformIO โหลด Library (`DHT sensor library`) ให้เสร็จสมบูรณ์
3. กดปุ่ม **Build (✓)** ที่แถบสถานะด้านล่างของ PlatformIO เพื่อสร้างไฟล์ `firmware.elf`
4. เมื่อ Build สำเร็จ (SUCCESS):
   - กดปุ่ม **`F1`** หรือ **`Ctrl + Shift + P`**
   - พิมพ์คำสั่ง: **`Wokwi: Start Simulator`** แล้วกด Enter
   - หรือคลิกเปิดไฟล์ `diagram.json` แล้วกดปุ่ม **Start Simulation (▶)**
5. แถบจำลอง Wokwi จะเปิดขึ้นมา พร้อมทั้ง Serial Monitor แสดงผลแบบเรียลไทม์!
