---
name: IoT Lab Worksheet Development
description: Standard guidelines, templates, and best practices for creating and maintaining interactive IoT lab worksheet web applications (ESP32/ESP8266, Google Apps Script, Glassmorphism UI, Theory Modals, Wiring Tables, and Online Status Indicators).
---

# IoT Lab Worksheet Development Guidelines

This skill provides comprehensive standards, UI architecture, hardware pin mappings, and code templates for building and maintaining interactive web-based IoT laboratory worksheets (Google Apps Script frontends & standalone HTML forms).

> [!IMPORTANT]
> **🎯 Official Gold Standard Templates (ต้นแบบมาตรฐานสำหรับทุกใบงาน):**
> - **ต้นแบบใบงานปฏิบัติการและชุดจำลอง VS Code + Wokwi:** ยึด **`lab2/`** (`lab2/index.html`, `lab2/vscode-wokwi-template/`, `lab2/Code.gs`, `lab2/GUIDE.md`) และ **`lab1/`** เป็น **Gold Standard Template**
> - **ต้นแบบระบบรวมคะแนนทุกแล็บและดึงคะแนนตามรหัสนักศึกษา:** ยึด **`Master_Code.gs`**, **`config.js`**, **`index.html`**, **`dashboard.html`** และ **`server.js`** เป็น **Gold Standard Backend & Portal Template**
> - อ่านสรุปกฎระดับ Workspace เพิ่มเติมได้ที่ไฟล์ `GEMINI.md` ที่รากของโปรเจกต์

---

## 1. Core Architecture & UI Standards

Each lab worksheet (`index.html`) is designed with a responsive, glassmorphic dark theme and contains two main columns on desktop (stacked on mobile):

1. **Left Column (คู่มือใบงานการทดลอง - Lab Manual):**
   - **Card Header (`.card-title`):** Contains title icon, label, and the **Interactive Theory Viewer Button** (`.btn-theory-toggle`).
   - **Objectives Frame (`.info-frame`):** Bulleted learning objectives.
   - **Equipment Required Table (`<h2>อุปกรณ์ที่ต้องใช้</h2>`):** 3-column table (`รายการอุปกรณ์`, `รายละเอียด`, `จำนวน`).
   - **Wiring Table (`<h2>การเชื่อมต่อวงจร (Wiring)</h2>`):** Comparative pin table between ESP32 and ESP8266.
   - **Skeleton Code (`<div class="code-container">`):** Copyable Arduino/C++ template with copy button.

2. **Right Column (ฟอร์มบันทึกและส่งรายงานผล - Student Submission Form):**
   - **Teacher Google Sheets Connection Box (`.gas-connect-box`):** กล่องตั้งค่า Web App URL ของผู้สอน, ปุ่มโหลด/คัดลอก Code.gs, บันทึก URL, คัดลอกลิงก์ให้นักเรียน และทดสอบเชื่อมต่อ (อยู่บนสุดของคอลัมน์ขวา).
   - **Student Profile Form:** Name, Student ID, Class Section, Date.
   - **Code Blank Fill-in / Challenge Code:** Inputs mapped to skeleton code blanks with real-time feedback.
   - **Analytical & Theory Questions:** Textareas with auto-resize.
   - **File Upload Area:** Drag & drop screenshot and source code file upload with Base64 encoding.
   - **Conclusion & Recommendations:** Student findings and reflections.
   - **5-Question Multiple Choice Quiz:** Automated grading with Google Sheets integration.
   - **Action Buttons:** Pre-check score button, Submit report button, Reset form button, and Print to PDF export button.

---

## 2. Header & Online/Offline Status Indicator

Every worksheet header must include real-time online/offline status detection for student reliability.

### HTML Structure
```html
<header>
  <div class="logo-container">
    <i class="fa-solid fa-microchip logo-icon"></i>
    <h1>ใบงานที่ X: [ชื่อการทดลอง]</h1>
  </div>
  <p class="subtitle">หลักสูตรการพัฒนาระบบ Hybrid Local/Cloud IoT Node ด้วย ESP32 / ESP8266</p>
  <div class="header-badges">
    <div class="course-badge">
      <i class="fa-solid fa-graduation-cap"></i> ระบบส่งใบงานปฏิบัติการออนไลน์
    </div>
    <div id="onlineStatusBadge" class="status-badge online">
      <span class="status-dot"></span>
      <span id="onlineStatusText">ออนไลน์ (พร้อมส่งรายงาน)</span>
    </div>
  </div>
</header>
```

### CSS Styles
```css
.header-badges {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 0.75rem;
  flex-wrap: wrap;
  margin-top: 1rem;
}

.status-badge {
  display: inline-flex;
  align-items: center;
  gap: 0.5rem;
  padding: 0.35rem 0.85rem;
  border-radius: 9999px;
  font-size: 0.75rem;
  font-weight: 600;
  transition: all 0.3s ease;
}

.status-badge.online {
  background: rgba(16, 185, 129, 0.12);
  color: #34d399;
  border: 1px solid rgba(16, 185, 129, 0.3);
}

.status-badge.offline {
  background: rgba(239, 68, 68, 0.12);
  color: #f87171;
  border: 1px solid rgba(239, 68, 68, 0.3);
}

.status-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  display: inline-block;
}

.status-badge.online .status-dot {
  background-color: #10b981;
  box-shadow: 0 0 8px #10b981;
  animation: pulse-dot 2s infinite;
}

.status-badge.offline .status-dot {
  background-color: #ef4444;
  box-shadow: 0 0 8px #ef4444;
}

@keyframes pulse-dot {
  0% { transform: scale(0.95); opacity: 0.8; }
  50% { transform: scale(1.2); opacity: 1; }
  100% { transform: scale(0.95); opacity: 0.8; }
}
```

### JavaScript Implementation
```javascript
function initOnlineStatus() {
  const badge = document.getElementById('onlineStatusBadge');
  const text = document.getElementById('onlineStatusText');
  if (!badge || !text) return;

  function update() {
    if (navigator.onLine) {
      badge.className = 'status-badge online';
      text.textContent = 'ออนไลน์ (พร้อมส่งรายงาน)';
    } else {
      badge.className = 'status-badge offline';
      text.textContent = 'ออฟไลน์ (ไม่มีการเชื่อมต่อเครือข่าย)';
    }
  }

  window.addEventListener('online', update);
  window.addEventListener('offline', update);
  update();
}

window.addEventListener('DOMContentLoaded', initOnlineStatus);
```

---

## 3. Interactive Multi-Tab Theory Modal

Provide rich educational background without cluttering the main worksheet layout.

