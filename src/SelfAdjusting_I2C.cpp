#include "SelfAdjusting_I2C.h"
#include <math.h>

// Global instance definition
SelfAdjustingI2C SmartWire;

// Implementation of remaining functions

void SelfAdjustingI2C::forceOptimization() {
    // Force immediate optimization regardless of cooldown
    lastAdjustmentTime = 0;
    
    if (currentConfig.metrics.successfulTransactions > 0) {
        AIDecision decision = analyzePerformanceAndDecide();
        if (decision.shouldAdjust) {
            applyAIDecision(decision);
        }
    }
}

void SelfAdjustingI2C::resetToDefaults() {
    // Reset to conservative defaults
    currentConfig.clockSpeedStep = 0; // Start at minimum safe speed
    currentConfig.riseTimeStep = DYNAMIC_RANGE_STEPS / 2; // Start at middle (125ns default)
    updateDynamicRange(clockSpeedRange, currentConfig.clockSpeedStep);
    updateDynamicRange(riseTimeRange, currentConfig.riseTimeStep);
    currentConfig.clockSpeed = clockSpeedRange.current_value;
    currentConfig.riseTime = riseTimeRange.current_value;
    currentConfig.isValid = true;
    
    // Clear metrics
    memset(&currentConfig.metrics, 0, sizeof(I2CPerformanceMetrics));
    currentConfig.metrics.lastUpdateTime = millis();
    
    // Reset state variables
    consecutiveErrors = 0;
    lastAdjustmentTime = 0;
    lastErrorTime = 0;
    adjustmentCooldown = 5000;
    learningMode = true;
    emergencyRecovery = true;
    adaptiveMode = true;
    performanceScore = 0.0;
    trendAnalysis = 0.0;
    adaptationRate = 5;
    lastError = ERROR_NONE;
    errorHistoryIndex = 0;
    
    // Clear error history
    memset(errorHistory, 0, sizeof(errorHistory));
    
    // Apply configuration
    applyConfiguration();
    
    // Copy current as best
    bestConfig = currentConfig;
}

void SelfAdjustingI2C::resetLearning() {
    // Clear performance history
    memset(performanceHistory, 0, sizeof(performanceHistory));
    historyIndex = 0;
    
    // Reset metrics but keep current configuration
    memset(&currentConfig.metrics, 0, sizeof(I2CPerformanceMetrics));
    currentConfig.metrics.lastUpdateTime = millis();
    
    // Reset AI variables
    performanceScore = 0.0;
    trendAnalysis = 0.0;
    consecutiveErrors = 0;
    lastAdjustmentTime = 0;
    
    // Clear error history
    memset(errorHistory, 0, sizeof(errorHistory));
    errorHistoryIndex = 0;
}

void SelfAdjustingI2C::setClockSpeed(uint32_t clockSpeed) {
    uint8_t step = calculateStepFromValue(clockSpeedRange, clockSpeed);
    if (isStepValid(step)) {
        currentConfig.clockSpeedStep = step;
        updateDynamicRange(clockSpeedRange, step);
        currentConfig.clockSpeed = clockSpeedRange.current_value;
        applyConfiguration();
    }
}

void SelfAdjustingI2C::setRiseTime(uint16_t riseTimeNs) {
    uint8_t step = calculateStepFromValue(riseTimeRange, riseTimeNs);
    if (isStepValid(step)) {
        currentConfig.riseTimeStep = step;
        updateDynamicRange(riseTimeRange, step);
        currentConfig.riseTime = riseTimeRange.current_value;
        applyConfiguration();
    }
}

I2CPerformanceMetrics SelfAdjustingI2C::getDeviceMetrics(uint8_t address) const {
    DeviceConfig* deviceConfig = const_cast<SelfAdjustingI2C*>(this)->findDeviceConfig(address);
    if (deviceConfig != nullptr) {
        return deviceConfig->config.metrics;
    }
    
    // Return empty metrics if device not found
    I2CPerformanceMetrics emptyMetrics;
    memset(&emptyMetrics, 0, sizeof(I2CPerformanceMetrics));
    return emptyMetrics;
}

