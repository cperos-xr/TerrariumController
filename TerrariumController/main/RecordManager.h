#ifndef RECORDMANAGER_H
#define RECORDMANAGER_H
#include <Arduino.h>
#include <RTClib.h>  // Add this include for DateTime
#include <limits>

#define MAX_ROLLING_RECORDS 20

enum SensorType
{
    TEMPERATURE,
    HUMIDITY
    //PRESSURE,
    //LIGHT
};

struct Record
{
    SensorType sensorType;
    float value;
    DateTime dateTime;
};

class RecordManager 
{
    public:
        RecordManager();
        
        // Weekly records
        Record highTempOfTheWeek;
        Record lowTempOfTheWeek;  // Fixed: was "lowTemp"
        Record highHumidOfTheWeek;
        Record lowHumidOfTheWeek;

        // Daily records
        Record highTempOfTheDay;
        Record lowTempOfTheDay;
        Record highHumidOfTheDay;
        Record lowHumidOfTheDay;

        void initRecords();
        void analyzeReading(SensorType sensorType, float value, DateTime dateTime);  // Fixed: removed extra )
        void clearRecords();
        void printRecords();

        float getHighTempDaily() const;
        float getLowTempDaily() const;
        float getHighHumidDaily() const;
        float getLowHumidDaily() const;
        
        float getHighTempWeekly() const;
        float getLowTempWeekly() const;
        float getHighHumidWeekly() const;
        float getLowHumidWeekly() const;
        bool recordsChanged = false;
        unsigned long lastSaveTime = 0; // Timestamp of the last save
        String getCurrentRecordsAsJSON() const;

    private:
        
        // Rolling buffers for maintaining recent records
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
