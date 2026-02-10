#include "am2315_driver.h"
#include "../driver_config.h"

#if POCKETOS_AM2315_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// AM2315 Register addresses
#define AM2315_REG_HUMIDITY_MSB     0x00
#define AM2315_REG_HUMIDITY_LSB     0x01
#define AM2315_REG_TEMP_MSB         0x02
#define AM2315_REG_TEMP_LSB         0x03

AM2315Driver::AM2315Driver() : address(0), initialized(false) {
#if POCKETOS_AM2315_ENABLE_LOGGING
    readCount = 0;
    errorCount = 0;
#endif
}

bool AM2315Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_AM2315_ENABLE_LOGGING
    Logger::info("AM2315: Initializing at address 0x" + String(address, HEX));
#endif
    
    // Wake up sensor
    if (!wakeup()) {
#if POCKETOS_AM2315_ENABLE_LOGGING
        Logger::error("AM2315: Failed to wake up sensor");
#endif
        return false;
    }
    
    initialized = true;
#if POCKETOS_AM2315_ENABLE_LOGGING
    Logger::info("AM2315: Initialized successfully");
#endif
    return true;
}

void AM2315Driver::deinit() {
    initialized = false;
}

AM2315Data AM2315Driver::readData() {
    AM2315Data data;
    
    if (!initialized) {
        return data;
    }
    
    // Wake up sensor (it goes to sleep between readings)
    if (!wakeup()) {
#if POCKETOS_AM2315_ENABLE_LOGGING
        errorCount++;
        Logger::error("AM2315: Failed to wake up sensor");
#endif
        return data;
    }
    
    // Read 4 registers starting at 0x00 (humidity MSB, humidity LSB, temp MSB, temp LSB)
    // Response format: function code (1), register count (1), data (4), CRC (2)
    uint8_t buffer[8];
    if (!readRegisters(AM2315_REG_HUMIDITY_MSB, 4, buffer)) {
#if POCKETOS_AM2315_ENABLE_LOGGING
        errorCount++;
        Logger::error("AM2315: Failed to read measurement data");
#endif
        return data;
    }
    
#if POCKETOS_AM2315_ENABLE_ERROR_HANDLING
    // Verify CRC (last 2 bytes, little-endian)
    uint16_t receivedCRC = buffer[7] | (buffer[6] << 8);
    uint16_t calculatedCRC = calculateCRC(buffer, 6);
    
    if (receivedCRC != calculatedCRC) {
#if POCKETOS_AM2315_ENABLE_LOGGING
        errorCount++;
        Logger::error("AM2315: CRC mismatch");
#endif
        return data;
    }
#endif
    
    // Extract raw data (data starts at buffer[2])
    uint16_t humRaw = (buffer[2] << 8) | buffer[3];
    uint16_t tempRaw = (buffer[4] << 8) | buffer[5];
    
    // Convert to physical values
    // Humidity: RH = rawValue / 10.0
    data.humidity = humRaw / 10.0f;
    
    // Temperature: T = rawValue / 10.0 (with sign bit handling)
    if (tempRaw & 0x8000) {
        // Negative temperature
        data.temperature = -((tempRaw & 0x7FFF) / 10.0f);
    } else {
        data.temperature = tempRaw / 10.0f;
    }
    
    // Clamp humidity to valid range
    if (data.humidity > 100.0f) data.humidity = 100.0f;
    if (data.humidity < 0.0f) data.humidity = 0.0f;
    
    data.valid = true;
    
#if POCKETOS_AM2315_ENABLE_LOGGING
    readCount++;
#endif
    
    return data;
}

CapabilitySchema AM2315Driver::getSchema() const {
    CapabilitySchema schema;
    
    schema.addSetting("address", ParamType::STRING, true, "0x5C", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "am2315", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_AM2315_TIER_NAME, "", "", "");
    
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    schema.addSignal("humidity", ParamType::FLOAT, true, "%RH");
    
#if POCKETOS_AM2315_ENABLE_LOGGING
    schema.addSignal("read_count", ParamType::INT, true, "");
    schema.addSignal("error_count", ParamType::INT, true, "");
#endif
    
    schema.addCommand("read", "");
    
    return schema;
}

String AM2315Driver::getParameter(const String& name) {
    if (name == "address") {
        return "0x" + String(address, HEX);
    } else if (name == "driver") {
        return "am2315";
    } else if (name == "tier") {
        return POCKETOS_AM2315_TIER_NAME;
    } else if (name == "initialized") {
        return initialized ? "true" : "false";
    }
#if POCKETOS_AM2315_ENABLE_LOGGING
    else if (name == "read_count") {
        return String(readCount);
    } else if (name == "error_count") {
        return String(errorCount);
    }
#endif
    return "";
}

bool AM2315Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool AM2315Driver::wakeup() {
    // AM2315 goes to sleep and needs to be woken up
    Wire.beginTransmission(address);
    Wire.endTransmission();
    delay(10); // Wait for sensor to wake up
    return true;
}

bool AM2315Driver::readRegisters(uint8_t reg, uint8_t count, uint8_t* buffer) {
    // Send read command: 0x03 (function code), register address, count
    Wire.beginTransmission(address);
    Wire.write(0x03); // Function code: read holding registers
    Wire.write(reg);
    Wire.write(count);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    delay(10); // Wait for measurement
    
    // Read response: function code (1), count (1), data (count), CRC (2)
    size_t responseLen = 2 + count + 2;
    Wire.requestFrom(address, (uint8_t)responseLen);
    
    size_t bytesRead = 0;
    while (Wire.available() && bytesRead < responseLen) {
        buffer[bytesRead++] = Wire.read();
    }
    
    return (bytesRead == responseLen);
}

uint16_t AM2315Driver::calculateCRC(const uint8_t* data, size_t len) {
    // CRC-16 Modbus
    uint16_t crc = 0xFFFF;
    
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}

} // namespace PocketOS
