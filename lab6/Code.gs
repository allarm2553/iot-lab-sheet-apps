/**
 * Web App for Lab 6: Hybrid Dual-Mode IoT Node (Local WebSockets & Cloud MQTT)
 * Designed by Antigravity AI (Auto-Grading Version)
 */

function doGet(e) {
  try {
    return HtmlService.createTemplateFromFile('index')
      .evaluate()
      .setTitle('ใบงานที่ 6: การบูรณาการระบบ Hybrid Dual-Mode IoT Node (Local WebSockets & Cloud MQTT)')
      .addMetaTag('viewport', 'width=device-width, initial-scale=1')
      .setXFrameOptionsMode(HtmlService.XFrameOptionsMode.ALLOWALL);
  } catch (err) {
    // กรณีผู้ใช้นำเฉพาะ Code.gs ไปวางใน Google Apps Script เพื่อใช้เป็น API เชื่อมต่อ Google Sheets
    if (e && e.parameter && (e.parameter.action === 'testConnection' || e.parameter.action === 'ping' || e.parameter.format === 'json')) {
      return ContentService.createTextOutput(JSON.stringify({
        status: 'success',
        message: 'เชื่อมต่อ Google Apps Script (Lab 6) สำเร็จพร้อมใช้งาน!'
      })).setMimeType(ContentService.MimeType.JSON);
    }

    // เคลียร์ประวัติการส่งงานเพื่อให้นักเรียนส่งใหม่ได้ (กรณีครูผู้สอนกดปลดล็อก)
    if (data.action === 'clearStudentLock' || data.action === 'clearStudent') {
      var sId = String(data.studentId || '').trim();
      if (sId) {
        var targetSheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
        var vals = targetSheet.getDataRange().getValues();
        for (var r = 1; r < vals.length; r++) {
          if (String(vals[r][1] || '').trim().replace(/^'/, '') === sId || String(vals[r][2] || '').trim().replace(/^'/, '') === sId) {
            targetSheet.deleteRow(r + 1);
            break;
          }
        }
      }
      return ContentService.createTextOutput(JSON.stringify({
        status: 'success',
        message: 'ปลดล็อกและลบประวัติเดิมใน Google Sheets เรียบร้อยแล้ว'
      })).setMimeType(ContentService.MimeType.JSON);
    }


    var html = '<!DOCTYPE html><html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1">' +
      '<title>IoT Lab 6 Web App - พร้อมใช้งาน</title>' +
      '<style>' +
      'body{font-family:system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,sans-serif;background:#0b1120;color:#f8fafc;display:flex;align-items:center;justify-content:center;min-height:100vh;margin:0;padding:1.25rem;box-sizing:border-box;}' +
      '.card{background:#1e293b;border:1px solid #334155;padding:2.25rem 2rem;border-radius:16px;text-align:center;max-width:540px;box-shadow:0 20px 25px -5px rgba(0,0,0,0.5);}' +
      '.badge{background:rgba(16,185,129,0.15);color:#34d399;border:1px solid rgba(16,185,129,0.4);padding:0.4rem 1rem;border-radius:9999px;font-size:0.875rem;font-weight:600;display:inline-block;margin-bottom:1.25rem;}' +
      'h2{color:#38bdf8;margin:0 0 0.75rem 0;font-size:1.4rem;font-weight:700;}p{color:#94a3b8;line-height:1.6;margin:0 0 1.25rem 0;font-size:0.95rem;}' +
      '.info{background:rgba(15,23,42,0.8);border:1px solid #334155;border-radius:10px;padding:1rem;font-size:0.875rem;color:#cbd5e1;text-align:left;line-height:1.5;}' +
      '</style></head><body><div class="card">' +
      '<div class="badge">✓ เชื่อมต่อสำเร็จ (Active)</div>' +
      '<h2>Google Apps Script API พร้อมทำงาน</h2>' +
      '<p>เว็บเซอร์วิสของ <strong>ใบงานที่ 6: การบูรณาการระบบ Hybrid Dual-Mode IoT Node (Local WebSockets & Cloud MQTT)</strong> เชื่อมต่อกับ Google Sheets เรียบร้อยแล้ว พร้อมรับผลการประเมินและบันทึกคะแนนจากหน้าเว็บใบงาน</p>' +
      '<div class="info">💡 <strong>คำแนะนำ:</strong> การเชื่อมต่อสมบูรณ์แล้ว ท่านสามารถปิดแท็บนี้และกลับไปที่หน้าใบงานปฏิบัติการเพื่อทำการทดลองและกดส่งรายงานได้ตามปกติครับ</div>' +
      '</div></body></html>';
    return HtmlService.createHtmlOutput(html)
      .setTitle('IoT Lab 6 Web App - พร้อมใช้งาน')
      .addMetaTag('viewport', 'width=device-width, initial-scale=1');
  }
}

