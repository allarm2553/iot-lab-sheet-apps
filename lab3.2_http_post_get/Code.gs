/**
 * Web App for Lab 3.2: HTTP GET / POST Web Server & REST API
 * Designed by Antigravity AI (Auto-Grading Version)
 */

function doGet(e) {
  return HtmlService.createTemplateFromFile('index')
    .evaluate()
    .setTitle('ใบงานที่ 3.2: การควบคุมอุปกรณ์และการอ่านค่าเซ็นเซอร์ผ่าน HTTP GET / POST')
    .addMetaTag('viewport', 'width=device-width, initial-scale=1')
    .setXFrameOptionsMode(HtmlService.XFrameOptionsMode.ALLOWALL);
}

// Auto-grading logic for lab3.2
function gradeSubmission(data) {
  var blankKeywords = [
    "handleGetData", 
    "handleSetRelay", 
    "serializeJson", 
    "server.send", 
    "deserializeJson"
  ];
  var q1Keywords = ["get", "post", "idempotent", "safe", "body", "payload", "url", "side effect", "ความปลอดภัย", "แคช", "สถานะ"];
  var q2Keywords = ["polling", "overhead", "header", "tcp", "ram", "latency", "websocket", "หน่วง", "แบนด์วิดท์", "ภาระ"];
  
  var skeletonScore = 0.0;
  var challengeScore = 0.0;
  var q1Score = 0.0;
  var q2Score = 0.0;
  var attachmentScore = 0.0;
  var feedbackDetails = [];

  // 1. Grade Skeleton Blanks (1.5 points max)
  var codeContent = (data.codeBlank1 || '') + ' ' + (data.codeBlank2 || '') + ' ' + (data.codeBlank3 || '') + ' ' + (data.codeBlank4 || '') + ' ' + (data.codeBlank5 || '');
  if (codeContent.replace(/\s+/g, '').length > 0) {
    var matchedBlanks = 0;
    for (var i = 0; i < blankKeywords.length; i++) {
      if (codeContent.toLowerCase().indexOf(blankKeywords[i].toLowerCase()) !== -1) {
        matchedBlanks++;
      }
    }
    skeletonScore = (matchedBlanks / blankKeywords.length) * 1.5;
    feedbackDetails.push("- เติมคำตอบโครงร่างโค้ด: ถูกต้องตรงประเด็น " + matchedBlanks + "/" + blankKeywords.length + " ส่วน (" + skeletonScore.toFixed(1) + "/1.5 คะแนน)");
  } else {
    feedbackDetails.push("- เติมคำตอบโครงร่างโค้ด: ไม่พบการส่งคำตอบ (+0.0/1.5 คะแนน)");
  }

  // 2. Grade Challenge / Full Code (2.5 points max)
  var challengeCodeText = data.challengeCode || '';
  if (challengeCodeText.trim().length > 30) {
    var codeMatches = 0;
    var keywords = ["handleGetData", "handleSetRelay", "HTTP_GET", "HTTP_POST", "ArduinoJson", "digitalWrite", "dht", "WiFi"];
    for (var k = 0; k < keywords.length; k++) {
      if (challengeCodeText.indexOf(keywords[k]) !== -1) {
        codeMatches++;
      }
    }
    challengeScore = Math.min(2.5, (codeMatches / keywords.length) * 2.5);
    feedbackDetails.push("- โค้ดโปรแกรมฉบับสมบูรณ์: โครงสร้างครบถ้วน (" + challengeScore.toFixed(1) + "/2.5 คะแนน)");
  } else {
    feedbackDetails.push("- โค้ดโปรแกรมฉบับสมบูรณ์: ไม่พบการแนบโค้ด (+0.0/2.5 คะแนน)");
  }

  // 3. Grade Question 1 (2.0 points max)
  var q1Text = data.question1 || '';
  if (q1Text.trim().length > 15) {
    var m1 = 0;
    for (var i = 0; i < q1Keywords.length; i++) {
      if (q1Text.toLowerCase().indexOf(q1Keywords[i]) !== -1) m1++;
    }
    q1Score = Math.min(2.0, Math.max(0.8, (m1 / 3.0) * 2.0));
    feedbackDetails.push("- คำถามที่ 1 (ความแตกต่าง HTTP GET vs POST): ให้เหตุผลตรงประเด็น (" + q1Score.toFixed(1) + "/2.0 คะแนน)");
  } else {
    feedbackDetails.push("- คำถามที่ 1: คำตอบสั้นเกินไปหรือไม่พบคำตอบ (+0.0/2.0 คะแนน)");
  }

  // 4. Grade Question 2 (2.0 points max)
  var q2Text = data.question2 || '';
  if (q2Text.trim().length > 15) {
    var m2 = 0;
    for (var j = 0; j < q2Keywords.length; j++) {
      if (q2Text.toLowerCase().indexOf(q2Keywords[j]) !== -1) m2++;
    }
    q2Score = Math.min(2.0, Math.max(0.8, (m2 / 3.0) * 2.0));
    feedbackDetails.push("- คำถามที่ 2 (ผลกระทบของ HTTP Polling): อธิบายผลกระทบและแนวทางแก้ไขชัดเจน (" + q2Score.toFixed(1) + "/2.0 คะแนน)");
  } else {
    feedbackDetails.push("- คำถามที่ 2: คำตอบสั้นเกินไปหรือไม่พบคำตอบ (+0.0/2.0 คะแนน)");
  }

  // 5. Grade Attachments & Results (2.0 points max)
  var hasAttachment = (data.fileData && data.fileData.length > 50) || (data.photoData && data.photoData.length > 50);
  var hasResults = (data.expResult1 && data.expResult1.length > 0) || (data.conclusion && data.conclusion.length > 15);
  if (hasAttachment && hasResults) {
    attachmentScore = 2.0;
    feedbackDetails.push("- ผลการทดลองและหลักฐานประกอบ: แนบหลักฐานและบันทึกผลครบถ้วน (+2.0/2.0 คะแนน)");
  } else if (hasAttachment || hasResults) {
    attachmentScore = 1.2;
    feedbackDetails.push("- ผลการทดลองและหลักฐานประกอบ: แนบหลักฐานหรือบันทึกผลบางส่วน (+1.2/2.0 คะแนน)");
  } else {
    feedbackDetails.push("- ผลการทดลองและหลักฐานประกอบ: ไม่พบหลักฐาน (+0.0/2.0 คะแนน)");
  }

  var totalScore = skeletonScore + challengeScore + q1Score + q2Score + attachmentScore;
  totalScore = Math.min(10.0, Math.round(totalScore * 10) / 10);

  return {
    score: totalScore,
    feedback: "คะแนนรวม: " + totalScore + "/10 คะแนน\n" + feedbackDetails.join("\n")
  };
}

