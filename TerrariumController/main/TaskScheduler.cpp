/* TaskScheduler.cpp */
#include "TaskScheduler.h"

// Constructor: initialize pins and state
TaskScheduler::TaskScheduler(int lp, int wp, int fp)
  : lightPin(lp)
  , waterPin(wp)
  , foggerPin(fp)
  , lightRunning(false)
  , waterRunning(false)
  , foggerRunning(false)
  , lightOffMillis(0)
  , waterOffMillis(0)
  , foggerOffMillis(0)
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

    bool wasLightAlwaysOn = (lightSchedule.type == ALWAYS_ON);
    bool wasWaterAlwaysOn = (waterSchedule.type == ALWAYS_ON);
    bool wasFoggerAlwaysOn = (foggerSchedule.type == ALWAYS_ON);

    // Assign to LIGHT, WATER, or FOGGER schedule
    if (parts[0] == "LIGHT") {
        lightSchedule = sch;
        Serial.println("Light schedule updated.");
        
        // Handle immediate state change if needed
        if (wasLightAlwaysOn && sch.type != ALWAYS_ON) {
            digitalWrite(lightPin, LOW);
            lightRunning = false;
            Serial.println("Light pin turned OFF due to schedule change");
        }
        else if (!wasLightAlwaysOn && sch.type == ALWAYS_ON) {
            digitalWrite(lightPin, HIGH);
            Serial.println("Light pin turned ON due to ALWAYS_ON schedule");
        }
        else if (sch.type == NONE) {
            digitalWrite(lightPin, LOW);
            lightRunning = false;
            Serial.println("Light pin turned OFF due to NONE schedule");
        }
    }
    else if (parts[0] == "WATER") {
        waterSchedule = sch;
        Serial.println("Water schedule updated.");
        
        // Handle immediate state change if needed
        if (wasWaterAlwaysOn && sch.type != ALWAYS_ON) {
            digitalWrite(waterPin, LOW);
            waterRunning = false;
            Serial.println("Water pin turned OFF due to schedule change");
        }
        else if (!wasWaterAlwaysOn && sch.type == ALWAYS_ON) {
            digitalWrite(waterPin, HIGH);
            Serial.println("Water pin turned ON due to ALWAYS_ON schedule");
        }
        else if (sch.type == NONE) {
            digitalWrite(waterPin, LOW);
            waterRunning = false;
            Serial.println("Water pin turned OFF due to NONE schedule");
        }
    }
    else if (parts[0] == "FOGGER") {
        foggerSchedule = sch;
        Serial.println("Fogger schedule updated.");
        
        // Handle immediate state change if needed (via simulated button press)
        if (wasFoggerAlwaysOn && sch.type != ALWAYS_ON) {
            // If we're turning off an ALWAYS_ON fogger, simulate a button press
            pressFoggerButton();
            foggerRunning = false;
            Serial.println("Fogger button pressed to turn OFF due to schedule change");
        }
        else if (!wasFoggerAlwaysOn && sch.type == ALWAYS_ON) {
            // If we're turning the fogger to ALWAYS_ON, simulate a button press
            pressFoggerButton();
            Serial.println("Fogger button pressed to turn ON due to ALWAYS_ON schedule");
        }
        else if (sch.type == NONE && foggerRunning) {
            // If fogger is running and we're setting to NONE, simulate a button press
            pressFoggerButton();
            foggerRunning = false;
            Serial.println("Fogger button pressed to turn OFF due to NONE schedule");
        }
    }
    else {
        Serial.println("Unknown target. Use LIGHT, WATER, or FOGGER.");
        return;
    }
    
    // Save schedules after update
    saveSchedules();
}

// Called every loop to check and trigger tasks
void TaskScheduler::updateTasks(const DateTime& now) {
  applySchedule(lightSchedule, lightPin, lightRunning, lightOffMillis, now);
  applySchedule(waterSchedule, waterPin, waterRunning, waterOffMillis, now);
  applySchedule(foggerSchedule, foggerPin, foggerRunning, foggerOffMillis, now);
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

  // ALWAYS_ON: special handling for fogger vs regular pins
  if (sch.type == ALWAYS_ON) {
    if (pin == foggerPin) {
      // For fogger with ALWAYS_ON, we should press the button every 4 hours
      // plus a 5-minute cooldown between cycles
      if (!running) {
        // If not running at all, start it
        pressFoggerButton(); // Simulate button press to turn on
        running = true;
        // Set next cycle time: 4 hours (fogger's internal timer) + 5 min cooldown
        offTime = millis() + 14400000 + 300000; // 4h + 5min in milliseconds
        Serial.println("Fogger button pressed for ALWAYS_ON schedule");
      } 
      else if (millis() > offTime) {
        // Time for next cycle after cooldown
        pressFoggerButton(); // Simulate button press to turn on again
        // Reset the timer for the next cycle
        offTime = millis() + 14400000 + 300000; // 4h + 5min
        Serial.println("Fogger button pressed for ALWAYS_ON schedule (after cooldown)");
      }
    } else {
      // Standard behavior for light and water pins
      digitalWrite(pin, HIGH);
    }
    return;
  }

  // Handle regular pins that need to be turned off after duration
  if (pin != foggerPin && running && millis() >= offTime) {
    digitalWrite(pin, LOW);
    running = false;
  }

  // For fogger, we don't check offTime for turning off since it self-manages
  
  bool secondInstance = false;
  if (!running && matchSchedule(now, sch, secondInstance)) {
    int durationMs = secondInstance ? sch.duration2 : sch.duration1;
    
    if (pin == foggerPin) {
      // For fogger, just press the button to activate and set flag
      pressFoggerButton();
      running = true;
      // For scheduled (non-ALWAYS_ON) operation, we still track when the 4-hour cycle should end
      offTime = millis() + 14400000; // 4 hours in milliseconds
      Serial.println("Fogger button pressed for scheduled activation");
    } else {
      // Standard pins get set HIGH with our duration
      digitalWrite(pin, HIGH);
      running = true;
      offTime = millis() + durationMs;
    }
  }
}

