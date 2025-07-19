#include "RecordManager.h"
#include "RTCManager.h"    // if you use it elsewhere
#include <Arduino.h>

// Constructor — don’t mount FS here; do it in setup()
RecordManager::RecordManager() : recordsChanged(false), lastSaveTime(0) {
  // nothing
}

void RecordManager::initRecords() {
    // Try to load records first
    bool hasValidData = false;
    
    if (LittleFS.exists(RECORD_FILE)) {
        loadRecords();
        
        // Check for more specific validity - low temp should NEVER be zero in a terrarium
        hasValidData = (lowTempOfTheDay.value > 0.1f && lowHumidOfTheDay.value > 0.1f);
        
        if (hasValidData) {
            Serial.println("Successfully loaded records from LittleFS");
            
            // Debug output
            Serial.print("Loaded lowTempOfTheDay: ");
            Serial.println(lowTempOfTheDay.value);
            Serial.print("Loaded lowHumidOfTheDay: ");
            Serial.println(lowHumidOfTheDay.value);
            return; // Records loaded successfully, no need to initialize
        } else {
            Serial.println("❌ Invalid or zero records found. Re-initializing...");
        }
    }
    
    // If we get here, either no file exists or loading failed
    Serial.println("Initializing with default values");
    
    // Initialize with default values
    highTempOfTheWeek = createInvalidHighRecord(TEMPERATURE);
    lowTempOfTheWeek = createInvalidLowRecord(TEMPERATURE);
    highTempOfTheDay = createInvalidHighRecord(TEMPERATURE);
    lowTempOfTheDay = createInvalidLowRecord(TEMPERATURE);
    
    highHumidOfTheWeek = createInvalidHighRecord(HUMIDITY);
    lowHumidOfTheWeek = createInvalidLowRecord(HUMIDITY);
    highHumidOfTheDay = createInvalidHighRecord(HUMIDITY);
    lowHumidOfTheDay = createInvalidLowRecord(HUMIDITY);
    
    // Initialize rolling buffers
    for (int i = 0; i < MAX_ROLLING_RECORDS; i++) {
        rollingTempHighs[i] = createInvalidHighRecord(TEMPERATURE);
        rollingTempLows[i] = createInvalidLowRecord(TEMPERATURE);
        rollingHumidHighs[i] = createInvalidHighRecord(HUMIDITY);
        rollingHumidLows[i] = createInvalidLowRecord(HUMIDITY);
    }
    
    // Verify records were initialized properly
    Serial.print("Initialized lowTempOfTheDay: ");
    Serial.println(lowTempOfTheDay.value);
    Serial.print("Initialized lowHumidOfTheDay: ");
    Serial.println(lowHumidOfTheDay.value);
    
    // Save the initialized records
    saveRecords();
    
    // Verify records after save
    loadRecords();
    Serial.print("After save/load lowTempOfTheDay: ");
    Serial.println(lowTempOfTheDay.value);
    
    Serial.println("Records initialized with default values");
}

// === SAVE ===
void RecordManager::saveRecords() {
  File f = LittleFS.open(RECORD_FILE, "w");
  if (!f) {
    Serial.println("❌ Failed to open record file for write");
    return;
  }

  // 1) summary records
  f.write((uint8_t*)&highTempOfTheDay,  sizeof(Record));
  f.write((uint8_t*)&lowTempOfTheDay,   sizeof(Record));
  f.write((uint8_t*)&highHumidOfTheDay, sizeof(Record));
  f.write((uint8_t*)&lowHumidOfTheDay,  sizeof(Record));

  f.write((uint8_t*)&highTempOfTheWeek,  sizeof(Record));
  f.write((uint8_t*)&lowTempOfTheWeek,   sizeof(Record));
  f.write((uint8_t*)&highHumidOfTheWeek, sizeof(Record));
  f.write((uint8_t*)&lowHumidOfTheWeek,  sizeof(Record));

  // 2) rolling buffers
  for (int i = 0; i < MAX_ROLLING_RECORDS; i++) {
    f.write((uint8_t*)&rollingTempHighs[i], sizeof(Record));
  }
  for (int i = 0; i < MAX_ROLLING_RECORDS; i++) {
    f.write((uint8_t*)&rollingTempLows[i], sizeof(Record));
  }
  for (int i = 0; i < MAX_ROLLING_RECORDS; i++) {
    f.write((uint8_t*)&rollingHumidHighs[i], sizeof(Record));
  }
  for (int i = 0; i < MAX_ROLLING_RECORDS; i++) {
    f.write((uint8_t*)&rollingHumidLows[i], sizeof(Record));
  }

  f.close();
  recordsChanged = false;
  lastSaveTime = millis();
  Serial.println("✅ Records saved to LittleFS");
}

