#include "tcs34725_driver.h"
#include "../driver_config.h"

#if POCKETOS_TCS34725_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// TCS34725 Register addresses
#define TCS34725_CMD_BIT       0x80
#define TCS34725_REG_ENABLE    0x00
#define TCS34725_REG_ATIME     0x01
#define TCS34725_REG_WTIME     0x03
#define TCS34725_REG_CONFIG    0x0D
#define TCS34725_REG_CONTROL   0x0F
#define TCS34725_REG_ID        0x12
#define TCS34725_REG_STATUS    0x13
#define TCS34725_REG_CDATAL    0x14
#define TCS34725_REG_CDATAH    0x15
#define TCS34725_REG_RDATAL    0x16
#define TCS34725_REG_RDATAH    0x17
#define TCS34725_REG_GDATAL    0x18
#define TCS34725_REG_GDATAH    0x19
#define TCS34725_REG_BDATAL    0x1A
#define TCS34725_REG_BDATAH    0x1B

#define TCS34725_ENABLE_PON    0x01
#define TCS34725_ENABLE_AEN    0x02

#if POCKETOS_TCS34725_ENABLE_REGISTER_ACCESS
static const RegisterDesc TCS34725_REGISTERS[] = {
    RegisterDesc(0x00, "ENABLE", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "ATIME", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x03, "WTIME", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x0D, "CONFIG", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0F, "CONTROL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x12, "ID", 1, RegisterAccess::RO, 0x44),
    RegisterDesc(0x13, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x14, "CDATAL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x15, "CDATAH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x16, "RDATAL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x17, "RDATAH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x18, "GDATAL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x19, "GDATAH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1A, "BDATAL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1B, "BDATAH", 1, RegisterAccess::RO, 0x00),
};

#define TCS34725_REGISTER_COUNT (sizeof(TCS34725_REGISTERS) / sizeof(RegisterDesc))
#endif

TCS34725Driver::TCS34725Driver() : address(0), initialized(false), integrationTime(0xFF), gain(0x00) {}

bool TCS34725Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_TCS34725_ENABLE_LOGGING
    Logger::info("TCS34725: Initializing at address 0x" + String(address, HEX));
#endif
    
    uint8_t id;
    if (!readRegister(TCS34725_REG_ID, &id)) {
#if POCKETOS_TCS34725_ENABLE_LOGGING
        Logger::error("TCS34725: Failed to read chip ID");
#endif
        return false;
    }
    
    if (id != 0x44 && id != 0x4D) {
#if POCKETOS_TCS34725_ENABLE_LOGGING
        Logger::error("TCS34725: Invalid chip ID: 0x" + String(id, HEX));
#endif
        return false;
    }
    
    // Enable device
    writeRegister(TCS34725_REG_ENABLE, TCS34725_ENABLE_PON);
    delay(3);
    writeRegister(TCS34725_REG_ENABLE, TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);
    
#if POCKETOS_TCS34725_ENABLE_CONFIGURATION
    // Set integration time and gain
    integrationTime = 0xFF;  // 2.4ms
    gain = 0x00;  // 1x gain
    writeRegister(TCS34725_REG_ATIME, integrationTime);
    writeRegister(TCS34725_REG_CONTROL, gain);
#endif
    
    initialized = true;
#if POCKETOS_TCS34725_ENABLE_LOGGING
    Logger::info("TCS34725: Initialized successfully");
#endif
    return true;
}

void TCS34725Driver::deinit() {
    if (initialized) {
        writeRegister(TCS34725_REG_ENABLE, 0x00);
    }
    initialized = false;
#if POCKETOS_TCS34725_ENABLE_LOGGING
    Logger::info("TCS34725: Deinitialized");
#endif
}

TCS34725Data TCS34725Driver::readData() {
    TCS34725Data data;
    
    if (!initialized) {
#if POCKETOS_TCS34725_ENABLE_LOGGING
        Logger::error("TCS34725: Not initialized");
#endif
        return data;
    }
    
    delay(50);  // Wait for integration
    
    uint8_t buffer[8];
    if (!readRegisters(TCS34725_REG_CDATAL, buffer, 8)) {
#if POCKETOS_TCS34725_ENABLE_LOGGING
        Logger::error("TCS34725: Failed to read color data");
#endif
        return data;
    }
    
    data.c = (buffer[1] << 8) | buffer[0];
    data.r = (buffer[3] << 8) | buffer[2];
    data.g = (buffer[5] << 8) | buffer[4];
    data.b = (buffer[7] << 8) | buffer[6];
    
    calculateLuxAndCCT(data.r, data.g, data.b, data.c);
    
    data.valid = true;
    return data;
}

void TCS34725Driver::calculateLuxAndCCT(uint16_t r, uint16_t g, uint16_t b, uint16_t c) {
    // Simplified lux calculation
    float illuminance = (-0.32466 * r) + (1.57837 * g) + (-0.73191 * b);
    
    // Color temperature calculation
    float X = (-0.14282 * r) + (1.54924 * g) + (-0.95641 * b);
    float Y = (-0.32466 * r) + (1.57837 * g) + (-0.73191 * b);
    float Z = (-0.68202 * r) + (0.77073 * g) + (0.56332 * b);
    
    float xc = X / (X + Y + Z);
    float yc = Y / (X + Y + Z);
    
    float n = (xc - 0.3320) / (0.1858 - yc);
    float cct = 449 * n * n * n + 3525 * n * n + 6823.3 * n + 5520.33;
}

CapabilitySchema TCS34725Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "tcs34725";
    schema.tier = POCKETOS_TCS34725_TIER_NAME;
    schema.category = "color";
    
    schema.addOutput("r", "uint16", "Red channel");
    schema.addOutput("g", "uint16", "Green channel");
    schema.addOutput("b", "uint16", "Blue channel");
    schema.addOutput("c", "uint16", "Clear channel");
    
#if POCKETOS_TCS34725_ENABLE_CONFIGURATION
    schema.addParameter("integration_time", "uint8", "Integration time", "255");
    schema.addParameter("gain", "uint8", "Gain (0=1x, 1=4x, 2=16x, 3=60x)", "0");
#endif
    
    return schema;
}

