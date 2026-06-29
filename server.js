const express = require('express');
const cors = require('cors');
const bodyParser = require('body-parser');
const fs = require('fs');
const path = require('path');

const app = express();
const PORT = 3000;

// Enable CORS and body size configuration (50mb for base64 files)
app.use(cors());
app.use(bodyParser.json({ limit: '50mb' }));
app.use(bodyParser.urlencoded({ limit: '50mb', extended: true }));

// Setup directory structures
const SUBMISSIONS_DIR = path.join(__dirname, 'submissions');
const DATA_DIR = path.join(SUBMISSIONS_DIR, 'data');
const UPLOADS_DIR = path.join(SUBMISSIONS_DIR, 'uploads');

// Ensure directories exist
if (!fs.existsSync(SUBMISSIONS_DIR)) fs.mkdirSync(SUBMISSIONS_DIR);
if (!fs.existsSync(DATA_DIR)) fs.mkdirSync(DATA_DIR);
if (!fs.existsSync(UPLOADS_DIR)) fs.mkdirSync(UPLOADS_DIR);

// Serve uploads as static resources
app.use('/submissions/uploads', express.static(UPLOADS_DIR));

// GAS Polyfill code block to be injected dynamically
const polyfillScript = `
  <!-- Google Apps Script Local Polyfill -->
  <script>
    if (typeof google === 'undefined') {
      window.google = {
        script: {
          run: {
            sh: null,
            fh: null,
            withSuccessHandler: function(handler) {
              this.sh = handler;
              return this;
            },
            withFailureHandler: function(handler) {
              this.fh = handler;
              return this;
            },
            submitLabData: function(payload) {
              const pathParts = window.location.pathname.split('/');
              const labName = pathParts[pathParts.length - 1] || pathParts[pathParts.length - 2] || 'unknown';
              fetch('/api/submit', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ lab: labName, data: payload })
              })
              .then(res => res.json())
              .then(res => {
                if (res.status === 'success') {
                  if (this.sh) this.sh(res);
                } else {
                  if (this.fh) this.fh(new Error(res.message));
                }
              })
              .catch(err => {
                if (this.fh) this.fh(err);
              });
            }
          }
        }
      };
    }
  </script>
`;

