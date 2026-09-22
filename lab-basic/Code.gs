/**
 * Web App for Basic Lab: การติดตั้ง Arduino IDE และคุณลักษณะของ ESP32 / ESP8266
 * Designed by Antigravity AI (Auto-Grading & Quiz Assessment Version)
 */

function doGet(e) {
  return HtmlService.createTemplateFromFile('index')
    .evaluate()
    .setTitle('ใบงานพื้นฐาน: การติดตั้ง Arduino IDE และคุณลักษณะของ ESP32')
    .addMetaTag('viewport', 'width=device-width, initial-scale=1')
    .setXFrameOptionsMode(HtmlService.XFrameOptionsMode.ALLOWALL);
}

// Auto-grading logic for Lab Basic (10.0 Points Total)
function gradeSubmission(data) {
  var feedbackDetails = [];
  
  // 1. Grade Challenge Code (3.0 pts max)
  var challengeScore = 0.0;
  var challengeCodeText = data.challengeCode || '';
  if (challengeCodeText.trim().length > 0) {
    var c1 = /pinMode\s*\(\s*(2|LED_PIN|LED_BUILTIN|18|14|d4|d5)\s*,\s*OUTPUT\s*\)/i.test(challengeCodeText);
    var c2 = /digitalWrite\s*\(\s*(2|LED_PIN|LED_BUILTIN|18|14|d4|d5)\s*,\s*HIGH\s*\)/i.test(challengeCodeText) &&
             /digitalWrite\s*\(\s*(2|LED_PIN|LED_BUILTIN|18|14|d4|d5)\s*,\s*LOW\s*\)/i.test(challengeCodeText);
    var c3 = /delay\s*\(\s*1000\s*\)/i.test(challengeCodeText) || /millis\(\)\s*-\s*\w+\s*(>=|>)\s*1000/i.test(challengeCodeText);
    
    var matchedCh = 0;
    if (c1) { matchedCh++; }
    if (c2) { matchedCh++; }
    if (c3) { matchedCh++; }
    
    challengeScore = matchedCh * 1.0;
    feedbackDetails.push("- โจทย์ท้าทาย (Challenge Code): ผ่านเกณฑ์เงื่อนไข " + matchedCh + "/3 เกณฑ์ (+" + challengeScore.toFixed(1) + "/3.0 คะแนน)");
  } else {
    feedbackDetails.push("- โจทย์ท้าทาย (Challenge Code): ไม่พบการส่งโค้ดคำตอบ (+0.0/3.0 คะแนน)");
  }

  // 2. Grade Multiple Choice Quiz (5 Questions x 0.4 pt = 2.0 pts max)
  var quizAnswers = {
    quiz1: '1b',
    quiz2: '2a',
    quiz3: '3c',
    quiz4: '4b',
    quiz5: '5c'
  };
  var correctQuiz = 0;
  var answeredQuiz = 0;
  var totalQuiz = 5;
  for (var k = 1; k <= totalQuiz; k++) {
    var studentAns = data['quiz' + k] || '';
    if (studentAns) {
      answeredQuiz++;
      if (studentAns === quizAnswers['quiz' + k]) {
        correctQuiz++;
      }
    }
  }
  var quizScore = (correctQuiz / totalQuiz) * 2.0;
  feedbackDetails.push("- แบบทดสอบเลือกตอบ (Quiz 5 ข้อ): ตอบถูก " + correctQuiz + "/" + totalQuiz + " ข้อ (+" + quizScore.toFixed(1) + "/2.0 คะแนน)");

  // 3. Grade Post-Lab Question 1: Input-Only Pins (1.5 pts max)
  var q1Score = 0.0;
  var q1Text = data.question1 || '';
  var q1Keywords = ["34", "35", "36", "39", "input", "pull", "output", "ขับ", "adc", "แอนะล็อก"];
  if (q1Text.trim().length > 10) {
    var matchedQ1 = 0;
    for (var i = 0; i < q1Keywords.length; i++) {
      if (q1Text.toLowerCase().indexOf(q1Keywords[i]) !== -1) {
        matchedQ1++;
      }
    }
    q1Score = matchedQ1 >= 2 ? 1.5 : (matchedQ1 >= 1 ? 1.0 : 0.5);
    feedbackDetails.push("- คำถามข้อที่ 1 (Input-Only GPIO): วิเคราะห์ตรงประเด็น " + matchedQ1 + " จุดสำคัญ (+" + q1Score.toFixed(1) + "/1.5 คะแนน)");
  } else {
    feedbackDetails.push("- คำถามข้อที่ 1: ไม่พบการตอบหรือข้อความสั้นเกินไป (+0.0/1.5 คะแนน)");
  }

  // 4. Grade Post-Lab Question 2: Strapping Pins (1.5 pts max)
  var q2Score = 0.0;
  var q2Text = data.question2 || '';
  var q2Keywords = ["strapping", "boot", "บูต", "gpio 0", "gpio0", "0", "uart", "download", "flash", "12", "15", "2", "pull"];
  if (q2Text.trim().length > 10) {
    var matchedQ2 = 0;
    for (var j = 0; j < q2Keywords.length; j++) {
      if (q2Text.toLowerCase().indexOf(q2Keywords[j]) !== -1) {
        matchedQ2++;
      }
    }
    q2Score = matchedQ2 >= 2 ? 1.5 : (matchedQ2 >= 1 ? 1.0 : 0.5);
    feedbackDetails.push("- คำถามข้อที่ 2 (Strapping Pins): วิเคราะห์ตรงประเด็น " + matchedQ2 + " จุดสำคัญ (+" + q2Score.toFixed(1) + "/1.5 คะแนน)");
  } else {
    feedbackDetails.push("- คำถามข้อที่ 2: ไม่พบการตอบหรือข้อความสั้นเกินไป (+0.0/1.5 คะแนน)");
  }

  // 5. Attachments (1.0 pt max: Screenshot 0.5 + Code 0.5)
  var screenshotOk = (data.screenshotBase64 && (data.screenshotName || data.screenshotType)) ? 0.5 : 0.0;
  var codeOk = (data.codeBase64 && (data.codeFileName || data.codeFileType)) ? 0.5 : 0.0;
  var attachmentScore = screenshotOk + codeOk;
  feedbackDetails.push("- ไฟล์แนบหลักฐาน: ภาพผลรัน (" + (screenshotOk ? "0.5" : "0.0") + ") + ไฟล์โค้ด .ino (" + (codeOk ? "0.5" : "0.0") + ") (+" + attachmentScore.toFixed(1) + "/1.0 คะแนน)");

  // 6. Conclusion (1.0 pt max)
  var conclusionText = data.conclusion || '';
  var conclusionScore = 0.0;
  if (conclusionText.trim().length > 100) {
    conclusionScore = 1.0;
    feedbackDetails.push("- สรุปผลการทดลอง: สมบูรณ์ (>100 ตัวอักษร) (+1.0/1.0 คะแนน)");
  } else if (conclusionText.trim().length > 30) {
    conclusionScore = 0.5;
    feedbackDetails.push("- สรุปผลการทดลอง: พอใช้ (+0.5/1.0 คะแนน)");
  } else {
    feedbackDetails.push("- สรุปผลการทดลอง: ไม่สมบูรณ์หรือสั้นเกินไป (+0.0/1.0 คะแนน)");
  }

  var finalScore = parseFloat((challengeScore + quizScore + q1Score + q2Score + attachmentScore + conclusionScore).toFixed(1));

  return {
    score: finalScore,
    feedback: feedbackDetails.join('\n')
  };
}

