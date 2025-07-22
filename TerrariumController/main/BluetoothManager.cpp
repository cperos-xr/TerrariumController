/* BluetoothManager.cpp */
#include "BluetoothManager.h"
#include "RecordManager.h"

#define SERVICE_UUID           "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID_WRITE_SCHEDULES "12345678-1234-5678-1234-56789abcdef3"
#define CHARACTERISTIC_UUID_READ_RTC   "12345678-1234-5678-1234-56789abcdef4" // New UUID for RTC
#define CHARACTERISTIC_UUID_READ_SCHEDULES   "12345678-1234-5678-1234-56789abcdef5" // New UUID for Schedule
#define CHARACTERISTIC_UUID_READ_SENSOR "12345678-1234-5678-1234-56789abcdef6" // New UUID for Sensor Read
#define CHARACTERISTIC_UUID_READ_RECORDS "12345678-1234-5678-1234-56789abcdef7" // New UUID for Record Read
#define CHARACTERISTIC_UUID_CLEAR_SCHEDULES "12345678-1234-5678-1234-56789abcdef8" // UUID for clearing schedules
#define CHARACTERISTIC_UUID_CLEAR_RECORDS "12345678-1234-5678-1234-56789abcdef9" // UUID for clearing records
#define CHARACTERISTIC_UUID_WRITE_RTC   "12345678-1234-5678-1234-56789abcdefa" // New UUID for writing to RTC
#define CHARACTERISTIC_UUID_FOGGER_BUTTON "12345678-1234-5678-1234-56789abcdefb" // New UUID for fogger button
#define CHARACTERISTIC_UUID_TOGGLE_LIGHT "12345678-1234-5678-1234-56789abcdefc"
#define CHARACTERISTIC_UUID_TOGGLE_WATER "12345678-1234-5678-1234-56789abcdefd"
#define CHARACTERISTIC_UUID_TOGGLE_FOGGER "12345678-1234-5678-1234-56789abcdefe"

// Server callback class to handle connections
class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override {
        Serial.println("🔵 Client connected");
    }

    void onDisconnect(BLEServer* pServer) override {
        Serial.println("🔴 Client disconnected - restarting advertising");
        delay(500); // Give a brief delay before restarting
        BLEDevice::startAdvertising();
        Serial.println("🔵 BLE advertising restarted");
    }
};

void BluetoothManager::initBLE(TaskScheduler* sched, RTCManager* rtcMgr) {
    scheduler = sched;
    rtc = rtcMgr;
    
    // Free memory from Bluetooth Classic to have more space for BLE
    esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    
    // Initialize BLE with increased table size
    BLEDevice::init("TerrariumController");
    
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());
    
    // Create a larger service to accommodate all characteristics
    pService = pServer->createService(BLEUUID(SERVICE_UUID), 40);  // Increase the handles count to 40

    pRxWater = pService->createCharacteristic(
        CHARACTERISTIC_UUID_WRITE_SCHEDULES,
        BLECharacteristic::PROPERTY_WRITE
    );
    pRxWater->addDescriptor(new BLE2902());
    pRxWater->setCallbacks(new WriteCallback(scheduler));

    // RTC characteristic
    pRtcTime = pService->createCharacteristic(
        CHARACTERISTIC_UUID_READ_RTC,
        BLECharacteristic::PROPERTY_READ
    );
    pRtcTime->addDescriptor(new BLE2902());
    pRtcTime->setCallbacks(new RTCReadCallback(rtc));

    pRtcWrite = pService->createCharacteristic(
        CHARACTERISTIC_UUID_WRITE_RTC,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
    );
    pRtcWrite->addDescriptor(new BLE2902());
    pRtcWrite->setCallbacks(new RTCWriteCallback(rtc));
    pRtcWrite->setValue("Format: YYYY-MM-DD HH:MM:SS");

    // Schedule characteristic
    pScheduleRead = pService->createCharacteristic(
        CHARACTERISTIC_UUID_READ_SCHEDULES,
        BLECharacteristic::PROPERTY_READ
    );
    pScheduleRead->addDescriptor(new BLE2902());
    pScheduleRead->setCallbacks(new ScheduleReadCallback(scheduler));

    pSensorRead = pService->createCharacteristic(
        CHARACTERISTIC_UUID_READ_SENSOR,
        BLECharacteristic::PROPERTY_READ
    );
    pSensorRead->addDescriptor(new BLE2902());
    pSensorRead->setCallbacks(new SensorReadCallback());

    pRecordRead = pService->createCharacteristic(
        CHARACTERISTIC_UUID_READ_RECORDS,
        BLECharacteristic::PROPERTY_READ
    );
    pRecordRead->addDescriptor(new BLE2902());
    pRecordRead->setCallbacks(new RecordReadCallback());

    // Clear Schedules characteristic
    pClearSchedules = pService->createCharacteristic(
        CHARACTERISTIC_UUID_CLEAR_SCHEDULES,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
    );
    pClearSchedules->addDescriptor(new BLE2902());
    pClearSchedules->setCallbacks(new ClearSchedulesCallback(scheduler));
    pClearSchedules->setValue("Send 'CLEAR' to reset schedules");

    // Clear Records characteristic
    pClearRecords = pService->createCharacteristic(
        CHARACTERISTIC_UUID_CLEAR_RECORDS,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
    );
    pClearRecords->addDescriptor(new BLE2902());
    pClearRecords->setCallbacks(new ClearRecordsCallback());
    pClearRecords->setValue("Send 'CLEAR' to reset records");

    // Add the new fogger button characteristic
    pFoggerButton = pService->createCharacteristic(
        CHARACTERISTIC_UUID_FOGGER_BUTTON,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
    );
    pFoggerButton->addDescriptor(new BLE2902());
    pFoggerButton->setCallbacks(new FoggerButtonCallback());
    pFoggerButton->setValue("Send 'PRESS' to activate fogger button");

    // Add the toggle light characteristic
    pToggleLight = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TOGGLE_LIGHT,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
    );
    pToggleLight->addDescriptor(new BLE2902());
    pToggleLight->setCallbacks(new ToggleLightCallback(scheduler));
    pToggleLight->setValue("Send 'TOGGLE' to switch light state");
    
    // Add the toggle water characteristic
    pToggleWater = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TOGGLE_WATER,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
    );
    pToggleWater->addDescriptor(new BLE2902());
    pToggleWater->setCallbacks(new ToggleWaterCallback(scheduler));
    pToggleWater->setValue("Send 'TOGGLE' to switch water state");
    
    // Add the toggle fogger characteristic
    pToggleFogger = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TOGGLE_FOGGER,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
    );
    pToggleFogger->addDescriptor(new BLE2902());
    pToggleFogger->setCallbacks(new ToggleFoggerCallback(scheduler));
    pToggleFogger->setValue("Send 'TOGGLE' to switch fogger state");
    
    pService->start();
    
    // Configure advertising
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    
    Serial.println("🔵 BLE initialized");
}