// Heuristic Rules for Grading Student Submissions (Keywords mapping)
const gradingRules = {
  'lab-basic': {
    blankKeywords: ['OUTPUT', 'digitalWrite', 'HIGH', 'LOW', 'delay'],
    challengeKeywords: ['pinMode', 'digitalWrite', 'delay', 'HIGH', 'LOW', '2|led|led_builtin|d5|18|14|gpio14|gpio18'],
    q1Keywords: ['input only|input|อินพุต', '34|35|36|39|a0|gpio16', 'pull-up|pullup', 'pull-down|pulldown'],
    q2Keywords: ['strapping|boot|บูต', '0', '12|15|2|d3|d4|d8|gpio0|gpio12|gpio15|gpio2', 'ดึงกระแส|แรงดัน|ดึง']
  },
  'lab1': {
    blankKeywords: ['OUTPUT', 'OUTPUT', '4095|1023', 'HIGH', 'LOW'],
    challengeKeywords: ['temperature', '30|29.5', 'digitalWrite', 'FAN_RELAY_PIN|fanState|MIST_RELAY_PIN|mistState', '5|13|d7|fan', '23|14|16|d0|d5|mist'],
    q1Keywords: ['12-bit|10-bit|12บิต|10บิต', '4095|1023', 'ความละเอียด|ละเอียด|resolution'],
    q2Keywords: ['ทรานซิสเตอร์|transistor', '3.3V|3.3', '5V|5', 'กระแส|ขับ', 'relay|รีเลย์']
  },
  'lab1.1': {
    blankKeywords: ['INPUT_PULLUP', 'OUTPUT', 'LOW', '!relayState|!fanState|!ledState', 'LOW|LOW'],
    challengeKeywords: ['0|d3|12|BUTTON_PIN', '18|d5|13|d7|RELAY_PIN|FAN_RELAY_PIN', '14|23|16|d0|MIST_RELAY_PIN', 'count|toggleCount', '3', 'longPressTriggered', 'digitalWrite', 'millis'],
    q1Keywords: ['3.3v|3.3', 'gnd|0v|0', 'high', 'low', 'ลอย|float'],
    q2Keywords: ['สั่น|bounce', 'debounce', 'millis', 'หน่วง|delay', 'สัมผัส|contact']
  },
  'lab2': {
    blankKeywords: ['SSD1306_WHITE|white|1', 'display'],
    challengeKeywords: ['display', 'temperature|temp|dht', 'analogRead|analogPercent|knob|rawAnalog', 'relayState|ledState|relay_pin|RELAY_PIN', 'toggleCount|count|press'],
    q1Keywords: ['SDA', 'SCL', 'ข้อมูล', 'clock', 'data', 'อนุกรม', 'จังหวะ'],
    q2Keywords: ['buffer', 'บัฟเฟอร์', 'หน่วยความจำ', 'แสดงผล', 'จอ']
  },
  'lab3': {
    blankKeywords: ['WiFi|ESP8266WiFi', 'LittleFS', 'onNotFound', 'handleClient', 'streamFile'],
    challengeKeywords: ['LittleFS', 'exists', 'open', 'streamFile'],
    q1Keywords: ['LittleFS', 'SPIFFS', 'ความเร็ว', 'เสถียร', 'directory', 'ย่อย'],
    q2Keywords: ['streamFile', 'send', 'RAM', 'หน่วยความจำ', 'สตรีม', 'ขนาดใหญ่']
  },
  'lab4': {
    blankKeywords: ['broadcastTXT', 'toggle_fan', 'toggle_mist', 'WStype_TEXT', 'handleClient'],
    challengeKeywords: ['webSocket', 'tempThreshold|threshold|30', 'broadcastSensorData|broadcast|send', 'containsKey|haskey'],
    q1Keywords: ['real-time', 'polling', 'ดึงข้อมูล', 'ทราฟฟิก', 'แบนด์วิธ', 'รวดเร็ว'],
    q2Keywords: ['โครงสร้าง', 'key', 'value', 'อ่านง่าย', 'parse', 'หลายค่า']
  },
  'lab5': {
    blankKeywords: ['mqttClient', 'publish', 'subscribe', 'callback', 'loop'],
    challengeKeywords: ['mqttClient|mqtt|client', 'publish', 'subscribe', 'callback'],
    q1Keywords: ['broker', 'ผู้รับ', 'ผู้ส่ง', 'publish', 'subscribe', 'ตัวกลาง'],
    q2Keywords: ['หลุด', 'disconnect', 'เตะ', 'ชน', 'ซ้ำ', 'reconnect']
  },
  'lab6': {
    challengeKeywords: ['WiFi', 'connect|reconnect', 'non-blocking|millis|nonblocking', 'webSocket|client', 'elec|elec1234'],
    q1Keywords: ['non-blocking', 'ไม่บล็อก', 'หลุด', 'ค้าง', 'ทำงานต่อ', 'loop'],
    q2Keywords: ['LAN', 'ออฟไลน์', 'เน็ตหลุด', 'เสถียร', 'โรงงาน', 'ปลอดภัย']
  },
  'lab-extra': {
    challengeKeywords: ['warning-blink|warning|blink', 'classList', 'add', 'remove', 'language|lang|th|en'],
    q1Keywords: ['blur', 'เบลอ', 'หลัง', 'กระจก', 'โปร่งแสง'],
    q2Keywords: ['ไอดี', 'id', 'อ้างอิง', 'เข้าถึง', 'ดึง', 'element']
  },
  'lab7': {
    blankKeywords: ['json', 'payload', 'msg', 'change', 'switch'],
    challengeKeywords: ['payload', 'humidity', '20', 'Alert: Dry Climate!|dry|alert'],
    q1Keywords: ['string', 'object', 'แปลง', 'json', 'สตริง', 'JavaScript'],
    q2Keywords: ['เปลี่ยน', 'ON', 'OFF', 'payload', 'ค่า', 'แปลง', 'กำหนด']
  },
  'lab8': {
    blankKeywords: ['time-series', 'Bucket', 'Measurement', 'Tags', 'Fields'],
    challengeKeywords: ['ON|high', 'OFF|low', '1', '0', 'msg.payload'],
    q1Keywords: ['อนุกรมเวลา', 'time-series', 'MySQL', 'เวลา', 'ความถี่', 'เขียนเร็ว'],
    q2Keywords: ['Bucket', 'Measurement', 'Tags', 'Fields', 'ถัง', 'ตาราง', 'ดัชนี']
  },
  'lab9': {
    blankKeywords: ['from', 'bucket', 'range', 'mean', 'aggregateWindow'],
    challengeKeywords: ['from', 'bucket', 'range', 'mean', 'aggregateWindow', '30%|30'],
    q1Keywords: ['ควบคุม', 'เรียลไทม์', 'ย้อนหลัง', 'วิเคราะห์', 'ระยะยาว', 'วิศวกร'],
    q2Keywords: ['aggregateWindow', 'windowPeriod', 'ลดขนาด', 'กลุ่ม', 'เฉลี่ย']
  }
};

