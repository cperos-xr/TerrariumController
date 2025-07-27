
# TerrariumController V1

**Purpose:**  
This modular Arduino-based controller manages a self-contained terrarium environment using an AHT25 sensor, OLED display, and digital actuators for light and water. It's designed to cycle through sensor readings, visual animations, and display modes in a visually appealing, low-power format ideal for embedded or remote installations.

**Core Features:**
- Displays temperature and humidity from AHT25 sensor.
- Runs a 0.91" I2C OLED screen in **portrait mode (rotated 90°)**
- Modular animation system including:
  - Vine with animated hearts and data leaves.
  - Umbrellas with floating raindrops.
  - Blooming flower with spinning petals.
  - Scrolling personalized message
- Animations are full-screen effects between info screens for user engagement, but might still require implementation.
- Scrolling screen for weekly/daily high/low stats.
- Real-time water and light control using digital pins.
- Frame timing and display refresh loop maintained separately from sensor reads and actuation logic.

**Optional Features:**
- Certain Hardware can be added or removed or added like lights, pump and a fogger
- RTC is required for timekeeping and scheduling.
- Without the component use backup logic or feature remains inactive

**About the OLED:**
- Runs a 0.91" I2C OLED screen in **portrait mode (rotated 90°)**
- Display is 128 x 32 px
- Words must be short or abbreviated maybe 5 char max on font size 1
- Example of 90 degree usage on DigitalClock.cpp DisplayManager.cpp:
  ```
    display.setCursor(0, 0); // Top left of screen
    display.print("Temp");
    display.setCursor(0, 8);
    display.print(String((int)tempF) + "F");  // Next Line
    display.setCursor(0, 16);  // Next line
    display.print("Humid");   
    display.setCursor(0, 24); // Next Line
    display.setCursor(8, 98); // indented text
    //...
    display.setCursor(0, 128); //Bottom left
  ```
**Updated Rule for Formatting on the Tall Narrow Screen:**
1. **Character Limit Per Row:**
   - **Large Font (`setTextSize(2`)**: Max **2 characters** per row. No exceptions.
   - **Small Font (`setTextSize(1`)**: Max **5-6 characters** per row.

2. **No Labels in Large Font:**
   - Large font rows are **data-only** (e.g., `11`, `00`). Labels like "Hr" or "Min" are **not allowed** in large font rows.

3. **Break Data into Rows:**
   - Each piece of data (hour, minute, second, etc.) gets its **own row**. Example:
     ```
     11
     00
     45
     ```

4. **Vertical Spacing:**
   - Use `setCursor` to space rows **16 pixels apart** for large font rows to avoid overlap.

5. **No Horizontal Alignment:**
   - Keep everything **left-aligned**. No attempts to fit multiple items on the same row.

6. **Compact Layout for Small Font:**
   - Use small font for additional data (e.g., month, day, year) if needed, but keep it **simple and readable**.

**Notes for Development:**
- Screen is rotated 90 degrees to fit narrow OLED vertically. Layout logic accounts for this.
- `Adafruit_GFX` + `Adafruit_SSD1306` used for drawing primitives and text.
- Timing is critical: animations render at ~30 FPS; sensor reads and actuations happen on a separate timer.
- All display functions are abstracted so new screens/modes can easily be added.
- Animations are intended to be full-screen effects between info screens for user engagement.

**Legacy Scripts:**
The following legacy scripts serve as working examples for functionality that has been integrated into the modular project:
- **`ScrollingViewLegacy.cpp`**:
  - Implements a scrolling view for daily and weekly high/low stats.
  - Displays current temperature and humidity at the top.
  - Tracks daily and weekly highs and lows for temperature and humidity.
  - Includes warnings for values outside desired limits.

- **`AHTVineLegacy.cpp`**:
  - Combines sensor readings with a vine animation.
  - Displays animated hearts and data leaves on a sine wave pattern.
  - Integrates temperature and humidity stats into the animation.

- **`AnimationCycleLegacy.cpp`**:
  - Cycles through multiple animations, including:
    - Vine with hearts and data leaves.
    - Umbrellas with gentle rain.
    - Blooming flower with spinning petals.
  - Includes timing logic for smooth transitions between modes.

**Suggestions for Extension:**
- Add Wi-Fi messaging or time sync with ESP32.
- Remote-triggered custom OLED messages.
- Multi-sensor support.
- Cap touch or button controls for mode switching.

**Migrating Legacy Scripts Into A Streamlined Project:**
- No longer using legacy scripts, but they ARE independently working examples for what we would like to achieve.
- Mirror the legacy behavior and functionality, but bring it into our new streamlined modular project.
- Again these are working so let's keep the functionality and behavior the same in the new scripts.
- Imitate the exact functionality but make it modular.


- Always refer to yourself as my "AI computer buddy"
- Talk like rick sanchez when explaining things

**Main.ino = New Primary project**
- Independent program
- this is a new Barebones 2.0 with no frills or animations just working proof of concept
- RTC is mandatory in this one as we will always need to keep track of time for the plant care schedule.
- const int PIN_LIGHT = 4; // Define light pin
- const int PIN_WATER = 3; 
- We all we do is set water on/off and light on/off (EX: DAILY,10,30,3000)
- we can change these values with bluetooth
- this is better and replaces the older code found in the TerrariumController subdirectory
- using serial communication for debugging and status updates

**Fogger integration**
- Fogger is different than lights and water pump
- Fogger is activated by a button press and has its own circuit board that drives the fogger
- We are electronically pressing this button with an npn transistor, so a short pulse will activate the fogger
- Fogger has 3 modes: 
  - OFF
  - ON
  - Inttermittent
- Three modes are controlled by a single button, so first press is for on, second press goes to intermittent, third press goes to off, and the cycle repeats
- Keep this pattern in mind when implementing the fogger control logic

