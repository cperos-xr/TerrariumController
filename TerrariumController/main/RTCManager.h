#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <Wire.h>
#include <RTClib.h> // Include Adafruit RTClib

#define RTC_ADDRESS 0x68 // RTC I²C address

class RTCManager {
public:
    RTCManager();
    void initRTC();
    void printCurrentTime(); // Add this method declaration
    void adjustToCompileTime(); // Adjust RTC to compile time if power is lost
    DateTime getCurrentTime(); // Method to get the current time
    bool isRTCAvailable(); // Method to detect RTC availability
    void setDateTime(int year, int month, int day, int hour, int minute, int second);

private:
    RTC_DS3231 rtc; // RTC object from RTClib
};

extern RTCManager rtc; // Declare a global RTCManager instance

#endif // RTC_MANAGER_H