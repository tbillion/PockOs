#ifndef POCKETOS_SPI_DRIVER_PACK_ADAPTER_H
#define POCKETOS_SPI_DRIVER_PACK_ADAPTER_H

#include "../core/device_registry.h"
#include "mcp2515_driver.h"
#include "nrf24l01_driver.h"
#include "w5500_driver.h"
#include "ili9341_driver.h"
#include "st7789_driver.h"
#include "spi_driver_base.h"

namespace PocketOS {

// Common helper to format tier name
inline String formatTierName(int tier) {
    switch (tier) {
        case 0: return "tier0";
        case 1: return "tier1";
        case 2: return "tier2";
        default: return "tier?";
    }
}

class MCP2515DeviceDriver : public IDriver, public IRegisterAccess {
public:
    explicit MCP2515DeviceDriver(const String& endpoint) : endpoint_(endpoint) {}

    bool init() override { return driver_.init(endpoint_); }
    bool setParam(const String& name, const String& value) override;
    String getParam(const String& name) override;
    CapabilitySchema getSchema() override;
    void update() override {}

    // IRegisterAccess
    const RegisterDesc* registers(size_t& count) const override;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len) override;
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len) override;
    BusType getBusType() const override { return BusType::SPI; }

private:
    MCP2515Driver driver_;
    String endpoint_;
};

class NRF24DeviceDriver : public IDriver, public IRegisterAccess {
public:
    explicit NRF24DeviceDriver(const String& endpoint) : endpoint_(endpoint) {}

    bool init() override { return driver_.init(endpoint_); }
    bool setParam(const String& name, const String& value) override;
    String getParam(const String& name) override;
    CapabilitySchema getSchema() override;
    void update() override {}

    const RegisterDesc* registers(size_t& count) const override;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len) override;
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len) override;
    BusType getBusType() const override { return BusType::SPI; }

private:
    NRF24L01Driver driver_;
    String endpoint_;
};

class W5500DeviceDriver : public IDriver, public IRegisterAccess {
public:
    explicit W5500DeviceDriver(const String& endpoint) : endpoint_(endpoint) {}

    bool init() override { return driver_.init(endpoint_); }
    bool setParam(const String& name, const String& value) override;
    String getParam(const String& name) override;
    CapabilitySchema getSchema() override;
    void update() override {}

    const RegisterDesc* registers(size_t& count) const override;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len) override;
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len) override;
    BusType getBusType() const override { return BusType::SPI; }

private:
    W5500Driver driver_;
    String endpoint_;
};

class ST7789DeviceDriver : public IDriver, public IRegisterAccess {
public:
    explicit ST7789DeviceDriver(const String& endpoint) : endpoint_(endpoint) {}

    bool init() override { return driver_.init(endpoint_); }
    bool setParam(const String& name, const String& value) override;
    String getParam(const String& name) override;
    CapabilitySchema getSchema() override;
    void update() override {}

    const RegisterDesc* registers(size_t& count) const override;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len) override;
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len) override;
    BusType getBusType() const override { return BusType::SPI; }

private:
    ST7789Driver driver_;
    String endpoint_;
};

class ILI9341DeviceDriver : public IDriver, public IRegisterAccess {
public:
    explicit ILI9341DeviceDriver(const String& endpoint) : endpoint_(endpoint) {}

    bool init() override { return driver_.init(endpoint_); }
    bool setParam(const String& name, const String& value) override;
    String getParam(const String& name) override;
    CapabilitySchema getSchema() override;
    void update() override {}

    const RegisterDesc* registers(size_t& count) const override;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len) override;
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len) override;
    BusType getBusType() const override { return BusType::SPI; }

private:
    ILI9341Driver driver_;
    String endpoint_;
};

} // namespace PocketOS

#endif // POCKETOS_SPI_DRIVER_PACK_ADAPTER_H
