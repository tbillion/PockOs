#ifndef POCKETOS_MLX90640_DRIVER_H
#define POCKETOS_MLX90640_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"
#include "register_types.h"

namespace PocketOS {

#define MLX90640_ADDR_COUNT 1
const uint8_t MLX90640_VALID_ADDRESSES[MLX90640_ADDR_COUNT] = { 0x33 };

struct MLX90640Data {
    float temperatures[768];  // 32x24 thermal array
    bool valid;
    
    MLX90640Data() : valid(false) { memset(temperatures, 0, sizeof(temperatures)); }
};

class MLX90640Driver {
public:
    MLX90640Driver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    MLX90640Data readData();
    CapabilitySchema getSchema() const;
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "mlx90640"; }
    String getDriverTier() const { return POCKETOS_MLX90640_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = MLX90640_ADDR_COUNT;
        return MLX90640_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < MLX90640_ADDR_COUNT; i++) {
            if (MLX90640_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_MLX90640_ENABLE_REGISTER_ACCESS
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t* value);
};

} // namespace PocketOS

#endif
