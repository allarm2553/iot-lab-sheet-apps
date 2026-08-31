/**
 * Web App for Lab 4.1: HTTP GET / POST Web Server & REST API
 * Designed by Antigravity AI (Auto-Grading & Sheet Logger Version)
 */

const SHEET_NAME = "Lab 4.1 Submissions";
const DRIVE_FOLDER_NAME = "Lab 4.1 Attachments";

const quizKeys = {
  quiz1: "1b", // GET is idempotent read without side-effects, POST mutates state with payload
  quiz2: "2a", // Content-Type: application/json tells browser to parse as JSON
  quiz3: "3a", // server.arg("plain") retrieves raw HTTP POST Request Body (JSON payload)
  quiz4: "4a", // HTTP Polling overhead from headers and repeated TCP handshakes
  quiz5: "5a"  // ArduinoJson JsonDocument avoids syntax errors and safely manages memory
};

function doGet(e) {
  return HtmlService.createTemplateFromFile("index")
    .evaluate()
    .setTitle("ใบงานที่ 4.1: การควบคุมอุปกรณ์และการอ่านค่าเซ็นเซอร์ผ่าน HTTP GET / POST")
    .addMetaTag("viewport", "width=device-width, initial-scale=1")
    .setXFrameOptionsMode(HtmlService.XFrameOptionsMode.ALLOWALL);
}

