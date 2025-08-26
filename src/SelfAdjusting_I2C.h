#ifndef SELF_ADJUSTING_I2C_H
#define SELF_ADJUSTING_I2C_H

#include <Wire.h>
#include <stdint.h>
#include <Arduino.h>

// Configuration constants
#define LEARNING_WINDOW_SIZE 10
#define ERROR_THRESHOLD 3
#define PERFORMANCE_SAMPLES 5
#define DEFAULT_TIMEOUT_MS 100
#define MAX_DEVICES 16
#define DYNAMIC_RANGE_STEPS 20  // Number of steps in dynamic ranges

// Dynamic range configuration for self-adapting I2C parameters
struct DynamicRange {
    uint32_t min_value;
    uint32_t max_value;
    uint32_t current_value;
    uint32_t default_value;
    uint8_t current_step;     // Current position in range (0 to DYNAMIC_RANGE_STEPS-1)
    uint8_t optimal_step;     // Best performing step found so far
    float step_size;          // Calculated step size for this range
};

// Self-adapting clock speed range: 75kHz to 3.5MHz
static DynamicRange clockSpeedRange = {
    75000,      // min_value: 75kHz safety minimum
    3500000,    // max_value: 3.5MHz maximum
    100000,     // current_value: 100kHz starting default
    100000,     // default_value: 100kHz safe default
    1,          // current_step: start near minimum for safety
    1,          // optimal_step: initially same as current
    0.0         // step_size: calculated dynamically
};

// Self-adapting rise time range: 40ns to 250ns
static DynamicRange riseTimeRange = {
    40,         // min_value: 40ns minimum (aggressive)
    250,        // max_value: 250ns maximum (conservative)
    125,        // current_value: 125ns Wire.h default
    125,        // default_value: 125ns Wire.h default
    8,          // current_step: middle of range for 125ns default
    8,          // optimal_step: initially same as current
    0.0         // step_size: calculated dynamically
};

// Performance metrics structure
struct I2CPerformanceMetrics {
    uint32_t successfulTransactions;
    uint32_t failedTransactions;
    uint32_t totalTransactionTime;
    uint32_t averageTransactionTime;
    uint8_t errorRate;
    uint8_t stabilityScore;
    uint32_t lastUpdateTime;
};

// Configuration state
struct I2CConfig {
    uint8_t clockSpeedStep;   // Current step in clock speed range
    uint8_t riseTimeStep;     // Current step in rise time range
    uint32_t clockSpeed;      // Current clock speed value
    uint16_t riseTime;        // Current rise time value
    I2CPerformanceMetrics metrics;
    bool isValid;
};

// Device-specific configuration
struct DeviceConfig {
    uint8_t address;
    I2CConfig config;
    bool hasCustomConfig;
};

// Mini AI decision structure
struct AIDecision {
    int8_t clockSpeedDelta;  // -1, 0, +1
    int8_t riseTimeDelta;    // -1, 0, +1
    uint8_t confidence;      // 0-100
    bool shouldAdjust;
    const char* reason;      // Reason for decision
};

// Error types
enum I2CErrorType {
    ERROR_NONE = 0,
    ERROR_TIMEOUT = 1,
    ERROR_NACK_ADDRESS = 2,
    ERROR_NACK_DATA = 3,
    ERROR_OTHER = 4
};

class SelfAdjustingI2C {
private:
    I2CConfig currentConfig;
    I2CConfig bestConfig;
    I2CPerformanceMetrics performanceHistory[LEARNING_WINDOW_SIZE];
    DeviceConfig deviceConfigs[MAX_DEVICES];
    uint8_t historyIndex;
    uint8_t consecutiveErrors;
    uint8_t deviceCount;
    uint32_t lastAdjustmentTime;
    uint32_t adjustmentCooldown;
    uint32_t lastErrorTime;
    bool learningMode;
    bool emergencyRecovery;
    bool adaptiveMode;
    
    // Mini AI variables
    float performanceScore;
    float trendAnalysis;
    uint8_t adaptationRate;
    uint8_t currentDeviceAddress;
    
