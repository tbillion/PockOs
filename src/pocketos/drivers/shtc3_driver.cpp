#include "shtc3_driver.h"
#include "../driver_config.h"

#if POCKETOS_SHTC3_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// SHTC3 Commands
#define SHTC3_CMD_WAKEUP               0x3517  // Wake up from sleep
#define SHTC3_CMD_SLEEP                0xB098  // Enter sleep mode
#define SHTC3_CMD_SOFT_RESET           0x805D  // Soft reset
#define SHTC3_CMD_READ_ID              0xEFC8  // Read ID register
#define SHTC3_CMD_MEASURE_TFIRST_NORM  0x7CA2  // Measure T first, normal mode
#define SHTC3_CMD_MEASURE_TFIRST_LP    0x6458  // Measure T first, low power mode
#define SHTC3_CMD_MEASURE_HFIRST_NORM  0x5C24  // Measure RH first, normal mode
#define SHTC3_CMD_MEASURE_HFIRST_LP    0x44DE  // Measure RH first, low power mode

SHTC3Driver::SHTC3Driver() : address(0), initialized(false) {
#if POCKETOS_SHTC3_ENABLE_LOGGING
    readCount = 0;
    errorCount = 0;
#endif
}

bool SHTC3Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_SHTC3_ENABLE_LOGGING
    Logger::info("SHTC3: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_SHTC3_ENABLE_CONFIGURATION
    // Soft reset
    if (!sendCommand(SHTC3_CMD_SOFT_RESET)) {
#if POCKETOS_SHTC3_ENABLE_LOGGING
        Logger::error("SHTC3: Failed to send reset command");
#endif
        return false;
    }
    delay(1); // Wait for reset to complete
#endif
    
    initialized = true;
#if POCKETOS_SHTC3_ENABLE_LOGGING
    Logger::info("SHTC3: Initialized successfully");
#endif
    return true;
}

void SHTC3Driver::deinit() {
    if (initialized) {
        // Put sensor to sleep to save power
        sendCommand(SHTC3_CMD_SLEEP);
        initialized = false;
    }
}

SHTC3Data SHTC3Driver::readData() {
    SHTC3Data data;
    
    if (!initialized) {
        return data;
    }
    
    // Wake up sensor
    if (!sendCommand(SHTC3_CMD_WAKEUP)) {
#if POCKETOS_SHTC3_ENABLE_LOGGING
        errorCount++;
        Logger::error("SHTC3: Failed to wake up sensor");
#endif
        return data;
    }
    delayMicroseconds(240); // Wait for wakeup
    
    // Send measurement command (T first, normal mode)
    if (!sendCommand(SHTC3_CMD_MEASURE_TFIRST_NORM)) {
#if POCKETOS_SHTC3_ENABLE_LOGGING
        errorCount++;
        Logger::error("SHTC3: Failed to send measurement command");
#endif
        return data;
    }
    
    // Wait for measurement to complete
    delay(13); // Normal mode measurement takes ~12.1ms
    
    // Read 6 bytes: temp MSB, temp LSB, temp CRC, hum MSB, hum LSB, hum CRC
    uint8_t buffer[6];
    if (!readData(buffer, 6)) {
#if POCKETOS_SHTC3_ENABLE_LOGGING
        errorCount++;
        Logger::error("SHTC3: Failed to read measurement data");
#endif
        sendCommand(SHTC3_CMD_SLEEP); // Put sensor back to sleep
        return data;
    }
    
#if POCKETOS_SHTC3_ENABLE_ERROR_HANDLING
    // Verify CRC for temperature
    if (calculateCRC(buffer, 2) != buffer[2]) {
#if POCKETOS_SHTC3_ENABLE_LOGGING
        errorCount++;
        Logger::error("SHTC3: Temperature CRC mismatch");
#endif
        sendCommand(SHTC3_CMD_SLEEP); // Put sensor back to sleep
        return data;
    }
    
    // Verify CRC for humidity
    if (calculateCRC(buffer + 3, 2) != buffer[5]) {
#if POCKETOS_SHTC3_ENABLE_LOGGING
        errorCount++;
        Logger::error("SHTC3: Humidity CRC mismatch");
#endif
        sendCommand(SHTC3_CMD_SLEEP); // Put sensor back to sleep
        return data;
    }
#endif
    
    // Put sensor back to sleep
    sendCommand(SHTC3_CMD_SLEEP);
    
    // Convert raw data to temperature and humidity
    uint16_t tempRaw = (buffer[0] << 8) | buffer[1];
    uint16_t humRaw = (buffer[3] << 8) | buffer[4];
    
    // Temperature conversion: T = -45 + 175 * (rawValue / 65535)
    data.temperature = -45.0f + 175.0f * (tempRaw / 65535.0f);
    
    // Humidity conversion: RH = 100 * (rawValue / 65535)
    data.humidity = 100.0f * (humRaw / 65535.0f);
    
    // Clamp humidity to valid range
    if (data.humidity > 100.0f) data.humidity = 100.0f;
    if (data.humidity < 0.0f) data.humidity = 0.0f;
    
    data.valid = true;
    
#if POCKETOS_SHTC3_ENABLE_LOGGING
    readCount++;
#endif
    
    return data;
}

CapabilitySchema SHTC3Driver::getSchema() const {
    CapabilitySchema schema;
    
    schema.addSetting("address", ParamType::STRING, true, "0x70", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "shtc3", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_SHTC3_TIER_NAME, "", "", "");
    
    schema.addSignal("temperature", ParamType::FLOAT, true, "°C");
    schema.addSignal("humidity", ParamType::FLOAT, true, "%RH");
    
#if POCKETOS_SHTC3_ENABLE_LOGGING
    schema.addSignal("read_count", ParamType::INT, true, "");
    schema.addSignal("error_count", ParamType::INT, true, "");
#endif
    
    schema.addCommand("read", "");
    
#if POCKETOS_SHTC3_ENABLE_CONFIGURATION
    schema.addCommand("reset", "");
#endif
    
    return schema;
}

String SHTC3Driver::getParameter(const String& name) {
    if (name == "address") {
        return "0x" + String(address, HEX);
    } else if (name == "driver") {
        return "shtc3";
    } else if (name == "tier") {
        return POCKETOS_SHTC3_TIER_NAME;
    } else if (name == "initialized") {
        return initialized ? "true" : "false";
    }
#if POCKETOS_SHTC3_ENABLE_LOGGING
    else if (name == "read_count") {
        return String(readCount);
    } else if (name == "error_count") {
        return String(errorCount);
    }
#endif
    return "";
}

bool SHTC3Driver::setParameter(const String& name, const String& value) {
    return false;
}

bool SHTC3Driver::sendCommand(uint16_t cmd) {
    Wire.beginTransmission(address);
    Wire.write((uint8_t)(cmd >> 8));   // MSB
    Wire.write((uint8_t)(cmd & 0xFF)); // LSB
    return (Wire.endTransmission() == 0);
}

bool SHTC3Driver::readData(uint8_t* buffer, size_t len) {
    Wire.requestFrom(address, (uint8_t)len);
    
    size_t bytesRead = 0;
    while (Wire.available() && bytesRead < len) {
        buffer[bytesRead++] = Wire.read();
    }
    
    return (bytesRead == len);
}

uint8_t SHTC3Driver::calculateCRC(const uint8_t* data, size_t len) {
    // CRC-8 with polynomial 0x31 (x^8 + x^5 + x^4 + 1)
    uint8_t crc = 0xFF;
    
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 8; bit > 0; bit--) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc = (crc << 1);
            }
        }
    }
    
    return crc;
}

} // namespace PocketOS
