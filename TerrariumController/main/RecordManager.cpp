#include "RecordManager.h"
#include "RTCManager.h"
#include <Arduino.h>

// Constructor
RecordManager::RecordManager() {
    initRecords();
}

void RecordManager::initRecords() {
    // Initialize all records with invalid values
    highTempOfTheWeek = createInvalidRecord(TEMPERATURE);
    lowTempOfTheWeek = createInvalidRecord(TEMPERATURE);
    highHumidOfTheWeek = createInvalidRecord(HUMIDITY);
    lowHumidOfTheWeek = createInvalidRecord(HUMIDITY);
    
    highTempOfTheDay = createInvalidRecord(TEMPERATURE);
    lowTempOfTheDay = createInvalidRecord(TEMPERATURE);
    highHumidOfTheDay = createInvalidRecord(HUMIDITY);
    lowHumidOfTheDay = createInvalidRecord(HUMIDITY);
    
    // Clear rolling buffers
    for (int i = 0; i < MAX_ROLLING_RECORDS; i++) {
        rollingTempHighs[i] = createInvalidRecord(TEMPERATURE);
        rollingTempLows[i] = createInvalidRecord(TEMPERATURE);
        rollingHumidHighs[i] = createInvalidRecord(HUMIDITY);
        rollingHumidLows[i] = createInvalidRecord(HUMIDITY);
    }
    
    Serial.println("RecordManager initialized with rolling records");
}

Record RecordManager::createInvalidRecord(SensorType type) {
    Record invalid;
    invalid.sensorType = type;
    // Use extremely high values for invalid records so they get replaced by any real reading
    invalid.value = (type == TEMPERATURE) ? 999.0 : 999.0; // Very high invalid values
    invalid.dateTime = DateTime(2000, 1, 1, 0, 0, 0); // Invalid date
    return invalid;
}

void RecordManager::analyzeReading(SensorType sensorType, float value, DateTime dateTime) {
    // Check for expired records and update rolling
    if (isNewDay(dateTime, highTempOfTheDay.dateTime)) {
        updateRollingDailyRecords(dateTime);
    }
    
    if (isNewWeek(dateTime, highTempOfTheWeek.dateTime)) {
        updateRollingWeeklyRecords(dateTime);
    }
    
    // Add to rolling buffer first
    addToRollingBuffer(sensorType, value, dateTime);
    
    // Update current records based on sensor type
    switch (sensorType) {
        case TEMPERATURE:
            // Daily temperature records
            if (highTempOfTheDay.value == 999.0 || value > highTempOfTheDay.value) {
                updateRecord(highTempOfTheDay, value, dateTime, true);
            }
            if (lowTempOfTheDay.value == 999.0 || value < lowTempOfTheDay.value) {
                updateRecord(lowTempOfTheDay, value, dateTime, false);
            }
            
            // Weekly temperature records
            if (highTempOfTheWeek.value == 999.0 || value > highTempOfTheWeek.value) {
                updateRecord(highTempOfTheWeek, value, dateTime, true);
            }
            if (lowTempOfTheWeek.value == 999.0 || value < lowTempOfTheWeek.value) {
                updateRecord(lowTempOfTheWeek, value, dateTime, false);
            }
            break;
            
        case HUMIDITY:
            // Daily humidity records
            if (highHumidOfTheDay.value == 999.0 || value > highHumidOfTheDay.value) {
                updateRecord(highHumidOfTheDay, value, dateTime, true);
            }
            if (lowHumidOfTheDay.value == 999.0 || value < lowHumidOfTheDay.value) {
                updateRecord(lowHumidOfTheDay, value, dateTime, false);
            }
            
            // Weekly humidity records
            if (highHumidOfTheWeek.value == 999.0 || value > highHumidOfTheWeek.value) {
                updateRecord(highHumidOfTheWeek, value, dateTime, true);
            }
            if (lowHumidOfTheWeek.value == 999.0 || value < lowHumidOfTheWeek.value) {
                updateRecord(lowHumidOfTheWeek, value, dateTime, false);
            }
            break;
    }
}


