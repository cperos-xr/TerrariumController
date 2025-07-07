/* BluetoothManager.cpp */
#include "BluetoothManager.h"

#define SERVICE_UUID           "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID_WATER "12345678-1234-5678-1234-56789abcdef3"
#define CHARACTERISTIC_UUID_RTC   "12345678-1234-5678-1234-56789abcdef4" // New UUID for RTC
#define CHARACTERISTIC_UUID_SCHEDULE   "12345678-1234-5678-1234-56789abcdef5" // New UUID for Schedule
#define CHARACTERISTIC_UUID_SENSOR_READ "12345678-1234-5678-1234-56789abcdef6" // New UUID for Sensor Read

void BluetoothManager::initBLE(TaskScheduler* sched, RTCManager* rtcMgr) {
    scheduler = sched;
    rtc = rtcMgr;

    BLEDevice::init("TerrariumController");
    BLEDevice::getAdvertising()->addServiceUUID(SERVICE_UUID);
    BLEDevice::getAdvertising()->start();

    pServer = BLEDevice::createServer();
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

    pService->start();
}

void BluetoothManager::startAdvertising() {
    BLEDevice::getAdvertising()->addServiceUUID(SERVICE_UUID);
    BLEDevice::getAdvertising()->start();
    Serial.println("🔵 BLE advertising started");
}

// WriteCallback implementation
WriteCallback::WriteCallback(TaskScheduler* sched) : scheduler(sched) {}

void WriteCallback::onWrite(BLECharacteristic* pCharacteristic) {
    String value = pCharacteristic->getValue(); // Arduino String from BLECharacteristic
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
    pCharacteristic->setValue(timeStr); // Set the current time as the characteristic value
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
    float tempC = snr.getCurrentTempC();
    float tempF = snr.getCurrentTempF();
    float humid = snr.getCurrentHumid();
    String sensorData = "Temperature: " + String(tempC) + "°C, " + String(tempF) + "°F | Humidity: " + String(humid) + "%";
    pCharacteristic->setValue(sensorData.c_str());
}