### Button on Manual Card Header
```html
<div class="card-title" style="display: flex; justify-content: space-between; align-items: center; flex-wrap: wrap; gap: 0.5rem;">
  <div style="display: flex; align-items: center; gap: 0.5rem;">
    <i class="fa-solid fa-book-open"></i>
    <span>คู่มือใบงานการทดลอง</span>
  </div>
  <button type="button" class="btn-theory-toggle" onclick="openTheoryModal()">
    <i class="fa-solid fa-lightbulb"></i> <span>ดูเนื้อหาทฤษฎีที่เกี่ยวข้อง</span>
  </button>
</div>
```

### Modal HTML Structure
```html
<div class="modal-overlay" id="theoryModal" style="display: none;">
  <div class="theory-modal-content">
    <div class="theory-modal-header">
      <h2><i class="fa-solid fa-graduation-cap" style="color: var(--accent);"></i> [ชื่อหัวข้อทฤษฎี]</h2>
      <button type="button" class="theory-close-x" onclick="closeTheoryModal()"><i class="fa-solid fa-xmark"></i></button>
    </div>

    <div class="theory-modal-body">
      <!-- Tabs -->
      <div class="theory-tab-bar">
        <button type="button" class="theory-tab-btn active" onclick="switchTheoryTab('tab1', this)">1. หัวข้อย่อย 1</button>
        <button type="button" class="theory-tab-btn" onclick="switchTheoryTab('tab2', this)">2. หัวข้อย่อย 2</button>
      </div>

      <!-- Tab Panels -->
      <div class="theory-tab-panel active" id="theory-tab-tab1">
        <div class="theory-card">
          <h4><i class="fa-solid fa-bolt"></i> 1.1 รายละเอียดทฤษฎี</h4>
          <p>คำอธิบายเนื้อหาพร้อมตัวอย่าง...</p>
        </div>
      </div>

      <div class="theory-tab-panel" id="theory-tab-tab2">
        <div class="theory-card">
          <h4><i class="fa-solid fa-chart-line"></i> 2.1 สูตรและการคำนวณ</h4>
          <div class="theory-formula">
            // สูตรคำนวณหรือโค้ดตัวอย่างในแท็บ<br>
            float percent = (raw / 4095.0) * 100.0;
          </div>
        </div>
      </div>
    </div>

    <div class="theory-modal-footer">
      <button type="button" class="modal-close-btn" onclick="closeTheoryModal()">ปิดหน้าต่างทฤษฎี</button>
    </div>
  </div>
</div>
```

### JavaScript Controller
```javascript
function openTheoryModal() {
  const m = document.getElementById('theoryModal');
  if (m) m.style.display = 'flex';
}

function closeTheoryModal() {
  const m = document.getElementById('theoryModal');
  if (m) m.style.display = 'none';
}

function switchTheoryTab(tabKey, btn) {
  document.querySelectorAll('.theory-tab-btn').forEach(b => b.classList.remove('active'));
  document.querySelectorAll('.theory-tab-panel').forEach(p => p.classList.remove('active'));
  
  if (btn) btn.classList.add('active');
  const panel = document.getElementById('theory-tab-' + tabKey);
  if (panel) panel.classList.add('active');
}
```

---

## 4. Standard Equipment Table Format

All worksheets must present equipment requirements in a clean, responsive table:

```html
<h2>อุปกรณ์ที่ต้องใช้</h2>
<div class="table-container">
  <table>
    <thead>
      <tr>
        <th>รายการอุปกรณ์</th>
        <th>รายละเอียด</th>
        <th style="text-align: center;">จำนวน</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>บอร์ดไมโครคอนโทรลเลอร์</td>
        <td>ESP32 (IPST-WiFi) หรือ ESP8266 (AX-WiFi / NodeMCU)</td>
        <td style="text-align: center;">1 บอร์ด</td>
      </tr>
      <tr>
        <td>สาย USB</td>
        <td>USB-to-Serial (Micro-USB หรือ USB-C ตามรุ่นบอร์ด)</td>
        <td style="text-align: center;">1 เส้น</td>
      </tr>
      <tr>
        <td>คอมพิวเตอร์ + Arduino IDE</td>
        <td>ติดตั้ง ESP32 / ESP8266 Board Package เรียบร้อยแล้ว</td>
        <td style="text-align: center;">1 ชุด</td>
      </tr>
    </tbody>
  </table>
</div>
```

---

## 5. Hardware Pin Mappings Reference Table

| อุปกรณ์ / โมดูล | ขาสัญญาณ | ESP32 (IPST-WiFi) | ESP8266 (AX-WiFi) | หมายเหตุ / คำอธิบาย |
| :--- | :--- | :--- | :--- | :--- |
| **เซ็นเซอร์ DHT11** | DATA | `GPIO 33` | `GPIO 2 / D4` | อุณหภูมิและความชื้น (ไฟเลี้ยง 3.3V) |
| **Analog Potentiometer** | Signal | `GPIO 36` (KNOB-S) | `A0` (จัมเปอร์ VR) | ESP32 (12-bit, 0-4095) / ESP8266 (10-bit, 0-1023) |
| **Push Button** | SW / Input | `GPIO 0` (ปุ่ม SW1) | `GPIO 0 / D3` (ปุ่ม FLASH) | ขาอินพุตแบบ Active LOW (`INPUT_PULLUP`) |
| **Onboard LED** | Built-in LED | `GPIO 18` | `GPIO 2 / D4` | ไฟแสดงผล Built-in บนบอร์ด |
| **Relay 1 (พัดลม)** | IN | `GPIO 5` (พอร์ต 5) | `GPIO 13 / D7` | ขับพัดลมระบายความร้อน (Active HIGH) |
| **Relay 2 (ปั๊ม/พ่นหมอก)** | IN | `GPIO 23` (พอร์ต 23) | `GPIO 16 / D0` | ขับปั๊มน้ำหรือหัวพ่นหมอก (Active HIGH) |
| **จอ OLED SSD1306** | SDA / SCL | `GPIO 21` / `GPIO 22` | `GPIO 4 (D2)` / `GPIO 5 (D1)` | I2C Address `0x3C` (ติดตั้ง Onboard) |

---

## 6. Dual-Mode Diagram Rendering & Print/PDF Export Architecture

Worksheets must print cleanly for physical grading and PDF archiving without iframe/canvas rendering glitches:

### Dual-Mode Diagram Strategy
1. **Interactive Screen Mode:** The live Wokwi simulation iframe (`#wokwiEmbedFrame`) runs continuously for student experimentation. The print SVG (`.wokwi-print-diagram`) is hidden (`display: none;`).
2. **Print/PDF Export Mode (`@media print`):** 
   - All interactive controls, toolbars, buttons, and iframes are hidden (`display: none !important;`).
   - The **Vector SVG Circuit Schematic (`.wokwi-print-diagram`)** is automatically displayed (`display: block !important;`), rendering crisp 300+ DPI schematics on white A4 paper.

