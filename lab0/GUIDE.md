# 📘 ใบงานที่ 0: การเตรียมสภาพแวดล้อมการพัฒนา (Arduino IDE 2.x & VS Code + PlatformIO + Wokwi) และการทดสอบอัปโหลดไฟกระพริบ (Blink LED)

คู่มือการเรียนรู้และทดลองเริ่มต้นสำหรับไมโครคอนโทรลเลอร์ **ESP32** (เช่น บอร์ด IPST-WiFi / ESP32 DevKit) ทั้งบนโปรแกรม **Arduino IDE 2.x** และ **VS Code (PlatformIO + Wokwi)**

---

## 🎯 วัตถุประสงค์ (Objectives)
1. สามารถติดตั้งและกำหนดค่าโปรแกรม **Arduino IDE 2.x** (ติดตั้งบอร์ด ESP32 Core ผ่าน Additional Boards Manager URL และไดรเวอร์ CP2102/CH340) ได้อย่างถูกต้อง
2. สามารถติดตั้งและกำหนดค่าโปรแกรม **Visual Studio Code (VS Code)** ร่วมกับส่วนขยาย **PlatformIO IDE** และ **Wokwi Simulator** ได้อย่างถูกต้อง
3. เข้าใจโครงสร้างไฟล์โปรเจกต์ของ PlatformIO (`platformio.ini`, `wokwi.toml`, `diagram.json`, `src/main.cpp`) และการจัดการโปรเจกต์บน Arduino IDE (`.ino`)
4. เข้าใจหลักการทำงานของฟังก์ชันพื้นฐานภาษา C++ บน Arduino Framework (`setup()`, `loop()`, `pinMode()`, `digitalWrite()`, `delay()`, `millis()`)
5. เข้าใจคุณลักษณะของขา **GPIO**, **Input-Only Pins** (`GPIO 34-39`) และ **Strapping Pins** (`GPIO 0, 2, 12, 15`)
6. สามารถคอมไพล์ (Build) และอัปโหลดโปรแกรมไฟกระพริบลงบนบอร์ดจริงผ่านพอร์ต USB/Serial พร้อมทั้งเปิดดูข้อความผ่าน Serial Monitor (115200 Baud) ทั้ง 2 โปรแกรม

---

## 🌐 Board Manager URLs สำหรับ Arduino IDE 2.x

| แพลตฟอร์ม | Additional Boards Manager URL | แพ็กเกจใน Boards Manager |
| :--- | :--- | :--- |
| **ESP32** | `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json` | `esp32` โดย *Espressif Systems* |
| **ESP8266** | `http://arduino.esp8266.com/stable/package_esp8266com_index.json` | `esp8266` โดย *ESP8266 Community* |

*(หมายเหตุ: สามารถใส่ทั้ง 2 URL ในช่อง Additional boards manager URLs โดยคั่นด้วยเครื่องหมายจุลภาค `,` หรือคลิกไอคอนหน้าต่างเพื่อวางทีละบรรทัด)*

---

## 🔌 การต่อวงจรและพินฮาร์ดแวร์ (Hardware Pinouts)

| ฟังก์ชัน / อุปกรณ์ | บอร์ด ESP32 (IPST-WiFi / DevKit) | บอร์ด ESP8266 (NodeMCU) | คำอธิบาย |
| :--- | :--- | :--- | :--- |
| **LED On-board** | `GPIO 2` หรือ `GPIO 18` | `GPIO 2` (`D4`) หรือ `GPIO 16` (`D0`) | LED แสดงสถานะบนตัวบอร์ด |
| **External Red LED (LED1)** | `GPIO 23` (ผ่าน R 220Ω) | `GPIO 14` (`D5`) | LED ภายนอกดวงที่ 1 |
| **External Blue LED (LED2)** | `GPIO 19` (ผ่าน R 220Ω) | `GPIO 12` (`D6`) | LED ภายนอกดวงที่ 2 |
| **Push Button (SW1)** | `GPIO 18` หรือ `GPIO 0` (Active LOW) | `GPIO 0` (`D3` - FLASH Button) | ปุ่มกดทดสอบอินพุต (INPUT_PULLUP) |
| **Input-Only Pins** | `GPIO 34, 35, 36 (VP), 39 (VN)` | `ADC0` (`A0`) | ขาอินพุต ไม่มี Internal Pull-up/down |
| **Strapping Pins** | `GPIO 0, 2, 5, 12, 15` | `GPIO 0, 2, 15` | ขากำหนด Boot Mode ห้ามต่อดึงแรงดันค้างไว้ |

---

## ✍️ เฉลยคำตอบในใบงาน (Worksheet Answers)

### 1. โค้ดเติมช่องว่าง (Code Blanks)
```cpp
void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);       // [ช่องว่าง 1: pinMode(2, OUTPUT)]
}

void loop() {
  digitalWrite(2, HIGH);    // [ช่องว่าง 2: digitalWrite(2, HIGH)]
  delay(500);
  digitalWrite(2, LOW);     // [ช่องว่าง 3: digitalWrite(2, LOW)]
  delay(500);
}
```