String TCS34725Driver::getParameter(const String& name) {
#if POCKETOS_TCS34725_ENABLE_CONFIGURATION
    if (name == "integration_time") return String(integrationTime);
    if (name == "gain") return String(gain);
#endif
    return "";
}

bool TCS34725Driver::setParameter(const String& name, const String& value) {
#if POCKETOS_TCS34725_ENABLE_CONFIGURATION
    if (name == "integration_time") {
        uint8_t newTime = value.toInt();
        if (writeRegister(TCS34725_REG_ATIME, newTime)) {
            integrationTime = newTime;
            return true;
        }
    } else if (name == "gain") {
        uint8_t newGain = value.toInt();
        if (newGain > 3) return false;
        if (writeRegister(TCS34725_REG_CONTROL, newGain)) {
            gain = newGain;
            return true;
        }
    }
#endif
    return false;
}

bool TCS34725Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(TCS34725_CMD_BIT | reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool TCS34725Driver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(TCS34725_CMD_BIT | reg);
    if (Wire.endTransmission() != 0) return false;
    
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available() != 1) return false;
    
    *value = Wire.read();
    return true;
}

bool TCS34725Driver::readRegisters(uint8_t reg, uint8_t* buffer, size_t len) {
    Wire.beginTransmission(address);
    Wire.write(TCS34725_CMD_BIT | reg);
    if (Wire.endTransmission() != 0) return false;
    
    Wire.requestFrom(address, (uint8_t)len);
    if (Wire.available() != len) return false;
    
    for (size_t i = 0; i < len; i++) {
        buffer[i] = Wire.read();
    }
    return true;
}

#if POCKETOS_TCS34725_ENABLE_REGISTER_ACCESS
const RegisterDesc* TCS34725Driver::registers(size_t& count) const {
    count = TCS34725_REGISTER_COUNT;
    return TCS34725_REGISTERS;
}

bool TCS34725Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF) return false;
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        TCS34725_REGISTERS, TCS34725_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) return false;
    
    return readRegister((uint8_t)reg, buf);
}

bool TCS34725Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) return false;
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        TCS34725_REGISTERS, TCS34725_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) return false;
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* TCS34725Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(TCS34725_REGISTERS, TCS34725_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
