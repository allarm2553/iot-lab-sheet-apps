/**
 * Web App for Lab 6: Hybrid Dual-Protocol IoT Dashboard (WebSockets + Cloud MQTT)
 * Designed by Antigravity AI (Auto-Grading Version)
 */

function doGet(e) {
  return HtmlService.createTemplateFromFile('index')
    .evaluate()
    .setTitle('ใบงานที่ 6: การประยุกต์ใช้การสื่อสารแบบผสมผสาน (Hybrid Dual-Protocol: WS + MQTT)')
    .addMetaTag('viewport', 'width=device-width, initial-scale=1')
    .setXFrameOptionsMode(HtmlService.XFrameOptionsMode.ALLOWALL);
}

// Auto-grading logic for Lab 6
function gradeSubmission(data) {
  var blankKeywords = [
    "webSocket|WebSocketsServer",
    "mqttClient|PubSubClient",
    "broadcastAndPublishState|broadcastTXT|publish",
    "handleIncomingCommand|deserializeJson"
  ];

  var challengeKeywords = [
    "WebSocketsServer|PubSubClient|webSocket|mqttClient",
    "broadcastTXT|publish|esp-node/state|esp-node/control/cmd",
    "tempThreshold|threshold|hysteresis|fanState|mistState",
    "ssd1306|oled|display|WiFi.localIP|wsClientCount"
  ];

  var q1Keywords = [
    "latency|ความหน่วง|เร็ว|เรียลไทม์|จุดต่อจุด|direct|lan|local",
    "อินเทอร์เน็ต|internet|cloud|ข้ามเครือข่าย|nat|firewall|remote|ระยะไกล",
    "คู่ขนาน|hybrid|สำรอง|fallback|เสถียร"
  ];

  var q2Keywords = [
    "overhead|หัวข้อมูล|โครงสร้างข้อมูล|json",
    "qos|quality of service|retained|last will|lwt",
    "broadcast|multicast|subscribe|publish"
  ];

  var challengeScore = 0.0;
  var q1Score = 0.0;
  var q2Score = 0.0;
  var attachmentScore = 0.0;
  var feedbackDetails = [];

  // 1. Grade Code / Challenge (4.0 points max)
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

  // 1.2 Grade Main Source Code / GitHub URL / Written Solution (2.5 points)
  var fullCode = (data.codeContent || '') + ' ' + (data.githubUrl || '');
  if (fullCode.replace(/\s+/g, '').length > 5) {
    var matchedChallenge = 0;
    for (var k = 0; k < challengeKeywords.length; k++) {
      var cKws = challengeKeywords[k].split('|');
      for (var l = 0; l < cKws.length; l++) {
        if (fullCode.toLowerCase().indexOf(cKws[l].toLowerCase()) !== -1) {
          matchedChallenge++;
          break;
        }
      }
    }
    var mainCodeScore = (matchedChallenge / challengeKeywords.length) * 2.5;
    if (!hasBlanks) {
      mainCodeScore = (matchedChallenge / challengeKeywords.length) * 4.0;
    }
    challengeScore = skeletonScore + mainCodeScore;
    if (challengeScore > 4.0) challengeScore = 4.0;
    feedbackDetails.push("- ส่วนซอร์สโค้ดโซลูชัน: ตรวจพบองค์ประกอบ Dual-Protocol (WS+MQTT) " + matchedChallenge + "/" + challengeKeywords.length + " ส่วนหลัก (+" + mainCodeScore.toFixed(1) + " คะแนน)");
  } else {
    challengeScore = skeletonScore;
    feedbackDetails.push("- ส่วนซอร์สโค้ดโซลูชัน: ไม่พบการแนบไฟล์/ลิงก์ซอร์สโค้ด (+0.0 คะแนน)");
  }

  // 2. Grade Theory Question 1: WebSocket vs MQTT & Hybrid Comparison (2.0 points max)
  var ans1 = data.ans1 || '';
  if (ans1.trim().length > 5) {
    var matchedQ1 = 0;
    for (var m = 0; m < q1Keywords.length; m++) {
      var q1Kws = q1Keywords[m].split('|');
      for (var n = 0; n < q1Kws.length; n++) {
        if (ans1.toLowerCase().indexOf(q1Kws[n].toLowerCase()) !== -1) {
          matchedQ1++;
          break;
        }
      }
    }
    q1Score = (matchedQ1 / q1Keywords.length) * 2.0;
    if (q1Score < 0.8 && ans1.length > 20) q1Score = 0.8;
    feedbackDetails.push("- คำถามข้อที่ 1 (เปรียบเทียบ WebSocket & MQTT): ได้คะแนน " + q1Score.toFixed(1) + "/2.0 คะแนน");
  } else {
    feedbackDetails.push("- คำถามข้อที่ 1: ไม่ได้ตอบคำถาม (+0.0/2.0 คะแนน)");
  }

  // 3. Grade Theory Question 2: QoS & Hybrid Protocol Optimization (2.0 points max)
  var ans2 = data.ans2 || '';
  if (ans2.trim().length > 5) {
    var matchedQ2 = 0;
    for (var p = 0; p < q2Keywords.length; p++) {
      var q2Kws = q2Keywords[p].split('|');
      for (var q = 0; q < q2Kws.length; q++) {
        if (ans2.toLowerCase().indexOf(q2Kws[q].toLowerCase()) !== -1) {
          matchedQ2++;
          break;
        }
      }
    }
    q2Score = (matchedQ2 / q2Keywords.length) * 2.0;
    if (q2Score < 0.8 && ans2.length > 20) q2Score = 0.8;
    feedbackDetails.push("- คำถามข้อที่ 2 (QoS & Synchronization): ได้คะแนน " + q2Score.toFixed(1) + "/2.0 คะแนน");
  } else {
    feedbackDetails.push("- คำถามข้อที่ 2: ไม่ได้ตอบคำถาม (+0.0/2.0 คะแนน)");
  }

  // 4. Grade File Attachment / Screenshot Proof (2.0 points max)
  if (data.fileData && data.fileData.contents) {
    attachmentScore = 2.0;
    feedbackDetails.push("- หลักฐานผลการทดลอง (ภาพถ่าย/ภาพแคปหน้าจอ): สมบูรณ์ (+2.0/2.0 คะแนน)");
  } else {
    feedbackDetails.push("- หลักฐานผลการทดลอง: ไม่ได้แนบไฟล์ภาพ (+0.0/2.0 คะแนน)");
  }

  var totalScore = challengeScore + q1Score + q2Score + attachmentScore;
  if (totalScore > 10.0) totalScore = 10.0;

  var result = {
    studentId: data.studentId,
    studentName: data.studentName,
    section: data.section,
    totalScore: Math.round(totalScore * 10) / 10,
    details: feedbackDetails.join('\n')
  };

  saveToSpreadsheet(data, result);
  return result;
}

