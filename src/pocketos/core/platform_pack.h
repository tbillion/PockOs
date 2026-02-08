#ifndef POCKETOS_PLATFORM_PACK_H
#define POCKETOS_PLATFORM_PACK_H

#include <Arduino.h>

namespace PocketOS {

/**
 * Platform Pack Contract
 * 
 * Defines the interface that each platform must implement.
 * Provides platform-specific functionality abstraction.
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
    
    // Hardware capabilities
    virtual bool supportsWiFi() const = 0;
    virtual bool supportsBluetooth() const = 0;
    virtual bool supportsI2C() const = 0;
    virtual bool supportsSPI() const = 0;
    virtual bool supportsADC() const = 0;
    virtual bool supportsPWM() const = 0;
    
    // Memory management
    virtual uint32_t getFreeHeap() const = 0;
    virtual uint32_t getTotalHeap() const = 0;
    virtual uint32_t getFlashSize() const = 0;
    
    // GPIO
    virtual int getGPIOCount() const = 0;
    virtual bool isValidPin(int pin) const = 0;
    virtual bool isSafePin(int pin) const = 0;  // Safe for general use
    
    // Persistence
    virtual bool hasNVS() const = 0;
    virtual bool hasEEPROM() const = 0;
    virtual bool hasFilesystem() const = 0;
    
    // Power management
    virtual bool supportsSleep() const = 0;
    virtual void enterLightSleep(uint32_t ms) = 0;
    virtual void enterDeepSleep(uint32_t ms) = 0;
    
    // Reset
    virtual void softReset() = 0;
    virtual String getResetReason() const = 0;
};

// Global platform pack instance
extern PlatformPack* g_platformPack;

// Factory function to create appropriate platform pack
PlatformPack* createPlatformPack();

} // namespace PocketOS

#endif // POCKETOS_PLATFORM_PACK_H
