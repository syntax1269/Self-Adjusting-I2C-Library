/*
 * ESP8266 Test Sketch for SelfAdjusting_I2C Library
 * 
 * This sketch tests the compilation of the SelfAdjusting_I2C library
 * on ESP8266 platform and verifies basic functionality.
 * 
 * Hardware: ESP8266 (NodeMCU, Wemos D1 Mini, etc.)
 * I2C Pins: GPIO 4 (SDA), GPIO 5 (SCL)
 */

#include "SelfAdjusting_I2C.h"

// Test I2C device addresses
const uint8_t TEST_DEVICE_1 = 0x48;  // Example: Temperature sensor
const uint8_t TEST_DEVICE_2 = 0x3C;  // Example: OLED display

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println();
  Serial.println("=== ESP8266 SelfAdjusting_I2C Test ===");
  Serial.println("Initializing I2C with self-adjusting capabilities...");
  
  // Initialize the self-adjusting I2C library
  SmartWire.begin();
  
  // Configure for ESP8266 (3.3V system)
  SmartWire.enableLearning(true);
  SmartWire.enableAdaptiveMode(true);
  SmartWire.enableEmergencyRecovery(true);
  
  // Set conservative adaptation rate for ESP8266
  SmartWire.setAdaptationRate(3);
  
  // Set longer cooldown for ESP8266
  SmartWire.setCooldownPeriod(10000); // 10 seconds
  
  Serial.println("Configuration complete.");
  
  // Scan for I2C devices
  Serial.println("\nScanning I2C bus...");
  SmartWire.scanAndOptimize();
  
  Serial.println("\nSetup complete. Starting main loop...");
}

void loop() {
  static unsigned long lastTest = 0;
  static unsigned long lastDiag = 0;
  
  unsigned long currentTime = millis();
  
  // Test I2C communication every 5 seconds
  if (currentTime - lastTest >= 5000) {
    testI2CCommunication();
    lastTest = currentTime;
  }
  
  // Print diagnostics every 30 seconds
  if (currentTime - lastDiag >= 30000) {
    printDiagnostics();
    lastDiag = currentTime;
  }
  
  // Small delay to prevent overwhelming the system
  delay(100);
}

void testI2CCommunication() {
  Serial.print("Testing I2C communication... ");
  
  bool anySuccess = false;
  
  // Test communication with example devices
  if (testDevice(TEST_DEVICE_1)) {
    anySuccess = true;
    Serial.print("[0x");
    Serial.print(TEST_DEVICE_1, HEX);
    Serial.print(" OK] ");
  }
  
  if (testDevice(TEST_DEVICE_2)) {
    anySuccess = true;
    Serial.print("[0x");
    Serial.print(TEST_DEVICE_2, HEX);
    Serial.print(" OK] ");
  }
  
  if (anySuccess) {
    Serial.println("Success");
  } else {
    Serial.println("No devices responded");
  }
}

bool testDevice(uint8_t address) {
  SmartWire.beginTransmission(address);
  uint8_t result = SmartWire.endTransmission();
  return (result == 0);
}

void printDiagnostics() {
  Serial.println("\n" + String('=', 40));
  Serial.println("ESP8266 I2C DIAGNOSTICS");
  Serial.println(String('=', 40));
  
  // Print main diagnostics
  SmartWire.printDiagnostics();
  
  // Print performance metrics
  I2CPerformanceMetrics metrics = SmartWire.getMetrics();
  Serial.println("\n=== Performance Summary ===");
  Serial.print("Total Transactions: ");
  Serial.println(metrics.successfulTransactions + metrics.failedTransactions);
  
  if (metrics.successfulTransactions > 0) {
    Serial.print("Success Rate: ");
    Serial.print(100.0 * metrics.successfulTransactions / 
                (metrics.successfulTransactions + metrics.failedTransactions));
    Serial.println("%");
  }
  
  Serial.print("Performance Score: ");
  Serial.println(SmartWire.getPerformanceScore());
  
  // Print current configuration
  Serial.println("\n=== Current Configuration ===");
  Serial.print("Clock Speed: ");
  Serial.print(SmartWire.getClockSpeed());
  Serial.println(" Hz");
  
  Serial.print("Rise Time: ");
  Serial.print(SmartWire.getRiseTime());
  Serial.println(" ns");
  
  // ESP8266 specific information
  Serial.println("\n=== ESP8266 Info ===");
  Serial.print("Free Heap: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
  
  Serial.print("Chip ID: 0x");
  Serial.println(ESP.getChipId(), HEX);
  
  Serial.print("CPU Frequency: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");
  
  Serial.println(String('=', 40));
}