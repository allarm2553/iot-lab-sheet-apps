/**
 * Web App for Lab 5 Dev: Cloud MQTT & Cross-Platform Dashboard
 * Designed by Antigravity AI (Auto-Grading Version with Multiple Choice Quiz & Troubleshooting)
 */

function doGet(e) {
  return HtmlService.createTemplateFromFile('index')
    .evaluate()
    .setTitle('ใบงานที่ 5 (Dev): การสื่อสารผ่านระบบคลาวด์และแดชบอร์ดข้ามแพลตฟอร์ม (Cloud MQTT)')
    .addMetaTag('viewport', 'width=device-width, initial-scale=1')
    .setXFrameOptionsMode(HtmlService.XFrameOptionsMode.ALLOWALL);
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
    var ss = SpreadsheetApp.getActiveSpreadsheet();
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
      sheet = ss.insertSheet(sheetName);
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
