/**
 * Web App for Lab WebConfig Wi-Fi: Web Wi-Fi Configurator
 * Designed by Antigravity AI (Auto-Grading Version)
 */

function doGet(e) {
  return HtmlService.createTemplateFromFile('index')
    .evaluate()
    .setTitle('ใบงาน: การตั้งค่า Wi-Fi ผ่านหน้าเว็บ (Web Wi-Fi Configurator)')
    .addMetaTag('viewport', 'width=device-width, initial-scale=1')
    .setXFrameOptionsMode(HtmlService.XFrameOptionsMode.ALLOWALL);
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
