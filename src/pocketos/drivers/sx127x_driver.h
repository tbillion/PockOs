#ifndef POCKETOS_SX127X_DRIVER_H
#define POCKETOS_SX127X_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "spi_driver_base.h"
#include "register_types.h"

namespace PocketOS {

// SX127x LoRa Transceiver Driver (SX1276/1277/1278/1279)
// Endpoint format: spi0:cs=5,rst=17,dio0=4 (rst and dio0 optional)

class SX127xDriver : public SPIDriverBase {
public:
    SX127xDriver();
    ~SX127xDriver();
    
    // Initialize from endpoint descriptor
    bool init(const String& endpoint);
    
    // Valid endpoint configurations
    static bool validEndpoints(const String& endpoint);
    
    // Identification probe - reads VERSION register (0x42)
    static bool identifyProbe(const String& endpoint);
    
#if POCKETOS_SX127X_ENABLE_BASIC_READ
    // Tier 0: Basic LoRa TX/RX
    bool transmit(const uint8_t* data, uint8_t len);
    bool receive(uint8_t* data, uint8_t& len, int16_t& rssi, int8_t& snr);
    bool available();
    bool setFrequency(uint32_t freq_hz);
    bool setTxPower(int8_t power);
#endif

#if POCKETOS_SX127X_ENABLE_ERROR_HANDLING
    // Tier 1: Spreading factor, bandwidth, coding rate
    bool setSpreadingFactor(uint8_t sf);  // 6-12
    bool setBandwidth(uint32_t bw_hz);    // 7800, 10400, 15600, 20800, 31250, 41700, 62500, 125000, 250000, 500000
    bool setCodingRate(uint8_t cr);       // 5-8 (4/5 to 4/8)
    bool setPreambleLength(uint16_t length);
    bool setSyncWord(uint8_t sw);
    bool setLowDataRateOptimize(bool enable);
    bool setCRC(bool enable);
    int16_t getRSSI();
    float getSNR();
#endif

#if POCKETOS_SX127X_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register access
    const RegisterDesc* registers(size_t& count) const override;
    bool regRead(uint16_t reg, uint8_t* buf, size_t len) override;
    bool regWrite(uint16_t reg, const uint8_t* buf, size_t len) override;
    const RegisterDesc* findRegisterByName(const String& name) const override;
#endif

private:
    bool initialized_;
    uint32_t frequency_;
    
    // Helper methods
    bool setMode(uint8_t mode);
    uint8_t readRegister(uint8_t reg);
    void writeRegister(uint8_t reg, uint8_t value);
    void setLoRaMode();
};

} // namespace PocketOS

#endif // POCKETOS_SX127X_DRIVER_H
