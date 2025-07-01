#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_AHTX0.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// ———— Pin definitions ————
#define SDA_PIN 5
#define SCL_PIN 6

// ———— OLED (72×40 ER, frame-buffer, HW-I2C) ————
U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// ———— AHT25 sensor ————
Adafruit_AHTX0 aht;

// ———— BLE UUIDs ————
#define SERVICE_UUID    "12345678-1234-5678-1234-56789abcdef0"
#define CTRL_CHAR_UUID  "12345678-1234-5678-1234-56789abcdef3"

// ———— State ————
bool useFahrenheit = false;

// ———— Forward declarations ————
void scanI2CAndDisplay();
void updateSensorDisplay(float tempC, float humPct);

// ———— BLE control characteristic ————
BLECharacteristic* ctrlChar;

// Callback that flips our unit-flag on any write
class ControlCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pChar) override {
    useFahrenheit = !useFahrenheit;
    Serial.printf("⟳ Toggled to %c\n", useFahrenheit ? 'F' : 'C');
  }
};

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n=== Booting ===");

  // I2C & OLED init
  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();

  // 1) Scan I²C bus and show found addresses
  scanI2CAndDisplay();

  // 2) Start AHT25
  Serial.println("Initializing AHT25…");
  if (!aht.begin()) {
    Serial.println("❌ AHT25 not found!");
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 0, "AHT25 ERROR");
    u8g2.drawStr(0, 12, "Sensor not found");
    u8g2.sendBuffer();
    while (1) delay(10);
  }
  Serial.println("✅ AHT25 ready");

  // 3) BLE GATT setup
  BLEDevice::init("ESP32_C3_Sensor");
  BLEServer* pServer = BLEDevice::createServer();
  BLEService* pService = pServer->createService(SERVICE_UUID);

  // Only a control characteristic (WRITE) to toggle units
  ctrlChar = pService->createCharacteristic(
    CTRL_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ  |
    BLECharacteristic::PROPERTY_WRITE
  );
  ctrlChar->setValue("OK");
  ctrlChar->addDescriptor(new BLE2902());
  ctrlChar->setCallbacks(new ControlCallbacks());

  pService->start();
  pServer->getAdvertising()->start();
  Serial.println("🔵 BLE advertising started");
}

void loop() {
  // Read sensor
  sensors_event_t humEvent, tmpEvent;
  aht.getEvent(&humEvent, &tmpEvent);
  float tempC  = tmpEvent.temperature;
  float humPct = humEvent.relative_humidity;

  // Update display (will convert units internally)
  updateSensorDisplay(tempC, humPct);

  delay(2000);  // respect sensor’s 1 s min
}

// ———— I²C Scanner + OLED display ————
void scanI2CAndDisplay() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.drawStr(0, 0, "I2C scan:");

  byte found = 0;
  char buf[8];
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf(" • 0x%02X\n", addr);
      uint8_t row = 1 + (found / 4);
      uint8_t col = (found % 4) * 16;
      snprintf(buf, sizeof(buf), "0x%02X", addr);
      u8g2.drawStr(col, row*10, buf);
      if (++found >= 8) break;
    }
  }
  if (found == 0) {
    u8g2.drawStr(0, 10, "None found");
    Serial.println(" • none");
  }
  u8g2.sendBuffer();
  delay(2000);
}

// ———— Draw temp & humidity ————
void updateSensorDisplay(float tempC, float humPct) {
  // Convert if needed
  float dispT = useFahrenheit ? (tempC * 9.0 / 5.0 + 32.0) : tempC;
  char unit = useFahrenheit ? 'F' : 'C';

  // Prepare lines
  char line1[16], line2[16];
  snprintf(line1, sizeof(line1), "T: %.1f %c", dispT, unit);
  snprintf(line2, sizeof(line2), "H: %.1f %%", humPct);

  // Render
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();

  u8g2.drawStr(0, 0,  "AHT25 Readings");
  u8g2.drawStr(0, 12, line1);
  u8g2.drawStr(0, 24, line2);

  // Humidity bar
  uint8_t barW = map((int)humPct, 0, 100, 0, 50);
  u8g2.drawFrame(10, 34, 52, 4);
  u8g2.drawBox(10, 34, barW, 4);

  u8g2.sendBuffer();
}
