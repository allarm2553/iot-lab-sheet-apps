# คู่มือการติดตั้งและแปลงแอปพลิเคชัน IoT Controller (Web to App)

โฟลเดอร์นี้ประกอบด้วยชุดหน้าเว็บและไฟล์สำเร็จรูปสำหรับแปลง **IoT Controller Dashboard** ให้กลายเป็น **แอปพลิเคชันติดตั้งบนสมาร์ทโฟน Android (.apk)** หรือ **Desktop App (.exe บน Windows / macOS)** รวมถึงการติดตั้งแบบ **PWA (Progressive Web App)**

---

## 🌟 ฟังก์ชันเด่นของ App Dashboard ตัวนี้

1. **รองรับ 2 โปรโตคอลพร้อมกัน (Hybrid Dual-Mode)**:
   - **Local WebSockets (`ws://<IP>:81/`)**: สำหรับควบคุมในวง LAN หรือต่อตรงกับ AP บอร์ด (ความหน่วงต่ำ < 10ms, ทำงานได้แม้ออฟไลน์ไม่มีเน็ต)
   - **Cloud MQTT (`wss://broker.emqx.io:8084/mqtt`)**: สำหรับมอนิเตอร์และสั่งการระยะไกลข้ามเครือข่ายผ่านอินเทอร์เน็ตได้จากทั่วทุกมุมโลก
2. **หน้าต่างตั้งค่าการเชื่อมต่อในตัว (Built-in Connection Settings Modal)**:
   - ป้อน **IP Address ของบอร์ด MCU** สำหรับเชื่อมต่อ Local WebSocket
   - ตั้งค่า **MQTT Server / Port / Username / Password**
   - ตั้งค่า **Subscribe Topic** และ **Publish Topic**
   - มีปุ่ม **⚡ สร้าง Topic จาก MAC Address** อัตโนมัติ
3. **จดจำค่าถาวร (Persistent LocalStorage)**:
   - แอปจะบันทึก IP และ Topic ล่าสุดลงในหน่วยความจำของมือถือ เมื่อเปิดแอปขึ้นมาใหม่จะเชื่อมต่ออัตโนมัติทันที
4. **ไฟล์ JavaScript ฝังตัว (Offline MQTT.js)**:
   - รวมไฟล์ `mqtt.min.js` ไว้ในตัวแอป ไม่ต้องต่อเน็ตเพื่อโหลด Library จากภายนอก

---

## 📱 วิธีที่ 1: แปลงเป็นไฟล์ติดตั้ง Android (.apk) ด้วย WebIntoApp.com (แนะนำ - ฟรี & ง่ายที่สุด)

1. ไปที่เว็บไซต์ **[https://www.webintoapp.com/](https://www.webintoapp.com/)**
2. เลือกโหมด **"Files" (อัปโหลดไฟล์ ZIP)**
3. อัปโหลดไฟล์: **[`iot_app_package.zip`](file:///Users/allarmmac/myjob_folder/MyLaB/IoT/LAB6_AppInstall/iot_app_package.zip)** (ที่เตรียมไว้ให้แล้วในโฟลเดอร์นี้)
4. ตั้งค่าข้อมูลแอป:
   - **App Name:** `IoT Controller`
   - **App Icon:** เลือก `assets/icon-512.png`
5. คลิกปุ่ม **"Make App"** แล้วรอระบบประมวลผลประมาณ 1–2 นาที
6. ดาวน์โหลดไฟล์ `.apk` แล้วโอนถ่ายไปติดตั้งบนมือถือ Android ได้ทันที!

---

## 🌐 วิธีที่ 2: ติดตั้งแบบ PWA (Progressive Web App) บนมือถือโดยตรง

หากเปิดหน้าเว็บ `index.html` ผ่านโฮสติ้ง HTTPS (เช่น GitHub Pages, Firebase หรือ Vercel):
1. **บน Android (Google Chrome):**
   - แตะที่ปุ่มเมนู 3 จุด (มุมขวาบน) &rarr; เลือก **"ติดตั้งแอป" (Install app)** หรือ **"เพิ่มลงในหน้าจอหลัก" (Add to Home screen)**
2. **บน iOS / iPhone (Safari):**
   - แตะที่ปุ่ม **แชร์ (Share Icon)** ด้านล่าง &rarr; เลือก **"เพิ่มไปยังหน้าจอโฮม" (Add to Home Screen)**
3. จะได้ไอคอนแอปบนหน้าจอมือถือ เปิดใช้งานได้เต็มจอเหมือนแอป Native ทันที

---

## 💻 วิธีที่ 3: ห่อหุ้มเป็น Desktop App (.exe) สำหรับ Windows ด้วย Electron

1. สร้างไฟล์ `main.js` และ `package.json` ในโฟลเดอร์นี้
2. ติดตั้งและรัน Electron:
   ```bash
   npm install electron --save-dev
   npx electron .
   ```
3. สั่ง Build เป็น `.exe` ด้วย `electron-packager` หรือ `electron-builder`

---

## 📂 รายการไฟล์ในโฟลเดอร์ [`LAB6_AppInstall/`](file:///Users/allarmmac/myjob_folder/MyLaB/IoT/LAB6_AppInstall):

* 📄 **`index.html`**: หน้าเว็บแดชบอร์ดหลักพร้อมหน้าต่าง Settings Modal
* 🎨 **`styles.css`**: ดีไซน์ Responsive สไตล์ Modern Dark Glassmorphism
* 📦 **`mqtt.min.js`**: ไลบรารี MQTT.js ฉบับออฟไลน์
* 📱 **`manifest.json`** & **`sw.js`**: คอนฟิก PWA & Service Worker
* 🖼️ **`assets/`**: ไอคอนแอปขนาด 192x192 และ 512x512 พิกเซล
* 🗜️ **`iot_app_package.zip`**: ไฟล์ ZIP สำเร็จรูปสำหรับอัปโหลดขึ้น WebIntoApp
