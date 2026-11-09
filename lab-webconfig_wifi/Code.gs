/**
 * Web App for Lab WebConfig Wi-Fi: Web Wi-Fi Configurator
 * Designed by Antigravity AI (Auto-Grading Version)
 */

function doGet(e) {
  try {
    return HtmlService.createTemplateFromFile('index')
      .evaluate()
      .setTitle('ใบงาน: การตั้งค่า Wi-Fi ผ่านหน้าเว็บ (Web Wi-Fi Configurator)')
      .addMetaTag('viewport', 'width=device-width, initial-scale=1')
      .setXFrameOptionsMode(HtmlService.XFrameOptionsMode.ALLOWALL);
  } catch (err) {
    // กรณีผู้ใช้นำเฉพาะ Code.gs ไปวางใน Google Apps Script เพื่อใช้เป็น API เชื่อมต่อ Google Sheets
    if (e && e.parameter && (e.parameter.action === 'testConnection' || e.parameter.action === 'ping' || e.parameter.format === 'json')) {
      return ContentService.createTextOutput(JSON.stringify({
        status: 'success',
        message: 'เชื่อมต่อ Google Apps Script (Lab WebConfig) สำเร็จพร้อมใช้งาน!'
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
      '<title>IoT Lab WebConfig Web App - พร้อมใช้งาน</title>' +
      '<style>' +
      'body{font-family:system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,sans-serif;background:#0b1120;color:#f8fafc;display:flex;align-items:center;justify-content:center;min-height:100vh;margin:0;padding:1.25rem;box-sizing:border-box;}' +
      '.card{background:#1e293b;border:1px solid #334155;padding:2.25rem 2rem;border-radius:16px;text-align:center;max-width:540px;box-shadow:0 20px 25px -5px rgba(0,0,0,0.5);}' +
      '.badge{background:rgba(16,185,129,0.15);color:#34d399;border:1px solid rgba(16,185,129,0.4);padding:0.4rem 1rem;border-radius:9999px;font-size:0.875rem;font-weight:600;display:inline-block;margin-bottom:1.25rem;}' +
      'h2{color:#38bdf8;margin:0 0 0.75rem 0;font-size:1.4rem;font-weight:700;}p{color:#94a3b8;line-height:1.6;margin:0 0 1.25rem 0;font-size:0.95rem;}' +
      '.info{background:rgba(15,23,42,0.8);border:1px solid #334155;border-radius:10px;padding:1rem;font-size:0.875rem;color:#cbd5e1;text-align:left;line-height:1.5;}' +
      '</style></head><body><div class="card">' +
      '<div class="badge">✓ เชื่อมต่อสำเร็จ (Active)</div>' +
      '<h2>Google Apps Script API พร้อมทำงาน</h2>' +
      '<p>เว็บเซอร์วิสของ <strong>ใบงาน: การตั้งค่า Wi-Fi ผ่านหน้าเว็บ (Web Wi-Fi Configurator)</strong> เชื่อมต่อกับ Google Sheets เรียบร้อยแล้ว พร้อมรับผลการประเมินและบันทึกคะแนนจากหน้าเว็บใบงาน</p>' +
      '<div class="info">💡 <strong>คำแนะนำ:</strong> การเชื่อมต่อสมบูรณ์แล้ว ท่านสามารถปิดแท็บนี้และกลับไปที่หน้าใบงานปฏิบัติการเพื่อทำการทดลองและกดส่งรายงานได้ตามปกติครับ</div>' +
      '</div></body></html>';
    return HtmlService.createHtmlOutput(html)
      .setTitle('IoT Lab WebConfig Web App - พร้อมใช้งาน')
      .addMetaTag('viewport', 'width=device-width, initial-scale=1');
  }
}

// Auto-grading logic for Lab WebConfig Wi-Fi
function gradeSubmission(data) {
  var blankKeywords = ["processNextRequest", "config.json"];
  var challengeKeywords = ["DNSServer|dnsServer", "WebServer|server", "LittleFS|LITTLEFS", "ArduinoJson|JsonDocument", "WiFi.softAP|softAP"];
  var q1Keywords = ["DNS", "redirect|ชี้", "192.168.4.1", "AP|Access Point", "เบราว์เซอร์"];
  var q2Keywords = ["ถาวร", "LittleFS", "SSID", "Password", "แก้โค้ด|เบิร์นโค้ด"];
  
  var challengeScore = 0.0;
  var q1Score = 0.0;
  var q2Score = 0.0;
  var attachmentScore = 0.0;
  var feedbackDetails = [];

  // 1. Grade Code / Challenge (4.0 points max)
  // 1.1 Grade Skeleton Blanks (1.5 points max)
  var hasBlanks = blankKeywords && blankKeywords.length > 0;
  var skeletonScore = 0.0;
  
  if (hasBlanks) {
    var codeContent = (data.codeBlank1 || '') + ' ' + (data.codeBlank2 || '');
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
        if (isMatched) {
          matchedBlanks++;
        }
      }
      skeletonScore = (matchedBlanks / blankKeywords.length) * 1.5;
      feedbackDetails.push("- เติมคำตอบโครงร่างโค้ด: ถูกต้องตรงประเด็น " + matchedBlanks + "/" + blankKeywords.length + " ส่วนหลัก (+" + skeletonScore.toFixed(1) + "/1.5 คะแนน)");
    } else {
      feedbackDetails.push("- เติมคำตอบโครงร่างโค้ด: ไม่พบการส่งคำตอบ (+0.0/1.5 คะแนน)");
    }
  }

  // 1.2 Grade Pasted Challenge Code (2.5 points max)
  var challengeMax = hasBlanks ? 2.5 : 4.0;
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
      if (isMatched) {
        matchedChallenge++;
      }
    }
    challengeScore = (matchedChallenge / challengeKeywords.length) * challengeMax;
    feedbackDetails.push("- โจทย์ท้าทาย (Challenge Code): ตรงตรรกะ " + matchedChallenge + "/" + challengeKeywords.length + " จุดหลัก (+" + challengeScore.toFixed(1) + "/" + challengeMax.toFixed(1) + " คะแนน)");
  } else {
    feedbackDetails.push("- โจทย์ท้าทาย (Challenge Code): ไม่พบการส่งโค้ดคำตอบ (+0.0/" + challengeMax.toFixed(1) + " คะแนน)");
  }

  // 2. Grade Question 1 (2.0 points max)
  var q1Text = data.question1 || '';
  var q1Max = 2.0;
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
      if (isMatched) {
        matchedQ1++;
      }
    }
    q1Score = (matchedQ1 / q1Keywords.length) * q1Max;
    feedbackDetails.push("- คำถามข้อ 1: ตรงจุดสำคัญ " + matchedQ1 + "/" + q1Keywords.length + " จุด (+" + q1Score.toFixed(1) + "/" + q1Max.toFixed(1) + " คะแนน)");
  } else {
    feedbackDetails.push("- คำถามข้อ 1: ไม่พบการตอบคำถาม (+0.0/" + q1Max.toFixed(1) + " คะแนน)");
  }

  // 3. Grade Question 2 (2.0 points max)
  var q2Text = data.question2 || '';
  var q2Max = 2.0;
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
      if (isMatched) {
        matchedQ2++;
      }
    }
    q2Score = (matchedQ2 / q2Keywords.length) * q2Max;
    feedbackDetails.push("- คำถามข้อ 2: ตรงจุดสำคัญ " + matchedQ2 + "/" + q2Keywords.length + " จุด (+" + q2Score.toFixed(1) + "/" + q2Max.toFixed(1) + " คะแนน)");
  } else {
    feedbackDetails.push("- คำถามข้อ 2: ไม่พบการตอบคำถาม (+0.0/" + q2Max.toFixed(1) + " คะแนน)");
  }

  // 4. Attachments (2.0 points max)
  var screenshotOk = (data.screenshotBase64 && data.screenshotName) ? 1.0 : 0.0;
  var codeOk = (data.codeBase64 && data.codeFileName) ? 1.0 : 0.0;
  attachmentScore = screenshotOk + codeOk;
  feedbackDetails.push("- ไฟล์แนบ: แนบรูปภาพ " + (screenshotOk ? "แล้ว" : "ไม่พบ") + ", แนบไฟล์โค้ด " + (codeOk ? "แล้ว" : "ไม่พบ") + " (+" + attachmentScore.toFixed(1) + "/2.0 คะแนน)");

  var finalScore = parseFloat((skeletonScore + challengeScore + q1Score + q2Score + attachmentScore).toFixed(1));

  return {
    score: finalScore,
    feedback: feedbackDetails.join('\n')
  };
}

