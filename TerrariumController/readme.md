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
- Scrolling screen for weekly/daily high/low stats.
- Real-time water and light control using digital pins.
- Frame timing and display refresh loop maintained separately from sensor reads and actuation logic.

**File Structure:**
- `TerrariumController.ino`: Main control loop and mode scheduler.
- `SensorManager.*`: Sensor read/update logic (AHT25).
- `DisplayManager.*`: Handles screen setup and rendering logic.
- `ActuatorControl.*`: Controls GPIO for water and light pumps.
- `Animations.*`: Fully implemented animation functions (vine, umbrella, bloom, etc.).

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