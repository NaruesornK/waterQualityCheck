#include <ModbusMaster.h>
#include "RS485_sensor.h"
#include "Things_Board.h"
#include <TinkerC6.h>

//==============================
// ดึงค่า interval จาก ThingsBoard มาใช้ (เหลื่อมเวลา 5 วินาที)
//==============================
extern const unsigned long tbInterval; 

//==============================
// Modbus
//==============================
ModbusMaster sensor;

//==============================
// Sensor Data
//==============================
float turbidity = 0.0;
float temperature = 0.0;

//==============================
// Timer
//==============================
unsigned long sensorTimer = 0;

// รอเซนเซอร์บูต 5 วินาที
const unsigned long sensorBootTime = 5000;

//==============================
// State Machine
//==============================
enum SensorState
{
    WAIT_TIME,
    WAIT_SENSOR
};

SensorState sensorState = WAIT_TIME;

//==================================================
// Setup RS485
//==================================================
void setupWaterSensor()
{
    TinkerC6.RS485.begin(4800, SERIAL_8N1);
    sensor.begin(1, TinkerC6.RS485);
    sensor.preTransmission(TinkerC6_RS485_preTransmission);
    sensor.postTransmission(TinkerC6_RS485_postTransmission);
    TinkerC6.RS485.enable();

    // เริ่มต้นให้ sensorTimer ทำงานล่วงหน้าเพื่อให้จังหวะเปิดไฟเหลื่อมกับเวลาส่งจริง
    sensorTimer = millis() - (tbInterval - 5000);
}

//==================================================
// Read Sensor
//==================================================
void readWaterSensor()
{
    uint8_t result;
    result = sensor.readHoldingRegisters(0x0000, 2);

    if(result == sensor.ku8MBSuccess)
    {
        turbidity = sensor.getResponseBuffer(0) / 10.0;
        temperature = sensor.getResponseBuffer(1) / 10.0;
    }
    else
    {
        Serial.print("Water Sensor Error : ");
        Serial.println(result);
    }
}

//==================================================
// Print
//==================================================
void printWaterSensor()
{
    Serial.println("----------------");
    Serial.print("Turbidity : ");
    Serial.print(turbidity);
    Serial.println(" NTU");
    Serial.print("Temperature : ");
    Serial.print(temperature);
    Serial.println(" C");
}

//==================================================
// Update (ใช้ tbInterval ของ ThingsBoard เป็นตัวคุมจังหวะหลัก)
//==================================================
void SensorUpdate()
{
    switch(sensorState)
    {
        //----------------------------
        // รอครบเวลา (tbInterval - 5s)
        //----------------------------
        case WAIT_TIME:
            if(millis() - sensorTimer >= (tbInterval - 5000))
            {
                sensorTimer = millis();
                Serial.println("12V ON (Sensor Warming Up)");
                TinkerC6.Power.enable12V();
                sensorState = WAIT_SENSOR;
            }
            break;

        //----------------------------
        // รอ Sensor Boot (5s) แล้วอ่านค่า
        //----------------------------
        case WAIT_SENSOR:
            if(millis() - sensorTimer >= sensorBootTime)
            {
                readWaterSensor();
                printWaterSensor();
                Serial.println("12V OFF (Data Read Complete)");
                TinkerC6.Power.disable12V();
                
                sensorTimer = millis();
                sensorState = WAIT_TIME;
            }
            break;
    }
}

//==================================================
// อ่านค่าครั้งแรกตอนเริ่มระบบ
//==================================================
void readSensorFirstTime() {
    Serial.println("\n=== [Reading First Time Data] ===");
    Serial.println("[Sensor] เปิดไฟ 12V ให้เซนเซอร์...");
    TinkerC6.Power.enable12V();
    delay(5000); 
    Serial.println("[Sensor] กำลังอ่านค่า...");
    readWaterSensor(); 
    printWaterSensor();
    Serial.println("[Sensor] อ่านเสร็จแล้ว -> ปิดไฟ 12V");
    TinkerC6.Power.disable12V();
    Serial.println("=================================\n");
}