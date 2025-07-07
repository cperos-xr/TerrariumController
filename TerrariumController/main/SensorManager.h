#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Wire.h>
#include <Arduino.h>

#define AHT_ADDRESS 0x38 // AHT25 I�C address

class SensorManager {
public:
    SensorManager();
    void initSensors();
    void printTempC();
    String getCurrentTempC();    // Fixed: String instead of string
    void printTempF();
    String getCurrentTempF();    // Fixed: String instead of string
    void printHumid();
    String getCurrentHumid();   // Fixed: String instead of string
    bool isSensorAvailable();

private:
    bool sensorInitialized;
    float lastTemperature;
    float lastHumidity;
    unsigned long lastReadTime;
    
    // AHT25 specific methods
    bool readSensor();
    void sendCommand(uint8_t cmd);
    bool waitForReady();
    uint32_t readRawData();

    float convertToFahrenheit(float tempC);
};

extern SensorManager snr; // Declare a global SensorManager instance

#endif
