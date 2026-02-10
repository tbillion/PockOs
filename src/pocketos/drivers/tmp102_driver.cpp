#include "tmp102_driver.h"
#include "../driver_config.h"

#if POCKETOS_TMP102_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// TMP102 Register Addresses
#define TMP102_REG_TEMP         0x00  // Temperature register
#define TMP102_REG_CONFIG       0x01  // Configuration register
#define TMP102_REG_TLOW         0x02  // Low temperature threshold
#define TMP102_REG_THIGH        0x03  // High temperature threshold

TMP102Driver::TMP102Driver() : address(0), initialized(false), extendedMode(false) {
#if POCKETOS_TMP102_ENABLE_LOGGING
    readCount = 0;
    errorCount = 0;
#endif
}

bool TMP102Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_TMP102_ENABLE_LOGGING
    Logger::info("TMP102: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_TMP102_ENABLE_CONFIGURATION
    // Read current configuration
    uint8_t buffer[2];
    if (!readRegister(TMP102_REG_CONFIG, buffer, 2)) {
#if POCKETOS_TMP102_ENABLE_LOGGING
        Logger::error("TMP102: Failed to read configuration");
#endif
        return false;
    }
    
    uint16_t config = (buffer[0] << 8) | buffer[1];
    
    // Check if extended mode is enabled (bit 4)
    extendedMode = (config & (1 << 4)) != 0;
    
    // Set default configuration: continuous conversion, 4 Hz rate
    config = 0x60A0;  // Default config with continuous conversion
    if (!writeRegister(TMP102_REG_CONFIG, config)) {
#if POCKETOS_TMP102_ENABLE_LOGGING
        Logger::error("TMP102: Failed to write configuration");
#endif
        return false;
    }
#endif
    
    initialized = true;
#if POCKETOS_TMP102_ENABLE_LOGGING
    Logger::info("TMP102: Initialized successfully");
#endif
    return true;
}

void TMP102Driver::deinit() {
    if (initialized) {
        initialized = false;
    }
}

TMP102Data TMP102Driver::readData() {
    TMP102Data data;
    
    if (!initialized) {
        return data;
    }
    
    // Read 2 bytes from temperature register
    uint8_t buffer[2];
    if (!readRegister(TMP102_REG_TEMP, buffer, 2)) {
#if POCKETOS_TMP102_ENABLE_LOGGING
        errorCount++;
        Logger::error("TMP102: Failed to read temperature");
#endif
        return data;
    }
    
    // Convert raw data to temperature
    int16_t tempRaw;
    
    if (extendedMode) {
        // 13-bit extended mode
        tempRaw = (buffer[0] << 5) | (buffer[1] >> 3);
        if (tempRaw & 0x1000) {
            // Sign extend negative values
            tempRaw |= 0xE000;
        }
        data.temperature = tempRaw * 0.0625f;
    } else {
        // 12-bit normal mode
        tempRaw = (buffer[0] << 4) | (buffer[1] >> 4);
        if (tempRaw & 0x800) {
            // Sign extend negative values
            tempRaw |= 0xF000;
        }
        data.temperature = tempRaw * 0.0625f;
    }
    
    data.valid = true;
    
#if POCKETOS_TMP102_ENABLE_LOGGING
    readCount++;
#endif
    
    return data;
}

CapabilitySchema TMP102Driver::getSchema() const {
    CapabilitySchema schema;
    
    // Basic settings (available in all tiers)
    schema.addSetting("address", ParamType::STRING, true, "0x48", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "tmp102", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_TMP102_TIER_NAME, "", "", "");
    
    // Signals (read-only measurements) - available in all tiers
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    
#if POCKETOS_TMP102_ENABLE_LOGGING
    // Diagnostic signals (Tier 1 only)
    schema.addSignal("read_count", ParamType::INT, true, "");
    schema.addSignal("error_count", ParamType::INT, true, "");
#endif
    
    // Commands
    schema.addCommand("read", "");
    
    return schema;
}

String TMP102Driver::getParameter(const String& name) {
    if (name == "address") {
        return "0x" + String(address, HEX);
    } else if (name == "driver") {
        return "tmp102";
    } else if (name == "tier") {
        return POCKETOS_TMP102_TIER_NAME;
    } else if (name == "initialized") {
        return initialized ? "true" : "false";
    }
#if POCKETOS_TMP102_ENABLE_LOGGING
    else if (name == "read_count") {
        return String(readCount);
    } else if (name == "error_count") {
        return String(errorCount);
    }
#endif
    return "";
}

bool TMP102Driver::setParameter(const String& name, const String& value) {
    return false;
}

#if POCKETOS_TMP102_ENABLE_CONFIGURATION
bool TMP102Driver::setConversionRate(uint8_t rate) {
    if (!initialized || rate > 3) {
        return false;
    }
    
    // Read current config
    uint8_t buffer[2];
    if (!readRegister(TMP102_REG_CONFIG, buffer, 2)) {
        return false;
    }
    
    uint16_t config = (buffer[0] << 8) | buffer[1];
    
    // Update conversion rate bits (6:7)
    config = (config & 0xFF3F) | (rate << 6);
    
    return writeRegister(TMP102_REG_CONFIG, config);
}

bool TMP102Driver::setExtendedMode(bool enable) {
    if (!initialized) {
        return false;
    }
    
    // Read current config
    uint8_t buffer[2];
    if (!readRegister(TMP102_REG_CONFIG, buffer, 2)) {
        return false;
    }
    
    uint16_t config = (buffer[0] << 8) | buffer[1];
    
    // Update extended mode bit (4)
    if (enable) {
        config |= (1 << 4);
    } else {
        config &= ~(1 << 4);
    }
    
    bool result = writeRegister(TMP102_REG_CONFIG, config);
    if (result) {
        extendedMode = enable;
    }
    
    return result;
}
#endif

bool TMP102Driver::readRegister(uint8_t reg, uint8_t* buffer, size_t len) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    Wire.requestFrom(address, (uint8_t)len);
    size_t bytesRead = 0;
    while (Wire.available() && bytesRead < len) {
        buffer[bytesRead++] = Wire.read();
    }
    
    return (bytesRead == len);
}

bool TMP102Driver::writeRegister(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write((uint8_t)(value >> 8));   // MSB
    Wire.write((uint8_t)(value & 0xFF)); // LSB
    return (Wire.endTransmission() == 0);
}

} // namespace PocketOS
