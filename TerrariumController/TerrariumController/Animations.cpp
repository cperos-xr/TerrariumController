#include "Animations.h"
#include "DisplayManager.h"
#include <math.h>

// Bouncing & Spinning Bloom Animation (Legacy exact)
static const int PETALS   = 8;
static const int RADIUS   = 8;   // petal distance from center
static const int PETAL_R  = 2;   // petal radius
static const int BOX_W    = RADIUS * 2;
static const int BOX_H    = RADIUS * 2;

// Initialize bloom position
static int16_t bx = (SCREEN_W - BOX_W) / 2;
static int16_t by = (SCREEN_H - BOX_H) / 2;
static int8_t  vx2 = 1, vy2 = 3;
static float   angle = 0, dAngle = 0.1;

void playAnimation2() {
  // draw bloom
  int16_t cx = bx + RADIUS;
  int16_t cy = by + RADIUS;
  for (int i = 0; i < PETALS; i++) {
    float a = angle + i * (TWO_PI / PETALS);
    int16_t px = cx + round(cos(a) * RADIUS);
    int16_t py = cy + round(sin(a) * RADIUS);
    display.fillCircle(px, py, PETAL_R, SSD1306_WHITE);
  }
  display.fillCircle(cx, cy, PETAL_R, SSD1306_WHITE);

  // bounce
  bx += vx2; by += vy2;
  if (bx <= 0 || bx >= SCREEN_W - BOX_W) { vx2 = -vx2; bx += vx2; }
  if (by <= 0 || by >= SCREEN_H - BOX_H) { vy2 = -vy2; by += vy2; }

  // spin
  angle += dAngle;
  if (angle > TWO_PI) angle -= TWO_PI;
}

void playAnimation3() {
  // Umbrella + Gentle Rain (legacy style placeholder)
  struct Umb { int16_t x,y; int8_t vx,vy; };
  static Umb umbrellas[3];
  static bool init = false;
  if (!init) {
    for (int i = 0; i < 3; i++) {
      umbrellas[i].x  = random(SCREEN_W - 12);
      umbrellas[i].y  = random(SCREEN_H - 8);
      umbrellas[i].vx = random(1,3) * (random(2)?1:-1);
      umbrellas[i].vy = random(1,3) * (random(2)?1:-1);
    }
    init = true;
  }
  struct Raindot { int x,y; } rain[10];
  for (auto &u: umbrellas) {
    // draw umbrella canopy
    int16_t cx = u.x + 4;
    int16_t cy = u.y + 4;
    display.fillCircle(cx, cy, 4, SSD1306_WHITE);
    u.x += u.vx; u.y += u.vy;
    if (u.x <= 0 || u.x >= SCREEN_W-12) u.vx = -u.vx;
    if (u.y <= 0 || u.y >= SCREEN_H-8)  u.vy = -u.vy;
  }
  // simple rain
  for (int i = 0; i < 10; i++) {
    int rx = random(SCREEN_W);
    int ry = random(SCREEN_H);
    display.drawPixel(rx, ry, SSD1306_WHITE);
  }
}

void drawVine() {
  static float phase = 0; // Phase for sine wave
  const float waveFreq = 2 * PI / 128; // Frequency adjusted for vertical movement
  const int amplitude = 6; // Amplitude for horizontal oscillation
  const int spacing = 16; // Spacing between leaves
  const int leafSize = 8; // Size of the leaves
  const int numLeaves = (128 / spacing) + 2; // Number of leaves for vertical screen
  static int starX[12], starY[12], leafY[numLeaves];
  static bool init = false;

  // Initialize stars and leaf positions
  if (!init) {
    randomSeed(micros());
    for (int i = 0; i < 12; i++) {
      starX[i] = random(32); // Full horizontal range for 32-wide screen
      starY[i] = random(128); // Full vertical range
    }
    for (int i = 0; i < numLeaves; i++) {
      leafY[i] = i * spacing;
    }
    init = true;
  }

  // Clear the display
  display.clearDisplay();

  // Draw the vine line (vertical with horizontal oscillation)
  int px = 16; // Center X position
  int py = 0;  // Start Y position
  
  for (int y = 0; y < 128; y += 4) {
    int x = 16 + sin(waveFreq * y + phase) * amplitude; // Oscillate horizontally
    if (y > 0) {
      display.drawLine(px, py, x, y, SSD1306_WHITE);
    }
    px = x;
    py = y;
  }

  // Draw stars
  for (int i = 0; i < 12; i++) {
    if (random(10) > 2) {
      display.drawPixel(starX[i], starY[i], SSD1306_WHITE);
    }
  }

  // Draw alternating hearts as leaves
  float panOffset = (millis() / 50.0); // Use float for smoother movement
  
  for (int i = 0; i < numLeaves; i++) {
    // Calculate leaf position with smooth panning
    float leafPos = leafY[i] + panOffset;
    
    // Create multiple instances of each leaf for continuous scrolling
    for (int wrap = -1; wrap <= 1; wrap++) {
      int sy = (int)(leafPos + wrap * (numLeaves * spacing));
      
      // Only draw if on screen
      if (sy < -leafSize || sy > 128 + leafSize) continue;
      // Get vine X position at this Y coordinate
      int vx = 16 + sin(waveFreq * sy + phase) * amplitude;
      
      // Alternate the side of the vine for hearts
      int sign = (i % 2 == 0) ? 1 : -1; // Alternate left/right
      int hx = vx + sign * (leafSize / 2 + 2); // Reduced offset - closer to vine
      int hy = sy;

      // Optional: Remove or adjust this line if hearts disappear at edges
      // if (hx < leafSize/2 || hx > 32 - leafSize/2) continue;

      if (sign > 0) {
        // Heart pointing right (for right side)
        display.fillCircle(hx - leafSize/4, hy - leafSize/3, leafSize/4, SSD1306_WHITE);
        display.fillCircle(hx - leafSize/4, hy + leafSize/3, leafSize/4, SSD1306_WHITE);
        display.fillTriangle(hx - leafSize/3, hy - leafSize/2,
                             hx - leafSize/3, hy + leafSize/2,
                             hx + leafSize/3, hy, SSD1306_WHITE);
      } else {
        // Heart pointing left (for left side)
        display.fillCircle(hx + leafSize/4, hy - leafSize/3, leafSize/4, SSD1306_WHITE);
        display.fillCircle(hx + leafSize/4, hy + leafSize/3, leafSize/4, SSD1306_WHITE);
        display.fillTriangle(hx + leafSize/3, hy - leafSize/2,
                             hx + leafSize/3, hy + leafSize/2,
                             hx - leafSize/3, hy, SSD1306_WHITE);
      }
    }
  }

  // Update the phase for the sine wave
  phase += 0.05; // Phase increment for smooth movement
  if (phase > TWO_PI) {
    phase -= TWO_PI;
  }

  // Display the updated animation
  display.display();
}

#undef SCREEN_W
#undef SCREEN_H
