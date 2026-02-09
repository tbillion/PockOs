#include "spi_driver_base.h"
#include "../core/resource_manager.h"
#include "../core/logger.h"
#include <SPI.h>

namespace PocketOS {

SPIDriverBase::SPIDriverBase() 
    : initialized_(false), 
      reg_convention_(SPIRegisterConvention::GENERIC) {
    pins_.cs = -1;
    pins_.dc = -1;
    pins_.rst = -1;
    pins_.irq = -1;
    pins_.busy = -1;
}

SPIDriverBase::~SPIDriverBase() {
    deinit();
}

bool SPIDriverBase::initFromEndpoint(const String& endpoint) {
    if (initialized_) {
        return false;
    }
    
    // Parse endpoint descriptor
    if (!parseEndpoint(endpoint)) {
        Logger::error("SPIDriverBase: Failed to parse endpoint: " + endpoint);
        return false;
    }
    
    // Validate required pins
    if (pins_.cs < 0) {
        Logger::error("SPIDriverBase: CS pin is required");
        return false;
    }
    
    // Claim pins via ResourceManager
    if (!claimPins()) {
        Logger::error("SPIDriverBase: Failed to claim pins");
        return false;
    }
    
    // Initialize SPI
    SPI.begin();
    
    // Configure CS pin as output
    pinMode(pins_.cs, OUTPUT);
    digitalWrite(pins_.cs, HIGH);  // CS inactive (active low)
    
    // Configure optional pins
    if (pins_.dc >= 0) {
        pinMode(pins_.dc, OUTPUT);
        digitalWrite(pins_.dc, HIGH);  // Default to data mode
    }
    
    if (pins_.rst >= 0) {
        pinMode(pins_.rst, OUTPUT);
        digitalWrite(pins_.rst, HIGH);  // RST inactive (active low)
    }
    
    if (pins_.irq >= 0) {
        pinMode(pins_.irq, INPUT);
    }
    
    if (pins_.busy >= 0) {
        pinMode(pins_.busy, INPUT);
    }
    
    initialized_ = true;
    Logger::info("SPIDriverBase: Initialized on SPI" + String(bus_config_.bus_id) + ", CS=" + String(pins_.cs));
    return true;
}

void SPIDriverBase::deinit() {
    if (!initialized_) {
        return;
    }
    
    // Release CS (set inactive)
    if (pins_.cs >= 0) {
        digitalWrite(pins_.cs, HIGH);
    }
    
    // Release pins via ResourceManager
    releasePins();
    
    initialized_ = false;
}

bool SPIDriverBase::parseEndpoint(const String& endpoint) {
    // Format: "spi0:cs=5,dc=16,rst=17,irq=4,busy=27"
    
    // Extract bus ID
    int colonIdx = endpoint.indexOf(':');
    if (colonIdx < 0) {
        return false;
    }
    
    String busStr = endpoint.substring(0, colonIdx);
    if (!busStr.startsWith("spi")) {
        return false;
    }
    
    bus_config_.bus_id = busStr.substring(3).toInt();
    
    // Parse pin assignments
    String params = endpoint.substring(colonIdx + 1);
    int startIdx = 0;
    
    while (startIdx < params.length()) {
        int commaIdx = params.indexOf(',', startIdx);
        if (commaIdx < 0) {
            commaIdx = params.length();
        }
        
        String pair = params.substring(startIdx, commaIdx);
        int eqIdx = pair.indexOf('=');
        if (eqIdx > 0) {
            String key = pair.substring(0, eqIdx);
            int value = pair.substring(eqIdx + 1).toInt();
            
            key.toLowerCase();
            if (key == "cs") {
                pins_.cs = value;
            } else if (key == "dc") {
                pins_.dc = value;
            } else if (key == "rst" || key == "reset") {
                pins_.rst = value;
            } else if (key == "irq" || key == "int") {
                pins_.irq = value;
            } else if (key == "busy") {
                pins_.busy = value;
            }
        }
        
        startIdx = commaIdx + 1;
    }
    
    // Generate owner ID for resource manager
    owner_id_ = "spi" + String(bus_config_.bus_id) + "_cs" + String(pins_.cs);
    
    return pins_.cs >= 0;  // CS is required
}

bool SPIDriverBase::claimPins() {
    // Track claimed pins for cleanup on failure
    bool cs_claimed = false;
    bool dc_claimed = false;
    bool rst_claimed = false;
    bool irq_claimed = false;
    bool busy_claimed = false;
    
    // Claim CS pin (required)
    if (!ResourceManager::claim(ResourceType::GPIO_PIN, pins_.cs, owner_id_)) {
        return false;
    }
    cs_claimed = true;
    
    // Claim optional pins, track each successful claim
    if (pins_.dc >= 0) {
        if (!ResourceManager::claim(ResourceType::GPIO_PIN, pins_.dc, owner_id_)) {
            goto cleanup;
        }
        dc_claimed = true;
    }
    
    if (pins_.rst >= 0) {
        if (!ResourceManager::claim(ResourceType::GPIO_PIN, pins_.rst, owner_id_)) {
            goto cleanup;
        }
        rst_claimed = true;
    }
    
    if (pins_.irq >= 0) {
        if (!ResourceManager::claim(ResourceType::GPIO_PIN, pins_.irq, owner_id_)) {
            goto cleanup;
        }
        irq_claimed = true;
    }
    
    if (pins_.busy >= 0) {
        if (!ResourceManager::claim(ResourceType::GPIO_PIN, pins_.busy, owner_id_)) {
            goto cleanup;
        }
        busy_claimed = true;
    }
    
    return true;
    
cleanup:
    // Release any successfully claimed pins
    if (cs_claimed) {
        ResourceManager::release(ResourceType::GPIO_PIN, pins_.cs, owner_id_);
    }
    if (dc_claimed) {
        ResourceManager::release(ResourceType::GPIO_PIN, pins_.dc, owner_id_);
    }
    if (rst_claimed) {
        ResourceManager::release(ResourceType::GPIO_PIN, pins_.rst, owner_id_);
    }
    if (irq_claimed) {
        ResourceManager::release(ResourceType::GPIO_PIN, pins_.irq, owner_id_);
    }
    if (busy_claimed) {
        ResourceManager::release(ResourceType::GPIO_PIN, pins_.busy, owner_id_);
    }
    
    return false;
}

void SPIDriverBase::releasePins() {
    if (pins_.cs >= 0) {
        ResourceManager::release(ResourceType::GPIO_PIN, pins_.cs, owner_id_);
    }
    if (pins_.dc >= 0) {
        ResourceManager::release(ResourceType::GPIO_PIN, pins_.dc, owner_id_);
    }
    if (pins_.rst >= 0) {
        ResourceManager::release(ResourceType::GPIO_PIN, pins_.rst, owner_id_);
    }
    if (pins_.irq >= 0) {
        ResourceManager::release(ResourceType::GPIO_PIN, pins_.irq, owner_id_);
    }
    if (pins_.busy >= 0) {
        ResourceManager::release(ResourceType::GPIO_PIN, pins_.busy, owner_id_);
    }
}

void SPIDriverBase::setCS(bool active) {
    if (pins_.cs >= 0) {
        digitalWrite(pins_.cs, active ? LOW : HIGH);  // Active low
    }
}

void SPIDriverBase::setDC(bool command) {
    if (pins_.dc >= 0) {
        digitalWrite(pins_.dc, command ? LOW : HIGH);  // Command=low, Data=high
    }
}

void SPIDriverBase::setRST(bool active) {
    if (pins_.rst >= 0) {
        digitalWrite(pins_.rst, active ? LOW : HIGH);  // Active low
    }
}

bool SPIDriverBase::readBusy() const {
    if (pins_.busy >= 0) {
        return digitalRead(pins_.busy) == HIGH;
    }
    return false;
}

void SPIDriverBase::beginTransaction() {
    SPISettings settings(bus_config_.speed_hz, bus_config_.bit_order, bus_config_.mode);
    SPI.beginTransaction(settings);
    setCS(true);  // Activate CS
}

void SPIDriverBase::endTransaction() {
    setCS(false);  // Deactivate CS
    SPI.endTransaction();
}

bool SPIDriverBase::spiTransfer(uint8_t* data, size_t len) {
    if (!initialized_) {
        return false;
    }
    
    beginTransaction();
    for (size_t i = 0; i < len; i++) {
        data[i] = SPI.transfer(data[i]);
    }
    endTransaction();
    
    return true;
}

bool SPIDriverBase::spiWrite(const uint8_t* data, size_t len) {
    if (!initialized_) {
        return false;
    }
    
    beginTransaction();
    for (size_t i = 0; i < len; i++) {
        SPI.transfer(data[i]);
    }
    endTransaction();
    
    return true;
}

bool SPIDriverBase::spiRead(uint8_t* data, size_t len) {
    if (!initialized_) {
        return false;
    }
    
    beginTransaction();
    for (size_t i = 0; i < len; i++) {
        data[i] = SPI.transfer(0x00);  // Send dummy byte
    }
    endTransaction();
    
    return true;
}

bool SPIDriverBase::spiWriteRead(const uint8_t* write_data, size_t write_len, 
                                  uint8_t* read_data, size_t read_len) {
    if (!initialized_) {
        return false;
    }
    
    beginTransaction();
    
    // Write phase
    for (size_t i = 0; i < write_len; i++) {
        SPI.transfer(write_data[i]);
    }
    
    // Read phase
    for (size_t i = 0; i < read_len; i++) {
        read_data[i] = SPI.transfer(0x00);  // Send dummy byte
    }
    
    endTransaction();
    
    return true;
}

uint8_t SPIDriverBase::prepareReadCommand(uint8_t reg) {
    return SPIRegisterUtils::toReadAddr(reg, reg_convention_);
}

uint8_t SPIDriverBase::prepareWriteCommand(uint8_t reg) {
    return SPIRegisterUtils::toWriteAddr(reg, reg_convention_);
}

bool SPIDriverBase::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (!initialized_ || reg > 0xFF) {
        return false;
    }
    
