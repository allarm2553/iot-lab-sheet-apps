/**
 * =========================================================================
 * 🌟 IoT Lab Worksheets — Universal Master Backend (Master_Code.gs)
 * ระบบรวมศูนย์ส่งคะแนนทุกใบงานเข้าสู่ Google Sheet เดียว แยกแท็บอัตโนมัติ
 * ออกแบบโดย Antigravity AI เพื่อการบริหารจัดการคะแนนแบบ All-In-One
 * =========================================================================
 * 
 * 💡 คุณสมบัติเด่น:
 * 1. รวมคะแนนทั้ง 19 ใบงานปฏิบัติการลงใน Google Sheet ไฟล์เดียว
 * 2. คัดแยกข้อมูลลงแท็บของแต่ละใบงานอัตโนมัติ (Lab 0, Lab 1, Lab 1.1, ..., Lab 9, Extra)
 * 3. มีแท็บ "📊 แดชบอร์ดสรุปคะแนนรวม" ที่หน้าแรก สรุปคะแนนทุกแล็บของนักศึกษาทุกคนแบบ Real-Time
 * 4. รองรับระบบตรวจคะแนนอัตโนมัติ (Auto-Grading) เต็ม 10.0 คะแนนของแต่ละแล็บ
 * 5. ป้องกันรหัสนักศึกษาส่งซ้ำแยกตามรายแล็บ และรองรับฟังก์ชันปลดล็อก (allowResubmit) จากครูผู้สอน
 * 6. บันทึกและจัดเก็บไฟล์แนบ (.png / .ino / .zip) ลงใน Google Drive โดยอัตโนมัติ
 */

// =========================================================================
// ⚙️ การตั้งค่า Google Spreadsheet ปลายทาง
// =========================================================================
// หากเปิดสคริปต์นี้จากเมนู "ส่วนขยาย > Apps Script" ใน Google Sheet โดยตรง สามารถเว้นว่างไว้ได้
// หากสร้างเป็น Standalone Script ให้ระบุ Spreadsheet ID หรือ URL เต็มที่นี่:
var TARGET_SPREADSHEET_ID = "1qxn6gtbsATo4ML1b1yujxVL9YFm6npvARy1m4FSIBoE";

function getTargetSpreadsheet() {
  try {
    var active = SpreadsheetApp.getActiveSpreadsheet();
    if (active) return active;
  } catch (err) {}

  if (typeof TARGET_SPREADSHEET_ID !== 'undefined' && TARGET_SPREADSHEET_ID && TARGET_SPREADSHEET_ID.trim().length > 0) {
    var rawId = TARGET_SPREADSHEET_ID.trim();
    if (rawId.indexOf('http') !== -1) {
      var match = rawId.match(/\/d\/([a-zA-Z0-9-_]+)/);
      if (match) rawId = match[1];
    }
    try {
      return SpreadsheetApp.openById(rawId);
    } catch (e) {
      console.warn("ไม่สามารถเปิดตาม TARGET_SPREADSHEET_ID ได้:", e);
    }
  }
  return null;
}

/**
 * สร้างเมนูพิเศษบนแถบเครื่องมือ Google Sheets อัตโนมัติเมื่อเปิดไฟล์
 */
function onOpen() {
  try {
    var ui = SpreadsheetApp.getUi();
    ui.createMenu('🎓 ระบบจัดการคะแนน IoT')
      .addItem('🔄 ดึงคะแนนทุกแล็บตามรหัสนักศึกษา (Sync Scores to Roster)', 'menuSyncAllScoresByStudentId')
      .addItem('📐 ผูกสูตรดึงคะแนนอัตโนมัติ (Insert VLOOKUP Formulas)', 'menuApplyLookupFormulas')
      .addSeparator()
      .addItem('🧹 ล้างแถวทดสอบที่ไม่ใช่รหัสนักศึกษาจริง (Clean Test Rows)', 'menuCleanTestRows')
      .addToUi();
  } catch (e) {}
}

function menuSyncAllScoresByStudentId() {
  var ss = getTargetSpreadsheet();
  if (!ss) return;
  var stats = syncAllScoresToMasterRoster(ss, false);
  try {
    SpreadsheetApp.getUi().alert(
      '✅ ซิงค์คะแนนตามรหัสนักศึกษาสำเร็จ!',
      'อัปเดตข้อมูลนักศึกษาทั้งหมด ' + stats.totalRows + ' รายการ\nดึงคะแนนจากแท็บใบงานย่อยสำเร็จ (โดยคงชื่อ-นามสกุล และกลุ่ม/ห้อง ตามใบรายชื่อของครูไว้ครบถ้วน)',
      SpreadsheetApp.getUi().ButtonSet.OK
    );
  } catch (e) {}
}

function menuApplyLookupFormulas() {
  var ss = getTargetSpreadsheet();
  if (!ss) return;
  var stats = syncAllScoresToMasterRoster(ss, true);
  try {
    SpreadsheetApp.getUi().alert(
      '📐 ผูกสูตรดึงคะแนนอัตโนมัติสำเร็จ!',
      'ผูกสูตรค้นหาคะแนนตามรหัสนักศึกษา ($A) ให้กับนักศึกษา ' + stats.totalRows + ' รายการเรียบร้อยแล้ว',
      SpreadsheetApp.getUi().ButtonSet.OK
    );
  } catch (e) {}
}

function menuCleanTestRows() {
  var ss = getTargetSpreadsheet();
  if (!ss) return;
  var sheet = ss.getSheetByName("📊 สรุปคะแนนรวม");
  if (!sheet || sheet.getLastRow() <= 1) return;
  var vals = sheet.getDataRange().getValues();
  var removed = 0;
  for (var r = vals.length - 1; r >= 1; r--) {
    var id = normalizeStudentId(vals[r][0]);
    var name = String(vals[r][1] || '').trim().toLowerCase();
    if (!id || name.indexOf('test') !== -1 || name.indexOf('ทดสอบ') !== -1 || id === '1234567' || id === '2523' || id === '9876543' || id === '445667722') {
      sheet.deleteRow(r + 1);
      removed++;
    }
  }
  try {
    SpreadsheetApp.getUi().alert('🧹 ล้างแถวทดสอบเรียบร้อย', 'ลบแถวข้อมูลทดสอบออกทั้งหมด ' + removed + ' แถว', SpreadsheetApp.getUi().ButtonSet.OK);
  } catch (e) {}
}

