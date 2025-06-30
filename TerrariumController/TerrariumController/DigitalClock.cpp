#include "DigitalClock.h"
#include "RTCManager.h" // Include RTCManager for the `rtc` object
#include "DisplayManager.h" // Include DisplayManager for the `display` object

void drawDigitalClock() {
    DateTime now = rtc.getCurrentTime(); // Use the public method to get the current time

    // Clear the display
    display.clearDisplay();

    char hourBuf[4]; // HH
    sprintf(hourBuf, "%02d", now.hour()); // Format hour as two digits

    char minuteBuf[3]; // MM
    sprintf(minuteBuf, "%02d", now.minute()); // Format minute as two digits

    char secondBuf[3]; // SS
    sprintf(secondBuf, "%02d", now.second()); // Format second as two digits

    char monthBuf[10]; // Month name
    sprintf(monthBuf, "June"); // Hardcoded for now, replace with dynamic month name if needed

    char monthDigitBuf[3]; // Month as two-digit number
    sprintf(monthDigitBuf, "%02d", now.month());

    char dayBuf[3]; // Day of the month
    sprintf(dayBuf, "%02d", now.day());

    char yearBuf[5]; // Year
    sprintf(yearBuf, "%04d", now.year());

    // Display the time and date
    display.setTextSize(2); // Set text size for big text
    display.setTextColor(SSD1306_WHITE); // Set text color

    // Row 1: Time
    display.setCursor(0, 0); // Top row
    display.println(hourBuf); // Print time

    // Row 2: Date
    display.setCursor(0, 24); // Second row
    display.println(minuteBuf); // Print date

    display.setCursor(0, 48); // Second row, right side
    display.println(secondBuf); // Print second
    

    // Row 3: Month
    display.setTextSize(1); // Smaller text for month
    display.setCursor(8, 98); // Third row
    display.println(monthDigitBuf); // Print month

    // Row 4: Day
    display.setCursor(8, 106); // Fourth row
    display.println(dayBuf); // Print day

    // Row 5: Year
    display.setCursor(0, 114); // Fifth row
    display.println(yearBuf); // Print year

    // Update the display
    display.display();
}