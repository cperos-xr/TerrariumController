/* RTCManager.cpp */
#include "RTCManager.h"
#include <Arduino.h>

RTCManager::RTCManager() {}

bool RTCManager::isRTCAvailable() {
    Wire.beginTransmission(RTC_ADDRESS);
    return (Wire.endTransmission() == 0); // Return true if RTC responds
}

void RTCManager::initRTC() {
    if (!rtc.begin()) {
        Serial.println("RTC not found!");
        while (1); // Halt if RTC is not found
    }
}

DateTime RTCManager::getCurrentTime() {
    return rtc.now();
}

void RTCManager::setDateTime(int year, int month, int day, int hour, int minute, int second) {
    rtc.adjust(DateTime(year, month, day, hour, minute, second));
    Serial.println("RTC time adjusted.");
}

void RTCManager::printCurrentTime() {
    DateTime now = rtc.now();
    Serial.print("Current Time: ");
    Serial.print(now.year());
    Serial.print("-");
    Serial.print(now.month());
    Serial.print("-");
    Serial.print(now.day());
    Serial.print(" ");
    Serial.print(now.hour());
    Serial.print(":");
    Serial.print(now.minute());
    Serial.print(":");
    Serial.println(now.second());
}