    // Error tracking
    I2CErrorType lastError;
    uint8_t errorHistory[10];
    uint8_t errorHistoryIndex;
    
public:
    SelfAdjustingI2C();
    
    // Core functionality
    void begin();
    void begin(uint8_t address);
    void end();
    
    // Enhanced I2C operations with auto-optimization
    uint8_t requestFrom(uint8_t address, uint8_t quantity);
    uint8_t requestFrom(uint8_t address, uint8_t quantity, uint8_t stop);
    void beginTransmission(uint8_t address);
    uint8_t endTransmission();
    uint8_t endTransmission(uint8_t stop);
    
    // Write operations
    size_t write(uint8_t data);
    size_t write(const uint8_t *data, size_t length);
    
    // Read operations
    int available();
    int read();
    int peek();
    void flush();
    
    // Self-adjustment and AI functions
    void enableLearning(bool enable = true);
    void enableAdaptiveMode(bool enable = true);
    void setAdaptationRate(uint8_t rate); // 1-10 (1=conservative, 10=aggressive)
    void forceOptimization();
    void resetToDefaults();
    void resetLearning();
    
    // Configuration and monitoring
    void setClockSpeed(uint32_t clockSpeed);
    void setRiseTime(uint16_t riseTimeNs);
    uint32_t getClockSpeed() const;
    uint16_t getRiseTime() const;
    uint8_t getCurrentClockSpeedStep() const;
    uint8_t getCurrentRiseTimeStep() const;
    I2CPerformanceMetrics getMetrics() const;
    I2CPerformanceMetrics getDeviceMetrics(uint8_t address) const;
    float getPerformanceScore() const;
    bool isInRecoveryMode() const;
    const char* getLastErrorString() const;
    
    // Advanced features
    void scanAndOptimize(); // Scan all devices and optimize for best overall performance
    void setDeviceSpecificConfig(uint8_t address, uint32_t clockSpeed, uint16_t riseTime);
    void removeDeviceConfig(uint8_t address);
    void enableEmergencyRecovery(bool enable = true);
    void setCooldownPeriod(uint32_t milliseconds);
    
    // Diagnostic functions
    void printDiagnostics() const;
    void printDeviceConfigs() const;
    bool testConfiguration(uint8_t clockStep, uint8_t riseStep);
    uint8_t scanBus(); // Returns number of devices found
    
private:
    // Core AI and optimization functions
    void updatePerformanceMetrics(bool success, uint32_t transactionTime, uint8_t deviceAddress);
    AIDecision analyzePerformanceAndDecide();
    void applyAIDecision(const AIDecision& decision);
    float calculatePerformanceScore(const I2CPerformanceMetrics& metrics);
    float analyzeTrend();
    
    // Configuration management
    void applyConfiguration();
    void applyDeviceConfiguration(uint8_t address);
    bool isStepValid(uint8_t step);
    void saveCurrentAsBest();
    void restoreBestConfiguration();
    DeviceConfig* findDeviceConfig(uint8_t address);
    void addDeviceConfig(uint8_t address);
    
    // Error handling and recovery
    void handleError(I2CErrorType errorType);
    void emergencyRecoveryProcedure();
    void incrementalRecovery();
    void adaptiveRecovery();
    I2CErrorType classifyError(uint8_t wireError);
    
    // Dynamic range management functions
    void initializeDynamicRanges();
    uint32_t calculateValueFromStep(const DynamicRange& range, uint8_t step);
    uint8_t calculateStepFromValue(const DynamicRange& range, uint32_t value);
    void updateDynamicRange(DynamicRange& range, uint8_t newStep);
    void optimizeDynamicRanges();
    
    // Utility functions
    uint32_t measureTransactionTime();
    
    // Mini AI helper functions
    float calculateStabilityScore();
    float calculateEfficiencyScore();
    float calculateReliabilityScore();
    bool shouldTriggerAdjustment();
    float calculateDeviceCompatibilityScore(uint8_t address);
    
