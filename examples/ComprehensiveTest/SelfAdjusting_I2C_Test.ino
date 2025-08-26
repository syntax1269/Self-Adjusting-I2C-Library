/*
 * SelfAdjusting_I2C Library Example
 * 
 * This example demonstrates how to use the SelfAdjusting_I2C library
 * to automatically optimize I2C communication timing and handle errors.
 * 
 * Features demonstrated:
 * - Automatic timing optimization
 * - Error recovery mechanisms
 * - Device-specific configurations
 * - Performance monitoring
 * - Learning mode and adaptive behavior
 * 
 * Hardware Requirements:
 * - Arduino (Uno, Nano, ESP8266, ESP32, etc.)
 * - I2C devices (sensors, displays, etc.)
 * - Pull-up resistors on SDA/SCL lines (typically 4.7kΩ)
 * 
 * Connections:
 * - SDA: Pin A4 (Uno/Nano) or GPIO 21 (ESP32) or GPIO 4 (ESP8266)
 * - SCL: Pin A5 (Uno/Nano) or GPIO 22 (ESP32) or GPIO 5 (ESP8266)
 * 
 * Author: SelfAdjusting_I2C Library
 * Version: 1.0
 */

#include "SelfAdjusting_I2C.h"

// Example I2C device addresses (adjust for your devices)
const uint8_t DEVICE_1_ADDR = 0x48;  // Example: Temperature sensor
const uint8_t DEVICE_2_ADDR = 0x3C;  // Example: OLED display
const uint8_t DEVICE_3_ADDR = 0x68;  // Example: RTC module

// Timing variables
unsigned long lastDiagnostics = 0;
unsigned long lastDeviceRead = 0;
unsigned long lastOptimization = 0;

const unsigned long DIAGNOSTICS_INTERVAL = 30000;  // 30 seconds
const unsigned long DEVICE_READ_INTERVAL = 5000;   // 5 seconds
const unsigned long OPTIMIZATION_INTERVAL = 60000; // 1 minute

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000); // Wait for Serial on boards that need it
  
  Serial.println("\n=== SelfAdjusting_I2C Library Example ===");
  Serial.println("Initializing I2C with self-adjusting capabilities...");
  
  // Initialize the self-adjusting I2C library
  SmartWire.begin();
  
  // Configure library settings
  configureSelfAdjustingI2C();
  
  // Scan for I2C devices
  Serial.println("\nScanning I2C bus for devices...");
  SmartWire.scanAndOptimize();
  
  // Set device-specific configurations if needed
  setupDeviceSpecificConfigs();
  
  Serial.println("\nSetup complete. Starting main loop...");
  Serial.println("Monitor will show diagnostics every 30 seconds.");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Read from I2C devices periodically
  if (currentTime - lastDeviceRead >= DEVICE_READ_INTERVAL) {
    readI2CDevices();
    lastDeviceRead = currentTime;
  }
  
  // Force optimization periodically
  if (currentTime - lastOptimization >= OPTIMIZATION_INTERVAL) {
    Serial.println("\n--- Forcing Optimization ---");
    SmartWire.forceOptimization();
    lastOptimization = currentTime;
  }
  
  // Print diagnostics periodically
  if (currentTime - lastDiagnostics >= DIAGNOSTICS_INTERVAL) {
    printDetailedDiagnostics();
    lastDiagnostics = currentTime;
  }
  
  // Error handling is now automatic - no manual update needed
  
  // Small delay to prevent overwhelming the system
  delay(100);
}

void configureSelfAdjustingI2C() {
  Serial.println("Configuring SelfAdjusting_I2C settings...");
  
  // Enable all adaptive features
  SmartWire.enableLearning(true);
  SmartWire.enableAdaptiveMode(true);
  SmartWire.enableEmergencyRecovery(true);
  
  // Set adaptation rate (1-10, higher = more aggressive)
  SmartWire.setAdaptationRate(5);
  
  // Set cooldown period between adjustments (milliseconds)
  SmartWire.setCooldownPeriod(5000); // 5 seconds
  
  Serial.println("Configuration complete.");
}

void setupDeviceSpecificConfigs() {
  Serial.println("\nSetting up device-specific configurations...");
  
  // Example: Set slower speed for sensitive device
  // SmartWire.setDeviceSpecificConfig(DEVICE_1_ADDR, 50000, 300); // 50kHz, 300ns rise time
  
  // Example: Set faster speed for robust device
  // SmartWire.setDeviceSpecificConfig(DEVICE_2_ADDR, 400000, 65); // 400kHz, 65ns rise time
  
  Serial.println("Device-specific configurations set.");
}