### CSS Rules for Print
```css
.wokwi-print-diagram {
  display: none;
}

@media print {
  body {
    background: #ffffff !important;
    color: #000000 !important;
    padding: 0 !important;
  }
  .container {
    max-width: 100% !important;
    margin: 0 !important;
    padding: 0 !important;
  }
  .glass-card, .section-card {
    background: #ffffff !important;
    border: 1px solid #cbd5e1 !important;
    box-shadow: none !important;
    backdrop-filter: none !important;
    color: #000000 !important;
    page-break-inside: avoid;
  }
  header, .course-badge, .btn-theory-toggle, .btn-submit, .btn-reset, .upload-area, .modal-overlay,
  .wokwi-header-bar, .wokwi-actions, .wokwi-iframe-wrapper, .wokwi-guide-banner {
    display: none !important;
  }
  .wokwi-print-diagram {
    display: block !important;
    padding: 0.75rem !important;
    background: #ffffff !important;
    text-align: center !important;
  }
  .wokwi-print-diagram svg {
    max-width: 100% !important;
    height: auto !important;
  }
  input[type="text"], input[type="date"], textarea {
    background: transparent !important;
    border: none !important;
    border-bottom: 1px dotted #000 !important;
    color: #000000 !important;
  }
  table {
    border-collapse: collapse !important;
    width: 100% !important;
  }
}
```

---

## 7. Interactive Inline Skeleton Code Blanks (`.code-blank`)

To maximize interactive learning, fill-in-the-blank questions must be embedded directly inside the skeleton code block (`<pre><code>`) rather than as a disconnected list.

### Pedagogical Placeholder Guidelines
Never provide direct answers inside the placeholder text. Always frame the placeholder as paired choices or category hints (e.g. `INPUT / OUTPUT`, `4095.0 / 1023.0`, `HIGH / LOW`) to encourage analytical reasoning.

### HTML Pattern
```html
<div class="code-container">
  <div class="code-header">
    <span>lab_skeleton.ino</span>
    <button type="button" class="copy-btn" onclick="copyCode()"><i class="fa-regular fa-copy"></i> Copy</button>
  </div>
  <pre><code>void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, <input type="text" id="codeBlank1" form="labForm" required class="code-blank" placeholder="INPUT / OUTPUT" style="width: 155px;">);
}</code></pre>
</div>
```

### CSS Styling
```css
.code-blank {
  background: rgba(99, 102, 241, 0.22);
  border: 1px dashed #818cf8;
  border-radius: 6px;
  padding: 0.15rem 0.55rem;
  color: #38bdf8;
  font-family: var(--mono-font), monospace;
  font-size: 0.85rem;
  font-weight: 600;
  outline: none;
  display: inline-block;
  vertical-align: middle;
  transition: all 0.2s ease;
  box-shadow: 0 0 8px rgba(99, 102, 241, 0.2);
}

.code-blank:focus {
  border-style: solid;
  border-color: #38bdf8;
  background: rgba(99, 102, 241, 0.4);
  color: #ffffff;
  box-shadow: 0 0 12px rgba(56, 189, 248, 0.45);
}

.code-blank::placeholder {
  color: rgba(165, 180, 252, 0.65);
  font-style: italic;
  font-weight: 400;
  font-size: 0.8rem;
}
```

---

## 8. Anti-Cheat & Plagiarism Prevention System

To maintain academic integrity and encourage genuine critical thinking, all worksheets implement client-side anti-plagiarism protection across Post-Lab Questions and Conclusion text areas:

### Features & Interceptions
- **Paste Prevention (`paste`):** Intercepts clipboard paste attempts and displays an animated warning toast.
- **Copy/Cut Prevention (`copy`, `cut`):** Prevents copying or cutting answers from analytical textareas.
- **Context Menu Lock (`contextmenu`):** Disables right-click context menus specifically over protected response fields.
- **Drag & Drop Lock (`drop`):** Prevents dragging external text directly into the answer boxes.
- **Whitelisting:** Code submission areas (`#challengeCode`) remain unrestricted to allow pasting program code from the IDE.

---

## 9. Multiple Choice Quiz & Real-time Assessment Standards

Every lab worksheet should feature a **5-Question Multiple Choice Quiz** testing theoretical fundamentals, compiler/hardware quirks, and troubleshooting awareness.

---

## 10. Wokwi Simulation & Local VS Code Template Standard (Gold Standard)

To provide 100% reliable simulation without Wokwi web server congestion or internet dropouts:

### 1. Persistent Single-Instance Simulation Engine & True Fullscreen
- Load iframe once with `view=both` to contain diagram, code, and Serial Monitor in a single session.
- **Dual-Mode True Fullscreen (`requestFullscreen()` + CSS Overlay):**
  - Triggers native browser fullscreen covering 100% of the display monitor.
  - Automatically overrides ancestor `.glass-card` styling (`backdrop-filter: none; overflow: visible;`) to prevent viewport clipping.
  - Synchronizes UI state via `fullscreenchange` event and `Escape` key handling without iframe reloads (preserves simulation execution state).

### 2. Local VS Code Template Structure (`vscode-wokwi-template/`)
Provide an offline-ready starter template folder and `.zip` download:
```text
vscode-wokwi-template/
├── .vscode/
│   ├── extensions.json       # Recommendations: PlatformIO & Wokwi for VS Code
│   └── tasks.json            # 1-Click build tasks
├── src/
│   └── main.cpp              # Starter firmware with setRelay() helper & guided TODOs (Never include full solution)
├── diagram.json              # Microcontroller + Sensor + Actuator wiring
├── wokwi.toml                # Firmware ELF mapping: '.pio/build/ipst_wifi/firmware.elf'
├── platformio.ini            # Environment configs & library dependencies
└── README.md                 # Step-by-step local execution guide
```

> [!IMPORTANT]
> **Pedagogical Integrity Rule for Starter Templates:**
> The `main.cpp` inside the offline VS Code template must contain only the starter scaffolding (Pin setup, Serial monitor, `millis()` timer loop, and `setRelay()` helper). The challenge logic (Hysteresis, Fail-safe `isnan()`, thresholds) **must be left as `TODO` exercise comments** to ensure learners actively design and write the algorithms themselves.