    // Hardware abstraction
    void setHardwareRiseTime(uint16_t riseTimeNs);
    void setHardwareClockSpeed(uint32_t clockSpeed);
    void resetHardware();
    
    // Memory management helpers
    void shiftPerformanceHistory();
    void updateErrorHistory(I2CErrorType error);
    float getRecentErrorRate();
};

// Global instance (similar to Wire)
extern SelfAdjustingI2C SmartWire;

// Implementation of key functions
inline SelfAdjustingI2C::SelfAdjustingI2C() {
    // Initialize dynamic ranges
    initializeDynamicRanges();
    
    // Initialize with safe defaults
    currentConfig.clockSpeedStep = clockSpeedRange.current_step;
    currentConfig.riseTimeStep = riseTimeRange.current_step;
    currentConfig.clockSpeed = clockSpeedRange.current_value;
    currentConfig.riseTime = riseTimeRange.current_value;
    currentConfig.isValid = true;
    
    // Initialize metrics
    memset(&currentConfig.metrics, 0, sizeof(I2CPerformanceMetrics));
    memset(&bestConfig, 0, sizeof(I2CConfig));
    memset(performanceHistory, 0, sizeof(performanceHistory));
    memset(deviceConfigs, 0, sizeof(deviceConfigs));
    memset(errorHistory, 0, sizeof(errorHistory));
    
    historyIndex = 0;
    consecutiveErrors = 0;
    deviceCount = 0;
    lastAdjustmentTime = 0;
    lastErrorTime = 0;
    adjustmentCooldown = 5000; // 5 seconds
    learningMode = true;
    emergencyRecovery = true;
    adaptiveMode = true;
    performanceScore = 0.0;
    trendAnalysis = 0.0;
    adaptationRate = 5; // Medium adaptation rate
    currentDeviceAddress = 0;
    lastError = ERROR_NONE;
    errorHistoryIndex = 0;
    
    // Copy current as best initially
    bestConfig = currentConfig;
}

inline void SelfAdjustingI2C::begin() {
    Wire.begin();
    applyConfiguration();
    
    // Initialize performance tracking
    currentConfig.metrics.lastUpdateTime = millis();
}

inline void SelfAdjustingI2C::begin(uint8_t address) {
    Wire.begin(address);
    applyConfiguration();
    
    // Initialize performance tracking
    currentConfig.metrics.lastUpdateTime = millis();
}

inline void SelfAdjustingI2C::end() {
    Wire.end();
}

inline uint8_t SelfAdjustingI2C::requestFrom(uint8_t address, uint8_t quantity) {
    currentDeviceAddress = address;
    
    // Apply device-specific configuration if available
    if (adaptiveMode) {
        applyDeviceConfiguration(address);
    }
    
    uint32_t startTime = micros();
    uint8_t result = Wire.requestFrom(address, quantity);
    uint32_t transactionTime = micros() - startTime;
    
    updatePerformanceMetrics(result > 0, transactionTime, address);
    
    if (result == 0) {
        handleError(ERROR_TIMEOUT);
    } else if (learningMode && shouldTriggerAdjustment()) {
        AIDecision decision = analyzePerformanceAndDecide();
        if (decision.shouldAdjust) {
            applyAIDecision(decision);
        }
    }
    
    return result;
}

inline uint8_t SelfAdjustingI2C::requestFrom(uint8_t address, uint8_t quantity, uint8_t stop) {
    currentDeviceAddress = address;
    
    // Apply device-specific configuration if available
    if (adaptiveMode) {
        applyDeviceConfiguration(address);
    }
    
    uint32_t startTime = micros();
    uint8_t result = Wire.requestFrom(address, quantity, stop);
    uint32_t transactionTime = micros() - startTime;
    
    updatePerformanceMetrics(result > 0, transactionTime, address);
    
    if (result == 0) {
        handleError(ERROR_TIMEOUT);
    } else if (learningMode && shouldTriggerAdjustment()) {
        AIDecision decision = analyzePerformanceAndDecide();
        if (decision.shouldAdjust) {
            applyAIDecision(decision);
        }
    }
    
    return result;
}

