/**
 * Web App for Lab 3: LittleFS Web Server
 * Designed by Antigravity AI (Auto-Grading Version with Multiple Choice Quiz & Anti-Cheat)
 */

function doGet(e) {
  return HtmlService.createTemplateFromFile('index')
    .evaluate()
    .setTitle('ใบงานที่ 3: เว็บเซิร์ฟเวอร์บนบอร์ด ESP32 / ESP8266 (LittleFS Web Server)')
    .addMetaTag('viewport', 'width=device-width, initial-scale=1')
    .setXFrameOptionsMode(HtmlService.XFrameOptionsMode.ALLOWALL);
}

// Auto-grading logic for Lab 3
function gradeSubmission(data) {
  var blankKeywords = [
    "LittleFS.begin|begin",
    "server.onNotFound|onNotFound",
    "LittleFS.exists|exists",
    "server.streamFile|streamFile",
    "server.handleClient|handleClient"
  ];
  
  var challengeKeywords = [
    "LittleFS",
    "getContentType|dataType|mime",
    "exists",
    "open|streamFile",
    "send|404|onNotFound"
  ];
  
  var q1Keywords = [
    "wear leveling|กระจายการเขียน|ยืดอายุ",
    "power-loss|ไฟดับ|เสถียร|ทนทาน|ไม่พัง",
    "directory|โครงสร้างแฟ้ม|ย่อย|แรม|ram"
  ];
  
  var q2Keywords = [
    "chunk|ก้อน|ทีละส่วน|สตรีม|stream",
    "ram|heap|หน่วยความจำ|ล้น|overflow",
    "flash|ตรงจากแฟลช|ไม่พักข้อมูล|ประหยัด"
  ];
  
  var q3Keywords = [
    "mime|content-type|text/plain|text/css",
    "ไม่แสดงผล|ไม่เรนเดอร์|plain text|ตัวหนังสือเปล่า",
    "css ไม่ทำงาน|ไม่โหลดสไตล์|เพี้ยน"
  ];
  
  // เฉลยแบบทดสอบแบบเลือกตอบ 5 ข้อ (Quiz Answer Keys)
  var quizKeys = {
    quiz1: "1b", // พอร์ต 80 คือค่าเริ่มต้นของ HTTP ใช้สื่อสารแบบ Stateless Request-Response
    quiz2: "2c", // LittleFS มี Wear Leveling และ Power-loss Resilience ดีกว่า SPIFFS
    quiz3: "3a", // streamFile ส่งตรงทีละ Chunk จาก Flash ไม่ต้องโหลดทั้งไฟล์เข้า RAM ช่วยป้องกัน Heap Overflow
    quiz4: "4d", // MIME Type แจ้งบราวเซอร์ให้ประมวลผลไฟล์ถูกต้อง เช่น text/css เพื่อแสดงผลสไตล์
    quiz5: "5a"  // onNotFound ทำงานเมื่อ URI ที่ Client ร้องขอไม่ตรงกับเส้นทางใดๆ ที่ประกาศไว้ (Catch-all)
  };

  var skeletonScore = 0.0;
  var quizScore = 0.0;
  var challengeScore = 0.0;
  var q1Score = 0.0;
  var q2Score = 0.0;
  var q3Score = 0.0;
  var attachmentScore = 0.0;
  var conclusionScore = 0.0;
  var feedbackDetails = [];

  // 1. ตรวจช่องว่างโครงร่างโค้ด (1.5 คะแนน)
  var codeContent = [
    data.codeBlank1 || '',
    data.codeBlank2 || '',
    data.codeBlank3 || '',
    data.codeBlank4 || '',
    data.codeBlank5 || ''
  ].join(' ');

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
    feedbackDetails.push("- เติมคำตอบโครงร่างโค้ด: ถูกต้องตรงประเด็น " + matchedBlanks + "/" + blankKeywords.length + " ช่อง (+" + skeletonScore.toFixed(1) + "/1.5 คะแนน)");
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
    feedbackDetails.push("- โจทย์ท้าทาย (Challenge Code): ตรงตรรกะ LittleFS File Server " + matchedChallenge + "/" + challengeKeywords.length + " จุดหลัก (+" + challengeScore.toFixed(1) + "/2.5 คะแนน)");
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
    feedbackDetails.push("- คำถามข้อ 1 (LittleFS vs SPIFFS): ตรงจุดสำคัญ " + matchedQ1 + "/" + q1Keywords.length + " จุด (+" + q1Score.toFixed(2) + "/0.85 คะแนน)");
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
    feedbackDetails.push("- คำถามข้อ 2 (streamFile vs send): ตรงจุดสำคัญ " + matchedQ2 + "/" + q2Keywords.length + " จุด (+" + q2Score.toFixed(2) + "/0.85 คะแนน)");
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
    feedbackDetails.push("- คำถามข้อ 3 (MIME Type Impact): ตรงจุดสำคัญ " + matchedQ3 + "/" + q3Keywords.length + " จุด (+" + q3Score.toFixed(2) + "/0.80 คะแนน)");
  } else {
    feedbackDetails.push("- คำถามข้อ 3: ไม่พบการตอบคำถาม (+0.0/0.80 คะแนน)");
  }

  // 5. ตรวจไฟล์แนบ (1.0 คะแนน)
  var screenshotOk = (data.screenshotBase64 && data.screenshotName) ? 0.5 : 0.0;
  var codeOk = (data.codeBase64 && data.codeFileName) ? 0.5 : 0.0;
  attachmentScore = screenshotOk + codeOk;
  feedbackDetails.push("- ไฟล์แนบ: แนบรูปภาพ " + (screenshotOk ? "แล้ว" : "ไม่พบ") + ", แนบไฟล์โค้ด " + (codeOk ? "แล้ว" : "ไม่พบ") + " (+" + attachmentScore.toFixed(1) + "/1.0 คะแนน)");

  // 6. สรุปผลการทดลอง (0.5 คะแนน)
  conclusionScore = (data.conclusion && data.conclusion.trim().length > 10) ? 0.5 : 0.0;
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
    var ss = SpreadsheetApp.getActiveSpreadsheet();
    var sheetName = "Lab 3 Submissions";
    var sheet = ss.getSheetByName(sheetName);
    
    // Auto-grading calculation
    var grading = gradeSubmission(data);
    
    // Auto-create sheet if it doesn't exist
    if (!sheet) {
      sheet = ss.insertSheet(sheetName);
      var headers = [
        "Timestamp", "ชื่อ-นามสกุล", "รหัสนักศึกษา", "กลุ่ม/ห้อง", "วันที่ทำการทดลอง",
        "โค้ดโจทย์ท้าทาย (Challenge Code)", "คำอธิบายตรรกะควบคุม",
        "คำตอบ Code ช่องที่ 1 (LittleFS.begin)", "คำตอบ Code ช่องที่ 2 (onNotFound)",
        "คำตอบ Code ช่องที่ 3 (LittleFS.exists)", "คำตอบ Code ช่องที่ 4 (streamFile)", "คำตอบ Code ช่องที่ 5 (handleClient)",
        "Quiz 1 (HTTP Port & Request)", "Quiz 2 (LittleFS Advantages)", "Quiz 3 (streamFile vs send)", "Quiz 4 (MIME Content-Type)", "Quiz 5 (onNotFound Handler)",
        "คำถามข้อที่ 1 (LittleFS vs SPIFFS)", "คำถามข้อที่ 2 (streamFile RAM Efficiency)", "คำถามข้อที่ 3 (MIME Type Issues)",
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
    var folderName = "Lab 3 Attachments";
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
    
    // 3. Log data to Spreadsheet
    var rowData = [
      new Date(),
      data.studentName,
      data.studentId,
      data.studentGroup,
      data.labDate,
      data.challengeCode || '',
      data.controlLogic || '',
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
      data.question1 || '',
      data.question2 || '',
      data.question3 || '',
      screenshotUrl,
      codeFileUrl,
      data.conclusion || '',
      grading.score,
      grading.feedback
    ];
    
    sheet.appendRow(rowData);
    
    return {
      status: "success",
      message: "บันทึกข้อมูลใบงานที่ 3 สำเร็จแล้ว! คะแนนประเมินอัตโนมัติ: " + grading.score + "/10.0 คะแนน\n\nรายละเอียดคะแนน:\n" + grading.feedback
    };
    
  } catch (error) {
    return {
      status: "error",
      message: "เกิดข้อผิดพลาดในการบันทึกข้อมูล: " + error.toString()
    };
  }
}
