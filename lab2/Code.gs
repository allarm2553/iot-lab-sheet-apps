/**
 * Web App for Lab 2: OLED SSD1306 Display & Multi-Sensor Dashboard
 * Designed by Antigravity AI (Auto-Grading Version with Multiple Choice Quiz & Diagnostics)
 */

function doGet(e) {
  return HtmlService.createTemplateFromFile('index')
    .evaluate()
    .setTitle('ใบงานที่ 2: การแสดงผลผ่านจอ OLED SSD1306 (I2C)')
    .addMetaTag('viewport', 'width=device-width, initial-scale=1')
    .setXFrameOptionsMode(HtmlService.XFrameOptionsMode.ALLOWALL);
}

// Auto-grading logic for Lab 2
function gradeSubmission(data) {
  var blankKeywords = ["SSD1306_WHITE|white|1", "display\\(\\)|display"];
  var challengeKeywords = ["display", "temperature|temp|dht", "analogRead|analogPercent|knob|rawAnalog", "fanState|relayState|relay_pin|FAN_RELAY_PIN", "toggleCount|count|press", "drawRect|fillRect|printf"];
  
  var q1Keywords = ["display.display|display\\(\\)|เรนเดอร์|flush", "frame buffer|บัฟเฟอร์|ram|แรม", "ส่งข้อมูล|i2c|กะพริบ|flicker"];
  var q2Keywords = ["ขนาน|parallel|sda|scl|ร่วมกัน|บัส", "address|แอดเดรส|ที่อยู่|0x|ระบุตัวตน", "ชนกัน|conflict|ไม่ชน"];
  var q3Keywords = ["non-blocking|millis|คอขวด|แบนด์วิดท์|i2c|หน่วง", "burn-in|เบิร์น|ภาพติดตา|พิกเซลเสื่อม", "auto-dim|timeout|ดับจอ|ลดแสง|shift"];
  
  // เฉลยแบบทดสอบแบบเลือกตอบ 5 ข้อ (Quiz Answer Keys)
  var quizKeys = {
    quiz1: "1b", // SDA ส่งข้อมูล, SCL ส่งนาฬิกา, แอดเดรส 0x3C
    quiz2: "2a", // บัฟเฟอร์ขนาด 1024 ไบต์ วาดในแรมก่อนแล้วยิงรวดเดียวผ่าน I2C ป้องกันจอกระพริบ
    quiz3: "3b", // จุดบนซ้ายคือ (0, 0), X = 0..127, Y = 0..63
    quiz4: "4d", // ลืม pinMode DHT ไม่ใช่สาเหตุของ SSD1306 allocation failed
    quiz5: "5b"  // รีเฟรชทุกรอบลูปแย่งแบนด์วิดท์ I2C ทำให้ระบบหน่วง
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
    feedbackDetails.push("- โจทย์ท้าทาย (Challenge Code): ตรงตรรกะ Dashboard & GFX " + matchedChallenge + "/" + challengeKeywords.length + " จุดหลัก (+" + challengeScore.toFixed(1) + "/2.5 คะแนน)");
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
    feedbackDetails.push("- คำถามข้อ 1 (Frame Buffer & display): ตรงจุดสำคัญ " + matchedQ1 + "/" + q1Keywords.length + " จุด (+" + q1Score.toFixed(2) + "/0.85 คะแนน)");
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
    feedbackDetails.push("- คำถามข้อ 2 (I2C Bus & Address Collision): ตรงจุดสำคัญ " + matchedQ2 + "/" + q2Keywords.length + " จุด (+" + q2Score.toFixed(2) + "/0.85 คะแนน)");
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
    feedbackDetails.push("- คำถามข้อ 3 (Frame Rate & Burn-in Prevention): ตรงจุดสำคัญ " + matchedQ3 + "/" + q3Keywords.length + " จุด (+" + q3Score.toFixed(2) + "/0.80 คะแนน)");
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
    var ss = SpreadsheetApp.getActiveSpreadsheet();
    var sheetName = "Lab 2 Submissions";
    var sheet = ss.getSheetByName(sheetName);
    
    // Auto-grading calculation
    var grading = gradeSubmission(data);
    
    // Auto-create sheet if it doesn't exist
    if (!sheet) {
      sheet = ss.insertSheet(sheetName);
      var headers = [
        "Timestamp", "ชื่อ-นามสกุล", "รหัสนักศึกษา", "กลุ่ม/ห้อง", "วันที่ทำการทดลอง",
        "โค้ดโจทย์ท้าทาย (Challenge Code)", "คำอธิบายตรรกะควบคุม",
        "คำตอบ Code ช่องที่ 1", "คำตอบ Code ช่องที่ 2",
        "Quiz 1 (I2C Signals & Address)", "Quiz 2 (Frame Buffer Size & Flush)", "Quiz 3 (Coordinate System)", "Quiz 4 (Allocation Error)", "Quiz 5 (Refresh Rate)",
        "คำถามข้อที่ 1 (Frame Buffer & display)", "คำถามข้อที่ 2 (I2C Bus & Address Collision)", "คำถามข้อที่ 3 (Frame Rate & Burn-in)",
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
    var folderName = "Lab 2 Attachments";
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
      data.codeBlank1,
      data.codeBlank2,
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
      message: "บันทึกข้อมูลใบงานที่ 2 สำเร็จแล้ว! คะแนนประเมินอัตโนมัติ: " + grading.score + "/10.0 คะแนน\n\nรายละเอียดคะแนน:\n" + grading.feedback
    };
    
  } catch (error) {
    return {
      status: "error",
      message: "เกิดข้อผิดพลาดในการบันทึกข้อมูล: " + error.toString()
    };
  }
}
