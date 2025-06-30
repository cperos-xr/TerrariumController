// ---- DisplayManager.cpp ----
#include "DisplayManager.h"
#include "SensorManager.h"
#include "ActuatorControl.h"

Adafruit_SSD1306 display(128, 32, &Wire, -1);
DisplayItem items[ITEM_COUNT];

void initDisplay() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setRotation(3);
  display.clearDisplay();
  display.display();
}


void drawScrollStats() {
  static int scrollOffset = 0;
  const int itemHeight = 3 * 8 + 4; // Height of each item
  const int gap = 4;                // Gap between items
  const int topFixedHeight = 4 * 8; // Fixed top portion height
  const int scrollAreaHeight = display.height() - topFixedHeight;
  const int totalHeight = ITEM_COUNT * (itemHeight + gap);

  // Update scroll offset
  scrollOffset += SCROLL_SPEED;
  if (scrollOffset >= totalHeight + scrollAreaHeight) {
    scrollOffset = -scrollAreaHeight;
  }

  // Clear display
  display.clearDisplay();

  // Draw fixed top portion
  display.setCursor(0, 0);
  display.print("Temp:");
  display.setCursor(0, 8);
  display.print(String((int)tempF) + "F");
  display.setCursor(0, 16);
  display.print("Hum:");
  display.setCursor(0, 24);
  display.print(String((int)hum) + "%");

  // Draw scrolling items
  const int scrollY = topFixedHeight; // Start of scroll area
  for (int i = 0; i < ITEM_COUNT; i++) {
    int y = items[i].baseY - scrollOffset + scrollY;

    // Skip items outside the scroll area
    if (y < scrollY || y + itemHeight > scrollY + scrollAreaHeight) {
      continue;
    }

    display.setCursor(0, y);
    display.print(items[i].l1);
    display.setCursor(0, y + 8);
    display.print(items[i].l2);
    display.setCursor(0, y + 16);
    display.print(items[i].l3);
  }

  // Display the updated content
  display.display();
}

void drawStatusScreen() {
  display.clearDisplay();

  // Top third: Temp
  display.setCursor(0, 0);
  display.print("Temp");
  display.setCursor(0, 8);
  display.print((int)tempF); display.print("F");

  // Middle third: Humidity
  display.setCursor(0, 24);
  display.print("Humid");
  display.setCursor(0, 32);
  display.print((int)hum); display.print("%");

  // Bottom third: Water & Light
  unsigned long now = millis();

  unsigned long nextWater = WATER_INTERVAL - (now - lastWaterTime);
  int waterSecs = max(int(nextWater / 1000), 0);
  display.setCursor(0, 48);
  display.print("Water");
  display.setCursor(0, 56);
  display.print(waterSecs); display.print("s");

  unsigned long nextLight = LIGHT_INTERVAL - (now - lastLightTime);
  int lightSecs = max(int(nextLight / 1000), 0);
  display.setCursor(0, 70);
  display.print("Light");
  display.setCursor(0, 80);
  display.print(lightSecs); display.print("s");

  display.display();
}

void drawSensorOverlayVine() {
  drawStatusScreen(); // placeholder combo for now
}

void updateItems() {
  // Live temperature with warning if out of range
  String temp = String((int)tempF) + "F";
  if (tempF < TLOW_WARN || tempF > THIGH_WARN) temp = "!" + temp;
  items[0] = {"Temp", temp, ""};

  // Rolling temperature stats
  items[1] = {"Day", "Max", String((int)dailyHi) + "F"};
  items[2] = {"Day", "Min", String((int)dailyLo) + "F"};
  items[3] = {"Week", "Max", String((int)weeklyHi) + "F"};
  items[4] = {"Week", "Min", String((int)weeklyLo) + "F"};

  // Live humidity with warning if out of range
  String humStr = String((int)hum) + "%";
  if (hum < HLOW_WARN || hum > HHIGH_WARN) humStr = "!" + humStr;
  items[5] = {"Hum", humStr, ""};

  // Rolling humidity stats
  items[6] = {"Day", "Max", String((int)dailyHumHi) + "%"};
  items[7] = {"Day", "Min", String((int)dailyHumLo) + "%"};
  items[8] = {"Week", "Max", String((int)weeklyHumHi) + "%"};
  items[9] = {"Week", "Min", String((int)weeklyHumLo) + "%"};

  // Compute Y positions for scrolling
  const int itemHeight = 3 * 8 + 4;
  const int gap = 4;
  for (int i = 0; i < ITEM_COUNT; i++) {
    items[i].baseY = i * (itemHeight + gap);
  }
}