// === LOAD ===
void RecordManager::loadRecords() {
  if (!LittleFS.exists(RECORD_FILE)) {
    Serial.println("ℹ️  No record file found, starting fresh");
    return;
  }

  File f = LittleFS.open(RECORD_FILE, "r");
  if (!f) {
    Serial.println("❌ Failed to open record file for read");
    return;
  }

  // 1) summary records
  f.read((uint8_t*)&highTempOfTheDay,  sizeof(Record));
  f.read((uint8_t*)&lowTempOfTheDay,   sizeof(Record));
  f.read((uint8_t*)&highHumidOfTheDay, sizeof(Record));
  f.read((uint8_t*)&lowHumidOfTheDay,  sizeof(Record));

  f.read((uint8_t*)&highTempOfTheWeek,  sizeof(Record));
  f.read((uint8_t*)&lowTempOfTheWeek,   sizeof(Record));
  f.read((uint8_t*)&highHumidOfTheWeek, sizeof(Record));
  f.read((uint8_t*)&lowHumidOfTheWeek,  sizeof(Record));

  // 2) rolling buffers
  for (int i = 0; i < MAX_ROLLING_RECORDS; i++) {
    f.read((uint8_t*)&rollingTempHighs[i], sizeof(Record));
  }
  for (int i = 0; i < MAX_ROLLING_RECORDS; i++) {
    f.read((uint8_t*)&rollingTempLows[i], sizeof(Record));
  }
  for (int i = 0; i < MAX_ROLLING_RECORDS; i++) {
    f.read((uint8_t*)&rollingHumidHighs[i], sizeof(Record));
  }
  for (int i = 0; i < MAX_ROLLING_RECORDS; i++) {
    f.read((uint8_t*)&rollingHumidLows[i], sizeof(Record));
  }

  f.close();
  
  // CRITICAL: Validate low values after loading - never allow zeros
  if (lowTempOfTheDay.value <= 0.1f) {
    Serial.println("⚠️ FIXING CORRUPTED LOW TEMP VALUE");
    lowTempOfTheDay = createInvalidLowRecord(TEMPERATURE);
  }
  
  if (lowHumidOfTheDay.value <= 0.1f) {
    Serial.println("⚠️ FIXING CORRUPTED LOW HUMID VALUE");
    lowHumidOfTheDay = createInvalidLowRecord(HUMIDITY);
  }
  
  if (lowTempOfTheWeek.value <= 0.1f) {
    lowTempOfTheWeek = createInvalidLowRecord(TEMPERATURE);
  }
  
  if (lowHumidOfTheWeek.value <= 0.1f) {
    lowHumidOfTheWeek = createInvalidLowRecord(HUMIDITY);
  }

  Serial.println("✅ Records loaded from LittleFS");
}

// Add this method to your RecordManager.cpp file:

String RecordManager::getCurrentRecordsAsJSON() {
    String json = "{";
    
    // Add daily records
    json += "\"daily\":{";
    json += "\"highTemp\":" + String(highTempOfTheDay.value) + ",";
    json += "\"lowTemp\":" + String(lowTempOfTheDay.value) + ",";
    json += "\"highHumid\":" + String(highHumidOfTheDay.value) + ",";
    json += "\"lowHumid\":" + String(lowHumidOfTheDay.value);
    json += "},";
    
    // Add weekly records
    json += "\"weekly\":{";
    json += "\"highTemp\":" + String(highTempOfTheWeek.value) + ",";
    json += "\"lowTemp\":" + String(lowTempOfTheWeek.value) + ",";
    json += "\"highHumid\":" + String(highHumidOfTheWeek.value) + ",";
    json += "\"lowHumid\":" + String(lowHumidOfTheWeek.value);
    json += "}";
    
    json += "}";
    return json;
}

// Add these implementations at the end of your file:

Record RecordManager::createInvalidHighRecord(SensorType type) {
    Record record;
    record.sensorType = type;
    record.value = -999.0f; // An extremely low value that will be replaced by any actual reading
    record.dateTime = DateTime(2000, 1, 1, 0, 0, 0); // Old date
    return record;
}

Record RecordManager::createInvalidLowRecord(SensorType type) {
    Record record;
    record.sensorType = type;
    record.value = 999.0f; // An extremely high value that will be replaced by any actual reading
    record.dateTime = DateTime(2000, 1, 1, 0, 0, 0); // Old date
    return record;
}

void RecordManager::analyzeReading(SensorType sensorType, float value, DateTime dateTime) {
    // Record changed flag
    bool changed = false;
    
    // Ignore invalid readings
    if (value <= 0.01f) {
        Serial.println("WARNING: Ignoring near-zero reading that would corrupt low records");
        return;
    }
    
    // Handle temperature readings
    if (sensorType == TEMPERATURE) {
        // Check for daily high/low
        if (value > highTempOfTheDay.value) {
            highTempOfTheDay.value = value;
            highTempOfTheDay.dateTime = dateTime;
            changed = true;
        }
        
        // Special handling for low - NEVER allow 0.00 as a valid low temperature
        if ((value < lowTempOfTheDay.value) && (value > 0.1f)) {
            Serial.print("New low temp record: ");
            Serial.println(value);
            lowTempOfTheDay.value = value;
            lowTempOfTheDay.dateTime = dateTime;
            changed = true;
        }
        
        // Check for weekly high/low - again protecting from 0.00 values
        if (value > highTempOfTheWeek.value) {
            highTempOfTheWeek.value = value;
            highTempOfTheWeek.dateTime = dateTime;
            changed = true;
        }
        if ((value < lowTempOfTheWeek.value) && (value > 0.1f)) {
            lowTempOfTheWeek.value = value;
            lowTempOfTheWeek.dateTime = dateTime;
            changed = true;
        }
        
        // Add to rolling buffers
        addToRollingBuffer(sensorType, value, dateTime);
    }
    // Handle humidity readings
    else if (sensorType == HUMIDITY) {
        // Check for daily high/low
        if (value > highHumidOfTheDay.value) {
            highHumidOfTheDay.value = value;
            highHumidOfTheDay.dateTime = dateTime;
            changed = true;
        }
        
        // Special handling for low - NEVER allow 0.00 as a valid low humidity
        if ((value < lowHumidOfTheDay.value) && (value > 0.1f)) {
            Serial.print("New low humid record: ");
            Serial.println(value);
            lowHumidOfTheDay.value = value;
            lowHumidOfTheDay.dateTime = dateTime;
            changed = true;
        }
        
        // Check for weekly high/low
        if (value > highHumidOfTheWeek.value) {
            highHumidOfTheWeek.value = value;
            highHumidOfTheWeek.dateTime = dateTime;
            changed = true;
        }
        if ((value < lowHumidOfTheWeek.value) && (value > 0.1f)) {
            lowHumidOfTheWeek.value = value;
            lowHumidOfTheWeek.dateTime = dateTime;
            changed = true;
        }
        
        // Add to rolling buffers
        addToRollingBuffer(sensorType, value, dateTime);
    }
    
    // Set changed flag if needed
    if (changed) {
        recordsChanged = true;
    }
}