void RecordManager::addToRollingBuffer(SensorType sensorType, float value, DateTime dateTime) {
    Record newRecord;
    newRecord.sensorType = sensorType;
    newRecord.value = value;
    newRecord.dateTime = dateTime;
    
    if (sensorType == TEMPERATURE) {
        // Add to temperature buffers (shift and insert)
        addToBuffer(rollingTempHighs, newRecord, MAX_ROLLING_RECORDS);
        addToBuffer(rollingTempLows, newRecord, MAX_ROLLING_RECORDS);
    } else if (sensorType == HUMIDITY) {
        // Add to humidity buffers (shift and insert)
        addToBuffer(rollingHumidHighs, newRecord, MAX_ROLLING_RECORDS);
        addToBuffer(rollingHumidLows, newRecord, MAX_ROLLING_RECORDS);
    }
}

void RecordManager::addToBuffer(Record buffer[], const Record& newRecord, int bufferSize) {
    // Shift all records down by one position
    for (int i = bufferSize - 1; i > 0; i--) {
        buffer[i] = buffer[i - 1];
    }
    // Insert new record at the beginning
    buffer[0] = newRecord;
}

void RecordManager::updateRollingDailyRecords(DateTime currentTime) {
    Serial.println("?? Updating daily records (rolling)");
    
    // Find best values from the last 24 hours
    highTempOfTheDay = findBestInTimeRange(rollingTempHighs, currentTime, 1, true, TEMPERATURE);
    lowTempOfTheDay = findBestInTimeRange(rollingTempLows, currentTime, 1, false, TEMPERATURE);
    highHumidOfTheDay = findBestInTimeRange(rollingHumidHighs, currentTime, 1, true, HUMIDITY);
    lowHumidOfTheDay = findBestInTimeRange(rollingHumidLows, currentTime, 1, false, HUMIDITY);
    
    // Clean expired records from buffers
    cleanExpiredRecords(rollingTempHighs, currentTime, 1);
    cleanExpiredRecords(rollingTempLows, currentTime, 1);
    cleanExpiredRecords(rollingHumidHighs, currentTime, 1);
    cleanExpiredRecords(rollingHumidLows, currentTime, 1);
}

void RecordManager::updateRollingWeeklyRecords(DateTime currentTime) {
    Serial.println("?? Updating weekly records (rolling)");
    
    // Find best values from the last 7 days
    highTempOfTheWeek = findBestInTimeRange(rollingTempHighs, currentTime, 7, true, TEMPERATURE);
    lowTempOfTheWeek = findBestInTimeRange(rollingTempLows, currentTime, 7, false, TEMPERATURE);
    highHumidOfTheWeek = findBestInTimeRange(rollingHumidHighs, currentTime, 7, true, HUMIDITY);
    lowHumidOfTheWeek = findBestInTimeRange(rollingHumidLows, currentTime, 7, false, HUMIDITY);
    
    // Clean expired records from buffers
    cleanExpiredRecords(rollingTempHighs, currentTime, 7);
    cleanExpiredRecords(rollingTempLows, currentTime, 7);
    cleanExpiredRecords(rollingHumidHighs, currentTime, 7);
    cleanExpiredRecords(rollingHumidLows, currentTime, 7);
}

Record RecordManager::findBestInTimeRange(Record buffer[], DateTime currentTime, int daysBack, bool findHigh, SensorType type) {
    Record best = createInvalidRecord(type);
    long cutoffTime = currentTime.unixtime() - (daysBack * 86400);
    
    for (int i = 0; i < MAX_ROLLING_RECORDS; i++) {
        // Skip invalid records
        if (buffer[i].value == 999.0) continue;
        
        // Skip records older than cutoff
        if (buffer[i].dateTime.unixtime() < cutoffTime) continue;
        
        // Check if this is a better record
        if (best.value == 999.0) {
            best = buffer[i]; // First valid record found
        } else if (findHigh && buffer[i].value > best.value) {
            best = buffer[i]; // New high found
        } else if (!findHigh && buffer[i].value < best.value) {
            best = buffer[i]; // New low found
        }
    }
    
    return best;
}

void RecordManager::cleanExpiredRecords(Record buffer[], DateTime currentTime, int daysBack) {
    long cutoffTime = currentTime.unixtime() - (daysBack * 86400);
    
    for (int i = 0; i < MAX_ROLLING_RECORDS; i++) {
        if (buffer[i].dateTime.unixtime() < cutoffTime) {
            buffer[i] = createInvalidRecord(buffer[i].sensorType);
        }
    }
}

