/* BluetoothManager.h */
#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "TaskScheduler.h"
#include "RTCManager.h"

class BluetoothManager {
public:
    void initBLE(TaskScheduler* sched, RTCManager* rtcMgr);
    void startAdvertising();

private:
    TaskScheduler* scheduler;
    RTCManager* rtc;

    BLEServer* pServer;
    BLEService* pService;
    BLECharacteristic* pRxWater;
    BLECharacteristic* pRtcTime;
    BLECharacteristic* pScheduleRead; // Add this new characteristic
};

// Callback for writing to BLE characteristics
class WriteCallback : public BLECharacteristicCallbacks {
public:
    WriteCallback(TaskScheduler* sched);
    void onWrite(BLECharacteristic* pCharacteristic) override;

private:
    TaskScheduler* scheduler;
};

// Callback for reading RTC time
class RTCReadCallback : public BLECharacteristicCallbacks {
public:
    RTCReadCallback(RTCManager* rtcMgr);
    void onRead(BLECharacteristic* pCharacteristic) override;

private:
    RTCManager* rtc;
};

// Add this new callback class
class ScheduleReadCallback : public BLECharacteristicCallbacks {
public:
    ScheduleReadCallback(TaskScheduler* sched);
    void onRead(BLECharacteristic* pCharacteristic) override;

private:
    TaskScheduler* scheduler;
};


#endif // BLUETOOTHMANAGER_H