# 🌐 คู่มือการใช้งาน Multi-Device Fleet Dashboard (LAB6_Fleet)
### ระบบบริหารจัดการและควบคุมเครือข่ายอุปกรณ์ IoT หลายโหนด

---

## 🌟 จุดเด่นของระบบ Fleet Commander
1. **ระบบตรวจจับโหนดอัตโนมัติ (Auto-Discovery)**:
   * Dashboard Subscribe ไปที่ `esp-node/+/state`
   * เมื่อมีบอร์ด ESP32/ESP8266 บูตขึ้นมาและส่งข้อมูล State ระบบจะสร้างการ์ดของอุปกรณ์นั้นขึ้นมาบนหน้าจอทันทีโดยไม่ต้องตั้งค่าล่วงหน้า
2. **สรุปภาพรวม KPI ทั้งเครือข่าย (Fleet KPIs Overview)**:
   * จำนวนโหนดทั้งหมด (Total / Online / Offline)
   * อุณหภูมิเฉลี่ย และ ความชื้นเฉลี่ยรวมทั้งฟาร์ม
   * จำนวนรีเลย์พัดลมและพ่นหมอกที่เปิดทำงานอยู่ขณะนั้น
3. **ระบบตรวจสอบสถานะการเชื่อมต่อ (Heartbeat Watchdog)**:
   * ตรวจจับการส่งข้อมูลล่าสุดแบบ Real-time (`🟢 Online` หรือ `🔴 Offline` หากขาดการติดต่อนานเกิน 15 วินาที)
4. **ตั้งชื่อเล่นของอุปกรณ์ (Device Alias)**:
   * กดไอคอน ✏️ เพื่อเปลี่ยนชื่อจากรหัส MAC เช่น `246F28ABCDEF` &rarr; *"โรงเรือนที่ 1 (แปลง A)"* และบันทึกไว้ใน `localStorage`
5. **สั่งการพร้อมกันทั้งระบบ (Master Fleet Batch Actions)**:
   * 🔄 สั่งทุกโหนดเข้าโหมด Auto / Manual พร้อมกัน
   * 🌀 เปิด/ปิดพัดลมทุกตัวใน 1 คลิก
   * 🌫️ เปิด/ปิดหัวพ่นหมอกทุกตัวใน 1 คลิก
   * 🎯 ตั้งเกณฑ์อุณหภูมิและความชื้นรวมทุกโหนด (Broadcast Setpoints)
6. **โหมดจำลองโหนดเสมือน (Virtual Node Simulator)**:
   * มีปุ่ม `🧪 เปิดโหมดจำลอง (Sim Nodes)` จำลอง ESP32 3 โหนดพร้อมข้อมูลเซ็นเซอร์วิ่งขึ้นลงตามธรรมชาติ เพื่อใช้ทดสอบระบบได้ทันทีแม้ไม่มีฮาร์ดแวร์จริงหลายตัว

---

## 🚀 วิธีการเปิดใช้งาน

### 1. เปิดผ่านเว็บเบราว์เซอร์ในเครื่อง (Local Server)
* เปิดผ่านลิงก์: [http://localhost:4000/LAB6_Fleet](http://localhost:4000/LAB6_Fleet)
* หรือดับเบิลคลิกไฟล์ `LAB6_Fleet/index.html` เพื่อเปิดตรงบนเบราว์เซอร์

### 2. นำไปสร้างเป็นแอปพลิเคชันมือถือ Android APK (.apk)
* นำไฟล์ `LAB6_Fleet/iot_fleet_package.zip` ไปอัปโหลดบน [WebIntoApp.com](https://www.webintoapp.com/)
* กดสร้างแอปเพื่อดาวน์โหลดไฟล์ `.apk` ติดตั้งลงในสมาร์ตโฟนหรือแท็บเล็ตได้ทันที

---

## 📡 โครงสร้าง MQTT Topics ในระบบ Fleet

| หน้าที่ | MQTT Topic | รูปแบบข้อมูล (Payload) |
| :--- | :--- | :--- |
| **รับ Telemetry ทุกโหนด** | `esp-node/+/state` | `{"mac":"...","temperature":28.5,"humidity":60.0,"fanState":false,"mistState":false,"autoMode":true,...}` |
| **ส่งคำสั่งคุมรายโหนด** | `esp-node/<MAC>/control/cmd` | `{"command":"set_fan","state":true}` หรือ `{"command":"set_mode","autoMode":true}` |
| **ส่งคำสั่งปรับเกณฑ์รายโหนด** | `esp-node/<MAC>/control/cmd` | `{"command":"set_threshold","tempThreshold":30.0,"mistThreshold":60.0}` |