void readI2CDevices() {
  Serial.print("Reading I2C devices... ");
  
  bool anySuccess = false;
  
  // Example device reads - replace with your actual device communication
  
  // Read from Device 1 (example: temperature sensor)
  if (readTemperatureSensor(DEVICE_1_ADDR)) {
    anySuccess = true;
  }
  
  // Read from Device 2 (example: OLED display)
  if (updateDisplay(DEVICE_2_ADDR)) {
    anySuccess = true;
  }
  
  // Read from Device 3 (example: RTC module)
  if (readRTC(DEVICE_3_ADDR)) {
    anySuccess = true;
  }
  
  if (anySuccess) {
    Serial.println("Success");
  } else {
    Serial.println("No devices responded");
  }
}

bool readTemperatureSensor(uint8_t address) {
  // Example temperature sensor read
  // Replace with actual sensor communication code
  
  SmartWire.beginTransmission(address);
  
  SmartWire.write(0x00); // Example register address
  uint8_t result = SmartWire.endTransmission(false); // Repeated start
  if (result != 0) return false;
  
  uint8_t bytesReceived = SmartWire.requestFrom(address, (uint8_t)2);
  if (bytesReceived != 2) return false;
  
  uint16_t rawTemp = (SmartWire.read() << 8) | SmartWire.read();
  float temperature = rawTemp * 0.0625; // Example conversion
  
  Serial.print("[Temp: ");
  Serial.print(temperature);
  Serial.print("°C] ");
  
  return true;
}

bool updateDisplay(uint8_t address) {
  // Example display update
  // Replace with actual display communication code
  
  SmartWire.beginTransmission(address);
  
  // Example: Send some data to display
  SmartWire.write(0x40); // Example command
  SmartWire.write(0x01); // Example data
  
  uint8_t result = SmartWire.endTransmission();
  return (result == 0);
}

bool readRTC(uint8_t address) {
  // Example RTC read
  // Replace with actual RTC communication code
  
  SmartWire.beginTransmission(address);
  
  SmartWire.write(0x00); // Start at seconds register
  uint8_t result = SmartWire.endTransmission(false);
  if (result != 0) return false;
  
  uint8_t bytesReceived = SmartWire.requestFrom(address, (uint8_t)7);
  if (bytesReceived != 7) return false;
  
  // Read time data (example)
  uint8_t seconds = SmartWire.read() & 0x7F;
  uint8_t minutes = SmartWire.read() & 0x7F;
  uint8_t hours = SmartWire.read() & 0x3F;
  SmartWire.read(); // Day of week
  uint8_t day = SmartWire.read() & 0x3F;
  uint8_t month = SmartWire.read() & 0x1F;
  uint8_t year = SmartWire.read();
  
  Serial.print("[Time: ");
  Serial.print(hours);
  Serial.print(":");
  Serial.print(minutes);
  Serial.print(":");
  Serial.print(seconds);
  Serial.print("] ");
  
  return true;
}

void printDetailedDiagnostics() {
  Serial.println("\n" + String('=', 50));
  Serial.println("DETAILED DIAGNOSTICS");
  Serial.println(String('=', 50));
  
  // Print main diagnostics
  SmartWire.printDiagnostics();
  
  // Print device-specific information
  Serial.println();
  SmartWire.printDeviceConfigs();
  
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
  
  Serial.print("Current Performance Score: ");
  Serial.println(SmartWire.getPerformanceScore());
  
  // Print current configuration
  Serial.println("\n=== Current Configuration ===");
  Serial.print("Clock Speed: ");
  Serial.print(SmartWire.getClockSpeed());
  Serial.println(" Hz");
  
  Serial.print("Rise Time: ");
  Serial.print(SmartWire.getRiseTime());
  Serial.println(" ns");
  
  Serial.print("Clock Speed Step: ");
  Serial.println(SmartWire.getCurrentClockSpeedStep());
  
  Serial.print("Rise Time Step: ");
  Serial.println(SmartWire.getCurrentRiseTimeStep());
  
  Serial.println(String('=', 50));
}

// Additional utility functions for testing

