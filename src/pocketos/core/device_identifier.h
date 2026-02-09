#ifndef POCKETOS_DEVICE_IDENTIFIER_H
#define POCKETOS_DEVICE_IDENTIFIER_H

#include <Arduino.h>

namespace PocketOS {

// Device identification result
struct DeviceIdentification {
    String deviceClass;      // e.g., "bme280", "bme680", "sht31"
    String confidence;       // "high", "medium", "low", "unknown"
    String details;          // Additional info
    bool identified;
    
    DeviceIdentification() : deviceClass("unknown"), confidence("unknown"), details(""), identified(false) {}
};

// Device Identifier - Extensible identification engine
class DeviceIdentifier {
public:
    static void init();
    
    // Identify device at I2C address
    static DeviceIdentification identifyI2C(uint8_t address);
    
    // Identify device at endpoint
    static DeviceIdentification identifyEndpoint(const String& endpoint);
    
private:
    // Specific device identification functions
    static DeviceIdentification identifyBME280(uint8_t address);
    
    // Helper: Read I2C register
    static bool readI2CRegister(uint8_t address, uint8_t reg, uint8_t* value);
    static bool readI2CRegisters(uint8_t address, uint8_t reg, uint8_t* buffer, size_t len);
};

} // namespace PocketOS

#endif // POCKETOS_DEVICE_IDENTIFIER_H