const ANSWER_KEY = {
  codeBlank1: ["81", "81.0", "webSocketPort", "WEBSOCKETS_PORT"],
  codeBlank2: ["espClient", "&espClient", "WiFiClient", "client"],
  codeBlank3: ["esp32-climate-node/state", "esp32-node/state", "esp-node/state", "pubTopic", "pub_topic", "\"esp32-climate-node/state\""],
  codeBlank4: ["webSocketEvent", "onWebSocketEvent", "webSocketCallback", "&webSocketEvent"],
  codeBlank5: ["esp32-climate-node/control/cmd", "esp32-node/control/cmd", "esp-node/control/cmd", "subTopic", "sub_topic", "\"esp32-climate-node/control/cmd\""],
  quiz1: ["1B", "B", "1b", "b"],
  quiz2: ["2C", "C", "2c", "c"],
  quiz3: ["3C", "C", "3c", "c"],
  quiz4: ["4B", "B", "4b", "b"],
  quiz5: ["5B", "B", "5b", "b"]
};

function gradeSubmission(data) {
  var score = 0;
  var breakdown = {};

  for (var i = 1; i <= 5; i++) {
    var key = "codeBlank" + i;
    var userAns = (data[key] || "").toString().trim().toLowerCase().replace(/['"\s;]/g, '');
    var validList = ANSWER_KEY[key].map(function(v) { return v.toLowerCase().replace(/['"\s;]/g, ''); });
    var isCorrect = validList.some(function(v) { return userAns === v || userAns.indexOf(v) !== -1; });
    if (isCorrect) {
      score += 1;
      breakdown[key] = { correct: true, points: 1 };
    } else {
      breakdown[key] = { correct: false, points: 0 };
    }
  }

  for (var j = 1; j <= 5; j++) {
    var qKey = "quiz" + j;
    var qAns = (data[qKey] || "").toString().trim().toLowerCase();
    var validQuiz = ANSWER_KEY[qKey].map(function(v) { return v.toString().toLowerCase(); });
    if (validQuiz.indexOf(qAns) !== -1) {
      score += 1;
      breakdown[qKey] = { correct: true, points: 1 };
    } else {
      breakdown[qKey] = { correct: false, points: 0 };
    }
  }

  return {
    score: score,
    total: 10,
    breakdown: breakdown
  };
}

function processForm(formData) {
  try {
    var grading = gradeSubmission(formData);
    var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
    
    var timestamp = new Date();
    var name = formData.studentName || '';
    var studentId = formData.studentId || '';
    var group = formData.studentGroup || '';

    // ตรวจสอบการส่งซ้ำ (Duplicate Submission Check)
    var existingData = sheet.getDataRange().getValues();
    var targetId = String(studentId).trim().replace(/^'/, '');
    if (targetId) {
      for (var r = 1; r < existingData.length; r++) {
        var existingId = String(existingData[r][1] || '').trim().replace(/^'/, '');
        if (existingId === targetId) {
        if (!data.allowResubmit) {
          return {
            status: 'duplicate',
            message: '❌ รหัสนักศึกษา ' + targetId + ' ได้ส่งใบงานนี้ไปแล้ว ไม่สามารถส่งซ้ำได้'
          };
        }
        sheet.deleteRow(r + 1);
        break;
      }
      }
    }
    
    sheet.appendRow([
      timestamp,
      studentId,
      name,
      group,
      grading.score,
      JSON.stringify(grading.breakdown),
      formData.analysis1 || '',
      formData.analysis2 || '',
      formData.analysis3 || '',
      formData.challengeCode || '',
      formData.conclusion || ''
    ]);

    return {
      status: 'success',
      score: grading.score,
      total: 10,
      message: 'บันทึกคะแนนและส่งใบงานปฏิบัติการที่ 6 เรียบร้อยแล้ว!'
    };
  } catch (err) {
    return {
      status: 'error',
      message: 'เกิดข้อผิดพลาดในการบันทึกข้อมูล: ' + err.toString()
    };
  }
}


function doPost(e) {
  var lock = LockService.getScriptLock();
  lock.tryLock(15000);

  try {
    var raw = (e && e.postData && e.postData.contents) ? e.postData.contents : '{}';
    var data = JSON.parse(raw);

    // ตรวจสอบการ Ping / ทดสอบเชื่อมต่อ
    if (data.action === 'testConnection' || data.action === 'ping' || data.test === true) {
      return ContentService.createTextOutput(JSON.stringify({
        status: 'success',
        message: 'เชื่อมต่อ Google Apps Script (Lab 6) สำเร็จพร้อมบันทึกคะแนน!'
      })).setMimeType(ContentService.MimeType.JSON);
    }

    var result = processForm(data);
    return ContentService.createTextOutput(JSON.stringify(result))
      .setMimeType(ContentService.MimeType.JSON);
  } catch (err) {
    return ContentService.createTextOutput(JSON.stringify({
      status: 'error',
      message: 'เกิดข้อผิดพลาดในการประมวลผล: ' + err.toString()
    })).setMimeType(ContentService.MimeType.JSON);
  } finally {
    lock.releaseLock();
  }
}
