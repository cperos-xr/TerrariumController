/* TaskScheduler.cpp */
#include "TaskScheduler.h"

// Constructor
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
{}

// String → enum
ScheduleType TaskScheduler::parseType(const String& s) {
    if (s == "ALWAYS_ON")     return ALWAYS_ON;
    if (s == "DAILY")         return DAILY;
    if (s == "WEEKLY")        return WEEKLY;
    if (s == "TWICE_DAILY")   return TWICE_DAILY;
    if (s == "TWICE_WEEKLY")  return TWICE_WEEKLY;
    if (s == "MONTHLY")       return MONTHLY;
    if (s == "TWICE_MONTHLY") return TWICE_MONTHLY;
    return NONE;
}

// Parse “TARGET,TYPE,h1,m1,d1[,h2,m2]”
void TaskScheduler::parseAndSetSchedule(const String& cmd) {
    String parts[8];
    int count = 0, start = 0;
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
    sch.duration1 = parts[4].toInt() * 1000;
    if ((sch.type == TWICE_DAILY || sch.type == TWICE_WEEKLY || sch.type == TWICE_MONTHLY)
        && count >= 7) {
        sch.hour2     = parts[5].toInt();
        sch.minute2   = parts[6].toInt();
        sch.duration2 = sch.duration1;
    } else {
        sch.hour2 = sch.minute2 = sch.duration2 = 0;
    }

    bool wasL = (lightSchedule.type == ALWAYS_ON);
    bool wasW = (waterSchedule.type == ALWAYS_ON);
    bool wasF = (foggerSchedule.type == ALWAYS_ON);

    if (parts[0] == "LIGHT") {
        lightSchedule = sch;
        Serial.println("Light schedule updated.");
        if (wasL && sch.type != ALWAYS_ON) {
            digitalWrite(lightPin, LOW);
            lightRunning = false;
        } else if (!wasL && sch.type == ALWAYS_ON) {
            digitalWrite(lightPin, HIGH);
        } else if (sch.type == NONE) {
            digitalWrite(lightPin, LOW);
            lightRunning = false;
        }
    }
    else if (parts[0] == "WATER") {
        waterSchedule = sch;
        Serial.println("Water schedule updated.");
        if (wasW && sch.type != ALWAYS_ON) {
            digitalWrite(waterPin, LOW);
            waterRunning = false;
        } else if (!wasW && sch.type == ALWAYS_ON) {
            digitalWrite(waterPin, HIGH);
        } else if (sch.type == NONE) {
            digitalWrite(waterPin, LOW);
            waterRunning = false;
        }
    }
    else if (parts[0] == "FOGGER") {
        foggerSchedule = sch;
        Serial.println("Fogger schedule updated.");
        if (wasF && sch.type != ALWAYS_ON) {
            pressFoggerButton();
            foggerRunning = false;
        } else if (!wasF && sch.type == ALWAYS_ON) {
            pressFoggerButton();
        } else if (sch.type == NONE && foggerRunning) {
            pressFoggerButton();
            foggerRunning = false;
        }
    }
    else {
        Serial.println("Unknown target. Use LIGHT, WATER, or FOGGER.");
        return;
    }

    saveSchedules();
}

// Manual toggle implementations - simplified version
void TaskScheduler::toggleLight() {
    if (lightRunning) {
        digitalWrite(lightPin, LOW);
        lightRunning = false;
        Serial.println("Light manually turned OFF");
    } else {
        digitalWrite(lightPin, HIGH);
        lightRunning = true;
        Serial.println("Light manually turned ON");
    }
    // No override flags - schedule will take over immediately
}

void TaskScheduler::toggleWater() {
    if (waterRunning) {
        digitalWrite(waterPin, LOW);
        waterRunning = false;
        Serial.println("Water manually turned OFF");
    } else {
        digitalWrite(waterPin, HIGH);
        waterRunning = true;
        Serial.println("Water manually turned ON");
    }
    // No override flags - schedule will take over immediately
}

void TaskScheduler::toggleFogger() {
    // For fogger, we just press the button - it toggles the state
    pressFoggerButton();
    foggerRunning = !foggerRunning;
    Serial.println(foggerRunning ? "Fogger manually turned ON" : "Fogger manually turned OFF");
    // No override flags - schedule will take over immediately
}

