# 🌐 ใบงานที่ 3.1: การพัฒนา IoT Web Dashboard ด้วย LittleFS และ REST API

คู่มือการทดลองสร้าง **Embedded HTTP Web Server (Port 80)** บนบอร์ดไมโครคอนโทรลเลอร์ ESP32 และ ESP8266 เพื่อให้บริการไฟล์หน้าเว็บ (HTML, CSS, JavaScript) จากระบบแฟ้มข้อมูล **LittleFS Flash Memory** พร้อมทั้งพัฒนา **REST API Endpoint (`/api/data` และ `/api/control`)** เพื่ออ่านค่าเซ็นเซอร์ DHT11, สัญญาณอนาล็อก และควบคุมรีเลย์แบบ 2 ทิศทาง (ระบบอัตโนมัติ 2 เงื่อนไข: อุณหภูมิควบคุมพัดลม และอนาล็อกควบคุมปั๊มหมอก)

---

## 🎯 วัตถุประสงค์การเรียนรู้ (Objectives)

1. เข้าใจหลักการทำงานของ **HTTP Protocol** (Port 80) และสถาปัตยกรรม Stateless Client-Server
2. เข้าใจโครงสร้างพาร์ติชัน Flash Memory และการใช้งานระบบไฟล์ **LittleFS** ร่วมกับบอร์ดไมโครคอนโทรลเลอร์
3. สามารถพัฒนา **REST API Endpoint (`/api/data` และ `/api/control`)** เพื่อแลกเปลี่ยนข้อมูลค่าอุณหภูมิ/ความชื้นจาก DHT11 และค่าอนาล็อกในรูปแบบ JSON
4. รู้วิธีการใช้ฟังก์ชัน `server.streamFile()` เพื่อสตรีมไฟล์ขนาดใหญ่ไปยังเว็บเบราว์เซอร์โดยตรงโดยไม่สิ้นเปลืองหน่วยความจำ RAM (ป้องกัน Heap Overflow)
5. สามารถพัฒนาตรรกะควบคุมอัตโนมัติ (Automation Thresholds): อุณหภูมิควบคุม Relay 1 (พัดลม) และค่าอนาล็อกควบคุม Relay 2 (ปั๊มหมอก) ควบคู่กับ Web Single Page Application (SPA) บน LittleFS

---

## 🔌 การเชื่อมต่อวงจรฮาร์ดแวร์ (Hardware Circuit - ต่อยอดจาก Lab 1)

| อุปกรณ์ / โมดูล | ขา ESP32 | ขา ESP8266 | โหมดการทำงาน | หน้าที่การทำงาน |
| :--- | :---: | :---: | :---: | :--- |
| **DHT11 Sensor** (Temp & Humidity) | `GPIO 33` | `D3` (`GPIO 0`) | `DIGITAL INPUT` | วัดค่าอุณหภูมิ (°C) และความชื้นสัมพัทธ์ (%RH) |
| **Analog Sensor** (Potentiometer / VR) | `GPIO 36` (VP / A0) | `A0` | `ANALOG INPUT` | อ่านสัญญาณแรงดันอนาล็อก (0–4095) |
| **Relay 1 Module** (พัดลมระบายความร้อน) | `GPIO 5` | `D7` (`GPIO 13`) | `OUTPUT (Active-LOW)` | ควบคุมพัดลมตามเงื่อนไขอุณหภูมิ DHT11 / สั่งผ่าน Web |
| **Relay 2 Module** (ปั๊มพ่นหมอก / รดน้ำ) | `GPIO 23` | `D0` (`GPIO 16`) | `OUTPUT (Active-LOW)` | ควบคุมปั๊มหมอกตามเงื่อนไขสัญญาณอนาล็อก / สั่งผ่าน Web |
| **Status LED** (Onboard LED) | `GPIO 2` | `D4` (`GPIO 2`) | `OUTPUT` | แสดงสถานะการเชื่อมต่อ Wi-Fi และการเรียก API |

---

## 📁 โครงสร้างไฟล์ในระบบ LittleFS (`data/`)

ไฟล์หน้าเว็บทั้งหมดจะถูกจัดเก็บไว้ในโฟลเดอร์ `data/` ภายในโปรเจกต์:

```text
lab3.1/
 ├── platformio.ini         (การตั้งค่าโปรเจกต์ PlatformIO พร้อม board_build.filesystem = littlefs และ lib_deps = DHT)
 ├── index.html             (เว็บแอปพลิเคชันใบงานออนไลน์พร้อมระบบ Auto-Grader)
 ├── Code.gs                (สคริปต์ Google Apps Script ตรวจคะแนนและบันทึก Google Sheets)
 ├── GUIDE.md               (คู่มือและเฉลยการทดลอง)
 └── solution/
      ├── platformio.ini
      └── src/
           └── main.cpp     (ไฟล์โปรแกรมฉบับสมบูรณ์สำหรับ PlatformIO)
      └── data/
           └── index.html   (หน้าหลัก Web Dashboard พร้อมสคริปต์ Polling API /api/data และสั่ง /api/control)
```

