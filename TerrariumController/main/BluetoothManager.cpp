/* BluetoothManager.cpp */
#include "BluetoothManager.h"

#define SERVICE_UUID           "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID_WATER "12345678-1234-5678-1234-56789abcdef3"
#define CHARACTERISTIC_UUID_LIGHT "12345678-1234-5678-1234-56789abcdef4"

void BluetoothManager::initBLE(TaskScheduler* sched, RTCManager* rtcMgr) {
  scheduler = sched;
  rtc = rtcMgr;

  BLEDevice::init("TerrariumController"); // Device name
  BLEDevice::getAdvertising()->addServiceUUID(SERVICE_UUID); // Add service UUID
  BLEDevice::getAdvertising()->start(); // Start advertising

  pServer = BLEDevice::createServer();
  pService = pServer->createService(SERVICE_UUID);

  pRxWater = pService->createCharacteristic(
    CHARACTERISTIC_UUID_WATER,
    BLECharacteristic::PROPERTY_WRITE
  );
  pRxWater->addDescriptor(new BLE2902());
  pRxWater->setCallbacks(new WriteCallback(scheduler));

  pRxLight = pService->createCharacteristic(
    CHARACTERISTIC_UUID_LIGHT,
    BLECharacteristic::PROPERTY_WRITE
  );
  pRxLight->addDescriptor(new BLE2902());
  pRxLight->setCallbacks(new WriteCallback(scheduler));

  pService->start();
}

void BluetoothManager::startAdvertising() {
  BLEDevice::getAdvertising()->addServiceUUID(SERVICE_UUID);
  BLEDevice::getAdvertising()->start();
  Serial.println("🔵 BLE advertising started");
}