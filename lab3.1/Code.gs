/**
 * Web App for Lab 3.1: Basic WiFi Connectivity (WiFi Scan & Station Mode)
 * Designed by Antigravity AI (Auto-Grading Version)
 *
 * Grading Breakdown (10 points total):
 *  - Skeleton Blanks  Exp1 (1-5)  : 2.0 pts
 *  - Skeleton Blanks  Exp2 (6-10) : 2.0 pts
 *  - Scan Result Table            : 1.0 pt
 *  - Challenge Code               : 2.0 pts
 *  - Question 1                   : 1.0 pt
 *  - Question 2                   : 1.0 pt
 *  - Attachments                  : 1.0 pt
 */

// ─────────────────────────────────────────────
//  doGet — Serve the HTML page
// ─────────────────────────────────────────────
function doGet(e) {
  return HtmlService.createTemplateFromFile('index')
    .evaluate()
    .setTitle('ใบงานที่ 3.1: การเชื่อมต่อ WiFi เบื้องต้น (WiFi Scan & Station Mode)')
    .addMetaTag('viewport', 'width=device-width, initial-scale=1')
    .setXFrameOptionsMode(HtmlService.XFrameOptionsMode.ALLOWALL);
}

// ─────────────────────────────────────────────
//  gradeSubmission — Auto-grading logic
// ─────────────────────────────────────────────
function gradeSubmission(data) {

  // Keywords for each graded section
  // Exp1 blanks: WIFI_STA, SSID, RSSI, channel, WIFI_AUTH_OPEN|ENC_TYPE_NONE
  var exp1Keywords = [
    "WIFI_STA|WIFI_MODE_STA",
    "SSID",
    "RSSI",
    "channel",
    "WIFI_AUTH_OPEN|ENC_TYPE_NONE|AUTH_OPEN"
  ];

  // Exp2 blanks: begin, WL_CONNECTED, localIP, RSSI, macAddress
  var exp2Keywords = [
    "begin",
    "WL_CONNECTED",
    "localIP",
    "RSSI",
    "macAddress"
  ];

  // Challenge code: scanNetworks, begin, WL_CONNECTED, digitalWrite|blink
  var challengeKeywords = [
    "scanNetworks",
    "begin",
    "WL_CONNECTED",
    "digitalWrite|ledcWrite|blink"
  ];

  // Question 1: RSSI, Received Signal Strength, ลบ, negative, dBm
  var q1Keywords = [
    "RSSI|Received Signal Strength",
    "ลบ|negative",
    "dBm",
    "แรง|strength|สัญญาณ"
  ];

  // Question 2: STA|Station, AP|Access Point, WIFI_AP_STA|AP_STA, โหมด|mode
  var q2Keywords = [
    "STA|Station|สถานีลูกข่าย",
    "AP|Access Point|จุดเชื่อมต่อ",
    "WIFI_AP_STA|AP_STA|ทั้งสอง",
    "เชื่อมต่อ|กระจาย|ให้บริการ"
  ];

  var exp1Score       = 0.0;
  var exp2Score       = 0.0;
  var scanScore       = 0.0;
  var challengeScore  = 0.0;
  var q1Score         = 0.0;
  var q2Score         = 0.0;
  var attachmentScore = 0.0;
  var feedbackDetails = [];

  // ── 1. Grade Exp1 Skeleton Blanks (2.0 pts) ──────────────────────────────
  var exp1Content = [
    data.codeBlank1 || '', data.codeBlank2 || '', data.codeBlank3 || '',
    data.codeBlank4 || '', data.codeBlank5 || ''
  ].join(' ');

  if (exp1Content.replace(/\s+/g, '').length > 0) {
    var matchedExp1 = 0;
    for (var i = 0; i < exp1Keywords.length; i++) {
      var subKws = exp1Keywords[i].split('|');
      for (var j = 0; j < subKws.length; j++) {
        if (exp1Content.toLowerCase().indexOf(subKws[j].toLowerCase()) !== -1) {
          matchedExp1++;
          break;
        }
      }
    }
    exp1Score = (matchedExp1 / exp1Keywords.length) * 2.0;
    feedbackDetails.push("- โค้ด Exp1 (WiFi Scan): ถูกต้อง " + matchedExp1 + "/" + exp1Keywords.length + " ช่อง (+"+exp1Score.toFixed(1)+"/2.0 คะแนน)");
  } else {
    feedbackDetails.push("- โค้ด Exp1 (WiFi Scan): ไม่พบการส่งคำตอบ (+0.0/2.0 คะแนน)");
  }

  // ── 2. Grade Exp2 Skeleton Blanks (2.0 pts) ──────────────────────────────
  var exp2Content = [
    data.codeBlank6 || '', data.codeBlank7 || '', data.codeBlank8 || '',
    data.codeBlank9 || '', data.codeBlank10 || ''
  ].join(' ');

  if (exp2Content.replace(/\s+/g, '').length > 0) {
    var matchedExp2 = 0;
    for (var i = 0; i < exp2Keywords.length; i++) {
      var subKws = exp2Keywords[i].split('|');
      for (var j = 0; j < subKws.length; j++) {
        if (exp2Content.toLowerCase().indexOf(subKws[j].toLowerCase()) !== -1) {
          matchedExp2++;
          break;
        }
      }
    }
    exp2Score = (matchedExp2 / exp2Keywords.length) * 2.0;
    feedbackDetails.push("- โค้ด Exp2 (WiFi Connect): ถูกต้อง " + matchedExp2 + "/" + exp2Keywords.length + " ช่อง (+"+exp2Score.toFixed(1)+"/2.0 คะแนน)");
  } else {
    feedbackDetails.push("- โค้ด Exp2 (WiFi Connect): ไม่พบการส่งคำตอบ (+0.0/2.0 คะแนน)");
  }

  // ── 3. Grade Scan Result Table (1.0 pt) ──────────────────────────────────
  var hasNetwork1 = (data.scan1ssid && data.scan1ssid.trim().length > 0) &&
                    (data.scan1rssi && data.scan1rssi.trim().length > 0);
  var hasNetwork2 = (data.scan2ssid && data.scan2ssid.trim().length > 0) &&
                    (data.scan2rssi && data.scan2rssi.trim().length > 0);
  var hasNetwork3 = (data.scan3ssid && data.scan3ssid.trim().length > 0) &&
                    (data.scan3rssi && data.scan3rssi.trim().length > 0);
  var hasIPMAC    = (data.connIP && data.connIP.trim().length > 0) &&
                    (data.connMAC && data.connMAC.trim().length > 0);

  var scanParts = [hasNetwork1, hasNetwork2, hasNetwork3, hasIPMAC].filter(Boolean).length;
  scanScore = (scanParts / 4) * 1.0;
  feedbackDetails.push("- ตารางบันทึกผล Scan: กรอกข้อมูล " + scanParts + "/4 ส่วน (+"+scanScore.toFixed(1)+"/1.0 คะแนน)");

  // ── 4. Grade Challenge Code (2.0 pts) ────────────────────────────────────
  var challengeText = data.challengeCode || '';
  if (challengeText.trim().length > 0) {
    var matchedChallenge = 0;
    for (var i = 0; i < challengeKeywords.length; i++) {
      var subKws = challengeKeywords[i].split('|');
      for (var j = 0; j < subKws.length; j++) {
        if (challengeText.toLowerCase().indexOf(subKws[j].toLowerCase()) !== -1) {
          matchedChallenge++;
          break;
        }
      }
    }
    challengeScore = (matchedChallenge / challengeKeywords.length) * 2.0;
    feedbackDetails.push("- โจทย์ท้าทาย (Challenge): ตรงตรรกะ " + matchedChallenge + "/" + challengeKeywords.length + " จุดหลัก (+"+challengeScore.toFixed(1)+"/2.0 คะแนน)");
  } else {
    feedbackDetails.push("- โจทย์ท้าทาย (Challenge): ไม่พบการส่งโค้ด (+0.0/2.0 คะแนน)");
  }

  // ── 5. Grade Question 1 (1.0 pt) ─────────────────────────────────────────
  var q1Text = data.question1 || '';
  if (q1Text.trim().length > 0) {
    var matchedQ1 = 0;
    for (var i = 0; i < q1Keywords.length; i++) {
      var subKws = q1Keywords[i].split('|');
      for (var j = 0; j < subKws.length; j++) {
        if (q1Text.toLowerCase().indexOf(subKws[j].toLowerCase()) !== -1) {
          matchedQ1++;
          break;
        }
      }
    }
    q1Score = (matchedQ1 / q1Keywords.length) * 1.0;
    feedbackDetails.push("- คำถามข้อ 1 (RSSI): ตรงจุดสำคัญ " + matchedQ1 + "/" + q1Keywords.length + " จุด (+"+q1Score.toFixed(1)+"/1.0 คะแนน)");
  } else {
    feedbackDetails.push("- คำถามข้อ 1 (RSSI): ไม่พบการตอบคำถาม (+0.0/1.0 คะแนน)");
  }

  // ── 6. Grade Question 2 (1.0 pt) ─────────────────────────────────────────
  var q2Text = data.question2 || '';
  if (q2Text.trim().length > 0) {
    var matchedQ2 = 0;
    for (var i = 0; i < q2Keywords.length; i++) {
      var subKws = q2Keywords[i].split('|');
      for (var j = 0; j < subKws.length; j++) {
        if (q2Text.toLowerCase().indexOf(subKws[j].toLowerCase()) !== -1) {
          matchedQ2++;
          break;
        }
      }
    }
    q2Score = (matchedQ2 / q2Keywords.length) * 1.0;
    feedbackDetails.push("- คำถามข้อ 2 (WiFi Modes): ตรงจุดสำคัญ " + matchedQ2 + "/" + q2Keywords.length + " จุด (+"+q2Score.toFixed(1)+"/1.0 คะแนน)");
  } else {
    feedbackDetails.push("- คำถามข้อ 2 (WiFi Modes): ไม่พบการตอบคำถาม (+0.0/1.0 คะแนน)");
  }

  // ── 7. Attachments (1.0 pt) ──────────────────────────────────────────────
  var screenshotOk = (data.screenshotBase64 && data.screenshotName) ? 0.5 : 0.0;
  var codeFileOk   = (data.codeBase64 && data.codeFileName) ? 0.5 : 0.0;
  attachmentScore  = screenshotOk + codeFileOk;
  feedbackDetails.push(
    "- ไฟล์แนบ: แนบรูปภาพ " + (screenshotOk ? "แล้ว" : "ไม่พบ") +
    ", แนบไฟล์โค้ด " + (codeFileOk ? "แล้ว" : "ไม่พบ") +
    " (+" + attachmentScore.toFixed(1) + "/1.0 คะแนน)"
  );

  // ── Final Score ───────────────────────────────────────────────────────────
  var finalScore = parseFloat(
    (exp1Score + exp2Score + scanScore + challengeScore + q1Score + q2Score + attachmentScore).toFixed(1)
  );

  return {
    score:    finalScore,
    feedback: feedbackDetails.join('\n')
  };
}

