// ---- Animations.cpp ----
#include "Animations.h"
#include "DisplayManager.h"
void playAnimation2() {
  display.setCursor(0, 0); display.print("Anim2...");
}
void playAnimation3() {
  display.setCursor(0, 0); display.print("Anim3...");
}
void drawVine() {
  for (int x = 0; x < 128; x += 4) {
    int y = 16 + 8 * sin(0.1 * x);
    display.drawPixel(x, y, SSD1306_WHITE);
  }
}