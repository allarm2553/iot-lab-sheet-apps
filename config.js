/**
 * IoT Lab Worksheets — Global Configuration
 * ตั้งค่าลิงก์ Google Apps Script Web App URL แยกทีละใบงาน สำหรับส่งคะแนนเข้า Google Sheets ของอาจารย์ผู้สอน
 * 
 * 💡 วิธีใช้งาน:
 * นำ Web App URL ที่ได้จากการ Deploy (Deploy > New Deployment > Web app) ใน Google Apps Script
 * ของแต่ละใบงาน มาวางในช่องด้านล่างนี้ หรือแก้ไขผ่านหน้าต่าง "ตั้งค่าลิงก์ส่งงาน" บนหน้ารวม Master Portal ได้ทันที
 */

const LAB_CONFIG = {
  // หมวดที่ 1: พื้นฐานฮาร์ดแวร์และการเชื่อมต่ออินพุต/เอาต์พุต (Module 1)
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

  // หมวดที่ 4: การประมวลผลและการวิเคราะห์ข้อมูลขนาดใหญ่ (Module 4)
  "lab7": "YOUR_GAS_URL_FOR_LAB7",
  "lab8": "YOUR_GAS_URL_FOR_LAB8",
  "lab9": "YOUR_GAS_URL_FOR_LAB9",

  // หมวดที่ 5: ใบงานเสริมและการแปลงแอปพลิเคชัน (Module 5)
  "lab-extra": "YOUR_GAS_URL_FOR_LAB_EXTRA",
  "lab-webconfig_wifi": "YOUR_GAS_URL_FOR_LAB_WEBCONFIG_WIFI"
};

/**
 * ดึง Web App URL ที่พร้อมใช้งาน
 * ลำดับการตรวจสอบ:
 * 1. ตรวจสอบจาก URL Query Parameter (?gas=... หรือ ?gas_url=...) ก่อน (สำหรับลิงก์ที่อาจารย์แนบส่งให้นักศึกษา)
 * 2. ตรวจสอบจาก localStorage (หากอาจารย์บันทึกผ่านหน้าเว็บในเบราว์เซอร์นี้)
 * 3. ใช้ค่าเริ่มต้นจาก LAB_CONFIG ใน config.js
 */
function getLabScriptUrl(labId) {
  if (typeof window !== 'undefined') {
    // 1. ตรวจสอบ URL Query Parameter (?gas=... หรือ ?gas_url=...)
    if (window.location && window.location.search) {
      const params = new URLSearchParams(window.location.search);
      const queryGas = params.get('gas') || params.get('gas_url');
      if (queryGas && queryGas.trim().startsWith('https://script.google.com')) {
        const cleanGas = queryGas.trim();
        if (window.localStorage && labId) {
          try { window.localStorage.setItem('gas_url_' + labId, cleanGas); } catch(e){}
        }
        return cleanGas;
      }
    }
    // 2. ตรวจสอบ localStorage ในเครื่อง
    if (window.localStorage) {
      const custom = window.localStorage.getItem('gas_url_' + labId);
      if (custom && custom.trim().startsWith('https://script.google.com')) {
        return custom.trim();
      }
    }
  }
  // 3. Fallback เป็นค่าใน config.js
  const url = LAB_CONFIG[labId] || '';
  if (url && !url.includes('YOUR_GAS_URL')) {
    return url.trim();
  }
  return '';
}

if (typeof module !== 'undefined' && module.exports) {
  module.exports = { LAB_CONFIG, getLabScriptUrl };
}
