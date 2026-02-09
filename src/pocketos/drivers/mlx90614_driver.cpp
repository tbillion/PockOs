#include "mlx90614_driver.h"
#include "../driver_config.h"

#if POCKETOS_MLX90614_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// MLX90614 Register Addresses (RAM)
#define MLX90614_REG_RAWIR1     0x04  // Raw IR data channel 1
#define MLX90614_REG_RAWIR2     0x05  // Raw IR data channel 2
#define MLX90614_REG_TA         0x06  // Ambient temperature
#define MLX90614_REG_TOBJ1      0x07  // Object 1 temperature
#define MLX90614_REG_TOBJ2      0x08  // Object 2 temperature

// MLX90614 Register Addresses (EEPROM)
#define MLX90614_REG_TOMAX      0x20  // To max
#define MLX90614_REG_TOMIN      0x21  // To min
#define MLX90614_REG_PWMCTRL    0x22  // PWM control
#define MLX90614_REG_TARANGE    0x23  // Ta range
#define MLX90614_REG_EMISS      0x24  // Emissivity correction
#define MLX90614_REG_CONFIG     0x25  // Configuration register
#define MLX90614_REG_ADDR       0x2E  // SMBus address
#define MLX90614_REG_ID1        0x3C  // ID number 1
#define MLX90614_REG_ID2        0x3D  // ID number 2
#define MLX90614_REG_ID3        0x3E  // ID number 3
#define MLX90614_REG_ID4        0x3F  // ID number 4

MLX90614Driver::MLX90614Driver() : address(0), initialized(false) {
#if POCKETOS_MLX90614_ENABLE_LOGGING
    readCount = 0;
    errorCount = 0;
#endif
}

bool MLX90614Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_MLX90614_ENABLE_LOGGING
    Logger::info("MLX90614: Initializing at address 0x" + String(address, HEX));
#endif
    
    // Try reading a register to verify device is present
    uint16_t temp;
    if (!readRegister(MLX90614_REG_TA, &temp)) {
#if POCKETOS_MLX90614_ENABLE_LOGGING
        Logger::error("MLX90614: Failed to communicate with device");
#endif
        return false;
    }
    
    initialized = true;
#if POCKETOS_MLX90614_ENABLE_LOGGING
    Logger::info("MLX90614: Initialized successfully");
#endif
    return true;
}

void MLX90614Driver::deinit() {
    if (initialized) {
        initialized = false;
    }
}

MLX90614Data MLX90614Driver::readData() {
    MLX90614Data data;
    
    if (!initialized) {
        return data;
    }
    
    // Read ambient temperature
    uint16_t ambientRaw;
    if (!readRegister(MLX90614_REG_TA, &ambientRaw)) {
#if POCKETOS_MLX90614_ENABLE_LOGGING
        errorCount++;
        Logger::error("MLX90614: Failed to read ambient temperature");
#endif
        return data;
    }
    
    // Read object temperature
    uint16_t objectRaw;
    if (!readRegister(MLX90614_REG_TOBJ1, &objectRaw)) {
#if POCKETOS_MLX90614_ENABLE_LOGGING
        errorCount++;
        Logger::error("MLX90614: Failed to read object temperature");
#endif
        return data;
    }
    
    // Convert raw data to temperature (Kelvin to Celsius)
    // Temperature resolution: 0.02K per LSB
    data.ambientTemperature = (ambientRaw * 0.02f) - 273.15f;
    data.objectTemperature = (objectRaw * 0.02f) - 273.15f;
    data.valid = true;
    
#if POCKETOS_MLX90614_ENABLE_LOGGING
    readCount++;
#endif
    
    return data;
}

