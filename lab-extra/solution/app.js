// JavaScript for Climate Dashboard - Lab Extra
// Features:
//  - Interactive simulation sliders.
//  - Challenge 1: Adding/Removing .warning-blink CSS class when temp > 35.0 C.
//  - Challenge 2: Translating page texts between Thai (TH) and English (EN) dynamically.

let currentLang = 'TH'; // Initial language state

// Translation Dictionary
const translations = {
    TH: {
        title: "ระบบควบคุมภูมิอากาศ IoT",
        langBtn: "EN",
        tempTitle: "อุณหภูมิห้อง",
        tempFooter: "อัปเดตแบบเรียลไทม์",
        humTitle: "ความชื้นอากาศ",
        humFooter: "ความชื้นสัมพัทธ์อากาศ",
        soilTitle: "เซ็นเซอร์วัดค่าดิน",
        soilFooter: "ระดับความชื้นสะสมในดิน",
        controlTitle: "ควบคุมอุปกรณ์เอาต์พุต",
        fanLabel: "เปิดพัดลมระบายอากาศ (GPIO 13)",
        fanDesc: "สั่งทำงานตัวพัดลมระบายความร้อนตัวหลัก",
        statusConn: "เชื่อมต่อสำเร็จ",
        statusDisc: "ขาดการเชื่อมต่อ"
    },
    EN: {
        title: "IoT Climate Dashboard",
        langBtn: "TH",
        tempTitle: "Room Temperature",
        tempFooter: "Updated in real-time",
        humTitle: "Air Humidity",
        humFooter: "Relative air humidity",
        soilTitle: "Soil Moisture",
        soilFooter: "Accumulated soil moisture level",
        controlTitle: "Output Actuator Controls",
        fanLabel: "Turn On Fan (GPIO 13)",
        fanDesc: "Manually trigger the primary cooling fan",
        statusConn: "Connected",
        statusDisc: "Disconnected"
    }
};

// Toggle Language Handler (Challenge 2)
function toggleLanguage() {
    currentLang = currentLang === 'TH' ? 'EN' : 'TH';
    
    // Get translations for current active language selection
    const langData = translations[currentLang];
    
    // Update texts
    document.getElementById('txt-title').textContent = langData.title;
    document.getElementById('txt-lang-btn').textContent = langData.langBtn;
    document.getElementById('txt-temp-title').textContent = langData.tempTitle;
    document.getElementById('txt-temp-footer').textContent = langData.tempFooter;
    document.getElementById('txt-hum-title').textContent = langData.humTitle;
    document.getElementById('txt-hum-footer').textContent = langData.humFooter;
    document.getElementById('txt-soil-title').textContent = langData.soilTitle;
    document.getElementById('txt-soil-footer').textContent = langData.soilFooter;
    document.getElementById('txt-control-title').textContent = langData.controlTitle;
    document.getElementById('txt-fan-label').textContent = langData.fanLabel;
    document.getElementById('txt-fan-desc').textContent = langData.fanDesc;
    
    // Update connection label based on connection class state
    const isConn = document.getElementById('conn-dot').classList.contains('disconnected') === false;
    document.getElementById('conn-text').textContent = isConn ? langData.statusConn : langData.statusDisc;
    
    console.log(`Language swapped to: ${currentLang}`);
}

// Temperature Simulation Handler (Challenge 1)
function simulateTemp(val) {
    const tempNum = parseFloat(val);
    document.getElementById('sim-temp-val').textContent = tempNum.toFixed(1);
    document.getElementById('temp-val').textContent = tempNum.toFixed(1);
    
    const tempCard = document.getElementById('temp-card');
    
    // Hysteresis/Warning condition check
    if (tempNum > 35.0) {
        tempCard.classList.add('warning-blink');
        console.warn("WARNING: Temperature is critically high (> 35.0 C)!");
    } else {
        tempCard.classList.remove('warning-blink');
    }
}

// Soil Moisture Simulation Handler
function simulateSoil(val) {
    const soilPercent = parseInt(val);
    document.getElementById('sim-soil-val').textContent = soilPercent;
    document.getElementById('analog-val').textContent = soilPercent;
    
    const progressBar = document.getElementById('analog-bar');
    progressBar.style.width = `${soilPercent}%`;
}

// Fan Manual Control Handler
function onToggleFan(state) {
    console.log(`Manual Fan override triggered: ${state ? 'ON' : 'OFF'}`);
    const isConn = document.getElementById('conn-dot').classList.contains('disconnected') === false;
    if (isConn) {
        // mock sending data or state
    }
}

// Page initialization
window.addEventListener('DOMContentLoaded', () => {
    // Setup initial mock metrics
    simulateTemp("28.5");
    simulateSoil("45");
    document.getElementById('hum-val').textContent = "60.0";
    
    // Set simulator default connection state
    const isConnected = true; 
    const connDot = document.getElementById('conn-dot');
    const connText = document.getElementById('conn-text');
    
    if (isConnected) {
        connDot.classList.remove('disconnected');
        connText.textContent = translations[currentLang].statusConn;
    } else {
        connDot.classList.add('disconnected');
        connText.textContent = translations[currentLang].statusDisc;
    }
});
