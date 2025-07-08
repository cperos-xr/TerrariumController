#ifndef RECORDMANAGER_H
#define RECORDMANAGER_H
#include <Arduino.h>
#include <RTClib.h>  // Add this include for DateTime
#include <limits>
#include <EEPROM.h>

#define MAX_ROLLING_RECORDS 20
#define EEPROM_MAGIC 0x42
#define EEPROM_MAGIC_ADDR 0
#define EEPROM_SIZE (MAX_ROLLING_RECORDS * sizeof(Record) * 4) // 4 buffers

//static const int EEPROM_SIZE            = 1024;    // adjust to fit your data

static const int EEPROM_ADDR_RECORDS    =   0;     // start of record data


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
        Record lowTempOfTheWeek;
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

        void saveRecordsToEEPROM();
        void loadRecordsFromEEPROM();

        float getHighTempDaily() const;
        float getLowTempDaily() const;
        float getHighHumidDaily() const;
        float getLowHumidDaily() const;
        
        float getHighTempWeekly() const;
        float getLowTempWeekly() const;
        float getHighHumidWeekly() const;
        float getLowHumidWeekly() const;

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