// Called each loop - now without override checks
void TaskScheduler::updateTasks(const DateTime& now) {
    // No need to check overrides anymore
    // Apply schedules directly
    applySchedule(lightSchedule, lightPin, lightRunning, lightOffMillis, now);
    applySchedule(waterSchedule, waterPin, waterRunning, waterOffMillis, now);
    applySchedule(foggerSchedule, foggerPin, foggerRunning, foggerOffMillis, now);
}

// Core on/off logic
void TaskScheduler::applySchedule(
    const Schedule& sch,
    int pin,
    bool& running,
    unsigned long& offTime,
    const DateTime& now
) {
    if (sch.type == NONE) return;

    // ALWAYS_ON
    if (sch.type == ALWAYS_ON) {
        if (pin == foggerPin) {
            if (!running) {
                pressFoggerButton();
                running = true;
                offTime = millis() + 14400000 + 300000;
            } else if (millis() > offTime) {
                pressFoggerButton();
                offTime = millis() + 14400000 + 300000;
            }
        } else {
            digitalWrite(pin, HIGH);
        }
        return;
    }

    // turn off when duration elapses
    if (pin != foggerPin && running && millis() >= offTime) {
        digitalWrite(pin, LOW);
        running = false;
    }

    bool secondInstance = false;
    if (!running && matchSchedule(now, sch, secondInstance)) {
        int dur = secondInstance ? sch.duration2 : sch.duration1;
        if (pin == foggerPin) {
            pressFoggerButton();
            running = true;
            offTime  = millis() + 14400000; 
        } else {
            digitalWrite(pin, HIGH);
            running = true;
            offTime = millis() + dur;
        }
    }
}

// matchSchedule: now → true if we should fire (once per minute)
bool TaskScheduler::matchSchedule(
    const DateTime& now,
    const Schedule& sch,
    bool& isSecond
) {
    switch (sch.type) {
      case DAILY:
        if (now.hour() == sch.hour1
            && now.minute() == sch.minute1) {
          isSecond = false;
          return true;
        }
        break;

      case TWICE_DAILY:
        if (now.hour() == sch.hour1
            && now.minute() == sch.minute1) {
          isSecond = false;
          return true;
        }
        if (now.hour() == sch.hour2
            && now.minute() == sch.minute2) {
          isSecond = true;
          return true;
        }
        break;

      case WEEKLY:
        if (now.dayOfTheWeek() == 0
            && now.hour() == sch.hour1
            && now.minute() == sch.minute1) {
          isSecond = false;
          return true;
        }
        break;

      case TWICE_WEEKLY:
        if (now.dayOfTheWeek() == 0
            && now.hour() == sch.hour1
            && now.minute() == sch.minute1) {
          isSecond = false;
          return true;
        }
        if (now.dayOfTheWeek() == 3
            && now.hour() == sch.hour1
            && now.minute() == sch.minute1) {
          isSecond = true;
          return true;
        }
        break;

      case MONTHLY:
        if (now.day() == 1
            && now.hour() == sch.hour1
            && now.minute() == sch.minute1) {
          isSecond = false;
          return true;
        }
        break;

      case TWICE_MONTHLY:
        if (now.day() == 1
            && now.hour() == sch.hour1
            && now.minute() == sch.minute1) {
          isSecond = false;
          return true;
        }
        if (now.day() == 15
            && now.hour() == sch.hour1
            && now.minute() == sch.minute1) {
          isSecond = true;
          return true;
        }
        break;

      default:
        break;
    }
    return false;
}

// Persistence: save/load/clear

bool TaskScheduler::saveSchedules() {
    File f = LittleFS.open(SCHEDULE_FILE, "w");
    if (!f) { Serial.println("❌ Failed to open for write"); return false; }
    f.write((uint8_t*)&lightSchedule,  sizeof(Schedule));
    f.write((uint8_t*)&waterSchedule,  sizeof(Schedule));
    f.write((uint8_t*)&foggerSchedule, sizeof(Schedule));
    f.close();
    Serial.println("✅ Schedules saved");
    return true;
}

bool TaskScheduler::loadSchedules() {
    if (!LittleFS.exists(SCHEDULE_FILE)) {
        Serial.println("ℹ️ No schedule file found");
        return false;
    }
    File f = LittleFS.open(SCHEDULE_FILE, "r");
    if (!f) { Serial.println("❌ Failed to open for read"); return false; }
    f.read((uint8_t*)&lightSchedule,  sizeof(Schedule));
    f.read((uint8_t*)&waterSchedule,  sizeof(Schedule));
    f.read((uint8_t*)&foggerSchedule, sizeof(Schedule));
    f.close();
    Serial.println("✅ Schedules loaded");
    return true;
}