void BluetoothManager::startAdvertising() {
    BLEDevice::startAdvertising(); // Use startAdvertising() instead of getAdvertising()->start()
    Serial.println("🔵 BLE advertising started");
}

// WriteCallback implementation
WriteCallback::WriteCallback(TaskScheduler* sched) : scheduler(sched) {}

void WriteCallback::onWrite(BLECharacteristic* pCharacteristic) {
    String value = pCharacteristic->getValue();
    if (value.length() > 0) {
        scheduler->parseAndSetSchedule(value);
        Serial.println("Command received and processed.");
    }
}

// RTCReadCallback implementation
RTCReadCallback::RTCReadCallback(RTCManager* rtcMgr) : rtc(rtcMgr) {}

void RTCReadCallback::onRead(BLECharacteristic* pCharacteristic) {
    DateTime now = rtc->getCurrentTime();
    char timeStr[20];
    snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02d %02d:%02d:%02d",
             now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
    pCharacteristic->setValue(timeStr);
}

// ScheduleReadCallback implementation
ScheduleReadCallback::ScheduleReadCallback(TaskScheduler* sched) : scheduler(sched) {}

void ScheduleReadCallback::onRead(BLECharacteristic* pCharacteristic) {
    String scheduleJson = scheduler->getSchedulesAsJSON();
    pCharacteristic->setValue(scheduleJson.c_str());
}

// SensorReadCallback implementation
SensorReadCallback::SensorReadCallback() {}

void SensorReadCallback::onRead(BLECharacteristic* pCharacteristic) {
    String sensorData = snr.getSensorDataAsJSON();
    pCharacteristic->setValue(sensorData.c_str());
}

RecordReadCallback::RecordReadCallback() {}

void RecordReadCallback::onRead(BLECharacteristic* pCharacteristic) {
    String recordData = rcd.getCurrentRecordsAsJSON();
    pCharacteristic->setValue(recordData.c_str());
}

// ClearSchedulesCallback implementation
ClearSchedulesCallback::ClearSchedulesCallback(TaskScheduler* sched) : scheduler(sched) {}

void ClearSchedulesCallback::onWrite(BLECharacteristic* pCharacteristic) {
    String value = pCharacteristic->getValue();
    if (value == "CLEAR") {
        if (scheduler->clearSchedules()) {
            pCharacteristic->setValue("Schedules cleared successfully");
            Serial.println("✅ Schedules cleared via BLE command");
        } else {
            pCharacteristic->setValue("Failed to clear schedules");
            Serial.println("❌ Failed to clear schedules via BLE command");
        }
    } else {
        pCharacteristic->setValue("Invalid command. Send 'CLEAR' to reset schedules");
    }
}

RTCWriteCallback::RTCWriteCallback(RTCManager* rtcMgr) : rtc(rtcMgr) {}

