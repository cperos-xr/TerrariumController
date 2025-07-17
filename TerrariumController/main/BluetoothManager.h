/* BluetoothManager.h */
#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "TaskScheduler.h"
#include "RTCManager.h"
#include "SensorManager.h"
#include "RecordManager.h"

// Function declaration for pressFoggerButton (defined in main.ino)
void pressFoggerButton();

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
    BLECharacteristic* pRtcWrite; 
    BLECharacteristic* pScheduleRead;
    BLECharacteristic* pSensorRead;
    BLECharacteristic* pRecordRead;
    BLECharacteristic* pClearSchedules;
    BLECharacteristic* pClearRecords;
    BLECharacteristic* pFoggerButton; // New characteristic for fogger button
};

// Add the fogger button callback class
class FoggerButtonCallback : public BLECharacteristicCallbacks {
public:
    void onWrite(BLECharacteristic* pCharacteristic) override;
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

class RTCWriteCallback : public BLECharacteristicCallbacks {
public:
    RTCWriteCallback(RTCManager* rtcMgr);
    void onWrite(BLECharacteristic* pCharacteristic) override;

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

// Add sensor read callback class
class SensorReadCallback : public BLECharacteristicCallbacks {
public:
    SensorReadCallback();
    void onRead(BLECharacteristic* pCharacteristic) override;
};

// Add record read callback class
class RecordReadCallback : public BLECharacteristicCallbacks {
    public:
    RecordReadCallback();
    void onRead(BLECharacteristic* pCharacteristic) override;
};

// Add these new callback classes after the other callback classes
// Callback for clearing schedules
class ClearSchedulesCallback : public BLECharacteristicCallbacks {
public:
    ClearSchedulesCallback(TaskScheduler* sched);
    void onWrite(BLECharacteristic* pCharacteristic) override;

private:
    TaskScheduler* scheduler;
};

// Callback for clearing records
class ClearRecordsCallback : public BLECharacteristicCallbacks {
public:
    ClearRecordsCallback();
    void onWrite(BLECharacteristic* pCharacteristic) override;
};


#endif // BLUETOOTHMANAGER_H