/**
 * Web App for Lab 6.1 (Perform): Industrial Modular IoT Node (Dynamic WebConfig & MAC Topics)
 * Designed by Antigravity AI (10.0-Point Standard Auto-Grading Version)
 */

function doGet(e) {
  return HtmlService.createTemplateFromFile('index')
    .evaluate()
    .setTitle('ใบงานที่ 6.1: การพัฒนาระบบ IoT เกรดอุตสาหกรรมด้วย WebConfig แบบไดนามิก & Auto MAC-Based Topics')
    .addMetaTag('viewport', 'width=device-width, initial-scale=1')
    .setXFrameOptionsMode(HtmlService.XFrameOptionsMode.ALLOWALL);
}

const ANSWER_KEY = {
  codeBlank1: ["/config.json", "\"r\"", "'r'", "config.json", "\"w\""],
  codeBlank2: ["deserializeJson", "deserializeJson(doc, file)", "deserializeJson(doc,file)"],
  codeBlank3: ["WiFi.macAddress()", "WiFi.macAddress", "macAddress()", "WiFi.macAddress().c_str()"],
  codeBlank4: ["subTopic.c_str()", "subTopic", "sub_topic", "subTopic.c_str()"],
  codeBlank5: ["LittleFS.remove", "LittleFS.remove(\"/config.json\")", "remove", "LittleFS.format()"],
  quiz1: ["1B", "B", "1b", "b"],
  quiz2: ["2C", "C", "2c", "c"],
  quiz3: ["3A", "A", "3a", "a"],
  quiz4: ["4C", "C", "4c", "c"],
  quiz5: ["5A", "A", "5a", "a"]
};

