#pragma once

#include <WiFi.h>
#include <WiFiManager.h>

void battery_();
void updateLED();
void Sw_press();
void WiFiSet();
void LedSet();
void WiFiUpdate();

void WiFiConnect();
void WiFiDisconnect();

extern bool wifiConnected;

int getBatteryPercent();