// Auto-grading evaluation engine
function gradeSubmission(lab, data) {
  const rules = gradingRules[lab];
  const maxScore = 10.0;
  
  let challengeScore = 0.0;
  let q1Score = 0.0;
  let q2Score = 0.0;
  let attachmentScore = 0.0;
  
  const feedbackDetails = [];

  if (!rules) {
    return {
      score: 10.0,
      maxScore: 10.0,
      breakdown: { challenge: 4.0, q1: 3.0, q2: 3.0, attachments: 0 },
      feedback: "ระบบยังไม่มีกฎเกณฑ์คำนวณคะแนนสำหรับใบงานนี้ ได้คะแนนเริ่มต้น 10.0"
    };
  }

  // 1. Grade Code / Challenge (4.0 points max)
  // 1.1 Grade Skeleton Blanks (1.5 points max if blanks exist, otherwise challenge gets full 4.0 points)
  const hasBlanks = !!rules.blankKeywords && rules.blankKeywords.length > 0;
  let skeletonScore = 0.0;
  
  if (hasBlanks) {
    const codeContent = (data.codeBlank1 || '') + ' ' + (data.codeBlank2 || '') + ' ' + (data.codeBlank3 || '') + ' ' + (data.codeBlank4 || '') + ' ' + (data.codeBlank5 || '');
    if (codeContent.replace(/\s+/g, '').length > 0) {
      let matchedBlanks = 0;
      rules.blankKeywords.forEach(kw => {
        const subKws = kw.split('|');
        const isMatched = subKws.some(subKw => codeContent.toLowerCase().indexOf(subKw.toLowerCase()) !== -1);
        if (isMatched) {
          matchedBlanks++;
        }
      });
      skeletonScore = (matchedBlanks / rules.blankKeywords.length) * 1.5;
      feedbackDetails.push(`- เติมคำตอบโครงร่างโค้ด: ถูกต้องตรงประเด็น ${matchedBlanks}/${rules.blankKeywords.length} ส่วนหลัก (+${skeletonScore.toFixed(1)}/1.5 คะแนน)`);
    } else {
      feedbackDetails.push(`- เติมคำตอบโครงร่างโค้ด: ไม่พบการส่งคำตอบ (+0.0/1.5 คะแนน)`);
    }
  }

  // 1.2 Grade Pasted Challenge Code (2.5 points max if blanks exist, otherwise 4.0 points max)
  const challengeMax = hasBlanks ? 2.5 : 4.0;
  const challengeCodeText = data.challengeCode || '';
  if (challengeCodeText.trim().length > 0) {
    let matchedChallenge = 0;
    rules.challengeKeywords.forEach(kw => {
      const subKws = kw.split('|');
      const isMatched = subKws.some(subKw => challengeCodeText.toLowerCase().indexOf(subKw.toLowerCase()) !== -1);
      if (isMatched) {
        matchedChallenge++;
      }
    });
    challengeScore = (matchedChallenge / rules.challengeKeywords.length) * challengeMax;
    feedbackDetails.push(`- โจทย์ท้าทาย (Challenge Code): ตรวจพบความสอดคล้องของตรรกะ ${matchedChallenge}/${rules.challengeKeywords.length} จุด (+${challengeScore.toFixed(1)}/${challengeMax.toFixed(1)} คะแนน)`);
  } else {
    feedbackDetails.push(`- โจทย์ท้าทาย (Challenge Code): ไม่พบการส่งโค้ดคำตอบ (+0.0/${challengeMax.toFixed(1)} คะแนน)`);
  }
  
  const totalCodeScore = parseFloat((skeletonScore + challengeScore).toFixed(1));

  // 2. Grade Question 1 (2.0 points, or 4.0 points if Q2 does not exist)
  const q1Text = data.question1 || '';
  const q2Text = data.question2 || '';
  const hasQ2 = !!rules.q2Keywords && rules.q2Keywords.length > 0;
  const q1Max = hasQ2 ? 2.0 : 4.0;
  
  if (q1Text.trim().length > 0) {
    let matchedQ1 = 0;
    rules.q1Keywords.forEach(kw => {
      const subKws = kw.split('|');
      const isMatched = subKws.some(subKw => q1Text.toLowerCase().indexOf(subKw.toLowerCase()) !== -1);
      if (isMatched) {
        matchedQ1++;
      }
    });
    
    if (rules.q1Keywords.length > 0) {
      q1Score = (matchedQ1 / rules.q1Keywords.length) * q1Max;
      feedbackDetails.push(`- คำถามข้อที่ 1: ตรวจพบคำอธิบายตรงประเด็น ${matchedQ1}/${rules.q1Keywords.length} จุดหลัก (+${q1Score.toFixed(1)}/${q1Max.toFixed(1)} คะแนน)`);
    } else {
      q1Score = q1Max;
      feedbackDetails.push(`- คำถามข้อที่ 1: ตอบคำถามเรียบร้อย (+${q1Max.toFixed(1)}/${q1Max.toFixed(1)} คะแนน)`);
    }
  } else {
    feedbackDetails.push(`- คำถามข้อที่ 1: ไม่พบการตอบคำถาม (+0.0/${q1Max.toFixed(1)} คะแนน)`);
  }

  // 3. Grade Question 2 (2.0 points max)
  if (hasQ2) {
    if (q2Text.trim().length > 0) {
      let matchedQ2 = 0;
      rules.q2Keywords.forEach(kw => {
        const subKws = kw.split('|');
        const isMatched = subKws.some(subKw => q2Text.toLowerCase().indexOf(subKw.toLowerCase()) !== -1);
        if (isMatched) {
          matchedQ2++;
        }
      });
      
      q2Score = (matchedQ2 / rules.q2Keywords.length) * 2.0;
      feedbackDetails.push(`- คำถามข้อที่ 2: ตรวจพบคำอธิบายตรงประเด็น ${matchedQ2}/${rules.q2Keywords.length} จุดหลัก (+${q2Score.toFixed(1)}/2.0 คะแนน)`);
    } else {
      feedbackDetails.push("- คำถามข้อที่ 2: ไม่พบการตอบคำถาม (+0.0/2.0 คะแนน)");
    }
  }

  // 4. Grade Attachments (2.0 points max)
  const screenshotOk = (data.screenshotBase64 && data.screenshotName) ? 1.0 : 0.0;
  const codeOk = (data.codeBase64 && data.codeFileName) ? 1.0 : 0.0;
  attachmentScore = screenshotOk + codeOk;
  
  feedbackDetails.push(`- แนบหลักฐาน: แนบรูปผลลัพธ์ ${screenshotOk ? "แล้ว (+1.0)" : "ไม่พบ (+0.0)"}, แนบไฟล์โค้ด ${codeOk ? "แล้ว (+1.0)" : "ไม่พบ (+0.0)"} (+${attachmentScore.toFixed(1)}/2.0 คะแนน)`);

  const finalScore = parseFloat((totalCodeScore + q1Score + q2Score + attachmentScore).toFixed(1));

  return {
    score: finalScore,
    maxScore: maxScore,
    breakdown: {
      challenge: totalCodeScore,
      q1: parseFloat(q1Score.toFixed(1)),
      q2: parseFloat(q2Score.toFixed(1)),
      attachments: parseFloat(attachmentScore.toFixed(1))
    },
    feedback: feedbackDetails.join('\n')
  };
}

