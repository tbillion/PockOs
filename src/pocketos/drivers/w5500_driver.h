#ifndef POCKETOS_W5500_DRIVER_H
#define POCKETOS_W5500_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "spi_driver_base.h"
#include "register_types.h"

namespace PocketOS {

// W5500 Ethernet Controller Driver
// Endpoint format: spi0:cs=5,rst=17,irq=4 (rst and irq optional)

class W5500Driver : public SPIDriverBase {
public:
    W5500Driver();
    ~W5500Driver();
    
    // Initialize from endpoint descriptor
    bool init(const String& endpoint);
    
    // Valid endpoint configurations
    static bool validEndpoints(const String& endpoint);
    
    // Identification probe - reads VERSIONR register
    static bool identifyProbe(const String& endpoint);
    
#if POCKETOS_W5500_ENABLE_BASIC_READ
    // Tier 0: Basic socket operations
    bool socketOpen(uint8_t socket, uint8_t protocol, uint16_t port);
    bool socketClose(uint8_t socket);
    bool socketConnect(uint8_t socket, const uint8_t* ip, uint16_t port);
    bool socketListen(uint8_t socket);
    int16_t socketSend(uint8_t socket, const uint8_t* data, uint16_t len);
    int16_t socketRecv(uint8_t socket, uint8_t* data, uint16_t len);
    uint8_t socketStatus(uint8_t socket);
#endif

#if POCKETOS_W5500_ENABLE_ERROR_HANDLING
    // Tier 1: Multiple sockets, UDP/TCP
    bool setMACAddress(const uint8_t* mac);
    bool setIPAddress(const uint8_t* ip);
    bool setSubnetMask(const uint8_t* subnet);
    bool setGateway(const uint8_t* gateway);
    bool socketSendTo(uint8_t socket, const uint8_t* data, uint16_t len, const uint8_t* ip, uint16_t port);
    int16_t socketRecvFrom(uint8_t socket, uint8_t* data, uint16_t len, uint8_t* ip, uint16_t& port);
    uint16_t getTxFreeSize(uint8_t socket);
    uint16_t getRxRecvSize(uint8_t socket);
#endif

#if POCKETOS_W5500_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const override;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len) override;
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len) override;
    const RegisterDesc* findRegisterByName(const String& name) const override;
#endif

private:
    bool initialized_;
    
    // Helper methods
    uint8_t getSocketBlockBase(uint8_t socket) const {
        return W5500_BSB_S0_REG + (socket * 0x20);
    }
    
    uint8_t getSocketTxBufBase(uint8_t socket) const {
        return W5500_BSB_S0_TX_BUF + (socket * 0x20);
    }
    
    uint8_t getSocketRxBufBase(uint8_t socket) const {
        return W5500_BSB_S0_RX_BUF + (socket * 0x20);
    }
    
    // W5500-specific register access (3-byte addressing: control, addr_hi, addr_lo)
    bool readReg(uint8_t block, uint16_t addr, uint8_t* data, uint16_t len);
    bool writeReg(uint8_t block, uint16_t addr, const uint8_t* data, uint16_t len);
    uint8_t readByte(uint8_t block, uint16_t addr);
    void writeByte(uint8_t block, uint16_t addr, uint8_t value);
    uint16_t readWord(uint8_t block, uint16_t addr);
    void writeWord(uint8_t block, uint16_t addr, uint16_t value);
};

} // namespace PocketOS

#endif // POCKETOS_W5500_DRIVER_H