inline void SelfAdjustingI2C::beginTransmission(uint8_t address) {
    currentDeviceAddress = address;
    
    // Apply device-specific configuration if available
    if (adaptiveMode) {
        applyDeviceConfiguration(address);
    }
    
    Wire.beginTransmission(address);
}

inline uint8_t SelfAdjustingI2C::endTransmission() {
    uint32_t startTime = micros();
    uint8_t result = Wire.endTransmission();
    uint32_t transactionTime = micros() - startTime;
    
    updatePerformanceMetrics(result == 0, transactionTime, currentDeviceAddress);
    
    if (result != 0) {
        handleError(classifyError(result));
    } else if (learningMode && shouldTriggerAdjustment()) {
        AIDecision decision = analyzePerformanceAndDecide();
        if (decision.shouldAdjust) {
            applyAIDecision(decision);
        }
    }
    
    return result;
}

inline uint8_t SelfAdjustingI2C::endTransmission(uint8_t stop) {
    uint32_t startTime = micros();
    uint8_t result = Wire.endTransmission(stop);
    uint32_t transactionTime = micros() - startTime;
    
    updatePerformanceMetrics(result == 0, transactionTime, currentDeviceAddress);
    
    if (result != 0) {
        handleError(classifyError(result));
    } else if (learningMode && shouldTriggerAdjustment()) {
        AIDecision decision = analyzePerformanceAndDecide();
        if (decision.shouldAdjust) {
            applyAIDecision(decision);
        }
    }
    
    return result;
}

inline size_t SelfAdjustingI2C::write(uint8_t data) {
    return Wire.write(data);
}

inline size_t SelfAdjustingI2C::write(const uint8_t *data, size_t length) {
    return Wire.write(data, length);
}

inline int SelfAdjustingI2C::available() {
    return Wire.available();
}

inline int SelfAdjustingI2C::read() {
    return Wire.read();
}

inline int SelfAdjustingI2C::peek() {
    return Wire.peek();
}

inline void SelfAdjustingI2C::flush() {
    Wire.flush();
}

// AI and optimization implementation
inline void SelfAdjustingI2C::updatePerformanceMetrics(bool success, uint32_t transactionTime, uint8_t deviceAddress) {
    // Update global metrics
    if (success) {
        currentConfig.metrics.successfulTransactions++;
        currentConfig.metrics.totalTransactionTime += transactionTime;
        consecutiveErrors = 0;
    } else {
        currentConfig.metrics.failedTransactions++;
        consecutiveErrors++;
    }
    
    // Update device-specific metrics if adaptive mode is enabled
    if (adaptiveMode) {
        DeviceConfig* deviceConfig = findDeviceConfig(deviceAddress);
        if (deviceConfig == nullptr) {
            addDeviceConfig(deviceAddress);
            deviceConfig = findDeviceConfig(deviceAddress);
        }
        
        if (deviceConfig != nullptr) {
            if (success) {
                deviceConfig->config.metrics.successfulTransactions++;
                deviceConfig->config.metrics.totalTransactionTime += transactionTime;
            } else {
                deviceConfig->config.metrics.failedTransactions++;
            }
        }
    }
    
    uint32_t totalTransactions = currentConfig.metrics.successfulTransactions + 
                                currentConfig.metrics.failedTransactions;
    
    if (totalTransactions > 0) {
        currentConfig.metrics.errorRate = 
            (currentConfig.metrics.failedTransactions * 100) / totalTransactions;
        
        if (currentConfig.metrics.successfulTransactions > 0) {
            currentConfig.metrics.averageTransactionTime = 
                currentConfig.metrics.totalTransactionTime / 
                currentConfig.metrics.successfulTransactions;
        }
    }
    
    currentConfig.metrics.lastUpdateTime = millis();
    
    // Update performance score
    performanceScore = calculatePerformanceScore(currentConfig.metrics);
}

