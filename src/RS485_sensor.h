#pragma once

extern float turbidity;   // NTU 0-1000 ความขุ่นของน้ำ
extern float temperature;  // อุณหภูมิน้ำ (°C)

void setupWaterSensor();
void readWaterSensor();
void printWaterSensor();
void SensorUpdate();
void readSensorFirstTime();
