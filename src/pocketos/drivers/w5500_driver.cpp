#include "w5500_driver.h"
#include "../core/logger.h"
#include <SPI.h>

namespace PocketOS {

// W5500 Block Select Bits
#define W5500_BSB_COMMON_REG    0x00
#define W5500_BSB_S0_REG        0x08
#define W5500_BSB_S0_TX_BUF     0x10
#define W5500_BSB_S0_RX_BUF     0x18

// W5500 Socket Commands
#define W5500_CMD_OPEN          0x01
#define W5500_CMD_LISTEN        0x02
#define W5500_CMD_CONNECT       0x04
#define W5500_CMD_DISCON        0x08
#define W5500_CMD_CLOSE         0x10
#define W5500_CMD_SEND          0x20
#define W5500_CMD_RECV          0x40

// W5500 Socket Status
#define W5500_SOCK_CLOSED       0x00
#define W5500_SOCK_INIT         0x13
#define W5500_SOCK_LISTEN       0x14
#define W5500_SOCK_ESTABLISHED  0x17
#define W5500_SOCK_CLOSE_WAIT   0x1C
#define W5500_SOCK_UDP          0x22

// W5500 Protocols
#define W5500_PROTO_TCP         0x01
#define W5500_PROTO_UDP         0x02

#if POCKETOS_W5500_ENABLE_REGISTER_ACCESS
// Complete W5500 Register Map (Common and Socket registers)
static const RegisterDesc W5500_REGISTERS[] = {
    // Common registers (0x0000-0x0039)
    RegisterDesc(0x0000, "MR", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0001, "GAR0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0002, "GAR1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0003, "GAR2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0004, "GAR3", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0005, "SUBR0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0006, "SUBR1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0007, "SUBR2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0008, "SUBR3", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0009, "SHAR0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x000A, "SHAR1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x000B, "SHAR2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x000C, "SHAR3", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x000D, "SHAR4", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x000E, "SHAR5", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x000F, "SIPR0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0010, "SIPR1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0011, "SIPR2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0012, "SIPR3", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0013, "INTLEVEL0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0014, "INTLEVEL1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0015, "IR", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0016, "IMR", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0017, "SIR", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0018, "SIMR", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0019, "RTR0", 1, RegisterAccess::RW, 0x07),
    RegisterDesc(0x001A, "RTR1", 1, RegisterAccess::RW, 0xD0),
    RegisterDesc(0x001B, "RCR", 1, RegisterAccess::RW, 0x08),
    RegisterDesc(0x001C, "PTIMER", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x001D, "PMAGIC", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x001E, "PHAR0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x001F, "PHAR1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0020, "PHAR2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0021, "PHAR3", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0022, "PHAR4", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0023, "PHAR5", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0024, "PSID0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0025, "PSID1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0026, "PMRU0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0027, "PMRU1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0028, "UIPR0", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0029, "UIPR1", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x002A, "UIPR2", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x002B, "UIPR3", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x002C, "UPORTR0", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x002D, "UPORTR1", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x002E, "PHYCFGR", 1, RegisterAccess::RW, 0xB8),
    RegisterDesc(0x0039, "VERSIONR", 1, RegisterAccess::RO, 0x04),
    
    // Socket 0 registers (0x0000-0x002F in socket block)
    RegisterDesc(0x1000, "S0_MR", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1001, "S0_CR", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1002, "S0_IR", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1003, "S0_SR", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1004, "S0_PORT0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1005, "S0_PORT1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1006, "S0_DHAR0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1007, "S0_DHAR1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1008, "S0_DHAR2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1009, "S0_DHAR3", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x100A, "S0_DHAR4", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x100B, "S0_DHAR5", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x100C, "S0_DIPR0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x100D, "S0_DIPR1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x100E, "S0_DIPR2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x100F, "S0_DIPR3", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1010, "S0_DPORT0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1011, "S0_DPORT1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1012, "S0_MSSR0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1013, "S0_MSSR1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1015, "S0_TOS", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1016, "S0_TTL", 1, RegisterAccess::RW, 0x80),
    RegisterDesc(0x1020, "S0_TX_FSR0", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1021, "S0_TX_FSR1", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1022, "S0_TX_RD0", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1023, "S0_TX_RD1", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1024, "S0_TX_WR0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1025, "S0_TX_WR1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1026, "S0_RX_RSR0", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1027, "S0_RX_RSR1", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1028, "S0_RX_RD0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x1029, "S0_RX_RD1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x102A, "S0_RX_WR0", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x102B, "S0_RX_WR1", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x102C, "S0_IMR", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x102D, "S0_FRAG0", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x102E, "S0_FRAG1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x102F, "S0_KPALVTR", 1, RegisterAccess::RW, 0x00),
};

#define W5500_REGISTER_COUNT (sizeof(W5500_REGISTERS) / sizeof(RegisterDesc))
#endif

W5500Driver::W5500Driver() 
    : initialized_(false) {
    setRegisterConvention(SPIRegisterConvention::GENERIC);
}

W5500Driver::~W5500Driver() {
    deinit();
}

bool W5500Driver::init(const String& endpoint) {
    if (!initFromEndpoint(endpoint)) {
        return false;
    }
    
    // Perform hardware reset if RST pin available
    if (getPinConfig().rst >= 0) {
        setRST(true);
        delay(10);
        setRST(false);
        delay(200);  // Wait for W5500 to initialize
    }
    
    // Read version register
    uint8_t version = readByte(W5500_BSB_COMMON_REG, 0x0039);
    if (version != 0x04) {
        Logger::error("W5500: Invalid version: 0x" + String(version, HEX));
        deinit();
        return false;
    }
    
    initialized_ = true;
    Logger::info("W5500: Initialized successfully (version 0x" + String(version, HEX) + ")");
    return true;
}

bool W5500Driver::validEndpoints(const String& endpoint) {
    // Format: spi0:cs=5,rst=17,irq=4
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

bool W5500Driver::identifyProbe(const String& endpoint) {
    W5500Driver driver;
    if (!driver.init(endpoint)) {
        return false;
    }
    
    // Read VERSIONR register
    uint8_t version = driver.readByte(W5500_BSB_COMMON_REG, 0x0039);
    return version == 0x04;
}

uint8_t W5500Driver::getSocketBlockBase(uint8_t socket) const {
    return W5500_BSB_S0_REG + (socket * 0x20);
}

bool W5500Driver::readReg(uint8_t block, uint16_t addr, uint8_t* data, uint16_t len) {
    if (!initialized_) {
        return false;
    }
    
    beginTransaction();
    
    // Send address (high byte, low byte)
    SPI.transfer((addr >> 8) & 0xFF);
    SPI.transfer(addr & 0xFF);
    
    // Send control byte (block select + read mode)
    SPI.transfer((block << 3) | 0x00);
    
    // Read data
    for (uint16_t i = 0; i < len; i++) {
        data[i] = SPI.transfer(0x00);
    }
    
    endTransaction();
    return true;
}

bool W5500Driver::writeReg(uint8_t block, uint16_t addr, const uint8_t* data, uint16_t len) {
    if (!initialized_) {
        return false;
    }
    
    beginTransaction();
    
    // Send address (high byte, low byte)
    SPI.transfer((addr >> 8) & 0xFF);
    SPI.transfer(addr & 0xFF);
    
    // Send control byte (block select + write mode)
    SPI.transfer((block << 3) | 0x04);
    
    // Write data
    for (uint16_t i = 0; i < len; i++) {
        SPI.transfer(data[i]);
    }
    
    endTransaction();
    return true;
}

uint8_t W5500Driver::readByte(uint8_t block, uint16_t addr) {
    uint8_t data;
    readReg(block, addr, &data, 1);
    return data;
}

void W5500Driver::writeByte(uint8_t block, uint16_t addr, uint8_t value) {
    writeReg(block, addr, &value, 1);
}

uint16_t W5500Driver::readWord(uint8_t block, uint16_t addr) {
    uint8_t data[2];
    readReg(block, addr, data, 2);
    return ((uint16_t)data[0] << 8) | data[1];
}

void W5500Driver::writeWord(uint8_t block, uint16_t addr, uint16_t value) {
    uint8_t data[2] = { (uint8_t)(value >> 8), (uint8_t)(value & 0xFF) };
    writeReg(block, addr, data, 2);
}

#if POCKETOS_W5500_ENABLE_BASIC_READ
bool W5500Driver::socketOpen(uint8_t socket, uint8_t protocol, uint16_t port) {
    if (!initialized_ || socket > 7) {
        return false;
    }
    
    uint8_t block = getSocketBlockBase(socket);
    
    // Close socket if open
    writeByte(block, 0x0001, W5500_CMD_CLOSE);
    delay(1);
    
    // Set protocol
    writeByte(block, 0x0000, protocol);
    
    // Set port
    writeWord(block, 0x0004, port);
    
    // Open socket
    writeByte(block, 0x0001, W5500_CMD_OPEN);
    delay(1);
    
    return socketStatus(socket) != W5500_SOCK_CLOSED;
}

bool W5500Driver::socketClose(uint8_t socket) {
    if (!initialized_ || socket > 7) {
        return false;
    }
    
    uint8_t block = getSocketBlockBase(socket);
    writeByte(block, 0x0001, W5500_CMD_CLOSE);
    delay(1);
    
    return true;
}

bool W5500Driver::socketConnect(uint8_t socket, const uint8_t* ip, uint16_t port) {
    if (!initialized_ || socket > 7) {
        return false;
    }
    
    uint8_t block = getSocketBlockBase(socket);
    
    // Set destination IP
    writeReg(block, 0x000C, ip, 4);
    
    // Set destination port
    writeWord(block, 0x0010, port);
    
    // Connect
    writeByte(block, 0x0001, W5500_CMD_CONNECT);
    
    return true;
}

bool W5500Driver::socketListen(uint8_t socket) {
    if (!initialized_ || socket > 7) {
        return false;
    }
    
    uint8_t block = getSocketBlockBase(socket);
    writeByte(block, 0x0001, W5500_CMD_LISTEN);
    
    return true;
}

int16_t W5500Driver::socketSend(uint8_t socket, const uint8_t* data, uint16_t len) {
    if (!initialized_ || socket > 7 || len == 0) {
        return -1;
    }
    
    uint8_t block_reg = getSocketBlockBase(socket);
    uint8_t block_tx = W5500_BSB_S0_TX_BUF + (socket * 0x20);
    
    // Get TX write pointer
    uint16_t ptr = readWord(block_reg, 0x0024);
    
    // Write data to TX buffer
    writeReg(block_tx, ptr, data, len);
    
    // Update TX write pointer
    ptr += len;
    writeWord(block_reg, 0x0024, ptr);
    
    // Send command
    writeByte(block_reg, 0x0001, W5500_CMD_SEND);
    
    return len;
}

int16_t W5500Driver::socketRecv(uint8_t socket, uint8_t* data, uint16_t len) {
    if (!initialized_ || socket > 7) {
        return -1;
    }
    
    uint8_t block_reg = getSocketBlockBase(socket);
    uint8_t block_rx = W5500_BSB_S0_RX_BUF + (socket * 0x20);
    
    // Get RX received size
    uint16_t recv_size = readWord(block_reg, 0x0026);
    if (recv_size == 0) {
        return 0;
    }
    
    // Limit to requested length
    if (recv_size > len) {
        recv_size = len;
    }
    
    // Get RX read pointer
    uint16_t ptr = readWord(block_reg, 0x0028);
    
    // Read data from RX buffer
    readReg(block_rx, ptr, data, recv_size);
    
    // Update RX read pointer
    ptr += recv_size;
    writeWord(block_reg, 0x0028, ptr);
    
    // Receive command
    writeByte(block_reg, 0x0001, W5500_CMD_RECV);
    
    return recv_size;
}

uint8_t W5500Driver::socketStatus(uint8_t socket) {
    if (!initialized_ || socket > 7) {
        return 0xFF;
    }
    
    uint8_t block = getSocketBlockBase(socket);
    return readByte(block, 0x0003);
}
#endif

#if POCKETOS_W5500_ENABLE_ERROR_HANDLING
bool W5500Driver::setMACAddress(const uint8_t* mac) {
    if (!initialized_) {
        return false;
    }
    
    return writeReg(W5500_BSB_COMMON_REG, 0x0009, mac, 6);
}

bool W5500Driver::setIPAddress(const uint8_t* ip) {
    if (!initialized_) {
        return false;
    }
    
    return writeReg(W5500_BSB_COMMON_REG, 0x000F, ip, 4);
}

bool W5500Driver::setSubnetMask(const uint8_t* subnet) {
    if (!initialized_) {
        return false;
    }
    
    return writeReg(W5500_BSB_COMMON_REG, 0x0005, subnet, 4);
}

bool W5500Driver::setGateway(const uint8_t* gateway) {
    if (!initialized_) {
        return false;
    }
    
    return writeReg(W5500_BSB_COMMON_REG, 0x0001, gateway, 4);
}

bool W5500Driver::socketSendTo(uint8_t socket, const uint8_t* data, uint16_t len, const uint8_t* ip, uint16_t port) {
    if (!initialized_ || socket > 7) {
        return false;
    }
    
    uint8_t block_reg = getSocketBlockBase(socket);
    uint8_t block_tx = W5500_BSB_S0_TX_BUF + (socket * 0x20);
    
    // Set destination IP and port
    writeReg(block_reg, 0x000C, ip, 4);
    writeWord(block_reg, 0x0010, port);
    
    // Get TX write pointer
    uint16_t ptr = readWord(block_reg, 0x0024);
    
    // Write data to TX buffer
    writeReg(block_tx, ptr, data, len);
    
    // Update TX write pointer
    ptr += len;
    writeWord(block_reg, 0x0024, ptr);
    
    // Send command
    writeByte(block_reg, 0x0001, W5500_CMD_SEND);
    
    return true;
}

int16_t W5500Driver::socketRecvFrom(uint8_t socket, uint8_t* data, uint16_t len, uint8_t* ip, uint16_t& port) {
    if (!initialized_ || socket > 7) {
        return -1;
    }
    
    uint8_t block_reg = getSocketBlockBase(socket);
    uint8_t block_rx = W5500_BSB_S0_RX_BUF + (socket * 0x20);
    
    // Get RX received size
    uint16_t recv_size = readWord(block_reg, 0x0026);
    if (recv_size < 8) {
        return 0;  // Need at least 8 bytes for UDP header
    }
    
    // Get RX read pointer
    uint16_t ptr = readWord(block_reg, 0x0028);
    
    // Read UDP header (IP:4 + port:2 + size:2)
    uint8_t header[8];
    readReg(block_rx, ptr, header, 8);
    ptr += 8;
    
    // Extract IP and port
    if (ip) {
        for (int i = 0; i < 4; i++) {
            ip[i] = header[i];
        }
    }
    port = ((uint16_t)header[4] << 8) | header[5];
    
    // Get data size
    uint16_t data_size = ((uint16_t)header[6] << 8) | header[7];
    if (data_size > len) {
        data_size = len;
    }
    
    // Read data
    readReg(block_rx, ptr, data, data_size);
    ptr += data_size;
    
    // Update RX read pointer
    writeWord(block_reg, 0x0028, ptr);
    
    // Receive command
    writeByte(block_reg, 0x0001, W5500_CMD_RECV);
    
    return data_size;
}

uint16_t W5500Driver::getTxFreeSize(uint8_t socket) {
    if (!initialized_ || socket > 7) {
        return 0;
    }
    
    uint8_t block = getSocketBlockBase(socket);
    return readWord(block, 0x0020);
}

uint16_t W5500Driver::getRxRecvSize(uint8_t socket) {
    if (!initialized_ || socket > 7) {
        return 0;
    }
    
    uint8_t block = getSocketBlockBase(socket);
    return readWord(block, 0x0026);
}
#endif

#if POCKETOS_W5500_ENABLE_REGISTER_ACCESS
const RegisterDesc* W5500Driver::registers(size_t& count) const {
    count = W5500_REGISTER_COUNT;
    return W5500_REGISTERS;
}

bool W5500Driver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    // Decode register address to block and offset
    uint8_t block;
    uint16_t addr;
    
    if (reg < 0x1000) {
        // Common registers
        block = W5500_BSB_COMMON_REG;
        addr = reg;
    } else {
        // Socket registers (simplified - socket 0 only)
        block = W5500_BSB_S0_REG;
        addr = reg & 0x0FFF;
    }
    
    return readReg(block, addr, buf, len);
}

bool W5500Driver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    // Decode register address to block and offset
    uint8_t block;
    uint16_t addr;
    
    if (reg < 0x1000) {
        // Common registers
        block = W5500_BSB_COMMON_REG;
        addr = reg;
    } else {
        // Socket registers (simplified - socket 0 only)
        block = W5500_BSB_S0_REG;
        addr = reg & 0x0FFF;
    }
    
    return writeReg(block, addr, buf, len);
}

const RegisterDesc* W5500Driver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(W5500_REGISTERS, W5500_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
