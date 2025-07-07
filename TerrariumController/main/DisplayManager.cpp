#include "DisplayManager.h"

DisplayManager::DisplayManager() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {
}

void DisplayManager::initDisplay() {
    if (display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDRESS)) {
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setRotation(3);
        display.clearDisplay();
        display.display();
        Serial.println("✅ Display initialized successfully");
    } else {
        Serial.println("❌ Display initialization failed");
    }
}

void DisplayManager::printMessage(const String& message) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(message);
    display.display();
}

bool DisplayManager::isDisplayAvailable() {
    Wire.beginTransmission(DISPLAY_ADDRESS);
    return (Wire.endTransmission() == 0);
}

void DisplayManager::clearDisplay() {
    display.clearDisplay();
    display.display();
}