bool TaskScheduler::clearSchedules() {
    if (LittleFS.exists(SCHEDULE_FILE)) {
        if (!LittleFS.remove(SCHEDULE_FILE)) {
            Serial.println("❌ Failed to remove file");
            return false;
        }
    }
    Schedule e = { NONE,0,0,0, 0,0,0 };
    lightSchedule = waterSchedule = foggerSchedule = e;
    digitalWrite(lightPin, LOW);
    digitalWrite(waterPin, LOW);
    if (foggerRunning) pressFoggerButton();
    lightRunning = waterRunning = foggerRunning = false;
    Serial.println("✅ Schedules cleared");
    return true;
}

// (Unchanged) Stringify schedules:

String TaskScheduler::getSchedulesAsString() {
    String r = "LIGHT:"   + scheduleToString(lightSchedule,  "LIGHT")
             + ";WATER:"  + scheduleToString(waterSchedule,  "WATER")
             + ";FOGGER:" + scheduleToString(foggerSchedule, "FOGGER");
    return r;
}

String TaskScheduler::getSchedulesAsJSON() {
    String j = "{";
    // Light
    j += "\"light\":{";
    j += "\"type\":\""   + scheduleTypeToString(lightSchedule.type) + "\",";
    j += "\"hour1\":"   + String(lightSchedule.hour1)                   + ",";
    j += "\"minute1\":" + String(lightSchedule.minute1)                 + ",";
    j += "\"duration1\":" + String(lightSchedule.duration1/1000)        + ",";
    if (lightSchedule.type == TWICE_DAILY || lightSchedule.type == TWICE_WEEKLY || lightSchedule.type == TWICE_MONTHLY) {
      j += "\"hour2\":"   + String(lightSchedule.hour2)   + ",";
      j += "\"minute2\":" + String(lightSchedule.minute2) + ",";
      j += "\"duration2\":" + String(lightSchedule.duration2/1000);
    } else {
      j += "\"hour2\":0,\"minute2\":0,\"duration2\":0";
    }
    j += "},";
    // Water
    j += "\"water\":{";
    j += "\"type\":\""   + scheduleTypeToString(waterSchedule.type) + "\",";
    j += "\"hour1\":"   + String(waterSchedule.hour1)                + ",";
    j += "\"minute1\":" + String(waterSchedule.minute1)              + ",";
    j += "\"duration1\":" + String(waterSchedule.duration1/1000)     + ",";
    if (waterSchedule.type == TWICE_DAILY || waterSchedule.type == TWICE_WEEKLY || waterSchedule.type == TWICE_MONTHLY) {
      j += "\"hour2\":"   + String(waterSchedule.hour2)   + ",";
      j += "\"minute2\":" + String(waterSchedule.minute2) + ",";
      j += "\"duration2\":" + String(waterSchedule.duration2/1000);
    } else {
      j += "\"hour2\":0,\"minute2\":0,\"duration2\":0";
    }
    j += "},";
    // Fogger
    j += "\"fogger\":{";
    j += "\"type\":\""   + scheduleTypeToString(foggerSchedule.type) + "\",";
    j += "\"hour1\":"   + String(foggerSchedule.hour1)                + ",";
    j += "\"minute1\":" + String(foggerSchedule.minute1)              + ",";
    j += "\"duration1\":" + String(foggerSchedule.duration1/1000)     + ",";
    if (foggerSchedule.type == TWICE_DAILY || foggerSchedule.type == TWICE_WEEKLY || foggerSchedule.type == TWICE_MONTHLY) {
      j += "\"hour2\":"   + String(foggerSchedule.hour2)   + ",";
      j += "\"minute2\":" + String(foggerSchedule.minute2) + ",";
      j += "\"duration2\":" + String(foggerSchedule.duration2/1000);
    } else {
      j += "\"hour2\":0,\"minute2\":0,\"duration2\":0";
    }
    j += "}";
    j += "}";
    return j;
}

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

String scheduleTypeToString(ScheduleType type) {
    switch (type) {
      case NONE:           return "NONE";
      case ALWAYS_ON:      return "ALWAYS_ON";
      case DAILY:          return "DAILY";
      case WEEKLY:         return "WEEKLY";
      case TWICE_DAILY:    return "TWICE_DAILY";
      case TWICE_WEEKLY:   return "TWICE_WEEKLY";
      case MONTHLY:        return "MONTHLY";
      case TWICE_MONTHLY:  return "TWICE_MONTHLY";
      default:             return "UNKNOWN";
    }
}
