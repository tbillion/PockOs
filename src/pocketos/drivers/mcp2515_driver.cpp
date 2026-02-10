#include "mcp2515_driver.h"
#include "../core/logger.h"
#include <SPI.h>

namespace PocketOS {

// MCP2515 SPI Commands
#define MCP2515_CMD_RESET       0xC0
#define MCP2515_CMD_READ        0x03
#define MCP2515_CMD_WRITE       0x02
#define MCP2515_CMD_READ_STATUS 0xA0
#define MCP2515_CMD_BIT_MODIFY  0x05

// MCP2515 Modes
#define MCP2515_MODE_NORMAL     0x00
#define MCP2515_MODE_SLEEP      0x20
#define MCP2515_MODE_LOOPBACK   0x40
#define MCP2515_MODE_LISTENONLY 0x60
#define MCP2515_MODE_CONFIG     0x80

#if POCKETOS_MCP2515_ENABLE_REGISTER_ACCESS
// Complete MCP2515 Register Map (128 registers, 0x00-0x7F)
static const RegisterDesc MCP2515_REGISTERS[] = {
    // RX Buffer 0
    RegisterDesc(0x00, "RXF0SIDH", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "RXF0SIDL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x02, "RXF0EID8", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x03, "RXF0EID0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x04, "RXF1SIDH", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x05, "RXF1SIDL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x06, "RXF1EID8", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x07, "RXF1EID0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x08, "RXF2SIDH", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x09, "RXF2SIDL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0A, "RXF2EID8", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0B, "RXF2EID0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0C, "BFPCTRL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0D, "TXRTSCTRL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0E, "CANSTAT", 1, RegisterAccess::RO, 0x80),
    RegisterDesc(0x0F, "CANCTRL", 1, RegisterAccess::RW, 0x87),
    
    // RX Filter 3-5
    RegisterDesc(0x10, "RXF3SIDH", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x11, "RXF3SIDL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x12, "RXF3EID8", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x13, "RXF3EID0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x14, "RXF4SIDH", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x15, "RXF4SIDL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x16, "RXF4EID8", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x17, "RXF4EID0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x18, "RXF5SIDH", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x19, "RXF5SIDL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1A, "RXF5EID8", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1B, "RXF5EID0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1C, "TEC", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1D, "REC", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1E, "CANSTAT", 1, RegisterAccess::RO, 0x80),
    RegisterDesc(0x1F, "CANCTRL", 1, RegisterAccess::RW, 0x87),
    
    // RX Masks
    RegisterDesc(0x20, "RXM0SIDH", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x21, "RXM0SIDL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x22, "RXM0EID8", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x23, "RXM0EID0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x24, "RXM1SIDH", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x25, "RXM1SIDL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x26, "RXM1EID8", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x27, "RXM1EID0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x28, "CNF3", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x29, "CNF2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2A, "CNF1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2B, "CANINTE", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2C, "CANINTF", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2D, "EFLG", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x2E, "CANSTAT", 1, RegisterAccess::RO, 0x80),
    RegisterDesc(0x2F, "CANCTRL", 1, RegisterAccess::RW, 0x87),
    
    // TX Buffer 0
    RegisterDesc(0x30, "TXB0CTRL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x31, "TXB0SIDH", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x32, "TXB0SIDL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x33, "TXB0EID8", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x34, "TXB0EID0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x35, "TXB0DLC", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x36, "TXB0D0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x37, "TXB0D1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x38, "TXB0D2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x39, "TXB0D3", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x3A, "TXB0D4", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x3B, "TXB0D5", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x3C, "TXB0D6", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x3D, "TXB0D7", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x3E, "CANSTAT", 1, RegisterAccess::RO, 0x80),
    RegisterDesc(0x3F, "CANCTRL", 1, RegisterAccess::RW, 0x87),
    
    // TX Buffer 1
    RegisterDesc(0x40, "TXB1CTRL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x41, "TXB1SIDH", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x42, "TXB1SIDL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x43, "TXB1EID8", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x44, "TXB1EID0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x45, "TXB1DLC", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x46, "TXB1D0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x47, "TXB1D1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x48, "TXB1D2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x49, "TXB1D3", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x4A, "TXB1D4", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x4B, "TXB1D5", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x4C, "TXB1D6", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x4D, "TXB1D7", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x4E, "CANSTAT", 1, RegisterAccess::RO, 0x80),
    RegisterDesc(0x4F, "CANCTRL", 1, RegisterAccess::RW, 0x87),
    
    // TX Buffer 2
    RegisterDesc(0x50, "TXB2CTRL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x51, "TXB2SIDH", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x52, "TXB2SIDL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x53, "TXB2EID8", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x54, "TXB2EID0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x55, "TXB2DLC", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x56, "TXB2D0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x57, "TXB2D1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x58, "TXB2D2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x59, "TXB2D3", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x5A, "TXB2D4", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x5B, "TXB2D5", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x5C, "TXB2D6", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x5D, "TXB2D7", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x5E, "CANSTAT", 1, RegisterAccess::RO, 0x80),
    RegisterDesc(0x5F, "CANCTRL", 1, RegisterAccess::RW, 0x87),
    
    // RX Buffer 0
    RegisterDesc(0x60, "RXB0CTRL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x61, "RXB0SIDH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x62, "RXB0SIDL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x63, "RXB0EID8", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x64, "RXB0EID0", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x65, "RXB0DLC", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x66, "RXB0D0", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x67, "RXB0D1", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x68, "RXB0D2", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x69, "RXB0D3", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x6A, "RXB0D4", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x6B, "RXB0D5", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x6C, "RXB0D6", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x6D, "RXB0D7", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x6E, "CANSTAT", 1, RegisterAccess::RO, 0x80),
    RegisterDesc(0x6F, "CANCTRL", 1, RegisterAccess::RW, 0x87),
    
    // RX Buffer 1
    RegisterDesc(0x70, "RXB1CTRL", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x71, "RXB1SIDH", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x72, "RXB1SIDL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x73, "RXB1EID8", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x74, "RXB1EID0", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x75, "RXB1DLC", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x76, "RXB1D0", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x77, "RXB1D1", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x78, "RXB1D2", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x79, "RXB1D3", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x7A, "RXB1D4", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x7B, "RXB1D5", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x7C, "RXB1D6", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x7D, "RXB1D7", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x7E, "CANSTAT", 1, RegisterAccess::RO, 0x80),
    RegisterDesc(0x7F, "CANCTRL", 1, RegisterAccess::RW, 0x87),
};

#define MCP2515_REGISTER_COUNT (sizeof(MCP2515_REGISTERS) / sizeof(RegisterDesc))
#endif

MCP2515Driver::MCP2515Driver() 
    : initialized_(false), oscillator_mhz_(16) {
    setRegisterConvention(SPIRegisterConvention::MCP2515);
}

MCP2515Driver::~MCP2515Driver() {
    deinit();
}

bool MCP2515Driver::init(const String& endpoint) {
    if (!initFromEndpoint(endpoint)) {
        return false;
    }
    
    if (!reset()) {
        deinit();
        return false;
    }
    
    delay(10);
    
    // Verify communication by reading CANSTAT
    uint8_t canstat;
    if (!regRead(0x0E, &canstat, 1)) {
        deinit();
        return false;
    }
    
    // Should be in config mode after reset (0x80)
    if ((canstat & 0xE0) != 0x80) {
        Logger::error("MCP2515: Failed to verify device (CANSTAT=" + String(canstat, HEX) + ")");
        deinit();
        return false;
    }
    
    initialized_ = true;
    Logger::info("MCP2515: Initialized successfully");
    return true;
}

bool MCP2515Driver::validEndpoints(const String& endpoint) {
    // Format: spi0:cs=5,irq=4
    if (!endpoint.startsWith("spi")) {
        return false;
    }
    
    int colonIdx = endpoint.indexOf(':');
    if (colonIdx < 0) {
        return false;
    }
    
    // Must have CS pin
    return endpoint.indexOf("cs=") > colonIdx;
}

bool MCP2515Driver::identifyProbe(const String& endpoint) {
    MCP2515Driver driver;
    if (!driver.init(endpoint)) {
        return false;
    }
    
    // Read CANSTAT - should be in config mode (0x80)
    uint8_t canstat;
    if (!driver.regRead(0x0E, &canstat, 1)) {
        return false;
    }
    
    return (canstat & 0xE0) == 0x80;
}

bool MCP2515Driver::reset() {
    uint8_t cmd = MCP2515_CMD_RESET;
    return spiWrite(&cmd, 1);
}

#if POCKETOS_MCP2515_ENABLE_BASIC_READ
bool MCP2515Driver::sendFrame(uint32_t id, const uint8_t* data, uint8_t len, bool extended) {
    if (!initialized_ || len > 8) {
        return false;
    }
    
    // Use TX buffer 0
    uint8_t txbuf[13];
    txbuf[0] = 0x00;  // TXB0CTRL - not requesting transmission yet
    
    // Setup ID registers
    if (extended) {
        txbuf[1] = (uint8_t)(id >> 21);  // SIDH
        txbuf[2] = (uint8_t)(((id >> 13) & 0xE0) | 0x08 | ((id >> 16) & 0x03));  // SIDL
        txbuf[3] = (uint8_t)(id >> 8);   // EID8
        txbuf[4] = (uint8_t)(id);        // EID0
    } else {
        txbuf[1] = (uint8_t)(id >> 3);   // SIDH
        txbuf[2] = (uint8_t)(id << 5);   // SIDL
        txbuf[3] = 0;                    // EID8
        txbuf[4] = 0;                    // EID0
    }
    
    txbuf[5] = len;  // DLC
    
    // Copy data
    for (uint8_t i = 0; i < len; i++) {
        txbuf[6 + i] = data[i];
    }
    
    // Write to TX buffer
    if (!regWrite(0x30, txbuf, 6 + len)) {
        return false;
    }
    
    // Request transmission
    return modifyRegister(0x30, 0x08, 0x08);
}

bool MCP2515Driver::receiveFrame(uint32_t& id, uint8_t* data, uint8_t& len, bool& extended) {
    if (!initialized_) {
        return false;
    }
    
    // Check if RX buffer 0 has data
    uint8_t status;
    if (!readStatus(status)) {
        return false;
    }
    
    if ((status & 0x01) == 0) {
        return false;  // No message
    }
    
    // Read RX buffer 0
    uint8_t rxbuf[13];
    if (!regRead(0x61, rxbuf, 13)) {
        return false;
    }
    
    // Parse ID
    extended = (rxbuf[1] & 0x08) != 0;
    if (extended) {
        id = ((uint32_t)rxbuf[0] << 21) | ((uint32_t)(rxbuf[1] & 0xE0) << 13) |
             ((uint32_t)(rxbuf[1] & 0x03) << 16) | ((uint32_t)rxbuf[2] << 8) | rxbuf[3];
    } else {
        id = ((uint32_t)rxbuf[0] << 3) | (rxbuf[1] >> 5);
    }
    
    // Parse length
    len = rxbuf[4] & 0x0F;
    if (len > 8) {
        len = 8;
    }
    
    // Copy data
    for (uint8_t i = 0; i < len; i++) {
        data[i] = rxbuf[5 + i];
    }
    
    // Clear interrupt flag
    return modifyRegister(0x2C, 0x01, 0x00);
}

bool MCP2515Driver::setMode(uint8_t mode) {
    if (!initialized_) {
        return false;
    }
    
    return modifyRegister(0x0F, 0xE0, mode);
}

bool MCP2515Driver::setBitrate(uint32_t bitrate, uint8_t oscillator_mhz) {
    if (!initialized_) {
        return false;
    }
    
    oscillator_mhz_ = oscillator_mhz;
    
    // Simple bitrate calculation for common rates
    uint8_t cnf1, cnf2, cnf3;
    
    if (oscillator_mhz == 16 && bitrate == 500000) {
        cnf1 = 0x00; cnf2 = 0x90; cnf3 = 0x02;
    } else if (oscillator_mhz == 16 && bitrate == 250000) {
        cnf1 = 0x01; cnf2 = 0x90; cnf3 = 0x02;
    } else if (oscillator_mhz == 16 && bitrate == 125000) {
        cnf1 = 0x03; cnf2 = 0x90; cnf3 = 0x02;
    } else {
        return false;  // Unsupported combination
    }
    
    return regWrite(0x2A, &cnf1, 1) && 
           regWrite(0x29, &cnf2, 1) && 
           regWrite(0x28, &cnf3, 1);
}
#endif

#if POCKETOS_MCP2515_ENABLE_ERROR_HANDLING
bool MCP2515Driver::setFilter(uint8_t filter_num, uint32_t mask, bool extended) {
    if (!initialized_ || filter_num > 5) {
        return false;
    }
    
    uint8_t regs[4];
    uint8_t base_addr = 0x00 + (filter_num * 4);
    
    if (extended) {
        regs[0] = (uint8_t)(mask >> 21);
        regs[1] = (uint8_t)(((mask >> 13) & 0xE0) | 0x08 | ((mask >> 16) & 0x03));
        regs[2] = (uint8_t)(mask >> 8);
        regs[3] = (uint8_t)(mask);
    } else {
        regs[0] = (uint8_t)(mask >> 3);
        regs[1] = (uint8_t)(mask << 5);
        regs[2] = 0;
        regs[3] = 0;
    }
    
    return regWrite(base_addr, regs, 4);
}

bool MCP2515Driver::setMask(uint8_t mask_num, uint32_t mask, bool extended) {
    if (!initialized_ || mask_num > 1) {
        return false;
    }
    
    uint8_t regs[4];
    uint8_t base_addr = 0x20 + (mask_num * 4);
    
    if (extended) {
        regs[0] = (uint8_t)(mask >> 21);
        regs[1] = (uint8_t)(((mask >> 13) & 0xE0) | 0x08 | ((mask >> 16) & 0x03));
        regs[2] = (uint8_t)(mask >> 8);
        regs[3] = (uint8_t)(mask);
    } else {
        regs[0] = (uint8_t)(mask >> 3);
        regs[1] = (uint8_t)(mask << 5);
        regs[2] = 0;
        regs[3] = 0;
    }
    
    return regWrite(base_addr, regs, 4);
}

uint8_t MCP2515Driver::getErrorFlags() {
    uint8_t eflg;
    if (!regRead(0x2D, &eflg, 1)) {
        return 0xFF;
    }
    return eflg;
}

uint8_t MCP2515Driver::getRxErrors() {
    uint8_t rec;
    if (!regRead(0x1D, &rec, 1)) {
        return 0xFF;
    }
    return rec;
}

uint8_t MCP2515Driver::getTxErrors() {
    uint8_t tec;
    if (!regRead(0x1C, &tec, 1)) {
        return 0xFF;
    }
    return tec;
}

void MCP2515Driver::clearErrors() {
    uint8_t clear = 0x00;
    regWrite(0x2D, &clear, 1);
}
#endif

bool MCP2515Driver::readStatus(uint8_t& status) {
    uint8_t cmd = MCP2515_CMD_READ_STATUS;
    return spiWriteRead(&cmd, 1, &status, 1);
}

bool MCP2515Driver::modifyRegister(uint8_t reg, uint8_t mask, uint8_t value) {
    beginTransaction();
    SPI.transfer(MCP2515_CMD_BIT_MODIFY);
    SPI.transfer(reg);
    SPI.transfer(mask);
    SPI.transfer(value);
    endTransaction();
    return true;
}

#if POCKETOS_MCP2515_ENABLE_REGISTER_ACCESS
const RegisterDesc* MCP2515Driver::registers(size_t& count) const {
    count = MCP2515_REGISTER_COUNT;
    return MCP2515_REGISTERS;
}

bool MCP2515Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    return SPIDriverBase::regRead(reg, buf, len);
}

bool MCP2515Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    return SPIDriverBase::regWrite(reg, buf, len);
}

const RegisterDesc* MCP2515Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(MCP2515_REGISTERS, MCP2515_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
