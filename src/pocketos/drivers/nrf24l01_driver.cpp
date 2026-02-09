#include "nrf24l01_driver.h"
#include "../core/logger.h"
#include "../core/resource_manager.h"
#include <SPI.h>

namespace PocketOS {

// nRF24L01+ Commands
#define NRF24_CMD_R_REGISTER    0x00
#define NRF24_CMD_W_REGISTER    0x20
#define NRF24_CMD_R_RX_PAYLOAD  0x61
#define NRF24_CMD_W_TX_PAYLOAD  0xA0
#define NRF24_CMD_FLUSH_TX      0xE1
#define NRF24_CMD_FLUSH_RX      0xE2
#define NRF24_CMD_REUSE_TX_PL   0xE3
#define NRF24_CMD_NOP           0xFF

#if POCKETOS_NRF24L01_ENABLE_REGISTER_ACCESS
// Complete nRF24L01+ Register Map (0x00-0x1D, 30 registers)
static const RegisterDesc NRF24L01_REGISTERS[] = {
    RegisterDesc(0x00, "CONFIG", 1, RegisterAccess::RW, 0x08),
    RegisterDesc(0x01, "EN_AA", 1, RegisterAccess::RW, 0x3F),
    RegisterDesc(0x02, "EN_RXADDR", 1, RegisterAccess::RW, 0x03),
    RegisterDesc(0x03, "SETUP_AW", 1, RegisterAccess::RW, 0x03),
    RegisterDesc(0x04, "SETUP_RETR", 1, RegisterAccess::RW, 0x03),
    RegisterDesc(0x05, "RF_CH", 1, RegisterAccess::RW, 0x02),
    RegisterDesc(0x06, "RF_SETUP", 1, RegisterAccess::RW, 0x0E),
    RegisterDesc(0x07, "STATUS", 1, RegisterAccess::RW, 0x0E),
    RegisterDesc(0x08, "OBSERVE_TX", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x09, "RPD", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0A, "RX_ADDR_P0", 5, RegisterAccess::RW, 0xE7E7E7E7E7),
    RegisterDesc(0x0B, "RX_ADDR_P1", 5, RegisterAccess::RW, 0xC2C2C2C2C2),
    RegisterDesc(0x0C, "RX_ADDR_P2", 1, RegisterAccess::RW, 0xC3),
    RegisterDesc(0x0D, "RX_ADDR_P3", 1, RegisterAccess::RW, 0xC4),
    RegisterDesc(0x0E, "RX_ADDR_P4", 1, RegisterAccess::RW, 0xC5),
    RegisterDesc(0x0F, "RX_ADDR_P5", 1, RegisterAccess::RW, 0xC6),
    RegisterDesc(0x10, "TX_ADDR", 5, RegisterAccess::RW, 0xE7E7E7E7E7),
    RegisterDesc(0x11, "RX_PW_P0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x12, "RX_PW_P1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x13, "RX_PW_P2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x14, "RX_PW_P3", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x15, "RX_PW_P4", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x16, "RX_PW_P5", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x17, "FIFO_STATUS", 1, RegisterAccess::RO, 0x11),
    RegisterDesc(0x1C, "DYNPD", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1D, "FEATURE", 1, RegisterAccess::RW, 0x00),
};

#define NRF24L01_REGISTER_COUNT (sizeof(NRF24L01_REGISTERS) / sizeof(RegisterDesc))
#endif

NRF24L01Driver::NRF24L01Driver() 
    : initialized_(false), ce_pin_(-1) {
    setRegisterConvention(SPIRegisterConvention::NRF24);
}

NRF24L01Driver::~NRF24L01Driver() {
    if (initialized_ && ce_pin_ >= 0) {
        setCE(false);
        ResourceManager::release(ResourceType::GPIO_PIN, ce_pin_, "nrf24l01_ce");
    }
    deinit();
}

bool NRF24L01Driver::init(const String& endpoint) {
    if (!initFromEndpoint(endpoint)) {
        return false;
    }
    
    // Parse CE pin from endpoint (required for nRF24L01+)
    int ceIdx = endpoint.indexOf("ce=");
    if (ceIdx < 0) {
        Logger::error("NRF24L01: CE pin is required");
        deinit();
        return false;
    }
    
    // Extract CE pin number
    int commaIdx = endpoint.indexOf(',', ceIdx);
    String cePinStr;
    if (commaIdx > 0) {
        cePinStr = endpoint.substring(ceIdx + 3, commaIdx);
    } else {
        cePinStr = endpoint.substring(ceIdx + 3);
    }
    ce_pin_ = cePinStr.toInt();
    
    // Claim CE pin
    if (!ResourceManager::claim(ResourceType::GPIO_PIN, ce_pin_, "nrf24l01_ce")) {
        Logger::error("NRF24L01: Failed to claim CE pin");
        deinit();
        return false;
    }
    
    // Configure CE pin
    pinMode(ce_pin_, OUTPUT);
    setCE(false);
    
    delay(100);  // Power on reset delay
    
    // Verify communication by reading CONFIG register
    uint8_t config;
    if (!readRegister(0x00, &config, 1)) {
        Logger::error("NRF24L01: Failed to read CONFIG register");
        deinit();
        return false;
    }
    
    // Power up and set to RX mode
    config = 0x0F;  // PWR_UP | PRIM_RX | EN_CRC | CRCO
    if (!writeRegister(0x00, &config, 1)) {
        deinit();
        return false;
    }
    
    delay(5);  // Power up delay
    
    initialized_ = true;
    Logger::info("NRF24L01: Initialized successfully");
    return true;
}

bool NRF24L01Driver::validEndpoints(const String& endpoint) {
    // Format: spi0:cs=5,ce=16,irq=4
    if (!endpoint.startsWith("spi")) {
        return false;
    }
    
    int colonIdx = endpoint.indexOf(':');
    if (colonIdx < 0) {
        return false;
    }
    
    // Must have CS and CE pins
    return endpoint.indexOf("cs=") > colonIdx && endpoint.indexOf("ce=") > colonIdx;
}

bool NRF24L01Driver::identifyProbe(const String& endpoint) {
    NRF24L01Driver driver;
    if (!driver.init(endpoint)) {
        return false;
    }
    
    // Read CONFIG register
    uint8_t config;
    if (!driver.readRegister(0x00, &config, 1)) {
        return false;
    }
    
    // CONFIG should have valid bits set
    return (config & 0x08) != 0;  // Check PWR_UP bit
}

void NRF24L01Driver::setCE(bool active) {
    if (ce_pin_ >= 0) {
        digitalWrite(ce_pin_, active ? HIGH : LOW);
    }
}

bool NRF24L01Driver::writeCommand(uint8_t cmd) {
    return spiWrite(&cmd, 1);
}

bool NRF24L01Driver::readRegister(uint8_t reg, uint8_t* data, uint8_t len) {
    uint8_t cmd = NRF24_CMD_R_REGISTER | (reg & 0x1F);
    return spiWriteRead(&cmd, 1, data, len);
}

bool NRF24L01Driver::writeRegister(uint8_t reg, const uint8_t* data, uint8_t len) {
    beginTransaction();
    uint8_t cmd = NRF24_CMD_W_REGISTER | (reg & 0x1F);
    SPI.transfer(cmd);
    for (uint8_t i = 0; i < len; i++) {
        SPI.transfer(data[i]);
    }
    endTransaction();
    return true;
}

#if POCKETOS_NRF24L01_ENABLE_BASIC_READ
bool NRF24L01Driver::transmit(const uint8_t* data, uint8_t len) {
    if (!initialized_ || len > 32) {
        return false;
    }
    
    // Switch to TX mode
    uint8_t config;
    if (!readRegister(0x00, &config, 1)) {
        return false;
    }
    
    config &= ~0x01;  // Clear PRIM_RX (TX mode)
    if (!writeRegister(0x00, &config, 1)) {
        return false;
    }
    
    // Write payload
    beginTransaction();
    SPI.transfer(NRF24_CMD_W_TX_PAYLOAD);
    for (uint8_t i = 0; i < len; i++) {
        SPI.transfer(data[i]);
    }
    endTransaction();
    
    // Pulse CE to start transmission
    setCE(true);
    delayMicroseconds(15);
    setCE(false);
    
    // Wait for transmission (simplified)
    delay(1);
    
    return true;
}

bool NRF24L01Driver::receive(uint8_t* data, uint8_t& len) {
    if (!initialized_) {
        return false;
    }
    
    if (!available()) {
        return false;
    }
    
    // Read payload length (assuming fixed size for simplicity)
    len = 32;
    
    // Read payload
    beginTransaction();
    SPI.transfer(NRF24_CMD_R_RX_PAYLOAD);
    for (uint8_t i = 0; i < len; i++) {
        data[i] = SPI.transfer(0xFF);
    }
    endTransaction();
    
    // Clear RX_DR flag
    uint8_t status = 0x40;
    writeRegister(0x07, &status, 1);
    
    return true;
}

bool NRF24L01Driver::available() {
    uint8_t status;
    if (!readRegister(0x07, &status, 1)) {
        return false;
    }
    
    return (status & 0x40) != 0;  // RX_DR flag
}

bool NRF24L01Driver::setPowerUp(bool powerUp) {
    if (!initialized_) {
        return false;
    }
    
    uint8_t config;
    if (!readRegister(0x00, &config, 1)) {
        return false;
    }
    
    if (powerUp) {
        config |= 0x02;
    } else {
        config &= ~0x02;
    }
    
    return writeRegister(0x00, &config, 1);
}

bool NRF24L01Driver::setChannel(uint8_t channel) {
    if (!initialized_ || channel > 125) {
        return false;
    }
    
    return writeRegister(0x05, &channel, 1);
}

bool NRF24L01Driver::setDataRate(uint8_t rate) {
    if (!initialized_ || rate > 2) {
        return false;
    }
    
    uint8_t rf_setup;
    if (!readRegister(0x06, &rf_setup, 1)) {
        return false;
    }
    
    rf_setup &= ~0x28;  // Clear RF_DR bits
    
    if (rate == 0) {
        // 1Mbps - both bits 0
    } else if (rate == 1) {
        rf_setup |= 0x08;  // 2Mbps
    } else if (rate == 2) {
        rf_setup |= 0x20;  // 250kbps
    }
    
    return writeRegister(0x06, &rf_setup, 1);
}
#endif

#if POCKETOS_NRF24L01_ENABLE_ERROR_HANDLING
bool NRF24L01Driver::setAutoAck(bool enable) {
    if (!initialized_) {
        return false;
    }
    
    uint8_t en_aa = enable ? 0x3F : 0x00;
    return writeRegister(0x01, &en_aa, 1);
}

bool NRF24L01Driver::setPayloadSize(uint8_t pipe, uint8_t size) {
    if (!initialized_ || pipe > 5 || size > 32) {
        return false;
    }
    
    uint8_t reg = 0x11 + pipe;
    return writeRegister(reg, &size, 1);
}

bool NRF24L01Driver::openReadingPipe(uint8_t pipe, uint64_t address) {
    if (!initialized_ || pipe > 5) {
        return false;
    }
    
    uint8_t addr[5];
    for (int i = 0; i < 5; i++) {
        addr[i] = (address >> (i * 8)) & 0xFF;
    }
    
    uint8_t reg = 0x0A + pipe;
    uint8_t width = (pipe < 2) ? 5 : 1;
    
    if (!writeRegister(reg, addr, width)) {
        return false;
    }
    
    // Enable pipe
    uint8_t en_rxaddr;
    if (!readRegister(0x02, &en_rxaddr, 1)) {
        return false;
    }
    
    en_rxaddr |= (1 << pipe);
    return writeRegister(0x02, &en_rxaddr, 1);
}

bool NRF24L01Driver::openWritingPipe(uint64_t address) {
    if (!initialized_) {
        return false;
    }
    
    uint8_t addr[5];
    for (int i = 0; i < 5; i++) {
        addr[i] = (address >> (i * 8)) & 0xFF;
    }
    
    return writeRegister(0x10, addr, 5);
}

bool NRF24L01Driver::setPowerMode(uint8_t mode) {
    if (!initialized_ || mode > 3) {
        return false;
    }
    
    uint8_t config;
    if (!readRegister(0x00, &config, 1)) {
        return false;
    }
    
    if (mode == 0) {
        // Power down
        config &= ~0x02;
        setCE(false);
    } else if (mode == 1) {
        // Standby
        config |= 0x02;
        setCE(false);
    } else if (mode == 2) {
        // RX mode
        config |= 0x03;
        if (!writeRegister(0x00, &config, 1)) {
            return false;
        }
        setCE(true);
        return true;
    } else if (mode == 3) {
        // TX mode
        config &= ~0x01;
        config |= 0x02;
        setCE(false);
    }
    
    return writeRegister(0x00, &config, 1);
}

uint8_t NRF24L01Driver::getStatus() {
    uint8_t status;
    if (!readRegister(0x07, &status, 1)) {
        return 0xFF;
    }
    return status;
}

void NRF24L01Driver::flushTx() {
    writeCommand(NRF24_CMD_FLUSH_TX);
}

void NRF24L01Driver::flushRx() {
    writeCommand(NRF24_CMD_FLUSH_RX);
}
#endif

#if POCKETOS_NRF24L01_ENABLE_REGISTER_ACCESS
const RegisterDesc* NRF24L01Driver::registers(size_t& count) const {
    count = NRF24L01_REGISTER_COUNT;
    return NRF24L01_REGISTERS;
}

bool NRF24L01Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (reg > 0x1D) {
        return false;
    }
    return readRegister((uint8_t)reg, buf, len);
}

bool NRF24L01Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (reg > 0x1D) {
        return false;
    }
    return writeRegister((uint8_t)reg, buf, len);
}

const RegisterDesc* NRF24L01Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(NRF24L01_REGISTERS, NRF24L01_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
