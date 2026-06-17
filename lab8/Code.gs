/**
 * Web App for Lab 8: การบันทึกสถิติมิติตามเวลาด้วย InfluxDB (Data Logging)
 * Designed by Antigravity AI (Auto-Grading Version)
 */

function doGet(e) {
  return HtmlService.createTemplateFromFile('index')
    .evaluate()
    .setTitle('ใบงานที่ 8: การบันทึกสถิติมิติตามเวลาด้วย InfluxDB (Data Logging)')
    .addMetaTag('viewport', 'width=device-width, initial-scale=1')
    .setXFrameOptionsMode(HtmlService.XFrameOptionsMode.ALLOWALL);
}

// Auto-grading logic for Lab 8
// Auto-grading logic for lab8
// Auto-grading logic for lab8
function gradeSubmission(data) {
  var blankKeywords = ["time-series","Bucket","Measurement","Tags","Fields"];
  var challengeKeywords = ["ON|high", "OFF|low", "1", "0", "msg.payload"];
  var q1Keywords = ["อนุกรมเวลา","time-series","MySQL","เวลา","ความถี่","เขียนเร็ว"];
  var q2Keywords = ["Bucket","Measurement","Tags","Fields","ถัง","ตาราง","ดัชนี"];
  
  var challengeScore = 0.0;
  var q1Score = 0.0;
  var q2Score = 0.0;
  var attachmentScore = 0.0;
  var feedbackDetails = [];

  // 1. Grade Code / Challenge (4.0 points max)
  // 1.1 Grade Skeleton Blanks (1.5 points max if blanks exist, otherwise challenge gets full 4.0 points)
  var hasBlanks = blankKeywords && blankKeywords.length > 0;
  var skeletonScore = 0.0;
  
  if (hasBlanks) {
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

  // 1.2 Grade Pasted Challenge Code (2.5 points max if blanks exist, otherwise 4.0 points max)
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

  // 2. Grade Question 1 (2.0 points, or 4.0 points if Q2 does not exist)
  var q1Text = data.question1 || '';
  var hasQ2 = q2Keywords && q2Keywords.length > 0;
  var q1Max = hasQ2 ? 2.0 : 4.0;
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
  if (hasQ2) {
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
        if (isMatched) {
          matchedQ2++;
        }
      }
      q2Score = (matchedQ2 / q2Keywords.length) * 2.0;
      feedbackDetails.push("- คำถามข้อ 2: ตรงจุดสำคัญ " + matchedQ2 + "/" + q2Keywords.length + " จุด (+" + q2Score.toFixed(1) + "/2.0 คะแนน)");
    } else {
      feedbackDetails.push("- คำถามข้อ 2: ไม่พบการตอบคำถาม (+0.0/2.0 คะแนน)");
    }
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
    // 1. Open the active spreadsheet
    var ss = SpreadsheetApp.getActiveSpreadsheet();
    var sheetName = "Lab 8 Submissions";
    var sheet = ss.getSheetByName(sheetName);
    
    // Auto-grading calculation
    var grading = gradeSubmission(data);
    
    // Auto-create sheet if it doesn't exist
    if (!sheet) {
      sheet = ss.insertSheet(sheetName);
      var headers = [
        "Timestamp", "ชื่อ-นามสกุล", "รหัสนักศึกษา", "กลุ่ม/ห้อง", "วันที่ทำการทดลอง",
        "โค้ดโจทย์ท้าทาย (Challenge Code)", "คำอธิบายตรรกะ (controlLogic)", "คำอธิบายโจทย์ท้าทาย (challengeLogic)",
        "คำตอบท้าทาย (JS Relay to 1/0)", 
        "คำถามข้อที่ 1 (Time-Series vs MySQL)", "คำถามข้อที่ 2 (InfluxDB Schema)",
        "ลิงก์ไฟล์รูปภาพผลการทดลอง", "ลิงก์ไฟล์ส่งออก Flow JSON", "สรุปผลการทดลอง",
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
    var folderName = "Lab 8 Attachments";
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
    
    // Process JSON file
    if (data.codeBase64 && data.codeFileName) {
      var codeBlob = Utilities.newBlob(
        Utilities.base64Decode(data.codeBase64.split(",")[1]),
        data.codeFileType,
        data.studentId + "_" + data.studentName.replace(/\s+/g, '_') + "_flow_" + data.codeFileName
      );
      var file = folder.createFile(codeBlob);
      file.setSharing(DriveApp.Access.ANYONE_WITH_LINK, DriveApp.Permission.VIEW);
      codeFileUrl = file.getUrl();
    }
    
    // 3. Log data to Spreadsheet
    var rowData = [
      new Date(),
      data.studentName,
      data.studentId,
      data.studentGroup,
      data.labDate,
      data.challengeCode || '',
      data.controlLogic || '',
      data.challengeLogic || '',
      data.codeBlank1,
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
      status: "success",
      message: "บันทึกข้อมูลใบงานที่ 8 สำเร็จแล้ว! คะแนนประเมินอัตโนมัติ: " + grading.score + "/10.0 คะแนน\n\nรายละเอียดคะแนน:\n" + grading.feedback
    };
    
  } catch (error) {
    return {
      status: "error",
      message: "เกิดข้อผิดพลาดในการบันทึกข้อมูล: " + error.toString()
    };
  }
}
