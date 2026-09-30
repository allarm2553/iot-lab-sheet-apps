---
name: esp32-iot-dual-protocol
description: >-
  Best practices and architectural blueprints for developing dual-protocol (WebSockets + MQTT)
  IoT controllers on ESP32/ESP8266 with PWA web dashboards, dynamic lean payloads, and race-condition-free state machines.
---

# ESP32 Dual-Protocol IoT Controller Skill

This skill provides architectural guidelines, code patterns, and troubleshooting procedures for developing robust, high-performance IoT controllers running on ESP32/ESP8266 with dual-protocol communication (Local WebSockets + Cloud MQTT) and responsive Web/PWA dashboards.

---

## 1. Dual-Protocol Architecture

```
                 +---------------------------+
                 |    Web Dashboard / PWA    |
                 +-------------+-------------+
                               |
               +---------------+---------------+
               | (LAN / 0ms)                   | (WAN / Cloud)
               v                               v
    +--------------------+           +--------------------+
    | WebSockets Port 81 |           |   MQTT Broker      |
    +----------+---------+           |  (EMQX / HiveMQ)   |
               |                     +----------+---------+
               |                                |
               +---------------+----------------+
                               v
                     +-------------------+
                     |  ESP32 Controller |
                     +-------------------+
```

### Communication Strategy
- **Local WebSockets (`ws://<mcu-ip>:81`)**: Instantaneous (0ms) low-latency control when connected to the local Wi-Fi network.
- **Cloud MQTT (`wss://broker:8884/mqtt` / `mqtt://broker:1883`)**: Remote control and telemetry publishing over the internet.
- **Hybrid Routing**: Dashboard uses WebSockets when available on local LAN and automatically fails over to MQTT when remote.

---

## 2. Dynamic Lean Payload Design

To prevent packet fragmentation, reduce memory usage, and eliminate latency on resource-constrained MCUs:

1. **Omit Disabled Modules**: Do not serialize keys for disabled sensors (e.g. if DHT is disabled, omit `temperature` and `humidity` entirely from JSON).
2. **Precision Rounding**: Use `serialized(String(value, 1))` to keep float payload size minimal.
3. **Adaptive Dashboard Rendering**: Dashboard inspects `enable<Module>` boolean flags from the MCU state to dynamically hide or show cards.

---

## 3. State Management & Race Condition Prevention

### The Double-Command Hazard
**Anti-Pattern:**
```javascript
// ❌ BAD: Sending two commands simultaneously causes race conditions
function sendFanCommand(state) {
  sendModeCommand(false); // Dispatches set_mode -> MCU broadcasts old fanState -> reverts UI!
  sendCommand({ command: 'set_fan', state: state });
}
```

**Correct Pattern (Atomic Dispatch):**
```javascript
// ✅ GOOD: Send a single atomic command & handle state transition on MCU
function sendFanCommand(state) {
  // 1. Optimistic UI update for instant feedback
  document.getElementById('switchFan').checked = state;
  updateLocalModeIndicators(false);

  // 2. Dispatch single atomic command
  sendCommand({ command: 'set_fan', state: Boolean(state) });
}
```

### Robust JSON Type Handling in Firmware
Always handle diverse type representations (`bool`, `int`, `String`) in `ArduinoJson`:
```cpp
if (doc.containsKey("state")) {
  if (doc["state"].is<bool>()) {
    fanState = doc["state"].as<bool>();
  } else if (doc["state"].is<int>()) {
    fanState = (doc["state"].as<int>() == 1);
  } else if (doc["state"].is<String>()) {
    String s = doc["state"].as<String>();
    s.toLowerCase();
    fanState = (s == "true" || s == "1" || s == "on");
  }
}
// Automatically switch out of Auto Mode into Manual Mode on manual actuation
autoMode = false;
stateChanged = true;
```

---

## 4. Hardware Safety & Strapping Pin Protection

1. **Graceful Teardown Before Reset / Reboot**:
   - Turn OFF all relays (`digitalWrite(pin, LOW)`) to prevent bootstrapping voltage level conflicts.
   - Close WebSockets, disconnect MQTT, stop WebServer, unmount `LittleFS`, and call `WiFi.disconnect(true)`.
   - Delay 200ms before calling `ESP.restart()`.
2. **GPIO 0 (BOOT Button) Factory Reset**:
   - Detect 3-second continuous hold with active visual countdown on OLED display (`HOLD RESET: 3s -> 2s -> 1s`).
   - Remove `/config.json` and cleanly reboot into AP Captive Portal mode (`192.168.4.1`).

---

## 5. Network Latency & Stability Optimizations

