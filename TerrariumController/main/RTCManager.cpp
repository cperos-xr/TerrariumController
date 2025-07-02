/* RTCManager.cpp */
#include "RTCManager.h"
#include <Arduino.h>

RTCManager::RTCManager() {}

bool RTCManager::isRTCAvailable() {
  Wire.beginTransmission(RTC_ADDRESS);
  return Wire.endTransmission()==0;
}

void RTCManager::initRTC() {
    Wire.begin(); // Initialize I2C bus
    if (!rtc.begin()) {
        Serial.println("RTC not found!");
        while (1); // Halt if RTC is not found
    }
}

DateTime RTCManager::getCurrentTime() { return rtc.now(); }

void RTCManager::setDateTime(int y,int mo,int d,int h,int mi,int s) {
  rtc.adjust(DateTime(y,mo,d,h,mi,s));
  Serial.println("RTC time adjusted.");
}

void RTCManager::printCurrentTime() {
  auto n=rtc.now();
  Serial.printf("Current Time: %04d-%02d-%02d %02d:%02d:%02d\n",
    n.year(),n.month(),n.day(),n.hour(),n.minute(),n.second());
}