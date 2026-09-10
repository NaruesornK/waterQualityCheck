// =====================================================
// Water Quality Check
// ESP32-WROOM-32E + ioxesp32 Modbus RTU Shield
// =====================================================
// ย้ายจาก Tinker C6 → ESP32 ทั่วไป
// - RS485 ผ่าน Serial2 (GPIO16/17) + MAX13487E Auto-direction
// - WiFiManager ตั้งค่า WiFi ผ่าน AP Mode
// - ThingsBoard MQTT ส่งข้อมูลทุก 10 นาที
// - Relay ควบคุม 12V ให้ Sensor
// =====================================================

#include <Arduino.h>
#include "RS485_sensor.h" 
#include "Wifi_Manager.h" 
#include "Things_Board.h"

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("╔═══════════════════════════════════════╗");
    Serial.println("║  Water Quality Check - ESP32 Edition  ║");
    Serial.println("╚═══════════════════════════════════════╝");

    LedSet();
    WiFiSet();

    setupWaterSensor();
    readSensorFirstTime();
    ThingBoard_Setup();
}

void loop() {
    SensorUpdate();
    ThingBoard_Update();
    updateLED();
    Sw_press();
}
