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
#include "SensorManager.h"
#include "TaskScheduler.h"
#include "BluetoothManager.h"
#include "DisplayManager.h"
#include "RecordManager.h"

// Pin definitions
const int PIN_LIGHT = 4;
const int PIN_WATER = 3;

#define SDA_PIN     5
#define SCL_PIN     6

RTCManager rtc;
TaskScheduler scheduler(PIN_LIGHT, PIN_WATER);
BluetoothManager ble;
DisplayManager dsp;

void setup() 
{
  Serial.begin(9600); // Start serial communication
  Serial.println("Setup started..."); // Debug message
  while (!Serial); // Wait for serial to be ready
  Wire.begin(SDA_PIN, SCL_PIN);
  
  if (rtc.isRTCAvailable())
  {
      Serial.println("RTC detected at address 0x68.");
      rtc.initRTC(); // Initialize RTC
      Serial.println("RTC initialization finished."); // Debug message
      rtc.printCurrentTime(); // Print current time to serial
  }
  
  if(snr.isSensorAvailable()) 
  {
      Serial.println("AHT25 sensor detected at address 0x38.");
      snr.initSensors(); // Initialize AHT25 sensor
      Serial.println("Sensor initialization finished."); // Debug message
  }

  if(dsp.isDisplayAvailable()) 
  {
      Serial.println("Display detected at address 0x3C.");
      dsp.initDisplay(); // Initialize display
      dsp.clearDisplay();
      dsp.printMessage("Terrarium Controller Ready");
      Serial.println("Display initialization finished."); // Debug message
  }

  Serial.println("Scanning for I2C devices...");
  for (byte address = 1; address < 127; address++)
  {
      Wire.beginTransmission(address);
      if (Wire.endTransmission() == 0) {
          Serial.print("Found I2C device at address 0x");
          Serial.println(address, HEX);
      }
  }
  Serial.println("Scan complete.");

  
  pinMode(PIN_LIGHT, OUTPUT);
  pinMode(PIN_WATER, OUTPUT);
  digitalWrite(PIN_LIGHT, LOW);
  digitalWrite(PIN_WATER, LOW);
  
  ble.initBLE(&scheduler, &rtc);
  ble.startAdvertising();
}

void loop() 
{
    DateTime now = rtc.getCurrentTime();
    float temp = snr.getCurrentTempF();
    float humid = snr.getCurrentHumid();
    
    if (temp != -1)
    {
        rcd.analyzeReading(TEMPERATURE, temp, now);
    }
    
    if (humid != -1)
    {
        rcd.analyzeReading(HUMIDITY, humid, now);
    }
    
    scheduler.updateTasks(now);
    String msg = "Temp\n" + String(temp, 1) + "F\n\n" + "Humid\n" + String(humid, 1) + "%\n";
    dsp.printMessage(msg);
    rcd.printRecords(); // Print records to serial for debugging
    delay(1000);
}