// ─────────────────────────────────────────────
//  submitLabData — Main submission handler
// ─────────────────────────────────────────────
function submitLabData(data) {
  try {
    var ss        = SpreadsheetApp.getActiveSpreadsheet();
    var sheetName = "Lab 3.1 Submissions";
    var sheet     = ss.getSheetByName(sheetName);

    var grading = gradeSubmission(data);

    // Auto-create sheet with headers if not found
    if (!sheet) {
      sheet = ss.insertSheet(sheetName);
      var headers = [
        "Timestamp", "ชื่อ-นามสกุล", "รหัสนักศึกษา", "กลุ่ม/ห้อง", "วันที่ทำการทดลอง",
        // Exp1 blanks
        "Blank1 (WiFi.mode)", "Blank2 (SSID fn)", "Blank3 (RSSI fn)",
        "Blank4 (channel fn)", "Blank5 (OPEN const)",
        // Exp2 blanks
        "Blank6 (WiFi.begin)", "Blank7 (WL_CONNECTED)", "Blank8 (localIP)",
        "Blank9 (RSSI)", "Blank10 (macAddress)",
        // Scan results
        "Scan1 SSID", "Scan1 RSSI", "Scan1 Security",
        "Scan2 SSID", "Scan2 RSSI", "Scan2 Security",
        "Scan3 SSID", "Scan3 RSSI", "Scan3 Security",
        "IP Address ที่ได้รับ", "MAC Address",
        // Challenge
        "โค้ดโจทย์ท้าทาย", "อธิบายตรรกะ Challenge",
        // Questions
        "คำถามข้อ 1 (RSSI)", "คำถามข้อ 2 (WiFi Modes)",
        // Attachments
        "ลิงก์รูปภาพ Serial Monitor", "ลิงก์ไฟล์โค้ด",
        // Conclusion
        "สรุปผลการทดลอง",
        // Grading
        "คะแนนประเมิน (เต็ม 10)", "ข้อเสนอแนะอัตโนมัติ"
      ];
      sheet.appendRow(headers);
      sheet.getRange(1, 1, 1, headers.length)
           .setFontWeight("bold")
           .setBackground("#e2e8f0");
      sheet.setFrozenRows(1);
    }

    // ── Handle File Uploads ──────────────────────────────────────────────────
    var screenshotUrl = "ไม่ได้แนบไฟล์";
    var codeFileUrl   = "ไม่ได้แนบไฟล์";

    var folderName = "Lab 3.1 Attachments";
    var folders    = DriveApp.getFoldersByName(folderName);
    var folder     = folders.hasNext() ? folders.next() : DriveApp.createFolder(folderName);

    if (data.screenshotBase64 && data.screenshotName) {
      var screenshotBlob = Utilities.newBlob(
        Utilities.base64Decode(data.screenshotBase64.split(",")[1]),
        data.screenshotType,
        data.studentId + "_" + data.studentName.replace(/\s+/g, '_') + "_screenshot_" + data.screenshotName
      );
      var f = folder.createFile(screenshotBlob);
      f.setSharing(DriveApp.Access.ANYONE_WITH_LINK, DriveApp.Permission.VIEW);
      screenshotUrl = f.getUrl();
    }

    if (data.codeBase64 && data.codeFileName) {
      var codeBlob = Utilities.newBlob(
        Utilities.base64Decode(data.codeBase64.split(",")[1]),
        data.codeFileType,
        data.studentId + "_" + data.studentName.replace(/\s+/g, '_') + "_code_" + data.codeFileName
      );
      var f = folder.createFile(codeBlob);
      f.setSharing(DriveApp.Access.ANYONE_WITH_LINK, DriveApp.Permission.VIEW);
      codeFileUrl = f.getUrl();
    }

    // ── Log to Spreadsheet ───────────────────────────────────────────────────
    var rowData = [
      new Date(),
      data.studentName,
      data.studentId,
      data.studentGroup,
      data.labDate,
      // Exp1
      data.codeBlank1  || '',
      data.codeBlank2  || '',
      data.codeBlank3  || '',
      data.codeBlank4  || '',
      data.codeBlank5  || '',
      // Exp2
      data.codeBlank6  || '',
      data.codeBlank7  || '',
      data.codeBlank8  || '',
      data.codeBlank9  || '',
      data.codeBlank10 || '',
      // Scan results
      data.scan1ssid || '', data.scan1rssi || '', data.scan1sec || '',
      data.scan2ssid || '', data.scan2rssi || '', data.scan2sec || '',
      data.scan3ssid || '', data.scan3rssi || '', data.scan3sec || '',
      data.connIP  || '',
      data.connMAC || '',
      // Challenge
      data.challengeCode  || '',
      data.challengeLogic || '',
      // Questions
      data.question1 || '',
      data.question2 || '',
      // Attachments
      screenshotUrl,
      codeFileUrl,
      // Conclusion
      data.conclusion || '',
      // Grading
      grading.score,
      grading.feedback
    ];

    sheet.appendRow(rowData);

    return {
      status:  "success",
      message: "บันทึกข้อมูลใบงานที่ 3.1 สำเร็จแล้ว!\n\nคะแนนประเมินอัตโนมัติ: " +
               grading.score + "/10.0 คะแนน\n\nรายละเอียดคะแนน:\n" + grading.feedback
    };

  } catch (error) {
    return {
      status:  "error",
      message: "เกิดข้อผิดพลาดในการบันทึกข้อมูล: " + error.toString()
    };
  }
}
