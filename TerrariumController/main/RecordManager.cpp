#include "RecordManager.h"
#include "RTCManager.h"
#include <Arduino.h>

// Constructor
RecordManager::RecordManager() : lastSaveTime(0) {
    initRecords();
}

void RecordManager::initRecords() {

    loadRecordsFromEEPROM();
    // Initialize HIGH records with very LOW values (so any real reading will be higher)
    highTempOfTheWeek = createInvalidHighRecord(TEMPERATURE);
    highHumidOfTheWeek = createInvalidHighRecord(HUMIDITY);
    highTempOfTheDay = createInvalidHighRecord(TEMPERATURE);
    highHumidOfTheDay = createInvalidHighRecord(HUMIDITY);
    
    // Initialize LOW records with very HIGH values (so any real reading will be lower)
    lowTempOfTheWeek = createInvalidLowRecord(TEMPERATURE);
    lowHumidOfTheWeek = createInvalidLowRecord(HUMIDITY);
    lowTempOfTheDay = createInvalidLowRecord(TEMPERATURE);
    lowHumidOfTheDay = createInvalidLowRecord(HUMIDITY);
    
    Serial.print("Initialized lowTempOfTheDay to: ");
    Serial.println(lowTempOfTheDay.value);
    Serial.print("Initialized lowHumidOfTheDay to: ");
    Serial.println(lowHumidOfTheDay.value);
    
    // Clear rolling buffers
    for (int i = 0; i < MAX_ROLLING_RECORDS; i++) {
        rollingTempHighs[i] = createInvalidHighRecord(TEMPERATURE);
        rollingTempLows[i] = createInvalidLowRecord(TEMPERATURE);
        rollingHumidHighs[i] = createInvalidHighRecord(HUMIDITY);
        rollingHumidLows[i] = createInvalidLowRecord(HUMIDITY);
    }

    
    
    Serial.println("RecordManager initialized with rolling records");
}

Record RecordManager::createInvalidRecord(SensorType type) {
    Record invalid;
    invalid.sensorType = type;
    invalid.dateTime = DateTime(2000, 1, 1, 0, 0, 0); // Invalid date
    
    // For temperature: use very high value for lows, very low value for highs
    // For humidity: use very high value for lows, very low value for highs
    if (type == TEMPERATURE) {
        invalid.value = 999.0; // This will work for both high and low initialization
    } else { // HUMIDITY
        invalid.value = 999.0; // This will work for both high and low initialization
    }
    
    return invalid;
}

Record RecordManager::createInvalidLowRecord(SensorType type) {
    Record invalid;
    invalid.sensorType = type;
    invalid.value      = std::numeric_limits<float>::max();  // Infinity sentinel
    invalid.dateTime   = DateTime(2000,1,1,0,0,0);
    return invalid;
}

Record RecordManager::createInvalidHighRecord(SensorType type) {
    Record invalid;
    invalid.sensorType = type;
    invalid.value      = -std::numeric_limits<float>::max(); // -infinity sentinel
    invalid.dateTime   = DateTime(2000,1,1,0,0,0);
    return invalid;
}

