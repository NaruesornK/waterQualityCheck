# Water Quality Check

ระบบตรวจวัดคุณภาพน้ำแบบอัตโนมัติ ส่งข้อมูลผ่าน MQTT ไปยัง ThingsBoard

## Features

- อ่านค่า RS485 Modbus Sensor (ความขุ่น + อุณหภูมิ)
- WiFiManager ตั้งค่า WiFi ผ่าน AP Mode
- MQTT ส่งข้อมูลไป ThingsBoard ทุก 10 นาที
- ประหยัดพลังงาน (เปิด WiFi เฉพาะตอนส่ง, ควบคุม 12V Sensor)
- ปุ่มกดค้าง 5 วินาทีรีเซ็ต WiFi

## Hardware

| อุปกรณ์ | รายละเอียด |
|---------|-----------|
| MCU | ESP32-WROOM-32E |
| RS485 Shield | ioxesp32 Modbus RTU Shield (MAX13487E Auto-direction) |
| Sensor | RS485 Modbus Turbidity + Temperature |
| Relay | Module ควบคุมไฟ 12V ให้ Sensor |

## Pinout

| Pin | Function |
|-----|----------|
| GPIO16 | RS485 RX (Serial2 ← Shield) |
| GPIO17 | RS485 TX (Serial2 → Shield) |
| GPIO4 | Relay ควบคุม 12V |
| GPIO2 | LED แสดงสถานะ |
| GPIO0 | ปุ่มกด (Boot Button) |

> **หมายเหตุ:** แก้ไขขาได้ที่ไฟล์ `src/pin_config.h`

## Libraries (PlatformIO)

- PubSubClient
- ArduinoJson
- ModbusMaster
- WiFiManager

## Build & Upload

```bash
# Build
pio run

# Upload
pio run --target upload

# Serial Monitor
pio device monitor
```

## การใช้งาน

1. เสียบ USB → ESP32 จะสร้าง WiFi AP ชื่อ **"WaterQ-ESP32"**
2. เชื่อมต่อ AP แล้วตั้งค่า WiFi ผ่านหน้าเว็บ `192.168.4.1`
3. ESP32 จะอ่านค่าจาก Sensor แล้วส่งไป ThingsBoard ทุก 10 นาที
4. **กดปุ่มค้าง 5 วินาที** → รีเซ็ต WiFi (เข้า AP Mode ใหม่)
