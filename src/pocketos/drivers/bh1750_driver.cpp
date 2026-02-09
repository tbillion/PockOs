#include "bh1750_driver.h"
#include "../driver_config.h"

#if POCKETOS_BH1750_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

// BH1750 Commands
#define BH1750_POWER_DOWN          0x00
#define BH1750_POWER_ON            0x01
#define BH1750_RESET               0x07
#define BH1750_CONTINUOUS_HIGH_RES 0x10
#define BH1750_CONTINUOUS_HIGH_RES2 0x11
#define BH1750_CONTINUOUS_LOW_RES  0x13
#define BH1750_ONE_TIME_HIGH_RES   0x20
#define BH1750_ONE_TIME_HIGH_RES2  0x21
#define BH1750_ONE_TIME_LOW_RES    0x23

#if POCKETOS_BH1750_ENABLE_REGISTER_ACCESS
static const RegisterDesc BH1750_REGISTERS[] = {
    RegisterDesc(0x00, "POWER_DOWN", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x01, "POWER_ON", 1, RegisterAccess::WO, 0x01),
    RegisterDesc(0x07, "RESET", 1, RegisterAccess::WO, 0x07),
    RegisterDesc(0x10, "CONT_HIGH_RES", 1, RegisterAccess::WO, 0x10),
    RegisterDesc(0x11, "CONT_HIGH_RES2", 1, RegisterAccess::WO, 0x11),
    RegisterDesc(0x13, "CONT_LOW_RES", 1, RegisterAccess::WO, 0x13),
    RegisterDesc(0x20, "ONE_HIGH_RES", 1, RegisterAccess::WO, 0x20),
    RegisterDesc(0x21, "ONE_HIGH_RES2", 1, RegisterAccess::WO, 0x21),
    RegisterDesc(0x23, "ONE_LOW_RES", 1, RegisterAccess::WO, 0x23),
};

#define BH1750_REGISTER_COUNT (sizeof(BH1750_REGISTERS) / sizeof(RegisterDesc))
#endif

BH1750Driver::BH1750Driver() : address(0), initialized(false), mode(BH1750_CONTINUOUS_HIGH_RES) {}

bool BH1750Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_BH1750_ENABLE_LOGGING
    Logger::info("BH1750: Initializing at address 0x" + String(address, HEX));
#endif
    
    // Power on
    if (!writeCommand(BH1750_POWER_ON)) {
#if POCKETOS_BH1750_ENABLE_LOGGING
        Logger::error("BH1750: Failed to power on");
#endif
        return false;
    }
    
#if POCKETOS_BH1750_ENABLE_CONFIGURATION
    // Reset
    writeCommand(BH1750_RESET);
    delay(10);
    
    // Start continuous high-res mode
    mode = BH1750_CONTINUOUS_HIGH_RES;
#else
    mode = BH1750_CONTINUOUS_HIGH_RES;
#endif
    
    if (!writeCommand(mode)) {
#if POCKETOS_BH1750_ENABLE_LOGGING
        Logger::error("BH1750: Failed to set measurement mode");
#endif
        return false;
    }
    
    initialized = true;
#if POCKETOS_BH1750_ENABLE_LOGGING
    Logger::info("BH1750: Initialized successfully");
#endif
    return true;
}

void BH1750Driver::deinit() {
    if (initialized) {
        writeCommand(BH1750_POWER_DOWN);
    }
    initialized = false;
#if POCKETOS_BH1750_ENABLE_LOGGING
    Logger::info("BH1750: Deinitialized");
#endif
}

BH1750Data BH1750Driver::readData() {
    BH1750Data data;
    
    if (!initialized) {
#if POCKETOS_BH1750_ENABLE_LOGGING
        Logger::error("BH1750: Not initialized");
#endif
        return data;
    }
    
    delay(120);  // Wait for measurement
    
    uint16_t rawValue;
    if (!readData(&rawValue)) {
#if POCKETOS_BH1750_ENABLE_LOGGING
        Logger::error("BH1750: Failed to read data");
#endif
        return data;
    }
    
    data.lux = rawValue / 1.2;
    data.valid = true;
    
    return data;
}

CapabilitySchema BH1750Driver::getSchema() const {
    CapabilitySchema schema;
    schema.driverId = "bh1750";
    schema.tier = POCKETOS_BH1750_TIER_NAME;
    schema.category = "light";
    
    schema.addOutput("lux", "float", "Ambient light in lux");
    
#if POCKETOS_BH1750_ENABLE_CONFIGURATION
    schema.addParameter("mode", "string", "Measurement mode (high/high2/low)", "high");
#endif
    
    return schema;
}

String BH1750Driver::getParameter(const String& name) {
#if POCKETOS_BH1750_ENABLE_CONFIGURATION
    if (name == "mode") {
        if (mode == BH1750_CONTINUOUS_HIGH_RES) return "high";
        if (mode == BH1750_CONTINUOUS_HIGH_RES2) return "high2";
        if (mode == BH1750_CONTINUOUS_LOW_RES) return "low";
    }
#endif
    return "";
}

bool BH1750Driver::setParameter(const String& name, const String& value) {
#if POCKETOS_BH1750_ENABLE_CONFIGURATION
    if (name == "mode") {
        uint8_t newMode;
        if (value == "high") newMode = BH1750_CONTINUOUS_HIGH_RES;
        else if (value == "high2") newMode = BH1750_CONTINUOUS_HIGH_RES2;
        else if (value == "low") newMode = BH1750_CONTINUOUS_LOW_RES;
        else return false;
        
        if (writeCommand(newMode)) {
            mode = newMode;
            return true;
        }
        return false;
    }
#endif
    return false;
}

bool BH1750Driver::writeCommand(uint8_t cmd) {
    Wire.beginTransmission(address);
    Wire.write(cmd);
    return Wire.endTransmission() == 0;
}

bool BH1750Driver::readData(uint16_t* value) {
    Wire.requestFrom(address, (uint8_t)2);
    if (Wire.available() != 2) {
        return false;
    }
    
    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    *value = (msb << 8) | lsb;
    return true;
}

#if POCKETOS_BH1750_ENABLE_REGISTER_ACCESS
const RegisterDesc* BH1750Driver::registers(size_t& count) const {
    count = BH1750_REGISTER_COUNT;
    return BH1750_REGISTERS;
}

bool BH1750Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    return false;  // BH1750 doesn't support register reads
}

bool BH1750Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(
        BH1750_REGISTERS, BH1750_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeCommand((uint8_t)reg);
}

const RegisterDesc* BH1750Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(BH1750_REGISTERS, BH1750_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
