/**
 * Google Apps Script - Auto Grading & Submission Backend
 * Lab 3.2: การพัฒนาระบบตั้งค่า Wi-Fi ผ่านหน้าเว็บ UI (Web Wi-Fi Configurator & Reset Button)
 * 
 * Score Breakdown (Total 10.0 Points):
 * 1. Code Skeleton Blanks (5 blanks)    : 1.5 Points
 * 2. Multiple Choice Quiz (5 questions) : 2.0 Points (0.4 pts each)
 * 3. Challenge Code Solution            : 2.5 Points
 * 4. Post-Lab Analytical Questions (3)  : 2.5 Points
 * 5. Attachments (Screenshot & Code)    : 1.0 Points (0.5 pts each)
 * 6. Conclusion & Remarks               : 0.5 Points
 */

const SHEET_NAME = "Lab 3.2 Submissions";
const DRIVE_FOLDER_NAME = "IoT_Lab3_2_Attachments";

// Quiz Answer Keys
const QUIZ_ANSWER_KEYS = {
  quiz1: "1b", // WIFI_AP_STA / SoftAP configuration
  quiz2: "2d", // DNS Server port 53 for captive portal redirection
  quiz3: "3a", // /config.json persistent storage in LittleFS
  quiz4: "4c", // Non-blocking millis() long-press detection
  quiz5: "5a"  // LittleFS.remove() and factory reset workflow
};

function doGet(e) {
  return HtmlService.createTemplateFromFile("index")
    .evaluate()
    .setTitle("ใบงานที่ 3.2: การพัฒนาระบบตั้งค่า Wi-Fi ผ่านหน้าเว็บ UI (Web Wi-Fi Configurator)")
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
        "Challenge Code", "Control Logic",
        "Question 1 (Captive Portal)", "Question 2 (LittleFS JSON)", "Question 3 (Long-Press Reset)",
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

    // ── 2. Evaluate Multiple Choice Quiz (2.0 Points) ──
    let correctQuiz = 0;
    for (let qKey in QUIZ_ANSWER_KEYS) {
      if ((formObject[qKey] || "").trim() === QUIZ_ANSWER_KEYS[qKey]) {
        correctQuiz++;
      }
    }
    const quizScore = Math.min(2.0, (correctQuiz / 5.0) * 2.0);

    // ── 3. Evaluate Challenge Code & Control Logic (2.5 Points) ──
    const chCode = (formObject.challengeCode || "").trim();
    const ctrlLogic = (formObject.controlLogic || "").trim();
    let challengeScore = 0.0;
    if (chCode.length > 25) challengeScore += 1.5;
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
        const imgBlob = Utilities.newBlob(decodedImg, formObject.screenshotType || "image/png", `${formObject.studentId}_Lab3_2_Screenshot_${new Date().getTime()}.png`);
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
        const codeBlob = Utilities.newBlob(decodedCode, formObject.codeFileType || "text/plain", `${formObject.studentId}_Lab3_2_Code_${new Date().getTime()}_${formObject.codeFileName || "solution.ino"}`);
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

    // Total Score
    const totalScore = parseFloat((blankScore + quizScore + challengeScore + qScore + attachScore + conclScore).toFixed(1));

    // Append to Sheet
    const rowData = [
      new Date(),
      formObject.studentId || "-",
      formObject.studentName || "-",
      formObject.studentGroup || "-",
      formObject.labDate || "-",
      totalScore,
      parseFloat(blankScore.toFixed(1)),
      parseFloat(quizScore.toFixed(1)),
      parseFloat(challengeScore.toFixed(1)),
      parseFloat(qScore.toFixed(1)),
      parseFloat(attachScore.toFixed(1)),
      parseFloat(conclScore.toFixed(1)),
      formObject.quiz1 || "-",
      formObject.quiz2 || "-",
      formObject.quiz3 || "-",
      formObject.quiz4 || "-",
      formObject.quiz5 || "-",
      blanks[0] || "-",
      blanks[1] || "-",
      blanks[2] || "-",
      blanks[3] || "-",
      blanks[4] || "-",
      chCode || "-",
      ctrlLogic || "-",
      q1 || "-",
      q2 || "-",
      q3 || "-",
      concl || "-",
      screenshotUrl || "-",
      codeFileUrl || "-"
    ];

    sheet.appendRow(rowData);

    return {
      status: "success",
      score: totalScore,
      message: `บันทึกรายงานผลการทดลองเรียบร้อยแล้ว! คะแนนรวมที่ได้: ${totalScore.toFixed(1)} / 10.0 คะแนน`
    };

  } catch (error) {
    return {
      status: "error",
      message: "เกิดข้อผิดพลาดในการบันทึกข้อมูล: " + error.toString()
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
