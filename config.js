/**
 * IoT Lab Worksheets — Global Configuration
 * ตั้งค่าลิงก์ Google Apps Script Web App URL แยกทีละใบงาน หรือใช้ Master URL รวมศูนย์ เพื่อส่งคะแนนเข้า Google Sheets ของอาจารย์ผู้สอน
 * 
 * 💡 วิธีใช้งานแบบใหม่ (แนะนำ):
 * สร้าง Google Apps Script ตัวเดียวจากไฟล์ Master_Code.gs แล้วนำ Web App URL มาใส่ใน MASTER_GAS_URL ด้านล่างนี้
 * ทุกใบงานปฏิบัติการ (ทั้ง 19 แล็บ) จะเชื่อมต่อเข้าสู่ Google Sheet เดียวกัน และแยกแท็บให้อัตโนมัติทันที!
 */

// 🌟 ลิงก์รวมศูนย์ส่งคะแนนทุกแล็บลง Google Sheet เดียวกัน (Master Universal Backend)
const MASTER_GAS_URL = "";

const LAB_CONFIG = {
  // หมวดที่ 1: พื้นฐานฮาร์ดแวร์และการเชื่อมต่ออินพุต/เอาต์พุต (Module 1)
  "lab0": "YOUR_GAS_URL_FOR_LAB_BASIC",
  "lab0-basic": "YOUR_GAS_URL_FOR_LAB_BASIC",
  "lab-basic": "YOUR_GAS_URL_FOR_LAB_BASIC",
  "lab1": "YOUR_GAS_URL_FOR_LAB1",
  "lab1.1": "YOUR_GAS_URL_FOR_LAB1_1",
  "lab2": "YOUR_GAS_URL_FOR_LAB2",

  // หมวดที่ 2: การเชื่อมต่อไร้สายและเว็บเซิร์ฟเวอร์ฝังตัว (Module 2)
  "lab3": "YOUR_GAS_URL_FOR_LAB3",
  "lab3.1": "YOUR_GAS_URL_FOR_LAB3_1",
  "lab3.2_Wifi_UI_config": "YOUR_GAS_URL_FOR_LAB3_2",
  "lab4": "YOUR_GAS_URL_FOR_LAB4",
  "lab4.1": "YOUR_GAS_URL_FOR_LAB4_1",

  // หมวดที่ 3: สื่อสารระดับคลาวด์และโครงงานบูรณาการ (Module 3)
  "lab5": "YOUR_GAS_URL_FOR_LAB5",
  "LAB5_Dev": "YOUR_GAS_URL_FOR_LAB5_DEV",
  "lab6": "YOUR_GAS_URL_FOR_LAB6",
  "LAB6.1Perform": "YOUR_GAS_URL_FOR_LAB6_1_PERFORM",

  // หมวดที่ 4: การประมวลผลและการวิเคราะห์ข้อมูลขนาดใหญ่ (Module 4)
  "lab7": "YOUR_GAS_URL_FOR_LAB7",
  "lab8": "YOUR_GAS_URL_FOR_LAB8",
  "lab9": "YOUR_GAS_URL_FOR_LAB9",

  // หมวดที่ 5: ใบงานเสริมและการแปลงแอปพลิเคชัน (Module 5)
  "lab-extra": "YOUR_GAS_URL_FOR_LAB_EXTRA",
  "lab-webconfig_wifi": "YOUR_GAS_URL_FOR_LAB_WEBCONFIG_WIFI"
};

/**
 * Helper คืนค่ารายการคีย์ localStorage ที่เกี่ยวข้องกับ labId นั้นๆ
 * (รองรับการซิงค์ระหว่าง lab0, lab0-basic และ lab-basic)
 */
function getRelatedGasStorageKeys(labId) {
  const isLab0 = (labId === 'lab0' || labId === 'lab0-basic' || labId === 'lab-basic');
  if (isLab0) {
    return [
      'gas_url_lab0',
      'gas_url_lab0-basic',
      'gas_url_lab-basic',
      'iot_lab0_gas_url',
      'teacher_gas_url_lab0',
      'teacher_gas_url_lab0-basic',
      'teacher_gas_url_lab-basic'
    ];
  }
  return [
    'gas_url_' + labId,
    'teacher_gas_url_' + labId
  ];
}

/**
 * บันทึก Master GAS URL ลงใน localStorage
 */
function setMasterScriptUrl(url) {
  if (typeof window === 'undefined' || !window.localStorage) return;
  const cleanUrl = (url || '').trim();
  try {
    if (cleanUrl && cleanUrl.startsWith('https://script.google.com')) {
      window.localStorage.setItem('master_gas_url', cleanUrl);
      window.localStorage.setItem('iot_master_gas_url', cleanUrl);
    } else if (!cleanUrl) {
      window.localStorage.removeItem('master_gas_url');
      window.localStorage.removeItem('iot_master_gas_url');
    }
  } catch (e) {}
}

/**
 * ดึง Master GAS URL ที่ตั้งไว้
 */
function getMasterScriptUrl() {
  if (typeof window !== 'undefined') {
    if (window.location && window.location.search) {
      const params = new URLSearchParams(window.location.search);
      let queryGas = params.get('master_gas') || params.get('master_gas_url');
      if (queryGas && queryGas.trim().startsWith('https://script.google.com')) {
        return queryGas.trim();
      }
    }
    if (window.localStorage) {
      const custom = window.localStorage.getItem('master_gas_url') || window.localStorage.getItem('iot_master_gas_url');
      if (custom && custom.trim().startsWith('https://script.google.com')) {
        return custom.trim();
      }
    }
  }
  if (typeof MASTER_GAS_URL !== 'undefined' && MASTER_GAS_URL && MASTER_GAS_URL.trim().startsWith('https://script.google.com')) {
    return MASTER_GAS_URL.trim();
  }
  return '';
}