inline AIDecision SelfAdjustingI2C::analyzePerformanceAndDecide() {
    AIDecision decision = {0, 0, 0, false, "No adjustment needed"};
    
    // Don't adjust too frequently
    if (millis() - lastAdjustmentTime < adjustmentCooldown) {
        decision.reason = "Cooldown period active";
        return decision;
    }
    
    float currentScore = calculatePerformanceScore(currentConfig.metrics);
    float bestScore = calculatePerformanceScore(bestConfig.metrics);
    float trend = analyzeTrend();
    float recentErrorRate = getRecentErrorRate();
    
    // AI decision logic based on multiple factors
    if (recentErrorRate > 10.0) {
        // High recent error rate - reduce speed for stability
        decision.clockSpeedDelta = -1;
        decision.riseTimeDelta = 1; // Increase rise time for stability
        decision.confidence = 85;
        decision.shouldAdjust = true;
        decision.reason = "High error rate detected";
    } else if (currentConfig.metrics.errorRate == 0 && 
               currentConfig.metrics.successfulTransactions > 20) {
        // No errors and good sample size - try to optimize
        if (currentScore > bestScore * 1.15) {
            // Current config is significantly better
            saveCurrentAsBest();
            decision.reason = "New best configuration found";
        }
        
        if (trend > 0.2 && adaptationRate > 6) {
            // Strong positive trend and aggressive mode - try faster
            decision.clockSpeedDelta = 1;
            decision.riseTimeDelta = -1;
            decision.confidence = 70;
            decision.shouldAdjust = true;
            decision.reason = "Positive trend, optimizing speed";
        } else if (trend > 0.1 && adaptationRate > 3) {
            // Moderate positive trend - conservative optimization
            decision.clockSpeedDelta = 1;
            decision.riseTimeDelta = 0;
            decision.confidence = 60;
            decision.shouldAdjust = true;
            decision.reason = "Moderate optimization";
        }
    } else if (currentScore < bestScore * 0.7) {
        // Current performance is significantly worse
        decision.clockSpeedDelta = 0;
        decision.riseTimeDelta = 0;
        decision.confidence = 95;
        decision.shouldAdjust = false;
        decision.reason = "Restoring best configuration";
        // Restore best configuration instead
        restoreBestConfiguration();
    } else if (consecutiveErrors >= 2) {
        // Multiple consecutive errors - be more conservative
        decision.clockSpeedDelta = -1;
        decision.riseTimeDelta = 1;
        decision.confidence = 80;
        decision.shouldAdjust = true;
        decision.reason = "Consecutive errors, reducing speed";
    }
    
    return decision;
}

inline void SelfAdjustingI2C::applyAIDecision(const AIDecision& decision) {
    uint8_t newClockStep = currentConfig.clockSpeedStep;
    uint8_t newRiseStep = currentConfig.riseTimeStep;
    
    // Apply clock speed delta
    if (decision.clockSpeedDelta > 0 && newClockStep < DYNAMIC_RANGE_STEPS - 1) {
        newClockStep++;
    } else if (decision.clockSpeedDelta < 0 && newClockStep > 0) {
        newClockStep--;
    }
    
    // Apply rise time delta
    if (decision.riseTimeDelta > 0 && newRiseStep < DYNAMIC_RANGE_STEPS - 1) {
        newRiseStep++;
    } else if (decision.riseTimeDelta < 0 && newRiseStep > 0) {
        newRiseStep--;
    }
    
    // Validate and apply new configuration
    if (isStepValid(newClockStep) && isStepValid(newRiseStep)) {
        currentConfig.clockSpeedStep = newClockStep;
        currentConfig.riseTimeStep = newRiseStep;
        
        // Update dynamic ranges and calculate new values
        updateDynamicRange(clockSpeedRange, newClockStep);
        updateDynamicRange(riseTimeRange, newRiseStep);
        
        currentConfig.clockSpeed = clockSpeedRange.current_value;
        currentConfig.riseTime = riseTimeRange.current_value;
        
        applyConfiguration();
        lastAdjustmentTime = millis();
        
        // Reset metrics for new configuration
        memset(&currentConfig.metrics, 0, sizeof(I2CPerformanceMetrics));
        currentConfig.metrics.lastUpdateTime = millis();
    }
}

