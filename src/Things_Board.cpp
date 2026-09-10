// =====================================================
// ThingsBoard MQTT - ESP32-WROOM-32E
// =====================================================
// เปลี่ยนจาก TinkerC6 → ESP32 ทั่วไป
// ลอจิกเดิมคงไว้ทั้งหมด: State Machine, Timeout, WiFi ON/OFF ประหยัดพลังงาน
// =====================================================

#include "Things_Board.h"
#include "Wifi_Manager.h"
#include "RS485_Sensor.h"

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

WiFiClient espClient;
PubSubClient client(espClient);

//==============================
// ThingBoard Cloud
//==============================
const char* MQTT_SERVER = "thingsboard.lesyslab.com";
const int MQTT_PORT = 1883;
const char* TOKEN = "8WoofV9qK9yK6UkWfLvI";

//==============================
// Timer
//==============================
unsigned long tbTimer = 0;
const unsigned long tbInterval = 600000;      // รอบปกติคือ 10 นาที (600,000 ms)
bool firstBoot = true;                         // ใช้เช็คเพื่อให้ส่งทันทีหลังลงชื่อเสร็จครั้งแรก

//======================================
// State Machine
//======================================
enum TB_STATE
{
    WAIT_TIME,
    WAIT_WIFI,
    WAIT_MQTT
};

TB_STATE tbState = WAIT_TIME;

void ThingBoard_Setup()
{
    client.setServer(MQTT_SERVER, MQTT_PORT);
}

unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 5000; 

void reconnect()
{
    if (client.connected()) {
        return; 
    }

    unsigned long now = millis();
    if (now - lastReconnectAttempt > reconnectInterval) 
    {
        lastReconnectAttempt = now;
        Serial.println("Connecting ThingsBoard...");

        if (client.connect("ESP32-WaterQ", TOKEN, "")) 
        {
            Serial.println("ThingsBoard Connected");
        } 
        
        else 
        {
            Serial.print("MQTT Error : ");
            Serial.println(client.state());
            Serial.println("Will try again in 5 seconds...");
        }
    }
}
unsigned long mqttTimeoutTimer = 0; // ตัวแปรสำหรับคุมเวลาถอยทัพ

void ThingBoard_Update()
{
    switch(tbState)
    {
        case WAIT_TIME:
            if (millis() - tbTimer >= tbInterval)
            {
                Serial.println("[TB] ถึงรอบเวลา -> กำลังเปิด WiFi");
                WiFiConnect(); 
                tbState = WAIT_WIFI;
            }
            else if (firstBoot && WiFi.status() == WL_CONNECTED)
            {
                Serial.println("[TB] ตรวจพบการลงชื่อ WiFi สำเร็จครั้งแรก -> เตรียมส่งข้อมูล");
                firstBoot = false; 
                tbState = WAIT_WIFI; 
            }
        break;

        case WAIT_WIFI:
            if(WiFi.status() == WL_CONNECTED)
            {
                Serial.println("[TB] WiFi Ready -> กำลังเชื่อมต่อ MQTT");
                lastReconnectAttempt = 0; 
                mqttTimeoutTimer = millis(); // เริ่มจับเวลา Timeout ตั้งแต่เริ่มเข้าสเตจ MQTT
                tbState = WAIT_MQTT;
            }
        break;

        case WAIT_MQTT:
            reconnect(); 

            if(client.connected())
            {
                Serial.println("[TB] MQTT OK -> กำลังส่ง Payload");
                
                ThingBoard_Send();   
                
                // รออัปเดตหน้า Dashboard 2 วินาที
                unsigned long waitDashboard = millis();
                while(millis() - waitDashboard < 2000) {
                    client.loop();
                    yield();
                }
                
                Serial.println("[TB] หน้า Dashboard อัปเดตเรียบร้อย -> ปิดการทำงาน");
                client.disconnect(); 
                WiFiDisconnect();    

                tbTimer = millis();   
                tbState = WAIT_TIME; 
                
                Serial.println("[TB] เข้าสู่โหมดจำศีล (WiFi OFF) รอรอบถัดไป...");
            }
            // เงื่อนไขถอยทัพ: ถ้าพยายามต่อ MQTT นานเกิน 30 วินาทีแล้วยังไม่ติด
            else if (millis() - mqttTimeoutTimer >= 30000) 
            {
                Serial.println("[TB] !!! MQTT Connection Timeout (30s) !!! -> ปิด WiFi เซฟพลังงาน");
                
                if (client.connected()) client.disconnect();
                WiFiDisconnect();
                
                tbTimer = millis();
                tbState = WAIT_TIME;
            }
            break;
    }

    client.loop(); 
    yield(); 
}

void ThingBoard_Send()
{
    JsonDocument doc;

    doc["turbidity"] = turbidity; 
    doc["temperature"] = temperature; 
    doc["wifi"] = wifiConnected ? WiFi.RSSI() : -100;

    char payload[256];
    serializeJson(doc, payload);

    client.publish("v1/devices/me/telemetry", payload);

    Serial.print("Send : ");
    Serial.println(payload);
    Serial.println("Upload Success");
}