---

## 💻 ขั้นตอนการอัปโหลดไฟล์ขึ้น LittleFS Flash Memory

### การใช้งานผ่าน PlatformIO (แนะนำ)

1. วางไฟล์เว็บทั้งหมด (`index.html`) ไว้ในโฟลเดอร์ `solution/data/`
2. ตรวจสอบว่าใน `platformio.ini` มีการกำหนด `board_build.filesystem = littlefs`
3. เปิดหน้าต่าง Terminal ใน VS Code แล้วรันคำสั่ง:
   ```bash
   # อัปโหลดระบบไฟล์ LittleFS ไปยังบอร์ด ESP32
   pio run -e ipst_wifi -t uploadfs

   # จากนั้น Build และ Flash เฟิร์มแวร์หลัก
   pio run -e ipst_wifi -t upload
   ```
4. หรือคลิกที่ไอคอน **PlatformIO** แถบด้านข้าง -> ขยายเมนู **Platform** -> คลิก **Upload Filesystem Image**

---

## ✍️ เฉลยคำตอบและแนวคิดในใบงาน (Worksheet Answers)

### 1. เฉลยโค้ดเติมคำตอบ (Skeleton Code Blanks)

| ช่องที่ | ฟังก์ชัน / คำตอบ | คำอธิบาย |
| :---: | :--- | :--- |
| **ช่องที่ 1** | `LittleFS.begin(true)` หรือ `LittleFS.begin()` | เมานต์ระบบไฟล์ LittleFS บน Flash (พารามิเตอร์ `true` ใน ESP32 สั่งให้ Format อัตโนมัติหากเมานต์ครั้งแรกไม่สำเร็จ) |
| **ช่องที่ 2** | `server.on("/api/data", HTTP_GET, handleApiData)` | กำหนด REST API Route สำหรับส่งค่าเซ็นเซอร์และสถานะรีเลย์กลับเป็น JSON |
| **ช่องที่ 3** | `LittleFS.exists(path)` | ตรวจสอบว่าไฟล์ตาม URI ที่ร้องขอมีอยู่ใน Flash Memory หรือไม่ |
| **ช่องที่ 4** | `server.streamFile(file, dataType)` | สตรีมไฟล์ตรงจาก Flash Memory ไปยัง Client ทีละ Chunk เพื่อประหยัด RAM |
| **ช่องที่ 5** | `server.handleClient()` | ประมวลผลคำขอ HTTP ที่เข้ามายังเซิร์ฟเวอร์ในฟังก์ชัน `loop()` |

---

### 2. เฉลยแบบทดสอบปรนัย 5 ข้อ (Multiple Choice Quiz Keys)

| ข้อที่ | หัวข้อคำถาม | คำตอบที่ถูกต้อง | เหตุผลทางเทคนิค |
| :---: | :--- | :---: | :--- |
| **1** | หมายเลขพอร์ต 80 และ HTTP Protocol | **ข (1b)** | พอร์ต 80 เป็นพอร์ตมาตรฐานสำหรับโปรโตคอล HTTP ในการรับส่งคำขอแบบ Stateless Request-Response |
| **2** | จุดเด่นของระบบไฟล์ LittleFS | **ค (2c)** | LittleFS มีระบบ Wear Leveling กระจายการเขียน ยืดอายุ Flash และทนทานต่อไฟดับกะทันหัน (Power-loss Resilient) |
| **3** | เหตุใด `server.streamFile()` จึงปลอดภัยต่อ RAM | **ก (3a)** | `streamFile()` อ่านข้อมูลจาก Flash ส่งตรงทีละบล็อก (Chunk) โดยไม่ต้องโหลดไฟล์ทั้งก้อนเข้า RAM จึงไม่เกิด Heap Overflow |
| **4** | ความสำคัญของ MIME Content-Type | **ง (4d)** | เพื่อแจ้งให้เบราว์เซอร์ทราบชนิดของข้อมูล เพื่อเรนเดอร์หรือ Parse JSON ได้อย่างถูกต้องตามมาตรฐานเว็บ |
| **5** | หน้าที่ของ `server.onNotFound()` | **ก (5a)** | ทำหน้าที่เป็น Catch-all Handler จัดการคำขอ URI ที่ไม่มี Route หรือส่งต่อไปหาไฟล์ Static ใน LittleFS และตอบกลับ 404 |

