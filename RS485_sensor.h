#pragma once

extern float turbidity;  //// NTU 0-1000 ความสะอาดของน้ำ
extern float temperature; ///// อุณภูมิ 

void setupWaterSensor();
void readWaterSensor();
void printWaterSensor();
void SensorUpdate();
void readSensorFirstTime();