void RecordManager::analyzeReading(SensorType sensorType, float value, DateTime dateTime) {
    // Force initialization if still at 0
    if (lowTempOfTheDay.value == 0) {
        lowTempOfTheDay.value = std::numeric_limits<float>::max();
        lowTempOfTheDay.dateTime = dateTime;
    }
    
    if (lowHumidOfTheDay.value == 0) {
        lowHumidOfTheDay.value = std::numeric_limits<float>::max();
        lowHumidOfTheDay.dateTime = dateTime;
    }
    
    if (lowTempOfTheWeek.value == 0) {
        lowTempOfTheWeek.value = std::numeric_limits<float>::max();
        lowTempOfTheWeek.dateTime = dateTime;
    }
    
    if (lowHumidOfTheWeek.value == 0) {
        lowHumidOfTheWeek.value = std::numeric_limits<float>::max();
        lowHumidOfTheWeek.dateTime = dateTime;
    }
    
    // Check for expired records and update rolling
    if (isNewDay(dateTime, highTempOfTheDay.dateTime)) {
        // Reset daily records for new day
        highTempOfTheDay = createInvalidHighRecord(TEMPERATURE);
        lowTempOfTheDay = createInvalidLowRecord(TEMPERATURE);
        highHumidOfTheDay = createInvalidHighRecord(HUMIDITY);
        lowHumidOfTheDay = createInvalidLowRecord(HUMIDITY);
    }
    
    if (isNewWeek(dateTime, highTempOfTheWeek.dateTime)) {
        // Reset weekly records for new week
        highTempOfTheWeek = createInvalidHighRecord(TEMPERATURE);
        lowTempOfTheWeek = createInvalidLowRecord(TEMPERATURE);
        highHumidOfTheWeek = createInvalidHighRecord(HUMIDITY);
        lowHumidOfTheWeek = createInvalidLowRecord(HUMIDITY);
    }
    
    // Add to rolling buffer first
    addToRollingBuffer(sensorType, value, dateTime);
    
    
    // Update current records based on sensor type
    switch (sensorType) {
        case TEMPERATURE:
            // Daily temperature records
            if (highTempOfTheDay.value == -std::numeric_limits<float>::max() || value > highTempOfTheDay.value) {
                updateRecord(highTempOfTheDay, value, dateTime, true);
                Serial.print("Updated daily high temp to: ");
                Serial.println(value);
            }
            
            // Force update if still at default value or new low
            if (lowTempOfTheDay.value == std::numeric_limits<float>::max() || value < lowTempOfTheDay.value) {
                updateRecord(lowTempOfTheDay, value, dateTime, false);
                Serial.print("Updated daily low temp to: ");
                Serial.println(value);
            }
            
            // Weekly temperature records
            if (highTempOfTheWeek.value == -std::numeric_limits<float>::max() || value > highTempOfTheWeek.value) {
                updateRecord(highTempOfTheWeek, value, dateTime, true);
            }
            
            if (lowTempOfTheWeek.value == std::numeric_limits<float>::max() || value < lowTempOfTheWeek.value) {
                updateRecord(lowTempOfTheWeek, value, dateTime, false);
                Serial.print("Updated weekly low temp to: ");
                Serial.println(value);
            }
            break;
            
        case HUMIDITY:
            // Daily humidity records
            if (highHumidOfTheDay.value == -std::numeric_limits<float>::max() || value > highHumidOfTheDay.value) {
                updateRecord(highHumidOfTheDay, value, dateTime, true);
            }
            if (lowHumidOfTheDay.value == std::numeric_limits<float>::max() || value < lowHumidOfTheDay.value) {
                updateRecord(lowHumidOfTheDay, value, dateTime, false);
                Serial.print("Updated daily low humidity to: ");
                Serial.println(value);
            }
            
            // Weekly humidity records
            if (highHumidOfTheWeek.value == -std::numeric_limits<float>::max() || value > highHumidOfTheWeek.value) {
                updateRecord(highHumidOfTheWeek, value, dateTime, true);
            }
            if (lowHumidOfTheWeek.value == std::numeric_limits<float>::max() || value < lowHumidOfTheWeek.value) {
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
    // Shift all records down to one position
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
    Record best;
    if (findHigh) {
        best = createInvalidHighRecord(type); // Initialize with very low value
    } else {
        best = createInvalidLowRecord(type);  // Initialize with very high value
    }
    
    long cutoffTime = currentTime.unixtime() - (daysBack * 86400);
    
    for (int i = 0; i < MAX_ROLLING_RECORDS; i++) {
        // Skip invalid or empty records
        if (buffer[i].dateTime.year() < 2020) continue; // Skip records with invalid dates
        
        // Skip records older than cutoff
        if (buffer[i].dateTime.unixtime() < cutoffTime) continue;
        
        // For first valid record or better records
        if ((findHigh && (best.value == -std::numeric_limits<float>::max() || buffer[i].value > best.value)) ||
            (!findHigh && (best.value == std::numeric_limits<float>::max() || buffer[i].value < best.value))) {
            best = buffer[i];
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
    String unit = (record.sensorType == TEMPERATURE) ? "C" : "%";
    
    Serial.print("New ");
    Serial.print(recordStr);
    Serial.print(" ");
    Serial.print(typeStr);
    Serial.print(": ");
    Serial.print(value, 1);
    Serial.print(unit);
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

    if (record.value != value || record.dateTime != dateTime)
    {
        record.value = value;
        record.dateTime = dateTime;
        recordsChanged = true; // Mark records as changed
    }
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
    if (highTempOfTheDay.value != -999.0 && highTempOfTheDay.value != 999.0) {
        Serial.print(highTempOfTheDay.value, 1);
        Serial.print("F (");  // Changed from "C" to "F"
        printDateTime(highTempOfTheDay.dateTime);
        Serial.println(")");
    } else {
        Serial.println("No data");
    }
    
    Serial.print("Low Temp: ");
    if (lowTempOfTheDay.value != std::numeric_limits<float>::max()) {
        Serial.print(lowTempOfTheDay.value, 1);
        Serial.print("F (");
        printDateTime(lowTempOfTheDay.dateTime);
        Serial.println(")");
    } else {
        Serial.println("No data");
    }
    
    Serial.print("High Humidity: ");
    if (highHumidOfTheDay.value != -999.0 && highHumidOfTheDay.value != 999.0) {
        Serial.print(highHumidOfTheDay.value, 1);
        Serial.print("% (");
        printDateTime(highHumidOfTheDay.dateTime);
        Serial.println(")");
    } else {
        Serial.println("No data");
    }
    
    Serial.print("Low Humidity: ");
    if (lowHumidOfTheDay.value != std::numeric_limits<float>::max()) {
        Serial.print(lowHumidOfTheDay.value, 1);
        Serial.print("% (");
        printDateTime(lowHumidOfTheDay.dateTime);
        Serial.println(")");
    } else {
        Serial.println("No data");
    }
    
    Serial.println("WEEKLY RECORDS:");
    Serial.print("High Temp: ");
    if (highTempOfTheWeek.value != -999.0 && highTempOfTheWeek.value != 999.0) {
        Serial.print(highTempOfTheWeek.value, 1);
        Serial.print("F (");  // Changed from "C" to "F"
        printDateTime(highTempOfTheWeek.dateTime);
        Serial.println(")");
    } else {
        Serial.println("No data");
    }
    
    Serial.print("Low Temp: ");
    if (lowTempOfTheWeek.value != std::numeric_limits<float>::max()) {
        Serial.print(lowTempOfTheWeek.value, 1);
        Serial.print("F (");  // Changed from "C" to "F"
        printDateTime(lowTempOfTheWeek.dateTime);
        Serial.println(")");
    } else {
        Serial.println("No data");
    }
    
    Serial.print("High Humidity: ");
    if (highHumidOfTheWeek.value != -999.0 && highHumidOfTheWeek.value != 999.0) {
        Serial.print(highHumidOfTheWeek.value, 1);
        Serial.print("% (");
        printDateTime(highHumidOfTheWeek.dateTime);
        Serial.println(")");
    } else {
        Serial.println("No data");
    }
    
    Serial.print("Low Humidity: ");
    if (lowHumidOfTheWeek.value != std::numeric_limits<float>::max()) {
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
    return (highTempOfTheDay.value != -std::numeric_limits<float>::max()) ? highTempOfTheDay.value : NAN;
}

float RecordManager::getLowTempDaily() const {
    return (lowTempOfTheDay.value != std::numeric_limits<float>::max()) ? lowTempOfTheDay.value : NAN;
}

float RecordManager::getHighHumidDaily() const {
    return (highHumidOfTheDay.value != -std::numeric_limits<float>::max()) ? highHumidOfTheDay.value : NAN;
}

float RecordManager::getLowHumidDaily() const {
    return (lowHumidOfTheDay.value != std::numeric_limits<float>::max()) ? lowHumidOfTheDay.value : NAN;
}

float RecordManager::getHighTempWeekly() const {
    return (highTempOfTheWeek.value != -std::numeric_limits<float>::max()) ? highTempOfTheWeek.value : NAN;
}

float RecordManager::getLowTempWeekly() const {
    return (lowTempOfTheWeek.value != std::numeric_limits<float>::max()) ? lowTempOfTheWeek.value : NAN;
}

float RecordManager::getHighHumidWeekly() const {
    return (highHumidOfTheWeek.value != -std::numeric_limits<float>::max()) ? highHumidOfTheWeek.value : NAN;
}

float RecordManager::getLowHumidWeekly() const {
    return (lowHumidOfTheWeek.value != std::numeric_limits<float>::max()) ? lowHumidOfTheWeek.value : NAN;
}

void RecordManager::saveRecordsToEEPROM() {
    EEPROM.begin(EEPROM_SIZE);
    int addr = EEPROM_ADDR_RECORDS;

    // Save daily and weekly records first
    EEPROM.put(addr, highTempOfTheDay); addr += sizeof(Record);
    EEPROM.put(addr, lowTempOfTheDay); addr += sizeof(Record);
    EEPROM.put(addr, highHumidOfTheDay); addr += sizeof(Record);
    EEPROM.put(addr, lowHumidOfTheDay); addr += sizeof(Record);
    
    EEPROM.put(addr, highTempOfTheWeek); addr += sizeof(Record);
    EEPROM.put(addr, lowTempOfTheWeek); addr += sizeof(Record);
    EEPROM.put(addr, highHumidOfTheWeek); addr += sizeof(Record);
    EEPROM.put(addr, lowHumidOfTheWeek); addr += sizeof(Record);

    // Then save rolling buffers
    auto dumpBuffer = [&](Record buffer[]) {
        for (int i = 0; i < MAX_ROLLING_RECORDS; i++) {
            EEPROM.put(addr, buffer[i].dateTime.unixtime()); addr += sizeof(uint32_t);
            EEPROM.put(addr, buffer[i].value); addr += sizeof(float);
            EEPROM.put(addr, buffer[i].sensorType); addr += sizeof(uint8_t);
        }
    };

    dumpBuffer(rollingTempHighs);
    dumpBuffer(rollingTempLows);
    dumpBuffer(rollingHumidHighs);
    dumpBuffer(rollingHumidLows);

    EEPROM.commit();
    Serial.println("Records saved to EEPROM.");
}

void RecordManager::loadRecordsFromEEPROM() {
    EEPROM.begin(EEPROM_SIZE);
    int addr = EEPROM_ADDR_RECORDS;

    // Load daily and weekly records first
    EEPROM.get(addr, highTempOfTheDay); addr += sizeof(Record);
    EEPROM.get(addr, lowTempOfTheDay); addr += sizeof(Record);
    EEPROM.get(addr, highHumidOfTheDay); addr += sizeof(Record);
    EEPROM.get(addr, lowHumidOfTheDay); addr += sizeof(Record);
    
    EEPROM.get(addr, highTempOfTheWeek); addr += sizeof(Record);
    EEPROM.get(addr, lowTempOfTheWeek); addr += sizeof(Record);
    EEPROM.get(addr, highHumidOfTheWeek); addr += sizeof(Record);
    EEPROM.get(addr, lowHumidOfTheWeek); addr += sizeof(Record);

    // Then load rolling buffers
    auto loadBuffer = [&](Record buffer[]) {
        for (int i = 0; i < MAX_ROLLING_RECORDS; i++) {
            uint32_t ts;
            float val;
            uint8_t type;
            EEPROM.get(addr, ts); addr += sizeof(uint32_t);
            EEPROM.get(addr, val); addr += sizeof(float);
            EEPROM.get(addr, type); addr += sizeof(uint8_t);
            buffer[i].dateTime = DateTime(ts);
            buffer[i].value = val;
            buffer[i].sensorType = (SensorType)type;
        }
    };

    loadBuffer(rollingTempHighs);
    loadBuffer(rollingTempLows);
    loadBuffer(rollingHumidHighs);
    loadBuffer(rollingHumidLows);

    Serial.print("Loaded Temp High: ");
    Serial.println(rollingTempHighs[0].value);
    Serial.print("Loaded Temp Low: ");
    Serial.println(rollingTempLows[0].value);

    Serial.println("Records loaded from EEPROM.");
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

