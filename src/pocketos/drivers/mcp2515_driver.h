#ifndef POCKETOS_MCP2515_DRIVER_H
#define POCKETOS_MCP2515_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "spi_driver_base.h"
#include "register_types.h"

namespace PocketOS {

// MCP2515 CAN Controller Driver
// Endpoint format: spi0:cs=5,irq=4 (irq optional)

#define MCP2515_ADDR_COUNT 1
const uint8_t MCP2515_VALID_CS[] = { 0xFF };  // CS pin is user-defined

class MCP2515Driver : public SPIDriverBase {
public:
    MCP2515Driver();
    ~MCP2515Driver();
    
    // Initialize from endpoint descriptor
    bool init(const String& endpoint);
    
    // Valid endpoint configurations
    static bool validEndpoints(const String& endpoint);
    
    // Identification probe - reads CANSTAT register
    static bool identifyProbe(const String& endpoint);
    
#if POCKETOS_MCP2515_ENABLE_BASIC_READ
    // Tier 0: Basic CAN operations
    bool sendFrame(uint32_t id, const uint8_t* data, uint8_t len, bool extended = false);
    bool receiveFrame(uint32_t& id, uint8_t* data, uint8_t& len, bool& extended);
    bool setMode(uint8_t mode);
    bool setBitrate(uint32_t bitrate, uint8_t oscillator_mhz = 16);
#endif

#if POCKETOS_MCP2515_ENABLE_ERROR_HANDLING
    // Tier 1: Filters, masks, error handling
    bool setFilter(uint8_t filter_num, uint32_t mask, bool extended = false);
    bool setMask(uint8_t mask_num, uint32_t mask, bool extended = false);
    uint8_t getErrorFlags();
    uint8_t getRxErrors();
    uint8_t getTxErrors();
    void clearErrors();
#endif

#if POCKETOS_MCP2515_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const override;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len) override;
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len) override;
    const RegisterDesc* findRegisterByName(const String& name) const override;
#endif

private:
    bool initialized_;
    uint8_t oscillator_mhz_;
    
    // Helper methods
    bool reset();
    bool readStatus(uint8_t& status);
    bool modifyRegister(uint8_t reg, uint8_t mask, uint8_t value);
};

} // namespace PocketOS

#endif // POCKETOS_MCP2515_DRIVER_H