CapabilitySchema MLX90614Driver::getSchema() const {
    CapabilitySchema schema;
    
    // Basic settings (available in all tiers)
    schema.addSetting("address", ParamType::STRING, true, "0x5A", "", "", "");
    schema.addSetting("driver", ParamType::STRING, true, "mlx90614", "", "", "");
    schema.addSetting("tier", ParamType::STRING, true, POCKETOS_MLX90614_TIER_NAME, "", "", "");
    
    // Signals (read-only measurements) - available in all tiers
    schema.addSignal("ambient_temperature", ParamType::FLOAT, true, "°C");
    schema.addSignal("object_temperature", ParamType::FLOAT, true, "°C");
    
#if POCKETOS_MLX90614_ENABLE_LOGGING
    // Diagnostic signals (Tier 1 only)
    schema.addSignal("read_count", ParamType::INT, true, "");
    schema.addSignal("error_count", ParamType::INT, true, "");
#endif
    
    // Commands
    schema.addCommand("read", "");
    
    return schema;
}

String MLX90614Driver::getParameter(const String& name) {
    if (name == "address") {
        return "0x" + String(address, HEX);
    } else if (name == "driver") {
        return "mlx90614";
    } else if (name == "tier") {
        return POCKETOS_MLX90614_TIER_NAME;
    } else if (name == "initialized") {
        return initialized ? "true" : "false";
    }
#if POCKETOS_MLX90614_ENABLE_LOGGING
    else if (name == "read_count") {
        return String(readCount);
    } else if (name == "error_count") {
        return String(errorCount);
    }
#endif
    return "";
}

bool MLX90614Driver::setParameter(const String& name, const String& value) {
    return false;
}

#if POCKETOS_MLX90614_ENABLE_CONFIGURATION
bool MLX90614Driver::setEmissivity(float emissivity) {
    if (!initialized || emissivity < 0.1f || emissivity > 1.0f) {
        return false;
    }
    
    // Emissivity is stored as 16-bit value: emissivity * 65535
    uint16_t emissValue = (uint16_t)(emissivity * 65535.0f);
    
    // Write to EEPROM (requires erase first)
    // This is a simplified version - real implementation needs proper EEPROM write procedure
    return writeRegister(MLX90614_REG_EMISS, emissValue);
}

float MLX90614Driver::getEmissivity() {
    if (!initialized) {
        return 0.0f;
    }
    
    uint16_t emissValue;
    if (!readRegister(MLX90614_REG_EMISS, &emissValue)) {
        return 0.0f;
    }
    
    return emissValue / 65535.0f;
}
#endif

bool MLX90614Driver::readRegister(uint8_t reg, uint16_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) {
        return false;
    }
    
    Wire.requestFrom(address, (uint8_t)3);  // 2 data bytes + 1 CRC byte
    
    if (Wire.available() < 3) {
        return false;
    }
    
    uint8_t dataLow = Wire.read();
    uint8_t dataHigh = Wire.read();
    uint8_t pec = Wire.read();
    
#if POCKETOS_MLX90614_ENABLE_ERROR_HANDLING
    // Verify CRC (PEC - Packet Error Code)
    uint8_t crcData[5] = { 
        (uint8_t)(address << 1), 
        reg, 
        (uint8_t)((address << 1) | 1), 
        dataLow, 
        dataHigh 
    };
    uint8_t calculatedCRC = calculateCRC(crcData, 5);
    
    if (calculatedCRC != pec) {
#if POCKETOS_MLX90614_ENABLE_LOGGING
        Logger::error("MLX90614: CRC mismatch");
#endif
        return false;
    }
#endif
    
    *value = (dataHigh << 8) | dataLow;
    return true;
}

bool MLX90614Driver::writeRegister(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write((uint8_t)(value & 0xFF));      // LSB
    Wire.write((uint8_t)(value >> 8));        // MSB
    
    // Calculate and send PEC
    uint8_t crcData[4] = { 
        (uint8_t)(address << 1), 
        reg, 
        (uint8_t)(value & 0xFF), 
        (uint8_t)(value >> 8) 
    };
    uint8_t pec = calculateCRC(crcData, 4);
    Wire.write(pec);
    
    return (Wire.endTransmission() == 0);
}

uint8_t MLX90614Driver::calculateCRC(const uint8_t* data, size_t len) {
    // CRC-8 with polynomial 0x07 (x^8 + x^2 + x + 1)
    uint8_t crc = 0x00;
    
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 8; bit > 0; bit--) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc = (crc << 1);
            }
        }
    }
    
    return crc;
}

} // namespace PocketOS