- **Disable Wi-Fi Modem Sleep**: `WiFi.setSleep(false);` (eliminates 100-300ms random packet delays).
- **Non-blocking MQTT Timeout**: `mqttClient.setSocketTimeout(2);` (prevents 15-second loop freezes when broker is offline).
- **Fast I2C Bus Clock**: `Wire.setClock(400000);` (prevents OLED rendering from stalling main loop).
- **Appropriate Buffer Sizes**: `mqttClient.setBufferSize(2048);` to handle JSON config schemas.

---

## 6. PWA & WebIntoApp Offline Packaging

- Bundle all client assets locally in `data/` (`mqtt.min.js`, `sw.js`, `manifest.json`, `icon-192.png`, `icon-512.png`).
- Avoid CDN dependencies inside the LittleFS root to ensure the device remains fully manageable in isolated AP Captive Portal mode.
- Use `zip -r -FS ../webintoapp_package.zip .` from inside `data/` to keep release packages in sync.

---

## 7. Decoupled Event-Driven Actuator Architecture (Zero-Bounce Pattern)

### The Stale Telemetry Hazard (Switch Bouncing Bug)
When periodic environmental telemetry (published every 2–3s due to slow sensor physical constraints such as DHT11/DHT22) includes actuator states (`fanState`, `mistState`), high network latency (e.g. congested MQTT broker with 3–8s round-trip time) creates a severe race condition:
1. User toggles a switch (e.g. Turn Fan OFF). The UI optimistically updates.
2. An in-flight periodic sensor packet (generated before the command arrived, containing old `fanState: true`) reaches the browser.
3. The browser's general telemetry handler receives the packet and forces `switchFan.checked = data.fanState`, snapping the switch back to ON.
4. Seconds later, the MCU finally processes the command and broadcasts the new state, causing the switch to flip again.

### Architectural Solution: Total Decoupling of Sensors & Actuators

#### 1. Firmware (ESP32 / ESP8266)
- **Periodic Sensor Telemetry (`broadcastAndPublishState()`)**:
  - MUST ONLY contain environmental telemetry: Temperature, Humidity, Soil, Gas/Smoke, BME, Thresholds.
  - **NEVER** serialize or send `fanState` or `mistState` in the periodic sensor telemetry loop.
- **Instant Fast-Lane Actuator Publisher (`publishActuatorState()`)**:
  - Dispatched strictly upon state change events: network commands (`set_fan`, `set_mist`, `set_mode`), physical hardware button debounce (`fanButtonPin`, `mistButtonPin`), or automatic threshold trips.
  - Lean payload (<150 bytes): `{"event":"actuator_change", "reason":"...", "fanState":..., "mistState":..., "autoMode":...}`.
  - Executed and published instantaneously (<5ms) across both Local WebSockets and Cloud MQTT.

```cpp
void publishActuatorState(const char* reason) {
  JsonDocument doc;
  doc["event"] = "actuator_change";
  doc["reason"] = reason;
  doc["mac"] = cleanMac;
  doc["autoMode"] = autoMode;
  doc["fanState"] = fanState;
  doc["mistState"] = mistState;
  doc["timestamp"] = millis();

  String jsonString;
  serializeJson(doc, jsonString);
  webSocket.broadcastTXT(jsonString);
  if (mqttClient.connected()) {
    mqttClient.publish(pubTopic.c_str(), jsonString.c_str());
  }
}
```

#### 2. Client Dashboard (PWA / HTML)
- **Separate Event Paths in `updateUI(data)`**:
  - If `data.event === 'actuator_change'`: Update switch elements (`switchFan.checked`, `switchMist.checked`) immediately.
  - Regular telemetry path: Update gauge cards and charts only. **NEVER** touch switch checkboxes from the regular telemetry path.
- **Initial Sync on Connection**:
  - On `ws.onopen` or `mqtt.connect`, send `{"command": "get_state"}`.
  - MCU responds with `publishActuatorState("sync")` and `broadcastAndPublishState()` so initial state is loaded accurately.

```javascript
function updateUI(data, source) {
  if (!data) return;

  // 1. Fast-Path: Actuator/Switch State Event (<1ms DOM update)
  if (data.event === 'actuator_change') {
    if (data.fanState !== undefined) {
      document.getElementById('switchFan').checked = data.fanState;
      document.getElementById('txtFanStatus').innerText = data.fanState ? 'เปิด (ON)' : 'ปิด (OFF)';
    }
    if (data.mistState !== undefined) {
      document.getElementById('switchMist').checked = data.mistState;
      document.getElementById('txtMistStatus').innerText = data.mistState ? 'เปิด (ON)' : 'ปิด (OFF)';
    }
    return;
  }

  // 2. Telemetry Path: Environmental sensor cards ONLY (never touches switches)
  if (data.temperature !== undefined) ...
}
```

**Result:**
- 0ms instant UI tactile feedback for the user.
- Absolute immunity to MQTT internet latency, packet jitter, and broker delays.
- Zero switch bouncing under all operating conditions.

