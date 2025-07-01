#include "BluetoothManager.h"
#include <Arduino.h>
#include "RTCManager.h"
#include "TaskScheduler.h" // Include TaskScheduler here

extern RTCManager rtc;
extern TaskScheduler scheduler;

BluetoothManager::BluetoothManager() {}

void BluetoothManager::initBLE() {
    BLEDevice::init("TerrariumController");
    BLEServer* pServer = BLEDevice::createServer();
    BLEService* pService = pServer->createService("12345678-1234-5678-1234-56789abcdef0");

    rtcChar = pService->createCharacteristic(
        "12345678-1234-5678-1234-56789abcdef1",
        BLECharacteristic::PROPERTY_WRITE
    );
    rtcChar->addDescriptor(new BLE2902());
    rtcChar->setCallbacks(new RTCWriteCallback());

    nameChar = pService->createCharacteristic(
        "12345678-1234-5678-1234-56789abcdef2",
        BLECharacteristic::PROPERTY_WRITE
    );
    nameChar->addDescriptor(new BLE2902());
    nameChar->setCallbacks(new NameWriteCallback(*this)); // Pass reference to BluetoothManager

    waterChar = pService->createCharacteristic(
        "12345678-1234-5678-1234-56789abcdef3",
        BLECharacteristic::PROPERTY_WRITE
    );
    waterChar->addDescriptor(new BLE2902());
    waterChar->setCallbacks(new WaterWriteCallback());

    lightChar = pService->createCharacteristic(
        "12345678-1234-5678-1234-56789abcdef4",
        BLECharacteristic::PROPERTY_WRITE
    );
    lightChar->addDescriptor(new BLE2902());
    lightChar->setCallbacks(new LightWriteCallback());

    pService->start();
    pServer->getAdvertising()->start();
    Serial.println("🔵 BLE advertising started");
}

void BluetoothManager::startAdvertising() {
    BLEDevice::getAdvertising()->start();
    Serial.println("🔵 BLE advertising restarted");
}

// Callback for RTC writes
void RTCWriteCallback::onWrite(BLECharacteristic* pChar) {
    String value = pChar->getValue().c_str(); // Convert std::string to Arduino String
    if (value.length() > 0) {
        Serial.printf("Received RTC update: %s\n", value.c_str());

        // Parse the incoming string (format: "YYYY-MM-DD HH:MM:SS")
        int year, month, day, hour, minute, second;
        sscanf(value.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);

        // Update RTC
        rtc.setDateTime(year, month, day, hour, minute, second); // Ensure RTCManager has this method
        Serial.println("RTC updated successfully!");
    }
}

// Callback for Device Name writes
void NameWriteCallback::onWrite(BLECharacteristic* pChar) {
    String newName = pChar->getValue().c_str(); // Convert std::string to Arduino String
    if (newName.length() > 0) {
        Serial.printf("Received new device name: %s\n", newName.c_str());

        // Update BLE device name
        BLEDevice::deinit(); // Deinitialize BLE
        BLEDevice::init(newName.c_str()); // Set new name

        // Restart BLE server and services
        BLEServer* pServer = BLEDevice::createServer();
        BLEService* pService = pServer->createService("12345678-1234-5678-1234-56789abcdef0");

        // Recreate RTC characteristic
        BLECharacteristic* rtcCharacteristic = pService->createCharacteristic(
            "12345678-1234-5678-1234-56789abcdef1",
            BLECharacteristic::PROPERTY_WRITE
        );
        rtcCharacteristic->addDescriptor(new BLE2902());
        rtcCharacteristic->setCallbacks(new RTCWriteCallback());
        manager.setRtcChar(rtcCharacteristic);

        // Recreate Device Name characteristic
        BLECharacteristic* nameCharacteristic = pService->createCharacteristic(
            "12345678-1234-5678-1234-56789abcdef2",
            BLECharacteristic::PROPERTY_WRITE
        );
        nameCharacteristic->addDescriptor(new BLE2902());
        nameCharacteristic->setCallbacks(new NameWriteCallback(manager));
        manager.setNameChar(nameCharacteristic);

        pService->start();
        pServer->getAdvertising()->start();
        Serial.printf("Device name updated to: %s\n", newName.c_str());
    }
}

// <frequency>,<startHour>,<startMinute>,<duration>,<secondStartHour>,<secondStartMinute>
// Examples you might send this:
// ALWAYS_ON,0,0,0
// DAILY,13,30,3600
// TWICE_DAILY,10,0,60,14,36

// Callback for Water scheduling
void WaterWriteCallback::onWrite(BLECharacteristic* pChar) {
    String value = pChar->getValue().c_str();
    if (value.length() > 0) {
        Serial.printf("Received Water schedule: %s\n", value.c_str());

        // Parse the command
        char frequency[20];
        int startHour, startMinute, duration, secondStartHour = -1, secondStartMinute = -1;
        int parsed = sscanf(value.c_str(), "%19[^,],%d,%d,%d,%d,%d", frequency, &startHour, &startMinute, &duration, &secondStartHour, &secondStartMinute);

        // Determine frequency type
        TaskFrequency freq = parseFrequency(frequency);
        if (freq == INVALID) {
            Serial.println("Invalid frequency type!");
            return;
        }

        // Update the Water task
        if (freq == TWICE_DAILY && parsed == 6) {
            scheduler.addTask({"Water", freq, startHour, startMinute, duration, secondStartHour, secondStartMinute});
        } else {
            scheduler.addTask({"Water", freq, startHour, startMinute, duration});
        }
        Serial.println("Water schedule updated!");
    }
}

// Callback for Light scheduling
void LightWriteCallback::onWrite(BLECharacteristic* pChar) {
    String value = pChar->getValue().c_str();
    if (value.length() > 0) {
        Serial.printf("Received Light schedule: %s\n", value.c_str());

        // Parse the command
        char frequency[20];
        int startHour, startMinute, duration, secondStartHour = -1, secondStartMinute = -1;
        int parsed = sscanf(value.c_str(), "%19[^,],%d,%d,%d,%d,%d", frequency, &startHour, &startMinute, &duration, &secondStartHour, &secondStartMinute);

        // Determine frequency type
        TaskFrequency freq = parseFrequency(frequency);
        if (freq == INVALID) {
            Serial.println("Invalid frequency type!");
            return;
        }

        // Update the Light task
        if (freq == TWICE_DAILY && parsed == 6) {
            scheduler.addTask({"Light", freq, startHour, startMinute, duration, secondStartHour, secondStartMinute});
        } else {
            scheduler.addTask({"Light", freq, startHour, startMinute, duration});
        }
        Serial.println("Light schedule updated!");
    }
}

// Helper function to parse frequency
TaskFrequency parseFrequency(const char* freqStr) {
    if (strcmp(freqStr, "ALWAYS_ON") == 0) return ALWAYS_ON;
    if (strcmp(freqStr, "DAILY") == 0) return DAILY;
    if (strcmp(freqStr, "TWICE_DAILY") == 0) return TWICE_DAILY;
    if (strcmp(freqStr, "WEEKLY") == 0) return WEEKLY;
    if (strcmp(freqStr, "TWICE_WEEKLY") == 0) return TWICE_WEEKLY;
    if (strcmp(freqStr, "MONTHLY") == 0) return MONTHLY;
    if (strcmp(freqStr, "TWICE_MONTHLY") == 0) return TWICE_MONTHLY;
    return INVALID;
}