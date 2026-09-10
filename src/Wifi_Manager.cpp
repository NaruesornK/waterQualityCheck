// =====================================================
// WiFi Manager - ESP32-WROOM-32E
// =====================================================
// เปลี่ยนจาก TinkerC6 → ESP32 ทั่วไป
// - NeoPixel RGB → Built-in LED (GPIO2)
// - TinkerC6 Battery → ลบออก (ESP32 ไม่มี IC วัดแบต)
// - ปุ่มกด GPIO9 → GPIO0 (Boot Button บน ESP32)
// =====================================================

#include "Wifi_Manager.h" 
#include "pin_config.h"
#include <WiFi.h>
#include <WiFiManager.h>

WiFiManager wm;

//=====================================================================
// ตัวแปรสถานะระบบ
//=====================================================================
unsigned long ledTimer = 0;
bool ledBlink = false;

unsigned long pressStart = 0;
bool pressed = false;
bool wifiReset = false;       
bool wifiConnected = false;   

//=====================================================================
// ฟังก์ชันตั้งค่าเริ่มต้น (Setup)
//=====================================================================

void LedSet()
{
    pinMode(LED_STATUS_PIN, OUTPUT);
    digitalWrite(LED_STATUS_PIN, LOW);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void WiFiSet()
{
    wm.setConfigPortalBlocking(false);   
    wm.setConnectTimeout(15);            

    Serial.println("[WM] กำลังเริ่มพยายามเชื่อมต่ออัตโนมัติ...");
    wm.autoConnect("WaterQ-ESP32");  // ชื่อ AP สำหรับตั้งค่า WiFi

    if (wm.getConfigPortalActive()) {
        Serial.println("[WM] เข้าสู่โหมดรอลงชื่อ WiFi... (ไฟจะกระพริบ)");
        
        unsigned long portalTimer = millis(); 

        while (wm.getConfigPortalActive()) {
            wm.process();  
            
            // LED กระพริบระหว่างรอตั้งค่า WiFi
            if (millis() - ledTimer >= 500) {
                ledTimer = millis();
                ledBlink = !ledBlink;
                digitalWrite(LED_STATUS_PIN, ledBlink ? HIGH : LOW);
            }

            // ถ้าค้างอยู่ในลูปนี้เกิน 2 นาที ให้ Reboot
            if (millis() - portalTimer >= 120000) {
                Serial.println("[WM] ค้างหน้า Portal นานเกิน 2 นาทีแล้ว! สั่งรีบูทบอร์ดใหม่...");
                digitalWrite(LED_STATUS_PIN, LOW);
                delay(500);
                ESP.restart();
            }

            delay(10); 
        }
        Serial.println("[WM] หลุดจากหน้า Portal แล้ว");
        digitalWrite(LED_STATUS_PIN, LOW);
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[WM] WiFi Initial Connected Successfully");
        wifiConnected = true;
    } else {
        Serial.println("[WM] WiFi Initial Connect Failed / Timeout");
        wifiConnected = false;
    }
}

//=====================================================================
// ฟังก์ชันควบคุม WiFi
//=====================================================================
void WiFiConnect()
{
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        return;
    }

    Serial.println(">>> WiFi Turning ON <<<");
    WiFi.mode(WIFI_STA);
    delay(200);           

    WiFi.begin();        
    wifiConnected = false;
}

void WiFiDisconnect()
{
    Serial.println(">>> WiFi Turning OFF (Save Power) <<<");
    
    digitalWrite(LED_STATUS_PIN, LOW);
    
    WiFi.disconnect(false);   
    WiFi.mode(WIFI_OFF);       
    wifiConnected = false;     
}

//=====================================================================
// LED แสดงสถานะ
//=====================================================================
void updateLED()
{
    // 1. เช็คสเตท WiFi Portal (LED กระพริบเร็ว)
    if (wm.getConfigPortalActive()) { 
        if (millis() - ledTimer >= 500) {
            ledTimer = millis();
            ledBlink = !ledBlink;
        }
        digitalWrite(LED_STATUS_PIN, ledBlink ? HIGH : LOW);
    }
    // 2. เช็คสเตทเน็ตหลุดกระทันหัน (LED กระพริบช้า)
    else if (WiFi.status() != WL_CONNECTED && WiFi.getMode() != WIFI_OFF) {
        if (millis() - ledTimer >= 1000) { 
            ledTimer = millis();
            ledBlink = !ledBlink;
        }
        digitalWrite(LED_STATUS_PIN, ledBlink ? HIGH : LOW);
    }
    // 3. ปกติ → LED ดับ (ประหยัดพลังงาน)
    else {
        digitalWrite(LED_STATUS_PIN, LOW);
    }
}

//=====================================================================
// ปุ่มกด - กดค้าง 5 วินาทีเพื่อรีเซ็ต WiFi
//=====================================================================
void Sw_press()
{
    if (digitalRead(BUTTON_PIN) == LOW) {
        if (!pressed) {
            pressed = true;
            pressStart = millis(); 
            wifiReset = false;
        }

        if (millis() - pressStart > 5000 && !wifiReset) { 
            wifiReset = true;
            Serial.println("Resetting WiFi Settings...");
            wm.resetSettings();
            ESP.restart();
        }
    }
    else {
        pressed = false;
    }
}
