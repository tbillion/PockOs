#include "mcp9808_driver.h"
#include "../driver_config.h"

#if POCKETOS_MCP9808_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// MCP9808 Register Addresses
#define MCP9808_REG_CONFIG      0x01  // Configuration register
#define MCP9808_REG_TUPPER      0x02  // Upper temperature threshold
#define MCP9808_REG_TLOWER      0x03  // Lower temperature threshold
#define MCP9808_REG_TCRIT       0x04  // Critical temperature threshold
#define MCP9808_REG_TAMBIENT    0x05  // Ambient temperature register
#define MCP9808_REG_MANUF_ID    0x06  // Manufacturer ID register
#define MCP9808_REG_DEVICE_ID   0x07  // Device ID register
#define MCP9808_REG_RESOLUTION  0x08  // Resolution register

MCP9808Driver::MCP9808Driver() : address(0), initialized(false) {
#if POCKETOS_MCP9808_ENABLE_LOGGING
    readCount = 0;
    errorCount = 0;
#endif
}

bool MCP9808Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_MCP9808_ENABLE_LOGGING
    Logger::info("MCP9808: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_MCP9808_ENABLE_CONFIGURATION
    // Set default configuration: continuous conversion
    if (!writeRegister(MCP9808_REG_CONFIG, 0x0000)) {
#if POCKETOS_MCP9808_ENABLE_LOGGING
        Logger::error("MCP9808: Failed to write configuration");
#endif
        return false;
    }
    
    // Set maximum resolution (0.0625°C)
    if (!writeRegister(MCP9808_REG_RESOLUTION, 0x0003)) {
#if POCKETOS_MCP9808_ENABLE_LOGGING
        Logger::error("MCP9808: Failed to set resolution");
#endif
        return false;
    }
#endif
    
    initialized = true;
#if POCKETOS_MCP9808_ENABLE_LOGGING
    Logger::info("MCP9808: Initialized successfully");
#endif
    return true;
}

void MCP9808Driver::deinit() {
    if (initialized) {
#if POCKETOS_MCP9808_ENABLE_CONFIGURATION
        // Put device in shutdown mode
        shutdown(true);
#endif
        initialized = false;
    }
}

MCP9808Data MCP9808Driver::readData() {
    MCP9808Data data;
    
    if (!initialized) {
        return data;
    }
    
    // Read 2 bytes from temperature register
    uint8_t buffer[2];
    if (!readRegister(MCP9808_REG_TAMBIENT, buffer, 2)) {
#if POCKETOS_MCP9808_ENABLE_LOGGING
        errorCount++;
        Logger::error("MCP9808: Failed to read temperature");
#endif
        return data;
    }
    
    // Convert raw data to temperature
    uint16_t tempRaw = (buffer[0] << 8) | buffer[1];
    
    // Clear flag bits (bits 13-15)
    tempRaw &= 0x1FFF;
    
    // Check sign bit (bit 12)
    if (tempRaw & 0x1000) {
        // Negative temperature
        tempRaw &= 0x0FFF;
        data.temperature = -((float)tempRaw * 0.0625f);
    } else {
        // Positive temperature
        data.temperature = (float)tempRaw * 0.0625f;
    }
    
    data.valid = true;
    
#if POCKETOS_MCP9808_ENABLE_LOGGING
    readCount++;
#endif
    
    return data;
}

CapabilitySchema MCP9808Driver::getSchema() const {
    CapabilitySchema schema;
    
    // Basic settings (available in all tiers)
    schema.addSetting("address", ParamType::STRING, true, "0x18", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "mcp9808", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_MCP9808_TIER_NAME, "", "", "");
    
    // Signals (read-only measurements) - available in all tiers
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    
#if POCKETOS_MCP9808_ENABLE_LOGGING
    // Diagnostic signals (Tier 1 only)
    schema.addSignal("read_count", ParamType::INT, true, "");
    schema.addSignal("error_count", ParamType::INT, true, "");
#endif
    
    // Commands
    schema.addCommand("read", "");
    
    return schema;
}

String MCP9808Driver::getParameter(const String& name) {
    if (name == "address") {
        return "0x" + String(address, HEX);
    } else if (name == "driver") {
        return "mcp9808";
    } else if (name == "tier") {
        return POCKETOS_MCP9808_TIER_NAME;
    } else if (name == "initialized") {
        return initialized ? "true" : "false";
    }
#if POCKETOS_MCP9808_ENABLE_LOGGING
    else if (name == "read_count") {
        return String(readCount);
    } else if (name == "error_count") {
        return String(errorCount);
    }
#endif
    return "";
}

bool MCP9808Driver::setParameter(const String& name, const String& value) {
    return false;
}

#if POCKETOS_MCP9808_ENABLE_CONFIGURATION
bool MCP9808Driver::setResolution(uint8_t resolution) {
    if (!initialized || resolution > 3) {
        return false;
    }
    
    return writeRegister(MCP9808_REG_RESOLUTION, resolution);
}

bool MCP9808Driver::shutdown(bool enable) {
    if (!initialized) {
        return false;
    }
    
    // Read current config
    uint8_t buffer[2];
    if (!readRegister(MCP9808_REG_CONFIG, buffer, 2)) {
        return false;
    }
    
    uint16_t config = (buffer[0] << 8) | buffer[1];
    
    // Update shutdown bit (8)
    if (enable) {
        config |= (1 << 8);
    } else {
        config &= ~(1 << 8);
    }
    
    return writeRegister(MCP9808_REG_CONFIG, config);
}
#endif

bool MCP9808Driver::readRegister(uint8_t reg, uint8_t* buffer, size_t len) {
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

bool MCP9808Driver::writeRegister(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write((uint8_t)(value >> 8));   // MSB
    Wire.write((uint8_t)(value & 0xFF)); // LSB
    return (Wire.endTransmission() == 0);
}

} // namespace PocketOS
