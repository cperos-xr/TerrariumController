#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <Wire.h>
#include <RTClib.h> // Include Adafruit RTClib

class RTCManager {
public:
    RTCManager();
    void initRTC();
    void printCurrentTime();
    void adjustToCompileTime(); // Adjust RTC to compile time if power is lost
    DateTime getCurrentTime(); // New method to get the current time

private:
    RTC_DS3231 rtc; // RTC object from RTClib
};

extern RTCManager rtc; // Declare a global RTCManager instance

#endif // RTC_MANAGER_H