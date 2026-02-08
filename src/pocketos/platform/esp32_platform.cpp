#ifdef ESP32

#include "platform_pack.h"
#include <esp_system.h>
#include <esp_sleep.h>
#include <Preferences.h>
#include <esp_chip_info.h>

namespace PocketOS {

// ESP32 safe pins (conservative list)
static const int ESP32_SAFE_PINS[] = {
    4, 5, 13, 14, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33
};
static const int ESP32_SAFE_PIN_COUNT = sizeof(ESP32_SAFE_PINS) / sizeof(ESP32_SAFE_PINS[0]);

class ESP32PlatformPack : public PlatformPack {
private:
    Preferences prefs;
    bool storageInitialized;
    uint32_t bootTime;
    
public:
    ESP32PlatformPack() : storageInitialized(false), bootTime(millis()) {}
    
    // Platform identification
    PlatformType getType() const override { return PlatformType::ESP32; }
    const char* getName() const override { return "ESP32"; }
    const char* getVersion() const override { return "1.0.0"; }
    
    const char* getChipModel() const override {
        esp_chip_info_t chip_info;
        esp_chip_info(&chip_info);
        return (chip_info.model == CHIP_ESP32) ? "ESP32" : "ESP32-variant";
    }
    
    // Hardware capabilities - detailed
    bool supportsWiFi() const override { return true; }
    bool supportsBluetooth() const override { return true; }
    bool supportsI2C() const override { return true; }
    bool supportsI2CSlave() const override { return true; }  // ESP32 supports I2C slave
    bool supportsSPI() const override { return true; }
    bool supportsADC() const override { return true; }
    bool supportsPWM() const override { return true; }
    bool supportsUART() const override { return true; }
    bool supportsOneWire() const override { return true; }
    
    // Capability counts
    int getI2CCount() const override { return 2; }  // I2C0, I2C1
    int getSPICount() const override { return 3; }  // VSPI, HSPI, SPI
    int getUARTCount() const override { return 3; }  // UART0, UART1, UART2
    int getADCChannelCount() const override { return 18; }  // ADC1: 8 channels, ADC2: 10 channels
    int getPWMChannelCount() const override { return 16; }  // LEDC has 16 channels
    
    // Memory management
    uint32_t getFreeHeap() const override { return ESP.getFreeHeap(); }
    uint32_t getTotalHeap() const override { return ESP.getHeapSize(); }
    uint32_t getFlashSize() const override { return ESP.getFlashChipSize(); }
    uint32_t getMinFreeHeap() const override { return ESP.getMinFreeHeap(); }
    
    // GPIO
    int getGPIOCount() const override { return 40; }
    
    bool isValidPin(int pin) const override {
        return pin >= 0 && pin < 40;
    }
    
    bool isSafePin(int pin) const override {
        // Flash pins (6-11) - NEVER use
        if (pin >= 6 && pin <= 11) return false;
        
        // Strapping pins that can cause boot issues
        if (pin == 0 || pin == 2 || pin == 12 || pin == 15) return false;
        
        // Input-only pins
        if (pin >= 34 && pin <= 39) return false;
        
        return isValidPin(pin);
    }
    
    bool isInputOnlyPin(int pin) const override {
        // GPIOs 34-39 are input-only
        return pin >= 34 && pin <= 39;
    }
    
    const int* getSafePins(int& count) const override {
        count = ESP32_SAFE_PIN_COUNT;
        return ESP32_SAFE_PINS;
    }
    
    // Persistence - NVS
    bool hasNVS() const override { return true; }
    bool hasEEPROM() const override { return false; }
    bool hasFilesystem() const override { return true; }  // SPIFFS/LittleFS available
    
    bool initStorage() override {
        if (!storageInitialized) {
            storageInitialized = prefs.begin("pocketos", false);
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
        esp_sleep_enable_timer_wakeup(ms * 1000ULL);
        esp_light_sleep_start();
    }
    
    void enterDeepSleep(uint32_t ms) override {
        esp_sleep_enable_timer_wakeup(ms * 1000ULL);
        esp_deep_sleep_start();
    }
    
    // Reset and diagnostics
    void softReset() override {
        ESP.restart();
    }
    
    String getResetReason() const override {
        esp_reset_reason_t reason = esp_reset_reason();
        switch (reason) {
            case ESP_RST_POWERON: return "Power-on";
            case ESP_RST_SW: return "Software reset";
            case ESP_RST_PANIC: return "Panic/exception";
            case ESP_RST_INT_WDT: return "Interrupt watchdog";
            case ESP_RST_TASK_WDT: return "Task watchdog";
            case ESP_RST_WDT: return "Other watchdog";
            case ESP_RST_DEEPSLEEP: return "Deep sleep wake";
            case ESP_RST_BROWNOUT: return "Brownout";
            case ESP_RST_SDIO: return "SDIO reset";
            default: return "Unknown";
        }
    }
    
    uint32_t getCycleCount() const override {
        return esp_cpu_get_cycle_count();
    }
    
    uint32_t getUptime() const override {
        return millis() - bootTime;
    }
};

// Factory function for ESP32
PlatformPack* createESP32PlatformPack() {
    return new ESP32PlatformPack();
}

} // namespace PocketOS

#endif // ESP32
