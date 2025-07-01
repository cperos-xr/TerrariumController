// ---- ActuatorControl.cpp ----
#include <Arduino.h>
#include "ActuatorControl.h"

const int PIN_LIGHT = 4; // Define light pin
const int PIN_WATER = 3; // Define water pin

unsigned long lastLightTime = 0, lastWaterTime = 0;
bool lightOn = false, waterOn = false;
const unsigned long LIGHT_DURATION = 3000; // 3 seconds
const unsigned long WATER_DURATION = 3000; // 3 seconds
const unsigned long LIGHT_INTERVAL = 4000; // 4 seconds
const unsigned long WATER_INTERVAL = 4000; // 4 seconds

void initActuators() {
    pinMode(PIN_LIGHT, OUTPUT);
    pinMode(PIN_WATER, OUTPUT);
    digitalWrite(PIN_LIGHT, LOW);
    digitalWrite(PIN_WATER, LOW);
}

void updateActuators() {
    unsigned long now = millis();
    if (!lightOn && now - lastLightTime >= LIGHT_INTERVAL) {
        lightOn = true; lastLightTime = now;
        digitalWrite(PIN_LIGHT, HIGH);
    }
    if (lightOn && now - lastLightTime >= LIGHT_DURATION) {
        lightOn = false;
        digitalWrite(PIN_LIGHT, LOW);
    }
    if (!waterOn && now - lastWaterTime >= WATER_INTERVAL) {
        waterOn = true; lastWaterTime = now;
        digitalWrite(PIN_WATER, HIGH);
    }
    if (waterOn && now - lastWaterTime >= WATER_DURATION) {
        waterOn = false;
        digitalWrite(PIN_WATER, LOW);
    }
}
