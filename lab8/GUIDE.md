# 🗄️ ใบงานที่ 8: การบันทึกสถิติมิติตามเวลาด้วย InfluxDB (Data Logging)

คู่มือการทดลองติดตั้งและส่งข้อมูลมิติตามเวลา (Time-Series Data) จากไมโครคอนโทรลเลอร์ผ่าน **Node-RED** เข้าสู่ฐานข้อมูล **InfluxDB** เพื่อบันทึกประวัติการทำงานของเซ็นเซอร์และสถานะรีเลย์อย่างมีประสิทธิภาพ

---

## 🎯 วัตถุประสงค์ (Objectives)

1. เข้าใจสถาปัตยกรรมและคุณสมบัติของฐานข้อมูลอนุกรมเวลา (**Time-Series Database**)
2. เข้าใจโครงสร้างข้อมูล (Data Schema) ของ InfluxDB ได้แก่ **Bucket**, **Measurement**, **Tags (Indexed metadata)** และ **Fields (Actual values)**
3. สามารถใช้งานโหนด `node-red-contrib-influxdb` เพื่อเขียนข้อมูลเซ็นเซอร์ลงฐานข้อมูล InfluxDB v2
4. สามารถเขียนสคริปต์ JavaScript ในโหนด Function เพื่อแปลงสถานะข้อความ `"ON"`/`"OFF"` ให้เป็นเลขจำนวนเต็ม `1`/`0` สำหรับบันทึกสถานะรีเลย์ลงฐานข้อมูล

---

## 📊 โครงสร้าง Data Schema ของ InfluxDB

| องค์ประกอบ | คำอธิบาย | ตัวอย่างในการทดลอง |
| :--- | :--- | :--- |
| **Bucket** | ถังจัดเก็บข้อมูลหลักที่กำหนดระยะเวลาการเก็บ (Retention Policy) | `iot_bucket` |
| **Measurement** | ชื่อตารางวัดค่า (เปรียบเสมือน Table ใน RDBMS) | `climate_sensor` |
| **Tags** | ข้อมูลระบุคุณลักษณะที่ถูกทำ Index (ค้นหาไว เหมาะสำหรับ filter) | `device_id="esp32_01"`, `location="greenhouse"` |
| **Fields** | ค่าตัวเลขที่บันทึกจริงตามเวลา (ไม่ทำ Index) | `temperature=28.5`, `humidity=65.2`, `relay_fan=1` |
| **Timestamp** | เวลาที่ข้อมูลเกิดขึ้นจริง (Nanosecond precision) | `2026-08-15T16:30:00Z` |

---

## ✍️ เฉลยคำตอบในใบงาน (Worksheet Answers)

### 1. โค้ด JavaScript ในโหนด Function (แปลงสถานะ ON/OFF เป็น 1/0)
```javascript
let statusVal = (msg.payload === 'ON' || msg.payload === 'HIGH' || msg.payload === 1 || msg.payload === true) ? 1 : 0;

msg.payload = [{
    measurement: "relay_state",
    fields: {
        fan: statusVal
    },
    tags: {
        device: "esp32_climate_node"
    }
}];

return msg;
```

---

### 2. คำถามสรุปผลการทดลอง (Review Questions)

**คำถามที่ 1: เหตุใดการเก็บข้อมูลเซ็นเซอร์ประเภทอุณหภูมิและความชื้นจึงนิยมเก็บลงบนฐานข้อมูล Time-Series (เช่น InfluxDB) มากกว่า MySQL?**
> **แนวคำตอบ:** ฐานข้อมูล Time-Series ถูกออกแบบมาเฉพาะสำหรับข้อมูลที่ผูกกับเวลาและมีการเขียนแบบเรียลไทม์ด้วยความถี่สูง (High Write Throughput) มีอัลกอริทึมการบีบอัดข้อมูล (Compression) ที่ทำให้ใช้พื้นที่จัดเก็บน้อยกว่าตารางเชิงสัมพันธ์ (RDBMS) ถึง 10 เท่า และมีฟังก์ชันการคำนวณสถิติอนุกรมเวลา (เช่น Rolling Average, Aggregate Window) ในตัวที่ประมวลผลได้รวดเร็วกว่า SQL ทั่วไปมาก

**คำถามที่ 2: โครงสร้างข้อมูล (Data Schema) ของ InfluxDB ประกอบด้วย Bucket, Measurement, Tags, และ Fields อย่างไร?**
> **แนวคำตอบ:** 
> - **Bucket:** ภาชนะบรรจุข้อมูลระดับบนสุด กำหนดระยะเวลาการเก็บรักษาข้อมูล (Retention Period)
> - **Measurement:** ชื่อหัวข้อหรือกลุ่มข้อมูลของการวัดค่า เช่น `environment` หรือ `actuators`
> - **Tags:** ข้อมูลกำกับ (Metadata) เป็น Key-Value ที่ถูกทำ Index ช่วยให้การ Query และ Filter ข้อมูลทำได้อย่างรวดเร็ว เช่น ชื่ออุปกรณ์ หรือสถานที่
> - **Fields:** ข้อมูลค่าจริงที่วัดได้ เช่น ตัวเลขค่าอุณหภูมิ ความชื้น หรือแรงดันไฟฟ้า ซึ่งไม่มีการทำ Index
