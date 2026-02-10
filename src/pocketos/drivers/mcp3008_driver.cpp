#include "mcp3008_driver.h"
#include "../core/logger.h"

namespace PocketOS {

const RegisterDesc MCP3008Driver::MCP3008_REGS[8] = {
    RegisterDesc(0x00, "CH0", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x01, "CH1", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x02, "CH2", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x03, "CH3", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x04, "CH4", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x05, "CH5", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x06, "CH6", 2, RegisterAccess::RO, 0x0000),
    RegisterDesc(0x07, "CH7", 2, RegisterAccess::RO, 0x0000),
};

MCP3008Driver::MCP3008Driver(const String& endpoint) : endpoint_(endpoint) {
    setRegisterConvention(SPIRegisterConvention::GENERIC);
}

bool MCP3008Driver::init() {
    return initFromEndpoint(endpoint_);
}

bool MCP3008Driver::setParam(const String& name, const String& value) {
    Logger::warn("MCP3008: unsupported param " + name + " value " + value);
    return false;
}

String MCP3008Driver::getParam(const String& name) {
    if (name == "endpoint") return endpoint_;
    if (name == "tier") {
        switch (POCKETOS_DRIVER_TIER_MCP3008) {
            case 0: return "tier0";
            case 1: return "tier1";
            case 2: return "tier2";
            default: return "tier?";
        }
    }
    return "";
}

CapabilitySchema MCP3008Driver::getSchema() {
    CapabilitySchema schema;
    schema.addSetting("driver", ParamType::STRING, false);
    schema.settings[schema.settingCount - 1].units = "mcp3008";
    schema.addSetting("endpoint", ParamType::STRING, false);
    schema.settings[schema.settingCount - 1].units = endpoint_;
    schema.addSetting("tier", ParamType::STRING, false);
    switch (POCKETOS_DRIVER_TIER_MCP3008) {
        case 0: schema.settings[schema.settingCount - 1].units = "tier0"; break;
        case 1: schema.settings[schema.settingCount - 1].units = "tier1"; break;
        case 2: schema.settings[schema.settingCount - 1].units = "tier2"; break;
        default: schema.settings[schema.settingCount - 1].units = "tier?"; break;
    }
    schema.addSignal("ch0", ParamType::FLOAT, false, "counts");
    schema.addSignal("ch1", ParamType::FLOAT, false, "counts");
    schema.addSignal("ch2", ParamType::FLOAT, false, "counts");
    schema.addSignal("ch3", ParamType::FLOAT, false, "counts");
    schema.addSignal("ch4", ParamType::FLOAT, false, "counts");
    schema.addSignal("ch5", ParamType::FLOAT, false, "counts");
    schema.addSignal("ch6", ParamType::FLOAT, false, "counts");
    schema.addSignal("ch7", ParamType::FLOAT, false, "counts");
    schema.addCommand("reg.list", "");
    schema.addCommand("reg.read", "ch [len]");
    return schema;
}

const RegisterDesc* MCP3008Driver::registers(size_t& count) const {
#if POCKETOS_MCP3008_ENABLE_REGISTER_ACCESS
    count = 8;
    return MCP3008_REGS;
#else
    count = 0;
    return nullptr;
#endif
}

bool MCP3008Driver::readChannel(uint8_t channel, uint16_t& value) {
    if (channel >= 8) {
        return false;
    }

    // MCP3008 uses 3-byte transaction: start bit, single-ended flag, channel bits
    uint8_t tx[3];
    tx[0] = 0x01; // start
    tx[1] = (uint8_t)((0x08 | channel) << 4); // single-ended + channel
    tx[2] = 0x00;

    uint8_t rx[3] = {0, 0, 0};
    if (!spiWriteRead(tx, 3, rx, 3)) {
        return false;
    }

    value = ((rx[1] & 0x03) << 8) | rx[2];
    return true;
}

bool MCP3008Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
#if !POCKETOS_MCP3008_ENABLE_REGISTER_ACCESS
    (void)reg; (void)buf; (void)len;
    return false;
#else
    if (len < 2) {
        return false;
    }
    uint16_t value;
    if (!readChannel((uint8_t)reg, value)) {
        return false;
    }
    buf[0] = (uint8_t)((value >> 8) & 0xFF);
    buf[1] = (uint8_t)(value & 0xFF);
    return true;
#endif
}

bool MCP3008Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    (void)reg;
    (void)buf;
    (void)len;
    // ADC registers are read-only
    return false;
}

} // namespace PocketOS
