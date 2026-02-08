#ifdef ARDUINO_ARCH_RP2040

#include "platform_pack.h"
#include <LittleFS.h>
#include <hardware/watchdog.h>
#include <hardware/resets.h>
#include <pico/stdlib.h>

namespace PocketOS {

// RP2040 safe pins (Raspberry Pi Pico)
// Most GPIO pins are safe on RP2040, avoiding only special function pins
static const int RP2040_SAFE_PINS[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 26, 27, 28  // 26-28 are ADC-capable
};
static const int RP2040_SAFE_PIN_COUNT = sizeof(RP2040_SAFE_PINS) / sizeof(RP2040_SAFE_PINS[0]);

class RP2040PlatformPack : public PlatformPack {
private:
    bool storageInitialized;
    uint32_t bootTime;
    uint32_t minFreeHeap;
    bool isRunningOnPicoW;
    
public:
    RP2040PlatformPack() : storageInitialized(false), bootTime(millis()), minFreeHeap(264000) {
        // Detect if running on Pico W (has WiFi)
        #ifdef PICO_DEFAULT_LED_PIN
        isRunningOnPicoW = false;  // Standard Pico
        #else
        isRunningOnPicoW = false;  // Assume standard Pico
        #endif
    }
    
    // Platform identification
    PlatformType getType() const override { return PlatformType::RP2040; }
    const char* getName() const override { return "RP2040"; }
    const char* getVersion() const override { return "1.0.0"; }
    
    const char* getChipModel() const override {
        return isRunningOnPicoW ? "RP2040 (Pico W)" : "RP2040 (Pico)";
    }
    
    // Hardware capabilities - detailed
    bool supportsWiFi() const override { return isRunningOnPicoW; }  // Only Pico W
    bool supportsBluetooth() const override { return false; }  // No Bluetooth on any Pico
    bool supportsI2C() const override { return true; }
    bool supportsI2CSlave() const override { return true; }  // RP2040 supports I2C slave
    bool supportsSPI() const override { return true; }
    bool supportsADC() const override { return true; }
    bool supportsPWM() const override { return true; }
    bool supportsUART() const override { return true; }
    bool supportsOneWire() const override { return true; }
    
    // Capability counts
    int getI2CCount() const override { return 2; }  // I2C0, I2C1
    int getSPICount() const override { return 2; }  // SPI0, SPI1
    int getUARTCount() const override { return 2; }  // UART0, UART1
    int getADCChannelCount() const override { return 4; }  // 4 ADC channels (GPIO 26-29, though 29 is special)
    int getPWMChannelCount() const override { return 16; }  // 8 PWM slices × 2 channels
    
    // Memory management
    uint32_t getFreeHeap() const override {
        // RP2040 doesn't have a built-in function for this
        // Approximate by tracking allocations
        extern char __StackLimit, __bss_end__;
        uint32_t free = &__StackLimit - &__bss_end__;
        if (free < minFreeHeap) {
            minFreeHeap = free;
        }
        return free;
    }
    
    uint32_t getTotalHeap() const override { return 264000; }  // 264KB SRAM
    uint32_t getFlashSize() const override { return 2097152; }  // 2MB flash (standard)
    uint32_t getMinFreeHeap() const override { return minFreeHeap; }
    
    // GPIO
    int getGPIOCount() const override { return 30; }  // GPIO 0-29
    
    bool isValidPin(int pin) const override {
        return pin >= 0 && pin < 30;
    }
    
    bool isSafePin(int pin) const override {
        // GPIO 23-25 are typically used for special functions (SMPS, LED, etc.)
        if (pin >= 23 && pin <= 25) return false;
        
        // GPIO 29 is ADC3 but also VSYS/3 input - avoid for general use
        if (pin == 29) return false;
        
        return isValidPin(pin);
    }
    
    bool isInputOnlyPin(int pin) const override {
        // RP2040 has no input-only pins (all GPIO are bidirectional)
        return false;
    }
    
    const int* getSafePins(int& count) const override {
        count = RP2040_SAFE_PIN_COUNT;
        return RP2040_SAFE_PINS;
    }
    
    // Persistence - LittleFS
    bool hasNVS() const override { return false; }  // No NVS on RP2040
    bool hasEEPROM() const override { return false; }  // No EEPROM emulation by default
    bool hasFilesystem() const override { return true; }  // LittleFS in flash
    
    bool initStorage() override {
        if (!storageInitialized) {
            LittleFSConfig cfg;
            cfg.setAutoFormat(true);  // Format if needed
            LittleFS.setConfig(cfg);
            storageInitialized = LittleFS.begin();
        }
        return storageInitialized;
    }
    
    bool storageReady() const override {
        return storageInitialized;
    }
    
    // Power management
    bool supportsSleep() const override { return true; }
    bool supportsDeepSleep() const override { return true; }
    
    void enterLightSleep(uint32_t ms) override {
        // RP2040 light sleep using dormant mode or sleep
        // For now, implement as delay (proper sleep needs more setup)
        sleep_ms(ms);
    }
    
    void enterDeepSleep(uint32_t ms) override {
        // RP2040 deep sleep using dormant mode
        // Note: This requires external wake source
        // For now, use watchdog-based reset after delay
        if (ms > 0 && ms < 8388) {  // Watchdog max ~8.3 seconds
            watchdog_enable(ms, 1);
            while(1) {
                __wfi();  // Wait for interrupt
            }
        } else {
            // For longer delays, just sleep
            sleep_ms(ms);
        }
    }
    
    // Reset and diagnostics
    void softReset() override {
        watchdog_enable(1, 1);  // Enable watchdog with 1ms timeout
        while(1);  // Wait for watchdog reset
    }
    
    String getResetReason() const override {
        // RP2040 doesn't have a reset reason register like ESP32
        // Check watchdog caused reset
        if (watchdog_caused_reboot()) {
            return "Watchdog reset";
        }
        return "Power-on or external reset";
    }
    
    uint32_t getCycleCount() const override {
        // RP2040 cycle counter via time_us_64
        return time_us_64();
    }
    
    uint32_t getUptime() const override {
        return millis() - bootTime;
    }
};

// Factory function for RP2040
PlatformPack* createRP2040PlatformPack() {
    return new RP2040PlatformPack();
}

} // namespace PocketOS

#endif // ARDUINO_ARCH_RP2040
