#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

#include <RTClib.h> // Include Adafruit RTClib for RTC functionality

enum TaskFrequency {
    ALWAYS_ON,
    DAILY,
    TWICE_DAILY,
    WEEKLY,
    TWICE_WEEKLY,
    MONTHLY,
    TWICE_MONTHLY
};



struct Task {
    const char* name;
    TaskFrequency frequency;
    uint8_t startHour; // Start time (hour)
    uint8_t startMinute; // Start time (minute)
    uint16_t durationSeconds; // Duration in seconds
    uint8_t secondStartHour; // Optional second start time (hour)
    uint8_t secondStartMinute; // Optional second start time (minute)
};

class TaskScheduler {
public:
    TaskScheduler();
    void initializeTasks(); // Initialize tasks
    void updateTasks(DateTime now); // Call this periodically to check and execute tasks
    void executeTask(const Task& task); // Execute a specific task
    void setRTCDetected(bool detected); // Set RTC detection status
    void addTask(const Task& task); // Add a new task to the scheduler
    const Task* getTaskByName(const char* name) const;

private:
    Task tasks[10]; // Array to store up to 10 tasks
    int taskCount; // Number of tasks added
    bool rtcDetected; // Flag to indicate if RTC is detected
};

#endif // TASK_SCHEDULER_H