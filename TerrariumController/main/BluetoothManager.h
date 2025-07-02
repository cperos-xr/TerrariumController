/* BluetoothManager.h */
#pragma once
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEService.h>
#include <BLECharacteristic.h>
#include <BLE2902.h>
#include "RTCManager.h"
#include "TaskScheduler.h"

class BluetoothManager {
public:
  void initBLE(TaskScheduler* sched, RTCManager* rtcMgr);
  void startAdvertising();
private:
  BLEServer* pServer;
  BLEService* pService;
  BLECharacteristic* pRxWater;
  BLECharacteristic* pRxLight;

  TaskScheduler* scheduler;
  RTCManager* rtc;

  class WriteCallback : public BLECharacteristicCallbacks {
  public:
    WriteCallback(TaskScheduler* sch) : sched(sch) {}
    void onWrite(BLECharacteristic* pChar) override {
      String cmd = pChar->getValue().c_str();
      sched->parseAndSetSchedule(cmd);
    }
  private:
    TaskScheduler* sched;
  };
};