---

### 2. เฉลยแบบทดสอบแบบเลือกตอบ (5-Question Multiple Choice Quiz)
1. **ข้อ 1: ฟังก์ชันใดในโปรแกรม Arduino C++ ที่จะทำงานเพียงรอบเดียวตอนเริ่มต้นบูตระบบ?**
   - **เฉลย:** `b) setup()`
2. **ข้อ 2: คำสั่งใดใช้กำหนดให้ขา GPIO 2 ทำหน้าที่เป็นขาเอาต์พุตสำหรับขับโหลด LED?**
   - **เฉลย:** `c) pinMode(2, OUTPUT);`
3. **ข้อ 3: ขา GPIO ชุดใดของ ESP32 ที่เป็น Input-Only (รับค่าได้เท่านั้น ห้ามสั่ง Output)?**
   - **เฉลย:** `a) GPIO 34, 35, 36 (VP), 39 (VN)`
4. **ข้อ 4: ในไฟล์ platformio.ini หากต้องการตั้งค่าความเร็วของ Serial Monitor เป็น 115200 ต้องเขียนอย่างไร?**
   - **เฉลย:** `c) monitor_speed = 115200`
5. **ข้อ 5: เหตุใดการเขียนโค้ดไฟกระพริบด้วย millis() จึงมีประสิทธิภาพดีกว่าการใช้ delay()?**
   - **เฉลย:** `a) เป็น Non-blocking ทำให้ CPU ประมวลผลงานอื่นควบคู่ไปด้วยได้โดยไม่หยุดค้าง**

---

### 3. คำถามท้ายการทดลอง (Review Questions)

**คำถามที่ 1: อธิบายความแตกต่างและลำดับการทำงานของฟังก์ชัน `setup()` และ `loop()` ในโปรแกรมไมโครคอนโทรลเลอร์**
> **แนวคำตอบ:** ฟังก์ชัน `setup()` จะถูกเรียกทำงานเพียงรอบเดียวเมื่อไมโครคอนโทรลเลอร์เริ่มจ่ายไฟหรือกดปุ่ม Reset เหมาะสำหรับกำหนดโหมดของพิน (`pinMode`), เริ่มต้นการสื่อสาร (`Serial.begin`) หรือเชื่อมต่อ Wi-Fi จากนั้นตัวประมวลผลจะเข้าสู่ฟังก์ชัน `loop()` ซึ่งจะทำงานวนซ้ำอย่างต่อเนื่องไม่รู้จบตลอดเวลาที่บอร์ดมีกระแสไฟเลี้ยง ใช้สำหรับการอ่านค่าเซ็นเซอร์และประมวลผลตรรกะควบคุม

**คำถามที่ 2: ขา GPIO ใดของ ESP32 ที่เป็น Input-Only และ Strapping Pins มีข้อควรระวังอย่างไรในการต่อวงจรภายนอก?**
> **แนวคำตอบ:** ขา `GPIO 34, 35, 36 (VP), 39 (VN)` เป็นขา Input-Only ที่ไม่มีวงจร Internal Pull-up / Pull-down ภายใน จึงไม่สามารถสั่งจ่ายแรงดัน `OUTPUT` ได้ เหมาะกับรับสัญญาณแอนะล็อกหรืออินพุตภายนอกเท่านั้น ส่วน Strapping Pins (เช่น GPIO 0, 2, 12, 15) คือขาที่ชิปใช้ตรวจจับระดับแรงดันตอนบูตเพื่อเลือกโหมดรันหรือโหมดอัปโหลดเฟิร์มแวร์ หากต่อวงจรภายนอกที่ดึงระดับสัญญาณค้างไว้ อาจทำให้บอร์ดไม่ยอมบูตเข้าระบบหรืออัปโหลดโค้ดไม่สำเร็จ

**คำถามที่ 3: อธิบายประโยชน์ของการจำลองบน Wokwi Simulator ร่วมกับ VS Code ก่อนการนำโค้ดไปอัปโหลดลงบอร์ดจริง**
> **แนวคำตอบ:** การจำลองบน Wokwi Simulator ช่วยให้นักพัฒนาสามารถทดสอบตรรกะการทำงานของโปรแกรม, ตรวจสอบไวยากรณ์, ดูพฤติกรรมการกะพริบของ LED และการส่งข้อความผ่าน Serial ได้ทันทีโดยไม่ต้องต่อสายไฟหรืออุปกรณ์ฮาร์ดแวร์จริง ช่วยลดความเสี่ยงจากไฟฟ้าลัดวงจรหรือความเสียหายของอุปกรณ์ และช่วยประหยัดเวลาในการแก้จุดบกพร่อง (Debug) ได้อย่างรวดเร็ว
