#include "spi_driver_pack_adapter.h"
#include "../core/logger.h"

namespace PocketOS {

// Helper to add common schema header data
static void addCommonSettings(CapabilitySchema& schema, const String& driverId, const String& endpoint, int tier, const String& signature) {
    schema.addSetting("driver", ParamType::STRING, false);
    schema.settings[schema.settingCount - 1].units = driverId;
    schema.addSetting("endpoint", ParamType::STRING, false);
    schema.settings[schema.settingCount - 1].units = endpoint;
    schema.addSetting("tier", ParamType::STRING, false);
    schema.settings[schema.settingCount - 1].units = formatTierName(tier);
    if (signature.length() > 0) {
        schema.addSetting("signature", ParamType::STRING, false);
        schema.settings[schema.settingCount - 1].units = signature;
    }
}

bool MCP2515DeviceDriver::setParam(const String& name, const String& value) {
#if POCKETOS_MCP2515_ENABLE_BASIC_READ
    if (name == "mode") {
        uint8_t mode = (uint8_t) value.toInt();
        return driver_.setMode(mode);
    }
    if (name == "bitrate") {
        return driver_.setBitrate(value.toInt());
    }
#endif
    Logger::warn("MCP2515: unsupported param " + name);
    return false;
}

String MCP2515DeviceDriver::getParam(const String& name) {
    if (name == "endpoint") return endpoint_;
    if (name == "tier") return formatTierName(POCKETOS_DRIVER_TIER_MCP2515);
    if (name == "signature") return "CANSTAT/CANCTRL";
    return "";
}

CapabilitySchema MCP2515DeviceDriver::getSchema() {
    CapabilitySchema schema;
    addCommonSettings(schema, "mcp2515", endpoint_, POCKETOS_DRIVER_TIER_MCP2515, "CANSTAT/CANCTRL");
    schema.addCommand("reset", "");
    schema.addCommand("reg.list", "requires tier2");
    schema.addCommand("reg.read", "addr [len]");
    schema.addCommand("reg.write", "addr value");
    return schema;
}

const RegisterDesc* MCP2515DeviceDriver::registers(size_t& count) const {
#if POCKETOS_MCP2515_ENABLE_REGISTER_ACCESS
    return driver_.registers(count);
#else
    count = 0;
    return nullptr;
#endif
}

bool MCP2515DeviceDriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
#if POCKETOS_MCP2515_ENABLE_REGISTER_ACCESS
    return driver_.regRead(reg, buf, len);
#else
    (void)reg; (void)buf; (void)len;
    return false;
#endif
}

bool MCP2515DeviceDriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
#if POCKETOS_MCP2515_ENABLE_REGISTER_ACCESS
    return driver_.regWrite(reg, buf, len);
#else
    (void)reg; (void)buf; (void)len;
    return false;
#endif
}

bool NRF24DeviceDriver::setParam(const String& name, const String& value) {
    Logger::warn("NRF24L01+: unsupported param " + name);
    return false;
}

String NRF24DeviceDriver::getParam(const String& name) {
    if (name == "endpoint") return endpoint_;
    if (name == "tier") return formatTierName(POCKETOS_DRIVER_TIER_NRF24L01);
    if (name == "signature") return "STATUS/CONFIG";
    return "";
}

CapabilitySchema NRF24DeviceDriver::getSchema() {
    CapabilitySchema schema;
    addCommonSettings(schema, "nrf24l01", endpoint_, POCKETOS_DRIVER_TIER_NRF24L01, "STATUS/CONFIG");
    schema.addCommand("reg.list", "");
    schema.addCommand("reg.read", "addr [len]");
    schema.addCommand("reg.write", "addr value");
    return schema;
}

const RegisterDesc* NRF24DeviceDriver::registers(size_t& count) const {
#if POCKETOS_NRF24L01_ENABLE_REGISTER_ACCESS
    return driver_.registers(count);
#else
    count = 0;
    return nullptr;
#endif
}

bool NRF24DeviceDriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
#if POCKETOS_NRF24L01_ENABLE_REGISTER_ACCESS
    return driver_.regRead(reg, buf, len);
#else
    (void)reg; (void)buf; (void)len;
    return false;
#endif
}

bool NRF24DeviceDriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
#if POCKETOS_NRF24L01_ENABLE_REGISTER_ACCESS
    return driver_.regWrite(reg, buf, len);
#else
    (void)reg; (void)buf; (void)len;
    return false;
#endif
}

bool W5500DeviceDriver::setParam(const String& name, const String& value) {
    Logger::warn("W5500: unsupported param " + name);
    return false;
}

String W5500DeviceDriver::getParam(const String& name) {
    if (name == "endpoint") return endpoint_;
    if (name == "tier") return formatTierName(POCKETOS_DRIVER_TIER_W5500);
    if (name == "signature") return "VERSIONR";
    return "";
}

CapabilitySchema W5500DeviceDriver::getSchema() {
    CapabilitySchema schema;
    addCommonSettings(schema, "w5500", endpoint_, POCKETOS_DRIVER_TIER_W5500, "VERSIONR");
    schema.addCommand("reg.list", "");
    schema.addCommand("reg.read", "addr [len]");
    schema.addCommand("reg.write", "addr value");
    return schema;
}

const RegisterDesc* W5500DeviceDriver::registers(size_t& count) const {
#if POCKETOS_W5500_ENABLE_REGISTER_ACCESS
    return driver_.registers(count);
#else
    count = 0;
    return nullptr;
#endif
}

bool W5500DeviceDriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
#if POCKETOS_W5500_ENABLE_REGISTER_ACCESS
    return driver_.regRead(reg, buf, len);
#else
    (void)reg; (void)buf; (void)len;
    return false;
#endif
}

