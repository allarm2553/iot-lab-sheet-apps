# คู่มือการนำไฟล์ไปเปิดและออกแบบวงจร PCB บน EasyEDA (LAB6_Dev)

เอกสารนี้เป็นคู่มือสรุปขั้นตอนการนำไฟล์ Schematic และ BOM ของ **LAB6_Dev (ESP32 Hybrid IoT Controller)** ไปเปิดและต่อยอดในโปรแกรม **EasyEDA (Standard / Pro)** และสั่งผลิตผ่าน **JLCPCB**

---

## 📁 ไฟล์ที่เตรียมไว้ในโฟลเดอร์นี้

1. **`LAB6_Dev_Schematic_EasyEDA.json`**: โครงสร้างผังวงจร (Schematic Document & Netlist Specification)
2. **`BOM_JLCPCB_LCSC.csv`**: รายการอุปกรณ์พร้อมรหัส **LCSC Part Number** สำหรับสั่งประกอบ SMT 1-Click บน JLCPCB

---

## 🚀 ขั้นตอนที่ 1: เปิดใช้งานและสร้างโปรเจกต์ใน EasyEDA

1. เปิดเว็บเบราว์เซอร์แล้วไปที่ **[https://easyeda.com/editor](https://easyeda.com/editor)** (หรือเปิด EasyEDA Desktop Client)
2. เข้าสู่ระบบ (Log in) บัญชี EasyEDA ของคุณ
3. สร้างโปรเจกต์ใหม่:
   - คลิกเมนู **File** &rarr; **New** &rarr; **Project**
   - ตั้งชื่อโปรเจกต์: `LAB6_Dev_ESP32_IoT_Controller`

---

## 🔌 ขั้นตอนที่ 2: การนำเข้า Schematic และเชื่อมต่อขาอุปกรณ์

หากต้องการวาด Schematic ตามโครงสร้างที่ออกแบบไว้:

### ตารางการเชื่อมต่อขา (Netlist Map)

| Net Name | อุปกรณ์ต้นทาง | อุปกรณ์ปลายทาง | หมายเหตุ |
| :--- | :--- | :--- | :--- |
| **VCC_5V** | Type-C VBUS (J1) &rarr; SS34 (D3) | AMS1117 VIN, ขดลวด Relay K1/K2 (+), PC817 Pin 4 | ไฟเลี้ยงหลัก 5V |
| **VCC_3V3** | AMS1117 VOUT (U2) | ESP32 3V3, OLED VCC, DHT VCC, VR VCC, Pull-up Resistors | ไฟเลี้ยงระบบ MCU 3.3V |
| **GND** | รวม Ground ทุกภาค | Solid Ground Plane ทั่วทั้งบอร์ด | ระนาบกราวด์รวม |
| **GPIO5_FAN** | ESP32 Pin GPIO 5 | Resistor 1k (R6) &rarr; PC817 Optocoupler &rarr; SS8050 &rarr; Relay 1 | ขับรีเลย์พัดลม |
| **GPIO23_MIST** | ESP32 Pin GPIO 23 | Resistor 1k (R7) &rarr; PC817 Optocoupler &rarr; SS8050 &rarr; Relay 2 | ขับรีเลย์พ่นหมอก |
| **GPIO21_SDA** | ESP32 Pin GPIO 21 | Header OLED (SDA) + 4.7k Pull-up (R1) | สัญญาณ I2C Data |
| **GPIO22_SCL** | ESP32 Pin GPIO 22 | Header OLED (SCL) + 4.7k Pull-up (R2) | สัญญาณ I2C Clock |
| **GPIO33_DHT** | ESP32 Pin GPIO 33 | Header DHT (DATA) + 10k Pull-up (R3) | ข้อมูลเซนเซอร์อุณหภูมิ/ชื้น |
| **GPIO36_ADC** | ESP32 Pin GPIO 36 (VP) | Header VR (Wiper/OUT) + C 100nF (C6) | สัญญาณ Analog ลูกบิด VR |
| **GPIO0_SW1** | ESP32 Pin GPIO 0 | Tactile Switch (SW1) + 10k Pull-up + C 100nF | สวิตช์กดติด-ปล่อยดับ |

---

## 🖨️ ขั้นตอนที่ 3: การแปลงเป็นแผ่น PCB (Convert to PCB)

1. ในหน้า Schematic ของ EasyEDA คลิกที่เมนู:
   - **Design** &rarr; **Convert Schematic to PCB**
2. ตั้งค่าขนาดบอร์ด (แนะนำขนาดกะทัดรัดประมาณ **80 mm x 60 mm** หรือ **100 mm x 75 mm**)
3. **การจัดวางชิ้นส่วน (Component Placement)**:
   - **ESP32 Module**: วางให้ **ส่วนเสาอากาศ WiFi (PCB Antenna) ยื่นพ้นขอบบอร์ด** หรืออย่าให้มีระนาบทองแดงอยู่ข้างใต้
   - **Terminal Blocks (TB1, TB2)**: วางชิดขอบบอร์ดด้านใดด้านหนึ่ง เพื่อความสะดวกในการต่อสายไฟ AC 220V
   - **OLED Socket**: วางในตำแหน่งที่มองเห็นได้ชัดเจน
   - **Type-C / DC Jack**: วางชิดขอบบอร์ดฝั่งตรงข้ามกับรีเลย์
4. **ระยะห่างความปลอดภัย (High Voltage Isolation)**:
   - ลากเส้น **Milling Slot (รูเจาะทะลุ)** บน Layer `BoardOutLine` หรือ `KeepOut` ระหว่างหน้าสัมผัสขารีเลย์กับวงจรแรงดันต่ำ
   - เททองแดง (Copper Pour) **GND** ที่ Top Layer และ Bottom Layer

---

## 📦 ขั้นตอนที่ 4: การสั่งผลิตผ่าน JLCPCB พร้อมประกอบชิ้นส่วน (SMT Assembly)

1. คลิกเมนู **Fabrication** &rarr; **PCB Order (JLCPCB)**
2. Export ไฟล์ Gerber และ BOM:
   - **Gerber File**: สั่งผลิตแผ่นวงจรพิมพ์
   - **BOM File**: นำเข้าไฟล์ `BOM_JLCPCB_LCSC.csv` ที่แนบไว้
   - **CPL / Pick and Place File**: พิกัดวางชิ้นส่วน
3. ระบบจะค้นหาอะไหล่จากรหัส **LCSC Part Number** โดยอัตโนมัติ ทำให้ไม่ต้องค้นหาอะไหล่ด้วยตนเอง
