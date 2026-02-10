#ifndef POCKETOS_NRF24L01_DRIVER_H
#define POCKETOS_NRF24L01_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "spi_driver_base.h"
#include "register_types.h"

namespace PocketOS {

// nRF24L01+ 2.4GHz Transceiver Driver
// Endpoint format: spi0:cs=5,ce=16,irq=4 (ce required, irq optional)

class NRF24L01Driver : public SPIDriverBase {
public:
    NRF24L01Driver();
    ~NRF24L01Driver();
    
    // Initialize from endpoint descriptor
    bool init(const String& endpoint);
    
    // Valid endpoint configurations
    static bool validEndpoints(const String& endpoint);
    
    // Identification probe - reads CONFIG register
    static bool identifyProbe(const String& endpoint);
    
#if POCKETOS_NRF24L01_ENABLE_BASIC_READ
    // Tier 0: Basic TX/RX
    bool transmit(const uint8_t* data, uint8_t len);
    bool receive(uint8_t* data, uint8_t& len);
    bool available();
    bool setPowerUp(bool powerUp);
    bool setChannel(uint8_t channel);
    bool setDataRate(uint8_t rate);  // 0=1Mbps, 1=2Mbps, 2=250kbps
#endif

#if POCKETOS_NRF24L01_ENABLE_ERROR_HANDLING
    // Tier 1: Auto-ack, pipes, power modes
    bool setAutoAck(bool enable);
    bool setPayloadSize(uint8_t pipe, uint8_t size);
    bool openReadingPipe(uint8_t pipe, uint64_t address);
    bool openWritingPipe(uint64_t address);
    bool setPowerMode(uint8_t mode);  // 0=power down, 1=standby, 2=RX, 3=TX
    uint8_t getStatus();
    void flushTx();
    void flushRx();
#endif

#if POCKETOS_NRF24L01_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const override;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len) override;
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len) override;
    const RegisterDesc* findRegisterByName(const String& name) const override;
#endif

private:
    bool initialized_;
    int8_t ce_pin_;
    
    // Helper methods
    void setCE(bool active);
    bool writeCommand(uint8_t cmd);
    bool readRegister(uint8_t reg, uint8_t* data, uint8_t len);
    bool writeRegister(uint8_t reg, const uint8_t* data, uint8_t len);
};

} // namespace PocketOS

#endif // POCKETOS_NRF24L01_DRIVER_H
