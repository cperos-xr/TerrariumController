#pragma once

#include <Arduino.h>
#include "ActuatorControl.h" // Include ActuatorControl for pin definitions
#include <RTClib.h> // Include RTC library for DateTime

// Define the TaskFrequency enum
enum TaskFrequency {
    ALWAYS_ON,
    DAILY,
    TWICE_DAILY,
    WEEKLY,
    TWICE_WEEKLY,
    MONTHLY,
    TWICE_MONTHLY,
    INVALID // For error handling
};

// Define the Task structure
struct Task {
    const char* name;
    TaskFrequency frequency;
    uint8_t startHour;
    uint8_t startMinute;
    uint16_t durationSeconds;
    uint8_t secondStartHour; // For TWICE_DAILY frequency
    uint8_t secondStartMinute; // For TWICE_DAILY frequency
};

// Define the TaskScheduler class
class TaskScheduler {
public:
    TaskScheduler();
    void setRTCDetected(bool detected);
    void initializeTasks();
    void addTask(const Task& task);
    void updateTasks(DateTime now);
    const Task* getTaskByName(const char* name) const;

private:
    Task tasks[10]; // Array to store tasks
    int taskCount; // Number of tasks added
    bool rtcDetected; // Flag to indicate if RTC is detected
    void executeTask(const Task& task);
    void updateActuators(); // Fallback logic for actuators
};