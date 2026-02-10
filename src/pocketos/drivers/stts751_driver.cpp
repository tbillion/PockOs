#include "stts751_driver.h"
#include "../driver_config.h"

#if POCKETOS_STTS751_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// STTS751 Register Addresses
#define STTS751_REG_TEMP_HIGH   0x00  // Temperature high byte
#define STTS751_REG_STATUS      0x01  // Status register
#define STTS751_REG_TEMP_LOW    0x02  // Temperature low byte
#define STTS751_REG_CONFIG      0x03  // Configuration register
#define STTS751_REG_CONV_RATE   0x04  // Conversion rate register
#define STTS751_REG_THIGH_LIMIT 0x05  // High temperature limit
#define STTS751_REG_TLOW_LIMIT  0x06  // Low temperature limit
#define STTS751_REG_ONESHOT     0x0F  // One-shot register
#define STTS751_REG_THERM_LIMIT 0x20  // Thermal limit register
#define STTS751_REG_THERM_HYST  0x21  // Thermal hysteresis register
#define STTS751_REG_SMBUS_TO    0x22  // SMBUS timeout register
#define STTS751_REG_PRODUCT_ID  0xFD  // Product ID register
#define STTS751_REG_MANUF_ID    0xFE  // Manufacturer ID register
#define STTS751_REG_REVISION    0xFF  // Revision ID register

STTS751Driver::STTS751Driver() : address(0), initialized(false) {
#if POCKETOS_STTS751_ENABLE_LOGGING
    readCount = 0;
    errorCount = 0;
#endif
}

bool STTS751Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_STTS751_ENABLE_LOGGING
    Logger::info("STTS751: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_STTS751_ENABLE_CONFIGURATION
    // Set default configuration: continuous conversion, 12-bit resolution
    if (!writeRegister(STTS751_REG_CONFIG, 0x00)) {
#if POCKETOS_STTS751_ENABLE_LOGGING
        Logger::error("STTS751: Failed to write configuration");
#endif
        return false;
    }
    
    // Set conversion rate to 1 Hz
    if (!writeRegister(STTS751_REG_CONV_RATE, 0x04)) {
#if POCKETOS_STTS751_ENABLE_LOGGING
        Logger::error("STTS751: Failed to set conversion rate");
#endif
        return false;
    }
#endif
    
    initialized = true;
#if POCKETOS_STTS751_ENABLE_LOGGING
    Logger::info("STTS751: Initialized successfully");
#endif
    return true;
}

void STTS751Driver::deinit() {
    if (initialized) {
#if POCKETOS_STTS751_ENABLE_CONFIGURATION
        // Stop conversion
        writeRegister(STTS751_REG_CONFIG, 0x40);
#endif
        initialized = false;
    }
}

STTS751Data STTS751Driver::readData() {
    STTS751Data data;
    
    if (!initialized) {
        return data;
    }
    
    // Read temperature high byte
    uint8_t tempHigh;
    if (!readRegister(STTS751_REG_TEMP_HIGH, &tempHigh)) {
#if POCKETOS_STTS751_ENABLE_LOGGING
        errorCount++;
        Logger::error("STTS751: Failed to read temperature high byte");
#endif
        return data;
    }
    
    // Read temperature low byte
    uint8_t tempLow;
    if (!readRegister(STTS751_REG_TEMP_LOW, &tempLow)) {
#if POCKETOS_STTS751_ENABLE_LOGGING
        errorCount++;
        Logger::error("STTS751: Failed to read temperature low byte");
#endif
        return data;
    }
    
    // Convert raw data to temperature
    int16_t tempRaw = ((int16_t)((int8_t)tempHigh) << 8) | tempLow;
    
    // Temperature conversion: 1/256°C per LSB
    data.temperature = tempRaw / 256.0f;
    data.valid = true;
    
#if POCKETOS_STTS751_ENABLE_LOGGING
    readCount++;
#endif
    
    return data;
}

CapabilitySchema STTS751Driver::getSchema() const {
    CapabilitySchema schema;
    
    // Basic settings (available in all tiers)
    schema.addSetting("address", ParamType::STRING, true, "0x39", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "stts751", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_STTS751_TIER_NAME, "", "", "");
    
    // Signals (read-only measurements) - available in all tiers
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    
#if POCKETOS_STTS751_ENABLE_LOGGING
    // Diagnostic signals (Tier 1 only)
    schema.addSignal("read_count", ParamType::INT, true, "");
    schema.addSignal("error_count", ParamType::INT, true, "");
#endif
    
    // Commands
    schema.addCommand("read", "");
    
    return schema;
}

String STTS751Driver::getParameter(const String& name) {
    if (name == "address") {
        return "0x" + String(address, HEX);
    } else if (name == "driver") {
        return "stts751";
    } else if (name == "tier") {
        return POCKETOS_STTS751_TIER_NAME;
    } else if (name == "initialized") {
        return initialized ? "true" : "false";
    }
#if POCKETOS_STTS751_ENABLE_LOGGING
    else if (name == "read_count") {
        return String(readCount);
    } else if (name == "error_count") {
        return String(errorCount);
    }
#endif
    return "";
}

bool STTS751Driver::setParameter(const String& name, const String& value) {
    return false;
}

#if POCKETOS_STTS751_ENABLE_CONFIGURATION
bool STTS751Driver::setResolution(uint8_t resolution) {
    if (!initialized || resolution > 3) {
        return false;
    }
    
    // Read current config
    uint8_t config;
    if (!readRegister(STTS751_REG_CONFIG, &config)) {
        return false;
    }
    
    // Update resolution bits (2:3)
    config = (config & 0xF3) | (resolution << 2);
    
    return writeRegister(STTS751_REG_CONFIG, config);
}

bool STTS751Driver::setConversionRate(uint8_t rate) {
    if (!initialized || rate > 0x0F) {
        return false;
    }
    
    return writeRegister(STTS751_REG_CONV_RATE, rate);
}
#endif

bool STTS751Driver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    Wire.requestFrom(address, (uint8_t)1);
    if (!Wire.available()) {
        return false;
    }
    
    *value = Wire.read();
    return true;
}

bool STTS751Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return (Wire.endTransmission() == 0);
}

} // namespace PocketOS
