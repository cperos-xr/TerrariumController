#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Adafruit_AHTX0.h>

extern Adafruit_AHTX0 aht;
extern float tempF, hum;
extern float dailyHi, dailyLo, weeklyHi, weeklyLo;
extern float dailyHumHi, dailyHumLo, weeklyHumHi, weeklyHumLo;
extern unsigned long lastSensorRead;

#define SENSOR_INTERVAL 3000
#define TLOW_WARN 32
#define THIGH_WARN 100
#define HLOW_WARN 50
#define HHIGH_WARN 100

void initSensors();
void readSensors();

#endif