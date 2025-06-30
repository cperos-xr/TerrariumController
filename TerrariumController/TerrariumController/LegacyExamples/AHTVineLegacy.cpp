#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_AHTX0.h>

#define SCREEN_W   128
#define SCREEN_H    32
#define OLED_ADDR   0x3C

Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);
Adafruit_AHTX0   aht;

const unsigned long READ_INTERVAL = 3000;
const unsigned long FRAME_MS      =  100;
const uint8_t        SCROLL_SPEED =    1;

float lastHum=0, lastTemp=0;
float dailyHi=-100, dailyLo=200;
float weeklyHi=-100, weeklyLo=200;
float dailyHumHi=-1, dailyHumLo=101;
float weeklyHumHi=-1, weeklyHumLo=101;

const float THIGH_WARN = 100, TLOW_WARN = 32;
const float HHIGH_WARN = 100, HLOW_WARN = 50;

struct DisplayItem {
  String l1, l2, l3;
  int    baseY;
};
#define ITEM_COUNT 15
DisplayItem items[ITEM_COUNT];
int scrollOffset = 0;

void updateItems() {
  // live Temp with “!” if out of [32..100]
  String t = String(int(lastTemp)) + "F";
  if (lastTemp < TLOW_WARN || lastTemp > THIGH_WARN) t = "!" + t;
  items[0] = {"Temp", t, ""};

  // rolling Temp
  items[1] = {"Day",  "Max", String(int(dailyHi))   + "F"};
  items[2] = {"Day",  "Min", String(int(dailyLo))   + "F"};
  items[3] = {"Week", "Max", String(int(weeklyHi))  + "F"};
  items[4] = {"Week", "Min", String(int(weeklyLo))  + "F"};
  // Temp thresholds
  items[5] = {"Warn", "Max", String(int(THIGH_WARN)) + "F"};
  items[6] = {"Warn", "Min", String(int(TLOW_WARN))  + "F"};

  // live Hum with “!” if out of [50..100]
  String h = String(int(lastHum)) + "%";
  if (lastHum < HLOW_WARN || lastHum > HHIGH_WARN) h = "!" + h;
  items[7] = {"Hum", h, ""};

  // rolling Hum
  items[8]  = {"Day",  "Max", String(int(dailyHumHi))  + "%"};
  items[9]  = {"Day",  "Min", String(int(dailyHumLo))  + "%"};
  items[10] = {"Week", "Max", String(int(weeklyHumHi)) + "%"};
  items[11] = {"Week", "Min", String(int(weeklyHumLo)) + "%"};
  // Hum thresholds
  items[12] = {"Warn", "Max", String(int(HHIGH_WARN)) + "%"};
  items[13] = {"Warn", "Min", String(int(HLOW_WARN))  + "%"};

  // watering
  items[14] = {"Water","2×Day","1×Week"};

  // compute Y positions
  const int itemH = 3*8 + 4;
  const int gap   = 4;
  for (int i = 0; i < ITEM_COUNT; i++) {
    items[i].baseY = i * (itemH + gap);
  }
}

void readSensor() {
  sensors_event_t eh, et;
  if (aht.getEvent(&eh, &et)) {
    lastHum  = eh.relative_humidity;
    lastTemp = et.temperature * 9.0/5.0 + 32.0;

    // rolling Temp
    if (dailyHi < -99) dailyHi = lastTemp;
    if (dailyLo > 199) dailyLo = lastTemp;
    dailyHi = max(dailyHi, lastTemp);
    dailyLo = min(dailyLo, lastTemp);
    if (weeklyHi < -99) weeklyHi = lastTemp;
    if (weeklyLo > 199) weeklyLo = lastTemp;
    weeklyHi = max(weeklyHi, lastTemp);
    weeklyLo = min(weeklyLo, lastTemp);

    // rolling Hum
    if (dailyHumHi <  0) dailyHumHi = lastHum;
    if (dailyHumLo >100) dailyHumLo = lastHum;
    dailyHumHi = max(dailyHumHi, lastHum);
    dailyHumLo = min(dailyHumLo, lastHum);
    if (weeklyHumHi <  0) weeklyHumHi = lastHum;
    if (weeklyHumLo >100) weeklyHumLo = lastHum;
    weeklyHumHi = max(weeklyHumHi, lastHum);
    weeklyHumLo = min(weeklyHumLo, lastHum);
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(5, 6);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) while (1);
  if (!aht.begin()) while (1);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setRotation(3);  // portrait
  readSensor();
  updateItems();
}

void loop() {
  static unsigned long lastRead=0, lastFrame=0;
  unsigned long now = millis();

  if (now - lastRead >= READ_INTERVAL) {
    readSensor();
    lastRead = now;
  }
  if (now - lastFrame < FRAME_MS) return;
  lastFrame = now;

  updateItems();
  display.clearDisplay();

  // rotated dims 32×128
  int H = display.height();

  // top 4 lines fixed (0–31)
  const int topH    = 4*8;
  const int scrollY = topH;
  const int scrollH = H - topH;

  // draw fixed 4-line portion
  display.setCursor(0, 0);
  display.print("Temp:");
  display.setCursor(0, 8);
  display.print(String(int(lastTemp)) + "F");
  display.setCursor(0,16);
  display.print("Hum:");
  display.setCursor(0,24);
  display.print(String(int(lastHum)) + "%");

  // scroll items downward
  const int itemH = 3*8 + 4;
  const int totalH = ITEM_COUNT * (itemH + 4);

  scrollOffset += SCROLL_SPEED;
  if (scrollOffset >= totalH + scrollH) scrollOffset = -scrollH;

  for (int i = 1; i < ITEM_COUNT; i++) {
    int y = items[i].baseY - scrollOffset + scrollY;

    // **NEW CLIP**: skip anything that would overlap line 0–31
    if (y < scrollY || y + itemH > scrollY + scrollH) continue;

    display.setCursor(0,     y);   display.print(items[i].l1);
    display.setCursor(0, y + 8);   display.print(items[i].l2);
    display.setCursor(0, y +16);   display.print(items[i].l3);
  }

  display.display();
}
