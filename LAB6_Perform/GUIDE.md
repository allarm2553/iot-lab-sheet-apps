# ใบงานที่ 6 Perform: ระบบกำหนดค่าฮาร์ดแวร์ผ่าน WebConfig & ระบบสร้าง MQTT Topic อัตโนมัติตาม MAC Address

## 🎯 วัตถุประสงค์ (Objectives)
1. เข้าใจสถาปัตยกรรมการแยกหน้าเว็บควบคุม (**Dashboard: `/index.html`**) ออกจากหน้าเว็บกำหนดค่าระบบ (**WebConfig Portal: `/config.html`**)
2. สามารถบันทึกและอ่านค่าคอนฟิกฮาร์ดแวร์ (Wi-Fi, MQTT, ชนิดเซ็นเซอร์ DHT, พินรีเลย์, พิน ADC และสวิตช์) ลงใน **LittleFS Flash Memory (`/config.json`)** ได้อย่างปลอดภัย
3. สามารถดึงค่า **Device MAC Address** ของไมโครคอนโทรลเลอร์ ESP32 มาสร้าง **Subscribe / Publish Topic** ประจำบอร์ดโดยอัตโนมัติ เพื่อป้องกันปัญหาข้อมูลชนกัน (Topic Collision) บน Cloud Broker
4. สามารถเปิดระบบ **Fallback AP Mode (192.168.4.1)** พร้อมระบบ Captive Portal เมื่อต่อ Wi-Fi ไม่สำเร็จ

---

## 🏗️ โครงสร้างของระบบ (System Architecture)

```
                       ┌───────────────────────────────┐
                       │     LittleFS Flash Memory     │
                       │         (/config.json)        │
                       └───────────────┬───────────────┘
                                       │ Load on Boot / Save on POST
                                       ▼
                       ┌───────────────────────────────┐
                       │   ESP32 Embedded Web Server   │
                       │           (Port 80)           │
                       └───────┬───────────────┬───────┘
                               │               │
            ┌──────────────────┴──┐         ┌──┴──────────────────┐
            │   /index.html       │         │    /config.html     │
            │  (Dashboard ควบคุม)  │         │  (WebConfig Portal) │
            └─────────────────────┘         └─────────────────────┘
```

---

## ⚙️ พารามิเตอร์ที่สามารถกำหนดค่าผ่าน `config.html`

| กลุ่มการตั้งค่า | พารามิเตอร์ | คำอธิบาย | ค่าเริ่มต้น (Default) |
| :--- | :--- | :--- | :--- |
| **Wi-Fi** | `ssid` | ชื่อสัญญาณเครือข่าย Wi-Fi | `iot_512` |
| | `password` | รหัสผ่าน Wi-Fi | `iot123456` |
| **MQTT** | `mqttServer` | URL หรือ IP ของ MQTT Broker | `broker.emqx.io` |
| | `mqttPort` | พอร์ตเชื่อมต่อ MQTT | `1883` |
| | `mqttUser` | Username สำหรับยืนยันตัวตน | `elec` |
| | `mqttPassword` | Password สำหรับยืนยันตัวตน | `elec1234` |
| **Hardware** | `dhtType` | ชนิดของเซ็นเซอร์อุณหภูมิ/ความชื้น | `11` (DHT11) หรือ `22` (DHT22) |
| | `dhtPin` | ขา GPIO ต่อ Data เซ็นเซอร์ DHT | `GPIO 33` (IPST-WiFi) |
| | `analogPin` | ขา GPIO รับค่าแอนะล็อก ADC | `GPIO 36` (VP) |
| | `fanRelayPin` | ขา GPIO ขับรีเลย์พัดลม | `GPIO 5` |
| | `mistRelayPin`| ขา GPIO ขับรีเลย์พ่นหมอก | `GPIO 23` |
| | `buttonPin` | ขา GPIO สวิตช์ปุ่มกด SW1 | `GPIO 0` |

---

## 🔑 ระบบสร้าง MQTT Topic อัตโนมัติ (Auto MAC Topics)

ในการทำงานจริง เมื่อมีบอร์ด ESP32 หลายสิบตัวเชื่อมต่อไปยัง Public Broker ตัวเดียวกัน (เช่น `broker.emqx.io`) หากใช้ Topic เดียวกัน บอร์ดจะแย่งกันรับส่งคำสั่งทำให้ระบบรวน

**การแก้ไขใน LAB6_Perform:**
ระบบจะอ่านค่า Hardware MAC Address ของชิป Wi-Fi เช่น `24:6F:28:AB:CD:EF` แล้วตัดเครื่องหมาย `:` ออกเป็น `246F28ABCDEF` จากนั้นสร้าง Topic เฉพาะตัว:

* **Subscribe Topic (รับคำสั่ง):** `esp-node/246F28ABCDEF/control/cmd`
* **Publish Topic (ส่งสถานะ):** `esp-node/246F28ABCDEF/state`
* **Client ID:** `ESP32_246F28ABCDEF`

---

## 🚀 ขั้นตอนการทดลองและการใช้งาน

1. เปิด VS Code &rarr; เปิดโฟลเดอร์ [`LAB6_Perform/solution`](file:///Users/allarmmac/myjob_folder/MyLaB/IoT/LAB6_Perform/solution)
2. อัปโหลด Filesystem:
   - รันคำสั่ง `pio run -t uploadfs` เพื่ออัปโหลดไฟล์ HTML/CSS ในโฟลเดอร์ `data/` ไปยัง LittleFS
3. อัปโหลดโปรแกรม:
   - รันคำสั่ง `pio run -t upload` เพื่อเบิร์นเฟิร์มแวร์ลงบอร์ด ESP32
4. เปิดเบราว์เซอร์ไปที่ IP ของบอร์ด เช่น `http://192.168.1.xxx/config.html` เพื่อตั้งค่าเครือข่ายและพินอุปกรณ์
5. เมื่อบันทึกสำเร็จ บอร์ดจะรีบูตอัตโนมัติ และเข้าใช้งาน Dashboard ที่ `http://192.168.1.xxx/index.html` ได้ทันที
