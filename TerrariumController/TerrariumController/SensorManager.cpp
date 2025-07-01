// ---- SensorManager.cpp ----
#include "SensorManager.h"
Adafruit_AHTX0 aht;
float tempF = 0, hum = 0;
float dailyHi = -100, dailyLo = 200;
float weeklyHi = -100, weeklyLo = 200;
float dailyHumHi = -1, dailyHumLo = 101;
float weeklyHumHi = -1, weeklyHumLo = 101;
unsigned long lastSensorRead = 0;
unsigned long lastDailyReset = 0; // Timestamp for last daily reset
unsigned long lastWeeklyReset = 0; // Timestamp for last weekly reset

const unsigned long DAILY_RESET_INTERVAL = 86400000; // 24 hours in milliseconds
const unsigned long WEEKLY_RESET_INTERVAL = 604800000; // 7 days in milliseconds

// Function prototypes
void resetDailyStats();
void resetWeeklyStats();
void checkAndResetStats();

void initSensors() {
    if (!aht.begin()) while (1);
    lastDailyReset = millis(); // Initialize daily reset timestamp
    lastWeeklyReset = millis(); // Initialize weekly reset timestamp
}

void readSensors() {
    sensors_event_t h, t;
    aht.getEvent(&h, &t);
    hum = h.relative_humidity;
    tempF = t.temperature * 9.0 / 5.0 + 32.0;

    // Update daily and weekly highs and lows for temperature
    dailyHi = max(dailyHi, tempF);
    dailyLo = min(dailyLo, tempF);
    weeklyHi = max(weeklyHi, tempF);
    weeklyLo = min(weeklyLo, tempF);

    // Update daily and weekly highs and lows for humidity
    dailyHumHi = max(dailyHumHi, hum);
    dailyHumLo = min(dailyHumLo, hum);
    weeklyHumHi = max(weeklyHumHi, hum);
    weeklyHumLo = min(weeklyHumLo, hum);

    // Check and reset stats if needed
    checkAndResetStats();
}

void resetDailyStats() {
    dailyHi = -100;
    dailyLo = 200;
    dailyHumHi = -1;
    dailyHumLo = 101;
}

void resetWeeklyStats() {
    weeklyHi = -100;
    weeklyLo = 200;
    weeklyHumHi = -1;
    weeklyHumLo = 101;
}

void checkAndResetStats() {
    unsigned long now = millis();

    // Reset daily stats every 24 hours
    if (now - lastDailyReset >= DAILY_RESET_INTERVAL) {
        resetDailyStats();
        lastDailyReset = now; // Update the last reset timestamp
    }

    // Reset weekly stats every 7 days
    if (now - lastWeeklyReset >= WEEKLY_RESET_INTERVAL) {
        resetWeeklyStats();
        lastWeeklyReset = now; // Update the last reset timestamp
    }
}