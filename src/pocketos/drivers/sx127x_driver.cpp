#include "sx127x_driver.h"
#include "../core/logger.h"
#include <SPI.h>

namespace PocketOS {

// SX127x Modes
#define SX127X_MODE_SLEEP       0x00
#define SX127X_MODE_STDBY       0x01
#define SX127X_MODE_TX          0x03
#define SX127X_MODE_RXCONT      0x05
#define SX127X_MODE_RXSINGLE    0x06

#if POCKETOS_SX127X_ENABLE_REGISTER_ACCESS
// Complete SX127x Register Map (0x00-0x70, common and LoRa mode)
static const RegisterDesc SX127X_REGISTERS[] = {
    // Common registers (0x00-0x0F)
    RegisterDesc(0x00, "FIFO", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x01, "OP_MODE", 1, RegisterAccess::RW, 0x01),
    RegisterDesc(0x06, "FRF_MSB", 1, RegisterAccess::RW, 0xE4),
    RegisterDesc(0x07, "FRF_MID", 1, RegisterAccess::RW, 0xC0),
    RegisterDesc(0x08, "FRF_LSB", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x09, "PA_CONFIG", 1, RegisterAccess::RW, 0x4F),
    RegisterDesc(0x0A, "PA_RAMP", 1, RegisterAccess::RW, 0x09),
    RegisterDesc(0x0B, "OCP", 1, RegisterAccess::RW, 0x2B),
    RegisterDesc(0x0C, "LNA", 1, RegisterAccess::RW, 0x20),
    
    // LoRa Mode registers (0x0D-0x3F)
    RegisterDesc(0x0D, "FIFO_ADDR_PTR", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0E, "FIFO_TX_BASE", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x0F, "FIFO_RX_BASE", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x10, "FIFO_RX_CURRENT", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x11, "IRQ_FLAGS_MASK", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x12, "IRQ_FLAGS", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x13, "RX_NB_BYTES", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x14, "RX_HEADER_CNT_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x15, "RX_HEADER_CNT_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x16, "RX_PACKET_CNT_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x17, "RX_PACKET_CNT_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x18, "MODEM_STAT", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x19, "PKT_SNR_VALUE", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1A, "PKT_RSSI_VALUE", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1B, "RSSI_VALUE", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1C, "HOP_CHANNEL", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x1D, "MODEM_CONFIG1", 1, RegisterAccess::RW, 0x72),
    RegisterDesc(0x1E, "MODEM_CONFIG2", 1, RegisterAccess::RW, 0x70),
    RegisterDesc(0x1F, "SYMB_TIMEOUT_LSB", 1, RegisterAccess::RW, 0x64),
    RegisterDesc(0x20, "PREAMBLE_MSB", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x21, "PREAMBLE_LSB", 1, RegisterAccess::RW, 0x08),
    RegisterDesc(0x22, "PAYLOAD_LENGTH", 1, RegisterAccess::RW, 0x01),
    RegisterDesc(0x23, "MAX_PAYLOAD_LENGTH", 1, RegisterAccess::RW, 0xFF),
    RegisterDesc(0x24, "HOP_PERIOD", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x25, "FIFO_RX_BYTE_ADDR", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x26, "MODEM_CONFIG3", 1, RegisterAccess::RW, 0x04),
    RegisterDesc(0x27, "PPM_CORRECTION", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x28, "FEI_MSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x29, "FEI_MID", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2A, "FEI_LSB", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x2C, "RSSI_WIDEBAND", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x31, "DETECT_OPTIMIZE", 1, RegisterAccess::RW, 0x03),
    RegisterDesc(0x33, "INVERT_IQ", 1, RegisterAccess::RW, 0x27),
    RegisterDesc(0x37, "DETECTION_THRESHOLD", 1, RegisterAccess::RW, 0x0A),
    RegisterDesc(0x39, "SYNC_WORD", 1, RegisterAccess::RW, 0x34),
    
    // Common registers (0x40-0x70)
    RegisterDesc(0x40, "DIO_MAPPING1", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x41, "DIO_MAPPING2", 1, RegisterAccess::RW, 0x00),
    RegisterDesc(0x42, "VERSION", 1, RegisterAccess::RO, 0x12),
    RegisterDesc(0x4B, "TCXO", 1, RegisterAccess::RW, 0x09),
    RegisterDesc(0x4D, "PA_DAC", 1, RegisterAccess::RW, 0x84),
    RegisterDesc(0x5B, "FORMER_TEMP", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x61, "AGC_REF", 1, RegisterAccess::RW, 0x19),
    RegisterDesc(0x62, "AGC_THRESH1", 1, RegisterAccess::RW, 0x0C),
    RegisterDesc(0x63, "AGC_THRESH2", 1, RegisterAccess::RW, 0x0B),
    RegisterDesc(0x64, "AGC_THRESH3", 1, RegisterAccess::RW, 0x0B),
    RegisterDesc(0x70, "PLL", 1, RegisterAccess::RW, 0xD0),
};

#define SX127X_REGISTER_COUNT (sizeof(SX127X_REGISTERS) / sizeof(RegisterDesc))
#endif

SX127xDriver::SX127xDriver() 
    : initialized_(false), frequency_(915000000) {
    setRegisterConvention(SPIRegisterConvention::GENERIC);
}

SX127xDriver::~SX127xDriver() {
    deinit();
}

bool SX127xDriver::init(const String& endpoint) {
    if (!initFromEndpoint(endpoint)) {
        return false;
    }
    
    // Perform hardware reset if RST pin available
    if (getPinConfig().rst >= 0) {
        setRST(true);
        delay(10);
        setRST(false);
        delay(10);
    }
    
    // Read version register
    uint8_t version = readRegister(0x42);
    if (version != 0x12) {
        Logger::error("SX127x: Invalid version: 0x" + String(version, HEX));
        deinit();
        return false;
    }
    
    // Set LoRa mode
    setLoRaMode();
    
    // Set to standby mode
    setMode(SX127X_MODE_STDBY);
    
    // Set default frequency (915 MHz)
    setFrequency(frequency_);
    
    initialized_ = true;
    Logger::info("SX127x: Initialized successfully (version 0x" + String(version, HEX) + ")");
    return true;
}

bool SX127xDriver::validEndpoints(const String& endpoint) {
    // Format: spi0:cs=5,rst=17,dio0=4
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

bool SX127xDriver::identifyProbe(const String& endpoint) {
    SX127xDriver driver;
    if (!driver.initFromEndpoint(endpoint)) {
        return false;
    }
    
    // Perform reset if available
    if (driver.getPinConfig().rst >= 0) {
        driver.setRST(true);
        delay(10);
        driver.setRST(false);
        delay(10);
    }
    
    // Read VERSION register
    uint8_t version = driver.readRegister(0x42);
    return version == 0x12;
}

uint8_t SX127xDriver::readRegister(uint8_t reg) {
    uint8_t value;
    regRead(reg, &value, 1);
    return value;
}

void SX127xDriver::writeRegister(uint8_t reg, uint8_t value) {
    regWrite(reg, &value, 1);
}

void SX127xDriver::setLoRaMode() {
    // Set LoRa mode in OP_MODE register
    uint8_t opMode = readRegister(0x01);
    opMode = (opMode & 0x7F) | 0x80;  // Set bit 7 for LoRa mode
    writeRegister(0x01, opMode);
    delay(10);
}

bool SX127xDriver::setMode(uint8_t mode) {
    uint8_t opMode = readRegister(0x01);
    opMode = (opMode & 0xF8) | (mode & 0x07);
    writeRegister(0x01, opMode);
    
    if (mode == SX127X_MODE_TX || mode == SX127X_MODE_RXCONT || mode == SX127X_MODE_RXSINGLE) {
        delay(1);
    }
    
    return true;
}

#if POCKETOS_SX127X_ENABLE_BASIC_READ
bool SX127xDriver::transmit(const uint8_t* data, uint8_t len) {
    if (!initialized_ || len > 255) {
        return false;
    }
    
    // Set to standby mode
    setMode(SX127X_MODE_STDBY);
    
    // Set FIFO address pointer to TX base
    writeRegister(0x0D, 0x00);
    writeRegister(0x0E, 0x00);
    
    // Write payload to FIFO
    for (uint8_t i = 0; i < len; i++) {
        writeRegister(0x00, data[i]);
    }
    
    // Set payload length
    writeRegister(0x22, len);
    
    // Start TX
    setMode(SX127X_MODE_TX);
    
    // Wait for TX done (simplified - check IRQ flags)
    while ((readRegister(0x12) & 0x08) == 0) {
        delay(1);
    }
    
    // Clear IRQ flags
    writeRegister(0x12, 0xFF);
    
    // Return to standby
    setMode(SX127X_MODE_STDBY);
    
    return true;
}

bool SX127xDriver::receive(uint8_t* data, uint8_t& len, int16_t& rssi, int8_t& snr) {
    if (!initialized_) {
        return false;
    }
    
    if (!available()) {
        return false;
    }
    
    // Get received packet length
    len = readRegister(0x13);
    
    // Get FIFO RX current address
    uint8_t fifoAddr = readRegister(0x10);
    writeRegister(0x0D, fifoAddr);
    
    // Read payload from FIFO
    for (uint8_t i = 0; i < len; i++) {
        data[i] = readRegister(0x00);
    }
    
    // Get RSSI and SNR
    rssi = -157 + readRegister(0x1A);
    snr = (int8_t)readRegister(0x19) / 4;
    
    // Clear IRQ flags
    writeRegister(0x12, 0xFF);
    
    return true;
}

bool SX127xDriver::available() {
    uint8_t irqFlags = readRegister(0x12);
    return (irqFlags & 0x40) != 0;  // RxDone flag
}

bool SX127xDriver::setFrequency(uint32_t freq_hz) {
    if (!initialized_) {
        return false;
    }
    
    frequency_ = freq_hz;
    
    // Calculate frequency register value
    // Frf = (Frf / 32MHz) * 2^19
    uint64_t frf = ((uint64_t)freq_hz << 19) / 32000000;
    
    writeRegister(0x06, (uint8_t)(frf >> 16));
    writeRegister(0x07, (uint8_t)(frf >> 8));
    writeRegister(0x08, (uint8_t)(frf));
    
    return true;
}

bool SX127xDriver::setTxPower(int8_t power) {
    if (!initialized_ || power < 2 || power > 20) {
        return false;
    }
    
    if (power > 17) {
        // Enable high power mode
        writeRegister(0x4D, 0x87);
        writeRegister(0x09, 0x80 | (power - 2));
    } else {
        writeRegister(0x4D, 0x84);
        writeRegister(0x09, 0x80 | (power - 2));
    }
    
    return true;
}
#endif

#if POCKETOS_SX127X_ENABLE_ERROR_HANDLING
bool SX127xDriver::setSpreadingFactor(uint8_t sf) {
    if (!initialized_ || sf < 6 || sf > 12) {
        return false;
    }
    
    uint8_t config2 = readRegister(0x1E);
    config2 = (config2 & 0x0F) | (sf << 4);
    writeRegister(0x1E, config2);
    
    // Special handling for SF6
    if (sf == 6) {
        writeRegister(0x31, 0xC5);
        writeRegister(0x37, 0x0C);
    } else {
        writeRegister(0x31, 0xC3);
        writeRegister(0x37, 0x0A);
    }
    
    return true;
}

bool SX127xDriver::setBandwidth(uint32_t bw_hz) {
    if (!initialized_) {
        return false;
    }
    
    uint8_t bw_code;
    if (bw_hz <= 7800) bw_code = 0;
    else if (bw_hz <= 10400) bw_code = 1;
    else if (bw_hz <= 15600) bw_code = 2;
    else if (bw_hz <= 20800) bw_code = 3;
    else if (bw_hz <= 31250) bw_code = 4;
    else if (bw_hz <= 41700) bw_code = 5;
    else if (bw_hz <= 62500) bw_code = 6;
    else if (bw_hz <= 125000) bw_code = 7;
    else if (bw_hz <= 250000) bw_code = 8;
    else bw_code = 9;  // 500kHz
    
    uint8_t config1 = readRegister(0x1D);
    config1 = (config1 & 0x0F) | (bw_code << 4);
    writeRegister(0x1D, config1);
    
    return true;
}

bool SX127xDriver::setCodingRate(uint8_t cr) {
    if (!initialized_ || cr < 5 || cr > 8) {
        return false;
    }
    
    uint8_t cr_code = cr - 4;
    uint8_t config1 = readRegister(0x1D);
    config1 = (config1 & 0xF1) | (cr_code << 1);
    writeRegister(0x1D, config1);
    
    return true;
}

bool SX127xDriver::setPreambleLength(uint16_t length) {
    if (!initialized_) {
        return false;
    }
    
    writeRegister(0x20, (uint8_t)(length >> 8));
    writeRegister(0x21, (uint8_t)(length));
    
    return true;
}

bool SX127xDriver::setSyncWord(uint8_t sw) {
    if (!initialized_) {
        return false;
    }
    
    writeRegister(0x39, sw);
    return true;
}

bool SX127xDriver::setLowDataRateOptimize(bool enable) {
    if (!initialized_) {
        return false;
    }
    
    uint8_t config3 = readRegister(0x26);
    if (enable) {
        config3 |= 0x08;
    } else {
        config3 &= ~0x08;
    }
    writeRegister(0x26, config3);
    
    return true;
}

bool SX127xDriver::setCRC(bool enable) {
    if (!initialized_) {
        return false;
    }
    
    uint8_t config2 = readRegister(0x1E);
    if (enable) {
        config2 |= 0x04;
    } else {
        config2 &= ~0x04;
    }
    writeRegister(0x1E, config2);
    
    return true;
}

int16_t SX127xDriver::getRSSI() {
    if (!initialized_) {
        return -999;
    }
    
    return -157 + readRegister(0x1B);
}

float SX127xDriver::getSNR() {
    if (!initialized_) {
        return -999.0;
    }
    
    return (int8_t)readRegister(0x19) / 4.0;
}
#endif

#if POCKETOS_SX127X_ENABLE_REGISTER_ACCESS
const RegisterDesc* SX127xDriver::registers(size_t& count) const {
    count = SX127X_REGISTER_COUNT;
    return SX127X_REGISTERS;
}

bool SX127xDriver::regRead(uint16_t reg, uint8_t* buf, size_t len) {
    if (reg > 0x70) {
        return false;
    }
    
    uint8_t cmd = reg & 0x7F;
    return spiWriteRead(&cmd, 1, buf, len);
}

bool SX127xDriver::regWrite(uint16_t reg, const uint8_t* buf, size_t len) {
    if (reg > 0x70) {
        return false;
    }
    
    beginTransaction();
    uint8_t cmd = 0x80 | (reg & 0x7F);
    SPI.transfer(cmd);
    for (size_t i = 0; i < len; i++) {
        SPI.transfer(buf[i]);
    }
    endTransaction();
    
    return true;
}

const RegisterDesc* SX127xDriver::findRegisterByName(const String& name) const {
    return RegisterUtils::findByName(SX127X_REGISTERS, SX127X_REGISTER_COUNT, name);
}
#endif

} // namespace PocketOS
