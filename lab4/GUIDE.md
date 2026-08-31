# ⚡ คู่มือการปฏิบัติการ ใบงานที่ 4: การส่งข้อมูลสองทางแบบเรียลไทม์ (Local WebSockets)

คู่มือการทดลองสร้างระบบควบคุมและแสดงผลแบบสองทาง (Full-Duplex Communication) ระหว่างหน้าเว็บแดชบอร์ดกับไมโครคอนโทรลเลอร์ ESP32 / ESP8266 ผ่านโปรโตคอล **WebSockets (Port 81)** ร่วมกับการแลกเปลี่ยนข้อมูลแบบ **JSON (ArduinoJson v7)**

---

## 🎯 1. วัตถุประสงค์การเรียนรู้ (Objectives)

1. เข้าใจความแตกต่างระหว่าง **HTTP Polling (Request-Response)** และ **WebSockets (Event-Driven Full-Duplex)**
2. สามารถสร้าง **WebSocketsServer (Port 81)** บน ESP32 / ESP8266 เพื่อรับส่งข้อมูลกับเว็บบราวเซอร์ได้แบบเรียลไทม์
3. สามารถเข้ารหัสและถอดรหัสข้อความ JSON ผ่านไลบรารี `ArduinoJson v7`
4. สามารถควบคุมเปิด-ปิดรีเลย์พัดลมและปั๊มหมอก และปรับค่าเกณฑ์อุณหภูมิ (Temperature Threshold) จากสไลเดอร์บนหน้าเว็บ พร้อมกระจายข้อมูลเซ็นเซอร์ (`broadcastTXT`) ไปยังทุกบราวเซอร์ที่เปิดอยู่พร้อมกัน

---

## 🔌 2. รายการอุปกรณ์และการเชื่อมต่อวงจร (Hardware Wiring)

| อุปกรณ์ / สัญญาณ | หน้าที่ทางตรรกะ | ESP32 (IPST-WiFi) | ESP8266 (AX-WiFi) | คำอธิบายและหมายเหตุ |
| :--- | :--- | :--- | :--- | :--- |
| **DHT11 Sensor** | วัดอุณหภูมิและความชื้น | <span class="badge-pin">GPIO 33</span> | <span class="badge-pin">GPIO 2 (D4)</span> | Data Line (Pull-up 10k) |
| **Potentiometer (VR)** | อินพุตแอนะล็อกปรับเกณฑ์ | <span class="badge-pin">GPIO 36 (KNOB)</span> | <span class="badge-pin">A0 (ADC0)</span> | 0-4095 (12-bit) / 0-1023 (10-bit) |
| **Fan Relay** | รีเลย์ขับพัดลมระบายความร้อน | <span class="badge-pin">GPIO 5</span> | <span class="badge-pin">GPIO 13 (D7)</span> | ควบคุมพัดลมอัตโนมัติ (Active HIGH) |
| **Mist Relay** | รีเลย์ปั๊มพ่นหมอก | <span class="badge-pin">GPIO 23</span> | <span class="badge-pin">GPIO 16 (D0)</span> | ควบคุมความชื้น (Active HIGH) |
| **Push Button** | ปุ่มกดสวิตช์ Manual Input | <span class="badge-pin">GPIO 0 (SW1)</span> | <span class="badge-pin">GPIO 0 (D3)</span> | Active LOW (Internal Pull-up) |
| **OLED Display** | จอแสดงผล I2C (0x3C) | <span class="badge-pin">SDA: 21 / SCL: 22</span> | <span class="badge-pin">SDA: 4 / SCL: 5</span> | SSD1306 128x64 พิกเซล |

---

## 🌐 3. สถาปัตยกรรมการสื่อสาร WebSockets vs HTTP Polling

```text
[วิธีแบบเดิม: HTTP Polling ทุก 2 วินาที]
Browser ── [GET /api/data + HTTP Header 500 Bytes] ──> ESP32 ──> [HTTP 200 OK + JSON] ──> Browser
(สิ้นเปลืองแบนด์วิดท์ เสียรอบ CPU และเกิด Latency สูง)

[วิธีแบบ Real-time: WebSockets Port 81]
Browser ── [HTTP Upgrade Request] ──> ESP32 ── [101 Switching Protocols] ──> Browser
   │                                                                            │
   ▼                                                                            ▼
Browser <═════════════════ [ Persistent TCP Full-Duplex Tunnel ] ════════════════> ESP32
(ส่งข้อมูลสองทางทันทีเมื่อมี Event ด้วย Overhead เพียง 2-6 Bytes ตอบสนองระดับมิลลิวินาที)
```

---