function submitLabData(formObject) {
  try {
    const ss = SpreadsheetApp.getActiveSpreadsheet();
    let sheet = ss.getSheetByName(SHEET_NAME);
    if (!sheet) {
      sheet = ss.insertSheet(SHEET_NAME);
      const headers = [
        "Timestamp", "Student ID", "Student Name", "Group/Sec", "Lab Date",
        "Total Score (10.0)", "Skeleton Blanks (1.5)", "Quiz Score (2.0)", 
        "Challenge (2.5)", "Questions (2.5)", "Attachments (1.0)", "Conclusion (0.5)",
        "Quiz 1", "Quiz 2", "Quiz 3", "Quiz 4", "Quiz 5",
        "Blank 1", "Blank 2", "Blank 3", "Blank 4", "Blank 5",
        "Exp Result GET", "Exp Result POST",
        "Challenge Code", "Control Logic",
        "Question 1 (GET vs POST)", "Question 2 (Polling Overhead)", "Question 3 (CORS Header)",
        "Conclusion Text", "Screenshot File URL", "Code File URL"
      ];
      sheet.appendRow(headers);
      sheet.setFrozenRows(1);
      sheet.getRange(1, 1, 1, headers.length).setFontWeight("bold").setBackground("#1e293b").setFontColor("#f8fafc");
    }

    // ── 1. Evaluate Skeleton Blanks (1.5 Points) ──
    const blanks = [
      (formObject.codeBlank1 || "").trim(),
      (formObject.codeBlank2 || "").trim(),
      (formObject.codeBlank3 || "").trim(),
      (formObject.codeBlank4 || "").trim(),
      (formObject.codeBlank5 || "").trim()
    ];
    let filledBlanks = 0;
    blanks.forEach(b => { if (b.length > 0) filledBlanks++; });
    const blankScore = Math.min(1.5, (filledBlanks / 5.0) * 1.5);

    // ── 2. Evaluate Multiple Choice Quiz (2.0 Points - 0.4 each) ──
    let quizScore = 0.0;
    for (const q in quizKeys) {
      if (formObject[q] === quizKeys[q]) {
        quizScore += 0.4;
      }
    }
    quizScore = Math.min(2.0, quizScore);

    // ── 3. Evaluate Challenge Code & Logic (2.5 Points) ──
    const chCode = (formObject.challengeCode || "").trim();
    const ctrlLogic = (formObject.controlLogic || "").trim();
    let challengeScore = 0.0;
    if (chCode.length > 30) challengeScore += 1.5;
    if (ctrlLogic.length > 15) challengeScore += 1.0;
    challengeScore = Math.min(2.5, challengeScore);

    // ── 4. Evaluate Analytical Questions (2.5 Points) ──
    const q1 = (formObject.question1 || "").trim();
    const q2 = (formObject.question2 || "").trim();
    const q3 = (formObject.question3 || "").trim();
    let qScore = 0.0;
    if (q1.length > 10) qScore += 0.85;
    if (q2.length > 10) qScore += 0.85;
    if (q3.length > 10) qScore += 0.80;
    qScore = Math.min(2.5, qScore);

    // ── 5. Evaluate File Attachments (1.0 Points) ──
    let screenshotUrl = "";
    let codeFileUrl = "";
    let attachScore = 0.0;

    let folder = getOrCreateFolder(DRIVE_FOLDER_NAME);

    if (formObject.screenshotBase64) {
      try {
        const decodedImg = Utilities.base64Decode(formObject.screenshotBase64.split(",")[1]);
        const imgBlob = Utilities.newBlob(decodedImg, formObject.screenshotType || "image/png", `${formObject.studentId}_Lab4_1_Screenshot_${new Date().getTime()}.png`);
        const savedImg = folder.createFile(imgBlob);
        savedImg.setSharing(DriveApp.Access.ANYONE_WITH_LINK, DriveApp.Permission.VIEW);
        screenshotUrl = savedImg.getUrl();
        attachScore += 0.5;
      } catch (e) {
        screenshotUrl = "Upload Error: " + e.toString();
      }
    }

    if (formObject.codeBase64) {
      try {
        const decodedCode = Utilities.base64Decode(formObject.codeBase64.split(",")[1]);
        const codeBlob = Utilities.newBlob(decodedCode, formObject.codeFileType || "text/plain", `${formObject.studentId}_Lab4_1_Code_${new Date().getTime()}_${formObject.codeFileName || "solution.ino"}`);
        const savedCode = folder.createFile(codeBlob);
        savedCode.setSharing(DriveApp.Access.ANYONE_WITH_LINK, DriveApp.Permission.VIEW);
        codeFileUrl = savedCode.getUrl();
        attachScore += 0.5;
      } catch (e) {
        codeFileUrl = "Upload Error: " + e.toString();
      }
    }
    attachScore = Math.min(1.0, attachScore);

    // ── 6. Evaluate Conclusion (0.5 Points) ──
    const concl = (formObject.conclusion || "").trim();
    const conclScore = (concl.length > 10) ? 0.5 : (concl.length > 0 ? 0.25 : 0.0);

    // ── Total Score ──
    const totalScore = Number((blankScore + quizScore + challengeScore + qScore + attachScore + conclScore).toFixed(1));

    const rowData = [
      new Date(),
      formObject.studentId,
      formObject.studentName,
      formObject.studentGroup,
      formObject.labDate,
      totalScore,
      blankScore,
      quizScore,
      challengeScore,
      qScore,
      attachScore,
      conclScore,
      formObject.quiz1 || "",
      formObject.quiz2 || "",
      formObject.quiz3 || "",
      formObject.quiz4 || "",
      formObject.quiz5 || "",
      formObject.codeBlank1 || "",
      formObject.codeBlank2 || "",
      formObject.codeBlank3 || "",
      formObject.codeBlank4 || "",
      formObject.codeBlank5 || "",
      formObject.expResult1 || "",
      formObject.expResult2 || "",
      chCode,
      ctrlLogic,
      q1,
      q2,
      q3,
      concl,
      screenshotUrl,
      codeFileUrl
    ];

    sheet.appendRow(rowData);

    return {
      status: "success",
      totalScore: totalScore,
      message: `คะแนนประเมินรวม: ${totalScore.toFixed(1)} / 10.0 คะแนน\nบันทึกข้อมูลเรียบร้อยแล้ว`
    };

  } catch (error) {
    return {
      status: "error",
      message: error.toString()
    };
  }
}

function getOrCreateFolder(folderName) {
  const folders = DriveApp.getFoldersByName(folderName);
  if (folders.hasNext()) {
    return folders.next();
  }
  return DriveApp.createFolder(folderName);
}
