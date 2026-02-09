#include "tmp117_driver.h"
#include "../driver_config.h"

#if POCKETOS_TMP117_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// TMP117 Register Addresses
#define TMP117_REG_TEMP_RESULT  0x00  // Temperature result register
#define TMP117_REG_CONFIG       0x01  // Configuration register
#define TMP117_REG_THIGH_LIMIT  0x02  // High temperature limit
#define TMP117_REG_TLOW_LIMIT   0x03  // Low temperature limit
#define TMP117_REG_EEPROM_UL    0x04  // EEPROM unlock register
#define TMP117_REG_EEPROM1      0x05  // EEPROM1 register
#define TMP117_REG_EEPROM2      0x06  // EEPROM2 register
#define TMP117_REG_TEMP_OFFSET  0x07  // Temperature offset register
#define TMP117_REG_EEPROM3      0x08  // EEPROM3 register
#define TMP117_REG_DEVICE_ID    0x0F  // Device ID register

TMP117Driver::TMP117Driver() : address(0), initialized(false) {
#if POCKETOS_TMP117_ENABLE_LOGGING
    readCount = 0;
    errorCount = 0;
#endif
}

bool TMP117Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_TMP117_ENABLE_LOGGING
    Logger::info("TMP117: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_TMP117_ENABLE_CONFIGURATION
    // Soft reset
    if (!writeRegister(TMP117_REG_CONFIG, 0x0002)) {
#if POCKETOS_TMP117_ENABLE_LOGGING
        Logger::error("TMP117: Failed to send soft reset");
#endif
        return false;
    }
    delay(2);  // Wait for reset to complete
    
    // Set default configuration: continuous conversion
    if (!writeRegister(TMP117_REG_CONFIG, 0x0000)) {
#if POCKETOS_TMP117_ENABLE_LOGGING
        Logger::error("TMP117: Failed to write configuration");
#endif
        return false;
    }
#endif
    
    initialized = true;
#if POCKETOS_TMP117_ENABLE_LOGGING
    Logger::info("TMP117: Initialized successfully");
#endif
    return true;
}

void TMP117Driver::deinit() {
    if (initialized) {
        initialized = false;
    }
}

TMP117Data TMP117Driver::readData() {
    TMP117Data data;
    
    if (!initialized) {
        return data;
    }
    
    // Read 2 bytes from temperature register
    uint8_t buffer[2];
    if (!readRegister(TMP117_REG_TEMP_RESULT, buffer, 2)) {
#if POCKETOS_TMP117_ENABLE_LOGGING
        errorCount++;
        Logger::error("TMP117: Failed to read temperature");
#endif
        return data;
    }
    
    // Convert raw data to temperature
    int16_t tempRaw = (buffer[0] << 8) | buffer[1];
    
    // Temperature conversion: 0.0078125°C per LSB
    data.temperature = tempRaw * 0.0078125f;
    data.valid = true;
    
#if POCKETOS_TMP117_ENABLE_LOGGING
    readCount++;
#endif
    
    return data;
}

CapabilitySchema TMP117Driver::getSchema() const {
    CapabilitySchema schema;
    
    // Basic settings (available in all tiers)
    schema.addSetting("address", ParamType::STRING, true, "0x48", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "tmp117", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_TMP117_TIER_NAME, "", "", "");
    
    // Signals (read-only measurements) - available in all tiers
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    
#if POCKETOS_TMP117_ENABLE_LOGGING
    // Diagnostic signals (Tier 1 only)
    schema.addSignal("read_count", ParamType::INT, true, "");
    schema.addSignal("error_count", ParamType::INT, true, "");
#endif
    
    // Commands
    schema.addCommand("read", "");
    
    return schema;
}

String TMP117Driver::getParameter(const String& name) {
    if (name == "address") {
        return "0x" + String(address, HEX);
    } else if (name == "driver") {
        return "tmp117";
    } else if (name == "tier") {
        return POCKETOS_TMP117_TIER_NAME;
    } else if (name == "initialized") {
        return initialized ? "true" : "false";
    }
#if POCKETOS_TMP117_ENABLE_LOGGING
    else if (name == "read_count") {
        return String(readCount);
    } else if (name == "error_count") {
        return String(errorCount);
    }
#endif
    return "";
}

bool TMP117Driver::setParameter(const String& name, const String& value) {
    return false;
}

#if POCKETOS_TMP117_ENABLE_CONFIGURATION
bool TMP117Driver::setConversionCycleTime(uint8_t cycleTime) {
    if (!initialized || cycleTime > 7) {
        return false;
    }
    
    // Read current config
    uint8_t buffer[2];
    if (!readRegister(TMP117_REG_CONFIG, buffer, 2)) {
        return false;
    }
    
    uint16_t config = (buffer[0] << 8) | buffer[1];
    
    // Update conversion cycle time bits (7:9)
    config = (config & 0xFC7F) | (cycleTime << 7);
    
    return writeRegister(TMP117_REG_CONFIG, config);
}

bool TMP117Driver::setAveraging(uint8_t averaging) {
    if (!initialized || averaging > 3) {
        return false;
    }
    
    // Read current config
    uint8_t buffer[2];
    if (!readRegister(TMP117_REG_CONFIG, buffer, 2)) {
        return false;
    }
    
    uint16_t config = (buffer[0] << 8) | buffer[1];
    
    // Update averaging bits (5:6)
    config = (config & 0xFF9F) | (averaging << 5);
    
    return writeRegister(TMP117_REG_CONFIG, config);
}
#endif

bool TMP117Driver::readRegister(uint8_t reg, uint8_t* buffer, size_t len) {
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

bool TMP117Driver::writeRegister(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write((uint8_t)(value >> 8));   // MSB
    Wire.write((uint8_t)(value & 0xFF)); // LSB
    return (Wire.endTransmission() == 0);
}

} // namespace PocketOS