    uint8_t reg8 = (uint8_t)reg;
    uint8_t cmd = prepareReadCommand(reg8);
    
    if (SPIRegisterUtils::requiresCommandByte(reg_convention_)) {
        // MCP2515 style: send READ command, then register address, then read data
        uint8_t cmd_buf[2] = { cmd, reg8 };
        return spiWriteRead(cmd_buf, 2, buf, len);
    } else {
        // NRF24/Generic style: send modified register address, then read data
        return spiWriteRead(&cmd, 1, buf, len);
    }
}

bool SPIDriverBase::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (!initialized_ || reg > 0xFF) {
        return false;
    }
    
    uint8_t reg8 = (uint8_t)reg;
    uint8_t cmd = prepareWriteCommand(reg8);
    
    if (SPIRegisterUtils::requiresCommandByte(reg_convention_)) {
        // MCP2515 style: send WRITE command, then register address, then data
        beginTransaction();
        SPI.transfer(cmd);    // WRITE command
        SPI.transfer(reg8);   // Register address
        for (size_t i = 0; i < len; i++) {
            SPI.transfer(buf[i]);
        }
        endTransaction();
        return true;
    } else {
        // NRF24/Generic style: send modified register address, then data
        beginTransaction();
        SPI.transfer(cmd);    // Modified register address
        for (size_t i = 0; i < len; i++) {
            SPI.transfer(buf[i]);
        }
        endTransaction();
        return true;
    }
}

const RegisterDesc* SPIDriverBase::findRegisterByName(const String& name) const {
    size_t count;
    const RegisterDesc* regs = registers(count);
    if (regs == nullptr || count == 0) {
        return nullptr;
    }
    return RegisterUtils::findByName(regs, count, name);
}

} // namespace PocketOS
