# SelfAdjusting_I2C Library

A comprehensive Arduino library that enhances the standard Wire.h library with intelligent, self-adjusting I2C timing optimization, error recovery, and performance monitoring.

## Features

### 🧠 Intelligent Optimization
- **Mini AI Algorithm**: Automatically determines optimal I2C settings during runtime
- **Adaptive Timing**: Self-adjusting `WIRE_RISE_TIME_NANOSECONDS` and `Wire.setClock` values
- **Performance Learning**: Continuously learns and adapts to your specific hardware configuration
- **Device-Specific Optimization**: Individual settings for different I2C devices

### 🛡️ Error Recovery
- **Emergency Recovery**: Automatic fallback to safe configurations on errors
- **Incremental Recovery**: Gradual adjustment when errors occur
- **Hardware Reset**: Complete I2C bus reset capability
- **Error Pattern Analysis**: Learns from error patterns to prevent future issues

### 📊 Performance Monitoring
- **Real-time Metrics**: Success rates, transaction times, error rates
- **Performance Scoring**: Quantitative performance evaluation
- **Trend Analysis**: Historical performance tracking
- **Diagnostic Tools**: Comprehensive debugging and monitoring

### ⚙️ Advanced Configuration
- **Multiple Clock Speeds**: 100kHz to 3.4MHz support
- **Variable Rise Times**: 50ns to 300ns options (optimized for 3.3V systems)
- **Learning Modes**: Adaptive behavior configuration
- **Cooldown Periods**: Configurable adjustment intervals

## Installation

1. Download or clone this repository
2. Copy the `Self_Adjusing_i2c` folder to your Arduino libraries directory:
   - **Windows**: `Documents\Arduino\libraries\`
   - **macOS**: `~/Documents/Arduino/libraries/`
   - **Linux**: `~/Arduino/libraries/`
3. Restart the Arduino IDE
4. Include the library in your sketch: `#include "SelfAdjusting_I2C.h"`

## Quick Start

```cpp
#include "SelfAdjusting_I2C.h"

void setup() {
  Serial.begin(115200);
  
  // Initialize the self-adjusting I2C library
  SmartWire.begin();
  
  // Enable adaptive features
  SmartWire.enableLearningMode(true);
  SmartWire.enableAdaptiveMode(true);
  
  // Scan and optimize for connected devices
  SmartWire.scanAndOptimize();
}

void loop() {
  // Use SmartWire exactly like Wire
  SmartWire.beginTransmission(0x48);
  SmartWire.write(0x00);
  uint8_t result = SmartWire.endTransmission();
  
  // Let the library optimize in the background
  SmartWire.update();
  
  delay(1000);
}
```

## API Reference

### Core Functions

#### `void begin()`
Initializes the self-adjusting I2C library with default settings.

#### `void update()`
Performs background optimization and monitoring. Call this regularly in your main loop.

#### `uint8_t beginTransmission(uint8_t address)`
Starts I2C transmission to specified device. Returns error code.

#### `uint8_t endTransmission(bool sendStop = true)`
Ends I2C transmission. Returns error code (0 = success).

#### `size_t write(uint8_t data)`
Writes a byte to the I2C bus.

#### `uint8_t requestFrom(uint8_t address, uint8_t quantity, bool sendStop = true)`
Requests data from I2C device.

#### `int read()`
Reads a byte from the I2C bus.

### Configuration Functions

#### `void enableLearningMode(bool enable)`
Enables/disables learning mode for automatic optimization.

#### `void enableAdaptiveMode(bool enable)`
Enables/disables adaptive behavior based on performance metrics.

#### `void setAdaptationRate(uint8_t rate)`
Sets adaptation aggressiveness (1-10, higher = more aggressive).

#### `void setClockSpeed(uint32_t clockSpeed)`
Manually sets I2C clock speed in Hz.

#### `void setRiseTime(uint16_t riseTimeNs)`
Manually sets rise time in nanoseconds.

#### `void setCooldownPeriod(uint32_t milliseconds)`
Sets minimum time between automatic adjustments.

### Device Management

#### `void scanAndOptimize()`
Scans I2C bus for devices and optimizes settings for all found devices.

#### `void setDeviceSpecificConfig(uint8_t address, uint32_t clockSpeed, uint16_t riseTime)`
Sets custom configuration for a specific device.

#### `void removeDeviceConfig(uint8_t address)`
Removes device-specific configuration.

### Recovery Functions

#### `void forceOptimization()`
Forces immediate optimization regardless of cooldown period.

#### `void resetToDefaults()`
Resets all settings to conservative defaults.

#### `void resetLearning()`
Clears learning history while keeping current configuration.

#### `void enableEmergencyRecovery(bool enable)`
Enables/disables automatic emergency recovery on errors.

### Monitoring Functions

#### `I2CPerformanceMetrics getMetrics()`
Returns current performance metrics.

#### `float getPerformanceScore()`
Returns current performance score (0-100).

#### `uint32_t getCurrentClockSpeed()`
Returns current I2C clock speed.

#### `uint16_t getCurrentRiseTime()`
Returns current rise time setting.

#### `void printDiagnostics()`
Prints comprehensive diagnostic information to Serial.

#### `void printDeviceConfigs()`
Prints device-specific configurations to Serial.

## Performance Metrics

The library tracks several key performance indicators:

- **Success Rate**: Percentage of successful I2C transactions
- **Error Rate**: Percentage of failed transactions
- **Average Transaction Time**: Mean time per I2C operation
- **Performance Score**: Composite score (0-100) based on all metrics
- **Consecutive Errors**: Number of consecutive failed operations