## ✍️ 4. เฉลยช่องว่างโค้ดโปรแกรม (Code Blanks)

1. **ช่องที่ 1:** `LittleFS.open(path, "r")` $\rightarrow$ เปิดอ่านไฟล์ HTML/CSS/JS จาก Flash Memory
2. **ช่องที่ 2:** `deserializeJson(doc, payload, length)` $\rightarrow$ ถอดรหัสข้อความ JSON Payload ที่ส่งมาจาก Client
3. **ช่องที่ 3:** `serializeJson(doc, output)` $\rightarrow$ แปลง JSON Document เป็น String เพื่อเตรียมส่งออก
4. **ช่องที่ 4:** `webSocket.broadcastTXT(output)` $\rightarrow$ ส่งข้อความกระจายไปยังทุก Web Browser ที่เชื่อมต่ออยู่
5. **ช่องที่ 5:** `JSON.stringify(msg)` $\rightarrow$ แปลง JavaScript Object เป็น JSON String ในฝั่ง Client

---

## 🧠 5. เฉลยแบบทดสอบและคำถามท้ายการทดลอง

### แบบทดสอบปรนัย (Quiz):
* **ข้อที่ 1 (ตอบ ข - 1b):** WebSockets ทำงานแบบ Full-Duplex ผ่านการเชื่อมต่อ TCP ครั้งเดียว ทำให้ส่งข้อมูลสองทางได้ทันทีโดยไม่ต้องส่ง HTTP Header ซ้ำๆ
* **ข้อที่ 2 (ตอบ ข - 2b):** ฟังก์ชัน `broadcastTXT()` กระจายข้อความ JSON ไปยังทุก Web Browser Client ที่เชื่อมต่ออยู่พร้อมกันแบบเรียลไทม์
* **ข้อที่ 3 (ตอบ ก - 3a):** `ArduinoJson` ช่วยรวมข้อมูลหลายค่าเป็นโครงสร้าง Key-Value ที่เป็นมาตรฐาน และฝั่ง JavaScript แปลงค่าได้ทันทีด้วย `JSON.parse()`
* **ข้อที่ 4 (ตอบ ค - 4c):** เหตุการณ์ที่ระบุว่ามีข้อความ String ส่งมาจาก Client คือ `WStype_TEXT`
* **ข้อที่ 5 (ตอบ ก - 5a):** บราวเซอร์ส่งข้อความ JSON ผ่าน `ws.send()` ไปยัง ESP ทันที เพื่อนำค่าใหม่ไปคำนวณ Hysteresis โดยไม่ต้องโหลดหน้าเว็บใหม่

### คำถามวิเคราะห์เชิงลึก:
1. **เปรียบเทียบความแตกต่างระหว่าง HTTP Request-Response และ WebSocket Full-Duplex?**
   > HTTP ต้องสร้างการเชื่อมต่อใหม่และส่ง Header ขนาดใหญ่ทุกครั้งที่ต้องการข้อมูล ทำให้เกิด Latency สูง ส่วน WebSocket ทำ Handshake ครั้งเดียวแล้วเปิดท่อ TCP ค้างไว้ ทำให้ส่งข้อมูลได้ทั้งสองทางทันทีที่มีการเปลี่ยนแปลงสถานะ ช่วยประหยัดแบนด์วิดท์และลด Latency ลงเหลือหลักมิลลิวินาที
2. **โครงสร้าง JSON มีประโยชน์อย่างไรในงาน IoT?**
   > JSON เป็นรูปแบบข้อความ Key-Value มาตรฐานที่อ่านง่ายและขยายได้สะดวก สามารถรวมค่าเซ็นเซอร์หลายตัว เช่น `{"temp":28.5, "humidity":65.0, "fan":true}` ส่งไปในแพ็กเก็ตเดียว ฝั่งรับสามารถดึงเฉพาะฟิลด์ที่ต้องการไปใช้งานได้ทันที
3. **การสั่ง `broadcastTXT()` เมื่อมี Client 10 เครื่องส่งผลกระทบต่อ Heap RAM อย่างไร และป้องกันอย่างไร?**
   > การ Broadcast จะต้องจัดสรร TCP Socket Buffer ให้กับทุก Client พร้อมกัน หาก Client มีจำนวนมากอาจทำให้ Heap RAM ของไมโครคอนโทรลเลอร์เหลือน้อยหรือเกิด Heap Fragmentation ได้ แนวทางป้องกันคือการจำกัดจำนวน Client สูงสุด (Max Clients), หลีกเลี่ยงการสร้าง String Object ซ้ำซ้อนในลูป, และกำหนดอัตราการ Broadcast ไม่ให้ถี่เกินไป (เช่น ทุก 1–2 วินาที)