void RecordManager::updateRecord(Record& record, float value, DateTime dateTime, bool isHigh) {
    record.value = value;
    record.dateTime = dateTime;
    
    // Optional: Print when a new record is set
    String typeStr = (record.sensorType == TEMPERATURE) ? "Temperature" : "Humidity";
    String recordStr = isHigh ? "High" : "Low";
    Serial.print("New ");
    Serial.print(recordStr);
    Serial.print(" ");
    Serial.print(typeStr);
    Serial.print(": ");
    Serial.print(value, 1);
    Serial.print((record.sensorType == TEMPERATURE) ? "C" : "%");
    Serial.print(" at ");
    Serial.print(dateTime.year());
    Serial.print("-");
    Serial.print(dateTime.month());
    Serial.print("-");
    Serial.print(dateTime.day());
    Serial.print(" ");
    Serial.print(dateTime.hour());
    Serial.print(":");
    Serial.println(dateTime.minute());
}

bool RecordManager::isNewDay(const DateTime& current, const DateTime& recorded) {
    // Check if it's a different day (year, month, or day different)
    return (current.year() != recorded.year() || 
            current.month() != recorded.month() || 
            current.day() != recorded.day());
}

bool RecordManager::isNewWeek(const DateTime& current, const DateTime& recorded) {
    // Calculate if we've crossed into a new week (Sunday = 0)
    // If more than 7 days difference or different week boundary
    
    // Simple approach: if more than 7 days have passed
    long daysDiff = (current.unixtime() - recorded.unixtime()) / 86400; // 86400 seconds in a day
    
    if (daysDiff >= 7) {
        return true;
    }
    
    // Check if we've crossed a Sunday (week boundary)
    uint8_t currentDayOfWeek = current.dayOfTheWeek();
    uint8_t recordedDayOfWeek = recorded.dayOfTheWeek();
    
    // If current day is before recorded day in the week, we've crossed Sunday
    return (currentDayOfWeek < recordedDayOfWeek);
}

// Rest of the methods remain the same...
void RecordManager::clearRecords() {
    Serial.println("Clearing all records");
    initRecords();
}

void RecordManager::printRecords() {
    Serial.println("=== CURRENT RECORDS ===");
    
    Serial.println("DAILY RECORDS:");
    Serial.print("High Temp: ");
    if (highTempOfTheDay.value != 999.0) {
        Serial.print(highTempOfTheDay.value, 1);
        Serial.print("C (");
        printDateTime(highTempOfTheDay.dateTime);
        Serial.println(")");
    } else {
        Serial.println("No data");
    }
    
    Serial.print("Low Temp: ");
    if (lowTempOfTheDay.value != 999.0) {
        Serial.print(lowTempOfTheDay.value, 1);
        Serial.print("C (");
        printDateTime(lowTempOfTheDay.dateTime);
        Serial.println(")");
    } else {
        Serial.println("No data");
    }
    
    Serial.print("High Humidity: ");
    if (highHumidOfTheDay.value != 999.0) {
        Serial.print(highHumidOfTheDay.value, 1);
        Serial.print("% (");
        printDateTime(highHumidOfTheDay.dateTime);
        Serial.println(")");
    } else {
        Serial.println("No data");
    }
    
    Serial.print("Low Humidity: ");
    if (lowHumidOfTheDay.value != 999.0) {
        Serial.print(lowHumidOfTheDay.value, 1);
        Serial.print("% (");
        printDateTime(lowHumidOfTheDay.dateTime);
        Serial.println(")");
    } else {
        Serial.println("No data");
    }
    
    Serial.println("WEEKLY RECORDS:");
    Serial.print("High Temp: ");
    if (highTempOfTheWeek.value != 999.0) {
        Serial.print(highTempOfTheWeek.value, 1);
        Serial.print("C (");
        printDateTime(highTempOfTheWeek.dateTime);
        Serial.println(")");
    } else {
        Serial.println("No data");
    }
    
    Serial.print("Low Temp: ");
    if (lowTempOfTheWeek.value != 999.0) {
        Serial.print(lowTempOfTheWeek.value, 1);
        Serial.print("C (");
        printDateTime(lowTempOfTheWeek.dateTime);
        Serial.println(")");
    } else {
        Serial.println("No data");
    }
    
    Serial.print("High Humidity: ");
    if (highHumidOfTheWeek.value != 999.0) {
        Serial.print(highHumidOfTheWeek.value, 1);
        Serial.print("% (");
        printDateTime(highHumidOfTheWeek.dateTime);
        Serial.println(")");
    } else {
        Serial.println("No data");
    }
    
    Serial.print("Low Humidity: ");
    if (lowHumidOfTheWeek.value != 999.0) {
        Serial.print(lowHumidOfTheWeek.value, 1);
        Serial.print("% (");
        printDateTime(lowHumidOfTheWeek.dateTime);
        Serial.println(")");
    } else {
        Serial.println("No data");
    }
}

