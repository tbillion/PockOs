#ifndef POCKETOS_MLX90614_DRIVER_H
#define POCKETOS_MLX90614_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

namespace PocketOS {

// MLX90614 valid I2C addresses
#define MLX90614_ADDR_COUNT 1
const uint8_t MLX90614_VALID_ADDRESSES[MLX90614_ADDR_COUNT] = { 0x5A };

// MLX90614 measurement data
struct MLX90614Data {
    float ambientTemperature;  // Celsius
    float objectTemperature;   // Celsius
    bool valid;
    
    MLX90614Data() : ambientTemperature(0), objectTemperature(0), valid(false) {}
};

// MLX90614 Device Driver (Tier 0/1)
class MLX90614Driver {
public:
    MLX90614Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Read measurements
    MLX90614Data readData();
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "mlx90614"; }
    String getDriverTier() const { return POCKETOS_MLX90614_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = MLX90614_ADDR_COUNT;
        return MLX90614_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < MLX90614_ADDR_COUNT; i++) {
            if (MLX90614_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_MLX90614_ENABLE_CONFIGURATION
    // Configuration (Tier 1 only)
    bool setEmissivity(float emissivity);
    float getEmissivity();
#endif
    
private:
    uint8_t address;
    bool initialized;
    
#if POCKETOS_MLX90614_ENABLE_LOGGING
    uint32_t readCount;
    uint32_t errorCount;
#endif
    
    // I2C communication
    bool readRegister(uint8_t reg, uint16_t* value);
    bool writeRegister(uint8_t reg, uint16_t value);
    
    // CRC calculation
    uint8_t calculateCRC(const uint8_t* data, size_t len);
};

} // namespace PocketOS

#endif // POCKETOS_MLX90614_DRIVER_H
