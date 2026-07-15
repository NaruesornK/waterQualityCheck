#include "Wifi_Manager.h" 
#include <TinkerC6.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <Adafruit_NeoPixel.h>

WiFiManager wm;

//=====================================================================
// ไฟ RGB และ ปุ่มกด
//=====================================================================
#define PIN_RGB     8  
#define NUM_PIXELS  1
Adafruit_NeoPixel pixels(NUM_PIXELS, PIN_RGB, NEO_GRB + NEO_KHZ800);

#define Sw9 9 

//=====================================================================
// ตัวแปรสถานะระบบ
//=====================================================================
unsigned long ledTimer = 0;
bool ledBlink = false;

unsigned long pressStart = 0;
bool pressed = false;
bool wifiReset = false;       
bool wifiConnected = false;   

// ตัวแปรควบคุมการแสดงผลไฟแบตเตอรี่ (Non-blocking ไม่ค้างลูป)
bool batteryShow = false;
unsigned long batteryTimer = 0;
int batR = 0, batG = 0, batB = 0;

//=====================================================================
// ฟังก์ชันตั้งค่าเริ่มต้น (Setup)
//=====================================================================

void LedSet()
{
    pixels.begin();
    pixels.setBrightness(50); 
    pixels.clear();
    pixels.show();
    
    pinMode(Sw9, INPUT_PULLUP);

    //-----------------------------------------------------------------
    // ดึงค่าแบตเตอรี่ทิ้งล่วงหน้า (Dummy Read) เพื่อให้ชิปนิ่งก่อนใช้งานจริง
    //-----------------------------------------------------------------
    Serial.println("[System] กำลังวอร์มระบบอ่านค่าแบตเตอรี่...");
    int dummyBat = TinkerC6.Power.getSOC(); 
    delay(100); 
    
    int readyBat = TinkerC6.Power.getSOC();
    Serial.print("[System] ระบบแบตเตอรี่พร้อมใช้งาน! ค่าเริ่มต้น: ");
    Serial.print(readyBat);
    Serial.println("%");
    Serial.println("----------------------------------------");
}

void WiFiSet()
{
    wm.setConfigPortalBlocking(false);   
    wm.setConnectTimeout(15);            

    Serial.println("[WM] กำลังเริ่มพยายามเชื่อมต่ออัตโนมัติ...");
    wm.autoConnect("TinkerC6");

    if (wm.getConfigPortalActive()) {
        Serial.println("[WM] เข้าสู่โหมดรอลงชื่อ WiFi... (ไฟสีฟ้าจะเริ่มกระพริบ)");
        
        // 🌟 ตั้งตัวแปรจับเวลาเริ่มต้นก่อนเข้าลูป while ไฟสีฟ้า
        unsigned long portalTimer = millis(); 

        while (wm.getConfigPortalActive()) {
            wm.process();  
            
            if (millis() - ledTimer >= 500) {
                ledTimer = millis();
                ledBlink = !ledBlink;
                if (ledBlink)
                    pixels.setPixelColor(0, pixels.Color(0, 0, 255)); 
                else
                    pixels.clear();
                pixels.show();
            }

            // 🌟 🛠️ แก้ไข: ฝังลอจิกรีเซ็ตบอร์ดไว้ข้างในลูปนี้เลยตามที่นายบอก
            // ถ้าค้างอยู่ในลูปไฟสีฟ้านี้เกิน 2 นาที (120,000 มิลลิวินาที) ให้สั่ง Reboot ตัวเองทันที
            if (millis() - portalTimer >= 120000) {
                Serial.println("[WM] ค้างหน้า Portal สีฟ้านานเกิน 2 นาทีแล้ว! สั่งรีบูทบอร์ดใหม่ในลูป...");
                pixels.clear();
                pixels.show();
                delay(500);
                ESP.restart(); // สั่งรีเซ็ตบอร์ดทันทีจากข้างในนี้
            }

            delay(10); 
        }
        Serial.println("[WM] หลุดจากหน้า Portal แล้ว");
        pixels.clear();
        pixels.show();
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
    
    // ดับไฟ RGB ให้มืดสนิทก่อนจะทำการปิดสัญญาณ WiFi 
    pixels.clear();
    pixels.show();
    
    WiFi.disconnect(false);   
    WiFi.mode(WIFI_OFF);       
    wifiConnected = false;     
}

//=====================================================================
// ฟังก์ชันจัดการเหตุการณ์และระบบเวลา (Update Loops)
//=====================================================================

void updateLED()
{
    pixels.clear(); 
    bool needShow = false;

    // 1. ลอจิกแสดงไฟแบตเตอรี่
    if (batteryShow) {
        if (millis() - batteryTimer < 2000) {
            pixels.setPixelColor(0, pixels.Color(batR, batG, batB));
            needShow = true;
        } else {
            batteryShow = false; 
            needShow = true;
        }
    }
    // 2. เช็คสเตท WiFi Portal (ไฟสีฟ้ากระพริบ)
    else if (wm.getConfigPortalActive()) { 
        if (millis() - ledTimer >= 500) {
            ledTimer = millis();
            ledBlink = !ledBlink;
        }
        if (ledBlink) {
            pixels.setPixelColor(0, pixels.Color(0, 0, 255)); 
        }
        needShow = true;
    }
    // 3. เช็คสเตทเน็ตหลุดกระทันหัน (ไฟสีแดงกระพริบ)
    else if (WiFi.status() != WL_CONNECTED && WiFi.getMode() != WIFI_OFF) {
        if (millis() - ledTimer >= 500) { 
            ledTimer = millis();
            ledBlink = !ledBlink;
        }
        if (ledBlink) {
            pixels.setPixelColor(0, pixels.Color(255, 0, 0)); 
        }
        needShow = true;
    }
    
    static bool lastNeedShow = true;
    if (needShow || lastNeedShow) {
        pixels.show();
        lastNeedShow = needShow; 
    }
}

void Sw_press()
{
    if (digitalRead(Sw9) == LOW) {
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
        if (pressed) {
            unsigned long pressTime = millis() - pressStart; 
            
            if (pressTime < 1500 && !wifiReset) {
                battery_(); 
            }
        }
        pressed = false;
    }
}

void battery_()
{
    int battery = TinkerC6.Power.getSOC(); 

    if (battery <= 20) {
        batR = 255; batG = 0; batB = 0;     
    }
    else if (battery <= 60) {
        batR = 255; batG = 100; batB = 0;   
    }
    else {
        batR = 0; batG = 255; batB = 0;     
    }

    Serial.print("Battery Percent : ");
    Serial.print(battery);
    Serial.println("%");
    Serial.println("-----------------------");

    batteryShow = true;
    batteryTimer = millis();
}

void WiFiUpdate()
{
    wm.process(); 
}

int getBatteryPercent()
{
    return TinkerC6.Power.getSOC();
}