### 3. Relay Active-HIGH / Active-LOW Standard Architecture
Firmware should always abstract hardware relay logic to allow seamless switching between simulation (Active-HIGH) and commercial Optocoupler relay boards (Active-LOW):
```cpp
#define RELAY_ACTIVE_LOW false  // Set true when using Active-LOW relay modules

#if RELAY_ACTIVE_LOW
  #define RELAY_ON   LOW
  #define RELAY_OFF  HIGH
#else
  #define RELAY_ON   HIGH
  #define RELAY_OFF  LOW
#endif

void setRelay(uint8_t pin, bool state) {
  digitalWrite(pin, state ? RELAY_ON : RELAY_OFF);
}
```

### 4. Action Toolbar Integration
```html
<div class="wokwi-actions">
  <button type="button" class="wokwi-ctrl-btn vscode" onclick="openInVsCode()"><i class="fa-solid fa-code"></i> เปิดใน VS Code</button>
  <a href="labX_vscode_wokwi_template.zip" download="labX_vscode_wokwi_template.zip" class="wokwi-ctrl-btn zip"><i class="fa-solid fa-file-zipper"></i> โหลด Template (.zip)</a>
  <button type="button" class="wokwi-ctrl-btn info" onclick="openVsCodeGuideModal()"><i class="fa-solid fa-circle-question"></i> วิธีใช้บน VS Code</button>
  <button type="button" class="wokwi-ctrl-btn" id="btnWokwiFullscreen" onclick="toggleWokwiFullscreen()"><i class="fa-solid fa-expand"></i> ขยายเต็มจอ</button>
  <a id="wokwiExternalLink" href="https://wokwi.com/projects/PROJECT_ID" target="_blank" rel="noopener noreferrer" class="wokwi-ctrl-btn external"><i class="fa-solid fa-arrow-up-right-from-square"></i> เปิดใน Wokwi</a>
</div>
```

---

## 11. Real-time Regex Evaluation & 10.0-Point Pre-check Score Engine

All worksheets implement client-side pre-submission grading to give students immediate feedback before final submission.

### Standard 10.0-Point Scoring Rubric

| Component | Assessment Engine | Weight | Success Criteria |
| :--- | :--- | :---: | :--- |
| **1. Code Blanks** | Regex Pattern Matching (`evaluateLabXBlanks`) | **1.5 pts** | Syntax & keyword exact matching |
| **2. Quiz (5 MCQs)** | Multiple-Choice Quiz Engine | **2.0 pts** | 5 questions (0.4 pt each) |
| **3. Challenge Code** | Syntax & Logic Evaluator (`evaluateLabXChallenge`) | **2.5 pts** | Regex logic, fail-safe, and pin definitions |
| **4. Post-Lab Questions** | Content & Analysis (3 questions) | **2.5 pts** | Minimum text length > 10 chars each |
| **5. Attachments** | Multi-layer File Check | **1.0 pt** | Screenshot (0.5 pt) + Code File (0.5 pt) |
| **6. Conclusion** | Analytical Depth Check | **0.5 pt** | Minimum text length **> 100 characters** |
| **Total** | | **10.0 pts** | Score $\ge$ 8.0 = Ready for Submission |

---

## 12. Single-Submission Lock & Shared Station Reset Protocol

1. **Submission Lock (`lockFormAsSubmitted`):**
   - Disables all form inputs and submit button upon successful submission.
   - Displays a glassmorphic confirmation banner (`#submittedNoticeBanner`) in View-Only mode.
   - Saves timestamp in `localStorage` under `_LAB_STORAGE_KEY + studentId`.

2. **New Student Unlock (`unlockFormForNewStudent`):**
   - Provides a prominent **`[ 👤+ เริ่มทำสำหรับ นศ. คนใหม่ ]`** button on both the banner and the bottom button group.
   - Shows a confirmation dialog (`Swal.fire`) warning the previous student to save/print their PDF before reset.
   - Fully clears form fields, cache, Base64 uploads, and file badges, restoring the page to a fresh state for the next learner.

---

## 13. Gold Standard Reference Implementation

- **Reference Model:** `lab1/index.html` (ESP32 Smart Greenhouse IoT Node)
- **Reference Template:** `lab1/vscode-wokwi-template/` and `lab1/lab1_vscode_wokwi_template.zip`
- **Reference Backend:** `lab1/Code.gs`

---

## 14. Interactive Real-Time Waveform & Hysteresis Plotter Standard

An integrated 60 FPS HTML5 Canvas oscilloscope simulating and visualizing dynamic sensor behavior and actuator hysteresis without external charting dependencies.

### Key Capabilities & Visual Tracks
1. **Zero External Dependencies:** Built with pure HTML5 Canvas 2D context, optimized for 60 FPS rendering with zero lag and full offline functionality.
2. **Dual Analog Traces:**
   - **Trace 1 (Cyan `#38bdf8`):** Temperature / Primary analog metric (0 – 50°C).
   - **Trace 2 (Violet `#c084fc`):** Humidity / Secondary analog metric (0 – 100%).
3. **Dynamic Hysteresis Threshold Markers:**
   - Visual dashed reference lines indicating activation thresholds (e.g. Fan ON at $30.0^\circ\text{C}$, Fan OFF at $29.5^\circ\text{C}$).
4. **Digital Actuator State Tracks (Bottom Sub-Tracks):**
   - High/Low logic level step-lines for Relay Actuators (Fan & Mist).
5. **Interactive Controls & 6 Scenario Presets:**
   - Real-time Sliders for manual value sweeps.
   - 6 Quick-Test Presets:
     - ☀️ **ร้อนจัด (>30°C):** Verifies cooling trigger.
     - ❄️ **เย็น (<29.5°C):** Verifies cooling shutdown.
     - 🏜️ **แห้งแล้ง (<50%):** Verifies humidification trigger.
     - 🌧️ **ชื้นสูง (>60%):** Verifies humidification shutdown.
     - ⚠️ **Sensor Error (`NaN`):** Verifies fail-safe shutdown of all relays.
     - 🌿 **ปกติ (Normal):** Restores balanced operating setpoints.

---

## 15. Web Serial API Direct Hardware Telemetry Bridge Standard (Production-Grade)

Allows direct browser-to-ESP32 USB connection via modern Chromium standard (`navigator.serial`), enabling seamless hardware validation and live serial telemetry streaming.

### Key Architectural Standards & Best Practices
1. **DTR / RTS Auto-Release Signal Management:**
   - When connecting on Chromium/Windows, always set `setSignals({ dataTerminalReady: false, requestToSend: false })` immediately after opening the port. This keeps both transistors OFF ($EN=1, IO0=1$), ensuring the ESP32 runs in Normal Flash Execution Mode without getting stuck in Reset or Download Boot.