// Helper to inject polyfill into HTML
function serveInjectedHtml(labPath, res) {
  const filePath = path.join(__dirname, labPath, 'index.html');
  if (!fs.existsSync(filePath)) {
    return res.status(404).send('<h1>404 - ไม่พบหน้าเว็บของใบงาน</h1>');
  }

  let htmlContent = fs.readFileSync(filePath, 'utf8');
  if (htmlContent.includes('</head>')) {
    htmlContent = htmlContent.replace('</head>', `${polyfillScript}\n</head>`);
  } else if (htmlContent.includes('<body>')) {
    htmlContent = htmlContent.replace('<body>', `<body>\n${polyfillScript}`);
  } else {
    htmlContent = polyfillScript + htmlContent;
  }
  res.send(htmlContent);
}

// Redirect root to dashboard
app.get('/', (req, res) => {
  res.redirect('/dashboard');
});

// Serve the teacher's dashboard
app.get('/dashboard', (req, res) => {
  res.sendFile(path.join(__dirname, 'dashboard.html'));
});

// Routing for lab index pages
const validLabs = ['lab-basic', 'lab1', 'lab1.1', 'lab2', 'lab3', 'lab4', 'lab5', 'lab6', 'lab-extra', 'lab7', 'lab8', 'lab9'];
validLabs.forEach(lab => {
  app.get(`/${lab}`, (req, res) => {
    serveInjectedHtml(lab, res);
  });
});