void RecordManager::printDateTime(const DateTime& dt) {
    Serial.print(dt.month());
    Serial.print("/");
    Serial.print(dt.day());
    Serial.print(" ");
    Serial.print(dt.hour());
    Serial.print(":");
    if (dt.minute() < 10) Serial.print("0");
    Serial.print(dt.minute());
}

// Getter methods - Updated to check for invalid values
float RecordManager::getHighTempDaily() const {
    return (highTempOfTheDay.value != 999.0) ? highTempOfTheDay.value : NAN;
}

float RecordManager::getLowTempDaily() const {
    return (lowTempOfTheDay.value != 999.0) ? lowTempOfTheDay.value : NAN;
}

float RecordManager::getHighHumidDaily() const {
    return (highHumidOfTheDay.value != 999.0) ? highHumidOfTheDay.value : NAN;
}

float RecordManager::getLowHumidDaily() const {
    return (lowHumidOfTheDay.value != 999.0) ? lowHumidOfTheDay.value : NAN;
}

float RecordManager::getHighTempWeekly() const {
    return (highTempOfTheWeek.value != 999.0) ? highTempOfTheWeek.value : NAN;
}

float RecordManager::getLowTempWeekly() const {
    return (lowTempOfTheWeek.value != 999.0) ? lowTempOfTheWeek.value : NAN;
}

float RecordManager::getHighHumidWeekly() const {
    return (highHumidOfTheWeek.value != 999.0) ? highHumidOfTheWeek.value : NAN;
}

float RecordManager::getLowHumidWeekly() const {
    return (lowHumidOfTheWeek.value != 999.0) ? lowHumidOfTheWeek.value : NAN;
}


String RecordManager::getCurrentRecordsAsJSON() const {
    String json = "{";
    
    // Daily records
    json += "\"daily\":{";
    json += "\"temperature\":{";
    
    float highTempDaily = getHighTempDaily();
    float lowTempDaily = getLowTempDaily();
    
    if (isnan(highTempDaily)) {
        json += "\"high\":null,";
    } else {
        json += "\"high\":" + String(highTempDaily, 1) + ",";
    }
    
    if (isnan(lowTempDaily)) {
        json += "\"low\":null";
    } else {
        json += "\"low\":" + String(lowTempDaily, 1);
    }
    
    json += "},";
    json += "\"humidity\":{";
    
    float highHumidDaily = getHighHumidDaily();
    float lowHumidDaily = getLowHumidDaily();
    
    if (isnan(highHumidDaily)) {
        json += "\"high\":null,";
    } else {
        json += "\"high\":" + String(highHumidDaily, 1) + ",";
    }
    
    if (isnan(lowHumidDaily)) {
        json += "\"low\":null";
    } else {
        json += "\"low\":" + String(lowHumidDaily, 1);
    }
    
    json += "}";
    json += "},";
    
    // Weekly records (same pattern)
    json += "\"weekly\":{";
    json += "\"temperature\":{";
    
    float highTempWeekly = getHighTempWeekly();
    float lowTempWeekly = getLowTempWeekly();
    
    if (isnan(highTempWeekly)) {
        json += "\"high\":null,";
    } else {
        json += "\"high\":" + String(highTempWeekly, 1) + ",";
    }
    
    if (isnan(lowTempWeekly)) {
        json += "\"low\":null";
    } else {
        json += "\"low\":" + String(lowTempWeekly, 1);
    }
    
    json += "},";
    json += "\"humidity\":{";
    
    float highHumidWeekly = getHighHumidWeekly();
    float lowHumidWeekly = getLowHumidWeekly();
    
    if (isnan(highHumidWeekly)) {
        json += "\"high\":null,";
    } else {
        json += "\"high\":" + String(highHumidWeekly, 1) + ",";
    }
    
    if (isnan(lowHumidWeekly)) {
        json += "\"low\":null";
    } else {
        json += "\"low\":" + String(lowHumidWeekly, 1);
    }
    
    json += "}";
    json += "}";
    
    json += "}";
    return json;
}


// Global instance
RecordManager rcd;