2. **Direct Reader + Non-Fatal TextDecoder Stream:**
   - Use `serialPort.readable.getReader()` with `new TextDecoder('utf-8', { fatal: false })` instead of `pipeTo()`. This prevents stream pipeline locking and gracefully skips binary noise emitted by the ESP32 bootloader (at 74880 baud).
3. **Hardware Reset Pulse via Web UI (`[ 🔄 รีเซ็ต ESP32 ]`):**
   - Provide a 1-click software trigger that pulses `DTR: false, RTS: true` ($120\text{ms}$ to pull $EN$ LOW) and then releases to `DTR: false, RTS: false` ($100\text{ms}$ to release $EN$ HIGH while $IO0$ remains HIGH). This triggers a clean hardware reboot directly into **Normal Flash Boot Mode** (`SPI_FAST_FLASH_BOOT`) rather than Flashing Download Mode.
4. **On-the-Fly Baud Rate Selector:**
   - Provide dropdown support (`115200`, `9600`, `57600`, `74880`, `19200`, `38400`) so learners can match their firmware configuration dynamically.
5. **Interactive Bidirectional Command TX Bar:**
   - Monospace input bar to transmit commands or test parameters directly to the microcontroller over the open serial stream.
6. **Flexible Multi-Pattern Telemetry Parser:**
   - Tolerates partial and mixed formats:
     - Basic Skeleton: `Temp: 28.5 C | Analog: 45.0%`
     - Grand Challenge: `[STATUS] Temp: 31.2 C | Hum: 55.0% | ADC: 45.0% (Raw: 1843) | Fan: ON | Mist: OFF`
     - Thai & Short tags: `อุณหภูมิ: 30`, `T: 29.5`, `Hum: 60`, `ADC: 2048`, `Fan: 1`, `Mist: 0`
     - Fail-Safe trigger: Automatically identifies `nan` or `[FAIL-SAFE]` lines to engage UI safe-mode chips.


---

## 16. Google Apps Script URL (`?gas=`) Propagation System

ระบบส่ง GAS URL จากครูไปยังนักเรียนผ่าน query parameter `?gas=` ใน URL แทนการพิมพ์ URL ซ้ำๆ

### 16.1 สถาปัตยกรรมกลาง — `config.js`

ทุกใบงานต้องนำเข้า `config.js` ใน `<head>` ก่อน `</head>`:
```html
<script src="../config.js"></script>
```

`config.js` ให้ฟังก์ชันกลาง 3 ตัว:

| ฟังก์ชัน | หน้าที่ |
| :--- | :--- |
| `getLabScriptUrl(labId)` | อ่าน GAS URL จาก `?gas=` param ก่อน → ตรวจ localStorage หลาย key → ตรวจ `LAB_CONFIG` |
| `setLabScriptUrl(labId, url)` | บันทึก GAS URL ไปทุก localStorage key ที่เกี่ยวข้องและอัปเดต `LAB_CONFIG` in-memory |
| `getRelatedGasStorageKeys(labId)` | คืนรายการ localStorage key ทั้งหมดที่ใช้กับ labId นั้น (รองรับ alias เช่น `lab0`, `lab0-basic`, `lab-basic`) |

### 16.2 กลยุทธ์ localStorage Key (Multi-Key Sync)

ระบบ legacy ใช้ key ต่างชุดกัน — `config.js` sync ทุก key พร้อมกันเพื่อไม่ให้ขาด:

| ชุด Key | ตัวอย่าง | ใช้ใน |
| :--- | :--- | :--- |
| `gas_url_{labId}` | `gas_url_lab1` | labs ทั่วไปที่ใช้ `config.js` |
| `teacher_gas_url_{labId}` | `teacher_gas_url_lab2` | `lab2`, `LAB6.1Perform` |
| `iot_{labId}_gas_url` | `iot_lab0_gas_url` | `lab0`, `lab0-basic` (legacy) |

### 16.3 Pattern มาตรฐาน — Labs ทั่วไป (lab1, lab3, lab4 ฯลฯ)

ใช้ฟังก์ชัน `initTeacherGasBox(labId)`, `saveTeacherGasUrl(labId)`, `copyStudentShareLink(labId)` pattern:

```javascript
function initTeacherGasBox(labId) {
  const saved = getLabScriptUrl(labId);
  // ตรวจ ?gas= query param ก่อน
  const urlParams = new URLSearchParams(window.location.search);
  const gasParam = urlParams.get('gas');
  if (gasParam) {
    const decoded = decodeURIComponent(gasParam);
    if (decoded.startsWith('https://')) {
      localStorage.setItem('gas_url_' + labId, decoded);
      setLabScriptUrl(labId, decoded);
      // อัปเดต URL bar โดยไม่ reload
      window.history.replaceState(null, '', '?gas=' + encodeURIComponent(decoded));
    }
  }
  const input = document.getElementById('teacherGasUrlInput');
  if (input && saved) input.value = saved;
  updateTeacherGasBadge(labId);
}

function saveTeacherGasUrl(labId) {
  const input = document.getElementById('teacherGasUrlInput');
  const url = input ? input.value.trim() : '';
  if (url && url.startsWith('https://')) {
    setLabScriptUrl(labId, url);
    localStorage.setItem('gas_url_' + labId, url);
    window.history.replaceState(null, '', '?gas=' + encodeURIComponent(url));
  } else if (!url) {
    localStorage.removeItem('gas_url_' + labId);
    window.history.replaceState(null, '', window.location.pathname);
  }
  updateTeacherGasBadge(labId);
}

function copyStudentShareLink(labId) {
  const input = document.getElementById('teacherGasUrlInput');
  const gasUrl = input ? input.value.trim() : getLabScriptUrl(labId);
  if (!gasUrl || !gasUrl.startsWith('https://')) {
    alert('⚠️ กรุณากรอก URL ของ Google Apps Script ก่อน');
    return;
  }
  // Auto-save ก่อน copy
  setLabScriptUrl(labId, gasUrl);
  localStorage.setItem('gas_url_' + labId, gasUrl);
  window.history.replaceState(null, '', '?gas=' + encodeURIComponent(gasUrl));
  initTeacherGasBox(labId);

  const shareUrl = window.location.origin + window.location.pathname
    + '?gas=' + encodeURIComponent(gasUrl);
  navigator.clipboard.writeText(shareUrl).then(() => {
    alert('✅ คัดลอกลิงก์สำหรับนักเรียนแล้ว!\n' + shareUrl);
  });
}
```

> [!IMPORTANT]
> `copyStudentShareLink()` ต้องเพิ่ม `?gas=...` ต่อท้าย `window.location.pathname` เท่านั้น (ไม่รวม `window.location.href`) เพื่อป้องกัน GAS URL เดิมซ้ำซ้อนใน link ที่ copy

