/**
 * Web App for Lab 1.1: Digital Inputs, Software Debounce & Relays
 * Designed by Antigravity AI (Auto-Grading Version with Multiple Choice Quiz & Diagnostics)
 */

// =========================================================================
// ⚙️ การตั้งค่า Google Spreadsheet ปลายทาง (Spreadsheet Configuration)
// =========================================================================
// • หากสร้างสคริปต์นี้จาก Google Sheets โดยตรง (ส่วนขยาย > Apps Script) สามารถเว้นว่างไว้ได้
// • หากสร้างเป็น Standalone Script หรือต้องการเจาะจงให้บันทึกลงชีตไฟล์อื่น ให้ใส่ Spreadsheet ID หรือ URL ด้านล่างนี้:
var TARGET_SPREADSHEET_ID = "";

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
      console.warn("ไม่สามารถเปิด Spreadsheet ตาม TARGET_SPREADSHEET_ID ได้:", e);
    }
  }
  try {
    return SpreadsheetApp.getActiveSpreadsheet();
  } catch (err) {
    return null;
  }
}


function doGet(e) {
  try {
    return HtmlService.createTemplateFromFile('index')
      .evaluate()
      .setTitle('ใบงานที่ 1.1: การควบคุมเอาต์พุตด้วยอินพุตสวิตช์ปุ่มกด')
      .addMetaTag('viewport', 'width=device-width, initial-scale=1')
      .setXFrameOptionsMode(HtmlService.XFrameOptionsMode.ALLOWALL);
  } catch (err) {
    // กรณีผู้ใช้นำเฉพาะ Code.gs ไปวางใน Google Apps Script เพื่อใช้เป็น API เชื่อมต่อ Google Sheets
    if (e && e.parameter && (e.parameter.action === 'testConnection' || e.parameter.action === 'ping' || e.parameter.format === 'json')) {
      return ContentService.createTextOutput(JSON.stringify({
        status: 'success',
        message: 'เชื่อมต่อ Google Apps Script (Lab 1.1) สำเร็จพร้อมใช้งาน!'
      })).setMimeType(ContentService.MimeType.JSON);
    }

    // เคลียร์ประวัติการส่งงานเพื่อให้นักเรียนส่งใหม่ได้ (กรณีครูผู้สอนกดปลดล็อก)
    if (data.action === 'clearStudentLock' || data.action === 'clearStudent') {
      var sId = String(data.studentId || '').trim();
      if (sId) {
        var targetSs = getTargetSpreadsheet(); var targetSheet = targetSs ? targetSs.getActiveSheet() : null;
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
      '<title>IoT Lab 1.1 Web App - พร้อมใช้งาน</title>' +
      '<style>' +
      'body{font-family:system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,sans-serif;background:#0b1120;color:#f8fafc;display:flex;align-items:center;justify-content:center;min-height:100vh;margin:0;padding:1.25rem;box-sizing:border-box;}' +
      '.card{background:#1e293b;border:1px solid #334155;padding:2.25rem 2rem;border-radius:16px;text-align:center;max-width:540px;box-shadow:0 20px 25px -5px rgba(0,0,0,0.5);}' +
      '.badge{background:rgba(16,185,129,0.15);color:#34d399;border:1px solid rgba(16,185,129,0.4);padding:0.4rem 1rem;border-radius:9999px;font-size:0.875rem;font-weight:600;display:inline-block;margin-bottom:1.25rem;}' +
      'h2{color:#38bdf8;margin:0 0 0.75rem 0;font-size:1.4rem;font-weight:700;}p{color:#94a3b8;line-height:1.6;margin:0 0 1.25rem 0;font-size:0.95rem;}' +
      '.info{background:rgba(15,23,42,0.8);border:1px solid #334155;border-radius:10px;padding:1rem;font-size:0.875rem;color:#cbd5e1;text-align:left;line-height:1.5;}' +
      '</style></head><body><div class="card">' +
      '<div class="badge">✓ เชื่อมต่อสำเร็จ (Active)</div>' +
      '<h2>Google Apps Script API พร้อมทำงาน</h2>' +
      '<p>เว็บเซอร์วิสของ <strong>ใบงานที่ 1.1: การควบคุมเอาต์พุตด้วยอินพุตสวิตช์ปุ่มกด</strong> เชื่อมต่อกับ Google Sheets เรียบร้อยแล้ว พร้อมรับผลการประเมินและบันทึกคะแนนจากหน้าเว็บใบงาน</p>' +
      '<div class="info">💡 <strong>คำแนะนำ:</strong> การเชื่อมต่อสมบูรณ์แล้ว ท่านสามารถปิดแท็บนี้และกลับไปที่หน้าใบงานปฏิบัติการเพื่อทำการทดลองและกดส่งรายงานได้ตามปกติครับ</div>' +
      '</div></body></html>';
    return HtmlService.createHtmlOutput(html)
      .setTitle('IoT Lab 1.1 Web App - พร้อมใช้งาน')
      .addMetaTag('viewport', 'width=device-width, initial-scale=1');
  }
}

// Auto-grading logic for Lab 1.1
function gradeSubmission(data) {
  var blankKeywords = ["INPUT_PULLUP", "OUTPUT", "LOW", "!relayState|!fanState|!ledState", "LOW|LOW|RELAY_OFF"];
  var challengeKeywords = ["0|d3|12|BUTTON_PIN", "18|d5|5|13|d7|RELAY_PIN|FAN_RELAY_PIN", "14|23|16|d0|MIST_RELAY_PIN", "count|toggleCount", "3", "longPressTriggered", "digitalWrite", "millis"];
  
  var q1Keywords = ["floating|สัญญาณลอย|ไม่นิ่ง|คลื่นรบกวน", "input_pullup|pull-up|pullup|ดึงขึ้น", "gnd|3.3v|ความต้านทาน"];
  var q2Keywords = ["blocking|บล็อก|ค้าง|หยุดรอ", "non-blocking|millis|ต่อเนื่อง|ไม่สะดุด", "wifi|สื่อสาร|ตอบสนอง|watchdog"];
  var q3Keywords = ["edge-triggered|ขอบสัญญาณ|ระดับสัญญาณ|level", "accidental|เผลอ|พลาด|อุบัติเหตุ", "reset|safe state|ความปลอดภัย"];
  
  // เฉลยแบบทดสอบแบบเลือกตอบ 5 ข้อ (Quiz Answer Keys)
  var quizKeys = {
    quiz1: "1c", // ยังไม่กดเป็น HIGH (3.3V), เมื่อกดเป็น LOW (0V)
    quiz2: "2a", // หน้าสัมผัสสปริงตัวเด้งสั่น 5-20ms ทำให้ MCU ตรวจจับกดซ้ำ
    quiz3: "3b", // delay() บล็อก CPU ขัดขวางเซ็นเซอร์และ WiFi
    quiz4: "4b", // Falling edge: lastState HIGH -> currentState LOW
    quiz5: "5d"  // ป้องกัน Reset trigger ซ้ำๆ ทุกรอบลูปขณะกดค้าง
  };

  var skeletonScore = 0.0;
  var quizScore = 0.0;
  var challengeScore = 0.0;
  var q1Score = 0.0;
  var q2Score = 0.0;
  var q3Score = 0.0;
  var attachmentScore = 0.0;
  var feedbackDetails = [];

  // 1. ตรวจช่องว่างโครงร่างโค้ด (1.5 คะแนน)
  var codeContent = (data.codeBlank1 || '') + ' ' + (data.codeBlank2 || '') + ' ' + (data.codeBlank3 || '') + ' ' + (data.codeBlank4 || '') + ' ' + (data.codeBlank5 || '');
  if (codeContent.replace(/\s+/g, '').length > 0) {
    var matchedBlanks = 0;
    for (var i = 0; i < blankKeywords.length; i++) {
      var subKws = blankKeywords[i].split('|');
      var isMatched = false;
      for (var j = 0; j < subKws.length; j++) {
        if (codeContent.toLowerCase().indexOf(subKws[j].toLowerCase()) !== -1) {
          isMatched = true;
          break;
        }
      }
      if (isMatched) matchedBlanks++;
    }
    skeletonScore = (matchedBlanks / blankKeywords.length) * 1.5;
    feedbackDetails.push("- เติมคำตอบโครงร่างโค้ด: ถูกต้องตรงประเด็น " + matchedBlanks + "/" + blankKeywords.length + " ส่วนหลัก (+" + skeletonScore.toFixed(1) + "/1.5 คะแนน)");
  } else {
    feedbackDetails.push("- เติมคำตอบโครงร่างโค้ด: ไม่พบการส่งคำตอบ (+0.0/1.5 คะแนน)");
  }

  // 2. ตรวจแบบทดสอบแบบเลือกตอบ (2.0 คะแนน: ข้อละ 0.4)
  var correctQuizCount = 0;
  var quizAnswers = [data.quiz1, data.quiz2, data.quiz3, data.quiz4, data.quiz5];
  var expectedKeys = [quizKeys.quiz1, quizKeys.quiz2, quizKeys.quiz3, quizKeys.quiz4, quizKeys.quiz5];
  for (var k = 0; k < expectedKeys.length; k++) {
    if (quizAnswers[k] && quizAnswers[k].trim() === expectedKeys[k]) {
      correctQuizCount++;
    }
  }
  quizScore = (correctQuizCount / 5.0) * 2.0;
  feedbackDetails.push("- แบบทดสอบแบบเลือกตอบ: ถูกต้อง " + correctQuizCount + "/5 ข้อ (+" + quizScore.toFixed(1) + "/2.0 คะแนน)");

  // 3. ตรวจโค้ดโจทย์ท้าทาย (2.5 คะแนน)
  var challengeCodeText = data.challengeCode || '';
  if (challengeCodeText.trim().length > 0) {
    var matchedChallenge = 0;
    for (var i = 0; i < challengeKeywords.length; i++) {
      var subKws = challengeKeywords[i].split('|');
      var isMatched = false;
      for (var j = 0; j < subKws.length; j++) {
        if (challengeCodeText.toLowerCase().indexOf(subKws[j].toLowerCase()) !== -1) {
          isMatched = true;
          break;
        }
      }
      if (isMatched) matchedChallenge++;
    }
    challengeScore = (matchedChallenge / challengeKeywords.length) * 2.5;
    feedbackDetails.push("- โจทย์ท้าทาย (Challenge Code): ตรงตรรกะ & Long Press " + matchedChallenge + "/" + challengeKeywords.length + " จุดหลัก (+" + challengeScore.toFixed(1) + "/2.5 คะแนน)");
  } else {
    feedbackDetails.push("- โจทย์ท้าทาย (Challenge Code): ไม่พบการส่งโค้ดคำตอบ (+0.0/2.5 คะแนน)");
  }

  // 4. ตรวจคำถามท้ายการทดลอง 3 ข้อ (2.5 คะแนน)
  var q1Text = data.question1 || '';
  if (q1Text.trim().length > 0) {
    var matchedQ1 = 0;
    for (var i = 0; i < q1Keywords.length; i++) {
      var subKws = q1Keywords[i].split('|');
      var isMatched = false;
      for (var j = 0; j < subKws.length; j++) {
        if (q1Text.toLowerCase().indexOf(subKws[j].toLowerCase()) !== -1) {
          isMatched = true;
          break;
        }
      }
      if (isMatched) matchedQ1++;
    }
    q1Score = (matchedQ1 / q1Keywords.length) * 0.85;
    feedbackDetails.push("- คำถามข้อ 1 (Floating Input & Pull-up): ตรงจุดสำคัญ " + matchedQ1 + "/" + q1Keywords.length + " จุด (+" + q1Score.toFixed(2) + "/0.85 คะแนน)");
  } else {
    feedbackDetails.push("- คำถามข้อ 1: ไม่พบการตอบคำถาม (+0.0/0.85 คะแนน)");
  }

  var q2Text = data.question2 || '';
  if (q2Text.trim().length > 0) {
    var matchedQ2 = 0;
    for (var i = 0; i < q2Keywords.length; i++) {
      var subKws = q2Keywords[i].split('|');
      var isMatched = false;
      for (var j = 0; j < subKws.length; j++) {
        if (q2Text.toLowerCase().indexOf(subKws[j].toLowerCase()) !== -1) {
          isMatched = true;
          break;
        }
      }
      if (isMatched) matchedQ2++;
    }
    q2Score = (matchedQ2 / q2Keywords.length) * 0.85;
    feedbackDetails.push("- คำถามข้อ 2 (Debounce Delay vs Millis): ตรงจุดสำคัญ " + matchedQ2 + "/" + q2Keywords.length + " จุด (+" + q2Score.toFixed(2) + "/0.85 คะแนน)");
  } else {
    feedbackDetails.push("- คำถามข้อ 2: ไม่พบการตอบคำถาม (+0.0/0.85 คะแนน)");
  }

  var q3Text = data.question3 || '';
  if (q3Text.trim().length > 0) {
    var matchedQ3 = 0;
    for (var i = 0; i < q3Keywords.length; i++) {
      var subKws = q3Keywords[i].split('|');
      var isMatched = false;
      for (var j = 0; j < subKws.length; j++) {
        if (q3Text.toLowerCase().indexOf(subKws[j].toLowerCase()) !== -1) {
          isMatched = true;
          break;
        }
      }
      if (isMatched) matchedQ3++;
    }
    q3Score = (matchedQ3 / q3Keywords.length) * 0.80;
    feedbackDetails.push("- คำถามข้อ 3 (Edge-Triggered & Safety Reset): ตรงจุดสำคัญ " + matchedQ3 + "/" + q3Keywords.length + " จุด (+" + q3Score.toFixed(2) + "/0.80 คะแนน)");
  } else {
    feedbackDetails.push("- คำถามข้อ 3: ไม่พบการตอบคำถาม (+0.0/0.80 คะแนน)");
  }

  // 5. ตรวจไฟล์แนบ (1.0 คะแนน)
  var screenshotOk = (data.screenshotBase64 && data.screenshotName) ? 0.5 : 0.0;
  var codeOk = (data.codeBase64 && data.codeFileName) ? 0.5 : 0.0;
  attachmentScore = screenshotOk + codeOk;
  feedbackDetails.push("- ไฟล์แนบ: แนบรูปภาพ " + (screenshotOk ? "แล้ว" : "ไม่พบ") + ", แนบไฟล์โค้ด " + (codeOk ? "แล้ว" : "ไม่พบ") + " (+" + attachmentScore.toFixed(1) + "/1.0 คะแนน)");

  // 6. สรุปผลการทดลอง (0.5 คะแนน)
  var conclusionScore = (data.conclusion && data.conclusion.trim().length > 10) ? 0.5 : 0.0;
  feedbackDetails.push("- สรุปผลการทดลอง: " + (conclusionScore > 0 ? "สมบูรณ์ (+0.5/0.5 คะแนน)" : "ไม่พบ (+0.0/0.5 คะแนน)"));

  var finalScore = parseFloat((skeletonScore + quizScore + challengeScore + q1Score + q2Score + q3Score + attachmentScore + conclusionScore).toFixed(1));

  return {
    score: finalScore,
    feedback: feedbackDetails.join('\n')
  };
}

function submitLabData(data) {
  try {
    // 1. Open the active spreadsheet
    var ss = getTargetSpreadsheet();
    var sheetName = "Lab 1.1 Submissions";
    var sheet = ss.getSheetByName(sheetName);
    
    // Auto-grading calculation
    var grading = gradeSubmission(data);
    
    // Auto-create sheet if it doesn't exist
    if (!sheet) {
      var allSheets = ss.getSheets();
      if (allSheets.length === 1 && allSheets[0].getLastRow() === 0) {
        sheet = allSheets[0];
        sheet.setName(sheetName);
      } else {
        sheet = ss.insertSheet(sheetName);
      }
      var headers = [
        "Timestamp", "ชื่อ-นามสกุล", "รหัสนักศึกษา", "กลุ่ม/ห้อง", "วันที่ทำการทดลอง",
        "โค้ดโจทย์ท้าทาย (Challenge Code)", "คำอธิบายตรรกะควบคุม",
        "คำตอบ Code ช่องที่ 1", "คำตอบ Code ช่องที่ 2", "คำตอบ Code ช่องที่ 3", "คำตอบ Code ช่องที่ 4", "คำตอบ Code ช่องที่ 5",
        "Quiz 1 (Pullup Logic)", "Quiz 2 (Contact Bounce)", "Quiz 3 (Delay vs Millis)", "Quiz 4 (Falling Edge)", "Quiz 5 (Long Press Lock)",
        "คำถามข้อที่ 1 (Floating Input & Pull-up)", "คำถามข้อที่ 2 (Debounce Architecture)", "คำถามข้อที่ 3 (Edge-Triggered & Safety Reset)",
        "ลิงก์ไฟล์รูปภาพผลการทดลอง", "ลิงก์ไฟล์โค้ด (.ino/.zip)", "สรุปผลการทดลอง",
        "คะแนนประเมิน (เต็ม 10)", "ข้อเสนอแนะอัตโนมัติ"
      ];
      sheet.appendRow(headers);
      sheet.getRange(1, 1, 1, headers.length).setFontWeight("bold").setBackground("#e2e8f0");
      sheet.setFrozenRows(1);
    }
    
    // 2. Handle File Uploads (Drive Storage)
    var screenshotUrl = "ไม่ได้แนบไฟล์";
    var codeFileUrl = "ไม่ได้แนบไฟล์";
    
    // Auto-create folders for uploads
    var folderName = "Lab 1.1 Attachments";
    var folders = DriveApp.getFoldersByName(folderName);
    var folder;
    if (folders.hasNext()) {
      folder = folders.next();
    } else {
      folder = DriveApp.createFolder(folderName);
    }
    
    // Process screenshot
    if (data.screenshotBase64 && data.screenshotName) {
      var screenshotBlob = Utilities.newBlob(
        Utilities.base64Decode(data.screenshotBase64.split(",")[1]),
        data.screenshotType,
        data.studentId + "_" + data.studentName.replace(/\s+/g, '_') + "_screenshot_" + data.screenshotName
      );
      var file = folder.createFile(screenshotBlob);
      file.setSharing(DriveApp.Access.ANYONE_WITH_LINK, DriveApp.Permission.VIEW);
      screenshotUrl = file.getUrl();
    }
    
    // Process code file
    if (data.codeBase64 && data.codeFileName) {
      var codeBlob = Utilities.newBlob(
        Utilities.base64Decode(data.codeBase64.split(",")[1]),
        data.codeFileType,
        data.studentId + "_" + data.studentName.replace(/\s+/g, '_') + "_code_" + data.codeFileName
      );
      var file = folder.createFile(codeBlob);
      file.setSharing(DriveApp.Access.ANYONE_WITH_LINK, DriveApp.Permission.VIEW);
      codeFileUrl = file.getUrl();
    }
    
    // 3. ตรวจสอบการส่งซ้ำ (Duplicate Submission Check)
    var existingData = sheet.getDataRange().getValues();
    for (var r = 1; r < existingData.length; r++) {
      if (String(existingData[r][2]).trim() === String(data.studentId).trim()) {
        if (!data.allowResubmit) {
          return {
            status: "duplicate",
            message: "❌ รหัสนักศึกษา " + data.studentId + " ได้ส่งใบงานนี้ไปแล้ว ไม่สามารถส่งซ้ำได้"
          };
        }
        sheet.deleteRow(r + 1);
        break;
      }
    }

    // 4. Log data to Spreadsheet
    var rowData = [
      new Date(),
      data.studentName,
      data.studentId,
      data.studentGroup,
      data.labDate,
      data.challengeCode || '',
      data.controlLogic || '',
      data.codeBlank1,
      data.codeBlank2,
      data.codeBlank3,
      data.codeBlank4,
      data.codeBlank5,
      data.quiz1 || '',
      data.quiz2 || '',
      data.quiz3 || '',
      data.quiz4 || '',
      data.quiz5 || '',
      data.question1,
      data.question2,
      data.question3 || '',
      screenshotUrl,
      codeFileUrl,
      data.conclusion,
      grading.score,
      grading.feedback
    ];
    
    sheet.appendRow(rowData);
    
    return {
      status: "success",
      message: "บันทึกข้อมูลใบงานที่ 1.1 สำเร็จแล้ว! คะแนนประเมินอัตโนมัติ: " + grading.score + "/10.0 คะแนน\n\nรายละเอียดคะแนน:\n" + grading.feedback
    };
    
  } catch (error) {
    return {
      status: "error",
      message: "เกิดข้อผิดพลาดในการบันทึกข้อมูล: " + error.toString()
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
        message: 'เชื่อมต่อ Google Apps Script (Lab 1.1) สำเร็จพร้อมบันทึกคะแนน!' + (getTargetSpreadsheet() ? (' (ไฟล์: "' + getTargetSpreadsheet().getName() + '")') : '') + '
      })).setMimeType(ContentService.MimeType.JSON);
    }

    var result = submitLabData(data);
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
