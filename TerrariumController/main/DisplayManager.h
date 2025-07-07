#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <Adafruit_SSD1306.h>
#include <Arduino.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
#define DISPLAY_ADDRESS 0x3C

class DisplayManager {
public:
    DisplayManager();
    void initDisplay();
    void printMessage(const String& message);
    void clearDisplay();
    bool isDisplayAvailable();

private:
    Adafruit_SSD1306 display;
};

#endif // DISPLAYMANAGER_H