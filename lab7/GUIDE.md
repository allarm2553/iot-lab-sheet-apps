# 🔴 ใบงานที่ 7: การเชื่อมโยงข้อมูลและการสร้างแดชบอร์ดด้วย Node-RED

คู่มือการทดลองใช้งานแพลตฟอร์ม **Node-RED** ในการสร้าง Flow การไหลของข้อมูลจาก **MQTT Broker** การแปลงข้อมูล JSON การประมวลผลด้วย JavaScript Function และการออกแบบหน้าจอควบคุม **Node-RED Dashboard**

---

## 🎯 วัตถุประสงค์ (Objectives)

1. เข้าใจหลักการทำงานแบบ **Flow-Based Programming** และสถาปัตยกรรมของ Node-RED
2. สามารถรับส่งข้อมูลกับ MQTT Broker ผ่านโหนด `mqtt in` และ `mqtt out`
3. สามารถแปลงข้อความสตริง JSON ให้เป็น JavaScript Object ด้วยโหนด `json`
4. สามารถเขียนฟังก์ชัน JavaScript ในโหนด `function` เพื่อตรวจสอบและประมวลผลข้อมูลเงื่อนไข (เช่น แจ้งเตือนเมื่อความชื้น < 20%)
5. สามารถสร้าง UI Interactive Dashboard ด้วยชุดโหนด `node-red-dashboard` (Gauge, Chart, Switch)

---

## 🏗️ โครงสร้าง Flow ใน Node-RED

```
[mqtt in] ──► [json node] ──┬──► [change node] ──► [ui_gauge (Temp)]
                            ├──► [change node] ──► [ui_chart (Humidity)]
                            └──► [function node] ──► [ui_toast / alert]
                                 (Check Humidity < 20%)

[ui_switch] ──► [change node] ──► [mqtt out]
                (Set payload "ON"/"OFF")
```

---

## ✍️ เฉลยคำตอบในใบงาน (Worksheet Answers)

### 1. โค้ด JavaScript ในโหนด Function (ตรวจสอบความชื้น < 20%)
```javascript
const humidity = parseFloat(msg.payload.humidity);
if (humidity < 20) {
    msg.payload = "Alert: Dry Climate! (Humidity: " + humidity + "%)";
    return msg;
}
return null;
```

---

### 2. คำถามสรุปผลการทดลอง (Review Questions)

**คำถามที่ 1: โหนด `json` ใน Node-RED ทำหน้าที่อะไร และทำไมจึงจำเป็นต้องใช้ต่อท้ายโหนด `mqtt in`?**
> **แนวคำตอบ:** โหนด `json` ทำหน้าที่แปลงข้อมูลแบบ Two-way Conversion คือแปลงข้อความ JSON String ที่รับเข้ามาจาก MQTT Broker ให้กลายเป็น JavaScript Object ทำให้โหนดถัดไปสามารถเข้าถึงค่าตัวแปรย่อยได้โดยตรงผ่าน `msg.payload.temperature` หรือ `msg.payload.humidity` หากไม่แปลง ข้อมูลจะยังเป็นข้อความก้อนเดียวที่นำไปพลอตกราฟหรือคำนวณต่อไม่ได้

**คำถามที่ 2: โหนด `change` มีประโยชน์อย่างไรในการตั้งค่า ON/OFF เมื่อเชื่อมต่อเข้ากับโหนด `ui_switch` ก่อนส่งออกไปยังโหนด `mqtt out`?**
> **แนวคำตอบ:** โหนด `change` ช่วยจัดระเบียบและแปลงค่า `msg.payload` (เช่น แปลงค่า Boolean `true/false` จากสวิตช์บนแดชบอร์ด ให้กลายเป็นข้อความสตริง `"ON"` หรือ `"OFF"`) ก่อนส่งไปยัง MQTT Topic ที่ฮาร์ดแวร์ปลายทางรอรับอยู่ ทำให้รูปแบบคำสั่งตรงกับที่บอร์ด ESP32 คาดหวัง
