#ifdef ESP8266

#include "platform_pack.h"
#include <Esp.h>
#include <LittleFS.h>

namespace PocketOS {

// ESP8266 safe pins (D1 Mini mapping)
// D0=GPIO16, D1=GPIO5, D2=GPIO4, D3=GPIO0, D4=GPIO2, D5=GPIO14, D6=GPIO12, D7=GPIO13, D8=GPIO15
static const int ESP8266_SAFE_PINS[] = {
    4, 5, 12, 13, 14  // GPIO 4, 5, 12, 13, 14 are generally safe
};
static const int ESP8266_SAFE_PIN_COUNT = sizeof(ESP8266_SAFE_PINS) / sizeof(ESP8266_SAFE_PINS[0]);

class ESP8266PlatformPack : public PlatformPack {
private:
    bool storageInitialized;
    uint32_t bootTime;
    uint32_t minFreeHeap;
    
public:
    ESP8266PlatformPack() : storageInitialized(false), bootTime(millis()), minFreeHeap(ESP.getFreeHeap()) {}
    
    // Platform identification
    PlatformType getType() const override { return PlatformType::ESP8266; }
    const char* getName() const override { return "ESP8266"; }
    const char* getVersion() const override { return "1.0.0"; }
    
    const char* getChipModel() const override {
        uint32_t chip_id = ESP.getChipId();
        return "ESP8266EX";
    }
    
    // Hardware capabilities - detailed
    bool supportsWiFi() const override { return true; }
    bool supportsBluetooth() const override { return false; }  // ESP8266 has no Bluetooth
    bool supportsI2C() const override { return true; }
    bool supportsI2CSlave() const override { return false; }  // ESP8266 I2C master only
    bool supportsSPI() const override { return true; }
    bool supportsADC() const override { return true; }
    bool supportsPWM() const override { return true; }
    bool supportsUART() const override { return true; }
    bool supportsOneWire() const override { return true; }
    
    // Capability counts
    int getI2CCount() const override { return 1; }  // One I2C bus (software implementation)
    int getSPICount() const override { return 1; }  // HSPI
    int getUARTCount() const override { return 2; }  // UART0, UART1 (TX only)
    int getADCChannelCount() const override { return 1; }  // Single ADC channel
    int getPWMChannelCount() const override { return 8; }  // Software PWM on most pins
    
    // Memory management
    uint32_t getFreeHeap() const override {
        uint32_t free = ESP.getFreeHeap();
        if (free < minFreeHeap) {
            minFreeHeap = free;
        }
        return free;
    }
    
    uint32_t getTotalHeap() const override { return 80000; }  // Approximately 80KB available
    uint32_t getFlashSize() const override { return ESP.getFlashChipRealSize(); }
    uint32_t getMinFreeHeap() const override { return minFreeHeap; }
    
    // GPIO
    int getGPIOCount() const override { return 17; }  // GPIO 0-16
    
    bool isValidPin(int pin) const override {
        return (pin >= 0 && pin <= 16);
    }
    
    bool isSafePin(int pin) const override {
        // Flash pins (6-11) - NEVER use
        if (pin >= 6 && pin <= 11) return false;
        
        // Strapping pins
        if (pin == 0 || pin == 2 || pin == 15) return false;
        
        // GPIO 16 is special (no interrupts, can't use for I2C/SPI)
        if (pin == 16) return false;
        
        // GPIO 1, 3 are UART TX/RX (usually avoid)
        if (pin == 1 || pin == 3) return false;
        
        return isValidPin(pin);
    }
    
    bool isInputOnlyPin(int pin) const override {
        // ESP8266 has no input-only pins
        return false;
    }
    
    const int* getSafePins(int& count) const override {
        count = ESP8266_SAFE_PIN_COUNT;
        return ESP8266_SAFE_PINS;
    }
    
    // Persistence - LittleFS
    bool hasNVS() const override { return false; }  // No NVS on ESP8266
    bool hasEEPROM() const override { return true; }  // Emulated EEPROM available
    bool hasFilesystem() const override { return true; }  // LittleFS
    
    bool initStorage() override {
        if (!storageInitialized) {
            storageInitialized = LittleFS.begin();
            if (!storageInitialized) {
                // Try formatting if mount fails
                LittleFS.format();
                storageInitialized = LittleFS.begin();
            }
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
        // ESP8266 light sleep requires specific setup
        // For simplicity, use delay (light sleep implementation is complex)
        wifi_set_sleep_type(LIGHT_SLEEP_T);
        delay(ms);
        wifi_set_sleep_type(NONE_SLEEP_T);
    }
    
    void enterDeepSleep(uint32_t ms) override {
        ESP.deepSleep(ms * 1000ULL);
    }
    
    // Reset and diagnostics
    void softReset() override {
        ESP.restart();
    }
    
    String getResetReason() const override {
        rst_info *resetInfo = ESP.getResetInfoPtr();
        switch (resetInfo->reason) {
            case REASON_DEFAULT_RST: return "Power-on";
            case REASON_WDT_RST: return "Hardware watchdog";
            case REASON_EXCEPTION_RST: return "Exception";
            case REASON_SOFT_WDT_RST: return "Software watchdog";
            case REASON_SOFT_RESTART: return "Software restart";
            case REASON_DEEP_SLEEP_AWAKE: return "Deep sleep wake";
            case REASON_EXT_SYS_RST: return "External reset";
            default: return "Unknown";
        }
    }
    
    uint32_t getCycleCount() const override {
        return ESP.getCycleCount();
    }
    
    uint32_t getUptime() const override {
        return millis() - bootTime;
    }
};

// Factory function for ESP8266
PlatformPack* createESP8266PlatformPack() {
    return new ESP8266PlatformPack();
}

} // namespace PocketOS

#endif // ESP8266