## Supported Clock Speeds

- **100 kHz** - Standard mode (default)
- **400 kHz** - Fast mode
- **1 MHz** - Fast mode plus
- **3.4 MHz** - High-speed mode

## Supported Rise Times

- **50 ns** - Maximum performance for 2kΩ resistors or lower
- **65 ns** - Very aggressive for 2.4kΩ resistors
- **75 ns** - Aggressive for 2.7kΩ resistors
- **80 ns** - Optimal default for most 3.3V systems
- **120 ns** - Moderate setting
- **300 ns** - Conservative rise time for legacy 5V systems or noisy environments

## Error Codes

- `0` - Success
- `1` - Data too long for transmit buffer
- `2` - Received NACK on transmit of address
- `3` - Received NACK on transmit of data
- `4` - Other error
- `5` - Timeout

## Hardware Compatibility

### Tested Platforms
- Arduino Uno/Nano (ATmega328P)
- Arduino Mega (ATmega2560)
- ESP8266
- ESP32
- Arduino Pro Mini

### Requirements
- Pull-up resistors on SDA/SCL lines (typically 4.7kΩ for 5v systems)
- Pull-up resistors on SDA/SCL lines (typically 2.4kΩ for 3.3v systems)
- Stable power supply
- Proper I2C device connections

## Advanced Usage

### Custom Device Configuration

```cpp
// Set slower speed for sensitive sensor
SmartWire.setDeviceSpecificConfig(0x48, 100000, 300); // 100kHz, 300ns

// Set faster speed for robust display
SmartWire.setDeviceSpecificConfig(0x3C, 400000, 80);  // 400kHz, 80ns
```

### Performance Monitoring

```cpp
void checkPerformance() {
  I2CPerformanceMetrics metrics = SmartWire.getMetrics();
  
  Serial.print("Success Rate: ");
  Serial.print(100.0 * metrics.successfulTransactions / 
              (metrics.successfulTransactions + metrics.failedTransactions));
  Serial.println("%");
  
  Serial.print("Performance Score: ");
  Serial.println(SmartWire.getPerformanceScore());
}
```

### Error Recovery Configuration

```cpp
void setupErrorRecovery() {
  SmartWire.enableEmergencyRecovery(true);
  SmartWire.setCooldownPeriod(3000);  // 3 second cooldown
  SmartWire.setAdaptationRate(7);     // Aggressive adaptation
}
```

## Troubleshooting

### Common Issues

1. **No devices found during scan**
   - Check wiring connections
   - Verify pull-up resistors are installed
   - Ensure devices are powered

2. **High error rates**
   - Check for electrical noise
   - Verify cable lengths are appropriate
   - Consider lower clock speeds

3. **Frequent resets to defaults**
   - Increase cooldown period
   - Reduce adaptation rate
   - Check for intermittent connections

### Debug Mode

Enable verbose diagnostics:

```cpp
void setup() {
  Serial.begin(115200);
  SmartWire.begin();
  
  // Print diagnostics every 10 seconds
  // (implement in your main loop)
}

void loop() {
  static unsigned long lastDiag = 0;
  if (millis() - lastDiag > 10000) {
    SmartWire.printDiagnostics();
    lastDiag = millis();
  }
  
  SmartWire.update();
}
```

## Memory Usage

The library is designed to be memory-efficient. Below are the measured memory consumption values for basic library usage (SmartWire.begin(), write(), and requestFrom() calls) across different Arduino platforms:

| Platform | Program Storage | Dynamic Memory | IRAM | Flash |
|----------|----------------|----------------|------|-------|
| **ESP8266** | 242,708 bytes (23%) | 30,092 bytes (37%) | 60,423 bytes (92%) | 1,048,576 bytes total |
| **ESP32** | 324,507 bytes (24%) | 23,304 bytes (7%) | - | 1,310,720 bytes total |
| **Arduino Uno** | 9,762 bytes (30%) | 1,409 bytes (68%) | - | 32,256 bytes total |
| **Arduino Nano** | 9,762 bytes (30%) | 1,409 bytes (68%) | - | 32,256 bytes total |
| **Arduino Mega** | 10,496 bytes (4%) | 1,409 bytes (17%) | - | 258,048 bytes total |
| **Arduino Pro Mini** | 9,762 bytes (31%) | 1,409 bytes (68%) | - | 32,256 bytes total |

### Memory Usage Notes:
- **ESP8266**: High IRAM usage (92%) due to platform requirements, but still functional
- **ESP32**: Excellent memory efficiency with moderate program storage (24%) and low dynamic memory usage (7%)
- **Arduino Uno/Nano/Pro Mini**: Moderate program storage (30-31%) and high dynamic memory usage (68%)
- **Arduino Mega**: Low program storage usage (4%) with plenty of memory available
- **Maximum Devices**: 8 (configurable)
- **Additional RAM**: ~200-300 bytes for performance tracking (depending on device count)

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## License

This library is released under the MIT License. See LICENSE file for details.

## Changelog

### Version 1.0.0
- Initial release
- Basic self-adjusting functionality
- Error recovery mechanisms
- Performance monitoring
- Device-specific configurations

## Support

For support, please:
1. Check the troubleshooting section
2. Review the example code
3. Open an issue on GitHub

## Acknowledgments

- Built upon Arduino's Wire library
- Inspired by adaptive control systems
- Community feedback and testing

---

**Note**: This library enhances but does not replace the standard Wire library. It provides additional intelligence and error recovery while maintaining full compatibility with existing I2C code.