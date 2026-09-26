---
name: IoT Lab Worksheet Development
description: Standard guidelines, templates, and best practices for creating and maintaining interactive IoT lab worksheet web applications (ESP32/ESP8266, Google Apps Script, Glassmorphism UI, Theory Modals, Wiring Tables, and Online Status Indicators).
---

# IoT Lab Worksheet Development Guidelines

This skill provides comprehensive standards, UI architecture, hardware pin mappings, and code templates for building and maintaining interactive web-based IoT laboratory worksheets (Google Apps Script frontends & standalone HTML forms).

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
| **เซ็นเซอร์ DHT11** | DATA | `GPIO 33` | `GPIO 0 / D3` | อุณหภูมิและความชื้น (ไฟเลี้ยง 3.3V) |
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


