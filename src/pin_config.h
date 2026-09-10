#pragma once

// =====================================================
// Pin Configuration - ESP32-WROOM-32E + ioxesp32 Shield
// =====================================================
// แก้ไขขาตามบอร์ดจริงของคุณ

// RS485 (ioxesp32 Modbus RTU Shield → Serial2)
#define RS485_RX_PIN    16      // GPIO16 = RX2 (รับจาก Shield)
#define RS485_TX_PIN    17      // GPIO17 = TX2 (ส่งไป Shield)
#define RS485_BAUD      4800    // Baud rate ของ Sensor ตัวเดิม

// Relay/MOSFET ควบคุมไฟ 12V ให้ Sensor
// ต่อ Relay Module ที่ขานี้ (HIGH = เปิด, LOW = ปิด)
#define RELAY_12V_PIN   4       // GPIO4 → Relay IN

// LED แสดงสถานะ (ใช้ LED บนบอร์ด ESP32)
#define LED_STATUS_PIN  2       // GPIO2 = Built-in LED

// ปุ่มกด (ถ้ามี)
#define BUTTON_PIN      0       // GPIO0 = Boot Button บน ESP32