/**
 * บันทึก GAS URL ลงใน localStorage ทุกคีย์ที่เกี่ยวข้องกับ labId
 */
function setLabScriptUrl(labId, url, isOverride = false) {
  if (typeof window === 'undefined' || !window.localStorage || !labId) return;
  const cleanUrl = (url || '').trim();
  const keys = getRelatedGasStorageKeys(labId);
  try {
    keys.forEach(k => {
      if (cleanUrl && cleanUrl.startsWith('https://script.google.com')) {
        window.localStorage.setItem(k, cleanUrl);
      } else if (!cleanUrl) {
        window.localStorage.removeItem(k);
      }
    });
    if (isOverride && cleanUrl) {
      window.localStorage.setItem('override_' + labId, 'true');
    } else if (!cleanUrl) {
      window.localStorage.removeItem('override_' + labId);
    }
    if (cleanUrl && cleanUrl.startsWith('https://script.google.com')) {
      LAB_CONFIG[labId] = cleanUrl;
      if (labId === 'lab0' || labId === 'lab0-basic' || labId === 'lab-basic') {
        LAB_CONFIG['lab0'] = cleanUrl;
        LAB_CONFIG['lab0-basic'] = cleanUrl;
        LAB_CONFIG['lab-basic'] = cleanUrl;
      }
    }
  } catch (e) {}
}

/**
 * ดึง Web App URL ที่พร้อมใช้งาน
 * ลำดับการตรวจสอบ:
 * 1. ตรวจสอบจาก URL Query Parameter (?gas=... หรือ ?gas_url=...) ก่อน (สำหรับลิงก์ที่อาจารย์แนบส่งให้นักศึกษา)
 * 2. หากมีการเปิดใช้งาน Master GAS URL (จาก localStorage 'master_gas_url' หรือตัวแปร MASTER_GAS_URL)
 *    และแล็บนี้ไม่ได้ระบุ override แยกต่างหาก (override_<labId> !== 'true'):
 *    ให้ใช้ Master GAS URL ทันที! (เพื่อป้องกันคีย์เก่าที่เคยบันทึกไว้ในเบราว์เซอร์ดึงคะแนนไปชีตอื่น)
 * 3. ตรวจสอบจาก localStorage ในเครื่องเฉพาะแล็บนั้นๆ (กรณีอาจารย์จงใจแยกชีตสำหรับแล็บนี้)
 * 4. ตรวจสอบจาก LAB_CONFIG ใน config.js
 * 5. Fallback เป็น MASTER_GAS_URL ส่วนกลาง
 */
function getLabScriptUrl(labId) {
  if (typeof window !== 'undefined') {
    // 1. ตรวจสอบ URL Query Parameter (?gas=... หรือ ?gas_url=...)
    if (window.location && window.location.search) {
      const params = new URLSearchParams(window.location.search);
      let queryGas = params.get('gas') || params.get('gas_url');
      if (queryGas) {
        try {
          if (queryGas.startsWith('https%3A') || queryGas.startsWith('http%3A')) {
            queryGas = decodeURIComponent(queryGas);
          }
        } catch (e) {}
        if (queryGas.trim().startsWith('https://script.google.com')) {
          const cleanGas = queryGas.trim();
          if (labId) setLabScriptUrl(labId, cleanGas);
          return cleanGas;
        }
      }
    }

    // 2. หากมีการเปิดใช้งาน Master GAS URL และไม่มีการระบุ override ให้ใช้ Master GAS URL ทันที
    const masterUrl = (window.localStorage ? (window.localStorage.getItem('master_gas_url') || window.localStorage.getItem('iot_master_gas_url')) : '') || (typeof MASTER_GAS_URL !== 'undefined' ? MASTER_GAS_URL : '');
    const isExplicitOverride = window.localStorage && labId && (window.localStorage.getItem('override_' + labId) === 'true');

    if (masterUrl && masterUrl.trim().startsWith('https://script.google.com') && !isExplicitOverride) {
      return masterUrl.trim();
    }

    // 3. ตรวจสอบ localStorage ในเครื่อง เฉพาะแล็บ (กรณีไม่ได้ตั้ง Master หรือมีการสั่ง override)
    if (window.localStorage && labId) {
      const keys = getRelatedGasStorageKeys(labId);
      for (let i = 0; i < keys.length; i++) {
        const custom = window.localStorage.getItem(keys[i]);
        if (custom && custom.trim().startsWith('https://script.google.com')) {
          return custom.trim();
        }
      }
    }
  }
  // 4. Fallback ตรวจสอบจาก LAB_CONFIG ใน config.js
  const url = LAB_CONFIG[labId] || ((labId === 'lab0' || labId === 'lab0-basic') ? LAB_CONFIG['lab-basic'] : '') || '';
  if (url && !url.includes('YOUR_GAS_URL')) {
    return url.trim();
  }
  // 5. Fallback ตรวจสอบจาก MASTER_GAS_URL ส่วนกลาง
  if (typeof MASTER_GAS_URL !== 'undefined' && MASTER_GAS_URL && MASTER_GAS_URL.trim().startsWith('https://script.google.com')) {
    return MASTER_GAS_URL.trim();
  }
  return '';
}

if (typeof module !== 'undefined' && module.exports) {
  module.exports = { MASTER_GAS_URL, LAB_CONFIG, getLabScriptUrl, setLabScriptUrl, getMasterScriptUrl, setMasterScriptUrl };
}

