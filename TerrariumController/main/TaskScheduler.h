/* TaskScheduler.h */
#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H
#include <Arduino.h>
#include <RTClib.h>
#include <LittleFS.h>

// Add this constant for the schedule file
static const char* SCHEDULE_FILE = "/schedules.bin";

enum ScheduleType { NONE, ALWAYS_ON, DAILY, WEEKLY, TWICE_DAILY, TWICE_WEEKLY, MONTHLY, TWICE_MONTHLY };

struct Schedule {
    ScheduleType type;
    int hour1, minute1, duration1, hour2, minute2, duration2;
};

class TaskScheduler {
public:
    TaskScheduler(int lightPin, int waterPin);
    void parseAndSetSchedule(const String& cmd);
    void updateTasks(const DateTime& now);
    String getSchedulesAsString();
    
    // Add these methods for schedule persistence
    bool saveSchedules();
    bool loadSchedules();
    bool clearSchedules();

private:
    int lightPin, waterPin;
    Schedule lightSchedule, waterSchedule;
    bool lightRunning, waterRunning;
    unsigned long lightOffMillis, waterOffMillis;
    void applySchedule(const Schedule&, int, bool&, unsigned long&, const DateTime&);
    void executeTask(int, int, bool&, unsigned long&);
    bool matchSchedule(const DateTime&, const Schedule&, bool&);
    ScheduleType parseType(const String& s);
    String scheduleToString(const Schedule& sch, const String& type);
};

#endif