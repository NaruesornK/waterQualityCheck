#include <TinkerC6.h>
#include "RS485_sensor.h" 
#include "Wifi_Manager.h" 
#include "Things_Board.h"

void setup() {
    Serial.begin(115200);
     WiFiSet();

    setupWaterSensor();
    readSensorFirstTime();
    ThingBoard_Setup();
    LedSet();

  
}

void loop() {
  SensorUpdate();
  ThingBoard_Update();
  updateLED();
  Sw_press();


}