void SelfAdjustingI2C::scanAndOptimize() {
    // Scan I2C bus for devices
    uint8_t devicesFound = scanBus();
    
    if (devicesFound == 0) {
        return; // No devices to optimize for
    }
    
    // Test different configurations and find the best overall performance
    I2CConfig originalConfig = currentConfig;
    I2CConfig bestOverallConfig = currentConfig;
    float bestOverallScore = 0.0;
    
    // Test each step combination for optimal performance
    for (uint8_t clockStep = 0; clockStep < DYNAMIC_RANGE_STEPS; clockStep++) {
        for (uint8_t riseStep = 0; riseStep < DYNAMIC_RANGE_STEPS; riseStep++) {
            if (testConfiguration(clockStep, riseStep)) {
                float score = calculatePerformanceScore(currentConfig.metrics);
                if (score > bestOverallScore) {
                    bestOverallScore = score;
                    bestOverallConfig = currentConfig;
                }
            }
        }
    }
    
    // Apply best configuration found
    currentConfig = bestOverallConfig;
    applyConfiguration();
    saveCurrentAsBest();
}

void SelfAdjustingI2C::setDeviceSpecificConfig(uint8_t address, uint32_t clockSpeed, uint16_t riseTime) {
    DeviceConfig* deviceConfig = findDeviceConfig(address);
    if (deviceConfig == nullptr) {
        addDeviceConfig(address);
        deviceConfig = findDeviceConfig(address);
    }
    
    if (deviceConfig != nullptr) {
        uint8_t clockStep = calculateStepFromValue(clockSpeedRange, clockSpeed);
        uint8_t riseStep = calculateStepFromValue(riseTimeRange, riseTime);
        
        if (isStepValid(clockStep) && isStepValid(riseStep)) {
            deviceConfig->config.clockSpeedStep = clockStep;
            deviceConfig->config.riseTimeStep = riseStep;
            updateDynamicRange(clockSpeedRange, clockStep);
            updateDynamicRange(riseTimeRange, riseStep);
            deviceConfig->config.clockSpeed = clockSpeedRange.current_value;
            deviceConfig->config.riseTime = riseTimeRange.current_value;
            deviceConfig->hasCustomConfig = true;
        }
    }
}

void SelfAdjustingI2C::removeDeviceConfig(uint8_t address) {
    for (uint8_t i = 0; i < deviceCount; i++) {
        if (deviceConfigs[i].address == address) {
            // Shift remaining configs down
            for (uint8_t j = i; j < deviceCount - 1; j++) {
                deviceConfigs[j] = deviceConfigs[j + 1];
            }
            deviceCount--;
            break;
        }
    }
}

void SelfAdjustingI2C::enableEmergencyRecovery(bool enable) {
    emergencyRecovery = enable;
}

void SelfAdjustingI2C::setCooldownPeriod(uint32_t milliseconds) {
    adjustmentCooldown = milliseconds;
}

