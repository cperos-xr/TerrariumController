// Terrarium Controller V1 - Modular Scheduler
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_AHTX0.h>

#include "SensorManager.h"
#include "DisplayManager.h"
#include "ActuatorControl.h"
#include "Animations.h"

#define SDA_PIN     5
#define SCL_PIN     6

unsigned long lastFrame = 0;
const unsigned long FRAME_INTERVAL = 30; // ~33 FPS

struct Mode {
  void (*func)();
  unsigned long duration;
};

Mode modes[] = {
  { vineAnimation, 9000 },
  { drawScrollStats, 10000 },
  { bloomAnimation, 3000 },
  { drawStatusScreen, 10000 },
  { bloomAnimation, 3000 },
  { drawSensorOverlayVine, 10000 },
};

const int MODE_COUNT = sizeof(modes) / sizeof(Mode);
int currentMode = 0;
unsigned long modeStart = 0;

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  initDisplay();
  initSensors();
  initActuators();
  modeStart = millis();
}

void loop() {
  unsigned long now = millis();

  // Update actuators
  updateActuators();

  // Update sensors and items periodically
  if (now - lastSensorRead >= SENSOR_INTERVAL) {
    readSensors();    // Update sensor readings
    updateItems();    // Refresh items array with updated stats
    lastSensorRead = now;
  }

  // Handle display modes
  if (now - lastFrame >= FRAME_INTERVAL) {
    lastFrame = now;
    display.clearDisplay();
    modes[currentMode].func(); // Call the current mode's function
    display.display();
  }

  // Switch modes based on duration
  if (now - modeStart >= modes[currentMode].duration) {
    currentMode = (currentMode + 1) % MODE_COUNT;
    modeStart = now;
  }
}