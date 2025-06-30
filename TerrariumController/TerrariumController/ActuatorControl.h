#ifndef ACTUATOR_CONTROL_H
#define ACTUATOR_CONTROL_H

void initActuators();
void updateActuators();

// Declare variables as extern for cross-file access
extern unsigned long lastLightTime, lastWaterTime;
extern const unsigned long LIGHT_DURATION, WATER_DURATION;
extern const unsigned long LIGHT_INTERVAL, WATER_INTERVAL;

#endif