// ---- SensorManager.cpp ----
#include "SensorManager.h"
Adafruit_AHTX0 aht;
float tempF = 0, hum = 0;
float dailyHi = -100, dailyLo = 200;
float weeklyHi = -100, weeklyLo = 200;
float dailyHumHi = -1, dailyHumLo = 101;
float weeklyHumHi = -1, weeklyHumLo = 101;
unsigned long lastSensorRead = 0;
void initSensors() {
  if (!aht.begin()) while (1);
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
}