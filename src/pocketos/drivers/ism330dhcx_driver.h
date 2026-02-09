#ifndef POCKETOS_ISM330DHCX_DRIVER_H
#define POCKETOS_ISM330DHCX_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "../core/capability_schema.h"
#include "register_types.h"

namespace PocketOS {

#define ISM330DHCX_ADDR_COUNT 2
const uint8_t ISM330DHCX_VALID_ADDRESSES[ISM330DHCX_ADDR_COUNT] = { 0x6A, 0x6B };

struct ISM330DHCXData {
    float accel_x, accel_y, accel_z;  // g
    float gyro_x, gyro_y, gyro_z;     // dps
    bool valid;
    
    ISM330DHCXData() : accel_x(0), accel_y(0), accel_z(0), 
                       gyro_x(0), gyro_y(0), gyro_z(0), valid(false) {}
};

class ISM330DHCXDriver {
public:
    ISM330DHCXDriver();
    
    bool init(uint8_t i2cAddress);
    void deinit();
    bool isInitialized() const { return initialized; }
    
    ISM330DHCXData readData();
    CapabilitySchema getSchema() const;
    
    uint8_t getAddress() const { return address; }
    String getDriverId() const { return "ism330dhcx"; }
    String getDriverTier() const { return POCKETOS_ISM330DHCX_TIER_NAME; }
    
    static const uint8_t* validAddresses(size_t& count) {
        count = ISM330DHCX_ADDR_COUNT;
        return ISM330DHCX_VALID_ADDRESSES;
    }
    
    static bool supportsAddress(uint8_t addr) {
        for (size_t i = 0; i < ISM330DHCX_ADDR_COUNT; i++) {
            if (ISM330DHCX_VALID_ADDRESSES[i] == addr) {
                return true;
            }
        }
        return false;
    }
    
#if POCKETOS_ISM330DHCX_ENABLE_REGISTER_ACCESS
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