inline float SelfAdjustingI2C::calculatePerformanceScore(const I2CPerformanceMetrics& metrics) {
    if (metrics.successfulTransactions == 0) return 0.0;
    
    float reliabilityScore = calculateReliabilityScore();
    float efficiencyScore = calculateEfficiencyScore();
    float stabilityScore = calculateStabilityScore();
    
    // Weighted combination - reliability is most important
    return (reliabilityScore * 0.6) + (efficiencyScore * 0.25) + (stabilityScore * 0.15);
}

inline float SelfAdjustingI2C::calculateReliabilityScore() {
    uint32_t total = currentConfig.metrics.successfulTransactions + 
                    currentConfig.metrics.failedTransactions;
    if (total == 0) return 0.0;
    
    return (float)currentConfig.metrics.successfulTransactions / total * 100.0;
}

inline float SelfAdjustingI2C::calculateEfficiencyScore() {
    if (currentConfig.metrics.averageTransactionTime == 0) return 0.0;
    
    // Lower transaction time = higher score, with diminishing returns
    float baseTime = 1000.0; // 1ms baseline
    float normalizedTime = currentConfig.metrics.averageTransactionTime / baseTime;
    return max(0.0, 100.0 / (1.0 + normalizedTime));
}

inline float SelfAdjustingI2C::calculateStabilityScore() {
    // Calculate variance in recent performance
    if (historyIndex < 3) return 50.0; // Not enough data
    
    float variance = 0.0;
    float mean = 0.0;
    uint8_t samples = min(historyIndex, LEARNING_WINDOW_SIZE);
    
    // Calculate mean
    for (uint8_t i = 0; i < samples; i++) {
        mean += calculatePerformanceScore(performanceHistory[i]);
    }
    mean /= samples;
    
    // Calculate variance
    for (uint8_t i = 0; i < samples; i++) {
        float score = calculatePerformanceScore(performanceHistory[i]);
        variance += (score - mean) * (score - mean);
    }
    variance /= samples;
    
    // Lower variance = higher stability score
    return max(0.0, 100.0 - sqrt(variance));
}

inline void SelfAdjustingI2C::handleError(I2CErrorType errorType) {
    lastError = errorType;
    lastErrorTime = millis();
    updateErrorHistory(errorType);
    
    if (consecutiveErrors >= ERROR_THRESHOLD) {
        if (emergencyRecovery) {
            emergencyRecoveryProcedure();
        } else if (adaptiveMode) {
            adaptiveRecovery();
        } else {
            incrementalRecovery();
        }
    }
}

inline void SelfAdjustingI2C::emergencyRecoveryProcedure() {
    // Reset to most conservative settings
    currentConfig.clockSpeedStep = 0; // Minimum clock speed (75kHz)
    currentConfig.riseTimeStep = DYNAMIC_RANGE_STEPS - 1; // Maximum rise time (250ns)
    
    // Update dynamic ranges to conservative values
    updateDynamicRange(clockSpeedRange, 0);
    updateDynamicRange(riseTimeRange, DYNAMIC_RANGE_STEPS - 1);
    
    currentConfig.clockSpeed = clockSpeedRange.current_value;
    currentConfig.riseTime = riseTimeRange.current_value;
    
    applyConfiguration();
    consecutiveErrors = 0;
    
    // Temporarily disable learning
    learningMode = false;
    
    // Re-enable learning after extended cooldown
    lastAdjustmentTime = millis();
    adjustmentCooldown = 15000; // 15 seconds
}