function submitLabData(data) {
  try {
    var ss = SpreadsheetApp.getActiveSpreadsheet();
    var sheetName = "Lab WebConfig Wi-Fi Submissions";
    var sheet = ss.getSheetByName(sheetName);
    
    var grading = gradeSubmission(data);
    
    if (!sheet) {
      sheet = ss.insertSheet(sheetName);
      var headers = [
        "Timestamp", "ชื่อ-นามสกุล", "รหัสนักศึกษา", "กลุ่ม/ห้อง", "วันที่ทำการทดลอง",
        "โค้ดโจทย์ท้าทาย (Challenge Code)", "คำตอบ Code ช่องที่ 1", "คำตอบ Code ช่องที่ 2",
        "คำถามข้อที่ 1 (Captive Portal & DNS)", "คำถามข้อที่ 2 (LittleFS vs Hardcode)",
        "ลิงก์ไฟล์รูปภาพผลการทดลอง", "ลิงก์ไฟล์โค้ด (.ino/.zip)", "สรุปผลการทดลอง",
        "คะแนนประเมิน (เต็ม 10)", "ข้อเสนอแนะอัตโนมัติ"
      ];
      sheet.appendRow(headers);
      sheet.getRange(1, 1, 1, headers.length).setFontWeight("bold").setBackground("#e2e8f0");
      sheet.setFrozenRows(1);
    }
    
    var screenshotUrl = "ไม่ได้แนบไฟล์";
    var codeFileUrl = "ไม่ได้แนบไฟล์";
    
    var folderName = "Lab WebConfig Wi-Fi Attachments";
    var folders = DriveApp.getFoldersByName(folderName);
    var folder;
    if (folders.hasNext()) {
      folder = folders.next();
    } else {
      folder = DriveApp.createFolder(folderName);
    }
    
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
    
    // ตรวจสอบการส่งซ้ำ (Duplicate Submission Check)
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

    var rowData = [
      new Date(),
      data.studentName,
      data.studentId,
      data.studentGroup,
      data.labDate,
      data.challengeCode,
      data.codeBlank1,
      data.codeBlank2,
      data.question1,
      data.question2,
      screenshotUrl,
      codeFileUrl,
      data.conclusion,
      grading.score,
      grading.feedback
    ];
    
    sheet.appendRow(rowData);
    
    return {
      success: true,
      score: grading.score,
      feedback: grading.feedback
    };
  } catch (err) {
    return {
      success: false,
      error: err.toString()
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
        message: 'เชื่อมต่อ Google Apps Script (Lab WebConfig) สำเร็จพร้อมบันทึกคะแนน!'
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
