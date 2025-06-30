// Terrarium Controller V1 - Modular Scheduler
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_AHTX0.h>

#include "SensorManager.h"
#include "DisplayManager.h"
#include "ActuatorControl.h"
#include "Animations.h"
#include "RTCManager.h"
#include "DigitalClock.h" // Include the DigitalClock module

#define SDA_PIN     5
#define SCL_PIN     6

unsigned long lastFrame = 0;
const unsigned long FRAME_INTERVAL = 30; // ~33 FPS

struct Mode {
  void (*func)();
  unsigned long duration;
};

Mode modes[] = {
  { drawDigitalClock, 10000 },
  { drawVineAnimation, 9000 },
  { drawScrollStats, 10000 },
  { drawBloomAnimation, 3000 },
  { drawStatusScreen, 10000 },
  { drawBloomAnimation, 3000 },
  { drawSensorOverlayVine, 10000 },
  { drawBloomAnimation, 3000 }
};

const int MODE_COUNT = sizeof(modes) / sizeof(Mode);
int currentMode = 0;
unsigned long modeStart = 0;

void setup() {
    Serial.begin(9600); // Start serial communication
    Serial.println("Setup started..."); // Debug message

    Wire.begin(SDA_PIN, SCL_PIN);

    initDisplay();
    initSensors();
    initActuators();

    Serial.println("Calling RTC initialization..."); // Debug message
    rtc.initRTC(); // Initialize RTC
    Serial.println("RTC initialization finished."); // Debug message
    rtc.printCurrentTime(); // Print current time to serial

    modeStart = millis();
    Serial.println("Setup complete."); // Debug message
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