---

### 3. เฉลยคำถามวิเคราะห์เชิงลึก (Analytical Questions)

**คำถามที่ 1: อธิบายกระบวนการทำงานเมื่อเบราว์เซอร์ส่งคำขอ `GET /api/data` ไปยังบอร์ด ESP32 จนกระทั่งได้รับข้อมูล JSON กลับมาแสดงผล**
> **แนวคำตอบ:** เมื่อเบราว์เซอร์ส่งคำขอ HTTP Request มายัง URI `/api/data` ฟังก์ชัน `server.handleClient()` ในลูปจะจับคู่กับ Route Handler ที่ลงทะเบียนไว้คือ `handleApiData()` จากนั้นบอร์ดจะอ่านค่าอุณหภูมิและความชื้นจาก DHT11 (`GPIO 33`) และอ่านค่า Analog จากเซ็นเซอร์ (`GPIO 36`) รวมถึงตรวจสอบสถานะ Relay 1 และ Relay 2 แล้วจัดเรียงข้อความให้อยู่ในรูปแบบ JSON String (เช่น `{"temp":34.7,"hum":65.2,"analog":1420,"relay1":false,"relay2":false}`) และส่งกลับไปยัง Client ด้วยคำสั่ง `server.send(200, "application/json", json)` เพื่อให้สคริปต์ JavaScript บนหน้าเว็บนำไป Parse และอัปเดต Gauge / Badge บนหน้าจอ

**คำถามที่ 2: การสตรีมไฟล์หน้าเว็บด้วย `server.streamFile()` แทนการอ่านไฟล์เป็น String ทั้งก้อนช่วยป้องกันปัญหา Heap Overflow ของ RAM ได้อย่างไร?**
> **แนวคำตอบ:** เนื่องจากไมโครคอนโทรลเลอร์ ESP32/ESP8266 มีหน่วยความจำ RAM (Heap) จำกัด หากใช้การอ่านไฟล์เข้ามาเก็บในตัวแปร `String` แล้วสั่ง `server.send()` ไฟล์ขนาดใหญ่ (เช่น รูปภาพหรือ CSS/JS ขนาดหลายสิบ KB) จะทำให้ RAM เต็มและบอร์ดค้าง/รีสตาร์ต การใช้ `server.streamFile()` จะใช้วิธีอ่านข้อมูลจาก Flash เป็นก้อนย่อย (Chunk Buffer ขนาดเล็ก เช่น 256–512 ไบต์) แล้วส่งออกทาง TCP Socket ทันทีวนไปจนจบไฟล์ จึงใช้ RAM น้อยมากคงที่ตลอดเวลา

**คำถามที่ 3: ในระบบ IoT การแยกบริการไฟล์หน้าเว็บ (LittleFS Static Files) ออกจาก API รับส่งข้อมูลเซ็นเซอร์ (REST API / JSON) มีข้อดีเชิงสถาปัตยกรรมอย่างไรเมื่อเทียบกับการรวมโค้ด HTML ไว้ใน String ตัวเดียวใน C++?**
> **แนวคำตอบ:** การแยกไฟล์ Static (HTML, CSS, JS) เก็บใน LittleFS ออกจาก Dynamic Data (REST API) มีข้อดีมหาศาลทางวิศวกรรมซอฟต์แวร์ (Decoupled Architecture):
> 1. **การบำรุงรักษาและพัฒนา (Maintainability):** สามารถแก้ไขหน้าตา UI, CSS และสคริปต์ JS ได้โดยตรงผ่าน VS Code โดยไม่ต้องคอมไพล์เฟิร์มแวร์ C++ ใหม่ทุกครั้ง
> 2. **ประหยัด RAM และ Flash Code Size:** ไม่ต้องเสีย Flash Code Space ไปกับการเก็บ String HTML ยาวๆ ในโค้ดโปรแกรม
> 3. **รองรับ Client ได้หลากหลาย (Reusability):** REST API Endpoint เดียวกันสามารถนำไปใช้เชื่อมต่อกับ Mobile App, Node-RED หรือระบบภายนอกอื่นๆ ได้ทันทีโดยไม่ต้องเปลี่ยนโค้ดฝั่ง Server

---

## 🏆 โค้ดเฉลยโจทย์ท้าทาย (Challenge Solution Code)