function gradeSubmission(data) {
  var score = 0;
  var breakdown = {};

  // 1. Code Blanks (1.5 pts: 5 blanks @ 0.3 pt)
  var blanksCorrect = 0;
  for (var i = 1; i <= 5; i++) {
    var key = "codeBlank" + i;
    var userAns = (data[key] || "").toString().trim().toLowerCase().replace(/['"\s;]/g, '');
    var validList = ANSWER_KEY[key].map(function(v) { return v.toLowerCase().replace(/['"\s;]/g, ''); });
    var isCorrect = validList.some(function(v) { return userAns === v || userAns.indexOf(v) !== -1; });
    if (isCorrect) blanksCorrect++;
    breakdown[key] = { correct: isCorrect, value: data[key] || "" };
  }
  var blankScore = Number(((blanksCorrect / 5) * 1.5).toFixed(1));
  score += blankScore;

  // 2. Quiz (2.0 pts: 5 questions @ 0.4 pt)
  var quizCorrect = 0;
  for (var j = 1; j <= 5; j++) {
    var qKey = "quiz" + j;
    var qAns = (data[qKey] || "").toString().trim().toLowerCase();
    var validQuiz = ANSWER_KEY[qKey].map(function(v) { return v.toString().toLowerCase(); });
    var qOk = validQuiz.indexOf(qAns) !== -1;
    if (qOk) quizCorrect++;
    breakdown[qKey] = { correct: qOk, selected: data[qKey] || "" };
  }
  var quizScore = Number(((quizCorrect / 5) * 2.0).toFixed(1));
  score += quizScore;

  // 3. Challenge Code (2.5 pts)
  var code = (data.challengeCode || "").toString();
  var chScore = 0.0;
  if (/(LittleFS|config\.json|loadConfiguration|saveConfiguration)/i.test(code)) chScore += 0.5;
  if (/(WiFi\.macAddress|cleanMac|subTopic|pubTopic)/i.test(code)) chScore += 0.5;
  if (/(WebSocketsServer|PubSubClient|broadcastAndPublishState|broadcastTXT)/i.test(code)) chScore += 0.5;
  if (/(resetButtonPin|resetHolding|LittleFS\.remove|ESP\.restart)/i.test(code)) chScore += 0.5;
  if (/(dhtPin|analogPin|fanRelayPin|mistRelayPin|fanButtonPin)/i.test(code)) chScore += 0.5;
  chScore = Number(chScore.toFixed(1));
  score += chScore;
  breakdown.challengeScore = chScore;

  // 4. Post-Lab Analysis (2.5 pts: 3 questions)
  var qScore = 0.0;
  var a1 = (data.question1 || "").toString().trim();
  var a2 = (data.question2 || "").toString().trim();
  var a3 = (data.question3 || "").toString().trim();
  if (a1.length > 10) qScore += 0.8;
  if (a2.length > 10) qScore += 0.85;
  if (a3.length > 10) qScore += 0.85;
  qScore = Number(qScore.toFixed(1));
  score += qScore;
  breakdown.questionScore = qScore;

  // 5. Attachments (1.0 pt)
  var attachScore = 0.0;
  if (data.screenshotBase64 && data.screenshotBase64.length > 50) attachScore += 0.5;
  if (data.codeBase64 && data.codeBase64.length > 50) attachScore += 0.5;
  score += attachScore;
  breakdown.attachScore = attachScore;

  // 6. Conclusion (0.5 pt)
  var concl = (data.conclusion || "").toString().trim();
  var conclScore = (concl.length >= 100) ? 0.5 : 0.0;
  score += conclScore;
  breakdown.conclScore = conclScore;

  return {
    score: Number(Math.min(score, 10.0).toFixed(1)),
    total: 10.0,
    breakdown: breakdown
  };
}

function processForm(formData) {
  try {
    var grading = gradeSubmission(formData);
    var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
    
    // Auto Headers if empty
    if (sheet.getLastRow() === 0) {
      sheet.appendRow([
        "Timestamp", "Student ID", "Student Name", "Group", "Lab Date",
        "Score (10.0)", "Code Blanks (1.5)", "Quiz (2.0)", "Challenge (2.5)", "Analysis Qs (2.5)", "Attachments (1.0)", "Conclusion (0.5)",
        "Q1 Analysis", "Q2 Analysis", "Q3 Analysis",
        "Challenge Code", "Conclusion Text", "Score Breakdown"
      ]);
    }
    
    var timestamp = new Date();
    var name = formData.studentName || '';
    var studentId = formData.studentId || '';
    var group = formData.studentGroup || '';
    var labDate = formData.labDate || '';
    
    sheet.appendRow([
      timestamp,
      studentId,
      name,
      group,
      labDate,
      grading.score,
      (grading.breakdown.codeBlank1.correct?0.3:0)+(grading.breakdown.codeBlank2.correct?0.3:0)+(grading.breakdown.codeBlank3.correct?0.3:0)+(grading.breakdown.codeBlank4.correct?0.3:0)+(grading.breakdown.codeBlank5.correct?0.3:0),
      grading.breakdown.quizScore || 0,
      grading.breakdown.challengeScore || 0,
      grading.breakdown.questionScore || 0,
      grading.breakdown.attachScore || 0,
      grading.breakdown.conclScore || 0,
      formData.question1 || '',
      formData.question2 || '',
      formData.question3 || '',
      formData.challengeCode || '',
      formData.conclusion || '',
      JSON.stringify(grading.breakdown)
    ]);

    return {
      status: 'success',
      score: grading.score,
      total: 10.0,
      message: 'บันทึกผลการประเมินใบงานที่ 6.1 (Perform) คะแนน ' + grading.score + ' / 10.0 เรียบร้อยแล้ว!'
    };
  } catch (err) {
    return {
      status: 'error',
      message: 'เกิดข้อผิดพลาดในการบันทึกข้อมูล: ' + err.toString()
    };
  }
}

function doPost(e) {
  try {
    var formData = JSON.parse(e.postData.contents);
    var result = processForm(formData);
    return ContentService.createTextOutput(JSON.stringify(result))
      .setMimeType(ContentService.MimeType.JSON);
  } catch (err) {
    return ContentService.createTextOutput(JSON.stringify({
      status: 'error',
      message: err.toString()
    })).setMimeType(ContentService.MimeType.JSON);
  }
}
