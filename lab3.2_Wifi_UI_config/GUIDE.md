# ⚡ คู่มือการปฏิบัติการ ใบงานที่ 3.2: การพัฒนาระบบตั้งค่า Wi-Fi ผ่านหน้าเว็บ UI พร้อมโหมด Debug บนจอ OLED (Web Wi-Fi Configurator & OLED Debug Display)

---

## 🎯 1. บทนำและวัตถุประสงค์การเรียนรู้ (Objectives)

ใบงานนี้เป็นการบูรณาการระหว่าง **ระบบจัดเก็บข้อมูล Flash (LittleFS)**, **ระบบสื่อสารไร้สาย (Wi-Fi AP/STA)**, และ **การแสดงผลสถานะระบบผ่านจอแสดงผล OLED SSD1306 (I2C)** เพื่อสร้างอุปกรณ์ IoT ที่สามารถตั้งค่าการเชื่อมต่อเครือข่ายผ่านหน้าเว็บ Dashboard และแสดงข้อมูล Debug ได้อย่างชัดเจน

### วัตถุประสงค์:
1. เข้าใจสถาปัตยกรรม **Captive Portal** และการสลับโหมดระหว่าง **Access Point (AP)** และ **Station (STA)**
2. สามารถจัดเก็บและอ่านการตั้งค่าเครือข่ายจาก Flash Memory ในรูปแบบ JSON (`/config.json`) ผ่าน **LittleFS**
3. สามารถพัฒนา Web Dashboard สำหรับสั่งสแกนหาเครือข่าย (`/api/scan`) และบันทึกรหัสผ่าน Wi-Fi (`/api/save`)
4. สามารถแสดงผล **OLED Debug Display (SSD1306 128x64 I2C 0x3C)**:
   - **เมื่ออยู่ในโหมด AP (ตั้งค่า):** แสดงชื่อ Hotspot `SSID: ESP_WiFi_Config` และ `IP: 192.168.4.1`
   - **เมื่อเชื่อมต่อสำเร็จโหมด STA (ออนไลน์):** แสดงชื่อ `SSID` ของเครือข่าย, หมายเลข `IP Address` ที่ได้รับจากเราเตอร์, และค่าความแรงสัญญาณ `RSSI (dBm)`
5. สามารถเขียนตรรกะตรวจจับการกดปุ่มทางกายภาพ **GPIO 0 ค้างไว้ 3 วินาที (Long-Press)** เพื่อล้างค่า Wi-Fi และเข้าสู่โหมดตั้งค่าใหม่ พร้อมแสดงผลแจ้งเตือนบนจอ OLED

---

## 🔌 2. รายการอุปกรณ์และการเชื่อมต่อวงจร (Hardware Wiring)

### รายการอุปกรณ์ที่ใช้:
* บอร์ดไมโครคอนโทรลเลอร์ ESP32 (IPST-WiFi) หรือ ESP8266 (AX-WiFi / NodeMCU)
* จอแสดงผล OLED SSD1306 (128x64 พิกเซล, I2C Address `0x3C`)
* ไฟแสดงสถานะ Status LED (GPIO 2)
* ปุ่มกดสวิตช์ SW1 / FLASH (GPIO 0)
* สายเชื่อมต่อ Micro-USB / USB-C

### ตารางการต่อวงจรและตำแหน่งขาฮาร์ดแวร์:

| อุปกรณ์ / สัญญาณ | หน้าที่การทำงาน | ESP32 (IPST-WiFi) | ESP8266 (AX-WiFi) | ระดับสัญญาณทางตรรกะ |
| :--- | :--- | :--- | :--- | :--- |
| **OLED SDA (Data)** | ขาสัญญาณข้อมูล I2C Data | <span class="badge-pin">GPIO 21</span> | <span class="badge-pin">GPIO 4 (D2)</span> | I2C Address `0x3C` |
| **OLED SCL (Clock)** | ขาสัญญาณนาฬิกา I2C Clock | <span class="badge-pin">GPIO 22</span> | <span class="badge-pin">GPIO 5 (D1)</span> | I2C Clock Line |
| **Push Button (SW1 / FLASH)** | ปุ่ม Factory Reset (กดค้าง 3 วินาที) | <span class="badge-pin">GPIO 0</span> (SW1) | <span class="badge-pin">GPIO 0 (D3)</span> (FLASH) | Active LOW (มี Internal Pull-up) |
| **Status LED** | ไฟแสดงสถานะเชื่อมต่อ / กะพริบแจ้งเตือน | <span class="badge-pin">GPIO 2</span> (Built-in) | <span class="badge-pin">GPIO 2 (D4)</span> (SLED) | Active HIGH (ESP32) / Active LOW (ESP8266) |
| **SoftAP Default IP** | IP Address โหมดปล่อยสัญญาณตั้งค่า | `192.168.4.1` | `192.168.4.1` | พอร์ต HTTP: 80 / พอร์ต DNS: 53 |

