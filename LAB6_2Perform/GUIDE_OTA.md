# 📖 คู่มือการทำระบบ OTA (Over-The-Air Update) สำหรับ ESP32 และ ESP8266
### โปรเจกต์ IoT Controller (LAB6_Perform & LAB6_AppInstall)

คู่มือนี้รวบรวมขั้นตอนและสถาปัตยกรรมการอัปเดตเฟิร์มแวร์ผ่านระบบไร้สาย (OTA) ทั้ง 2 รูปแบบอย่างละเอียด พร้อมตัวอย่างโค้ดฝั่งไมโครคอนโทรลเลอร์และหน้าเว็บ PWA

---

## 📑 สารบัญ
1. [เปรียบเทียบสถาปัตยกรรม OTA ทั้ง 2 รูปแบบ](#1-เปรียบเทียบสถาปัตยกรรม-ota-ทั้ง-2-รูปแบบ)
2. [ข้อกำหนดด้าน Partition Table (ESP32 Memory Layout)](#2-ข้อกำหนดด้าน-partition-table-esp32-memory-layout)
3. [แบบที่ 1: Local Web OTA (อัปโหลดไฟล์ตรงผ่านหน้าเว็บ PWA)](#3-แบบที่-1-local-web-ota-อัปโหลดไฟล์ตรงผ่านหน้าเว็บ-pwa)
   * [3.1 โค้ดฝั่ง ESP32/ESP8266 Firmware](#31-โค้ดฝั่ง-esp32esp8266-firmware)
   * [3.2 โค้ดฝั่งหน้าเว็บ PWA (HTML + JavaScript Progress Bar)](#32-โค้ดฝั่งหน้าเว็บ-pwa-html--javascript-progress-bar)
   * [3.3 ขั้นตอนการ Export ไฟล์ `.bin` บน PlatformIO](#33-ขั้นตอนการ-export-ไฟล์-bin-บน-platformio)
4. [แบบที่ 2: Remote Cloud OTA (อัปเดตผ่านคำสั่ง MQTT + Cloud Storage)](#4-แบบที่-2-remote-cloud-ota-อัปเดตผ่านคำสั่ง-mqtt--cloud-storage)
   * [4.1 โค้ดฝั่ง ESP32/ESP8266 Firmware](#41-โค้ดฝั่ง-esp32esp8266-firmware)
   * [4.2 การฝากไฟล์ `.bin` บน Cloud (GitHub Releases / Server)](#42-การฝากไฟล์-bin-บน-cloud-github-releases--server)
   * [4.3 โครงสร้างคำสั่ง JSON ส่งผ่าน MQTT](#43-โครงสร้างคำสั่ง-json-ส่งผ่าน-mqtt)
5. [ข้อควรระวังและแนวทางป้องกันบอร์ด Bricked (Best Practices)](#5-ข้อควรระวังและแนวทางป้องกันบอร์ด-bricked-best-practices)

---

## 1. เปรียบเทียบสถาปัตยกรรม OTA ทั้ง 2 รูปแบบ

```
                      ┌─────────────────────────────────────────────────┐
                      │            ระบบ OTA (Over-The-Air Update)       │
                      └────────────────────────┬────────────────────────┘
                                               │
                    ┌──────────────────────────┴──────────────────────────┐
                    ▼                                                     ▼
     ┌─────────────────────────────┐                       ┌─────────────────────────────┐
     │  แบบที่ 1: Local Web OTA    │                       │ แบบที่ 2: Remote Cloud OTA  │
     ├─────────────────────────────┤                       ├─────────────────────────────┤
     │ • อัปโหลดตรงผ่าน Browser/PWA │                       │ • สั่งงานผ่าน Cloud MQTT    │
     │ • ทำงานในวง LAN / Wi-Fi     │                       │ • อัปเดตข้ามเครือข่าย/นอกบ้าน │
     │ • ผู้ใช้เลือกไฟล์ .bin เอง   │                       │ • บอร์ดโหลดไฟล์จาก HTTPS URL│
     │ • มี Progress Bar แสดง %     │                       │ • เหมาะกับงาน Fleet Scale   │
     └─────────────────────────────┘                       └─────────────────────────────┘
```

| คุณสมบัติ | แบบที่ 1: Local Web OTA | แบบที่ 2: Remote Cloud OTA |
| :--- | :--- | :--- |
| **ตำแหน่งอุปกรณ์** | อยู่ในวง Wi-Fi / LAN เดียวกัน | อยู่ที่ไหนก็ได้ที่ต่ออินเทอร์เน็ต |
| **ช่องทางการสื่อสาร** | HTTP POST Multipart (`/update`) | MQTT Command + HTTPS Download |
| **ความสะดวก** | ลากไฟล์ `.bin` วางในหน้าแอปได้ทันที | อัปเดตบอร์ดหลายตัวพร้อมกันอัตโนมัติ |
| **ความต้องการภายนอก** | ไม่ต้องพึ่งพา Cloud Storage | ต้องมี Web Server / GitHub ฝากไฟล์ |

---

## 2. ข้อกำหนดด้าน Partition Table (ESP32 Memory Layout)

เพื่อให้ ESP32 สามารถทำ OTA ได้ Flash Memory จะต้องถูกแบ่งออกเป็น 2 Bank (`app0` และ `app1`) เพื่อให้บอร์ดเขียนโปรแกรมใหม่ลง Bank ว่างก่อน เมื่อตรวจทานความถูกต้องสมบูรณ์ จึงจะสลับไปบูต Bank ใหม่

ในไฟล์ `platformio.ini` ควรกำหนด Partition Scheme ดังนี้:
```ini
[env:ipst_wifi]
platform = espressif32
board = esp32dev
framework = arduino
board_build.partitions = min_spiffs.csv   ; ให้พื้นที่ App Partition ขนาด 1.9MB x 2 (รองรับ OTA สบายๆ)
```

---

## 3. แบบที่ 1: Local Web OTA (อัปโหลดไฟล์ตรงผ่านหน้าเว็บ PWA)

### 3.1 โค้ดฝั่ง ESP32/ESP8266 Firmware

เพิ่ม Endpoint `/update` ใน Web Server:

```cpp
#include <Arduino.h>
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <Updater.h>
  #define WebServer ESP8266WebServer
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  #include <Update.h>
#endif

extern WebServer server;

void setupWebOTA() {
  // 1. Endpoint จัดการสถานะหลัง Flash เสร็จสิ้น
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "application/json", "{\"success\":true,\"message\":\"OTA สำเร็จ! บอร์ดกำลังรีบูต...\"}");
    delay(1000);
    ESP.restart();
  }, []() {
    // 2. Handler รับข้อมูลไบนารีแบบ Streaming Chunks
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("[OTA] เริ่มต้นอัปเดตไฟล์: %s\n", upload.filename.c_str());
      
      #if defined(ESP32)
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { // เริ่มต้น OTA Partition
          Update.printError(Serial);
        }
      #elif defined(ESP8266)
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace)) {
          Update.printError(Serial);
        }
      #endif
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      // เขียนข้อมูลลง Flash Memory ทีละ Packet
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { // สิ้นสุดการ Flash และตรวจสอบ MD5 Checksum
        Serial.printf("[OTA] อัปเดตสำเร็จ! ขนาดรวม: %u Bytes\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
}
```

---

### 3.2 โค้ดฝั่งหน้าเว็บ PWA (HTML + JavaScript Progress Bar)

ใส่การ์ด UI นี้ลงในหน้า `index.html` หรือ `config.html`:

```html
<!-- OTA Update Card -->
<div class="card" style="background: rgba(30, 41, 59, 0.85); border: 1px solid rgba(255,255,255,0.1); border-radius: 12px; padding: 20px; margin-top: 20px;">
  <h3 style="color: #38bdf8; margin-top: 0;">📦 อัปเดตเฟิร์มแวร์ไร้สาย (Web OTA)</h3>
  <p style="font-size: 0.85rem; color: #94a3b8;">เลือกไฟล์ <code>firmware.bin</code> เพื่ออัปเดตระบบไปยังไมโครคอนโทรลเลอร์โดยตรง</p>
  
  <div style="display: flex; gap: 10px; margin-top: 15px; flex-wrap: wrap;">
    <input type="file" id="otaFileInput" accept=".bin" style="flex: 1; padding: 8px; background: #0f172a; border: 1px solid #334155; border-radius: 6px; color: #f8fafc;">
    <button type="button" onclick="startWebOTA()" id="btnOtaUpload" style="background: #0284c7; color: white; border: none; padding: 8px 20px; border-radius: 6px; cursor: pointer; font-weight: 600;">🚀 Flash Firmware</button>
  </div>

  <!-- แถบ Progress Bar -->
  <div id="otaProgressWrapper" style="display: none; margin-top: 15px;">
    <div style="display: flex; justify-content: space-between; font-size: 0.8rem; color: #cbd5e1; margin-bottom: 4px;">
      <span id="otaStatusText">กำลังส่งข้อมูล...</span>
      <span id="otaPercentText">0%</span>
    </div>
    <div style="width: 100%; height: 10px; background: #334155; border-radius: 5px; overflow: hidden;">
      <div id="otaProgressBar" style="width: 0%; height: 100%; background: #38bdf8; transition: width 0.2s;"></div>
    </div>
  </div>
</div>

<script>
function startWebOTA() {
  const fileInput = document.getElementById('otaFileInput');
  if (!fileInput.files || fileInput.files.length === 0) {
    alert('กรุณาเลือกไฟล์ firmware.bin ก่อนครับ!');
    return;
  }

  const file = fileInput.files[0];
  const formData = new FormData();
  formData.append('update', file);

  const xhr = new XMLHttpRequest();
  const progressBar = document.getElementById('otaProgressBar');
  const percentText = document.getElementById('otaPercentText');
  const statusText = document.getElementById('otaStatusText');
  const progressWrapper = document.getElementById('otaProgressWrapper');
  const btn = document.getElementById('btnOtaUpload');

  progressWrapper.style.display = 'block';
  btn.disabled = true;

  // ตรวจจับเปอร์เซ็นต์การ Upload แบบ Real-time
  xhr.upload.onprogress = function(event) {
    if (event.lengthComputable) {
      const percent = Math.round((event.loaded / event.total) * 100);
      progressBar.style.width = percent + '%';
      percentText.innerText = percent + '%';
      statusText.innerText = `กำลังส่งข้อมูล (${(event.loaded/1024).toFixed(0)} KB / ${(event.total/1024).toFixed(0)} KB)...`;
    }
  };

  xhr.onload = function() {
    if (xhr.status === 200) {
      progressBar.style.background = '#10b981';
      statusText.innerText = '✅ Flash สำเร็จ! บอร์ดกำลัง Reboot ใน 3 วินาที...';
      setTimeout(() => window.location.reload(), 4000);
    } else {
      progressBar.style.background = '#ef4444';
      statusText.innerText = '❌ Flash ล้มเหลว (HTTP ' + xhr.status + ')';
      btn.disabled = false;
    }
  };

  xhr.onerror = function() {
    progressBar.style.background = '#ef4444';
    statusText.innerText = '❌ การเชื่อมต่อขัดข้อง';
    btn.disabled = false;
  };

  // ยิง POST ไปยังบอร์ด (เช่น /update หรือ http://<ESP_IP>/update)
  xhr.open('POST', '/update', true);
  xhr.send(formData);
}
</script>
```

---

### 3.3 ขั้นตอนการ Export ไฟล์ `.bin` บน PlatformIO

1. เมื่อแก้ไขโค้ดเสร็จแล้ว กดปุ่ม **Build** บน PlatformIO
2. ไฟล์ Binary จะถูกสร้างขึ้นที่โฟลเดอร์ในเครื่องของคุณ:
   * สำหรับ ESP32: `.pio/build/ipst_wifi/firmware.bin`
   * สำหรับ ESP8266: `.pio/build/ax_wifi/firmware.bin`
3. เปิดหน้าเว็บ PWA &rarr; เลือกไฟล์ `firmware.bin` นี้ &rarr; กด **Flash Firmware** ได้ทันที!

---

## 4. แบบที่ 2: Remote Cloud OTA (อัปเดตผ่านคำสั่ง MQTT + Cloud Storage)

### 4.1 โค้ดฝั่ง ESP32/ESP8266 Firmware

ใช้ไลบรารี `<HTTPUpdate.h>` เพื่อสั่งให้บอร์ดดาวน์โหลดไฟล์ไบนารีจาก URL ผ่านระบบเน็ต:

```cpp
#include <Arduino.h>
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266httpUpdate.h>
  #define HTTPUpdate ESPhttpUpdate
#elif defined(ESP32)
  #include <WiFi.h>
  #include <HTTPUpdate.h>
#endif

// ฟังก์ชันสั่งดาวน์โหลดและ Flash Firmware จาก Cloud URL
void performCloudOTA(String binUrl) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[Cloud-OTA] WiFi ไม่ได้เชื่อมต่อ!");
    return;
  }

  Serial.println("\n========================================");
  Serial.printf("[Cloud-OTA] กำลังดาวน์โหลดเฟิร์มแวร์จาก:\n%s\n", binUrl.c_str());
  Serial.println("========================================");

  WiFiClient client; // หรือ WiFiClientSecure หากเป็น HTTPS
  
  // Callback ติดตามสถานะ
  httpUpdate.onProgress([](int cur, int total) {
    Serial.printf("[Cloud-OTA] Progress: %d%%\r", (cur * 100) / total);
  });

  t_httpUpdate_return ret = httpUpdate.update(client, binUrl);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("[Cloud-OTA] ✕ ล้มเหลว! Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("[Cloud-OTA] ! ไม่มีเวอร์ชันใหม่");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("[Cloud-OTA] ✓ อัปเดตสำเร็จ! บอร์ดกำลังรีบูต...");
      break;
  }
}
```

และในส่วน `handleIncomingCommand()` ของ MQTT:
```cpp
if (cmd == "cloud_ota") {
  String url = doc["url"].as<String>();
  if (url.length() > 0) {
    performCloudOTA(url);
  }
}
```

---

### 4.2 การฝากไฟล์ `.bin` บน Cloud (GitHub Releases / Server)
1. **GitHub Releases (แนะนำ ฟรีและเสถียร)**:
   * นำไฟล์ `firmware.bin` ไปแนบใน Release ของ GitHub Repository
   * จะได้ Direct URL เช่น: `https://raw.githubusercontent.com/allarm2553/iot-lab-sheet-apps/main/releases/firmware.bin`
2. **Web Server / S3 / Firebase Storage**:
   * วางไฟล์ไว้บน Host และตั้งค่าสิทธิ์ให้เข้าถึงได้แบบ Public Read

---

### 4.3 โครงสร้างคำสั่ง JSON ส่งผ่าน MQTT
ส่งคำสั่งนี้ไปยัง Sub Topic ของอุปกรณ์ (`esp-node/<CLEAN_MAC>/control/cmd`):

```json
{
  "command": "cloud_ota",
  "version": "1.2.0",
  "url": "http://your-domain.com/firmware.bin"
}
```

---

## 5. ข้อควรระวังและแนวทางป้องกันบอร์ด Bricked (Best Practices)

1. **ห้ามตัดไฟหรือปิดสวิตช์ระหว่าง Flash**: หากไฟดับระหว่างที่บอร์ดกำลังเขียน Flash อาจทำให้บอร์ดไม่บูต ต้องกลับมาเสียบสาย USB แฟลชใหม่
2. **ตรวจสอบการเชื่อมต่อ Wi-Fi ให้เสถียร**: แนะนำให้ตั้ง Timeout ของการดาวน์โหลดไว้ที่ $\ge 30$ วินาที
3. **Rollback Safety บน ESP32**: ใน Partition แบบ Dual-Bank หากเฟิร์มแวร์ใหม่บูตไม่ขึ้น ESP32 จะ Rollback กลับไปรันเฟิร์มแวร์เดิมใน Bank ก่อนหน้าโดยอัตโนมัติ
4. **ความถูกต้องของฮาร์ดแวร์ Pin**: อย่าลืมตรวจสอบว่าไฟล์ `.bin` ที่ Build ถูกตั้งค่า Target Board ตรงกัน (ESP32 หรือ ESP8266) ก่อนกดส่งอัปเดต
