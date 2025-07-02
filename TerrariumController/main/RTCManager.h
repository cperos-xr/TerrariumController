/* RTCManager.h */
#ifndef RTCMANAGER_H
#define RTCMANAGER_H
#include <Wire.h>
#include <RTClib.h>
#define RTC_ADDRESS 0x68

class RTCManager {
public:
  RTCManager();
  bool isRTCAvailable();
  void initRTC();
  DateTime getCurrentTime();
  void setDateTime(int y,int mo,int d,int h,int mi,int s);
  void printCurrentTime();
private:
  RTC_DS3231 rtc;
};
#endif