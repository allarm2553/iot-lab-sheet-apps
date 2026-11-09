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
  try {
    return SpreadsheetApp.getActiveSpreadsheet();
  } catch (err) {
    return null;
  }
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

    // 1. ตรวจสอบการ Ping / ทดสอบเชื่อมต่อ
    if (data.action === 'testConnection' || data.action === 'ping' || data.test === true) {
      return ContentService.createTextOutput(JSON.stringify({
        status: 'success',
        type: 'master',
        message: 'เชื่อมต่อ Universal Master Google Sheet: "' + ss.getName() + '" สำเร็จพร้อมบันทึกคะแนน!',
        spreadsheetName: ss.getName(),
        spreadsheetUrl: ss.getUrl()
      })).setMimeType(ContentService.MimeType.JSON);
    }

    // 2. ปลดล็อกล้างประวัติส่งซ้ำ (กรณีครูกดเคลียร์แคช)
    if (data.action === 'clearStudentLock' || data.action === 'clearStudent') {
      var sId = String(data.studentId || '').trim();
      if (sId) {
        clearStudentSubmissionFromAllSheets(ss, sId, data.labId);
      }
      return ContentService.createTextOutput(JSON.stringify({
        status: 'success',
        message: 'ปลดล็อกและลบประวัติเดิมของรหัส ' + sId + ' ใน Google Sheets เรียบร้อยแล้ว'
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

/**
 * สร้างหรืออัปเดตข้อมูลในแท็บแรก: "📊 Dashboard สรุปคะแนนรวม"
 */
function updateMasterSummaryDashboard(ss, labKey, labMeta, data, labScore) {
  var summarySheetName = "📊 สรุปคะแนนรวม";
  var sheet = ss.getSheetByName(summarySheetName);

  var labColumns = [
    { key: "lab0", label: "Lab 0" },
    { key: "lab0-basic", label: "Lab 0 Basic" },
    { key: "lab1", label: "Lab 1" },
    { key: "lab1.1", label: "Lab 1.1" },
    { key: "lab2", label: "Lab 2" },
    { key: "lab3", label: "Lab 3" },
    { key: "lab3.1", label: "Lab 3.1" },
    { key: "lab3.2_Wifi_UI_config", label: "Lab 3.2" },
    { key: "lab4", label: "Lab 4" },
    { key: "lab4.1", label: "Lab 4.1" },
    { key: "lab5", label: "Lab 5" },
    { key: "LAB5_Dev", label: "Lab 5 Dev" },
    { key: "lab6", label: "Lab 6" },
    { key: "LAB6.1Perform", label: "Lab 6.1" },
    { key: "lab7", label: "Lab 7" },
    { key: "lab8", label: "Lab 8" },
    { key: "lab9", label: "Lab 9" },
    { key: "lab-extra", label: "Lab Extra" },
    { key: "lab-webconfig_wifi", label: "WebConfig" }
  ];

  if (!sheet) {
    sheet = ss.insertSheet(summarySheetName, 0); // แทรกเป็นแท็บแรกสุด
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

    // ลบแท็บว่างเปล่าเริ่มต้น (เช่น ชีต1 หรือ Sheet1) ที่ติดมากับสเปรดชีตใหม่
    var defaultEmpty = ss.getSheetByName("ชีต1") || ss.getSheetByName("Sheet1");
    if (defaultEmpty && defaultEmpty.getLastRow() === 0 && ss.getSheets().length > 1) {
      try { ss.deleteSheet(defaultEmpty); } catch (e) {}
    }
  }

  var studentId = String(data.studentId || '').trim();
  var studentName = data.studentName || '';
  var studentGroup = data.studentGroup || '';
  if (!studentId) return;

  var vals = sheet.getDataRange().getValues();
  var targetRow = -1;

  for (var r = 1; r < vals.length; r++) {
    if (String(vals[r][0] || '').trim() === studentId) {
      targetRow = r + 1;
      break;
    }
  }

  // หา Index ของคอลัมน์แล็บนี้
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

  if (targetRow === -1) {
    // สร้างแถวใหม่สำหรับนักศึกษาคนนี้
    var newRow = [studentId, studentName, studentGroup];
    for (var i = 0; i < totalLabsCount; i++) {
      if (labColumns[i].key === labKey) {
        newRow.push(labScore);
      } else {
        newRow.push("-");
      }
    }
    newRow.push(labScore); // คะแนนรวม
    newRow.push(1);        // ส่ง 1 แล็บ
    newRow.push(new Date().toLocaleString("th-TH"));

    sheet.appendRow(newRow);
  } else {
    // อัปเดตแถวเดิมที่มีอยู่
    if (labColIdx !== -1) {
      sheet.getRange(targetRow, labColIdx).setValue(labScore);
    }
    sheet.getRange(targetRow, 2).setValue(studentName);
    sheet.getRange(targetRow, 3).setValue(studentGroup);
    sheet.getRange(targetRow, timeColIdx).setValue(new Date().toLocaleString("th-TH"));

    // คำนวณคะแนนรวมและจำนวนแล็บใหม่
    var rowScores = sheet.getRange(targetRow, 4, 1, totalLabsCount).getValues()[0];
    var sum = 0.0;
    var submittedCount = 0;
    for (var s = 0; s < rowScores.length; s++) {
      var val = parseFloat(rowScores[s]);
      if (!isNaN(val)) {
        sum += val;
        submittedCount++;
      }
    }
    sheet.getRange(targetRow, sumColIdx).setValue(parseFloat(sum.toFixed(1)));
    sheet.getRange(targetRow, countColIdx).setValue(submittedCount);
  }
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
 * ลบประวัติการส่งงานของนักศึกษาจากแท็บใบงานและแดชบอร์ด
 */
function clearStudentSubmissionFromAllSheets(ss, studentId, specificLabKey) {
  var sheets = ss.getSheets();
  sheets.forEach(function(s) {
    if (specificLabKey) {
      var meta = LAB_REGISTRY[specificLabKey];
      if (meta && s.getName() !== meta.sheetName && s.getName() !== "📊 สรุปคะแนนรวม") return;
    }
    var vals = s.getDataRange().getValues();
    for (var r = vals.length - 1; r >= 1; r--) {
      var id1 = String(vals[r][0] || '').trim().replace(/^'/, '');
      var id2 = String(vals[r][1] || '').trim().replace(/^'/, '');
      var id3 = String(vals[r][2] || '').trim().replace(/^'/, '');
      if (id1 === studentId || id2 === studentId || id3 === studentId) {
        s.deleteRow(r + 1);
      }
    }
  });
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