// Submissions API POST: Handles file uploads, auto-grading, and logging
app.post('/api/submit', (req, res) => {
  const { lab, data } = req.body;

  if (!lab || !data) {
    return res.status(400).json({ status: 'error', message: 'ข้อมูลไม่ครบถ้วน' });
  }

  try {
    const labUploadsDir = path.join(UPLOADS_DIR, lab);
    if (!fs.existsSync(labUploadsDir)) {
      fs.mkdirSync(labUploadsDir, { recursive: true });
    }

    let screenshotUrl = 'ไม่ได้แนบไฟล์';
    let codeFileUrl = 'ไม่ได้แนบไฟล์';

    // Decode and save screenshot
    if (data.screenshotBase64 && data.screenshotName) {
      const sanitizedName = data.screenshotName.replace(/[^a-zA-Z0-9.\-_]/g, '_');
      const filename = `${data.studentId}_${data.studentName.replace(/\s+/g, '_')}_screenshot_${sanitizedName}`;
      const base64Data = data.screenshotBase64.split(',')[1];
      const buffer = Buffer.from(base64Data, 'base64');
      fs.writeFileSync(path.join(labUploadsDir, filename), buffer);
      screenshotUrl = `/submissions/uploads/${lab}/${filename}`;
    }

    // Decode and save code file
    if (data.codeBase64 && data.codeFileName) {
      const sanitizedName = data.codeFileName.replace(/[^a-zA-Z0-9.\-_]/g, '_');
      const filename = `${data.studentId}_${data.studentName.replace(/\s+/g, '_')}_code_${sanitizedName}`;
      const base64Data = data.codeBase64.split(',')[1];
      const buffer = Buffer.from(base64Data, 'base64');
      fs.writeFileSync(path.join(labUploadsDir, filename), buffer);
      codeFileUrl = `/submissions/uploads/${lab}/${filename}`;
    }

    // Run Auto-Grading Engine
    const grading = gradeSubmission(lab, data);

    // Prepare log record
    const submissionRecord = {
      timestamp: new Date().toISOString(),
      studentName: data.studentName,
      studentId: data.studentId,
      studentGroup: data.studentGroup,
      labDate: data.labDate,
      codeBlank1: data.codeBlank1 || '',
      codeBlank2: data.codeBlank2 || '',
      codeBlank3: data.codeBlank3 || '',
      codeBlank4: data.codeBlank4 || '',
      codeBlank5: data.codeBlank5 || '',
      challengeCode: data.challengeCode || '',
      controlLogic: data.controlLogic || '',
      challengeLogic: data.challengeLogic || '',
      question1: data.question1 || '',
      question2: data.question2 || '',
      screenshotUrl: screenshotUrl,
      codeFileUrl: codeFileUrl,
      conclusion: data.conclusion || '',
      
      // Auto-grading fields
      score: grading.score,
      maxScore: grading.maxScore,
      gradingBreakdown: grading.breakdown,
      feedback: grading.feedback,
      isManuallyGraded: false
    };

    // Load existing database logs
    const logFilePath = path.join(DATA_DIR, `${lab}.json`);
    let logData = [];
    if (fs.existsSync(logFilePath)) {
      const raw = fs.readFileSync(logFilePath, 'utf8');
      logData = JSON.parse(raw);
    }

    // Check for previous submission by this student ID in this lab, and replace it (to allow re-submission)
    const existingIndex = logData.findIndex(s => s.studentId === data.studentId);
    if (existingIndex !== -1) {
      logData[existingIndex] = submissionRecord;
    } else {
      logData.push(submissionRecord);
    }

    fs.writeFileSync(logFilePath, JSON.stringify(logData, null, 2), 'utf8');

    res.json({
      status: 'success',
      message: `บันทึกรายงานใบงาน (${lab}) เรียบร้อย! ตรวจคะแนนอัตโนมัติสำเร็จ: ${grading.score}/${grading.maxScore} คะแนน\n\nรายละเอียดคะแนน:\n${grading.feedback}`
    });

  } catch (error) {
    console.error('Error handling submission:', error);
    res.status(500).json({ status: 'error', message: 'เกิดข้อผิดพลาดในการประมวลผลเซิร์ฟเวอร์: ' + error.toString() });
  }
});

