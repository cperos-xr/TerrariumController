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
const int PIN_WATER = 5;
const int PIN_FOGGER = 20; // Fogger button simulation pin

#define SDA_PIN     6
#define SCL_PIN     7

RTCManager rtc;
TaskScheduler scheduler(PIN_LIGHT, PIN_WATER, PIN_FOGGER);
BluetoothManager ble;
DisplayManager dsp;
RecordManager rcd;

// Function to simulate fogger button press
void pressFoggerButton() {
    digitalWrite(PIN_FOGGER, HIGH);
    delay(200); // Short pulse - simulating button press
    digitalWrite(PIN_FOGGER, LOW);
    Serial.println("Fogger button pressed");
}

void setup() 
{
  Serial.begin(9600);
  // Try to mount; format on first failure
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed → formatting and retrying");
    LittleFS.format();
    if (!LittleFS.begin()) {
      Serial.println("!! LittleFS still failing");
      while (1) delay(1000);
    }
  }

  Serial.println("LittleFS mounted");
  rcd.initRecords();
  Serial.println("Setup started..."); // Debug message
  //while (!Serial); // Wait for serial to be ready
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

  rcd.loadRecords();
  scheduler.loadSchedules(); // Load saved schedules

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
  pinMode(PIN_FOGGER, OUTPUT); // Initialize fogger button pin
  
  digitalWrite(PIN_LIGHT, LOW);
  digitalWrite(PIN_WATER, LOW);
  digitalWrite(PIN_FOGGER, LOW); // Start with fogger button not pressed
  
  ble.initBLE(&scheduler, &rtc);
  ble.startAdvertising();
}

void loop() 
{
    static bool firstRun = true;
    
    if (firstRun) {
        Serial.println("===== FIRST RUN DEBUG =====");
        Serial.print("Initial low temp: ");
        Serial.println(rcd.lowTempOfTheDay.value);
        Serial.print("Initial low humid: ");
        Serial.println(rcd.lowHumidOfTheDay.value);
        firstRun = false;
    }

    Serial.print("Internal low temp: ");
    Serial.print(rcd.lowTempOfTheDay.value, 2);
    Serial.print(" | Internal low humid: ");
    Serial.println(rcd.lowHumidOfTheDay.value, 2);
    static unsigned long lastAdvertisingCheck = 0;
    unsigned long currentTime = millis();
    if (rcd.recordsChanged || currentTime - rcd.lastSaveTime >= 3600000) { // Save if changed or hourly
        rcd.saveRecords();
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
  
    // Make sure this is called to check schedules
    now = rtc.getCurrentTime();
    scheduler.updateTasks(now);
  
    // Add debug to verify schedules are being checked
    static unsigned long lastDebugMillis = 0;
    if (millis() - lastDebugMillis > 60000) { // Every minute
      lastDebugMillis = millis();
      now = rtc.getCurrentTime(); // Get fresh time
      Serial.println("Checking schedules at " + String(now.hour()) + ":" + String(now.minute()));
    }
}