#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Adafruit_SSD1306.h>
#include "TaskScheduler.h"

extern Adafruit_SSD1306 display;
extern TaskScheduler scheduler; // Declare the global TaskScheduler instance

#define ITEM_COUNT 15
#define SCROLL_SPEED 1

struct DisplayItem {
  String l1, l2, l3;
  int baseY;
};

extern DisplayItem items[ITEM_COUNT];

void initDisplay();
void drawScrollStats();
void drawStatusScreen();
void drawSensorOverlayVine();
void updateItems(); // Declare updateItems function

#endif