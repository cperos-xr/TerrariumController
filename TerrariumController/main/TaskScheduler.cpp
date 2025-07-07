/* TaskScheduler.cpp */
#include "TaskScheduler.h"

// Constructor: initialize pins and state
TaskScheduler::TaskScheduler(int lp, int wp)
  : lightPin(lp)
  , waterPin(wp)
  , lightRunning(false)
  , waterRunning(false)
  , lightOffMillis(0)
  , waterOffMillis(0)
{
}

// Map a string to the corresponding ScheduleType enum
ScheduleType TaskScheduler::parseType(const String& s) {
  if (s == "ALWAYS_ON")      return ALWAYS_ON;
  if (s == "DAILY")          return DAILY;
  if (s == "WEEKLY")         return WEEKLY;
  if (s == "TWICE_DAILY")    return TWICE_DAILY;
  if (s == "TWICE_WEEKLY")   return TWICE_WEEKLY;
  if (s == "MONTHLY")        return MONTHLY;
  if (s == "TWICE_MONTHLY")  return TWICE_MONTHLY;
  return NONE;
}

// Parse a comma-separated command and update the appropriate schedule
void TaskScheduler::parseAndSetSchedule(const String& cmd) {
  String parts[8];
  int count = 0;
  int start = 0;

  // Split by comma
  for (int i = 0; i <= cmd.length() && count < 8; ++i) {
    if (i == cmd.length() || cmd[i] == ',') {
      parts[count++] = cmd.substring(start, i);
      start = i + 1;
    }
  }

  if (count < 5) {
    Serial.println("Invalid command format");
    return;
  }

  Schedule sch;
  sch.type      = parseType(parts[1]);
  sch.hour1     = parts[2].toInt();
  sch.minute1   = parts[3].toInt();
  sch.duration1 = parts[4].toInt() * 1000;  // seconds → milliseconds

  // Handle second time for TWICE_* schedules
  if ((sch.type == TWICE_DAILY || sch.type == TWICE_WEEKLY || sch.type == TWICE_MONTHLY) && count >= 7) {
    sch.hour2     = parts[5].toInt();
    sch.minute2   = parts[6].toInt();
    sch.duration2 = sch.duration1;
  }

  // Assign to LIGHT or WATER schedule
  if (parts[0] == "LIGHT") {
    lightSchedule = sch;
    Serial.println("Light schedule updated.");
  }
  else if (parts[0] == "WATER") {
    waterSchedule = sch;
    Serial.println("Water schedule updated.");
  }
  else {
    Serial.println("Unknown target. Use LIGHT or WATER.");
  }
}

// Called every loop to check and trigger tasks
void TaskScheduler::updateTasks(const DateTime& now) {
  applySchedule(lightSchedule, lightPin, lightRunning, lightOffMillis, now);
  applySchedule(waterSchedule, waterPin, waterRunning, waterOffMillis, now);
}

// Core logic to turn pins on/off based on schedule and elapsed time
void TaskScheduler::applySchedule(
  const Schedule& sch,
  int pin,
  bool& running,
  unsigned long& offTime,
  const DateTime& now
) {
  if (sch.type == NONE) return;

  // ALWAYS_ON: keep the pin HIGH
  if (sch.type == ALWAYS_ON) {
    digitalWrite(pin, HIGH);
    return;
  }

  // If currently running and time elapsed, turn off
  if (running && millis() >= offTime) {
    digitalWrite(pin, LOW);
    running = false;
  }

  bool secondInstance = false;
  if (!running && matchSchedule(now, sch, secondInstance)) {
    int durationMs = secondInstance ? sch.duration2 : sch.duration1;
    executeTask(pin, durationMs, running, offTime);
  }
}

String TaskScheduler::getSchedulesAsString() {
    String result = "LIGHT:";
    
    // Format light schedule
    result += scheduleToString(lightSchedule, "LIGHT");
    result += ";WATER:";
    
    // Format water schedule  
    result += scheduleToString(waterSchedule, "WATER");
    
    return result;
}

// Helper method to convert schedule to string
String TaskScheduler::scheduleToString(const Schedule& sch, const String& type) {
    if (sch.type == NONE) {
        return "NONE";
    }
    
    String typeStr;
    switch (sch.type) {
        case ALWAYS_ON: typeStr = "ALWAYS_ON"; break;
        case DAILY: typeStr = "DAILY"; break;
        case WEEKLY: typeStr = "WEEKLY"; break;
        case TWICE_DAILY: typeStr = "TWICE_DAILY"; break;
        case TWICE_WEEKLY: typeStr = "TWICE_WEEKLY"; break;
        case MONTHLY: typeStr = "MONTHLY"; break;
        case TWICE_MONTHLY: typeStr = "TWICE_MONTHLY"; break;
        default: typeStr = "UNKNOWN"; break;
    }
    
    String result = typeStr + "," + String(sch.hour1) + ":" + String(sch.minute1) + 
                   "," + String(sch.duration1/1000) + "s";
    
    // Add second time for TWICE_* schedules
    if (sch.type == TWICE_DAILY || sch.type == TWICE_WEEKLY || sch.type == TWICE_MONTHLY) {
        result += "," + String(sch.hour2) + ":" + String(sch.minute2);
    }
    
    return result;
}

// Engage the pin for the specified duration
void TaskScheduler::executeTask(
  int pin,
  int durationMs,
  bool& running,
  unsigned long& offTime
) {
  digitalWrite(pin, HIGH);
  running = true;
  offTime = millis() + durationMs;
}

// Check if current DateTime matches the schedule trigger
bool TaskScheduler::matchSchedule(
  const DateTime& now,
  const Schedule& sch,
  bool& isSecond
) {
  switch (sch.type) {
    case DAILY:
      if (now.hour() == sch.hour1 && now.minute() == sch.minute1 && now.second() == 0) {
        isSecond = false;
        return true;
      }
      break;

    case TWICE_DAILY:
      if (now.second() == 0) {
        if (now.hour() == sch.hour1 && now.minute() == sch.minute1) {
          isSecond = false;
          return true;
        }
        if (now.hour() == sch.hour2 && now.minute() == sch.minute2) {
          isSecond = true;
          return true;
        }
      }
      break;

    case TWICE_WEEKLY:
      if (now.second() == 0) {
        if (now.dayOfTheWeek() == 0 && now.hour() == sch.hour1
            && now.minute() == sch.minute1) {
          isSecond = false;
          return true;
        }
        if (now.dayOfTheWeek() == 3 && now.hour() == sch.hour1
            && now.minute() == sch.minute1) {
          isSecond = true;
          return true;
        }
      }
      break;

    case WEEKLY:
      if (now.dayOfTheWeek() == 0 && now.hour() == sch.hour1
          && now.minute() == sch.minute1 && now.second() == 0) {
        isSecond = false;
        return true;
      }
      break;

    case TWICE_MONTHLY:
      if (now.second() == 0) {
        if (now.day() == 1 && now.hour() == sch.hour1
            && now.minute() == sch.minute1) {
          isSecond = false;
          return true;
        }
        if (now.day() == 15 && now.hour() == sch.hour1
            && now.minute() == sch.minute1) {
          isSecond = true;
          return true;
        }
      }
      break;

    case MONTHLY:
      if (now.day() == 1 && now.hour() == sch.hour1
          && now.minute() == sch.minute1 && now.second() == 0) {
        isSecond = false;
        return true;
      }
      break;

    default:
      break;
  }
  return false;
}
