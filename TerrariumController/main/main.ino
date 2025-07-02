/**
 * Terrarium Controller for ESP32-C3 using BLE Manager
 * Files:
 *  - main.ino
 *  - RTCManager.h/.cpp
 *  - TaskScheduler.h/.cpp
 *  - BluetoothManager.h/.cpp
 *
 * BLE scheduling commands for LIGHT/WATER:
 * Format: TARGET,TYPE,h1,m1,d1[,h2,m2]
 * TYPE: ALWAYS_ON, DAILY, WEEKLY, TWICE_DAILY, TWICE_WEEKLY, MONTHLY, TWICE_MONTHLY
 * Duration in seconds; multiplied to ms internally.
 */

/* main.ino */
#include <Arduino.h>
#include <Wire.h>
#include "RTCManager.h"
#include "TaskScheduler.h"
#include "BluetoothManager.h"

// Pin definitions
const int PIN_LIGHT = 4;
const int PIN_WATER = 3;

RTCManager rtc;
TaskScheduler scheduler(PIN_LIGHT, PIN_WATER);
BluetoothManager ble;

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial port to connect. Needed for native USB
  rtc.initRTC();
  rtc.printCurrentTime();
  
  pinMode(PIN_LIGHT, OUTPUT);
  pinMode(PIN_WATER, OUTPUT);
  digitalWrite(PIN_LIGHT, LOW);
  digitalWrite(PIN_WATER, LOW);
  
  ble.initBLE(&scheduler, &rtc);
  ble.startAdvertising();

  Wire.begin();
  Serial.println("Scanning for I2C devices...");
  for (byte address = 1; address < 127; address++) {
      Wire.beginTransmission(address);
      if (Wire.endTransmission() == 0) {
          Serial.print("Found I2C device at address 0x");
          Serial.println(address, HEX);
      }
  }
  Serial.println("Scan complete.");
}

void loop() {
  scheduler.updateTasks(rtc.getCurrentTime());
  delay(1000);
}