// GET all submissions logs
app.get('/api/submissions', (req, res) => {
  try {
    const allLogs = {};
    validLabs.forEach(lab => {
      const logFilePath = path.join(DATA_DIR, `${lab}.json`);
      if (fs.existsSync(logFilePath)) {
        const raw = fs.readFileSync(logFilePath, 'utf8');
        allLogs[lab] = JSON.parse(raw);
      } else {
        allLogs[lab] = [];
      }
    });
    res.json(allLogs);
  } catch (error) {
    res.status(500).json({ status: 'error', message: error.toString() });
  }
});

// POST API to override / modify score manually
app.post('/api/override-score', (req, res) => {
  const { lab, studentId, score, feedback } = req.body;

  if (!lab || !studentId || score === undefined) {
    return res.status(400).json({ status: 'error', message: 'ข้อมูลสำหรับการปรับคะแนนไม่ครบถ้วน' });
  }

  try {
    const logFilePath = path.join(DATA_DIR, `${lab}.json`);
    if (!fs.existsSync(logFilePath)) {
      return res.status(404).json({ status: 'error', message: 'ไม่พบฐานข้อมูลใบงานนี้' });
    }

    const raw = fs.readFileSync(logFilePath, 'utf8');
    const logData = JSON.parse(raw);

    const recordIndex = logData.findIndex(s => s.studentId === studentId);
    if (recordIndex === -1) {
      return res.status(404).json({ status: 'error', message: 'ไม่พบรายงานส่งงานของรหัสนักศึกษานี้' });
    }

    // Update score and override status
    logData[recordIndex].score = parseFloat(score);
    logData[recordIndex].feedback = feedback || 'ปรับคะแนนโดยคุณครูผู้ตรวจ';
    logData[recordIndex].isManuallyGraded = true;

    fs.writeFileSync(logFilePath, JSON.stringify(logData, null, 2), 'utf8');

    res.json({
      status: 'success',
      message: 'ปรับเปลี่ยนผลคะแนนและข้อเสนอแนะเรียบร้อยแล้ว'
    });

  } catch (error) {
    res.status(500).json({ status: 'error', message: error.toString() });
  }
});

// Start listening
app.listen(PORT, () => {
  console.log('\n==================================================');
  console.log('🚀 IoT Worksheets Local Simulation Server is Running!');
  console.log('✅ ระบบตรวจประเมินคะแนนอัตโนมัติ (Auto-Grading Engine) พร้อมทำงาน');
  console.log(`📡 URL สำหรับใช้งานของนักศึกษา:`);
  validLabs.forEach(lab => {
    console.log(`   - http://localhost:${PORT}/${lab}`);
  });
  console.log('--------------------------------------------------');
  console.log(`📊 หน้าแดชบอร์ดคุณครู (Teacher Portal):`);
  console.log(`   - http://localhost:${PORT}/dashboard`);
  console.log('==================================================\n');
});
