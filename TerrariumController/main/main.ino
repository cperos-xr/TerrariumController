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

  rcd.loadRecordsFromEEPROM();

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

    Serial.print("Internal low temp: ");
    Serial.print(rcd.lowTempOfTheDay.value, 2);
    Serial.print(" | Internal low humid: ");
    Serial.println(rcd.lowHumidOfTheDay.value, 2);
    static unsigned long lastAdvertisingCheck = 0;
    unsigned long currentTime = millis();
    if (rcd.recordsChanged || currentTime - rcd.lastSaveTime >= 3600000) { // Save if changed or hourly
        rcd.saveRecordsToEEPROM();
        rcd.recordsChanged = false; // Reset the flag
        rcd.lastSaveTime = currentTime; // Update the last save time
    }

    DateTime now = rtc.getCurrentTime();
    float tempC = snr.getCurrentTempC();
    float tempF = snr.getCurrentTempF(); // Get Fahrenheit temp instead
    float humid = snr.getCurrentHumid();
    
    if (tempC != -1)
    {
        rcd.analyzeReading(TEMPERATURE, tempC, now); // Pass Celsius for records
    }
    
    if (humid != -1)
    {
        rcd.analyzeReading(HUMIDITY, humid, now);
    }
    
    scheduler.updateTasks(now);
    String msg = "Temp\n" + String(tempC, 1) + "C\n\n" + "Humid\n" + String(humid, 1) + "%\n";
    dsp.printMessage(msg);
    
    rcd.printRecords();
    delay(3000);
    float highTemp = rcd.getHighTempDaily();
    float lowTemp = rcd.getLowTempDaily();

    float highHumid = rcd.getHighHumidDaily();
    float lowHumid = rcd.getLowHumidDaily();

    dsp.clearDisplay();
    dsp.printMessage("High\nTemp\n" + String(highTemp, 1) + "C\n\n" +
                     "Low\nTemp\n" + String(lowTemp, 1) + "C\n\n" +
                     "High\nHumid\n" + String(highHumid, 1) + "%\n\n" +
                     "Low\nHumid\n" + String(lowHumid, 1) + "%");
    delay(3000);

    Serial.print("Low Temp Check - Is max: ");
    Serial.print((rcd.lowTempOfTheDay.value == std::numeric_limits<float>::max()) ? "YES" : "NO");
    Serial.print(", Value: ");
    Serial.println(rcd.lowTempOfTheDay.value);

    Serial.print("Current Temp C: ");
    Serial.println(tempC);
}