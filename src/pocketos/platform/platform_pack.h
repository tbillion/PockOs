#ifndef POCKETOS_PLATFORM_PACK_H
#define POCKETOS_PLATFORM_PACK_H

#include <Arduino.h>

namespace PocketOS {

/**
 * Platform Pack Contract
 * 
 * Defines the interface that each platform must implement.
 * Provides platform-specific functionality abstraction.
 * 
 * NO STUBS ALLOWED - All implementations must be complete and functional
 */

enum class PlatformType {
    ESP32,
    ESP8266,
    RP2040,
    UNKNOWN
};

class PlatformPack {
public:
    virtual ~PlatformPack() {}
    
    // Platform identification
    virtual PlatformType getType() const = 0;
    virtual const char* getName() const = 0;
    virtual const char* getVersion() const = 0;
    virtual const char* getChipModel() const = 0;
    
    // Hardware capabilities - detailed
    virtual bool supportsWiFi() const = 0;
    virtual bool supportsBluetooth() const = 0;
    virtual bool supportsI2C() const = 0;
    virtual bool supportsI2CSlave() const = 0;  // Separate check for slave mode
    virtual bool supportsSPI() const = 0;
    virtual bool supportsADC() const = 0;
    virtual bool supportsPWM() const = 0;
    virtual bool supportsUART() const = 0;
    virtual bool supportsOneWire() const = 0;
    
    // Capability counts
    virtual int getI2CCount() const = 0;
    virtual int getSPICount() const = 0;
    virtual int getUARTCount() const = 0;
    virtual int getADCChannelCount() const = 0;
    virtual int getPWMChannelCount() const = 0;
    
    // Memory management
    virtual uint32_t getFreeHeap() const = 0;
    virtual uint32_t getTotalHeap() const = 0;
    virtual uint32_t getFlashSize() const = 0;
    virtual uint32_t getMinFreeHeap() const = 0;  // Lowest free heap since boot
    
    // GPIO
    virtual int getGPIOCount() const = 0;
    virtual bool isValidPin(int pin) const = 0;
    virtual bool isSafePin(int pin) const = 0;  // Safe for general use
    virtual bool isInputOnlyPin(int pin) const = 0;  // Input-only pins
    virtual const int* getSafePins(int& count) const = 0;  // Get safe pin list
    
    // Persistence
    virtual bool hasNVS() const = 0;
    virtual bool hasEEPROM() const = 0;
    virtual bool hasFilesystem() const = 0;
    virtual bool initStorage() = 0;  // Initialize storage backend
    virtual bool storageReady() const = 0;  // Check if storage is initialized
    
    // Power management
    virtual bool supportsSleep() const = 0;
    virtual bool supportsDeepSleep() const = 0;
    virtual void enterLightSleep(uint32_t ms) = 0;
    virtual void enterDeepSleep(uint32_t ms) = 0;
    
    // Reset and diagnostics
    virtual void softReset() = 0;
    virtual String getResetReason() const = 0;
    virtual uint32_t getCycleCount() const = 0;  // CPU cycle counter if available
    virtual uint32_t getUptime() const = 0;  // Uptime in milliseconds
};

// Global platform pack instance
extern PlatformPack* g_platformPack;

// Factory function to create appropriate platform pack
PlatformPack* createPlatformPack();

// Initialize platform pack (call once at startup)
bool initializePlatformPack();

} // namespace PocketOS

#endif // POCKETOS_PLATFORM_PACK_H
