#include "DigitalClock.h"
#include "RTCManager.h" // Include RTCManager for the `rtc` object
#include "DisplayManager.h" // Include DisplayManager for the `display` object
#include "Fonts/DSEG14/DSEG14Classic_Regular8pt7b.h" // Include the segmented LED font

void drawDigitalClock() {
    DateTime now = rtc.getCurrentTime(); // Get current time

    // Clear the display
    display.clearDisplay();

    // Format time and date
    char hourBuf[3]; // HH
    sprintf(hourBuf, "%02d", now.hour());

    char minuteBuf[3]; // MM
    sprintf(minuteBuf, "%02d", now.minute());

    char secondBuf[3]; // SS
    sprintf(secondBuf, "%02d", now.second());

    char monthBuf[3]; // MM
    sprintf(monthBuf, "%02d", now.month());

    char dayBuf[3]; // DD
    sprintf(dayBuf, "%02d", now.day());

    char yearBuf[5]; // YYYY
    sprintf(yearBuf, "%04d", now.year());

    // Set the segmented LED font
    display.setFont(&DSEG14Classic_Regular8pt7b);
    display.setTextColor(SSD1306_WHITE); // White text

    // Display the time
    display.setTextSize(1); // Text size is controlled by the font
    display.setCursor(0, 16); // Top row
    display.println(hourBuf);

    display.setCursor(0, 48); // Middle row
    display.println(minuteBuf);

    display.setCursor(0, 80); // Bottom row
    display.println(secondBuf);

    // Display the date
    display.setFont(NULL); // Reset to default font for the date
    display.setTextSize(1); // Smaller text for date
    display.setCursor(8, 104); // Sixth row
    display.println(monthBuf);

    display.setCursor(8, 112); // Seventh row
    display.println(dayBuf);

    display.setCursor(0, 120); // Eighth row
    display.println(yearBuf);

    // Update the display
    display.display();
}