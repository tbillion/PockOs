#ifndef POCKETOS_MCP3008_DRIVER_H
#define POCKETOS_MCP3008_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "spi_driver_base.h"
#include "../core/device_registry.h"
#include "register_types.h"

namespace PocketOS {

// MCP3008 10-bit ADC (SPI)
// Endpoint format: spi0:cs=5

class MCP3008Driver : public SPIDriverBase, public IDriver, public IRegisterAccess {
public:
    explicit MCP3008Driver(const String& endpoint);

    // IDriver
    bool init() override;
    bool setParam(const String& name, const String& value) override;
    String getParam(const String& name) override;
    CapabilitySchema getSchema() override;
    void update() override {}

    // IRegisterAccess (channels exposed as registers 0-7)
    const RegisterDesc* registers(size_t& count) const override;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len) override;
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len) override;
    BusType getBusType() const override { return BusType::SPI; }

private:
    String endpoint_;
    static const RegisterDesc MCP3008_REGS[8];

    bool readChannel(uint8_t channel, uint16_t& value);
};

} // namespace PocketOS

#endif // POCKETOS_MCP3008_DRIVER_H