bool W5500DeviceDriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
#if POCKETOS_W5500_ENABLE_REGISTER_ACCESS
    return driver_.regWrite(reg, buf, len);
#else
    (void)reg; (void)buf; (void)len;
    return false;
#endif
}

bool ST7789DeviceDriver::setParam(const String& name, const String& value) {
    if (name == "rotation") {
        return driver_.setRotation((uint8_t)value.toInt());
    }
    Logger::warn("ST7789: unsupported param " + name);
    return false;
}

String ST7789DeviceDriver::getParam(const String& name) {
    if (name == "endpoint") return endpoint_;
    if (name == "tier") return formatTierName(POCKETOS_DRIVER_TIER_ST7789);
    if (name == "signature") return "RDDID";
    return "";
}

CapabilitySchema ST7789DeviceDriver::getSchema() {
    CapabilitySchema schema;
    addCommonSettings(schema, "st7789", endpoint_, POCKETOS_DRIVER_TIER_ST7789, "RDDID");
    schema.addCommand("reg.list", "");
    schema.addCommand("reg.read", "addr [len]");
    schema.addCommand("reg.write", "addr value");
    return schema;
}

const RegisterDesc* ST7789DeviceDriver::registers(size_t& count) const {
#if POCKETOS_ST7789_ENABLE_REGISTER_ACCESS
    return driver_.registers(count);
#else
    count = 0;
    return nullptr;
#endif
}

bool ST7789DeviceDriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
#if POCKETOS_ST7789_ENABLE_REGISTER_ACCESS
    return driver_.readData((uint8_t)reg, buf, len);
#else
    (void)reg; (void)buf; (void)len;
    return false;
#endif
}

bool ST7789DeviceDriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (len == 0) return false;
#if POCKETOS_ST7789_ENABLE_REGISTER_ACCESS
    // For simplicity write first byte as command, remaining as data
    driver_.writeCommand((uint8_t)reg);
    for (size_t i = 0; i < len; i++) {
        driver_.writeData(buf[i]);
    }
    return true;
#else
    (void)reg; (void)buf; (void)len;
    return false;
#endif
}

bool ILI9341DeviceDriver::setParam(const String& name, const String& value) {
    if (name == "rotation") {
        return driver_.setRotation((uint8_t)value.toInt());
    }
    Logger::warn("ILI9341: unsupported param " + name);
    return false;
}

String ILI9341DeviceDriver::getParam(const String& name) {
    if (name == "endpoint") return endpoint_;
    if (name == "tier") return formatTierName(POCKETOS_DRIVER_TIER_ILI9341);
    if (name == "signature") return "RDDID";
    return "";
}

CapabilitySchema ILI9341DeviceDriver::getSchema() {
    CapabilitySchema schema;
    addCommonSettings(schema, "ili9341", endpoint_, POCKETOS_DRIVER_TIER_ILI9341, "RDDID");
    schema.addCommand("reg.list", "");
    schema.addCommand("reg.read", "addr [len]");
    schema.addCommand("reg.write", "addr value");
    return schema;
}

const RegisterDesc* ILI9341DeviceDriver::registers(size_t& count) const {
#if POCKETOS_ILI9341_ENABLE_REGISTER_ACCESS
    return driver_.registers(count);
#else
    count = 0;
    return nullptr;
#endif
}

bool ILI9341DeviceDriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
#if POCKETOS_ILI9341_ENABLE_REGISTER_ACCESS
    return driver_.readData((uint8_t)reg, buf, len);
#else
    (void)reg; (void)buf; (void)len;
    return false;
#endif
}

bool ILI9341DeviceDriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (len == 0) return false;
#if POCKETOS_ILI9341_ENABLE_REGISTER_ACCESS
    driver_.writeCommand((uint8_t)reg);
    for (size_t i = 0; i < len; i++) {
        driver_.writeData(buf[i]);
    }
    return true;
#else
    (void)reg; (void)buf; (void)len;
    return false;
#endif
}

bool UnsupportedSPIDriver::setParam(const String& name, const String& value) {
    Logger::warn(id_ + ": unsupported param " + name + "=" + value);
    return false;
}

String UnsupportedSPIDriver::getParam(const String& name) {
    if (name == "endpoint") return endpoint_;
    if (name == "tier") return "disabled";
    return "";
}

CapabilitySchema UnsupportedSPIDriver::getSchema() {
    CapabilitySchema schema;
    schema.addSetting("driver", ParamType::STRING, false);
    schema.settings[schema.settingCount - 1].units = id_;
    schema.addSetting("endpoint", ParamType::STRING, false);
    schema.settings[schema.settingCount - 1].units = endpoint_;
    schema.addSetting("status", ParamType::STRING, false);
    schema.settings[schema.settingCount - 1].units = "ERR_UNSUPPORTED: enable Tier or driver";
    return schema;
}

bool isUnsupportedSPIId(const String& id) {
    static const char* UNSUPPORTED_IDS[] = {
        "st7735", "st7796", "ili9486", "ili9488", "hx8357", "gc9a01a", "ra8875",
        "ssd1306.spi", "ssd1327.spi", "ssd1351", "w25qxx", "gd25qxx", "at25dfxxx",
        "at45dbxxx", "fm25vxx", "ads1118.spi", "ads1220", "ads1248", "ads1256",
        "ads1262", "dac855x", "mcp492x", "icm20948.spi", "lis3dh.spi", "ssd1675",
        "ssd1680", "uc8151", "il0373", "tlc5947"
    };
    for (size_t i = 0; i < sizeof(UNSUPPORTED_IDS)/sizeof(UNSUPPORTED_IDS[0]); i++) {
        if (id == UNSUPPORTED_IDS[i]) {
            return true;
        }
    }
    return false;
}

} // namespace PocketOS