void RecordManager::addToRollingBuffer(SensorType sensorType, float value, DateTime dateTime) {
    Record newRecord;
    newRecord.sensorType = sensorType;
    newRecord.value = value;
    newRecord.dateTime = dateTime;
    
    if (sensorType == TEMPERATURE) {
        // For temperature readings
        if (value > highTempOfTheDay.value) {
            // This is a new high, add it to the highs buffer
            addToBuffer(rollingTempHighs, newRecord, MAX_ROLLING_RECORDS);
        }
        if (value < lowTempOfTheDay.value || lowTempOfTheDay.value > 900) {
            // This is a new low, add it to the lows buffer
            addToBuffer(rollingTempLows, newRecord, MAX_ROLLING_RECORDS);
        }
    } else if (sensorType == HUMIDITY) {
        // For humidity readings
        if (value > highHumidOfTheDay.value) {
            // This is a new high, add it to the highs buffer
            addToBuffer(rollingHumidHighs, newRecord, MAX_ROLLING_RECORDS);
        }
        if (value < lowHumidOfTheDay.value || lowHumidOfTheDay.value > 900) {
            // This is a new low, add it to the lows buffer
            addToBuffer(rollingHumidLows, newRecord, MAX_ROLLING_RECORDS);
        }
    }
}

void RecordManager::addToBuffer(Record buffer[], const Record& newRecord, int bufferSize) {
    // Shift all records one position
    for (int i = bufferSize - 1; i > 0; i--) {
        buffer[i] = buffer[i-1];
    }
    
    // Add the new record at the beginning
    buffer[0] = newRecord;
}

void RecordManager::printRecords() {
    Serial.println("===== CURRENT RECORDS =====");
    
    // Print daily records
    Serial.println("--- Daily Records ---");
    Serial.print("High Temp: ");
    Serial.print(highTempOfTheDay.value);
    Serial.print(" °C on ");
    printDateTime(highTempOfTheDay.dateTime);
    Serial.println();
    
    Serial.print("Low Temp: ");
    Serial.print(lowTempOfTheDay.value);
    Serial.print(" °C on ");
    printDateTime(lowTempOfTheDay.dateTime);
    Serial.println();
    
    Serial.print("High Humidity: ");
    Serial.print(highHumidOfTheDay.value);
    Serial.print(" % on ");
    printDateTime(highHumidOfTheDay.dateTime);
    Serial.println();
    
    Serial.print("Low Humidity: ");
    Serial.print(lowHumidOfTheDay.value);
    Serial.print(" % on ");
    printDateTime(lowHumidOfTheDay.dateTime);
    Serial.println();
    
    // Print weekly records
    Serial.println("--- Weekly Records ---");
    Serial.print("High Temp: ");
    Serial.print(highTempOfTheWeek.value);
    Serial.print(" °C on ");
    printDateTime(highTempOfTheWeek.dateTime);
    Serial.println();
    
    Serial.print("Low Temp: ");
    Serial.print(lowTempOfTheWeek.value);
    Serial.print(" °C on ");
    printDateTime(lowTempOfTheWeek.dateTime);
    Serial.println();
    
    Serial.print("High Humidity: ");
    Serial.print(highHumidOfTheWeek.value);
    Serial.print(" % on ");
    printDateTime(highHumidOfTheWeek.dateTime);
    Serial.println();
    
    Serial.print("Low Humidity: ");
    Serial.print(lowHumidOfTheWeek.value);
    Serial.print(" % on ");
    printDateTime(lowHumidOfTheWeek.dateTime);
    Serial.println();
}

void RecordManager::printDateTime(const DateTime& dt) {
    Serial.print(dt.year(), DEC);
    Serial.print('/');
    Serial.print(dt.month(), DEC);
    Serial.print('/');
    Serial.print(dt.day(), DEC);
    Serial.print(' ');
    Serial.print(dt.hour(), DEC);
    Serial.print(':');
    Serial.print(dt.minute(), DEC);
    Serial.print(':');
    Serial.print(dt.second(), DEC);
}
