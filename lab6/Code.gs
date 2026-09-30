/**
 * Web App for Lab 6: Hybrid Dual-Mode IoT Node (Local WebSockets & Cloud MQTT)
 * Designed by Antigravity AI (Auto-Grading Version)
 */

function doGet(e) {
  return HtmlService.createTemplateFromFile('index')
    .evaluate()
    .setTitle('ใบงานที่ 6: การบูรณาการระบบ Hybrid Dual-Mode IoT Node (Local WebSockets & Cloud MQTT)')
    .addMetaTag('viewport', 'width=device-width, initial-scale=1')
    .setXFrameOptionsMode(HtmlService.XFrameOptionsMode.ALLOWALL);
}

const ANSWER_KEY = {
  codeBlank1: ["81", "81.0", "webSocketPort"],
  codeBlank2: ["espClient", "&espClient", "WiFiClient"],
  codeBlank3: ["esp32-climate-node/state", "esp32-node/state", "esp-node/state", "pubTopic", "pub_topic", ""esp32-climate-node/state""],
  codeBlank4: ["webSocketEvent", "onWebSocketEvent", "webSocketCallback"],
  codeBlank5: ["esp32-climate-node/control/cmd", "esp32-node/control/cmd", "esp-node/control/cmd", "subTopic", "sub_topic", ""esp32-climate-node/control/cmd""],
  quiz1: "A",
  quiz2: "A",
  quiz3: "A",
  quiz4: "A",
  quiz5: "A"
};

function gradeSubmission(data) {
  var score = 0;
  var breakdown = {};

  for (var i = 1; i <= 5; i++) {
    var key = "codeBlank" + i;
    var userAns = (data[key] || "").toString().trim().toLowerCase().replace(/['"\s;]/g, '');
    var validList = ANSWER_KEY[key].map(function(v) { return v.toLowerCase().replace(/['"\s;]/g, ''); });
    var isCorrect = validList.some(function(v) { return userAns === v || userAns.indexOf(v) !== -1; });
    if (isCorrect) {
      score += 1;
      breakdown[key] = { correct: true, points: 1 };
    } else {
      breakdown[key] = { correct: false, points: 0 };
    }
  }

  for (var j = 1; j <= 5; j++) {
    var qKey = "quiz" + j;
    var qAns = (data[qKey] || "").toString().trim().toUpperCase();
    if (qAns === ANSWER_KEY[qKey]) {
      score += 1;
      breakdown[qKey] = { correct: true, points: 1 };
    } else {
      breakdown[qKey] = { correct: false, points: 0 };
    }
  }

  return {
    score: score,
    total: 10,
    breakdown: breakdown
  };
}

function processForm(formData) {
  try {
    var grading = gradeSubmission(formData);
    var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
    
    var timestamp = new Date();
    var name = formData.studentName || '';
    var studentId = formData.studentId || '';
    var group = formData.studentGroup || '';
    
    sheet.appendRow([
      timestamp,
      studentId,
      name,
      group,
      grading.score,
      JSON.stringify(grading.breakdown),
      formData.analysis1 || '',
      formData.analysis2 || '',
      formData.analysis3 || '',
      formData.challengeCode || '',
      formData.conclusion || ''
    ]);

    return {
      status: 'success',
      score: grading.score,
      total: 10,
      message: 'บันทึกคะแนนและส่งใบงานปฏิบัติการที่ 6 เรียบร้อยแล้ว!'
    };
  } catch (err) {
    return {
      status: 'error',
      message: 'เกิดข้อผิดพลาดในการบันทึกข้อมูล: ' + err.toString()
    };
  }
}