inline void SelfAdjustingI2C::adaptiveRecovery() {
    // Analyze error pattern and adjust accordingly
    float recentErrorRate = getRecentErrorRate();
    
    if (recentErrorRate > 20.0) {
        // Very high error rate - emergency recovery
        emergencyRecoveryProcedure();
    } else {
        // Moderate error rate - incremental adjustment
        if (currentConfig.clockSpeedStep > 0) {
            currentConfig.clockSpeedStep--;
            updateDynamicRange(clockSpeedRange, currentConfig.clockSpeedStep);
            currentConfig.clockSpeed = clockSpeedRange.current_value;
        }
        
        if (currentConfig.riseTimeStep < DYNAMIC_RANGE_STEPS - 1) {
            currentConfig.riseTimeStep++;
            updateDynamicRange(riseTimeRange, currentConfig.riseTimeStep);
            currentConfig.riseTime = riseTimeRange.current_value;
        }
        
        applyConfiguration();
        consecutiveErrors = 0;
    }
}

inline I2CErrorType SelfAdjustingI2C::classifyError(uint8_t wireError) {
    switch (wireError) {
        case 1: return ERROR_TIMEOUT;
        case 2: return ERROR_NACK_ADDRESS;
        case 3: return ERROR_NACK_DATA;
        case 4: return ERROR_OTHER;
        default: return ERROR_NONE;
    }
}

inline void SelfAdjustingI2C::applyConfiguration() {
    setHardwareClockSpeed(currentConfig.clockSpeed);
    setHardwareRiseTime(currentConfig.riseTime);
}

inline void SelfAdjustingI2C::setHardwareClockSpeed(uint32_t clockSpeed) {
    Wire.setClock(clockSpeed);
}

inline void SelfAdjustingI2C::setHardwareRiseTime(uint16_t riseTimeNs) {
    // Platform-specific rise time configuration
    // This is a simplified implementation - actual implementation
    // would require platform-specific register manipulation
    
#ifdef ESP32
    // ESP32-specific rise time configuration
    // Would require direct register manipulation for precise control
#elif defined(ESP8266)
    // ESP8266-specific rise time configuration
#elif defined(__AVR__)
    // AVR-specific rise time configuration
    // May require TWI register adjustments
#else
    // Generic implementation - limited control
    // Some platforms may not support precise rise time control
#endif
}

// Utility functions
inline bool SelfAdjustingI2C::shouldTriggerAdjustment() {
    uint32_t totalTransactions = currentConfig.metrics.successfulTransactions + 
                                currentConfig.metrics.failedTransactions;
    return (totalTransactions > 0) && (totalTransactions % PERFORMANCE_SAMPLES == 0);
}

inline void SelfAdjustingI2C::enableLearning(bool enable) {
    learningMode = enable;
    if (enable) {
        adjustmentCooldown = 5000; // Reset to normal cooldown
    }
}

inline void SelfAdjustingI2C::enableAdaptiveMode(bool enable) {
    adaptiveMode = enable;
}

inline void SelfAdjustingI2C::setAdaptationRate(uint8_t rate) {
    adaptationRate = constrain(rate, 1, 10);
}

inline uint32_t SelfAdjustingI2C::getClockSpeed() const {
    return currentConfig.clockSpeed;
}

inline uint16_t SelfAdjustingI2C::getRiseTime() const {
    return currentConfig.riseTime;
}

inline I2CPerformanceMetrics SelfAdjustingI2C::getMetrics() const {
    return currentConfig.metrics;
}

inline float SelfAdjustingI2C::getPerformanceScore() const {
    return performanceScore;
}

inline bool SelfAdjustingI2C::isInRecoveryMode() const {
    return consecutiveErrors >= ERROR_THRESHOLD;
}

inline const char* SelfAdjustingI2C::getLastErrorString() const {
    switch (lastError) {
        case ERROR_NONE: return "No error";
        case ERROR_TIMEOUT: return "Timeout";
        case ERROR_NACK_ADDRESS: return "NACK on address";
        case ERROR_NACK_DATA: return "NACK on data";
        case ERROR_OTHER: return "Other error";
        default: return "Unknown error";
    }
}

inline DeviceConfig* SelfAdjustingI2C::findDeviceConfig(uint8_t address) {
    for (uint8_t i = 0; i < deviceCount; i++) {
        if (deviceConfigs[i].address == address) {
            return &deviceConfigs[i];
        }
    }
    return nullptr;
}