void testAllConfigurations() {
  Serial.println("\n=== Testing All Configurations ===");
  
  // Test each available clock speed
  uint32_t clockSpeeds[] = {100000, 400000, 1000000, 3400000};
  uint16_t riseTimes[] = {100, 250, 500, 1000};
  
  for (uint8_t i = 0; i < 4; i++) {
    for (uint8_t j = 0; j < 4; j++) {
      Serial.print("Testing: ");
      Serial.print(clockSpeeds[i]);
      Serial.print(" Hz, ");
      Serial.print(riseTimes[j]);
      Serial.print(" ns... ");
      
      SmartWire.setClockSpeed(clockSpeeds[i]);
      SmartWire.setRiseTime(riseTimes[j]);
      
      delay(100); // Let configuration settle
      
      // Test communication
      bool success = testCommunication();
      Serial.println(success ? "OK" : "FAIL");
      
      if (!success) {
        Serial.println("  Reverting to safe configuration...");
        SmartWire.resetToDefaults();
        break;
      }
    }
  }
  
  Serial.println("Configuration testing complete.");
}

bool testCommunication() {
  // Simple communication test
  uint8_t testAddresses[] = {DEVICE_1_ADDR, DEVICE_2_ADDR, DEVICE_3_ADDR};
  uint8_t successCount = 0;
  
  for (uint8_t i = 0; i < 3; i++) {
    SmartWire.beginTransmission(testAddresses[i]);
    uint8_t result = SmartWire.endTransmission();
    if (result == 0) {
      successCount++;
    }
  }
  
  return (successCount > 0); // At least one device responded
}

void demonstrateErrorRecovery() {
  Serial.println("\n=== Demonstrating Error Recovery ===");
  
  // Force an error condition by trying invalid address
  Serial.println("Attempting communication with invalid address...");
  
  for (uint8_t i = 0; i < 5; i++) {
    SmartWire.beginTransmission(0x7F); // Invalid address
    uint8_t result = SmartWire.endTransmission();
    
    Serial.print("Attempt ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(result == 0 ? "Success" : "Failed");
    
    // Library automatically handles errors during endTransmission()
    delay(1000);
  }
  
  Serial.println("Error recovery demonstration complete.");
  Serial.println("Library should have adjusted settings automatically.");
}

// Serial command interface for interactive testing
void handleSerialCommands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();
    
    if (command == "help") {
      printHelp();
    } else if (command == "scan") {
      SmartWire.scanAndOptimize();
    } else if (command == "reset") {
      SmartWire.resetToDefaults();
      Serial.println("Reset to defaults.");
    } else if (command == "optimize") {
      SmartWire.forceOptimization();
      Serial.println("Forced optimization.");
    } else if (command == "test") {
      testAllConfigurations();
    } else if (command == "error") {
      demonstrateErrorRecovery();
    } else if (command == "diag") {
      printDetailedDiagnostics();
    } else if (command.startsWith("speed ")) {
      uint32_t speed = command.substring(6).toInt();
      SmartWire.setClockSpeed(speed);
      Serial.print("Clock speed set to: ");
      Serial.println(speed);
    } else if (command.startsWith("rise ")) {
      uint16_t rise = command.substring(5).toInt();
      SmartWire.setRiseTime(rise);
      Serial.print("Rise time set to: ");
      Serial.println(rise);
    } else if (command != "") {
      Serial.println("Unknown command. Type 'help' for available commands.");
    }
  }
}

void printHelp() {
  Serial.println("\n=== Available Commands ===");
  Serial.println("help     - Show this help");
  Serial.println("scan     - Scan I2C bus and optimize");
  Serial.println("reset    - Reset to default configuration");
  Serial.println("optimize - Force optimization");
  Serial.println("test     - Test all configurations");
  Serial.println("error    - Demonstrate error recovery");
  Serial.println("diag     - Show detailed diagnostics");
  Serial.println("speed X  - Set clock speed to X Hz");
  Serial.println("rise X   - Set rise time to X ns");
  Serial.println("=========================");
}

/*
 * Additional Notes:
 * 
 * 1. Replace the example device communication functions with your actual
 *    device-specific code.
 * 
 * 2. Adjust device addresses to match your hardware setup.
 * 
 * 3. The library automatically optimizes I2C timing based on performance
 *    metrics and error rates.
 * 
 * 4. Use Serial commands for interactive testing and configuration.
 * 
 * 5. Monitor the diagnostics output to understand how the library is
 *    adapting to your specific hardware configuration.
 * 
 * 6. For production use, you may want to disable some of the verbose
 *    diagnostic output.
 */