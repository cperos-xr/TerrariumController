#include "SensorManager.h"
#include <Arduino.h>

// AHT25 Commands
#define AHT_CMD_INIT        0xBE
#define AHT_CMD_MEASURE     0xAC
#define AHT_CMD_SOFT_RESET  0xBA
#define AHT_STATUS_BUSY     0x80
#define AHT_STATUS_CALIBRATED 0x08

// Timing constants
#define AHT_MEASURE_DELAY   80   // milliseconds
#define AHT_INIT_DELAY      40   // milliseconds
#define AHT_READ_INTERVAL   2000 // milliseconds between readings

SensorManager::SensorManager() 
    : sensorInitialized(false)
    , lastTemperature(0.0)
    , lastHumidity(0.0)
    , lastReadTime(0)
{
}

void SensorManager::initSensors() {
    Wire.begin();
    delay(40); // Power-on delay for AHT25
    
    if (!isSensorAvailable()) {
        Serial.println("AHT25 sensor not found!");
        sensorInitialized = false;
        return;
    }
    
    // Soft reset
    sendCommand(AHT_CMD_SOFT_RESET);
    delay(20);
    
    // Initialize sensor
    Wire.beginTransmission(AHT_ADDRESS);
    Wire.write(AHT_CMD_INIT);
    Wire.write(0x08);
    Wire.write(0x00);
    Wire.endTransmission();
    
    delay(AHT_INIT_DELAY);
    
    // Check if calibrated
    Wire.requestFrom(AHT_ADDRESS, 1);
    if (Wire.available()) {
        uint8_t status = Wire.read();
        if (status & AHT_STATUS_CALIBRATED) {
            sensorInitialized = true;
            Serial.println("AHT25 sensor initialized successfully");
        } else {
            Serial.println("AHT25 sensor calibration failed");
            sensorInitialized = false;
        }
    }
}

bool SensorManager::isSensorAvailable() {
    Wire.beginTransmission(AHT_ADDRESS);
    return (Wire.endTransmission() == 0);
}

bool SensorManager::readSensor() {
    if (!sensorInitialized) {
        return false;
    }
    
    // Don't read too frequently
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime < AHT_READ_INTERVAL) {
        return true; // Return last values
    }
    
    // Start measurement
    Wire.beginTransmission(AHT_ADDRESS);
    Wire.write(AHT_CMD_MEASURE);
    Wire.write(0x33);
    Wire.write(0x00);
    Wire.endTransmission();
    
    // Wait for measurement to complete
    delay(AHT_MEASURE_DELAY);
    
    if (!waitForReady()) {
        Serial.println("AHT25 measurement timeout");
        return false;
    }
    
    // Read 6 bytes of data
    Wire.requestFrom(AHT_ADDRESS, 6);
    if (Wire.available() < 6) {
        Serial.println("AHT25 insufficient data");
        return false;
    }
    
    uint8_t data[6];
    for (int i = 0; i < 6; i++) {
        data[i] = Wire.read();
    }
    
    // Parse humidity (20 bits)
    uint32_t rawHumidity = ((uint32_t)data[1] << 12) | 
                          ((uint32_t)data[2] << 4) | 
                          ((uint32_t)data[3] >> 4);
    
    // Parse temperature (20 bits)
    uint32_t rawTemperature = (((uint32_t)data[3] & 0x0F) << 16) | 
                             ((uint32_t)data[4] << 8) | 
                             (uint32_t)data[5];
    
    // Convert to actual values
    lastHumidity = ((float)rawHumidity * 100.0) / 1048576.0; // 2^20
    lastTemperature = (((float)rawTemperature * 200.0) / 1048576.0) - 50.0;
    
    lastReadTime = currentTime;
    return true;
}

bool SensorManager::waitForReady() {
    unsigned long startTime = millis();
    while (millis() - startTime < 100) { // 100ms timeout
        Wire.requestFrom(AHT_ADDRESS, 1);
        if (Wire.available()) {
            uint8_t status = Wire.read();
            if (!(status & AHT_STATUS_BUSY)) {
                return true; // Not busy, ready
            }
        }
        delay(5);
    }
    return false; // Timeout
}

void SensorManager::sendCommand(uint8_t cmd) {
    Wire.beginTransmission(AHT_ADDRESS);
    Wire.write(cmd);
    Wire.endTransmission();
}

String SensorManager::getCurrentTempC() {
    if (readSensor()) {
        return String(lastTemperature, 1) + "°C";
    }
    return "Error";
}

String SensorManager::getCurrentHumid() {
    if (readSensor()) {
        return String(lastHumidity, 1) + "%";
    }
    return "Error";
}

void SensorManager::printTempC() {
    if (readSensor()) {
        Serial.print("Temperature: ");
        Serial.print(lastTemperature, 1);
        Serial.println("°C");
    } else {
        Serial.println("Failed to read temperature");
    }
}

void SensorManager::printTempF() {
    if (readSensor()) {
        Serial.print("Temperature: ");
        float tempF = convertToFahrenheit(lastTemperature);
        Serial.print(tempF, 1);
        Serial.println("°F");
    } else {
        Serial.println("Failed to read temperature");
    }
}

float SensorManager::convertToFahrenheit(uint32_t rawData) {
    return (rawData * 9.0 / 5.0) + 32.0;
}

void SensorManager::printHumid() {
    if (readSensor()) {
        Serial.print("Humidity: ");
        Serial.print(lastHumidity, 1);
        Serial.println("%");
    } else {
        Serial.println("Failed to read humidity");
    }
}

// Global instance
SensorManager snr;

