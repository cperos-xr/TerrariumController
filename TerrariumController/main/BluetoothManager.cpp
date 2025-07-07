/* BluetoothManager.cpp */
#include "BluetoothManager.h"
#include "RecordManager.h"

#define SERVICE_UUID           "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID_WATER "12345678-1234-5678-1234-56789abcdef3"
#define CHARACTERISTIC_UUID_RTC   "12345678-1234-5678-1234-56789abcdef4" // New UUID for RTC
#define CHARACTERISTIC_UUID_SCHEDULE   "12345678-1234-5678-1234-56789abcdef5" // New UUID for Schedule
#define CHARACTERISTIC_UUID_SENSOR_READ "12345678-1234-5678-1234-56789abcdef6" // New UUID for Sensor Read
#define CHARACTERISTIC_UUID_RECORD_READ "12345678-1234-5678-1234-56789abcdef7" // New UUID for Record Read

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

    BLEDevice::init("TerrariumController");
    
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks()); // Add connection callbacks
    
    pService = pServer->createService(SERVICE_UUID);

    pRxWater = pService->createCharacteristic(
        CHARACTERISTIC_UUID_WATER,
        BLECharacteristic::PROPERTY_WRITE
    );
    pRxWater->addDescriptor(new BLE2902());
    pRxWater->setCallbacks(new WriteCallback(scheduler));

    // RTC characteristic
    pRtcTime = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RTC,
        BLECharacteristic::PROPERTY_READ
    );
    pRtcTime->addDescriptor(new BLE2902());
    pRtcTime->setCallbacks(new RTCReadCallback(rtc));

    // Schedule characteristic
    pScheduleRead = pService->createCharacteristic(
        CHARACTERISTIC_UUID_SCHEDULE,
        BLECharacteristic::PROPERTY_READ
    );
    pScheduleRead->addDescriptor(new BLE2902());
    pScheduleRead->setCallbacks(new ScheduleReadCallback(scheduler));

    pSensorRead = pService->createCharacteristic(
        CHARACTERISTIC_UUID_SENSOR_READ,
        BLECharacteristic::PROPERTY_READ
    );
    pSensorRead->addDescriptor(new BLE2902());
    pSensorRead->setCallbacks(new SensorReadCallback());

    pRecordRead = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RECORD_READ,
        BLECharacteristic::PROPERTY_READ
    );
    pRecordRead->addDescriptor(new BLE2902());
    pRecordRead->setCallbacks(new RecordReadCallback());

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
    String scheduleInfo = scheduler->getSchedulesAsString();
    pCharacteristic->setValue(scheduleInfo.c_str());
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
