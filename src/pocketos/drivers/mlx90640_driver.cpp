#include "mlx90640_driver.h"
#include "../driver_config.h"

#if POCKETOS_MLX90640_ENABLE_LOGGING
#include "../core/logger.h"
#endif

#include <Wire.h>

namespace PocketOS {

#define MLX90640_REG_CONTROL    0x00
#define MLX90640_REG_STATUS     0x01
#define MLX90640_REG_CONFIG     0x02

#if POCKETOS_MLX90640_ENABLE_REGISTER_ACCESS
static const RegisterDesc MLX90640_REGISTERS[] = {
    RegisterDesc(0x00, "CONTROL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "STATUS", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x02, "CONFIG", 1, RegisterAccess::RW, 0x00),
};

#define MLX90640_REGISTER_COUNT (sizeof(MLX90640_REGISTERS) / sizeof(RegisterDesc))
#endif

MLX90640Driver::MLX90640Driver() : address(0), initialized(false) {}

bool MLX90640Driver::init(uint8_t i2cAddress) {
    address = i2cAddress;
    
#if POCKETOS_MLX90640_ENABLE_LOGGING
    Logger::info("MLX90640: Initializing at address 0x" + String(address, HEX));
#endif
    
#if POCKETOS_MLX90640_ENABLE_CONFIGURATION
    writeRegister(MLX90640_REG_CONTROL, 0x00);
    writeRegister(MLX90640_REG_CONFIG, 0x00);
#endif
    
    initialized = true;
#if POCKETOS_MLX90640_ENABLE_LOGGING
    Logger::info("MLX90640: Initialized successfully");
#endif
    return true;
}

void MLX90640Driver::deinit() {
    initialized = false;
}

MLX90640Data MLX90640Driver::readData() {
    MLX90640Data data;
    
    if (!initialized) {
        return data;
    }
    
    data.valid = true;
    return data;
}

CapabilitySchema MLX90640Driver::getSchema() const {
    CapabilitySchema schema;
    schema.tier = POCKETOS_MLX90640_TIER_NAME;
    return schema;
}

bool MLX90640Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool MLX90640Driver::readRegister(uint8_t reg, uint8_t* value) {
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

#if POCKETOS_MLX90640_ENABLE_REGISTER_ACCESS
const RegisterDesc* MLX90640Driver::registers(size_t& count) const {
    count = MLX90640_REGISTER_COUNT;
    return MLX90640_REGISTERS;
}

bool MLX90640Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(MLX90640_REGISTERS, MLX90640_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isReadable(regDesc->access)) {
        return false;
    }
    
    return readRegister((uint8_t)reg, buf);
}

bool MLX90640Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized || reg > 0xFF || len != 1) {
        return false;
    }
    
    const RegisterDesc* regDesc = RegisterUtils::findByAddr(MLX90640_REGISTERS, MLX90640_REGISTER_COUNT, reg);
    if (!regDesc || !RegisterUtils::isWritable(regDesc->access)) {
        return false;
    }
    
    return writeRegister((uint8_t)reg, buf[0]);
}

const RegisterDesc* MLX90640Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(MLX90640_REGISTERS, MLX90640_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
