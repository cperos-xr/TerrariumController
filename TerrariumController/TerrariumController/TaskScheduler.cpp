#include "TaskScheduler.h"
#include <Arduino.h>
#include "ActuatorControl.h" // Include ActuatorControl for pin definitions

// Frequency_Type values:
// ALWAYS_ON: Task runs continuously.
// DAILY: Task runs once every day at the specified time.
// TWICE_DAILY: Task runs twice a day at the specified times.
// WEEKLY: Task runs once a week on a specific day.
// TWICE_WEEKLY: Task runs twice a week on specific days (currently Sunday and Wednesday).
// MONTHLY: Task runs once a month on the 1st day.
// TWICE_MONTHLY: Task runs twice a month on the 1st and 15th days.

TaskScheduler::TaskScheduler() : taskCount(0), rtcDetected(false) {}

void TaskScheduler::setRTCDetected(bool detected) {
    rtcDetected = detected;
}

void TaskScheduler::initializeTasks() {
    if (rtcDetected) {
        // Add tasks only if RTC is detected

        // Example 1: Light Always On
        // The light will remain on continuously.
        addTask({"Light", ALWAYS_ON, 0, 0, 0});

        // Example 2: Pump Always On
        // The water pump will remain on continuously.
        // addTask({"Water", ALWAYS_ON, 0, 0, 0});

        // Example 3: Watering Once a Week
        // addTask({"Water", WEEKLY, Time_Hour1, Time_Minute1, Duration_Seconds});
        // The water pump will turn on every Sunday at 8 AM for 1 minute.
        // addTask({"Water", WEEKLY, 8, 0, 60});

        // Example 4: Light Daily
        // addTask({"Light", DAILY, Time_Hour1, Time_Minute1, Duration_Seconds});
        // The light will turn on daily at 1 PM for 1 hours.
        // addTask({"Light", DAILY, 13, 30, 1 * 60 * 60});  // 1 hour = 3600 seconds

        // Example 5: Water Twice Daily
        // addTask({"Water", TWICE_DAILY, Time_Hour1, Time_Minute1, Duration_Seconds, Time_Hour2, Time_Minute2});
        // The water pump will turn on twice daily at 10 AM and 2:36 PM for 60 seconds.
        addTask({"Water", TWICE_DAILY, 10, 0, 60, 14, 36});

        // Example 6: Water Twice Weekly
        // addTask({"Water", TWICE_WEEKLY, Time_Hour1, Time_Minute1, Duration_Seconds}); (days are currently set in TWICE_WEEKLY, but we may want customize this in the future)
        // The water pump will turn on every Sunday and Wednesday at 7:30 AM for 2 minutes.
        // addTask({"Water", TWICE_WEEKLY, 7, 30, 120});

        // Example 7: Light Monthly
        // addTask({"Light", MONTHLY, Time_Hour1, Time_Minute1, Duration_Seconds}); (days are currently set in MONTHLY, but we may want customize this in the future)
        // The light will turn on on the 1st of every month at 9 PM for 4 hours.
        // addTask({"Light", MONTHLY, 21, 0, 4 * 60 * 60});

        // Example 8: Water Twice Monthly
        // addTask({"Water", TWICE_MONTHLY, Time_Hour1, Time_Minute1, Duration_Seconds}); (days are currently set in TWICE_MONTHLY, but we may want customize this in the future)
        // The water pump will turn on on the 1st and 15th of every month at 6 AM for 1 minute.
        // addTask({"Water", TWICE_MONTHLY, 6, 0, 60});
    }
}

void TaskScheduler::addTask(const Task& task) {
    if (taskCount < 10) {
        tasks[taskCount++] = task;
    } else {
        Serial.println("TaskScheduler: Maximum task limit reached!");
    }
}

void TaskScheduler::updateTasks(DateTime now) {
    if (!rtcDetected) {
        // If RTC is not detected, fall back to ActuatorControl
        updateActuators(); // Use the default pin-based scheduling
        return;
    }

    for (int i = 0; i < taskCount; i++) {
        const Task& task = tasks[i];

        // Check if the task should run
        if (task.frequency == ALWAYS_ON) {
            // ALWAYS_ON: Task runs continuously
            executeTask(task);
        } else if (task.frequency == DAILY) {
            // DAILY: Task runs once every day at the specified start time
            if (now.hour() == task.startHour && now.minute() == task.startMinute) {
                executeTask(task);
            }
        } else if (task.frequency == TWICE_DAILY) {
            // TWICE_DAILY: Task runs twice a day at the specified start times
            if ((now.hour() == task.startHour && now.minute() == task.startMinute) ||
                (now.hour() == task.secondStartHour && now.minute() == task.secondStartMinute)) {
                executeTask(task);
            }
        } else if (task.frequency == WEEKLY) {
            // WEEKLY: Task runs once a week on Sunday at the specified start time
            if (now.dayOfTheWeek() == 0 && now.hour() == task.startHour && now.minute() == task.startMinute) {
                executeTask(task);
            }
        } else if (task.frequency == TWICE_WEEKLY) {
            // TWICE_WEEKLY: Task runs twice a week on Sunday and Wednesday at the specified start time
            if ((now.dayOfTheWeek() == 0 || now.dayOfTheWeek() == 3) &&
                now.hour() == task.startHour && now.minute() == task.startMinute) {
                executeTask(task);
            }
        } else if (task.frequency == MONTHLY) {
            // MONTHLY: Task runs once a month on the 1st day at the specified start time
            if (now.day() == 1 && now.hour() == task.startHour && now.minute() == task.startMinute) {
                executeTask(task);
            }
        } else if (task.frequency == TWICE_MONTHLY) {
            // TWICE_MONTHLY: Task runs twice a month on the 1st and 15th days at the specified start time
            if ((now.day() == 1 || now.day() == 15) &&
                now.hour() == task.startHour && now.minute() == task.startMinute) {
                executeTask(task);
            }
        }
    }
}

void TaskScheduler::executeTask(const Task& task) {
    Serial.print("Executing task: ");
    Serial.println(task.name);

    if (strcmp(task.name, "Light") == 0) {
        if (task.frequency == ALWAYS_ON) {
            // Turn on the light continuously
            digitalWrite(PIN_LIGHT, HIGH);
        } else {
            // Turn on the light for the specified duration
            digitalWrite(PIN_LIGHT, HIGH);
            delay(task.durationSeconds * 1000); // Keep the light on for the specified duration
            digitalWrite(PIN_LIGHT, LOW); // Turn off the light
        }
    } else if (strcmp(task.name, "Water") == 0) {
        if (task.frequency == ALWAYS_ON) {
            // Turn on the water pump continuously
            digitalWrite(PIN_WATER, HIGH);
        } else {
            // Turn on the water pump for the specified duration
            digitalWrite(PIN_WATER, HIGH);
            delay(task.durationSeconds * 1000); // Keep the pump on for the specified duration
            digitalWrite(PIN_WATER, LOW); // Turn off the pump
        }
    }
}

const Task* TaskScheduler::getTaskByName(const char* name) const {
    for (int i = 0; i < taskCount; i++) {
        if (strcmp(tasks[i].name, name) == 0) {
            return &tasks[i];
        }
    }
    return nullptr; // Return nullptr if the task is not found
}