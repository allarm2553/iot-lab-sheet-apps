/**
 * Web App for Lab 5 Dev: Cloud MQTT & Cross-Platform Dashboard
 * Designed by Antigravity AI (Auto-Grading Version with Multiple Choice Quiz & Troubleshooting)
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
      .setTitle('ใบงานที่ 5 (Dev): การสื่อสารผ่านระบบคลาวด์และแดชบอร์ดข้ามแพลตฟอร์ม (Cloud MQTT)')
      .addMetaTag('viewport', 'width=device-width, initial-scale=1')
      .setXFrameOptionsMode(HtmlService.XFrameOptionsMode.ALLOWALL);
  } catch (err) {
    // กรณีผู้ใช้นำเฉพาะ Code.gs ไปวางใน Google Apps Script เพื่อใช้เป็น API เชื่อมต่อ Google Sheets
    if (e && e.parameter && (e.parameter.action === 'testConnection' || e.parameter.action === 'ping' || e.parameter.format === 'json')) {
      return ContentService.createTextOutput(JSON.stringify({
        status: 'success',
        message: 'เชื่อมต่อ Google Apps Script (Lab 5 (Dev)) สำเร็จพร้อมใช้งาน!'
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
      '<title>IoT Lab 5 (Dev) Web App - พร้อมใช้งาน</title>' +
      '<style>' +
      'body{font-family:system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,sans-serif;background:#0b1120;color:#f8fafc;display:flex;align-items:center;justify-content:center;min-height:100vh;margin:0;padding:1.25rem;box-sizing:border-box;}' +
      '.card{background:#1e293b;border:1px solid #334155;padding:2.25rem 2rem;border-radius:16px;text-align:center;max-width:540px;box-shadow:0 20px 25px -5px rgba(0,0,0,0.5);}' +
      '.badge{background:rgba(16,185,129,0.15);color:#34d399;border:1px solid rgba(16,185,129,0.4);padding:0.4rem 1rem;border-radius:9999px;font-size:0.875rem;font-weight:600;display:inline-block;margin-bottom:1.25rem;}' +
      'h2{color:#38bdf8;margin:0 0 0.75rem 0;font-size:1.4rem;font-weight:700;}p{color:#94a3b8;line-height:1.6;margin:0 0 1.25rem 0;font-size:0.95rem;}' +
      '.info{background:rgba(15,23,42,0.8);border:1px solid #334155;border-radius:10px;padding:1rem;font-size:0.875rem;color:#cbd5e1;text-align:left;line-height:1.5;}' +
      '</style></head><body><div class="card">' +
      '<div class="badge">✓ เชื่อมต่อสำเร็จ (Active)</div>' +
      '<h2>Google Apps Script API พร้อมทำงาน</h2>' +
      '<p>เว็บเซอร์วิสของ <strong>ใบงานที่ 5 (Dev): การสื่อสารผ่านระบบคลาวด์และแดชบอร์ดข้ามแพลตฟอร์ม (Cloud MQTT)</strong> เชื่อมต่อกับ Google Sheets เรียบร้อยแล้ว พร้อมรับผลการประเมินและบันทึกคะแนนจากหน้าเว็บใบงาน</p>' +
      '<div class="info">💡 <strong>คำแนะนำ:</strong> การเชื่อมต่อสมบูรณ์แล้ว ท่านสามารถปิดแท็บนี้และกลับไปที่หน้าใบงานปฏิบัติการเพื่อทำการทดลองและกดส่งรายงานได้ตามปกติครับ</div>' +
      '</div></body></html>';
    return HtmlService.createHtmlOutput(html)
      .setTitle('IoT Lab 5 (Dev) Web App - พร้อมใช้งาน')
      .addMetaTag('viewport', 'width=device-width, initial-scale=1');
  }
}

// Auto-grading logic for Lab 5 Dev
function gradeSubmission(data) {
  var blankKeywords = ["espClient", "1883", "callback", "subTopic|control|cmd", "pubTopic|state"];
  var challengeKeywords = [
    "mqttClient|client|pubSubClient",
    "publish|pubTopic",
    "subscribe|subTopic",
    "ArduinoJson|JsonDocument|serializeJson|deserializeJson",
    "temperature|temp|humidity|hum|soil|analogPercent",
    "fan|fanState|digitalWrite",
    "toggleCount|press|count"
  ];
  var q1Keywords = ["disconnect|หลุด|เตะ|ชน|ซ้ำ", "reconnect|วนลูป|แย่ง|สลับ", "mac address|student id|chip id|เฉพาะตัว|unique"];
  var q2Keywords = ["+|1 ระดับ|single-level", "#|ทุกระดับ|multi-level", "qos 0|ไม่การันตี|telemetry", "qos 1|qos 2|การันตี|ฉุกเฉิน|แน่นอน"];
  var q3Keywords = ["json|แพ็กเกจเดียว|แบนด์วิดท์|ประสิทธิภาพ", "delay|บล็อก|ค้าง|ชะงัก", "loop|callback|keep-alive|ping|ไม่ตอบสนอง|หลุด"];
  
  // เฉลยแบบทดสอบแบบเลือกตอบ 5 ข้อ (Quiz Answer Keys)
  var quizKeys = {
    quiz1: "1b", // ตัวกลางคัดแยกและกระจายข้อความระหว่าง Publisher และ Subscriber ตาม Topic
    quiz2: "2c", // Broker จะตัดการเชื่อมต่อไคลเอนต์เดิมออก และวนลูปเชื่อมต่อใหม่สลับกันไปมา (Collision Loop)
    quiz3: "3c", // QoS 2 การันตีได้รับแน่นอนและเพียงครั้งเดียวเท่านั้น (Exactly once)
    quiz4: "4b", // แทนที่ชื่อระดับชั้นของหัวข้อเฉพาะระดับนั้นได้ 1 ระดับชั้น (Single-level Wildcard)
    quiz5: "5b"  // ทำให้ mqttClient.loop() หยุดทำงาน ไม่สามารถรับ Callback และพลาด Keep-Alive จน Broker ตัดการเชื่อมต่อ
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
    feedbackDetails.push("- โจทย์ท้าทาย (Challenge Code): ตรงตรรกะ & JSON Telemetry " + matchedChallenge + "/" + challengeKeywords.length + " จุดหลัก (+" + challengeScore.toFixed(1) + "/2.5 คะแนน)");
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
    feedbackDetails.push("- คำถามข้อ 1 (Client ID Collision): ตรงจุดสำคัญ " + matchedQ1 + "/" + q1Keywords.length + " จุด (+" + q1Score.toFixed(2) + "/0.85 คะแนน)");
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
    feedbackDetails.push("- คำถามข้อ 2 (Topic & QoS Level): ตรงจุดสำคัญ " + matchedQ2 + "/" + q2Keywords.length + " จุด (+" + q2Score.toFixed(2) + "/0.85 คะแนน)");
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
    feedbackDetails.push("- คำถามข้อ 3 (JSON Payload & Non-blocking loop): ตรงจุดสำคัญ " + matchedQ3 + "/" + q3Keywords.length + " จุด (+" + q3Score.toFixed(2) + "/0.80 คะแนน)");
  } else {
    feedbackDetails.push("- คำถามข้อ 3: ไม่พบการตอบคำถาม (+0.0/0.80 คะแนน)");
  }

  // 5. ตรวจไฟล์แนบ (1.0 คะแนน)
  var screenshotOk = (data.screenshotBase64 && data.screenshotName) ? 0.5 : 0.0;
  var codeOk = (data.codeBase64 && data.codeFileName) ? 0.5 : 0.0;
  attachmentScore = screenshotOk + codeOk;
  feedbackDetails.push("- ไฟล์แนบ: แนบรูปภาพ " + (screenshotOk ? "แล้ว" : "ไม่พบ") + ", แนบไฟล์โค้ด " + (codeOk ? "แล้ว" : "ไม่พบ") + " (+" + attachmentScore.toFixed(1) + "/1.0 คะแนน)");

  // 6. สรุปผลการทดลอง (0.5 คะแนน)
  var conclusionScore = (data.conclusion && data.conclusion.trim().length > 100) ? 0.5 : 0.0;
  feedbackDetails.push("- สรุปผลการทดลอง (>100 ตัวอักษร): " + (conclusionScore > 0 ? "สมบูรณ์ (+0.5/0.5 คะแนน)" : "ไม่ผ่านเกณฑ์ (+0.0/0.5 คะแนน)"));

  var finalScore = parseFloat((skeletonScore + quizScore + challengeScore + q1Score + q2Score + q3Score + attachmentScore + conclusionScore).toFixed(1));

  return {
    score: finalScore,
    feedback: feedbackDetails.join('\n')
  };
}

function submitLabData(data) {
  try {
    var ss = getTargetSpreadsheet();
    var sheetName = "Lab 5 Dev Submissions";
    var sheet = ss.getSheetByName(sheetName);
    
    // Auto-grading calculation
    var gradeResult = gradeSubmission(data);
    var score = gradeResult.score;
    var feedback = gradeResult.feedback;
    
    // Upload files to Google Drive
    var screenshotUrl = "";
    var codeUrl = "";
    var folderName = "IoT_Lab5_Dev_Uploads";
    var folders = DriveApp.getFoldersByName(folderName);
    var folder = folders.hasNext() ? folders.next() : DriveApp.createFolder(folderName);
    
    if (data.screenshotBase64 && data.screenshotName) {
      try {
        var base64Data = data.screenshotBase64.split(',')[1] || data.screenshotBase64;
        var decodedData = Utilities.base64Decode(base64Data);
        var blob = Utilities.newBlob(decodedData, data.screenshotType || 'image/png', (data.studentId || 'unknown') + "_screenshot_" + data.screenshotName);
        var file = folder.createFile(blob);
        file.setSharing(DriveApp.Access.ANYONE_WITH_LINK, DriveApp.Permission.VIEW);
        screenshotUrl = file.getUrl();
      } catch (err) {
        screenshotUrl = "Upload Error: " + err.toString();
      }
    }
    
    if (data.codeBase64 && data.codeFileName) {
      try {
        var base64Data = data.codeBase64.split(',')[1] || data.codeBase64;
        var decodedData = Utilities.base64Decode(base64Data);
        var blob = Utilities.newBlob(decodedData, data.codeFileType || 'text/plain', (data.studentId || 'unknown') + "_code_" + data.codeFileName);
        var file = folder.createFile(blob);
        file.setSharing(DriveApp.Access.ANYONE_WITH_LINK, DriveApp.Permission.VIEW);
        codeUrl = file.getUrl();
      } catch (err) {
        codeUrl = "Upload Error: " + err.toString();
      }
    }

    if (!sheet) {
      var allSheets = ss.getSheets();
      if (allSheets.length === 1 && allSheets[0].getLastRow() === 0) {
        sheet = allSheets[0];
        sheet.setName(sheetName);
      } else {
        sheet = ss.insertSheet(sheetName);
      }
      sheet.appendRow([
        "Timestamp", "ชื่อ-นามสกุล", "รหัสนักศึกษา", "กลุ่ม/ห้อง", "วันที่ทำแล็บ",
        "ช่อง 1 (Client)", "ช่อง 2 (Port)", "ช่อง 3 (Callback)", "ช่อง 4 (SubTopic)", "ช่อง 5 (PubTopic)",
        "Quiz 1", "Quiz 2", "Quiz 3", "Quiz 4", "Quiz 5",
        "โจทย์ท้าทาย (Challenge Code)",
        "คำถาม 1 (Client ID Collision)", "คำถาม 2 (Topic & QoS)", "คำถาม 3 (JSON & Non-blocking)",
        "รูปภาพผลงาน", "ไฟล์โค้ด", "สรุปผลการทดลอง",
        "คะแนนที่ได้ (เต็ม 10.0)", "ข้อเสนอแนะ/Feedback"
      ]);
      sheet.getRange(1, 1, 1, 24).setFontWeight("bold").setBackground("#e0e7ff");
    }

    // ตรวจสอบการส่งซ้ำ (Duplicate Submission Check)
    var existingData = sheet.getDataRange().getValues();
    var targetId = String(data.studentId || '').trim().replace(/^'/, '');
    if (targetId) {
      for (var r = 1; r < existingData.length; r++) {
        var existingId = String(existingData[r][2] || '').trim().replace(/^'/, '');
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
      new Date(),
      data.studentName || '',
      "'" + (data.studentId || ''),
      data.studentGroup || '',
      data.labDate || '',
      data.codeBlank1 || '',
      data.codeBlank2 || '',
      data.codeBlank3 || '',
      data.codeBlank4 || '',
      data.codeBlank5 || '',
      data.quiz1 || '',
      data.quiz2 || '',
      data.quiz3 || '',
      data.quiz4 || '',
      data.quiz5 || '',
      data.challengeCode || '',
      data.question1 || '',
      data.question2 || '',
      data.question3 || '',
      screenshotUrl,
      codeUrl,
      data.conclusion || '',
      score,
      feedback
    ]);

    return {
      status: 'success',
      message: 'บันทึกข้อมูลเรียบร้อยแล้ว ได้รับการประเมิน ' + score + ' / 10.0 คะแนน',
      score: score,
      feedback: feedback
    };
  } catch (e) {
    return {
      status: 'error',
      message: 'เกิดข้อผิดพลาด: ' + e.toString()
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
        message: 'เชื่อมต่อ Google Apps Script (Lab 5 (Dev)) สำเร็จพร้อมบันทึกคะแนน!' + (getTargetSpreadsheet() ? (' (ไฟล์: "' + getTargetSpreadsheet().getName() + '")') : '') + '
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
