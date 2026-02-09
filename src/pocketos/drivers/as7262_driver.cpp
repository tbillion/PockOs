#include "as7262_driver.h"
#include "../driver_config.h"

#if POCKETOS_AS7262_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// AS7262 Virtual Register Map
#define AS7262_VREG_HW_VERSION    0x00
#define AS7262_VREG_CONTROL       0x04
#define AS7262_VREG_INT_TIME      0x05
#define AS7262_VREG_DEVICE_TEMP   0x06
#define AS7262_VREG_LED_CONTROL   0x07
#define AS7262_VREG_V_HIGH        0x08
#define AS7262_VREG_V_LOW         0x09
#define AS7262_VREG_B_HIGH        0x0A
#define AS7262_VREG_B_LOW         0x0B
#define AS7262_VREG_G_HIGH        0x0C
#define AS7262_VREG_G_LOW         0x0D
#define AS7262_VREG_Y_HIGH        0x0E
#define AS7262_VREG_Y_LOW         0x0F
#define AS7262_VREG_O_HIGH        0x10
#define AS7262_VREG_O_LOW         0x11
#define AS7262_VREG_R_HIGH        0x12
#define AS7262_VREG_R_LOW         0x13

#if POCKETOS_AS7262_ENABLE_REGISTER_ACCESS
static const RegisterDesc AS7262_REGISTERS[] = {
    RegisterDesc(0x00, "HW_VERSION", 1, RegisterAccess::RO, 0x3E),
    RegisterDesc(0x04, "CONTROL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x05, "INT_TIME", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x06, "DEVICE_TEMP", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x07, "LED_CONTROL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x08, "V_HIGH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x09, "V_LOW", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0A, "B_HIGH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0B, "B_LOW", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0C, "G_HIGH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0D, "G_LOW", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0E, "Y_HIGH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0F, "Y_LOW", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x10, "O_HIGH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x11, "O_LOW", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x12, "R_HIGH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x13, "R_LOW", 1, RegisterAccess::RO, 0x00),
};

#define AS7262_REGISTER_COUNT (sizeof(AS7262_REGISTERS) / sizeof(RegisterDesc))
#endif

AS7262Driver::AS7262Driver() : address(0), initialized(false) {}

bool AS7262Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_AS7262_ENABLE_LOGGING
    Logger::info("AS7262: Initializing at address 0x" + String(address, HEX));
#endif
    
    // Check hardware version
    uint8_t hwVersion = 0;
    if (!readRegister(AS7262_VREG_HW_VERSION, &hwVersion)) {
#if POCKETOS_AS7262_ENABLE_LOGGING
        Logger::error("AS7262: Failed to read hardware version");
#endif
        return false;
    }
    
#if POCKETOS_AS7262_ENABLE_CONFIGURATION
    // Initialize sensor with default settings
    writeRegister(AS7262_VREG_CONTROL, 0x00);  // Normal mode
    writeRegister(AS7262_VREG_INT_TIME, 0xFF); // Max integration time
#endif
    
    initialized = true;
#if POCKETOS_AS7262_ENABLE_LOGGING
    Logger::info("AS7262: Initialized successfully");
#endif
    return true;
}

void AS7262Driver::deinit() {
    initialized = false;
}

AS7262Data AS7262Driver::readData() {
    AS7262Data data;
    
    if (!initialized) {
        return data;
    }
    
    // Read all 6 channels (2 bytes each)
    uint8_t raw[12];
    for (int i = 0; i < 12; i++) {
        if (!readRegister(AS7262_VREG_V_HIGH + i, &raw[i])) {
            return data;
        }
    }
    
    data.violet = (raw[0] << 8) | raw[1];
    data.blue = (raw[2] << 8) | raw[3];
    data.green = (raw[4] << 8) | raw[5];
    data.yellow = (raw[6] << 8) | raw[7];
    data.orange = (raw[8] << 8) | raw[9];
    data.red = (raw[10] << 8) | raw[11];
    data.valid = true;
    
    return data;
}

CapabilitySchema AS7262Driver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_AS7262_TIER_NAME;
    return schema;
}

bool AS7262Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool AS7262Driver::readRegister(uint8_t reg, uint8_t* value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available()) {
        *value = Wire.read();
        return true;
    }
    return false;
}

#if POCKETOS_AS7262_ENABLE_REGISTER_ACCESS
const RegisterDesc* AS7262Driver::registers(size_t& count) const {
    count = AS7262_REGISTER_COUNT;
    return AS7262_REGISTERS;
}

bool AS7262Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(AS7262_REGISTERS, AS7262_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool AS7262Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(AS7262_REGISTERS, AS7262_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* AS7262Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(AS7262_REGISTERS, AS7262_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
