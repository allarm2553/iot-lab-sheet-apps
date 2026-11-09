/**
 * ============================================================================
 * Web App Backend for Lab 0: Environment Setup (VS Code, PlatformIO, Wokwi) & LED Blink Test
 * Course: Hybrid Local/Cloud IoT Node with ESP32 / IPST-WiFi
 * Designed by Antigravity AI (Auto-Grading Version with 10.0-Point Rubric)
 * ============================================================================
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
      .setTitle('ใบงานที่ 0: การเตรียมสภาพแวดล้อมการพัฒนา (VS Code, PlatformIO, Wokwi) และการทดสอบอัปโหลดไฟกระพริบ')
      .addMetaTag('viewport', 'width=device-width, initial-scale=1')
      .setXFrameOptionsMode(HtmlService.XFrameOptionsMode.ALLOWALL);
  } catch (err) {
    // กรณีผู้ใช้นำเฉพาะ Code.gs ไปวางใน Google Apps Script เพื่อใช้เป็น API เชื่อมต่อ Google Sheets
    if (e && e.parameter && (e.parameter.action === 'testConnection' || e.parameter.action === 'ping' || e.parameter.format === 'json')) {
      return ContentService.createTextOutput(JSON.stringify({
        status: 'success',
        message: 'เชื่อมต่อ Google Apps Script (Lab 0) สำเร็จพร้อมใช้งาน!'
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
      '<title>IoT Lab 0 Web App - พร้อมใช้งาน</title>' +
      '<style>' +
      'body{font-family:system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,sans-serif;background:#0b1120;color:#f8fafc;display:flex;align-items:center;justify-content:center;min-height:100vh;margin:0;padding:1.25rem;box-sizing:border-box;}' +
      '.card{background:#1e293b;border:1px solid #334155;padding:2.25rem 2rem;border-radius:16px;text-align:center;max-width:540px;box-shadow:0 20px 25px -5px rgba(0,0,0,0.5);}' +
      '.badge{background:rgba(16,185,129,0.15);color:#34d399;border:1px solid rgba(16,185,129,0.4);padding:0.4rem 1rem;border-radius:9999px;font-size:0.875rem;font-weight:600;display:inline-block;margin-bottom:1.25rem;}' +
      'h2{color:#38bdf8;margin:0 0 0.75rem 0;font-size:1.4rem;font-weight:700;}p{color:#94a3b8;line-height:1.6;margin:0 0 1.25rem 0;font-size:0.95rem;}' +
      '.info{background:rgba(15,23,42,0.8);border:1px solid #334155;border-radius:10px;padding:1rem;font-size:0.875rem;color:#cbd5e1;text-align:left;line-height:1.5;}' +
      '</style></head><body><div class="card">' +
      '<div class="badge">✓ เชื่อมต่อสำเร็จ (Active)</div>' +
      '<h2>Google Apps Script API พร้อมทำงาน</h2>' +
      '<p>เว็บเซอร์วิสของ <strong>ใบงานที่ 0: การเตรียมสภาพแวดล้อมการพัฒนา (VS Code, PlatformIO, Wokwi) และการทดสอบอัปโหลดไฟกระพริบ</strong> เชื่อมต่อกับ Google Sheets เรียบร้อยแล้ว พร้อมรับผลการประเมินและบันทึกคะแนนจากหน้าเว็บใบงาน</p>' +
      '<div class="info">💡 <strong>คำแนะนำ:</strong> การเชื่อมต่อสมบูรณ์แล้ว ท่านสามารถปิดแท็บนี้และกลับไปที่หน้าใบงานปฏิบัติการเพื่อทำการทดลองและกดส่งรายงานได้ตามปกติครับ</div>' +
      '</div></body></html>';
    return HtmlService.createHtmlOutput(html)
      .setTitle('IoT Lab 0 Web App - พร้อมใช้งาน')
      .addMetaTag('viewport', 'width=device-width, initial-scale=1');
  }
}

// Auto-grading logic for Lab 0 (10.0 Points Total)
function gradeSubmission(data) {
  var blankKeywords = ["OUTPUT", "HIGH|1"];
  var challengeKeywords = ["millis|currentMillis|previousMillis", "interval|blink|toggle|state", "digitalWrite|HIGH|LOW", "Serial|printf|println", "LED|2|23|19|18|0|BUTTON_PIN|INPUT_PULLUP"];
  var controlLogicKeywords = ["setup|loop", "output|กระแส|ขับโหลด|จ่ายแรงดัน", "delay|หน่วงเวลา|บล็อก|ค้าง|blocking|millis"];
  var q1Keywords = ["setup|รอบเดียว|ครั้งเดียว|เริ่มต้น|init", "loop|วนซ้ำ|ตลอดเวลา|ไม่รู้จบ|repeat", "ลำดับ|initialization|execution|start"];
  var q2Keywords = ["input only|รับค่าอย่างเดียว|ไม่มี pull-up|pull down|34|35|36|39", "strapping|boot mode|gpio 0|gpio 2|gpio 15", "ระดับแรงดัน|flash boot|ดาวน์โหลด|บูตไม่ขึ้น"];
  var q3Keywords = ["จำลอง|simulation|wokwi|virtual", "ตรวจสอบโค้ด|debug|ลดความเสียหาย|ประหยัดเวลา|ไม่ต้องต่อวงจรจริง", "platformio|vs code|สะดวก|รวดเร็ว|arduino"];
  var conclusionKeywords = ["สรุป|ติดตั้ง|vscode|platformio|wokwi|arduino", "อัปโหลด|ไฟกระพริบ|blink|gpio", "ผลการทดลอง|สำเร็จ|วิเคราะห์|non-blocking"];

  // เฉลยแบบทดสอบเลือกตอบ (Quiz Answer Keys)
  var quizKeys = {
    quiz1: "1b", // setup() ทำงานรอบเดียว
    quiz2: "2a", // File -> Preferences (Additional Boards Manager URLs)
    quiz3: "3a", // GPIO 34, 35, 36, 39 เป็น Input-Only
    quiz4: "4c", // monitor_speed = 115200
    quiz5: "5a"  // Non-blocking CPU ทำงานอื่นได้
  };

  var skeletonScore = 0.0;
  var quizScore = 0.0;
  var challengeScore = 0.0;
  var controlLogicScore = 0.0;
  var q1Score = 0.0;
  var q2Score = 0.0;
  var q3Score = 0.0;
  var conclusionScore = 0.0;
  var attachmentScore = 0.0;
  var feedbackDetails = [];

  // 1. ตรวจช่องว่างโครงร่างโค้ด (1.5 คะแนน)
  var codeContent = (data.codeBlank1 || '') + ' ' + (data.codeBlank2 || '') + ' ' + (data.codeBlank3 || '') + ' ' + (data.codeBlank4 || '');
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
  feedbackDetails.push("- แบบทดสอบเลือกตอบ 5 ข้อ: ถูกต้อง " + correctQuizCount + "/5 ข้อ (+" + quizScore.toFixed(1) + "/2.0 คะแนน)");

  // 3. ตรวจโค้ดโจทย์ท้าทาย (2.0 คะแนน)
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
    challengeScore = (matchedChallenge / challengeKeywords.length) * 2.0;
    feedbackDetails.push("- โค้ดโจทย์ท้าทาย (Non-blocking / Multi-LED): ตรงตรรกะ " + matchedChallenge + "/" + challengeKeywords.length + " จุดหลัก (+" + challengeScore.toFixed(1) + "/2.0 คะแนน)");
  } else {
    feedbackDetails.push("- โค้ดโจทย์ท้าทาย: ไม่พบการส่งโค้ดคำตอบ (+0.0/2.0 คะแนน)");
  }

  // 4. ตรวจการวิเคราะห์ Control Logic (0.8 คะแนน)
  var controlLogicText = data.controlLogic || '';
  if (controlLogicText.trim().length > 0) {
    var matchedCL = 0;
    for (var i = 0; i < controlLogicKeywords.length; i++) {
      var subKws = controlLogicKeywords[i].split('|');
      var isMatched = false;
      for (var j = 0; j < subKws.length; j++) {
        if (controlLogicText.toLowerCase().indexOf(subKws[j].toLowerCase()) !== -1) {
          isMatched = true;
          break;
        }
      }
      if (isMatched) matchedCL++;
    }
    controlLogicScore = (matchedCL / controlLogicKeywords.length) * 0.8;
    feedbackDetails.push("- วิเคราะห์ Control Logic & Functions: ครอบคลุม " + matchedCL + "/" + controlLogicKeywords.length + " ประเด็น (+" + controlLogicScore.toFixed(1) + "/0.8 คะแนน)");
  } else {
    feedbackDetails.push("- วิเคราะห์ Control Logic: ไม่พบคำอธิบาย (+0.0/0.8 คะแนน)");
  }

  // 5. ตรวจคำถามท้ายการทดลอง 3 ข้อ (ข้อละ 0.8 รวม 2.4 คะแนน)
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
    q1Score = (matchedQ1 / q1Keywords.length) * 0.8;
    feedbackDetails.push("- คำถามที่ 1 (setup vs loop): วิเคราะห์ตรงเป้า " + matchedQ1 + "/" + q1Keywords.length + " จุด (+" + q1Score.toFixed(1) + "/0.8 คะแนน)");
  } else {
    feedbackDetails.push("- คำถามที่ 1: ไม่พบคำตอบ (+0.0/0.8 คะแนน)");
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
    q2Score = (matchedQ2 / q2Keywords.length) * 0.8;
    feedbackDetails.push("- คำถามที่ 2 (Input-Only & Strapping Pins): วิเคราะห์ตรงเป้า " + matchedQ2 + "/" + q2Keywords.length + " จุด (+" + q2Score.toFixed(1) + "/0.8 คะแนน)");
  } else {
    feedbackDetails.push("- คำถามที่ 2: ไม่พบคำตอบ (+0.0/0.8 คะแนน)");
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
    q3Score = (matchedQ3 / q3Keywords.length) * 0.8;
    feedbackDetails.push("- คำถามที่ 3 (Wokwi & VS Code Simulator Benefits): วิเคราะห์ตรงเป้า " + matchedQ3 + "/" + q3Keywords.length + " จุด (+" + q3Score.toFixed(1) + "/0.8 คะแนน)");
  } else {
    feedbackDetails.push("- คำถามที่ 3: ไม่พบคำตอบ (+0.0/0.8 คะแนน)");
  }

  // 6. ตรวจสรุปผลการทดลอง (0.8 คะแนน)
  var conclusionText = data.conclusion || '';
  if (conclusionText.trim().length > 0) {
    var matchedConclusion = 0;
    for (var i = 0; i < conclusionKeywords.length; i++) {
      var subKws = conclusionKeywords[i].split('|');
      var isMatched = false;
      for (var j = 0; j < subKws.length; j++) {
        if (conclusionText.toLowerCase().indexOf(subKws[j].toLowerCase()) !== -1) {
          isMatched = true;
          break;
        }
      }
      if (isMatched) matchedConclusion++;
    }
    conclusionScore = (matchedConclusion / conclusionKeywords.length) * 0.8;
    feedbackDetails.push("- สรุปผลและอภิปรายการทดลอง: วิเคราะห์ " + matchedConclusion + "/" + conclusionKeywords.length + " ด้าน (+" + conclusionScore.toFixed(1) + "/0.8 คะแนน)");
  } else {
    feedbackDetails.push("- สรุปผลการทดลอง: ไม่พบข้อความสรุป (+0.0/0.8 คะแนน)");
  }

  // 7. ตรวจการแนบไฟล์หลักฐาน (0.5 คะแนน)
  if (data.screenshotBase64 && data.screenshotBase64.length > 50) {
    attachmentScore += 0.25;
  }
  if (data.codeBase64 && data.codeBase64.length > 50) {
    attachmentScore += 0.25;
  }
  if (attachmentScore > 0) {
    feedbackDetails.push("- การแนบไฟล์หลักฐาน (รูปภาพ/โค้ด): สมบูรณ์ (+" + attachmentScore.toFixed(2) + "/0.5 คะแนน)");
  } else {
    feedbackDetails.push("- การแนบไฟล์หลักฐาน: ไม่พบไฟล์แนบ (+0.0/0.5 คะแนน)");
  }

  var totalScore = skeletonScore + quizScore + challengeScore + controlLogicScore + q1Score + q2Score + q3Score + conclusionScore + attachmentScore;
  totalScore = Math.min(10.0, Math.round(totalScore * 10) / 10);

  return {
    score: totalScore,
    maxScore: 10.0,
    feedback: feedbackDetails.join('\n')
  };
}

function doPost(e) {
  var lock = LockService.getScriptLock();
  lock.tryLock(10000);

  try {
    var data = JSON.parse(e.postData.contents);
    var action = data.action;

    if (action === 'testConnection') {
      return ContentService.createTextOutput(JSON.stringify({
        status: 'success',
        message: 'เชื่อมต่อ Google Apps Script สำเร็จพร้อมใช้งาน!'
      })).setMimeType(ContentService.MimeType.JSON);
    }

    if (action === 'precheckScore') {
      var checkResult = gradeSubmission(data);
      return ContentService.createTextOutput(JSON.stringify({
        status: 'success',
        score: checkResult.score,
        maxScore: checkResult.maxScore,
        feedback: checkResult.feedback
      })).setMimeType(ContentService.MimeType.JSON);
    }

    var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
    
    // Check and create headers if sheet is newly initialized
    if (sheet.getLastRow() === 0) {
      var headers = [
        "Timestamp", "Student ID", "Name", "Group", "Lab Date",
        "Total Score (10.0)", "Evaluation Feedback",
        "Quiz 1", "Quiz 2", "Quiz 3", "Quiz 4", "Quiz 5",
        "Code Blank 1", "Code Blank 2", "Control Logic",
        "Challenge Code",
        "Question 1 (setup vs loop)",
        "Question 2 (Input-Only & Strapping)",
        "Question 3 (Wokwi & VS Code)",
        "Conclusion",
        "Screenshot File Name", "Screenshot Drive URL",
        "Code File Name", "Code Drive URL"
      ];
      sheet.appendRow(headers);
      sheet.getRange(1, 1, 1, headers.length).setFontWeight("bold").setBackground("#1e293b").setFontColor("#ffffff");
    }

    // ตรวจสอบการส่งซ้ำ (Duplicate Submission Check) — คอลัมน์ที่ 2 (index 1) คือ Student ID
    var existingData = sheet.getDataRange().getValues();
    var targetStudentId = String(data.studentId || '').trim().replace(/^'/, '');
    if (targetStudentId && targetStudentId !== '-') {
      for (var r = 1; r < existingData.length; r++) {
        var existingId = String(existingData[r][1] || '').trim().replace(/^'/, '');
        if (existingId === (typeof targetStudentId !== 'undefined' ? targetStudentId : targetId)) {
        if (!data.allowResubmit) {
          return ContentService.createTextOutput(JSON.stringify({
            status: 'duplicate',
            message: '❌ รหัสนักศึกษา ' + (typeof targetStudentId !== 'undefined' ? targetStudentId : targetId) + ' ได้ส่งใบงานนี้ไปแล้ว ไม่สามารถส่งซ้ำได้'
          })).setMimeType(ContentService.MimeType.JSON);
        }
        sheet.deleteRow(r + 1);
        break;
      }
      }
    }

    var grading = gradeSubmission(data);
    var timestamp = new Date();

    // Folder Handling for File Uploads
    var screenshotUrl = "-";
    var codeFileUrl = "-";

    try {
      var folderName = "IoT_Lab0_Submissions";
      var folders = DriveApp.getFoldersByName(folderName);
      var targetFolder = folders.hasNext() ? folders.next() : DriveApp.createFolder(folderName);

      if (data.screenshotBase64 && data.screenshotName) {
        var imgData = Utilities.base64Decode(data.screenshotBase64.split(',')[1] || data.screenshotBase64);
        var imgBlob = Utilities.newBlob(imgData, data.screenshotType || 'image/png', (data.studentId || 'unknown') + '_Lab0_Screenshot_' + data.screenshotName);
        var savedImg = targetFolder.createFile(imgBlob);
        savedImg.setSharing(DriveApp.Access.ANYONE_WITH_LINK, DriveApp.Permission.VIEW);
        screenshotUrl = savedImg.getUrl();
      }

      if (data.codeBase64 && data.codeFileName) {
        var rawCode = Utilities.base64Decode(data.codeBase64.split(',')[1] || data.codeBase64);
        var codeBlob = Utilities.newBlob(rawCode, data.codeFileType || 'text/plain', (data.studentId || 'unknown') + '_Lab0_Code_' + data.codeFileName);
        var savedCode = targetFolder.createFile(codeBlob);
        savedCode.setSharing(DriveApp.Access.ANYONE_WITH_LINK, DriveApp.Permission.VIEW);
        codeFileUrl = savedCode.getUrl();
      }
    } catch (driveErr) {
      Logger.log("Drive save warning: " + driveErr.toString());
    }

    var newRow = [
      timestamp,
      data.studentId || '-',
      data.studentName || '-',
      data.studentGroup || '-',
      data.labDate || '-',
      grading.score,
      grading.feedback,
      data.quiz1 || '-',
      data.quiz2 || '-',
      data.quiz3 || '-',
      data.quiz4 || '-',
      data.quiz5 || '-',
      data.codeBlank1 || '-',
      data.codeBlank2 || '-',
      data.controlLogic || '-',
      data.challengeCode || '-',
      data.question1 || '-',
      data.question2 || '-',
      data.question3 || '-',
      data.conclusion || '-',
      data.screenshotName || '-',
      screenshotUrl,
      data.codeFileName || '-',
      codeFileUrl
    ];

    sheet.appendRow(newRow);

    return ContentService.createTextOutput(JSON.stringify({
      status: 'success',
      score: grading.score,
      maxScore: grading.maxScore,
      feedback: grading.feedback,
      message: 'ส่งรายงานใบงานที่ 0 สำเร็จ! คะแนนของคุณ: ' + grading.score + ' / 10.0 คะแนน'
    })).setMimeType(ContentService.MimeType.JSON);

  } catch (error) {
    return ContentService.createTextOutput(JSON.stringify({
      status: 'error',
      message: 'เกิดข้อผิดพลาดในการบันทึกข้อมูล: ' + error.toString()
    })).setMimeType(ContentService.MimeType.JSON);
  } finally {
    lock.releaseLock();
  }
}