function submitLabData(data) {
  try {
    // 1. Open the active spreadsheet
    var ss = SpreadsheetApp.getActiveSpreadsheet();
    var sheetName = "Lab Basic Submissions";
    var sheet = ss.getSheetByName(sheetName);
    
    // Auto-grading calculation
    var grading = gradeSubmission(data);
    
    // Auto-create sheet if it doesn't exist
    if (!sheet) {
      sheet = ss.insertSheet(sheetName);
      var headers = [
        "Timestamp", "ชื่อ-นามสกุล", "รหัสนักศึกษา", "กลุ่ม/ห้อง", "วันที่ทำการทดลอง",
        "คะแนนประเมิน (เต็ม 10)", "ข้อเสนอแนะอัตโนมัติ",
        "โจทย์ท้าทาย (Challenge Code)",
        "แบบทดสอบข้อ 1", "แบบทดสอบข้อ 2", "แบบทดสอบข้อ 3", "แบบทดสอบข้อ 4", "แบบทดสอบข้อ 5",
        "คำถามข้อที่ 1 (Input-Only GPIO)", "คำถามข้อที่ 2 (Strapping Pins)",
        "ลิงก์ไฟล์รูปภาพผลการทดลอง", "ลิงก์ไฟล์โค้ด (.ino)", "สรุปผลการทดลอง"
      ];
      sheet.appendRow(headers);
      sheet.getRange(1, 1, 1, headers.length).setFontWeight("bold").setBackground("#e2e8f0");
      sheet.setFrozenRows(1);
    }
    
    // 2. Handle File Uploads (Drive Storage)
    var screenshotUrl = "ไม่ได้แนบไฟล์";
    var codeFileUrl = "ไม่ได้แนบไฟล์";
    
    // Auto-create folders for uploads
    var folderName = "Lab Basic Attachments";
    var folders = DriveApp.getFoldersByName(folderName);
    var folder;
    if (folders.hasNext()) {
      folder = folders.next();
    } else {
      folder = DriveApp.createFolder(folderName);
    }
    
    // Process screenshot
    if (data.screenshotBase64 && (data.screenshotName || data.screenshotType)) {
      var sName = data.screenshotName || "screenshot.png";
      var sType = data.screenshotType || "image/png";
      var sBase64 = data.screenshotBase64.indexOf(",") !== -1 ? data.screenshotBase64.split(",")[1] : data.screenshotBase64;
      var screenshotBlob = Utilities.newBlob(
        Utilities.base64Decode(sBase64),
        sType,
        data.studentId + "_" + (data.studentName || 'Student').replace(/\s+/g, '_') + "_screenshot_" + sName
      );
      var file = folder.createFile(screenshotBlob);
      file.setSharing(DriveApp.Access.ANYONE_WITH_LINK, DriveApp.Permission.VIEW);
      screenshotUrl = file.getUrl();
    }
    
    // Process code file
    if (data.codeBase64 && (data.codeFileName || data.codeFileType)) {
      var cName = data.codeFileName || "blink.ino";
      var cType = data.codeFileType || "text/plain";
      var cBase64 = data.codeBase64.indexOf(",") !== -1 ? data.codeBase64.split(",")[1] : data.codeBase64;
      var codeBlob = Utilities.newBlob(
        Utilities.base64Decode(cBase64),
        cType,
        data.studentId + "_" + (data.studentName || 'Student').replace(/\s+/g, '_') + "_code_" + cName
      );
      var file2 = folder.createFile(codeBlob);
      file2.setSharing(DriveApp.Access.ANYONE_WITH_LINK, DriveApp.Permission.VIEW);
      codeFileUrl = file2.getUrl();
    }
    
    // 3. Log data to Spreadsheet
    var rowData = [
      new Date(),
      data.studentName || '',
      data.studentId || '',
      data.studentGroup || '',
      data.labDate || '',
      grading.score,
      grading.feedback,
      data.challengeCode || '',
      data.quiz1 || '',
      data.quiz2 || '',
      data.quiz3 || '',
      data.quiz4 || '',
      data.quiz5 || '',
      data.question1 || '',
      data.question2 || '',
      screenshotUrl,
      codeFileUrl,
      data.conclusion || ''
    ];
    
    sheet.appendRow(rowData);
    
    return {
      status: "success",
      message: "บันทึกข้อมูลใบงานพื้นฐานสำเร็จแล้ว! คะแนนตรวจอัตโนมัติ: " + grading.score + "/10.0 คะแนน\n\nรายละเอียดคะแนน:\n" + grading.feedback
    };
    
  } catch (error) {
    return {
      status: "error",
      message: "เกิดข้อผิดพลาดในการบันทึกข้อมูล: " + error.toString()
    };
  }
}
