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
#include "TaskScheduler.h"

#define SDA_PIN     5
#define SCL_PIN     6

unsigned long lastFrame = 0;
const unsigned long FRAME_INTERVAL = 30; // ~33 FPS

struct Mode {
  void (*func)();
  unsigned long duration;
};

Mode modesWithClock[] = {
  { drawStatusScreen, 10000 },
  { drawDigitalClock, 10000 },
  { drawVineAnimation, 9000 },
  { drawScrollStats, 10000 },
  { drawBloomAnimation, 3000 },
  { drawStatusScreen, 10000 },
  { drawBloomAnimation, 3000 },
  { drawSensorOverlayVine, 10000 },
  { drawBloomAnimation, 3000 }
};

Mode modesWithoutClock[] = {
  { drawVineAnimation, 9000 },
  { drawScrollStats, 10000 },
  { drawBloomAnimation, 3000 },
  { drawStatusScreen, 10000 },
  { drawBloomAnimation, 3000 },
  { drawSensorOverlayVine, 10000 },
  { drawBloomAnimation, 3000 }
};

Mode* modes; // Pointer to the active mode array
int modeCount; // Number of modes in the active array

const int MODE_COUNT = sizeof(modesWithClock) / sizeof(Mode);
int currentMode = 0;
unsigned long modeStart = 0;

TaskScheduler scheduler;

void setup() {
    Serial.begin(9600); // Start serial communication
    Serial.println("Setup started..."); // Debug message
    while (!Serial); // Wait for serial to be ready
    Wire.begin(SDA_PIN, SCL_PIN);

    initDisplay();
    initSensors();
    initActuators();

    // Use RTCManager to detect and initialize RTC
    if (rtc.isRTCAvailable()) {
        Serial.println("RTC detected at address 0x68.");
        rtc.initRTC(); // Initialize RTC
        Serial.println("RTC initialization finished."); // Debug message
        rtc.printCurrentTime(); // Print current time to serial

        scheduler.setRTCDetected(true); // Notify scheduler that RTC is available
        modes = modesWithClock; // Use modes with digital clock
        modeCount = sizeof(modesWithClock) / sizeof(Mode);
    } else {
        Serial.println("RTC not detected. Skipping digital clock.");
        scheduler.setRTCDetected(false); // Notify scheduler that RTC is not available
        modes = modesWithoutClock; // Use modes without digital clock
        modeCount = sizeof(modesWithoutClock) / sizeof(Mode);
    }

    scheduler.initializeTasks(); // Initialize tasks based on RTC availability

    modeStart = millis();
    Serial.println("Setup complete."); // Debug message
}

void loop() {
    unsigned long now = millis();

    // Update sensors and items periodically
    if (now - lastSensorRead >= SENSOR_INTERVAL) {
        readSensors();    // Update sensor readings
        updateItems();    // Refresh items array with updated stats
        lastSensorRead = now;
    }

    // Update tasks
    if (rtc.isRTCAvailable()) { // Use RTCManager to check RTC availability
        scheduler.updateTasks(rtc.getCurrentTime());
    } else {
        updateActuators(); // Fallback to default actuator control
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
        currentMode = (currentMode + 1) % modeCount;
        modeStart = now;
    }
}