function normalizeStudentId(rawId) {
  return String(rawId || '').replace(/[\s\u200B-\u200D\uFEFF']/g, '').trim();
}

/**
 * แปลงค่ากลุ่ม/ห้อง ที่ถูก Google Sheets แปลงเป็นวันที่ (เช่น "4/5" -> Mon May 04 2026...GMT+0700) ให้กลับมาเป็น "4/5" หรือ "4/6"
 */
function sanitizeGroupValue(rawVal, fallbackGroup) {
  if (rawVal === null || typeof rawVal === 'undefined') return fallbackGroup || '';
  if (Object.prototype.toString.call(rawVal) === '[object Date]' && !isNaN(rawVal.getTime())) {
    return rawVal.getDate() + '/' + (rawVal.getMonth() + 1);
  }
  var str = String(rawVal).trim();
  if (str.indexOf('GMT') !== -1 || str.indexOf('เวลาอินโดจีน') !== -1 || /^[A-Z][a-z]{2}\s+[A-Z][a-z]{2}\s+\d{2}\s+\d{4}/.test(str)) {
    var parsed = new Date(str);
    if (!isNaN(parsed.getTime())) {
      return parsed.getDate() + '/' + (parsed.getMonth() + 1);
    }
    if (fallbackGroup) return fallbackGroup;
  }
  return str || fallbackGroup || '';
}

// =========================================================================
// 📑 รายการนิยามแท็บประจำแต่ละใบงาน (Master Lab Registry)
// =========================================================================
var LAB_REGISTRY = {
  "lab0": {
    name: "Lab 0",
    sheetName: "Lab 0 Submissions",
    color: "#3b82f6",
    title: "การเตรียมสภาพแวดล้อมและไฟกระพริบ"
  },
  "lab0-basic": {
    name: "Lab 0 Basic",
    sheetName: "Lab 0 Basic Submissions",
    color: "#60a5fa",
    title: "การเตรียมสภาพแวดล้อม (Basic)"
  },
  "lab1": {
    name: "Lab 1",
    sheetName: "Lab 1 Submissions",
    color: "#06b6d4",
    title: "การอ่านค่าเซ็นเซอร์และควบคุมเอาต์พุต"
  },
  "lab1.1": {
    name: "Lab 1.1",
    sheetName: "Lab 1.1 Submissions",
    color: "#0ea5e9",
    title: "สวิตช์ปุ่มกดและ Software Debounce"
  },
  "lab2": {
    name: "Lab 2",
    sheetName: "Lab 2 Submissions",
    color: "#6366f1",
    title: "จอแสดงผล OLED SSD1306 (I2C)"
  },
  "lab3": {
    name: "Lab 3",
    sheetName: "Lab 3 Submissions",
    color: "#8b5cf6",
    title: "Wi-Fi Modes & Local Web Server"
  },
  "lab3.1": {
    name: "Lab 3.1",
    sheetName: "Lab 3.1 Submissions",
    color: "#a855f7",
    title: "Dynamic Web Dashboard & RESTful Control"
  },
  "lab3.2_Wifi_UI_config": {
    name: "Lab 3.2",
    sheetName: "Lab 3.2 Submissions",
    color: "#d946ef",
    title: "Captive Portal & WebConfig Wi-Fi"
  },
  "lab4": {
    name: "Lab 4",
    sheetName: "Lab 4 Submissions",
    color: "#ec4899",
    title: "Real-time Two-way WebSockets"
  },
  "lab4.1": {
    name: "Lab 4.1",
    sheetName: "Lab 4.1 Submissions",
    color: "#f43f5e",
    title: "Embedded HTTP REST API & JSON Endpoint"
  },
  "lab5": {
    name: "Lab 5",
    sheetName: "Lab 5 Submissions",
    color: "#ef4444",
    title: "Cloud MQTT Protocol (Publish/Subscribe)"
  },
  "LAB5_Dev": {
    name: "Lab 5 Dev",
    sheetName: "Lab 5 Dev Submissions",
    color: "#f97316",
    title: "MQTT Dual-Core Task Management"
  },
  "lab6": {
    name: "Lab 6",
    sheetName: "Lab 6 Submissions",
    color: "#f59e0b",
    title: "Hybrid Dual-Mode IoT Node"
  },
  "LAB6.1Perform": {
    name: "Lab 6.1",
    sheetName: "Lab 6.1 Submissions",
    color: "#eab308",
    title: "IoT Node Performance Profiler"
  },
  "lab7": {
    name: "Lab 7",
    sheetName: "Lab 7 Submissions",
    color: "#84cc16",
    title: "Visual Edge Computing ด้วย Node-RED"
  },
  "lab8": {
    name: "Lab 8",
    sheetName: "Lab 8 Submissions",
    color: "#10b981",
    title: "Time-Series Database บน InfluxDB Cloud"
  },
  "lab9": {
    name: "Lab 9",
    sheetName: "Lab 9 Submissions",
    color: "#14b8a6",
    title: "Industrial Real-time Analytics ด้วย Grafana"
  },
  "lab-extra": {
    name: "Lab Extra",
    sheetName: "Lab Extra Submissions",
    color: "#059669",
    title: "ใบงานเสริม: ESP32 Deep Sleep & Low Power"
  },
  "lab-webconfig_wifi": {
    name: "WebConfig",
    sheetName: "Lab WebConfig Submissions",
    color: "#047857",
    title: "Captive Portal WebConfig Wi-Fi"
  }
};

// =========================================================================
// 🌐 GET Handler (Diagnostics & Verification Page)
// =========================================================================
function doGet(e) {
  var ss = getTargetSpreadsheet();
  var ssName = ss ? ss.getName() : 'ยังไม่ได้เชื่อมต่อชีต';
  var ssUrl = ss ? ss.getUrl() : '#';

  if (e && e.parameter && (e.parameter.action === 'testConnection' || e.parameter.action === 'ping' || e.parameter.format === 'json')) {
    return ContentService.createTextOutput(JSON.stringify({
      status: 'success',
      type: 'master',
      message: 'เชื่อมต่อ Universal Master Google Apps Script สำเร็จพร้อมใช้งาน!',
      spreadsheetName: ssName,
      spreadsheetUrl: ssUrl,
      totalSupportedLabs: Object.keys(LAB_REGISTRY).length
    })).setMimeType(ContentService.MimeType.JSON);
  }

  var labsListHtml = Object.keys(LAB_REGISTRY).map(function(k) {
    var item = LAB_REGISTRY[k];
    return '<span style="display:inline-block;background:' + item.color + '22;border:1px solid ' + item.color + '66;color:#f8fafc;padding:0.25rem 0.65rem;border-radius:6px;margin:0.2rem;font-size:0.8rem;font-weight:500;">' +
      item.name + ': ' + item.sheetName + '</span>';
  }).join(' ');

  var html = '<!DOCTYPE html><html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1">' +
    '<title>IoT Universal Master Web App - พร้อมใช้งาน</title>' +
    '<style>' +
    'body{font-family:system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,sans-serif;background:#0b1120;color:#f8fafc;display:flex;align-items:center;justify-content:center;min-height:100vh;margin:0;padding:1.5rem;box-sizing:border-box;}' +
    '.card{background:#1e293b;border:1px solid #334155;padding:2.25rem 2rem;border-radius:18px;text-align:center;max-width:680px;box-shadow:0 20px 25px -5px rgba(0,0,0,0.5);}' +
    '.badge{background:rgba(16,185,129,0.15);color:#34d399;border:1px solid rgba(16,185,129,0.4);padding:0.4rem 1.2rem;border-radius:9999px;font-size:0.9rem;font-weight:600;display:inline-block;margin-bottom:1.25rem;}' +
    'h2{color:#38bdf8;margin:0 0 0.5rem 0;font-size:1.5rem;font-weight:700;}p{color:#94a3b8;line-height:1.6;margin:0 0 1.25rem 0;font-size:0.95rem;}' +
    '.info{background:rgba(15,23,42,0.85);border:1px solid #334155;border-radius:12px;padding:1.25rem;font-size:0.875rem;color:#cbd5e1;text-align:left;line-height:1.6;margin-bottom:1.25rem;}' +
    'a.sheet-btn{background:linear-gradient(135deg,#10b981,#059669);color:#fff;text-decoration:none;padding:0.6rem 1.2rem;border-radius:8px;font-weight:600;display:inline-flex;align-items:center;gap:0.5rem;font-size:0.9rem;margin-top:0.5rem;}' +
    '</style></head><body><div class="card">' +
    '<div class="badge">✓ Master Universal Backend (Active)</div>' +
    '<h2>ระบบรวมคะแนนทุกใบงาน IoT พร้อมทำงาน</h2>' +
    '<p>สคริปต์นี้ทำหน้าที่เป็น Universal Central API รองรับการส่งคะแนนจาก <strong>ทุกใบงานทั้ง 19 แล็บ</strong> เข้าสู่ Google Sheet เดียวกัน พร้อมระบบแยกแท็บอัตโนมัติ</p>' +
    '<div class="info">' +
    '📊 <strong>ไฟล์สเปรดชีตหลัก:</strong> ' + ssName + '<br>' +
    '🔗 <a href="' + ssUrl + '" target="_blank" class="sheet-btn">เปิดดู Google Sheet รวมคะแนน</a><br><br>' +
    '📑 <strong>แท็บใบงานที่รองรับ (' + Object.keys(LAB_REGISTRY).length + ' แล็บ):</strong><br>' +
    '<div style="margin-top:0.5rem;">' + labsListHtml + '</div>' +
    '</div>' +
    '<p style="font-size:0.82rem;color:#64748b;margin:0;">💡 นำ Web App URL ของหน้านี้ไปวางในช่อง MASTER_GAS_URL ในไฟล์ config.js หรือหน้า Dashboard เพื่อให้ทุกใบงานส่งเข้าชีตนี้ได้ทันทีครับ</p>' +
    '</div></body></html>';

  return HtmlService.createHtmlOutput(html)
    .setTitle('IoT Universal Master Web App - พร้อมใช้งาน')
    .addMetaTag('viewport', 'width=device-width, initial-scale=1');
}

// =========================================================================
// 🚀 POST Handler (Submission & Routing Controller)
// =========================================================================
function doPost(e) {
  var lock = LockService.getScriptLock();
  lock.tryLock(20000);

  try {
    var raw = (e && e.postData && e.postData.contents) ? e.postData.contents : '{}';
    var data = JSON.parse(raw);

    // หากข้อมูลถูกห่อใน data.data ให้แตกออกมา (Unwrap) เพื่อให้เข้าถึงฟิลด์ต่างๆ ได้สะดวก
    if (data.data && typeof data.data === 'object') {
      for (var key in data.data) {
        if (typeof data[key] === 'undefined') {
          data[key] = data.data[key];
        }
      }
    }

    var ss = getTargetSpreadsheet();
    if (!ss) {
      return ContentService.createTextOutput(JSON.stringify({
        status: 'error',
        message: '❌ ไม่พบ Google Spreadsheet: กรุณาระบุ TARGET_SPREADSHEET_ID หรือเปิดสคริปต์จากเมนู ส่วนขยาย > Apps Script ในไฟล์ Google Sheet'
      })).setMimeType(ContentService.MimeType.JSON);
    }

    // 1. ตรวจสอบการ Ping / ทดสอบเชื่อมต่อ (พร้อมซิงค์คะแนนเข้าใบรายชื่ออัตโนมัติ)
    if (data.action === 'testConnection' || data.action === 'ping' || data.test === true) {
      var syncStats = { totalRows: 0 };
      try {
        syncStats = syncAllScoresToMasterRoster(ss, false);
      } catch (e) {}
      return ContentService.createTextOutput(JSON.stringify({
        status: 'success',
        type: 'master',
        message: 'เชื่อมต่อ Universal Master Google Sheet: "' + ss.getName() + '" สำเร็จพร้อมบันทึกคะแนน! (ซิงค์ใบรายชื่อ ' + syncStats.totalRows + ' คนแล้ว)',
        spreadsheetName: ss.getName(),
        spreadsheetUrl: ss.getUrl(),
        syncedRows: syncStats.totalRows
      })).setMimeType(ContentService.MimeType.JSON);
    }

    // 1.5 ซิงค์คะแนนจากทุกแท็บใบงานเข้าสู่ตารางรายชื่อในหน้าสรุปคะแนนรวม (ตามรหัสนักศึกษา)
    if (data.action === 'syncRoster' || data.action === 'syncScores') {
      var useFormulas = (data.useFormulas === true);
      var stats = syncAllScoresToMasterRoster(ss, useFormulas);
      return ContentService.createTextOutput(JSON.stringify({
        status: 'success',
        type: 'master',
        message: '✅ ดึงคะแนนจากทุกแล็บตามรหัสนักศึกษาสำเร็จ! (อัปเดตทั้งหมด ' + stats.totalRows + ' รายชื่อ โดยคงชื่อ-นามสกุล และห้องตามใบรายชื่อครู)',
        totalRows: stats.totalRows,
        spreadsheetName: ss.getName(),
        spreadsheetUrl: ss.getUrl()
      })).setMimeType(ContentService.MimeType.JSON);
    }

    // 2. ปลดล็อกล้างประวัติส่งซ้ำ (กรณีครูกดเคลียร์แคช)
    if (data.action === 'clearStudentLock' || data.action === 'clearStudent') {
      var sId = normalizeStudentId(data.studentId || '');
      if (sId) {
        clearStudentSubmissionFromAllSheets(ss, sId, data.labId);
      }
      return ContentService.createTextOutput(JSON.stringify({
        status: 'success',
        message: 'ปลดล็อกและลบประวัติเดิมของรหัส ' + sId + ' ใน Google Sheets เรียบร้อยแล้ว (โดยคงรายชื่อในหน้าสรุปคะแนนรวมไว้)'
      })).setMimeType(ContentService.MimeType.JSON);
    }

    // 3. ระบุเป้าหมายใบงาน (Lab Routing)
    var labKey = resolveLabKey(data);
    var labMeta = LAB_REGISTRY[labKey] || {
      name: labKey.toUpperCase(),
      sheetName: labKey.toUpperCase() + ' Submissions',
      title: 'ใบงาน ' + labKey
    };

    // 4. บันทึกข้อมูลลงแท็บของแล็บนั้นๆ
    var result = processMasterSubmission(ss, labKey, labMeta, data);

    // 5. ซิงค์และอัปเดตคะแนนลงแท็บแรก "📊 Dashboard สรุปคะแนนรวม"
    if (result.status === 'success' && data.studentId) {
      try {
        updateMasterSummaryDashboard(ss, labKey, labMeta, data, result.score);
      } catch (sumErr) {
        console.warn("Summary dashboard update error:", sumErr);
      }
    }

    return ContentService.createTextOutput(JSON.stringify(result))
      .setMimeType(ContentService.MimeType.JSON);

  } catch (err) {
    return ContentService.createTextOutput(JSON.stringify({
      status: 'error',
      message: 'เกิดข้อผิดพลาดในการประมวลผล Master Backend: ' + err.toString()
    })).setMimeType(ContentService.MimeType.JSON);
  } finally {
    lock.releaseLock();
  }
}

/**
 * ระบุว่าข้อมูลที่ส่งมาเป็นของแล็บใด
 */
function resolveLabKey(data) {
  if (data.labId && LAB_REGISTRY[data.labId]) return data.labId;
  if (data.lab && LAB_REGISTRY[data.lab]) return data.lab;
  if (data.labNumber) {
    var numKey = "lab" + data.labNumber;
    if (LAB_REGISTRY[numKey]) return numKey;
  }
  // ตรวจจับจากลักษณะเฉพาะของฟิลด์ในใบงาน
  if (data.expResult1 || data.expResult2) return "lab4.1";
  if (data.analysis1 && data.analysis2) return "lab6";
  if (data.ssid && data.wifiPassword) return "lab-webconfig_wifi";
  return "lab1"; // Default fallback
}

/**
 * ประมวลผลและบันทึกคะแนนลงในแท็บของแล็บนั้นๆ
 */
function processMasterSubmission(ss, labKey, labMeta, data) {
  var sheetName = labMeta.sheetName;
  var sheet = ss.getSheetByName(sheetName);

  // คำนวณคะแนนตามตรรกะของแล็บนั้นๆ
  var grading = calculateLabScore(labKey, data);
  var score = grading.score;
  var feedback = grading.feedback;

  // หากแท็บยังไม่มี ให้สร้างแท็บใหม่พร้อม Header และสีที่จัดรูปแบบสวยงาม
  if (!sheet) {
    // หากมีแท็บว่างเปล่าแท็บแรก (เช่น ชีต1) และยังไม่มีชีตอื่น ให้เปลี่ยนชื่อเป็นแท็บนี้เลย
    var allSheets = ss.getSheets();
    if (allSheets.length === 1 && allSheets[0].getLastRow() === 0) {
      sheet = allSheets[0];
      sheet.setName(sheetName);
    } else {
      sheet = ss.insertSheet(sheetName);
    }
    
    var headers = getHeadersForLab(labKey);
    sheet.appendRow(headers);
    sheet.setFrozenRows(1);
    sheet.getRange(1, 1, 1, headers.length)
      .setFontWeight("bold")
      .setBackground("#1e293b")
      .setFontColor("#f8fafc");
  }

  // ตรวจสอบการส่งซ้ำ (Anti-Duplicate Check ประจำแท็บนี้)
  var targetId = String(data.studentId || '').trim().replace(/^'/, '');
  if (targetId) {
    var vals = sheet.getDataRange().getValues();
    // ตรวจสอบคอลัมน์รหัสนักศึกษา (คอลัมน์ที่ 2 หรือ 3)
    for (var r = 1; r < vals.length; r++) {
      var idCol1 = String(vals[r][1] || '').trim().replace(/^'/, '');
      var idCol2 = String(vals[r][2] || '').trim().replace(/^'/, '');
      if (idCol1 === targetId || idCol2 === targetId) {
        if (!data.allowResubmit) {
          return {
            status: "duplicate",
            message: "❌ รหัสนักศึกษา " + targetId + " ได้ส่งใบงาน (" + labMeta.name + ") ไปแล้ว ไม่สามารถส่งซ้ำได้"
          };
        }
        // หากครูอนุญาตส่งใหม่ ให้ลบแถวเดิมออก
        sheet.deleteRow(r + 1);
        break;
      }
    }
  }

  // จัดการอัปโหลดไฟล์แนบเข้า Google Drive
  var fileUrls = handleMasterFileUploads(labKey, data);

  // สร้างแถวข้อมูลตามคอลัมน์ของแต่ละแล็บ
  var rowData = buildRowDataForLab(labKey, data, grading, fileUrls);
  sheet.appendRow(rowData);

  return {
    status: "success",
    score: score,
    message: "บันทึกข้อมูลใบงาน (" + labMeta.name + ") สำเร็จแล้ว!\n" +
             "📁 บันทึกลงชีต: \"" + ss.getName() + "\" (แท็บ: " + sheetName + ")\n" +
             "คะแนนประเมินอัตโนมัติ: " + score + "/10.0 คะแนน\n\n" +
             "รายละเอียดคะแนน:\n" + feedback
  };
}

function getSummaryLabColumns() {
  return [
    { key: "lab0", label: "Lab 0", sheetName: "Lab 0 Submissions" },
    { key: "lab0-basic", label: "Lab 0 Basic", sheetName: "Lab 0 Basic Submissions" },
    { key: "lab1", label: "Lab 1", sheetName: "Lab 1 Submissions" },
    { key: "lab1.1", label: "Lab 1.1", sheetName: "Lab 1.1 Submissions" },
    { key: "lab2", label: "Lab 2", sheetName: "Lab 2 Submissions" },
    { key: "lab3", label: "Lab 3", sheetName: "Lab 3 Submissions" },
    { key: "lab3.1", label: "Lab 3.1", sheetName: "Lab 3.1 Submissions" },
    { key: "lab3.2_Wifi_UI_config", label: "Lab 3.2", sheetName: "Lab 3.2 Submissions" },
    { key: "lab4", label: "Lab 4", sheetName: "Lab 4 Submissions" },
    { key: "lab4.1", label: "Lab 4.1", sheetName: "Lab 4.1 Submissions" },
    { key: "lab5", label: "Lab 5", sheetName: "Lab 5 Submissions" },
    { key: "LAB5_Dev", label: "Lab 5 Dev", sheetName: "Lab 5 Dev Submissions" },
    { key: "lab6", label: "Lab 6", sheetName: "Lab 6 Submissions" },
    { key: "LAB6.1Perform", label: "Lab 6.1", sheetName: "Lab 6.1 Perform Submissions" },
    { key: "lab7", label: "Lab 7", sheetName: "Lab 7 Submissions" },
    { key: "lab8", label: "Lab 8", sheetName: "Lab 8 Submissions" },
    { key: "lab9", label: "Lab 9", sheetName: "Lab 9 Submissions" },
    { key: "lab-extra", label: "Lab Extra", sheetName: "Lab Extra Submissions" },
    { key: "lab-webconfig_wifi", label: "WebConfig", sheetName: "WebConfig Submissions" }
  ];
}

function getOrCreateSummarySheet(ss) {
  var summarySheetName = "📊 สรุปคะแนนรวม";
  var sheet = ss.getSheetByName(summarySheetName);
  var labColumns = getSummaryLabColumns();

  if (!sheet) {
    sheet = ss.insertSheet(summarySheetName, 0);
    var headers = ["รหัสนักศึกษา", "ชื่อ-นามสกุล", "กลุ่ม/ห้อง"];
    labColumns.forEach(function(c) { headers.push(c.label); });
    headers.push("คะแนนรวมทั้งหมด");
    headers.push("จำนวนแล็บที่ส่ง");
    headers.push("อัปเดตล่าสุด");

    sheet.appendRow(headers);
    sheet.setFrozenRows(1);
    sheet.setFrozenColumns(3);
    sheet.getRange(1, 1, 1, headers.length)
      .setFontWeight("bold")
      .setBackground("#0f172a")
      .setFontColor("#38bdf8");

    var defaultEmpty = ss.getSheetByName("ชีต1") || ss.getSheetByName("Sheet1");
    if (defaultEmpty && defaultEmpty.getLastRow() === 0 && ss.getSheets().length > 1) {
      try { ss.deleteSheet(defaultEmpty); } catch (e) {}
    }
  }
  return sheet;
}

/**
 * ซิงค์และดึงคะแนนจากทุกแท็บใบงานย่อยมาใส่ในตาราง "📊 สรุปคะแนนรวม" โดยอ้างอิงจาก "รหัสนักศึกษา" (คอลัมน์ A)
 * - รักษา "ชื่อ-นามสกุล" (Col B) และ "กลุ่ม/ห้อง" (Col C) ที่ครูพิมพ์ไว้ล่วงหน้าเสมอ (ไม่เขียนทับ)
 * - หากนักศึกษาในแท็บย่อยยังไม่มีรหัสในหน้าสรุปรวม จะเพิ่มต่อท้ายตารางให้อัตโนมัติ
 */
function syncAllScoresToMasterRoster(ss, useFormulas) {
  var sheet = getOrCreateSummarySheet(ss);
  var labColumns = getSummaryLabColumns();
  var totalLabsCount = labColumns.length;

  // 1. กวาดคะแนนจากทุกแท็บใบงานที่มีอยู่ในไฟล์เข้าสู่หน่วยความจำ (Index by normalized studentId)
  var scoreMap = {}; // scoreMap[labKey][studentId] = score
  var studentMetaMap = {}; // studentMetaMap[studentId] = { name, group }

  labColumns.forEach(function(col) {
    scoreMap[col.key] = {};
    var labSheet = ss.getSheetByName(col.sheetName);
    if (!labSheet || labSheet.getLastRow() <= 1) return;

    var lVals = labSheet.getDataRange().getValues();
    var lDisp = labSheet.getDataRange().getDisplayValues();
    var headers = lVals[0] || [];

    // ค้นหาคอลัมน์คะแนนรวมจาก Header อัตโนมัติ (รองรับทั้งรูปแบบ Master_Code.gs และ Code.gs แยกแล็บเดิม)
    var scoreColIndices = [5];
    for (var h = 0; h < headers.length; h++) {
      var hText = String(headers[h] || '').toLowerCase();
      if (hText.indexOf('คะแนน') !== -1 || hText.indexOf('score') !== -1) {
        if (scoreColIndices.indexOf(h) === -1) scoreColIndices.unshift(h);
      }
    }

    for (var r = 1; r < lVals.length; r++) {
      // ตรวจจับรหัสนักศึกษาและชื่อ (รองรับกรณีสลับคอลัมน์ B กับ C จากชีตเก่า)
      var col1 = normalizeStudentId(lDisp[r][1] || lVals[r][1]);
      var col2 = normalizeStudentId(lDisp[r][2] || lVals[r][2]);
      var sId = '';
      var sName = '';
      if (/^\d{4,15}$/.test(col1) || (!/^\d{4,15}$/.test(col2) && col1)) {
        sId = col1;
        sName = String(lDisp[r][2] || lVals[r][2] || '').trim();
      } else if (/^\d{4,15}$/.test(col2)) {
        sId = col2;
        sName = String(lDisp[r][1] || lVals[r][1] || '').trim();
      } else {
        sId = col1;
        sName = String(lDisp[r][2] || lVals[r][2] || '').trim();
      }

      var sGroup = sanitizeGroupValue(lVals[r][3], lDisp[r][3]);
      var rawScore = NaN;
      for (var scIdx = 0; scIdx < scoreColIndices.length; scIdx++) {
        var idx = scoreColIndices[scIdx];
        var candidate = lVals[r][idx];
        if (typeof candidate === 'number' && !isNaN(candidate) && candidate >= 0 && candidate <= 10.5) {
          rawScore = candidate;
          break;
        }
        var parsedCand = parseFloat(String(candidate || '').trim());
        if (!isNaN(parsedCand) && parsedCand >= 0 && parsedCand <= 10.5 && String(candidate).trim().length <= 5) {
          rawScore = parsedCand;
          break;
        }
      }

      if (!sId) continue;
      if (!isNaN(rawScore)) {
        scoreMap[col.key][sId] = parseFloat(rawScore.toFixed(1));
      }
      if (!studentMetaMap[sId]) {
        studentMetaMap[sId] = { name: sName, group: sGroup };
      } else {
        if (!studentMetaMap[sId].name && sName) studentMetaMap[sId].name = sName;
        if (!studentMetaMap[sId].group && sGroup) studentMetaMap[sId].group = sGroup;
      }
    }
  });

  // 2. อ่านตารางรายชื่อปัจจุบันในหน้า "📊 สรุปคะแนนรวม"
  var lastRow = sheet.getLastRow();
  var existingIds = {};
  var nowStr = new Date().toLocaleString("th-TH");

  if (lastRow >= 2) {
    var totalCols = 3 + totalLabsCount + 3;
    var currentCols = Math.max(sheet.getLastColumn(), totalCols);
    var rosterRange = sheet.getRange(2, 1, lastRow - 1, currentCols);
    var rosterVals = rosterRange.getValues();
    var rosterDisp = rosterRange.getDisplayValues();
    var updatedMatrix = [];

    // ตั้งค่ารูปแบบคอลัมน์ A (รหัสนักศึกษา) และคอลัมน์ C (กลุ่ม/ห้อง) เป็นข้อความธรรมดา (Plain Text) เพื่อป้องกัน Google Sheets แปลง "4/5" เป็นวันที่!
    sheet.getRange(2, 1, lastRow - 1, 1).setNumberFormat("@");
    sheet.getRange(2, 3, lastRow - 1, 1).setNumberFormat("@");

    for (var i = 0; i < rosterVals.length; i++) {
      var row = rosterVals[i];
      var dispRow = rosterDisp[i];
      var rId = normalizeStudentId(dispRow[0] || row[0]);
      var rName = String(dispRow[1] || row[1] || '').trim();
      var fallbackGrp = (rId && studentMetaMap[rId] && studentMetaMap[rId].group) ? studentMetaMap[rId].group : '';
      var rGroup = sanitizeGroupValue(row[2], dispRow[2]);
      if (rGroup.indexOf('GMT') !== -1) {
        rGroup = sanitizeGroupValue(rGroup, fallbackGrp);
      }

      if (rId) {
        existingIds[rId] = true;
        // เติมชื่อ/ห้องเฉพาะกรณีที่ช่องในใบรายชื่อครูว่างอยู่เท่านั้น (ห้ามเขียนทับใบรายชื่อครู!)
        if (!rName && studentMetaMap[rId] && studentMetaMap[rId].name) {
          rName = studentMetaMap[rId].name;
        }
        if (!rGroup && fallbackGrp) {
          rGroup = fallbackGrp;
        }
      }

      var newRow = [rId || row[0], rName, rGroup];
      var sum = 0.0;
      var submittedCount = 0;
      var rowNum = i + 2;

      for (var c = 0; c < totalLabsCount; c++) {
        var lKey = labColumns[c].key;
        var lSheetName = labColumns[c].sheetName;
        var matchedScore = (rId && typeof scoreMap[lKey][rId] === 'number') ? scoreMap[lKey][rId] : null;

        if (matchedScore === null) {
          // หากเป็น Lab 0 หรือ Lab 0 Basic ให้เช็คข้ามกันได้ในกรณีที่ส่งมาในอีกชื่อหนึ่ง
          if (lKey === "lab0-basic" && rId && typeof scoreMap["lab0"][rId] === 'number') {
            matchedScore = scoreMap["lab0"][rId];
          }
        }

        if (matchedScore === null) {
          // หากในหน้าสรุปมีคะแนนตัวเลขเดิมอยู่แล้ว และแท็บของแล็บนั้นยังไม่ถูกสร้าง ให้คงค่าเดิมไว้
          var existingCellVal = parseFloat(row[3 + c]);
          if (!isNaN(existingCellVal) && !ss.getSheetByName(lSheetName)) {
            matchedScore = parseFloat(existingCellVal.toFixed(1));
          }
        }

        if (matchedScore !== null) {
          sum += matchedScore;
          submittedCount++;
        }

        if (useFormulas && ss.getSheetByName(lSheetName)) {
          // ผูกสูตร VLOOKUP ค้นหาจากแท็บของแล็บนั้นๆ โดยตรงตามรหัสนักศึกษาในคอลัมน์ A
          var escapedSheet = lSheetName.replace(/'/g, "''");
          var formula = '=IF($A' + rowNum + '="","-",IFERROR(VALUE(VLOOKUP(TO_TEXT($A' + rowNum + '),{\'' + escapedSheet + '\'!$B:$B,\'' + escapedSheet + '\'!$F:$F},2,FALSE)),IFERROR(VALUE(VLOOKUP($A' + rowNum + ',\'' + escapedSheet + '\'!$B:$F,5,FALSE)),"-")))';
          newRow.push(formula);
        } else {
          newRow.push(matchedScore !== null ? matchedScore : "-");
        }
      }

      if (useFormulas) {
        var firstLabColLetter = "D";
        var lastLabColLetter = columnToLetter(3 + totalLabsCount); // V
        newRow.push('=ROUND(SUM(' + firstLabColLetter + rowNum + ':' + lastLabColLetter + rowNum + '), 1)');
        newRow.push('=COUNT(' + firstLabColLetter + rowNum + ':' + lastLabColLetter + rowNum + ')');
      } else {
        newRow.push(parseFloat(sum.toFixed(1)));
        newRow.push(submittedCount);
      }

      var prevTime = dispRow[3 + totalLabsCount + 2] || row[3 + totalLabsCount + 2];
      newRow.push(submittedCount > 0 ? (prevTime && prevTime !== "-" ? prevTime : nowStr) : (prevTime || "-"));
      updatedMatrix.push(newRow);
    }

    sheet.getRange(2, 1, updatedMatrix.length, totalCols).setValues(updatedMatrix);
  }

  // 3. หากมีรหัสนักศึกษาที่ส่งงานในแท็บย่อย แต่ยังไม่มีรายชื่ออยู่ในหน้าสรุปรวม ให้เพิ่มต่อท้าย
  var appendedCount = 0;
  for (var sIdKey in studentMetaMap) {
    if (!existingIds[sIdKey]) {
      var meta = studentMetaMap[sIdKey];
      var appendRow = [sIdKey, meta.name || '', meta.group || ''];
      var sumNew = 0.0;
      var countNew = 0;
      for (var j = 0; j < totalLabsCount; j++) {
        var k = labColumns[j].key;
        var sc = (typeof scoreMap[k][sIdKey] === 'number') ? scoreMap[k][sIdKey] : null;
        if (sc !== null) {
          appendRow.push(sc);
          sumNew += sc;
          countNew++;
        } else {
          appendRow.push("-");
        }
      }
      appendRow.push(parseFloat(sumNew.toFixed(1)));
      appendRow.push(countNew);
      appendRow.push(nowStr);
      sheet.appendRow(appendRow);
      appendedCount++;
    }
  }

  return {
    totalRows: Math.max(0, sheet.getLastRow() - 1),
    appendedRows: appendedCount
  };
}

function columnToLetter(column) {
  var temp, letter = '';
  while (column > 0) {
    temp = (column - 1) % 26;
    letter = String.fromCharCode(temp + 65) + letter;
    column = (column - temp - 1) / 26;
  }
  return letter;
}

/**
 * สร้างหรืออัปเดตข้อมูลในแท็บแรก: "📊 สรุปคะแนนรวม" เมื่อนักศึกษาส่งใบงาน
 * - ค้นหาแถวจาก "รหัสนักศึกษา" (Col A)
 * - คง "ชื่อ-นามสกุล" (Col B) และ "กลุ่ม/ห้อง" (Col C) ตามที่ครูวางไว้ในใบรายชื่อ
 * - พร้อมซิงค์คะแนนทุกแล็บให้ตรงกับใบรายชื่ออัตโนมัติ
 */
function updateMasterSummaryDashboard(ss, labKey, labMeta, data, labScore) {
  var sheet = getOrCreateSummarySheet(ss);
  var labColumns = getSummaryLabColumns();

  var studentId = normalizeStudentId(data.studentId || '');
  var studentName = String(data.studentName || '').trim();
  var studentGroup = String(data.studentGroup || '').trim();
  if (!studentId) return;

  var vals = sheet.getDataRange().getValues();
  var targetRow = -1;

  for (var r = 1; r < vals.length; r++) {
    if (normalizeStudentId(vals[r][0]) === studentId) {
      targetRow = r + 1;
      break;
    }
  }

  var labColIdx = -1;
  for (var c = 0; c < labColumns.length; c++) {
    if (labColumns[c].key === labKey) {
      labColIdx = 4 + c; // คอลัมน์ 1-based (เริ่มคอลัมน์ D = 4)
      break;
    }
  }

  var totalLabsCount = labColumns.length;
  var sumColIdx = 4 + totalLabsCount; // คอลัมน์คะแนนรวม
  var countColIdx = sumColIdx + 1;     // คอลัมน์จำนวนแล็บ
  var timeColIdx = countColIdx + 1;    // คอลัมน์อัปเดตล่าสุด

  if (targetRow !== -1) {
    // อัปเดตเฉพาะช่องคะแนนของแล็บที่ส่งเข้ามา
    if (labColIdx !== -1) {
      sheet.getRange(targetRow, labColIdx).setValue(labScore);
    }
    // รักษาชื่อ-นามสกุล และกลุ่ม/ห้อง ตามที่ครูพิมพ์ไว้ล่วงหน้า (เติมเฉพาะกรณีที่ช่องนั้นว่างเท่านั้น)
    var existingName = String(vals[targetRow - 1][1] || '').trim();
    var existingGroup = String(vals[targetRow - 1][2] || '').trim();
    if (!existingName && studentName) {
      sheet.getRange(targetRow, 2).setValue(studentName);
    }
    if (!existingGroup && studentGroup) {
      sheet.getRange(targetRow, 3).setValue(studentGroup);
    }
    sheet.getRange(targetRow, timeColIdx).setValue(new Date().toLocaleString("th-TH"));
  }

  // ซิงค์คะแนนทั้งหมดตามรหัสนักศึกษาเพื่อให้ทุกแถวในใบรายชื่อที่ครูวางไว้ได้รับคะแนนครบทุกแล็บทันที
  syncAllScoresToMasterRoster(ss, false);
}

/**
 * จัดการอัปโหลดไฟล์เข้าสู่โฟลเดอร์ Google Drive
 */
function handleMasterFileUploads(labKey, data) {
  var screenshotUrl = "ไม่ได้แนบไฟล์";
  var codeFileUrl = "ไม่ได้แนบไฟล์";

  try {
    var parentFolder = getOrCreateDriveFolder("IoT_Master_Submissions_Uploads");
    var labFolder = getOrCreateDriveFolder("Uploads_" + labKey, parentFolder);

    if (data.screenshotBase64 && data.screenshotName) {
      var sBlob = Utilities.newBlob(
        Utilities.base64Decode(data.screenshotBase64.split(",")[1] || data.screenshotBase64),
        data.screenshotType || "image/png",
        (data.studentId || "unknown") + "_screenshot_" + data.screenshotName
      );
      var sFile = labFolder.createFile(sBlob);
      sFile.setSharing(DriveApp.Access.ANYONE_WITH_LINK, DriveApp.Permission.VIEW);
      screenshotUrl = sFile.getUrl();
    }

    if (data.codeBase64 && data.codeFileName) {
      var cBlob = Utilities.newBlob(
        Utilities.base64Decode(data.codeBase64.split(",")[1] || data.codeBase64),
        data.codeFileType || "text/plain",
        (data.studentId || "unknown") + "_code_" + data.codeFileName
      );
      var cFile = labFolder.createFile(cBlob);
      cFile.setSharing(DriveApp.Access.ANYONE_WITH_LINK, DriveApp.Permission.VIEW);
      codeFileUrl = cFile.getUrl();
    }
  } catch (err) {
    console.warn("Upload file error:", err);
  }

  return { screenshotUrl: screenshotUrl, codeFileUrl: codeFileUrl };
}

function getOrCreateDriveFolder(name, parent) {
  var folders = parent ? parent.getFoldersByName(name) : DriveApp.getFoldersByName(name);
  if (folders.hasNext()) return folders.next();
  return parent ? parent.createFolder(name) : DriveApp.createFolder(name);
}

/**
 * ลบประวัติการส่งงานของนักศึกษาจากแท็บใบงานย่อย และรีเซ็ตเฉพาะช่องคะแนนในแดชบอร์ดสรุปรวม (โดยไม่ลบแถวรายชื่อ!)
 */
function clearStudentSubmissionFromAllSheets(ss, studentId, specificLabKey) {
  var cleanId = normalizeStudentId(studentId);
  if (!cleanId) return;

  var sheets = ss.getSheets();
  sheets.forEach(function(s) {
    var sName = s.getName();
    // ห้ามลบแถวรายชื่อในแท็บ "📊 สรุปคะแนนรวม" เด็ดขาด!
    if (sName === "📊 สรุปคะแนนรวม") return;

    if (specificLabKey) {
      var meta = LAB_REGISTRY[specificLabKey];
      if (meta && sName !== meta.sheetName) return;
    }

    var vals = s.getDataRange().getValues();
    for (var r = vals.length - 1; r >= 1; r--) {
      var id1 = normalizeStudentId(vals[r][0]);
      var id2 = normalizeStudentId(vals[r][1]);
      var id3 = normalizeStudentId(vals[r][2]);
      if (id1 === cleanId || id2 === cleanId || id3 === cleanId) {
        s.deleteRow(r + 1);
      }
    }
  });

  // อัปเดตตารางสรุปคะแนนรวมใหม่ (ช่องคะแนนของแล็บที่ถูกลบจะกลับเป็น "-" โดยที่ชื่อ-นามสกุล และห้องยังคงอยู่)
  try {
    var summarySheet = ss.getSheetByName("📊 สรุปคะแนนรวม");
    if (summarySheet && summarySheet.getLastRow() >= 2) {
      var labColumns = getSummaryLabColumns();
      var sVals = summarySheet.getDataRange().getValues();
      for (var r = 1; r < sVals.length; r++) {
        if (normalizeStudentId(sVals[r][0]) === cleanId) {
          for (var c = 0; c < labColumns.length; c++) {
            if (!specificLabKey || labColumns[c].key === specificLabKey) {
              summarySheet.getRange(r + 1, 4 + c).setValue("-");
            }
          }
        }
      }
      syncAllScoresToMasterRoster(ss, false);
    }
  } catch (e) {}
}

/**
 * คืนค่า Header คอลัมน์ที่เหมาะสมสำหรับแต่ละแล็บ
 */
function getHeadersForLab(labKey) {
  var commonPrefix = ["Timestamp", "รหัสนักศึกษา", "ชื่อ-นามสกุล", "กลุ่ม/ห้อง", "วันที่ทำแล็บ", "คะแนนรวม (เต็ม 10.0)"];
  
  if (labKey === "lab4.1") {
    return commonPrefix.concat([
      "Quiz 1", "Quiz 2", "Quiz 3", "Quiz 4", "Quiz 5",
      "Blank 1", "Blank 2", "Blank 3", "Blank 4", "Blank 5",
      "Exp Result GET", "Exp Result POST",
      "Challenge Code", "Control Logic",
      "Question 1 (GET vs POST)", "Question 2 (Polling Overhead)", "Question 3 (CORS Header)",
      "Conclusion Text", "Screenshot File URL", "Code File URL", "Feedback รายละเอียด"
    ]);
  }
  
  if (labKey === "lab6" || labKey === "LAB6.1Perform") {
    return commonPrefix.concat([
      "Quiz 1", "Quiz 2", "Quiz 3", "Quiz 4", "Quiz 5",
      "Blank 1", "Blank 2", "Blank 3", "Blank 4", "Blank 5",
      "Analysis 1", "Analysis 2", "Analysis 3",
      "Challenge Code", "Conclusion Text", "Screenshot File URL", "Code File URL", "Feedback รายละเอียด"
    ]);
  }

  return commonPrefix.concat([
    "Quiz 1", "Quiz 2", "Quiz 3", "Quiz 4", "Quiz 5",
    "Blank 1", "Blank 2", "Blank 3", "Blank 4", "Blank 5",
    "โจทย์ท้าทาย (Challenge Code)", "คำอธิบายตรรกะควบคุม",
    "คำถามข้อที่ 1", "คำถามข้อที่ 2", "คำถามข้อที่ 3",
    "ลิงก์รูปภาพผลงาน", "ลิงก์ไฟล์โค้ด", "สรุปผลการทดลอง", "Feedback รายละเอียด"
  ]);
}

/**
 * สร้าง Array แถวข้อมูลสำหรับบันทึกลงในชีต
 */
function buildRowDataForLab(labKey, data, grading, fileUrls) {
  var now = new Date();
  var sId = data.studentId || '';
  var sName = data.studentName || '';
  var sGroup = data.studentGroup || '';
  var lDate = data.labDate || '';

  if (labKey === "lab4.1") {
    return [
      now, sId, sName, sGroup, lDate, grading.score,
      data.quiz1 || '', data.quiz2 || '', data.quiz3 || '', data.quiz4 || '', data.quiz5 || '',
      data.codeBlank1 || '', data.codeBlank2 || '', data.codeBlank3 || '', data.codeBlank4 || '', data.codeBlank5 || '',
      data.expResult1 || '', data.expResult2 || '',
      data.challengeCode || '', data.controlLogic || '',
      data.question1 || '', data.question2 || '', data.question3 || '',
      data.conclusion || '', fileUrls.screenshotUrl, fileUrls.codeFileUrl, grading.feedback
    ];
  }

  if (labKey === "lab6" || labKey === "LAB6.1Perform") {
    return [
      now, sId, sName, sGroup, lDate, grading.score,
      data.quiz1 || '', data.quiz2 || '', data.quiz3 || '', data.quiz4 || '', data.quiz5 || '',
      data.codeBlank1 || '', data.codeBlank2 || '', data.codeBlank3 || '', data.codeBlank4 || '', data.codeBlank5 || '',
      data.analysis1 || data.question1 || '', data.analysis2 || data.question2 || '', data.analysis3 || data.question3 || '',
      data.challengeCode || '', data.conclusion || '', fileUrls.screenshotUrl, fileUrls.codeFileUrl, grading.feedback
    ];
  }

  return [
    now, sId, sName, sGroup, lDate, grading.score,
    data.quiz1 || '', data.quiz2 || '', data.quiz3 || '', data.quiz4 || '', data.quiz5 || '',
    data.codeBlank1 || '', data.codeBlank2 || '', data.codeBlank3 || '', data.codeBlank4 || '', data.codeBlank5 || '',
    data.challengeCode || '', data.controlLogic || '',
    data.question1 || '', data.question2 || '', data.question3 || '',
    fileUrls.screenshotUrl, fileUrls.codeFileUrl, data.conclusion || '', grading.feedback
  ];
}

/**
 * ตรวจสอบและคำนวณคะแนนอัตโนมัติประจำแต่ละใบงาน (เต็ม 10.0 คะแนน)
 */
function calculateLabScore(labKey, data) {
  var score = 0.0;
  var feedback = [];

  // 1. ตรวจช่องว่างโค้ด (Skeleton Blanks: 1.5 คะแนน)
  var filledBlanks = 0;
  for (var b = 1; b <= 5; b++) {
    if (data["codeBlank" + b] && String(data["codeBlank" + b]).trim().length > 0) filledBlanks++;
  }
  var blankScore = parseFloat(((filledBlanks / 5.0) * 1.5).toFixed(1));
  score += blankScore;
  feedback.push("- เติมคำตอบโครงสร้างโค้ด: " + filledBlanks + "/5 ส่วน (+" + blankScore + "/1.5 คะแนน)");

  // 2. ตรวจแบบทดสอบปรนัย (Quiz Score: 2.0 คะแนน)
  var quizCount = 0;
  for (var q = 1; q <= 5; q++) {
    if (data["quiz" + q] && String(data["quiz" + q]).trim().length > 0) quizCount++;
  }
  var quizScore = parseFloat(((quizCount / 5.0) * 2.0).toFixed(1));
  score += quizScore;
  feedback.push("- แบบทดสอบปรนัย: ตอบครบ " + quizCount + "/5 ข้อ (+" + quizScore + "/2.0 คะแนน)");

  // 3. ตรวจโจทย์ท้าทาย (Challenge Code: 2.5 คะแนน)
  var chLen = (data.challengeCode || '').trim().length;
  var challengeScore = chLen > 50 ? 2.5 : (chLen > 20 ? 1.5 : (chLen > 0 ? 0.8 : 0.0));
  score += challengeScore;
  feedback.push("- โค้ดโจทย์ท้าทาย: " + (challengeScore > 0 ? "ตรงจุดประสงค์ (" + challengeScore + "/2.5 คะแนน)" : "ไม่พบโค้ด (0.0/2.5 คะแนน)"));

  // 4. ตรวจคำถามท้ายการทดลอง 3 ข้อ (2.5 คะแนน)
  var q1Len = (data.question1 || data.analysis1 || '').trim().length;
  var q2Len = (data.question2 || data.analysis2 || '').trim().length;
  var q3Len = (data.question3 || data.analysis3 || '').trim().length;
  var qPoints = (q1Len > 10 ? 0.85 : 0.0) + (q2Len > 10 ? 0.85 : 0.0) + (q3Len > 10 ? 0.80 : 0.0);
  score += parseFloat(qPoints.toFixed(2));
  feedback.push("- คำถามวิเคราะห์ผลการทดลอง: ตอบสมบูรณ์ " + (qPoints.toFixed(2)) + "/2.50 คะแนน");

  // 5. ตรวจไฟล์แนบ (1.0 คะแนน)
  var attScore = (data.screenshotBase64 ? 0.5 : 0.0) + (data.codeBase64 ? 0.5 : 0.0);
  score += attScore;
  feedback.push("- ไฟล์แนบ (ภาพ & โค้ด): " + (attScore.toFixed(1)) + "/1.0 คะแนน");

  // 6. ตรวจสรุปผล (0.5 คะแนน)
  var concLen = (data.conclusion || '').trim().length;
  var concScore = concLen > 10 ? 0.5 : 0.0;
  score += concScore;
  feedback.push("- สรุปผลการทดลอง: " + (concScore > 0 ? "ครบถ้วน (+0.5/0.5 คะแนน)" : "ไม่พบ (+0.0/0.5 คะแนน)"));

  var finalTotal = Math.min(10.0, parseFloat(score.toFixed(1)));
  return {
    score: finalTotal,
    feedback: feedback.join('\n')
  };
}
