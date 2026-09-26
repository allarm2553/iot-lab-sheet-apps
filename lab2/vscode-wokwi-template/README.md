# 📟 ESP32 Lab 2: OLED SSD1306 Display & Multi-Sensor Dashboard (Local VS Code Starter Template)

ชุดโปรเจกต์จำลองการทดลองใบงานที่ 2 สำหรับรันบน Visual Studio Code ออฟไลน์ 100% ผ่าน PlatformIO และ Wokwi Simulator

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

## 🔌 ตารางการเชื่อมต่อพิน (Pinout Wiring)
- **OLED SSD1306 (128x64 I2C):**
  - VCC -> 3.3V, GND -> GND, SDA -> GPIO 21, SCL -> GPIO 22 (Address `0x3C`)
- **DHT22 Sensor:**
  - VCC -> 3.3V, GND -> GND, DATA -> GPIO 33
- **Potentiometer 10k (ADC):**
  - VCC -> 3.3V, GND -> GND, SIG -> GPIO 36
- **Push Button SW1:**
  - Pin 1 -> GPIO 0, Pin 2 -> GND (Internal PULLUP)
- **Relay 1 (Fan):**
  - IN -> GPIO 5, VCC -> 5V, GND -> GND
- **Relay 2 (Mist):**
  - IN -> GPIO 23, VCC -> 5V, GND -> GND
- **Alert LED:**
  - Anode -> GPIO 18, Cathode -> 220Ω -> GND

---

## 🏆 โจทย์ท้าทายการทดลอง (Grand Challenge)
ศึกษาและเขียนโค้ดเพิ่มเติมในไฟล์ `src/main.cpp` ตรงจุดที่มีคอมเมนต์ `// TODO`:
1. **Inverted Header Bar:** วาดแถบข้อความหัวข้อแบบพื้นหลังทึบตัวอักษรสีดำ (`fillRect` และ `setTextColor(SSD1306_BLACK, SSD1306_WHITE)`)
2. **DHT Readings & NaN Check:** อ่านค่าอุณหภูมิและความชื้น พร้อมตรวจสอบ `isnan()`
3. **ADC Progress Bar:** วาดกรอบสี่เหลี่ยม (`drawRect`) และแถบพลังงานตามเปอร์เซ็นต์ค่าแอนะล็อก (`fillRect`)
4. **Relay & Toggle Counter:** สลับเปิด-ปิด Relay เมื่อกดปุ่ม พร้อมแสดงผลจำนวนรอบบนจอ OLED
