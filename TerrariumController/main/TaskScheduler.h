/* TaskScheduler.h */
#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H

#include <Arduino.h>
#include <RTClib.h>
#include <LittleFS.h>

// Persistent schedule file
static const char* SCHEDULE_FILE = "/schedules.bin";

// External hook for fogger button simulation
extern void pressFoggerButton();

enum ScheduleType {
    NONE,
    ALWAYS_ON,
    DAILY,
    WEEKLY,
    TWICE_DAILY,
    TWICE_WEEKLY,
    MONTHLY,
    TWICE_MONTHLY
};

String scheduleTypeToString(ScheduleType type);

struct Schedule {
    ScheduleType type;
    int hour1, minute1, duration1;
    int hour2, minute2, duration2;
};

class TaskScheduler {
public:
    TaskScheduler(int lightPin, int waterPin, int foggerPin);

    void parseAndSetSchedule(const String& cmd);
    void updateTasks(const DateTime& now);

    String getSchedulesAsString();
    String getSchedulesAsJSON();

    bool saveSchedules();
    bool loadSchedules();
    bool clearSchedules();

    // New manual toggle methods
    void toggleLight();
    void toggleWater();
    void toggleFogger();

    // Get current device states
    bool getLightState() const { return lightRunning; }
    bool getWaterState() const { return waterRunning; }
    bool getFoggerState() const { return foggerRunning; }

private:
    int lightPin, waterPin, foggerPin;
    Schedule lightSchedule, waterSchedule, foggerSchedule;
    bool lightRunning, waterRunning, foggerRunning;
    unsigned long lightOffMillis, waterOffMillis, foggerOffMillis;

    void applySchedule(const Schedule& sch, int pin, bool& running, unsigned long& offTime, const DateTime& now);
    void executeTask(int pin, int durationMs, bool& running, unsigned long& offTime);
    bool matchSchedule(const DateTime& now, const Schedule& sch, bool& isSecond);

    ScheduleType parseType(const String& s);
    String scheduleToString(const Schedule& sch, const String& type);
};

#endif // TASKSCHEDULER_H
