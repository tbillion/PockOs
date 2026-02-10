#ifndef POCKETOS_PCA9536_DRIVER_H
#define POCKETOS_PCA9536_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_PCA9536_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

// PCA9536 valid I2C address
#define PCA9536_ADDR_COUNT 1
const uint8_t PCA9536_VALID_ADDRESSES[PCA9536_ADDR_COUNT] = { 0x41 };

// PCA9536 register addresses
#define PCA9536_REG_INPUT       0x00
#define PCA9536_REG_OUTPUT      0x01
#define PCA9536_REG_POLARITY    0x02
#define PCA9536_REG_CONFIG      0x03

// PCA9536 Device Driver (4-bit I/O expander)
class PCA9536Driver {
public:
    PCA9536Driver();
    
    // Driver lifecycle
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    // Tier 0: Basic GPIO operations
    bool pinMode(uint8_t pin, uint8_t mode);
    bool digitalWrite(uint8_t pin, bool value);
    int digitalRead(uint8_t pin);
    bool writePort(uint8_t value);
    uint8_t readPort();
    
#if POCKETOS_PCA9536_ENABLE_CONFIGURATION
    // Tier 1: Advanced features
    bool setPolarity(uint8_t pin, bool inverted);
#endif
    
    // Get capability schema
    CapabilitySchema getSchema() const;
    
    // Parameter get/set
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    // Device info
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "pca9536"; }
    String getDriverTier() const { return POCKETOS_PCA9536_TIER_NAME; }
    
    // Address enumeration (all tiers)
    static const uint8_t* validAddresses(size_t& count) {
        count = PCA9536_ADDR_COUNT;
        return PCA9536_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        return addr == 0x41;
    }
    
#if POCKETOS_PCA9536_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    
#if POCKETOS_PCA9536_ENABLE_LOGGING
    uint32_t operationCount;
    uint32_t errorCount;
#endif
    
    // I2C communication
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t* value);
};

} // namespace PocketOS

#endif // POCKETOS_PCA9536_DRIVER_H
