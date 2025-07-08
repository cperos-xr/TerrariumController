#ifndef RECORDMANAGER_H
#define RECORDMANAGER_H

#include <Arduino.h>
#include <RTClib.h>
#include <LittleFS.h>
#include <limits>

#define MAX_ROLLING_RECORDS 20
static const char* RECORD_FILE = "/records.bin";

enum SensorType { TEMPERATURE, HUMIDITY };

struct Record {
  SensorType sensorType;
  float      value;
  DateTime   dateTime;
};

class RecordManager {
public:
    RecordManager();

    // public state
    Record highTempOfTheWeek, lowTempOfTheWeek;
    Record highHumidOfTheWeek, lowHumidOfTheWeek;
    Record highTempOfTheDay,  lowTempOfTheDay;
    Record highHumidOfTheDay, lowHumidOfTheDay;

    // Make these public or provide getter/setter methods
    bool recordsChanged = false;
    unsigned long lastSaveTime = 0;

    void initRecords();
    void analyzeReading(SensorType sensorType, float value, DateTime dateTime);
    void printRecords();

    // Persistence via LittleFS
    void saveRecords();
    void loadRecords();
    void clearRecords();

    // Helper methods for main.ino
    float getHighTempDaily() { return highTempOfTheDay.value; }
    float getLowTempDaily() { return lowTempOfTheDay.value; }
    float getHighHumidDaily() { return highHumidOfTheDay.value; }
    float getLowHumidDaily() { return lowHumidOfTheDay.value; }

    // For Bluetooth (replace with appropriate implementation)
    String getCurrentRecordsAsJSON();

private:
    // ONLY DECLARE THESE ONCE
    Record rollingTempHighs[MAX_ROLLING_RECORDS];
    Record rollingTempLows[MAX_ROLLING_RECORDS];
    Record rollingHumidHighs[MAX_ROLLING_RECORDS];
    Record rollingHumidLows[MAX_ROLLING_RECORDS];

    // Helper methods for rolling functionality
    void addToRollingBuffer(SensorType sensorType, float value, DateTime dateTime);
    void addToBuffer(Record buffer[], const Record& newRecord, int bufferSize);
    void updateRollingDailyRecords(DateTime currentTime);
    void updateRollingWeeklyRecords(DateTime currentTime);
    Record findBestInTimeRange(Record buffer[], DateTime currentTime, int daysBack, bool findHigh, SensorType type);
    void cleanExpiredRecords(Record buffer[], DateTime currentTime, int daysBack);
    void printDateTime(const DateTime& dt);
    
    // Helper methods
    bool isNewDay(const DateTime& current, const DateTime& recorded);
    bool isNewWeek(const DateTime& current, const DateTime& recorded);
    void updateRecord(Record& record, float value, DateTime dateTime, bool isHigh);
    void resetDailyRecords(DateTime currentTime);
    void resetWeeklyRecords(DateTime currentTime);
    Record createInvalidRecord(SensorType type);
    Record createInvalidHighRecord(SensorType type);
    Record createInvalidLowRecord(SensorType type);
};

extern RecordManager rcd; // Declare a global RecordManager instance

#endif