### 16.4 Portal (`index.html`) — `syncPortalLaunchLinksWithGas()`

Portal ต้องอัปเดต link ปุ่ม "เปิดใบงาน" ให้ติด `?gas=` โดยอัตโนมัติ:
```javascript
function syncPortalLaunchLinksWithGas() {
  document.querySelectorAll('.btn-launch[data-lab-id]').forEach(btn => {
    const labId = btn.dataset.labId;
    const gasUrl = getLabScriptUrl(labId);
    const baseHref = btn.dataset.baseHref || btn.getAttribute('href').split('?')[0];
    btn.dataset.baseHref = baseHref;
    if (gasUrl) {
      btn.href = baseHref + '?gas=' + encodeURIComponent(gasUrl);
    } else {
      btn.href = baseHref;
    }
  });
}
// เรียกเมื่อ DOMContentLoaded และหลัง save/reset GAS URL
document.addEventListener('DOMContentLoaded', syncPortalLaunchLinksWithGas);
```

### 16.5 ลำดับการค้นหา GAS URL (Priority)

```
1. ?gas= query parameter ใน URL (URL-decoded)
2. localStorage key ทุก key ที่ getRelatedGasStorageKeys() คืนมา
3. LAB_CONFIG[labId].gasUrl (ค่า default ใน config.js)
```

---

## 17. Challenge Code (`#challengeCode`) — Paste Unlock Button

กล่อง `#challengeCode` ต้องรองรับการวางโค้ดจาก IDE ได้ แม้ระบบ anti-cheat จะบล็อก paste ทั่วไป:

### HTML Pattern
```html
<div style="position: relative; margin-bottom: 0.5rem;">
  <textarea id="challengeCode" name="challengeCode" form="labForm" rows="8"
    placeholder="วางโค้ดของคุณที่นี่..."
    style="width:100%; font-family: monospace; font-size:0.85rem; resize:vertical;"></textarea>
  <button type="button" onclick="pasteToChallenge()"
    style="margin-top:0.4rem; padding:0.35rem 0.9rem; border-radius:6px;
           background:rgba(99,102,241,0.25); border:1px solid #818cf8;
           color:#a5b4fc; cursor:pointer; font-size:0.8rem;">
    <i class="fa-solid fa-paste"></i> วางโค้ดจากคลิปบอร์ด
  </button>
</div>
```

### JavaScript
```javascript
async function pasteToChallenge() {
  try {
    const text = await navigator.clipboard.readText();
    const ta = document.getElementById('challengeCode');
    if (ta) { ta.value = text; ta.dispatchEvent(new Event('input')); }
  } catch(e) {
    alert('⚠️ ไม่สามารถอ่านคลิปบอร์ดได้ กรุณาวาง (Ctrl+V) ในช่องโดยตรง');
  }
}
```

> [!NOTE]
> `#challengeCode` ต้องถูก **whitelist** ออกจาก anti-cheat event listeners (ดู Section 8) เพื่อให้ paste ด้วย `Ctrl+V` ทำงานได้ด้วย

---

## 18. ESP32 & ESP8266 Hardware Pin Rules (Board-Specific Gold Standard)

### 18.1 บอร์ด IPST-WiFi (ESP32)

| GPIO | การใช้งานบนบอร์ด | กฎเหล็ก (Strict Rules) |
| :--- | :--- | :--- |
| **GPIO 18** | **Built-in Onboard LED / Status LED** | ✅ ใช้เป็น Onboard LED เพียงขาเดียว (**ห้ามระบุเป็น GPIO 2** และ **ห้ามใช้ต่อสวิตช์/ปุ่มกด**) |
| **GPIO 0** | **Push Button SW1 (ปุ่ม BOOT/FLASH)** — Active LOW | ✅ ใช้ต่อปุ่มกด `INPUT_PULLUP` |
| **GPIO 33** | **DHT11 / DHT22 Data** | ✅ เซ็นเซอร์วัดอุณหภูมิและความชื้น |
| **GPIO 36 (VP)** | **Potentiometer / KNOB-S (ADC Input Only)** | ✅ อ่านค่า Analog 12-bit (`0–4095`) |
| **GPIO 5** | **Relay 1 (พัดลม)** | ✅ เอาต์พุตขับรีเลย์ตัวที่ 1 |
| **GPIO 23** | **Relay 2 (ปั๊มน้ำ/หัวพ่นหมอก)** | ✅ เอาต์พุตขับรีเลย์ตัวที่ 2 |
| **GPIO 21 / 22** | **I2C SDA (21) / SCL (22)** | ✅ จอแสดงผล OLED SSD1306 (`0x3C`) |

### 18.2 บอร์ด NodeMCU 1.0 / AX-WiFi (ESP8266)

| พิน / GPIO | การใช้งานบนบอร์ด | กฎเหล็ก (Strict Rules) |
| :--- | :--- | :--- |
| **GPIO 0 (`D3`)** | **Push Button (ปุ่ม FLASH บนบอร์ด)** — Active LOW | ❌ **ห้ามนำมาต่อเซ็นเซอร์ DHT เด็ดขาด** เพราะต่ออยู่กับสวิตช์ปุ่มกด FLASH บนบอร์ด |
| **GPIO 2 (`D4`)** | **DHT11 / DHT22 Data** (และ Onboard LED) | ✅ ใช้ต่อขาสัญญาณ DATA ของเซ็นเซอร์ DHT11/DHT22 (`#define DHTPIN 2`) |
| **ADC0 (`A0`)** | **Potentiometer / VR** | ✅ อ่านค่า Analog 10-bit (`0–1023`) |
| **GPIO 13 (`D7`)** | **Relay 1 (พัดลม)** | ✅ เอาต์พุตขับรีเลย์ตัวที่ 1 |
| **GPIO 16 (`D0`)** | **Relay 2 (ปั๊มน้ำ/หัวพ่นหมอก)** | ✅ เอาต์พุตขับรีเลย์ตัวที่ 2 |
| **GPIO 4 (`D2`) / GPIO 5 (`D1`)** | **I2C SDA (`D2`) / SCL (`D1`)** | ✅ จอแสดงผล OLED SSD1306 (`0x3C`) |

> [!CAUTION]
> 1. บนบอร์ด **ESP32 (IPST-WiFi)**: On-board LED คือ **`GPIO 18` เท่านั้น** (ไม่ใช่ `GPIO 2`)
> 2. บนบอร์ด **ESP8266 (NodeMCU 1.0 / AX-WiFi)**: ขา `GPIO 0 (D3)` ต่อกับสวิตช์ปุ่มกด FLASH บนบอร์ดแล้ว จึง**ต้องใช้ขา `GPIO 2 (D4)` สำหรับต่อเซ็นเซอร์ DHT11/DHT22 เสมอ**