---

## 🖥️ 3. รูปแบบการแสดงผลบนจอ OLED (OLED Debug Displays)

```text
+--------------------------------+      +--------------------------------+
| === [AP MODE] ===              |      | === [STA ONLINE] ===           |
| SSID: ESP_WiFi_Config          |      | SSID: Home_WiFi_2.4G           |
| IP  : 192.168.4.1              | ---> | IP  : 192.168.1.145            |
| Portal: http://                |      | RSSI: -54 dBm                  |
| 192.168.4.1 (Port 80)          |      | Signal: Excellent (++)         |
+--------------------------------+      +--------------------------------+
    (หน้าจอขณะรอผู้ใช้ตั้งค่า Wi-Fi)           (หน้าจอเมื่อเชื่อมต่อ Wi-Fi สำเร็จ)
```

---

## 🔄 4. แผนผังการทำงานของระบบ (System Workflow)

```mermaid
flowchart TD
    Start([เริ่มการทำงาน Setup]) --> InitOLED[1. เริ่มต้นระบบ I2C และจอ OLED SSD1306]
    InitOLED --> MountFS[2. เมานต์ระบบไฟล์ LittleFS]
    MountFS --> CheckConfig{มีไฟล์ /config.json หรือไม่?}
    
    CheckConfig -- มีไฟล์ config.json --> TrySTA[3. โหลด SSID/Pass แล้วเชื่อมต่อโหมด STA]
    TrySTA --> ConnectSuccess{เชื่อมต่อ Wi-Fi สำเร็จใน 12.5s?}
    
    ConnectSuccess -- สำเร็จ --> STAOnline[4. เข้าสู่ Normal Online Mode<br/>จอ OLED แสดง SSID, IP, RSSI<br/>Status LED กะพริบ 3 ครั้ง]
    ConnectSuccess -- ล้มเหลว/Timeout --> StartAP[5. เข้าสู่โหมด AP Config Portal<br/>SSID: ESP_WiFi_Config<br/>จอ OLED แสดง SSID และ IP 192.168.4.1]
    CheckConfig -- ไม่มีไฟล์ --> StartAP
    
    StartAP --> WebPortal[6. รัน DNS Port 53 + Web Server Port 80<br/>ให้บริการหน้า Web Setup Dashboard]
    WebPortal --> UserSubmit[7. ผู้ใช้เลือก SSID และกรอกรหัสผ่าน]
    UserSubmit --> SaveJSON[8. บันทึกค่าลง /config.json ผ่าน LittleFS]
    SaveJSON --> RestartDevice[9. รีสตาร์ตระบบ (ESP.restart)]
    RestartDevice --> Start
    
    STAOnline --> CheckButton{ผู้ใช้กดปุ่ม GPIO 0<br/>ค้างเกิน 3 วินาที?}
    CheckButton -- ใช่ (Long Press) --> EraseConfig[10. OLED แสดง >> FACTORY RESET << <br/>ลบไฟล์ /config.json และกะพริบ LED 5 ครั้ง]
    EraseConfig --> RestartDevice
    CheckButton -- ไม่ใช่ --> STAOnline
```

---

## ✍️ 5. เฉลยช่องว่างโค้ดโปรแกรม (Code Blanks)