void SelfAdjustingI2C::printDiagnostics() const {
    Serial.println("=== SelfAdjustingI2C Diagnostics ===");
    Serial.print("Current Clock Speed: ");
    Serial.print(currentConfig.clockSpeed);
    Serial.println(" Hz");
    
    Serial.print("Current Rise Time: ");
    Serial.print(currentConfig.riseTime);
    Serial.println(" ns");
    
    Serial.print("Performance Score: ");
    Serial.println(performanceScore);
    
    Serial.print("Successful Transactions: ");
    Serial.println(currentConfig.metrics.successfulTransactions);
    
    Serial.print("Failed Transactions: ");
    Serial.println(currentConfig.metrics.failedTransactions);
    
    Serial.print("Error Rate: ");
    Serial.print(currentConfig.metrics.errorRate);
    Serial.println("%");
    
    Serial.print("Average Transaction Time: ");
    Serial.print(currentConfig.metrics.averageTransactionTime);
    Serial.println(" μs");
    
    Serial.print("Consecutive Errors: ");
    Serial.println(consecutiveErrors);
    
    Serial.print("Learning Mode: ");
    Serial.println(learningMode ? "Enabled" : "Disabled");
    
    Serial.print("Adaptive Mode: ");
    Serial.println(adaptiveMode ? "Enabled" : "Disabled");
    
    Serial.print("Emergency Recovery: ");
    Serial.println(emergencyRecovery ? "Enabled" : "Disabled");
    
    Serial.print("Recovery Mode: ");
    Serial.println(isInRecoveryMode() ? "Active" : "Inactive");
    
    Serial.print("Last Error: ");
    Serial.println(getLastErrorString());
    
    Serial.print("Adaptation Rate: ");
    Serial.println(adaptationRate);
    
    Serial.print("Device Count: ");
    Serial.println(deviceCount);
    
    Serial.println("=================================");
}

void SelfAdjustingI2C::printDeviceConfigs() const {
    Serial.println("=== Device Configurations ===");
    
    for (uint8_t i = 0; i < deviceCount; i++) {
        Serial.print("Device 0x");
        Serial.print(deviceConfigs[i].address, HEX);
        Serial.print(": ");
        
        if (deviceConfigs[i].hasCustomConfig) {
            Serial.print(deviceConfigs[i].config.clockSpeed);
            Serial.print(" Hz, ");
            Serial.print(deviceConfigs[i].config.riseTime);
            Serial.print(" ns");
        } else {
            Serial.print("Using global config");
        }
        
        Serial.print(" | Success: ");
        Serial.print(deviceConfigs[i].config.metrics.successfulTransactions);
        Serial.print(", Fail: ");
        Serial.println(deviceConfigs[i].config.metrics.failedTransactions);
    }
    
    Serial.println("=============================");
}

bool SelfAdjustingI2C::testConfiguration(uint8_t clockStep, uint8_t riseStep) {
    if (!isStepValid(clockStep) || !isStepValid(riseStep)) {
        return false;
    }
    
    // Save current configuration
    I2CConfig originalConfig = currentConfig;
    
    // Apply test configuration
    currentConfig.clockSpeedStep = clockStep;
    currentConfig.riseTimeStep = riseStep;
    currentConfig.clockSpeed = calculateValueFromStep(clockSpeedRange, clockStep);
    currentConfig.riseTime = calculateValueFromStep(riseTimeRange, riseStep);
    
    // Clear metrics for test
    memset(&currentConfig.metrics, 0, sizeof(I2CPerformanceMetrics));
    currentConfig.metrics.lastUpdateTime = millis();
    
    applyConfiguration();
    
    // Perform test transactions with known devices
    bool testPassed = true;
    uint8_t testErrors = 0;
    
    // Test with each known device
    for (uint8_t i = 0; i < deviceCount && testPassed; i++) {
        uint8_t address = deviceConfigs[i].address;
        
        // Simple ping test - try to start transmission
        Wire.beginTransmission(address);
        uint8_t result = Wire.endTransmission();
        
        if (result != 0) {
            testErrors++;
            if (testErrors > 2) { // Allow some tolerance
                testPassed = false;
            }
        } else {
            currentConfig.metrics.successfulTransactions++;
        }
        
        currentConfig.metrics.failedTransactions += testErrors;
    }
    
    // If no devices to test with, assume configuration is valid
    if (deviceCount == 0) {
        testPassed = true;
    }
    
    // Restore original configuration if test failed
    if (!testPassed) {
        currentConfig = originalConfig;
        applyConfiguration();
    }
    
    return testPassed;
}