inline void SelfAdjustingI2C::addDeviceConfig(uint8_t address) {
    if (deviceCount < MAX_DEVICES) {
        deviceConfigs[deviceCount].address = address;
        deviceConfigs[deviceCount].config = currentConfig;
        deviceConfigs[deviceCount].hasCustomConfig = false;
        deviceCount++;
    }
}

inline void SelfAdjustingI2C::updateErrorHistory(I2CErrorType error) {
    errorHistory[errorHistoryIndex] = (uint8_t)error;
    errorHistoryIndex = (errorHistoryIndex + 1) % 10;
}

inline float SelfAdjustingI2C::getRecentErrorRate() {
    uint8_t errorCount = 0;
    for (uint8_t i = 0; i < 10; i++) {
        if (errorHistory[i] != ERROR_NONE) {
            errorCount++;
        }
    }
    return (errorCount / 10.0) * 100.0;
}

inline bool SelfAdjustingI2C::isStepValid(uint8_t step) {
    return step < DYNAMIC_RANGE_STEPS;
}

inline void SelfAdjustingI2C::saveCurrentAsBest() {
    bestConfig = currentConfig;
}

inline void SelfAdjustingI2C::restoreBestConfiguration() {
    currentConfig = bestConfig;
    applyConfiguration();
}

inline uint8_t SelfAdjustingI2C::getCurrentClockSpeedStep() const {
    return currentConfig.clockSpeedStep;
}

inline uint8_t SelfAdjustingI2C::getCurrentRiseTimeStep() const {
    return currentConfig.riseTimeStep;
}

// Dynamic range management function implementations
inline void SelfAdjustingI2C::initializeDynamicRanges() {
    // Calculate step sizes for both ranges
    clockSpeedRange.step_size = (float)(clockSpeedRange.max_value - clockSpeedRange.min_value) / (DYNAMIC_RANGE_STEPS - 1);
    riseTimeRange.step_size = (float)(riseTimeRange.max_value - riseTimeRange.min_value) / (DYNAMIC_RANGE_STEPS - 1);
    
    // Set current step based on default values
    clockSpeedRange.current_step = calculateStepFromValue(clockSpeedRange, clockSpeedRange.default_value);
    riseTimeRange.current_step = calculateStepFromValue(riseTimeRange, riseTimeRange.default_value);
    
    // Initialize optimal steps
    clockSpeedRange.optimal_step = clockSpeedRange.current_step;
    riseTimeRange.optimal_step = riseTimeRange.current_step;
}

inline uint32_t SelfAdjustingI2C::calculateValueFromStep(const DynamicRange& range, uint8_t step) {
    if (step >= DYNAMIC_RANGE_STEPS) step = DYNAMIC_RANGE_STEPS - 1;
    return range.min_value + (uint32_t)(step * range.step_size);
}

inline uint8_t SelfAdjustingI2C::calculateStepFromValue(const DynamicRange& range, uint32_t value) {
    if (value <= range.min_value) return 0;
    if (value >= range.max_value) return DYNAMIC_RANGE_STEPS - 1;
    
    uint8_t step = (uint8_t)((value - range.min_value) / range.step_size);
    return min(step, (uint8_t)(DYNAMIC_RANGE_STEPS - 1));
}

inline void SelfAdjustingI2C::updateDynamicRange(DynamicRange& range, uint8_t newStep) {
    if (newStep < DYNAMIC_RANGE_STEPS) {
        range.current_step = newStep;
        range.current_value = calculateValueFromStep(range, newStep);
    }
}

inline void SelfAdjustingI2C::optimizeDynamicRanges() {
    // Update optimal steps based on current performance
    if (performanceScore > calculatePerformanceScore(bestConfig.metrics)) {
        clockSpeedRange.optimal_step = clockSpeedRange.current_step;
        riseTimeRange.optimal_step = riseTimeRange.current_step;
    }
}

#endif // SELF_ADJUSTING_I2C_H