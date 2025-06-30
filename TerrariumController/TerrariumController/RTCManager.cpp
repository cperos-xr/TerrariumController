#include "RTCManager.h"
#include <Arduino.h>

RTCManager rtc; // Define the global RTCManager instance

RTCManager::RTCManager() {}

bool RTCManager::isRTCAvailable() {
    Wire.beginTransmission(RTC_ADDRESS);
    return (Wire.endTransmission() == 0); // Return true if RTC responds
}

void RTCManager::initRTC() {
    if (!rtc.begin()) {
        Serial.println("❌ RTC not found!");
        while (1) { delay(10); } // Halt execution if RTC is not found
    }

    if (rtc.lostPower()) {
        Serial.println("⚡ RTC lost power, setting compile time.");
        adjustToCompileTime();
    }
}

void RTCManager::adjustToCompileTime() {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Set RTC to compile time
}

void RTCManager::printCurrentTime() {
    DateTime now = rtc.now(); // Get current time from RTC

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

DateTime RTCManager::getCurrentTime() {
    return rtc.now(); // Return the current time
}