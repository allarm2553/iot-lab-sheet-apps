# 🔘 ESP32 Lab 1.1: Digital Inputs & Debouncing (Local VS Code Starter Template)

ชุดโปรเจกต์จำลองการทดลองใบงานที่ 1.1 สำหรับรันบน Visual Studio Code ออฟไลน์ 100%

---

## 🚀 วิธีการเริ่มต้นใช้งาน (Step-by-Step)

### 1. ติดตั้ง Extension ที่จำเป็น
- **PlatformIO IDE** (สำหรับคอมไพล์เฟิร์มแวร์ ESP32)
- **Wokwi for VS Code** (สำหรับรันวงจรจำลอง `diagram.json`)

### 2. คอมไพล์โปรแกรม (Build Firmware)
- กดปุ่ม **Build (✓)** ที่แถบสถานะด้านล่างของ PlatformIO หรือกด `Ctrl + Alt + B`
- รอจนขึ้นข้อความ `[SUCCESS]`

### 3. เริ่มการจำลองเสมือนจริง (Start Simulation)
- เปิดไฟล์ `diagram.json`
- กดปุ่ม **Start Simulation (▶)** หรือกด `F1` พิมพ์ `Wokwi: Start Simulator`

---

## 🏆 โจทย์ท้าทายการทดลอง (Grand Challenge)
ศึกษาและเขียนโค้ดเพิ่มเติมในไฟล์ `src/main.cpp` ตรงจุดที่มีคอมเมนต์ `// TODO`:
1. **Toggle Count:** นับจำนวนครั้งการกดสวิตช์เปิด-ปิด Relay 1
2. **Mist Trigger:** เมื่อกดครบ 3 ครั้ง ให้สั่งเปิด Relay 2 (ปั๊มพ่นหมอก)
3. **Long Press Reset:** เมื่อกดปุ่มค้างไว้นานกว่า 2 วินาที ให้สั่งตัดการทำงานของรีเลย์ทั้งหมดทันที (Safe State) และรีเซ็ตตัวนับเป็น 0
