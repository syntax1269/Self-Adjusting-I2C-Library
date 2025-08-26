/*
 * SelfAdjusting_I2C Memory Usage Test
 * 
 * This minimal sketch tests only the basic library functions:
 * - SmartWire.begin()
 * - SmartWire.write()
 * - SmartWire.requestFrom()
 * 
 * Used to measure baseline memory consumption across platforms.
 */

#include "SelfAdjusting_I2C.h"

void setup() {
  Serial.begin(115200);
  
  // Initialize the library
  SmartWire.begin();
  
  Serial.println("SelfAdjusting_I2C Memory Test");
  Serial.println("Library initialized successfully");
}

void loop() {
  // Basic I2C operations for memory testing
  uint8_t testAddress = 0x48;
  
  // Test write operation
  SmartWire.beginTransmission(testAddress);
  SmartWire.write(0x00);
  SmartWire.endTransmission();
  
  // Test read operation
  SmartWire.requestFrom(testAddress, (uint8_t)1);
  if (SmartWire.available()) {
    SmartWire.read();
  }
  
  delay(1000);
}