// Engage the pin for the specified duration
void TaskScheduler::executeTask(
  int pin,
  int durationMs,
  bool& running,
  unsigned long& offTime
) {
  if (pin == foggerPin) {
    // For fogger, simulate button press to turn on
    pressFoggerButton();
    Serial.println("Fogger button pressed for scheduled activation");
  } else {
    // Standard pins get set HIGH
    digitalWrite(pin, HIGH);
  }
  
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

// Add these new methods to save and load schedules

bool TaskScheduler::saveSchedules() {
    File f = LittleFS.open(SCHEDULE_FILE, "w");
    if (!f) {
        Serial.println("❌ Failed to open schedule file for write");
        return false;
    }

    // Write light schedule
    f.write((uint8_t*)&lightSchedule, sizeof(Schedule));
    
    // Write water schedule
    f.write((uint8_t*)&waterSchedule, sizeof(Schedule));
    
    // Write fogger schedule
    f.write((uint8_t*)&foggerSchedule, sizeof(Schedule));

    f.close();
    Serial.println("✅ Schedules saved to LittleFS");
    return true;
}

bool TaskScheduler::loadSchedules() {
    if (!LittleFS.exists(SCHEDULE_FILE)) {
        Serial.println("ℹ️ No schedule file found, using defaults");
        return false;
    }

    File f = LittleFS.open(SCHEDULE_FILE, "r");
    if (!f) {
        Serial.println("❌ Failed to open schedule file for read");
        return false;
    }

    // Read light schedule
    f.read((uint8_t*)&lightSchedule, sizeof(Schedule));
    
    // Read water schedule
    f.read((uint8_t*)&waterSchedule, sizeof(Schedule));
    
    // Read fogger schedule
    f.read((uint8_t*)&foggerSchedule, sizeof(Schedule));

    f.close();
    Serial.println("✅ Schedules loaded from LittleFS");
    
    // Debug output
    Serial.println("Loaded schedules:");
    Serial.print("Light: ");
    Serial.println(scheduleToString(lightSchedule, "LIGHT"));
    Serial.print("Water: ");
    Serial.println(scheduleToString(waterSchedule, "WATER"));
    Serial.print("Fogger: ");
    Serial.println(scheduleToString(foggerSchedule, "FOGGER"));
    
    return true;
}

// Add this method implementation
bool TaskScheduler::clearSchedules() {
    // Remove the schedule file if it exists
    if (LittleFS.exists(SCHEDULE_FILE)) {
        if (LittleFS.remove(SCHEDULE_FILE)) {
            Serial.println("✅ Schedules file removed successfully");
        } else {
            Serial.println("❌ Failed to remove schedules file");
            return false;
        }
    }
    
    // Reset schedules to NONE
    Schedule emptySchedule = {NONE, 0, 0, 0, 0, 0, 0};
    lightSchedule = emptySchedule;
    waterSchedule = emptySchedule;
    foggerSchedule = emptySchedule;
    
    // Turn off any running outputs
    digitalWrite(lightPin, LOW);
    digitalWrite(waterPin, LOW);
    
    // For fogger, press button if it's running to turn it off
    if (foggerRunning) {
        pressFoggerButton();
    }
    
    lightRunning = false;
    waterRunning = false;
    foggerRunning = false;
    
    Serial.println("✅ All schedules reset to default values");
    return true;
}

// Update getSchedulesAsString to include fogger
String TaskScheduler::getSchedulesAsString() {
    String result = "LIGHT:";
    
    // Format light schedule
    result += scheduleToString(lightSchedule, "LIGHT");
    result += ";WATER:";
    
    // Format water schedule  
    result += scheduleToString(waterSchedule, "WATER");
    
    // Add fogger schedule
    result += ";FOGGER:";
    result += scheduleToString(foggerSchedule, "FOGGER");
    
    return result;
}

// Update JSON output to include fogger
String TaskScheduler::getSchedulesAsJSON() {
    String json = "{";
    
    // Light schedule
    json += "\"light\":{";
    json += "\"type\":\"" + scheduleTypeToString(lightSchedule.type) + "\",";
    json += "\"hour1\":" + String(lightSchedule.hour1) + ",";
    json += "\"minute1\":" + String(lightSchedule.minute1) + ",";
    json += "\"duration1\":" + String(lightSchedule.duration1/1000) + ","; // Convert to seconds
    
    // Include second time for TWICE_* schedules
    if (lightSchedule.type == TWICE_DAILY || lightSchedule.type == TWICE_WEEKLY || lightSchedule.type == TWICE_MONTHLY) {
        json += "\"hour2\":" + String(lightSchedule.hour2) + ",";
        json += "\"minute2\":" + String(lightSchedule.minute2) + ",";
        json += "\"duration2\":" + String(lightSchedule.duration2/1000); // Convert to seconds
    } else {
        json += "\"hour2\":0,\"minute2\":0,\"duration2\":0";
    }
    json += "},";
    
    // Water schedule
    json += "\"water\":{";
    json += "\"type\":\"" + scheduleTypeToString(waterSchedule.type) + "\",";
    json += "\"hour1\":" + String(waterSchedule.hour1) + ",";
    json += "\"minute1\":" + String(waterSchedule.minute1) + ",";
    json += "\"duration1\":" + String(waterSchedule.duration1/1000) + ","; // Convert to seconds
    
    // Include second time for TWICE_* schedules
    if (waterSchedule.type == TWICE_DAILY || waterSchedule.type == TWICE_WEEKLY || waterSchedule.type == TWICE_MONTHLY) {
        json += "\"hour2\":" + String(waterSchedule.hour2) + ",";
        json += "\"minute2\":" + String(waterSchedule.minute2) + ",";
        json += "\"duration2\":" + String(waterSchedule.duration2/1000); // Convert to seconds
    } else {
        json += "\"hour2\":0,\"minute2\":0,\"duration2\":0";
    }
    json += "},";
    
    // Fogger schedule
    json += "\"fogger\":{";
    json += "\"type\":\"" + scheduleTypeToString(foggerSchedule.type) + "\",";
    json += "\"hour1\":" + String(foggerSchedule.hour1) + ",";
    json += "\"minute1\":" + String(foggerSchedule.minute1) + ",";
    json += "\"duration1\":" + String(foggerSchedule.duration1/1000) + ","; // Convert to seconds
    
    // Include second time for TWICE_* schedules
    if (foggerSchedule.type == TWICE_DAILY || foggerSchedule.type == TWICE_WEEKLY || foggerSchedule.type == TWICE_MONTHLY) {
        json += "\"hour2\":" + String(foggerSchedule.hour2) + ",";
        json += "\"minute2\":" + String(foggerSchedule.minute2) + ",";
        json += "\"duration2\":" + String(foggerSchedule.duration2/1000); // Convert to seconds
    } else {
        json += "\"hour2\":0,\"minute2\":0,\"duration2\":0";
    }
    json += "}";
    
    json += "}";
    return json;
}

// Add this implementation for the scheduleToString method
String TaskScheduler::scheduleToString(const Schedule& sch, const String& type) {
    switch (sch.type) {
        case ALWAYS_ON:
            return type + " ALWAYS_ON";
        case DAILY:
            return type + " DAILY," + String(sch.hour1) + "," + String(sch.minute1) + "," + String(sch.duration1/1000);
        case WEEKLY:
            return type + " WEEKLY," + String(sch.hour1) + "," + String(sch.minute1) + "," + String(sch.duration1/1000);
        case TWICE_DAILY:
            return type + " TWICE_DAILY," + String(sch.hour1) + "," + String(sch.minute1) + "," + 
                   String(sch.duration1/1000) + "," + String(sch.hour2) + "," + String(sch.minute2);
        case TWICE_WEEKLY:
            return type + " TWICE_WEEKLY," + String(sch.hour1) + "," + String(sch.minute1) + "," + 
                   String(sch.duration1/1000) + "," + String(sch.hour2) + "," + String(sch.minute2);
        case MONTHLY:
            return type + " MONTHLY," + String(sch.hour1) + "," + String(sch.minute1) + "," + String(sch.duration1/1000);
        case TWICE_MONTHLY:
            return type + " TWICE_MONTHLY," + String(sch.hour1) + "," + String(sch.minute1) + "," + 
                   String(sch.duration1/1000) + "," + String(sch.hour2) + "," + String(sch.minute2);
        default:
            return type + " NONE";
    }
}

// Helper to convert schedule type to string
String scheduleTypeToString(ScheduleType type) {
    switch (type) {
        case NONE: return "NONE";
        case ALWAYS_ON: return "ALWAYS_ON";
        case DAILY: return "DAILY";
        case WEEKLY: return "WEEKLY";
        case TWICE_DAILY: return "TWICE_DAILY";
        case TWICE_WEEKLY: return "TWICE_WEEKLY";
        case MONTHLY: return "MONTHLY";
        case TWICE_MONTHLY: return "TWICE_MONTHLY";
        default: return "UNKNOWN";
    }
}
