#include "as6212_driver.h"
#include "../driver_config.h"

#if POCKETOS_AS6212_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// AS6212 Register Addresses
#define AS6212_REG_TVAL         0x00  // Temperature value register
#define AS6212_REG_CONFIG       0x01  // Configuration register
#define AS6212_REG_TLOW         0x02  // Low temperature threshold
#define AS6212_REG_THIGH        0x03  // High temperature threshold

AS6212Driver::AS6212Driver() : address(0), initialized(false) {
#if POCKETOS_AS6212_ENABLE_LOGGING
    readCount = 0;
    errorCount = 0;
#endif
}

bool AS6212Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_AS6212_ENABLE_LOGGING
    Logger::info("AS6212: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_AS6212_ENABLE_CONFIGURATION
    // Set default configuration: continuous conversion, 4 Hz rate
    if (!writeRegister(AS6212_REG_CONFIG, 0x0000)) {
#if POCKETOS_AS6212_ENABLE_LOGGING
        Logger::error("AS6212: Failed to write configuration");
#endif
        return false;
    }
#endif
    
    initialized = true;
#if POCKETOS_AS6212_ENABLE_LOGGING
    Logger::info("AS6212: Initialized successfully");
#endif
    return true;
}

void AS6212Driver::deinit() {
    if (initialized) {
        initialized = false;
    }
}

AS6212Data AS6212Driver::readData() {
    AS6212Data data;
    
    if (!initialized) {
        return data;
    }
    
    // Read 2 bytes from temperature register
    uint8_t buffer[2];
    if (!readRegister(AS6212_REG_TVAL, buffer, 2)) {
#if POCKETOS_AS6212_ENABLE_LOGGING
        errorCount++;
        Logger::error("AS6212: Failed to read temperature");
#endif
        return data;
    }
    
    // Convert raw data to temperature
    int16_t tempRaw = (buffer[0] << 8) | buffer[1];
    
    // Temperature conversion: 0.0078125°C per LSB
    data.temperature = tempRaw * 0.0078125f;
    data.valid = true;
    
#if POCKETOS_AS6212_ENABLE_LOGGING
    readCount++;
#endif
    
    return data;
}

CapabilitySchema AS6212Driver::getSchema() const {
    CapabilitySchema schema;
    
    // Basic settings (available in all tiers)
    schema.addSetting("address", ParamType::STRING, true, "0x48", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "as6212", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_AS6212_TIER_NAME, "", "", "");
    
    // Signals (read-only measurements) - available in all tiers
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    
#if POCKETOS_AS6212_ENABLE_LOGGING
    // Diagnostic signals (Tier 1 only)
    schema.addSignal("read_count", ParamType::INT, true, "");
    schema.addSignal("error_count", ParamType::INT, true, "");
#endif
    
    // Commands
    schema.addCommand("read", "");
    
    return schema;
}

String AS6212Driver::getParameter(const String& name) {
    if (name == "address") {
        return "0x" + String(address, HEX);
    } else if (name == "driver") {
        return "as6212";
    } else if (name == "tier") {
        return POCKETOS_AS6212_TIER_NAME;
    } else if (name == "initialized") {
        return initialized ? "true" : "false";
    }
#if POCKETOS_AS6212_ENABLE_LOGGING
    else if (name == "read_count") {
        return String(readCount);
    } else if (name == "error_count") {
        return String(errorCount);
    }
#endif
    return "";
}

bool AS6212Driver::setParameter(const String& name, const String& value) {
    return false;
}

#if POCKETOS_AS6212_ENABLE_CONFIGURATION
bool AS6212Driver::setConversionRate(uint8_t rate) {
    if (!initialized || rate > 3) {
        return false;
    }
    
    // Read current config
    uint8_t buffer[2];
    if (!readRegister(AS6212_REG_CONFIG, buffer, 2)) {
        return false;
    }
    
    uint16_t config = (buffer[0] << 8) | buffer[1];
    
    // Update conversion rate bits (6:7)
    config = (config & 0xFF3F) | (rate << 6);
    
    return writeRegister(AS6212_REG_CONFIG, config);
}

bool AS6212Driver::setAlertPolarity(bool activeHigh) {
    if (!initialized) {
        return false;
    }
    
    // Read current config
    uint8_t buffer[2];
    if (!readRegister(AS6212_REG_CONFIG, buffer, 2)) {
        return false;
    }
    
    uint16_t config = (buffer[0] << 8) | buffer[1];
    
    // Update polarity bit (2)
    if (activeHigh) {
        config |= (1 << 2);
    } else {
        config &= ~(1 << 2);
    }
    
    return writeRegister(AS6212_REG_CONFIG, config);
}
#endif

bool AS6212Driver::readRegister(uint8_t reg, uint8_t* buffer, size_t len) {
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

bool AS6212Driver::writeRegister(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write((uint8_t)(value >> 8));   // MSB
    Wire.write((uint8_t)(value & 0xFF)); // LSB
    return (Wire.endTransmission() == 0);
}

} // namespace PocketOS