### ตัวอย่าง Arduino Code มาตรฐานรองรับทั้ง ESP32 และ ESP8266
```cpp
#if defined(ESP8266)
  #define DHTPIN          2     // GPIO 2 / D4 สำหรับ ESP8266 (NodeMCU / AX-WiFi)
  #define BUTTON_PIN      0     // GPIO 0 / D3 (ปุ่ม FLASH บนบอร์ด)
  #define ALERT_LED_PIN   2     // Onboard LED (GPIO 2 / D4)
#elif defined(ESP32)
  #define DHTPIN          33    // GPIO 33 สำหรับ ESP32 (IPST-WiFi)
  #define BUTTON_PIN      0     // GPIO 0 (ปุ่ม SW1 บนบอร์ด)
  #define ALERT_LED_PIN   18    // Onboard LED (GPIO 18 สำหรับ IPST-WiFi)
#endif
```

---

## 19. Universal Master Backend (`Master_Code.gs`), Single-Sheet Multi-Tab & Roster Lookup Architecture

เพื่อลดภาระการจัดการ Google Sheets 19 ไฟล์แยกกัน ระบบใช้ **Universal Master Backend (`Master_Code.gs`)** ที่รวบรวมการส่งคะแนนจากทุกใบงานลงใน Google Sheet เดียวกัน พร้อมระบบ **Master Roster Protection & Student ID Score Lookup**

```mermaid
flowchart TD
    subgraph Worksheets ["19 IoT Lab Worksheets"]
        W0["Lab 0 & Lab 0 Basic"]
        W1["Lab 1, 1.1 & Lab 2 (Gold Standard)"]
        W3["Lab 3, 3.1, 3.2 & WebConfig"]
        W6["Lab 4, 4.1, 5, 5 Dev, 6, 6.1"]
        W9["Lab 7, 8, 9 & Lab Extra"]
    end

    Router["Universal Master API Router (Master_Code.gs)<br>doPost() / resolveLabKey() / syncAllScoresToMasterRoster()"]

    subgraph MasterSheet ["Single Master Google Sheet (Master_IoT_LAB)"]
        Dash["📊 สรุปคะแนนรวม (Master Roster)<br>Col A: รหัส นศ. (PK) | Col B: ชื่อ (Locked) | Col C: ห้อง (Plain Text @)"]
        T0["Lab 0 Basic Submissions"]
        T1["Lab 1 Submissions"]
        T11["Lab 1.1 Submissions"]
        T2["Lab 2 Submissions"]
        TAll["... แท็บแยกตามแล็บ (19 แท็บ)"]
    end

    Worksheets -->|payload.labId + auto-grading| Router
    Router -->|สร้างแท็บ & บันทึกคะแนน| T0
    Router -->|สร้างแท็บ & บันทึกคะแนน| T1
    Router -->|สร้างแท็บ & บันทึกคะแนน| T11
    Router -->|สร้างแท็บ & บันทึกคะแนน| T2
    Router -->|สร้างแท็บ & บันทึกคะแนน| TAll
    T0 & T1 & T11 & T2 & TAll -->|ดึงคะแนนตามรหัสนักศึกษา (Col A)| Dash
```

### คุณสมบัติหลักของ Master_Code.gs (Gold Standard)
1. **Master Lab Registry (`LAB_REGISTRY`):**
   - รองรับคีย์แล็บมาตรฐานครบทั้ง 19 แล็บ: `lab0`, `lab0-basic`, `lab1`, `lab1.1`, `lab2`, `lab3`, `lab3.1`, `lab3.2_Wifi_UI_config`, `lab4`, `lab4.1`, `lab5`, `LAB5_Dev`, `lab6`, `LAB6.1Perform`, `lab7`, `lab8`, `lab9`, `lab-extra`, `lab-webconfig_wifi`
2. **Master Roster Protection & Student ID Lookup (`syncAllScoresToMasterRoster`):**
   - ครูสามารถวางใบรายชื่อ (`รหัสนักศึกษา | ชื่อ-นามสกุล | กลุ่ม/ห้อง`) ลงในแท็บแรก **`📊 สรุปคะแนนรวม`** ล่วงหน้าได้
   - ยึด **`รหัสนักศึกษา` (คอลัมน์ A)** เป็น Primary Key (`normalizeStudentId`)
   - **ห้ามเขียนทับ `ชื่อ-นามสกุล` (Col B) และ `กลุ่ม/ห้อง` (Col C)** ที่ครูวางไว้ล่วงหน้า (จะเติมให้เฉพาะกรณีที่ช่องนั้นว่างอยู่เท่านั้น)
3. **ป้องกัน Google Sheets แปลงกลุ่ม/ห้อง (`4/5`, `4/6`) เป็นวันที่ (`sanitizeGroupValue`):**
   - บังคับตั้งค่าคอลัมน์ A และคอลัมน์ C เป็นข้อความธรรมดา (`setNumberFormat("@")`) เสมอ
   - มีฟังก์ชัน `sanitizeGroupValue()` แปลงค่าวันที่ (`Mon May 04 2026...GMT+0700`) กลับเป็น `"4/5"` หรือ `"4/6"` โดยอัตโนมัติ
4. **3 ช่องทางในการซิงค์คะแนนเข้าใบรายชื่อ:**
   - **อัตโนมัติเมื่อนักศึกษาส่งใบงาน (`updateMasterSummaryDashboard`):** อัปเดตคะแนนและซิงค์ทุกแล็บเข้าใบรายชื่อทันที
   - **ปุ่มบนหน้าเว็บพอร์ทัล (`index.html` & `dashboard.html`):** ปุ่ม `[ 🎓 ดึงคะแนนตามรหัส นศ. ]` (`syncMasterRosterScores()`) ส่ง `action: "syncRoster"`
   - **เมนูบนแถบเครื่องมือ Google Sheets (`onOpen` -> `🎓 ระบบจัดการคะแนน IoT`):**
     - `🔄 ดึงคะแนนทุกแล็บตามรหัสนักศึกษา (Sync Scores to Roster)`
     - `📐 ผูกสูตรดึงคะแนนอัตโนมัติ (Insert VLOOKUP Formulas)`
     - `🧹 ล้างแถวทดสอบที่ไม่ใช่รหัสนักศึกษาจริง (Clean Test Rows)`
5. **ลำดับความสำคัญของ Master URL ใน `config.js`:**
   - ใน `getLabScriptUrl(labId)` กำหนดให้ **Master GAS URL มีลำดับความสำคัญสูงกว่าแคชเก่าใน localStorage** เว้นแต่แล็บนั้นถูกตั้ง `override_<labId> === 'true'`
   - มีปุ่ม `[ 🔄 บังคับใช้ Master กับทุกแล็บ ]` (`applyMasterToAllLabs()`) เพื่อล้างลิงก์ชีตเก่าในเบราว์เซอร์ทั้งหมด

