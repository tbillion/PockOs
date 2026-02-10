#ifndef POCKETOS_QMC5883L_DRIVER_H
#define POCKETOS_QMC5883L_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"

#if POCKETOS_QMC5883L_ENABLE_REGISTER_ACCESS
#include "register_types.h"
#endif

namespace PocketOS {

#define QMC5883L_ADDR_COUNT 1
const uint8_t QMC5883L_VALID_ADDRESSES[QMC5883L_ADDR_COUNT] = { 0x0D };

struct QMC5883LData {
    int16_t x;
    int16_t y;
    int16_t z;
    bool valid;
    
    QMC5883LData() : x(0), y(0), z(0), valid(false) {}
};

class QMC5883LDriver {
public:
    QMC5883LDriver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    QMC5883LData readData();
    
    CapabilitySchema getSchema() const;
    String getParameter(const String& name);
    bool setParameter(const String& name, const String& value);
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "qmc5883l"; }
    String getDriverTier() const { return POCKETOS_QMC5883L_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = QMC5883L_ADDR_COUNT;
        return QMC5883L_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        return addr == 0x0D;
    }
    
#if POCKETOS_QMC5883L_ENABLE_REGISTER_ACCESS
    const RegisterDesc* registers(size_t& count) const;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len);
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const;
#endif
    
private:
    uint8_t address;
    bool initialized;
    
    bool readRegister(uint8_t reg, uint8_t* value);
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readBlock(uint8_t reg, uint8_t* buffer, size_t length);
};

} // namespace PocketOS

#endif