function saveToSpreadsheet(data, result) {
  try {
    var ss = SpreadsheetApp.getActiveSpreadsheet();
    var sheet = ss.getSheetByName("ผลการส่งใบงาน_LAB6") || ss.insertSheet("ผลการส่งใบงาน_LAB6");
    
    if (sheet.getLastRow() === 0) {
      sheet.appendRow([
        "ประทับเวลา", 
        "รหัสนักศึกษา", 
        "ชื่อ-นามสกุล", 
        "กลุ่ม (Section)", 
        "คะแนนรวม (10)", 
        "คะแนนโจทย์ท้าทาย (4)", 
        "คะแนนข้อ 1 (2)", 
        "คะแนนข้อ 2 (2)", 
        "คะแนนไฟล์แนบ (2)", 
        "สรุปผลการทดลอง",
        "ลิงก์ไฟล์แนบ Google Drive",
        "รายละเอียดการตรวจคะแนน"
      ]);
    }
    
    var fileUrl = "";
    if (data.fileData && data.fileData.contents) {
      try {
        var folder = getOrCreateFolder("LAB6_Submissions");
        var bytes = Utilities.base64Decode(data.fileData.contents.split(',')[1]);
        var blob = Utilities.newBlob(bytes, data.fileData.mimeType, data.studentId + "_LAB6_" + data.fileData.name);
        var file = folder.createFile(blob);
        file.setSharing(DriveApp.Access.ANYONE_WITH_LINK, DriveApp.Permission.VIEW);
        fileUrl = file.getUrl();
      } catch (err) {
        fileUrl = "Error uploading: " + err.toString();
      }
    }
    
    sheet.appendRow([
      new Date(),
      data.studentId,
      data.studentName,
      data.section,
      result.totalScore,
      (result.totalScore >= 4.0 ? 4.0 : result.totalScore),
      2.0,
      2.0,
      (fileUrl ? 2.0 : 0.0),
      (data.conclusion || ''),
      fileUrl,
      result.details
    ]);
  } catch (e) {
    Logger.log("Error saving to spreadsheet: " + e.toString());
  }
}

function getOrCreateFolder(folderName) {
  var folders = DriveApp.getFoldersByName(folderName);
  if (folders.hasNext()) {
    return folders.next();
  } else {
    return DriveApp.createFolder(folderName);
  }
}