void RTCWriteCallback::onWrite(BLECharacteristic* pCharacteristic) {
    String value = pCharacteristic->getValue();
    
    // Expected format: YYYY-MM-DD HH:MM:SS
    if (value.length() == 19) {
        int year = value.substring(0, 4).toInt();
        int month = value.substring(5, 7).toInt();
        int day = value.substring(8, 10).toInt();
        int hour = value.substring(11, 13).toInt();
        int minute = value.substring(14, 16).toInt();
        int second = value.substring(17, 19).toInt();
        
        // Validate the input
        if (year >= 2000 && year <= 2099 && 
            month >= 1 && month <= 12 && 
            day >= 1 && day <= 31 && 
            hour >= 0 && hour <= 23 && 
            minute >= 0 && minute <= 59 && 
            second >= 0 && second <= 59) {
            
            // Set the RTC
            rtc->setDateTime(year, month, day, hour, minute, second);
            
            // Confirm success
            pCharacteristic->setValue("RTC time set successfully");
            Serial.println("✅ RTC time updated via BLE command");
        } else {
            pCharacteristic->setValue("Invalid date/time values");
            Serial.println("❌ Invalid date/time values received via BLE");
        }
    } else {
        pCharacteristic->setValue("Invalid format. Use YYYY-MM-DD HH:MM:SS");
        Serial.println("❌ Invalid time format received via BLE");
    }
}

// ClearRecordsCallback implementation
ClearRecordsCallback::ClearRecordsCallback() {}

void ClearRecordsCallback::onWrite(BLECharacteristic* pCharacteristic) {
    String value = pCharacteristic->getValue();
    if (value == "CLEAR") {
        rcd.clearRecords();
        pCharacteristic->setValue("Records cleared successfully");
        Serial.println("✅ Records cleared via BLE command");
    } else {
        pCharacteristic->setValue("Invalid command. Send 'CLEAR' to reset records");
    }
}

// Add the fogger button callback implementation
void FoggerButtonCallback::onWrite(BLECharacteristic* pCharacteristic) {
    String value = pCharacteristic->getValue();
    if (value == "PRESS") {
        // Call the fogger button press function defined in main.ino
        pressFoggerButton();
        pCharacteristic->setValue("Fogger button pressed");
        Serial.println("Fogger button activated via BLE");
    } else {
        pCharacteristic->setValue("Invalid command. Send 'PRESS' to activate fogger");
        Serial.println("❌ Invalid fogger command received via BLE");
    }
}

// Light toggle callback implementations
ToggleLightCallback::ToggleLightCallback(TaskScheduler* sched) : scheduler(sched) {}

void ToggleLightCallback::onWrite(BLECharacteristic* pCharacteristic) {
    String value = pCharacteristic->getValue();
    if (value == "TOGGLE") {
        scheduler->toggleLight();
        pCharacteristic->setValue(scheduler->getLightState() ? "Light ON" : "Light OFF");
        Serial.println("Light toggled via BLE");
    } else {
        pCharacteristic->setValue("Invalid command. Send 'TOGGLE' to switch light state");
    }
}

void ToggleLightCallback::onRead(BLECharacteristic* pCharacteristic) {
    pCharacteristic->setValue(scheduler->getLightState() ? "Light ON" : "Light OFF");
}

// Water toggle callback implementations
ToggleWaterCallback::ToggleWaterCallback(TaskScheduler* sched) : scheduler(sched) {}

void ToggleWaterCallback::onWrite(BLECharacteristic* pCharacteristic) {
    String value = pCharacteristic->getValue();
    if (value == "TOGGLE") {
        scheduler->toggleWater();
        pCharacteristic->setValue(scheduler->getWaterState() ? "Water ON" : "Water OFF");
        Serial.println("Water toggled via BLE");
    } else {
        pCharacteristic->setValue("Invalid command. Send 'TOGGLE' to switch water state");
    }
}

void ToggleWaterCallback::onRead(BLECharacteristic* pCharacteristic) {
    pCharacteristic->setValue(scheduler->getWaterState() ? "Water ON" : "Water OFF");
}

// Fogger toggle callback implementations
ToggleFoggerCallback::ToggleFoggerCallback(TaskScheduler* sched) : scheduler(sched) {}

void ToggleFoggerCallback::onWrite(BLECharacteristic* pCharacteristic) {
    String value = pCharacteristic->getValue();
    if (value == "TOGGLE") {
        scheduler->toggleFogger();
        pCharacteristic->setValue(scheduler->getFoggerState() ? "Fogger ON" : "Fogger OFF");
        Serial.println("Fogger toggled via BLE");
    } else {
        pCharacteristic->setValue("Invalid command. Send 'TOGGLE' to switch fogger state");
    }
}

void ToggleFoggerCallback::onRead(BLECharacteristic* pCharacteristic) {
    pCharacteristic->setValue(scheduler->getFoggerState() ? "Fogger ON" : "Fogger OFF");
}
