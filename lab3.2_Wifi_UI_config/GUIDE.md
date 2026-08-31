# ⚡ คู่มือการปฏิบัติการ ใบงานที่ 3.2: การพัฒนาระบบตั้งค่า Wi-Fi ผ่านหน้าเว็บ UI พร้อมโหมด Debug บนจอ OLED และระบบ Auto-Reconnect แบบ Event-Driven

---

## 🎯 1. บทนำและวัตถุประสงค์การเรียนรู้ (Objectives)

ใบงานนี้เป็นการบูรณาการระดับ **Capstone IoT System** ระหว่าง **ระบบจัดเก็บข้อมูล Flash (LittleFS)**, **ระบบสื่อสารไร้สาย (Wi-Fi AP/STA)**, **จอแสดงผล OLED SSD1306 (I2C)**, และ **Event-Driven Callback Architecture** เพื่อสร้างอุปกรณ์ IoT เกรดอุตสาหกรรมที่สามารถตั้งค่าผ่านหน้าเว็บ และกู้คืนสัญญาณได้อย่างชาญฉลาด

### วัตถุประสงค์:
1. เข้าใจสถาปัตยกรรม **Captive Portal** และการสลับโหมดระหว่าง **Access Point (AP)** และ **Station (STA)**
2. สามารถจัดเก็บและอ่านการตั้งค่าเครือข่ายจาก Flash Memory ในรูปแบบ JSON (`/config.json`) ผ่าน **LittleFS**
3. สามารถพัฒนา Web Dashboard สำหรับสั่งสแกนหาเครือข่าย (`/api/scan`) และบันทึกรหัสผ่าน Wi-Fi (`/api/save`)
4. สามารถประยุกต์ใช้ **Event-Driven Auto-Reconnect (`WiFi.onEvent` / `WiFiEventHandler`)**:
   - เชื่อมต่อใหม่อัตโนมัติในระดับ Background Task ทันทีที่สัญญาณหลุด
   - **Auto-Fallback to AP Portal:** หากสัญญาณหลุดเกิน **30 วินาที** ระบบจะสลับกลับมาเปิด Hotspot AP Portal ใหม่อัตโนมัติ เพื่อให้ผู้ใช้ตั้งค่าเครือข่ายใหม่ได้โดยไม่ต้องกดปุ่ม Reset
5. สามารถแสดงผล **OLED Debug Display (SSD1306 128x64 I2C 0x3C)**:
   - **โหมด AP (ตั้งค่า):** แสดงชื่อ Hotspot `SSID: ESP_WiFi_Config` และ `IP: 192.168.4.1`
   - **โหมด STA (ออนไลน์):** แสดงชื่อ `SSID` เครือข่าย, หมายเลข `IP Address`, และค่าความแรงสัญญาณ `RSSI (dBm)`
   - **โหมด Reconnecting:** แสดงเวลานับถอยหลังก่อน Auto-Fallback เข้าสู่โหมด AP (30 วินาที)
6. สามารถเขียนตรรกะตรวจจับการกดปุ่มทางกายภาพ **GPIO 0 ค้างไว้ 3 วินาที (Long-Press)** เพื่อล้างค่า Wi-Fi (Factory Reset) และเข้าสู่โหมดตั้งค่าใหม่ทันที

---

## 🔌 2. รายการอุปกรณ์และการเชื่อมต่อวงจร (Hardware Wiring)

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
+--------------------------------+      +--------------------------------+      +--------------------------------+
| === [AP MODE] ===              |      | === [STA ONLINE] ===           |      | >> RECONNECTING <<             |
| SSID: ESP_WiFi_Config          |      | SSID: Home_WiFi_2.4G           |      | Target: Home_WiFi_2.4G         |
| IP  : 192.168.4.1              | ---> | IP  : 192.168.1.145            | ---> | Signal Lost...                 |
| Portal: http://                |      | RSSI: -54 dBm                  |      | AP Fallback: 25s               |
| 192.168.4.1 (Port 80)          |      | Signal: Excellent (++)         |      | (สลับเป็น AP เมื่อครบ 30s)     |
+--------------------------------+      +--------------------------------+      +--------------------------------+
```

---

## 🔄 4. แผนผังการทำงานของระบบ (System Workflow)

```mermaid
flowchart TD
    Start([เริ่มการทำงาน Setup]) --> InitOLED[1. เริ่มต้น I2C และจอ OLED SSD1306]
    InitOLED --> InitEvents[2. ลงทะเบียน Event Handler ดักจับ Wi-Fi Disconnect/GotIP]
    InitEvents --> MountFS[3. เมานต์ระบบไฟล์ LittleFS]
    MountFS --> CheckConfig{มีไฟล์ /config.json หรือไม่?}
    
    CheckConfig -- มีไฟล์ config.json --> TrySTA[4. เชื่อมต่อโหมด STA ด้วย SSID/Pass เดิม]
    TrySTA --> ConnectSuccess{เชื่อมต่อสำเร็จใน 12.5s?}
    
    ConnectSuccess -- สำเร็จ --> STAOnline[5. เข้าสู่ STA Online Mode<br/>OLED แสดง SSID, IP, RSSI<br/>LED กะพริบ 3 ครั้ง]
    ConnectSuccess -- ล้มเหลว/Timeout --> StartAP[6. เข้าสู่โหมด AP Config Portal<br/>SSID: ESP_WiFi_Config<br/>OLED แสดง SSID และ IP 192.168.4.1]
    CheckConfig -- ไม่มีไฟล์ --> StartAP
    
    StartAP --> WebPortal[7. รัน DNS Port 53 + Web Server Port 80<br/>ให้บริการหน้า Web Setup Dashboard]
    WebPortal --> UserSubmit[8. ผู้ใช้เลือก SSID และกรอกรหัสผ่าน]
    UserSubmit --> SaveJSON[9. บันทึกค่าลง /config.json ผ่าน LittleFS]
    SaveJSON --> RestartDevice[10. รีสตาร์ตระบบ (ESP.restart)]
    RestartDevice --> Start
    
    STAOnline --> DisconnectCheck{สัญญาณ Wi-Fi หลุด?}
    DisconnectCheck -- เกิด Event Disconnect --> ReconnectBackground[11. Event Callback สั่ง WiFi.reconnect ใน Background<br/>OLED แสดงนับถอยหลัง 30s]
    ReconnectBackground --> ReconnectTimeout{หลุดเกิน 30 วินาที?}
    ReconnectTimeout -- ใช่ (เราเตอร์ดับนาน) --> StartAP
    ReconnectTimeout -- เชื่อมต่อสำเร็จ --> STAOnline
    
    STAOnline --> CheckButton{กดปุ่ม GPIO 0<br/>ค้างเกิน 3 วินาที?}
    CheckButton -- ใช่ (Factory Reset) --> EraseConfig[12. OLED แสดง FACTORY RESET<br/>ลบไฟล์ /config.json และกะพริบ LED 5 ครั้ง]
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
3. **การออกแบบระบบ Event-Driven Auto-Reconnect ร่วมกับ 30-Second Auto-Fallback ดีกว่าอย่างไร?**
   > Event-Driven ทำงานแบบ Asynchronous ในระดับ Background Task ไม่บล็อกการทำงานของ Main Loop และการมีระบบ Timeout 30 วินาทีช่วยให้เมื่อ Router เดิมปิดตัวหรือเปลี่ยนรหัสผ่าน บอร์ดจะเปิดหน้าเว็บตั้งค่า Hotspot ให้ผู้ใช้ใหม่ได้โดยอัตโนมัติทันที
