#pragma once

#include <WiFi.h>
#include <WiFiManager.h>

void WiFiSet();
void LedSet();
void WiFiConnect();
void WiFiDisconnect();
void updateLED();
void Sw_press();

extern bool wifiConnected;
