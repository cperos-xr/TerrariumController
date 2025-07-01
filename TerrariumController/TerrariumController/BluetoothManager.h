#pragma once

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEService.h>
#include <BLECharacteristic.h>
#include <BLE2902.h>
#include "TaskScheduler.h" // Include TaskScheduler for TaskFrequency and Task structure

class BluetoothManager {
public:
    BluetoothManager();
    void initBLE();
    void startAdvertising();
    void setRtcChar(BLECharacteristic* characteristic) { rtcChar = characteristic; }
    void setNameChar(BLECharacteristic* characteristic) { nameChar = characteristic; }

private:
    BLECharacteristic* rtcChar; // RTC characteristic
    BLECharacteristic* nameChar; // Device name characteristic
    BLECharacteristic* waterChar; // Water scheduling characteristic
    BLECharacteristic* lightChar; // Light scheduling characteristic

    friend class NameWriteCallback; // Allow NameWriteCallback to access private members
};

// Callback classes
class RTCWriteCallback : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pChar) override;
};

class NameWriteCallback : public BLECharacteristicCallbacks {
public:
    explicit NameWriteCallback(BluetoothManager& manager) : manager(manager) {}
    void onWrite(BLECharacteristic* pChar) override;

private:
    BluetoothManager& manager;
};

class WaterWriteCallback : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pChar) override;
};

class LightWriteCallback : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pChar) override;
};

// Helper function to parse frequency
TaskFrequency parseFrequency(const char* freqStr);