/**
 * Web App for Lab 4.1: HTTP GET / POST Web Server & REST API
 * Designed by Antigravity AI (Auto-Grading & Sheet Logger Version)
 */

const SHEET_NAME = "Lab 4.1 Submissions";
const DRIVE_FOLDER_NAME = "Lab 4.1 Attachments";

const quizKeys = {
  quiz1: "1b", // GET is idempotent read without side-effects, POST mutates state with payload
  quiz2: "2a", // Content-Type: application/json tells browser to parse as JSON
  quiz3: "3a", // server.arg("plain") retrieves raw HTTP POST Request Body (JSON payload)
  quiz4: "4a", // HTTP Polling overhead from headers and repeated TCP handshakes
  quiz5: "5a"  // ArduinoJson JsonDocument avoids syntax errors and safely manages memory
};

function doGet(e) {
  try {
    return HtmlService.createTemplateFromFile('index')
      .evaluate()
      .setTitle('ใบงานที่ 4.1: การควบคุมอุปกรณ์และการอ่านค่าเซ็นเซอร์ผ่าน HTTP GET / POST')
      .addMetaTag('viewport', 'width=device-width, initial-scale=1')
      .setXFrameOptionsMode(HtmlService.XFrameOptionsMode.ALLOWALL);
  } catch (err) {
    // กรณีผู้ใช้นำเฉพาะ Code.gs ไปวางใน Google Apps Script เพื่อใช้เป็น API เชื่อมต่อ Google Sheets
    if (e && e.parameter && (e.parameter.action === 'testConnection' || e.parameter.action === 'ping' || e.parameter.format === 'json')) {
      return ContentService.createTextOutput(JSON.stringify({
        status: 'success',
        message: 'เชื่อมต่อ Google Apps Script (Lab 4.1) สำเร็จพร้อมใช้งาน!'
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
      '<title>IoT Lab 4.1 Web App - พร้อมใช้งาน</title>' +
      '<style>' +
      'body{font-family:system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,sans-serif;background:#0b1120;color:#f8fafc;display:flex;align-items:center;justify-content:center;min-height:100vh;margin:0;padding:1.25rem;box-sizing:border-box;}' +
      '.card{background:#1e293b;border:1px solid #334155;padding:2.25rem 2rem;border-radius:16px;text-align:center;max-width:540px;box-shadow:0 20px 25px -5px rgba(0,0,0,0.5);}' +
      '.badge{background:rgba(16,185,129,0.15);color:#34d399;border:1px solid rgba(16,185,129,0.4);padding:0.4rem 1rem;border-radius:9999px;font-size:0.875rem;font-weight:600;display:inline-block;margin-bottom:1.25rem;}' +
      'h2{color:#38bdf8;margin:0 0 0.75rem 0;font-size:1.4rem;font-weight:700;}p{color:#94a3b8;line-height:1.6;margin:0 0 1.25rem 0;font-size:0.95rem;}' +
      '.info{background:rgba(15,23,42,0.8);border:1px solid #334155;border-radius:10px;padding:1rem;font-size:0.875rem;color:#cbd5e1;text-align:left;line-height:1.5;}' +
      '</style></head><body><div class="card">' +
      '<div class="badge">✓ เชื่อมต่อสำเร็จ (Active)</div>' +
      '<h2>Google Apps Script API พร้อมทำงาน</h2>' +
      '<p>เว็บเซอร์วิสของ <strong>ใบงานที่ 4.1: การควบคุมอุปกรณ์และการอ่านค่าเซ็นเซอร์ผ่าน HTTP GET / POST</strong> เชื่อมต่อกับ Google Sheets เรียบร้อยแล้ว พร้อมรับผลการประเมินและบันทึกคะแนนจากหน้าเว็บใบงาน</p>' +
      '<div class="info">💡 <strong>คำแนะนำ:</strong> การเชื่อมต่อสมบูรณ์แล้ว ท่านสามารถปิดแท็บนี้และกลับไปที่หน้าใบงานปฏิบัติการเพื่อทำการทดลองและกดส่งรายงานได้ตามปกติครับ</div>' +
      '</div></body></html>';
    return HtmlService.createHtmlOutput(html)
      .setTitle('IoT Lab 4.1 Web App - พร้อมใช้งาน')
      .addMetaTag('viewport', 'width=device-width, initial-scale=1');
  }
}

function submitLabData(formObject) {
  try {
    const ss = SpreadsheetApp.getActiveSpreadsheet();
    let sheet = ss.getSheetByName(SHEET_NAME);
    if (!sheet) {
      sheet = ss.insertSheet(SHEET_NAME);
      const headers = [
        "Timestamp", "Student ID", "Student Name", "Group/Sec", "Lab Date",
        "Total Score (10.0)", "Skeleton Blanks (1.5)", "Quiz Score (2.0)", 
        "Challenge (2.5)", "Questions (2.5)", "Attachments (1.0)", "Conclusion (0.5)",
        "Quiz 1", "Quiz 2", "Quiz 3", "Quiz 4", "Quiz 5",
        "Blank 1", "Blank 2", "Blank 3", "Blank 4", "Blank 5",
        "Exp Result GET", "Exp Result POST",
        "Challenge Code", "Control Logic",
        "Question 1 (GET vs POST)", "Question 2 (Polling Overhead)", "Question 3 (CORS Header)",
        "Conclusion Text", "Screenshot File URL", "Code File URL"
      ];
      sheet.appendRow(headers);
      sheet.setFrozenRows(1);
      sheet.getRange(1, 1, 1, headers.length).setFontWeight("bold").setBackground("#1e293b").setFontColor("#f8fafc");
    }

    // ── 1. Evaluate Skeleton Blanks (1.5 Points) ──
    const blanks = [
      (formObject.codeBlank1 || "").trim(),
      (formObject.codeBlank2 || "").trim(),
      (formObject.codeBlank3 || "").trim(),
      (formObject.codeBlank4 || "").trim(),
      (formObject.codeBlank5 || "").trim()
    ];
    let filledBlanks = 0;
    blanks.forEach(b => { if (b.length > 0) filledBlanks++; });
    const blankScore = Math.min(1.5, (filledBlanks / 5.0) * 1.5);

    // ── 2. Evaluate Multiple Choice Quiz (2.0 Points - 0.4 each) ──
    let quizScore = 0.0;
    for (const q in quizKeys) {
      if (formObject[q] === quizKeys[q]) {
        quizScore += 0.4;
      }
    }
    quizScore = Math.min(2.0, quizScore);

    // ── 3. Evaluate Challenge Code & Logic (2.5 Points) ──
    const chCode = (formObject.challengeCode || "").trim();
    const ctrlLogic = (formObject.controlLogic || "").trim();
    let challengeScore = 0.0;
    if (chCode.length > 30) challengeScore += 1.5;
    if (ctrlLogic.length > 15) challengeScore += 1.0;
    challengeScore = Math.min(2.5, challengeScore);

    // ── 4. Evaluate Analytical Questions (2.5 Points) ──
    const q1 = (formObject.question1 || "").trim();
    const q2 = (formObject.question2 || "").trim();
    const q3 = (formObject.question3 || "").trim();
    let qScore = 0.0;
    if (q1.length > 10) qScore += 0.85;
    if (q2.length > 10) qScore += 0.85;
    if (q3.length > 10) qScore += 0.80;
    qScore = Math.min(2.5, qScore);

    // ── 5. Evaluate File Attachments (1.0 Points) ──
    let screenshotUrl = "";
    let codeFileUrl = "";
    let attachScore = 0.0;

    let folder = getOrCreateFolder(DRIVE_FOLDER_NAME);

    if (formObject.screenshotBase64) {
      try {
        const decodedImg = Utilities.base64Decode(formObject.screenshotBase64.split(",")[1]);
        const imgBlob = Utilities.newBlob(decodedImg, formObject.screenshotType || "image/png", `${formObject.studentId}_Lab4_1_Screenshot_${new Date().getTime()}.png`);
        const savedImg = folder.createFile(imgBlob);
        savedImg.setSharing(DriveApp.Access.ANYONE_WITH_LINK, DriveApp.Permission.VIEW);
        screenshotUrl = savedImg.getUrl();
        attachScore += 0.5;
      } catch (e) {
        screenshotUrl = "Upload Error: " + e.toString();
      }
    }

    if (formObject.codeBase64) {
      try {
        const decodedCode = Utilities.base64Decode(formObject.codeBase64.split(",")[1]);
        const codeBlob = Utilities.newBlob(decodedCode, formObject.codeFileType || "text/plain", `${formObject.studentId}_Lab4_1_Code_${new Date().getTime()}_${formObject.codeFileName || "solution.ino"}`);
        const savedCode = folder.createFile(codeBlob);
        savedCode.setSharing(DriveApp.Access.ANYONE_WITH_LINK, DriveApp.Permission.VIEW);
        codeFileUrl = savedCode.getUrl();
        attachScore += 0.5;
      } catch (e) {
        codeFileUrl = "Upload Error: " + e.toString();
      }
    }
    attachScore = Math.min(1.0, attachScore);

    // ── 6. Evaluate Conclusion (0.5 Points) ──
    const concl = (formObject.conclusion || "").trim();
    const conclScore = (concl.length > 10) ? 0.5 : (concl.length > 0 ? 0.25 : 0.0);

    // ── Total Score ──
    const totalScore = Number((blankScore + quizScore + challengeScore + qScore + attachScore + conclScore).toFixed(1));

    // ตรวจสอบการส่งซ้ำ (Duplicate Submission Check)
    const existingData = sheet.getDataRange().getValues();
    const targetId = String(formObject.studentId || "").trim();
    if (targetId) {
      for (let r = 1; r < existingData.length; r++) {
        if (String(existingData[r][1]).trim() === targetId) {
        if (!data.allowResubmit) {
          return {
            status: "duplicate",
            message: "❌ รหัสนักศึกษา " + targetId + " ได้ส่งใบงานนี้ไปแล้ว ไม่สามารถส่งซ้ำได้"
          };
        }
        sheet.deleteRow(r + 1);
        break;
      }
      }
    }

    const rowData = [
      new Date(),
      formObject.studentId,
      formObject.studentName,
      formObject.studentGroup,
      formObject.labDate,
      totalScore,
      blankScore,
      quizScore,
      challengeScore,
      qScore,
      attachScore,
      conclScore,
      formObject.quiz1 || "",
      formObject.quiz2 || "",
      formObject.quiz3 || "",
      formObject.quiz4 || "",
      formObject.quiz5 || "",
      formObject.codeBlank1 || "",
      formObject.codeBlank2 || "",
      formObject.codeBlank3 || "",
      formObject.codeBlank4 || "",
      formObject.codeBlank5 || "",
      formObject.expResult1 || "",
      formObject.expResult2 || "",
      chCode,
      ctrlLogic,
      q1,
      q2,
      q3,
      concl,
      screenshotUrl,
      codeFileUrl
    ];

    sheet.appendRow(rowData);

    return {
      status: "success",
      totalScore: totalScore,
      message: `คะแนนประเมินรวม: ${totalScore.toFixed(1)} / 10.0 คะแนน\nบันทึกข้อมูลเรียบร้อยแล้ว`
    };

  } catch (error) {
    return {
      status: "error",
      message: error.toString()
    };
  }
}

function getOrCreateFolder(folderName) {
  const folders = DriveApp.getFoldersByName(folderName);
  if (folders.hasNext()) {
    return folders.next();
  }
  return DriveApp.createFolder(folderName);
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
        message: 'เชื่อมต่อ Google Apps Script (Lab 4.1) สำเร็จพร้อมบันทึกคะแนน!'
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