uint8_t SelfAdjustingI2C::scanBus() {
    uint8_t devicesFound = 0;
    
    Serial.println("Scanning I2C bus...");
    
    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();
        
        if (error == 0) {
            Serial.print("Device found at address 0x");
            if (address < 16) Serial.print("0");
            Serial.println(address, HEX);
            
            // Add to device list if not already present
            if (findDeviceConfig(address) == nullptr) {
                addDeviceConfig(address);
            }
            
            devicesFound++;
        }
    }
    
    Serial.print("Scan complete. Found ");
    Serial.print(devicesFound);
    Serial.println(" devices.");
    
    return devicesFound;
}

float SelfAdjustingI2C::analyzeTrend() {
    if (historyIndex < 3) {
        return 0.0; // Not enough data for trend analysis
    }
    
    uint8_t samples = min(historyIndex, LEARNING_WINDOW_SIZE);
    float trend = 0.0;
    
    // Simple linear trend analysis
    for (uint8_t i = 1; i < samples; i++) {
        float currentScore = calculatePerformanceScore(performanceHistory[i]);
        float previousScore = calculatePerformanceScore(performanceHistory[i-1]);
        trend += (currentScore - previousScore);
    }
    
    return trend / (samples - 1);
}

void SelfAdjustingI2C::applyDeviceConfiguration(uint8_t address) {
    DeviceConfig* deviceConfig = findDeviceConfig(address);
    
    if (deviceConfig != nullptr && deviceConfig->hasCustomConfig) {
        // Apply device-specific configuration
        if (currentConfig.clockSpeed != deviceConfig->config.clockSpeed ||
            currentConfig.riseTime != deviceConfig->config.riseTime) {
            
            currentConfig.clockSpeedStep = deviceConfig->config.clockSpeedStep;
            currentConfig.riseTimeStep = deviceConfig->config.riseTimeStep;
            currentConfig.clockSpeed = deviceConfig->config.clockSpeed;
            currentConfig.riseTime = deviceConfig->config.riseTime;
            
            applyConfiguration();
        }
    }
}

void SelfAdjustingI2C::incrementalRecovery() {
    // Simple incremental recovery - reduce speed by one step
    if (currentConfig.clockSpeedStep > 0) {
        currentConfig.clockSpeedStep--;
        updateDynamicRange(clockSpeedRange, currentConfig.clockSpeedStep);
        currentConfig.clockSpeed = clockSpeedRange.current_value;
        applyConfiguration();
    }
    
    consecutiveErrors = 0;
}

uint32_t SelfAdjustingI2C::measureTransactionTime() {
    // This is a placeholder - actual implementation would measure
    // the time for a specific I2C transaction
    return micros(); // Return current time as placeholder
}

// These functions are replaced by calculateStepFromValue in the dynamic range system
// uint8_t findClockSpeedIndex and uint8_t findRiseTimeIndex are no longer needed

float SelfAdjustingI2C::calculateDeviceCompatibilityScore(uint8_t address) {
    DeviceConfig* deviceConfig = findDeviceConfig(address);
    
    if (deviceConfig == nullptr) {
        return 50.0; // Neutral score for unknown devices
    }
    
    return calculatePerformanceScore(deviceConfig->config.metrics);
}

void SelfAdjustingI2C::resetHardware() {
    // Reset I2C hardware to default state
    Wire.end();
    delay(10);
    Wire.begin();
    
    // Apply current configuration
    applyConfiguration();
}

void SelfAdjustingI2C::shiftPerformanceHistory() {
    // Shift performance history to make room for new entry
    if (historyIndex >= LEARNING_WINDOW_SIZE) {
        for (uint8_t i = 0; i < LEARNING_WINDOW_SIZE - 1; i++) {
            performanceHistory[i] = performanceHistory[i + 1];
        }
        historyIndex = LEARNING_WINDOW_SIZE - 1;
    }
    
    // Add current metrics to history
    performanceHistory[historyIndex] = currentConfig.metrics;
    historyIndex++;
}