#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Wire.h>
#include <Arduino.h>

#define AHT_ADDRESS 0x38 // AHT25 I2C address

class SensorManager {
public:
    SensorManager();
    void initSensors();
    void printTempC();
    float getCurrentTempC();
    void printTempF();
    float getCurrentTempF();
    void printHumid();
    float getCurrentHumid();
    bool isSensorAvailable();
    bool readSensor();
    String getSensorDataAsJSON(); // Fixed: Added missing semicolon

private:
    bool sensorInitialized;
    float lastTemperature;
    float lastHumidity;
    unsigned long lastReadTime;
    
    // AHT25 specific methods
    // Removed duplicate readSensor() declaration
    void sendCommand(uint8_t cmd);
    bool waitForReady();
    uint32_t readRawData();

    float convertToFahrenheit(float tempC);
};

extern SensorManager snr; // Declare a global SensorManager instance

#endif