---

## 20. Submission Lock Layout & Teacher Password Protection

เมื่อนักศึกษาส่งใบงานแล้ว หน้าเว็บจะเข้าสู่โหมดดูอย่างเดียว (Read-Only Lock) โดยมีมาตรฐานการจัดวางปุ่มและการป้องกันดังนี้:

1. **กล่องแจ้งเตือนสีเขียวด้านบน (`#submittedNoticeBanner`):**
   - แสดงข้อความแจ้งเวลาที่ส่งใบงาน และแสดง **เฉพาะปุ่ม `[ 🔓 ครูผู้สอน: ปลดล็อกส่งใหม่ ]`** เท่านั้น
2. **ตำแหน่งปุ่มเริ่มทำสำหรับนักศึกษาคนใหม่ (`#newStudentTopBar` & `#newStudentBtn`):**
   - **ห้ามวางปุ่มเริ่มทำใหม่ไว้ข้างปุ่มครูในกล่องสีเขียวด้านบน**
   - ให้สร้าง `#newStudentTopBar` วางไว้ **ใต้กล่อง `.gas-connect-box` (ก่อนหัวข้อ ข้อมูลผู้ส่งรายงาน)** เป็นปุ่มสีน้ำเงินเต็มความกว้าง:
     `[ 👤+ เริ่มทำสำหรับ นศ. คนใหม่ (ล้างฟอร์มเริ่มใหม่) ]`
   - และคงปุ่ม `#newStudentBtn` ไว้ที่ท้ายฟอร์มด้านล่างสุด (ใต้ปุ่มส่งใบงาน / พิมพ์ PDF) ด้วย
3. **ระบบรหัสผ่านครูผู้สอน (`verifyTeacherUnlockPassword` ใน `config.js` และ `teacherClearLabCache`):**
   - เมื่อกดปุ่ม `[ 🔓 ครูผู้สอน: ปลดล็อกส่งใหม่ ]` หรือ `[ 🔓 เคลียร์แคชส่งงาน ]` **ต้องแสดงหน้าต่าง `🔒 ยืนยันสิทธิ์ครูผู้สอน` (`input: 'password'`) ให้กรอกรหัสผ่านก่อนเสมอ** เพื่อป้องกันนักศึกษากดปลดล็อกเอง
   - รหัสผ่านเริ่มต้น: `"1234"` หรือ `"teacher"` (หรือ `"iot512"` / ตั้งค่าใน `TEACHER_UNLOCK_PASSWORD` ใน `config.js`)
   - เมื่อปลดล็อก (`clearStudentSubmissionFromAllSheets`): ลบเฉพาะแถวในแท็บใบงานย่อย และเปลี่ยนช่องคะแนนของแล็บนั้นในหน้า `📊 สรุปคะแนนรวม` เป็น `"-"` **โดยห้ามลบแถวรายชื่อของนักศึกษาออกจากหน้าสรุปคะแนนรวมเด็ดขาด**

---

## 21. VS Code + Wokwi Direct Project Launch Standard (`openProjectInVsCode`)

ทุกใบงานที่มีส่วนจำลองวงจร Wokwi (`lab0`, `lab0-basic`, `lab1`, `lab1.1`, `lab2`, `lab5`, `LAB5_Dev`, `lab6`, `LAB6.1Perform`) ใช้มาตรฐานเดียวกับโปรเจกต์ `MCU_WebApps` โดยยึด **`lab2/`** เป็นต้นแบบ (Gold Standard Template):

1. **ปุ่ม `</> VS Code Wokwi` (`openProjectInVsCode`):**
   - เมื่อคลิกปุ่ม `<button type="button" class="wokwi-ctrl-btn vscode" onclick="openProjectInVsCode('lab2')">` จะทำงาน 2 ช่องทางพร้อมกันโดยไม่มี Modal กั้น:
     ```javascript
     function openProjectInVsCode(labFolder) {
       labFolder = labFolder || 'lab2';
       const subDir = (labFolder === 'lab5' || labFolder === 'LAB5_Dev' || labFolder === 'lab6' || labFolder === 'LAB6.1Perform')
         ? 'solution'
         : 'vscode-wokwi-template';
       const workspacePath = `/Users/allarmmac/myjob_folder/MyLaB/IoT/${labFolder}/${subDir}`;
       window.location.href = `vscode://file${workspacePath}`;
       fetch('/api/open-vscode', {
         method: 'POST',
         headers: { 'Content-Type': 'application/json' },
         body: JSON.stringify({ lab: labFolder })
       }).catch(() => {});
       if (window.location.hostname !== 'localhost' && window.location.hostname !== '127.0.0.1') {
         fetch('http://localhost:4000/api/open-vscode', {
           method: 'POST',
           headers: { 'Content-Type': 'application/json' },
           body: JSON.stringify({ lab: labFolder })
         }).catch(() => {});
       }
     }
     ```
2. **Backend API (`/api/open-vscode` ใน `server.js`):**
   - รันคำสั่งเปิดโฟลเดอร์โปรเจกต์พร้อมเปิดไฟล์ `src/main.cpp` และ `diagram.json` ขึ้นมาใน VS Code ทันที
3. **โครงสร้างโฟลเดอร์ `vscode-wokwi-template/` ต้นแบบ (`lab2/vscode-wokwi-template/`):**
   - `src/main.cpp`: ซอร์สโค้ด C++ ฉบับสมบูรณ์พร้อมคอมเมนต์ภาษาไทย
   - `diagram.json`: ผังวงจร Wokwi (ใช้ `board-ssd1306` สำหรับจอ OLED และมี `wokwi-logic-analyzer` สำหรับส่งออกไฟล์ `.vcd`)
   - `platformio.ini`: ตั้งค่า `[env:esp32dev]` และ `lib_deps` ครบถ้วน
   - `wokwi.toml`:
     ```toml
     [wokwi]
     version = 1
     firmware = '.pio/build/esp32dev/firmware.bin'
     elf = '.pio/build/esp32dev/firmware.elf'
     ```
   - `.vscode/extensions.json`: แนะนำ `"platformio.platformio-ide"` และ `"wokwi.wokwi-vscode"`
   - `.pio/build/esp32dev/firmware.bin` & `firmware.elf`: ใส่ไฟล์ Firmware ที่คอมไพล์แล้วไว้ล่วงหน้าเพื่อให้กดรันจำลอง (Play ▶) ใน VS Code ได้ทันที


