/**
 * Google Apps Script for LAB6_Perform: Auto-Grading & Google Sheets Integration
 */

function doPost(e) {
  try {
    var data = JSON.parse(e.postData.contents);
    var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
    
    // Auto-create headers if sheet is empty
    if (sheet.getLastRow() === 0) {
      sheet.appendRow([
        "Timestamp", "Student ID", "Student Name", "Group", 
        "Device MAC", "Sub Topic", "Pub Topic",
        "Q1 (MAC Topic Reason)", "Q2 (Config & Reboot Flow)", 
        "Source Code", "Auto Score", "Status"
      ]);
    }
    
    var timestamp = new Date();
    var studentId = data.studentId || "";
    var studentName = data.studentName || "";
    var studentGroup = data.studentGroup || "";
    var deviceMac = data.deviceMac || "";
    var cleanMac = deviceMac.replace(/:/g, "").toUpperCase();
    var subTopic = "esp-node/" + cleanMac + "/control/cmd";
    var pubTopic = "esp-node/" + cleanMac + "/state";
    
    var q1 = data.answers ? data.answers.q1 : "";
    var q2 = data.answers ? data.answers.q2 : "";
    var code = data.answers ? data.answers.code : "";
    
    // Auto grading logic
    var score = 0;
    if (q1.length > 20) score += 3;
    if (q2.length > 20) score += 3;
    if (code.indexOf("LittleFS") !== -1 || code.indexOf("config") !== -1 || code.indexOf("serializeJson") !== -1) score += 4;
    
    sheet.appendRow([
      timestamp, studentId, studentName, studentGroup,
      deviceMac, subTopic, pubTopic,
      q1, q2, code, score + "/10", "Graded"
    ]);
    
    return ContentService.createTextOutput(JSON.stringify({
      status: "success",
      score: score + "/10",
      message: "บันทึกผลการประเมิน LAB6_Perform เรียบร้อยแล้ว"
    })).setMimeType(ContentService.MimeType.JSON);
    
  } catch (error) {
    return ContentService.createTextOutput(JSON.stringify({
      status: "error",
      message: error.toString()
    })).setMimeType(ContentService.MimeType.JSON);
  }
}