1. **ช่องที่ 1:** `LittleFS.open("/config.json", "r")` $\rightarrow$ เปิดอ่านไฟล์การตั้งค่าจาก Flash Memory
2. **ช่องที่ 2:** `WiFi.softAP(apSSID)` $\rightarrow$ สั่งให้ชิปปล่อยสัญญาณ Hotspot Wi-Fi ในโหมด Access Point
3. **ช่องที่ 3:** `dnsServer.start(DNS_PORT, "*", apIP)` $\rightarrow$ เริ่มต้นบริการ DNS Server เพื่อดักจับทุก Domain Name ส่งต่อไปยังหน้า Portal (`192.168.4.1`)
4. **ช่องที่ 4:** `serializeJson(doc, file)` $\rightarrow$ แปลงข้อมูล JSON เป็นข้อความและบันทึกลงในไฟล์ LittleFS
5. **ช่องที่ 5:** `LittleFS.remove("/config.json")` $\rightarrow$ ลบไฟล์บันทึกรหัสผ่าน Wi-Fi เพื่อทำ Factory Reset

---

## 🧠 6. เฉลยแบบทดสอบและคำถามท้ายการทดลอง

### แบบทดสอบปรนัย (Quiz):
* **ข้อที่ 1 (ตอบ ข - 1b):** โหมด `WIFI_AP_STA` ช่วยให้ ESP สามารถเปิด Access Point ให้ผู้ใช้เชื่อมต่อ ในขณะที่ยังสามารถสั่งสแกนหา Wi-Fi รอบตัวได้พร้อมกัน
* **ข้อที่ 2 (ตอบ ง - 2d):** พอร์ต DNS หมายเลข 53 ใช้สำหรับดักจับคำขอ Domain Name จากสมาร์ตโฟน แล้ว Redirect ไปยัง IP `192.168.4.1` เพื่อให้หน้าเว็บตั้งค่าเด้งขึ้นมาอัตโนมัติ (Captive Portal)
* **ข้อที่ 3 (ตอบ ก - 3a):** ข้อดีของการเก็บข้อมูลในไฟล์ `/config.json` คือ ข้อมูลไม่สูญหายเมื่อปิดเครื่อง (Non-volatile) และสามารถแก้ไขหรือขยายโครงสร้าง JSON ได้ง่าย
* **ข้อที่ 4 (ตอบ ค - 4c):** การใช้ฟังก์ชัน `millis()` จับเวลาช่วยให้ระบบตรวจสอบการกดค้างได้แบบ Non-blocking โดยไม่ทำให้วงจรหลักและ Web Server หยุดชะงัก
* **ข้อที่ 5 (ตอบ ก - 5a):** เมื่อกดปุ่มค้างครบ 3 วินาที โปรแกรมจะสั่งลบไฟล์ `/config.json` กะพริบ LED แจ้งเตือน และรีสตาร์ตเพื่อเข้าสู่โหมดตั้งค่าใหม่

### คำถามวิเคราะห์เชิงลึก:
1. **ระบบ Captive Portal ดักจับคำขอของผู้ใช้ได้อย่างไร?**
   > เมื่อสมาร์ตโฟนเชื่อมต่อ Hotspot `ESP_WiFi_Config` ระบบปฏิบัติการจะพยายามส่งคำขอ DNS ไปยังพอร์ต 53 เพื่อทดสอบอินเทอร์เน็ต (เช่น generate_204) DNS Server บน ESP จะตอบกลับว่าทุก URL ชี้มาที่ `192.168.4.1` ทำให้หน้าต่าง Web Wi-Fi Setup Dashboard เปิดขึ้นมาบนหน้าจอมือถือโดยอัตโนมัติ
2. **เหตุใดจึงควรใช้ LittleFS จัดเก็บค่า Config แทนการใช้ EEPROM แบบเดิม?**
   > LittleFS มีระบบ Wear Leveling ช่วยกระจายการเขียน ไม่ทำลาย Flash Memory ส่วนเดิมซ้ำๆ ทนทานต่อกรณีไฟดับกะทันหัน และจัดการข้อมูลในรูปแบบไฟล์ JSON ที่มีความยืดหยุ่นสูงกว่าการจำกัด Address Byte แบบ EEPROM ดั้งเดิม
3. **การออกแบบฟังก์ชันตรวจจับการกดปุ่ม Reset ด้วย millis() ดีกว่าการใช้ delay(3000) อย่างไร?**
   > หากใช้ `delay(3000)` ตัวไมโครคอนโทรลเลอร์จะหยุดการทำงานทั้งหมด (CPU Blocked) ไม่สามารถตอบสนองคำขอ HTTP หรือประมวลผลเครือข่ายได้ การใช้ `millis()` เป็นการตรวจสอบแบบ Non-blocking ทำให้บอร์ดประมวลผลงานอื่นได้อย่างลื่นไหลตลอดเวลา