```cpp
/**
 * Lab 3.1 Challenge Solution: Smart Environment IoT Web Dashboard & REST API
 * Hardware: DHT11 (GPIO 33), Analog (GPIO 36), Relay 1 Fan (GPIO 5), Relay 2 Mist (GPIO 23)
 */
#include <Arduino.h>
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  ESP8266WebServer server(80);
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  WebServer server(80);
#endif
#include <LittleFS.h>
#include <DHT.h>

const char* ssid = "iot_512";
const char* password = "iot123456";

#define DHTPIN        33   // ขาเซ็นเซอร์ DHT11
#define DHTTYPE       DHT11
#define ANALOG_PIN    36   // สัญญาณอนาล็อก (Potentiometer / VR)
#define RELAY1_PIN     5   // Relay 1 (พัดลมระบายความร้อน - Active-LOW)
#define RELAY2_PIN    23   // Relay 2 (ปั๊มพ่นหมอก/รดน้ำ - Active-LOW)

DHT dht(DHTPIN, DHTTYPE);

bool relay1State = false;
bool relay2State = false;
bool autoMode = true;

// 1. REST API: อ่านค่าเซ็นเซอร์ (DHT11 + Analog) และสถานะรีเลย์แบบ JSON
void handleApiData() {
  float tempC = dht.readTemperature();
  float hum = dht.readHumidity();
  int rawAnalog = analogRead(ANALOG_PIN);
  
  if (isnan(tempC)) tempC = 0.0;
  if (isnan(hum)) hum = 0.0;

  String json = "{";
  json += "\"temp\":" + String(tempC, 1) + ",";
  json += "\"hum\":" + String(hum, 1) + ",";
  json += "\"analog\":" + String(rawAnalog) + ",";
  json += "\"relay1\":" + String(relay1State ? "true" : "false") + ",";
  json += "\"relay2\":" + String(relay2State ? "true" : "false") + ",";
  json += "\"auto\":" + String(autoMode ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

// 2. REST API: ควบคุมรีเลย์เปิด-ปิด (Override Control)
void handleApiControl() {
  if (server.hasArg("relay") && server.hasArg("state")) {
    String relay = server.arg("relay");
    bool state = (server.arg("state") == "1" || server.arg("state") == "true");

    if (relay == "1" || relay == "fan") {
      relay1State = state;
      digitalWrite(RELAY1_PIN, relay1State ? LOW : HIGH); // Active-LOW
    } else if (relay == "2" || relay == "mist") {
      relay2State = state;
      digitalWrite(RELAY2_PIN, relay2State ? LOW : HIGH);
    }
    server.send(200, "application/json", "{\"success\":true}");
  } else {
    server.send(400, "application/json", "{\"error\":\"Missing parameters\"}");
  }
}

// 3. Static Web Serving from LittleFS
void handleFileRequest() {
  String path = server.uri();
  if (path.endsWith("/")) path += "index.html";
  String dataType = "text/plain";
  if (path.endsWith(".html")) dataType = "text/html";
  else if (path.endsWith(".css")) dataType = "text/css";
  else if (path.endsWith(".js")) dataType = "application/javascript";
  else if (path.endsWith(".png")) dataType = "image/png";
  else if (path.endsWith(".ico")) dataType = "image/x-icon";

  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    server.streamFile(file, dataType);
    file.close();
  } else {
    server.send(404, "text/plain", "404: File Not Found");
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.printf("Dashboard URL: http://%s\n", WiFi.localIP().toString().c_str());

  #if defined(ESP8266)
  if (!LittleFS.begin()) return;
  #else
  if (!LittleFS.begin(true)) return;
  #endif

  server.on("/api/data", HTTP_GET, handleApiData);
  server.on("/api/control", HTTP_GET, handleApiControl);
  server.onNotFound(handleFileRequest);
  server.begin();
}

void loop() {
  server.handleClient();

  // 1. เงื่อนไขอุณหภูมิควบคุม Relay 1 (พัดลม)
  float tempC = dht.readTemperature();
  if (!isnan(tempC)) {
    if (tempC >= 35.0) {
      if (!relay1State) {
        relay1State = true;
        digitalWrite(RELAY1_PIN, LOW); // เปิดพัดลม (Active-LOW)
      }
    } else if (tempC < 32.0) {
      if (relay1State) {
        relay1State = false;
        digitalWrite(RELAY1_PIN, HIGH); // ปิดพัดลม
      }
    }
  }

  // 2. เงื่อนไขสัญญาณอนาล็อกควบคุม Relay 2 (ปั๊มหมอก/รดน้ำ)
  int rawAnalog = analogRead(ANALOG_PIN);
  if (rawAnalog > 2500) {
    if (!relay2State) {
      relay2State = true;
      digitalWrite(RELAY2_PIN, LOW); // เปิดปั๊มหมอก (Active-LOW)
    }
  } else if (rawAnalog <= 2000) {
    if (relay2State) {
      relay2State = false;
      digitalWrite(RELAY2_PIN, HIGH); // ปิดปั๊มหมอก
    }
  }

  delay(2);
}
```
