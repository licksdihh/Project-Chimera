#include <Arduino.h>
#include <Wire.h>
#include <ArduinoJson.h>

// --- Sensor Pins ---
#define MQ135_PIN A0
#define MQ2_PIN   A1

// --- Thresholds ---
#define MQ135_THRESHOLD 400
#define MQ2_THRESHOLD   350
#define TOF_THRESHOLD   300 // mm

// --- I2C Addresses ---
#define DRV2605_ADDR 0x5A
#define TOF_ADDR     0x29

// --- Globals ---
unsigned long lastSend = 0;
bool alertTriggered = false;

// --- DRV2605L Haptic Trigger ---
void triggerHaptic() {
  Wire.beginTransmission(DRV2605_ADDR);
  Wire.write(0x01); // Mode register
  Wire.write(0x00); // Internal trigger
  Wire.endTransmission();

  Wire.beginTransmission(DRV2605_ADDR);
  Wire.write(0x04); // GO register
  Wire.write(0x01); // Start vibration
  Wire.endTransmission();
}

// --- Fake ToF Read (Replace with VL53L1X lib if available) ---
uint16_t readToF() {
  // Placeholder (replace with real driver)
  return random(100, 600);
}

// --- Setup ---
void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(MQ135_PIN, INPUT);
  pinMode(MQ2_PIN, INPUT);

  // Init DRV2605L
  Wire.beginTransmission(DRV2605_ADDR);
  Wire.write(0x01);
  Wire.write(0x00);
  Wire.endTransmission();
}

// --- Loop ---
void loop() {
  int mq135 = analogRead(MQ135_PIN);
  int mq2   = analogRead(MQ2_PIN);
  uint16_t distance = readToF();

  // --- Hazard Detection ---
  if (mq135 > MQ135_THRESHOLD || mq2 > MQ2_THRESHOLD || distance < TOF_THRESHOLD) {
    if (!alertTriggered) {
      triggerHaptic();
      alertTriggered = true;
    }
  } else {
    alertTriggered = false;
  }

  // --- Send JSON every 100ms ---
  if (millis() - lastSend >= 100) {
    lastSend = millis();

    StaticJsonDocument<256> doc;
    doc["mq135"] = mq135;
    doc["mq2"] = mq2;
    doc["distance"] = distance;
    doc["alert"] = alertTriggered;

    String output;
    serializeJson(doc, output);

    // RouterBridge → stdout (picked by MPU)
    Serial.println(output);
  }
}
