# 🎨 ใบงานเสริมปฏิบัติการ: การพัฒนา Web Dashboard ด้วย HTML, CSS และ JavaScript

คู่มือการทดลองออกแบบและพัฒนาเว็บแดชบอร์ดสไตล์ **Glassmorphism (กระจกโปร่งแสง)** ด้วย HTML5, Modern CSS และ JavaScript สำหรับควบคุมอุปกรณ์และแสดงผลข้อมูลจากระบบ IoT

---

## 🎯 วัตถุประสงค์ (Objectives)

1. เข้าใจโครงสร้างของหน้าเว็บ Responsive Web Design ด้วย **Semantic HTML5** และ **CSS Grid / Flexbox**
2. สามารถออกแบบหน้าเว็บสไตล์โมเดิร์น **Glassmorphism UI** โดยใช้ CSS Variables, ความโปร่งแสง (`rgba`), และเอฟเฟกต์เบลอพื้นหลัง (`backdrop-filter: blur()`)
3. สามารถควบคุมและอัพเดทโครงสร้างเอกสาร (**DOM Manipulation**) ด้วย JavaScript (`document.getElementById`, `classList.add / remove`, `innerHTML`)
4. สามารถพัฒนาฟังก์ชันเสริมความปลอดภัยและการใช้งาน เช่น การแจ้งเตือนสถานะกระพริบฉุกเฉิน (Warning Blink Effect) และระบบสลับภาษา (Multi-language Support)

---

## 📁 โครงสร้างโปรเจกต์เว็บแดชบอร์ด

```text
solution/
 ├── index.html     (โครงสร้างหน้าเว็บ UI และส่วนประกอบการ์ดเซ็นเซอร์)
 ├── styles.css     (สไตล์ CSS Glassmorphism และ Responsive Layout)
 └── app.js         (การจัดการอีเวนต์, DOM manipulation, และการสลับภาษา)
```

---

## ✍️ เฉลยคำตอบในใบงาน (Worksheet Answers)

### 1. โค้ด JavaScript โจทย์ท้าทาย (การเพิ่ม Warning Blink และสลับภาษา)
```javascript
// ฟังก์ชันแจ้งเตือนกระพริบเตือนภัยเมื่อค่าเกินเกณฑ์
function updateWarningState(elementId, isWarning) {
    const el = document.getElementById(elementId);
    if (isWarning) {
        el.classList.add('warning-blink');
    } else {
        el.classList.remove('warning-blink');
    }
}

// ฟังก์ชันสลับภาษา (TH / EN)
function setLanguage(lang) {
    const elements = document.querySelectorAll('[data-lang-' + lang + ']');
    elements.forEach(el => {
        el.textContent = el.getAttribute('data-lang-' + lang);
    });
}
```

---

### 2. คำถามสรุปผลการทดลอง (Review Questions)

**คำถามที่ 1: คุณสมบัติ `backdrop-filter: blur(8px)` ใน CSS ส่งผลอย่างไรต่อการแสดงผลแดชบอร์ดแบบ Glassmorphism?**
> **แนวคำตอบ:** `backdrop-filter: blur()` ทำหน้าที่เบลอภาพขององค์ประกอบกราฟิกหรือเลเยอร์ที่อยู่ด้านหลังการ์ดหรือคอนเทนเนอร์นั้นๆ เมื่อใช้ร่วมกับสีพื้นหลังที่มีความโปร่งแสง เช่น `rgba(30, 41, 59, 0.7)` และเส้นขอบสีขาวบางๆ จะทำให้เกิดเอฟเฟกต์เหมือนแผ่นกระจกฝ้าโปร่งแสง ช่วยให้อ่านตัวหนังสือด้านบนได้ชัดเจนและเพิ่มมิติความสวยงามแบบโมเดิร์น

**คำถามที่ 2: ฟังก์ชัน `document.getElementById()` มีหน้าที่อะไร และทำไมต้องประกาศตัวแปรอ้างอิงตรงกับ ID ใน HTML?**
> **แนวคำตอบ:** ทำหน้าที่ค้นหาและดึงโหนดของ Element ในโครงสร้าง DOM (Document Object Model) ที่มีแอตทริบิวต์ `id` ตรงกับที่ระบุ เพื่อให้ JavaScript สามารถอ่านค่า แก้ไขเนื้อหา (เช่น ตัวเลขอุณหภูมิ) หรือเพิ่มคลาสควบคุมสไตล์ CSS ได้ การตั้ง ID ให้ตรงกันเป็นสิ่งจำเป็นเพราะ ID ในหน้า HTML ต้องมีความเป็นเอกลักษณ์ (Unique) ป้องกันการชี้เป้าผิดองค์ประกอบ