function doPost(e) {
  var lock = LockService.getScriptLock();
  lock.tryLock(10000);
  
  try {
    var data = JSON.parse(e.postData.contents);
    var grading = gradeSubmission(data);

    var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
    
    // Check Header row
    if (sheet.getLastRow() === 0) {
      sheet.appendRow([
        "Timestamp", "Student ID", "Full Name", "Class", "Group", 
        "Score (10)", "Grading Feedback", 
        "Exp 1: GET Temp/Hum", "Exp 2: POST Relay Latency",
        "Code Blank 1", "Code Blank 2", "Code Blank 3", "Code Blank 4", "Code Blank 5",
        "Question 1 (GET vs POST)", "Question 2 (HTTP Polling)", "Conclusion",
        "Challenge Code", "Attachment File", "Photo URL"
      ]);
    }

    var row = [
      new Date(),
      data.studentId || '',
      data.studentName || '',
      data.studentClass || '',
      data.groupNumber || '',
      grading.score,
      grading.feedback,
      data.expResult1 || '',
      data.expResult2 || '',
      data.codeBlank1 || '',
      data.codeBlank2 || '',
      data.codeBlank3 || '',
      data.codeBlank4 || '',
      data.codeBlank5 || '',
      data.question1 || '',
      data.question2 || '',
      data.conclusion || '',
      data.challengeCode || '',
      data.fileName ? "Attached: " + data.fileName : '',
      data.photoName ? "Attached: " + data.photoName : ''
    ];

    sheet.appendRow(row);

    return ContentService.createTextOutput(JSON.stringify({
      status: "success",
      score: grading.score,
      feedback: grading.feedback,
      message: "บันทึกข้อมูลและตรวจคะแนนเรียบร้อยแล้ว"
    })).setMimeType(ContentService.MimeType.JSON);

  } catch (error) {
    return ContentService.createTextOutput(JSON.stringify({
      status: "error",
      message: error.toString()
    })).setMimeType(ContentService.MimeType.JSON);
  } finally {
    lock.releaseLock();
  }
}
