#include "platform_pack.h"

#ifdef ESP32
#include <esp_system.h>
#include <Preferences.h>
#endif

#ifdef ESP8266
#include <Esp.h>
#endif

namespace PocketOS {

// Global platform pack instance
PlatformPack* g_platformPack = nullptr;

// ESP32 Platform Pack
#ifdef ESP32
class ESP32PlatformPack : public PlatformPack {
public:
    PlatformType getType() const override { return PlatformType::ESP32; }
    const char* getName() const override { return "ESP32"; }
    const char* getVersion() const override { return "1.0.0"; }
    
    bool supportsWiFi() const override { return true; }
    bool supportsBluetooth() const override { return true; }
    bool supportsI2C() const override { return true; }
    bool supportsSPI() const override { return true; }
    bool supportsADC() const override { return true; }
    bool supportsPWM() const override { return true; }
    
    uint32_t getFreeHeap() const override { return ESP.getFreeHeap(); }
    uint32_t getTotalHeap() const override { return ESP.getHeapSize(); }
    uint32_t getFlashSize() const override { return ESP.getFlashChipSize(); }
    
    int getGPIOCount() const override { return 40; }
    
    bool isValidPin(int pin) const override {
        return pin >= 0 && pin < 40;
    }
    
    bool isSafePin(int pin) const override {
        // Avoid flash pins (6-11), and strapping pins that can cause issues
        if (pin >= 6 && pin <= 11) return false;  // Flash
        if (pin == 0 || pin == 2 || pin == 12 || pin == 15) return false;  // Strapping
        return isValidPin(pin);
    }
    
    bool hasNVS() const override { return true; }
    bool hasEEPROM() const override { return false; }
    bool hasFilesystem() const override { return true; }
    
    bool supportsSleep() const override { return true; }
    
    void enterLightSleep(uint32_t ms) override {
        #ifdef ESP32
        esp_sleep_enable_timer_wakeup(ms * 1000);
        esp_light_sleep_start();
        #endif
    }
    
    void enterDeepSleep(uint32_t ms) override {
        #ifdef ESP32
        esp_sleep_enable_timer_wakeup(ms * 1000);
        esp_deep_sleep_start();
        #endif
    }
    
    void softReset() override {
        ESP.restart();
    }
    
    String getResetReason() const override {
        #ifdef ESP32
        esp_reset_reason_t reason = esp_reset_reason();
        switch (reason) {
            case ESP_RST_POWERON: return "Power-on";
            case ESP_RST_SW: return "Software reset";
            case ESP_RST_PANIC: return "Panic/exception";
            case ESP_RST_INT_WDT: return "Watchdog timeout";
            case ESP_RST_BROWNOUT: return "Brownout";
            default: return "Unknown";
        }
        #else
        return "Unknown";
        #endif
    }
};
#endif

// ESP8266 Platform Pack (stub implementation)
#ifdef ESP8266
class ESP8266PlatformPack : public PlatformPack {
public:
    PlatformType getType() const override { return PlatformType::ESP8266; }
    const char* getName() const override { return "ESP8266"; }
    const char* getVersion() const override { return "1.0.0-stub"; }
    
    bool supportsWiFi() const override { return true; }
    bool supportsBluetooth() const override { return false; }
    bool supportsI2C() const override { return true; }
    bool supportsSPI() const override { return true; }
    bool supportsADC() const override { return true; }
    bool supportsPWM() const override { return true; }
    
    uint32_t getFreeHeap() const override { return ESP.getFreeHeap(); }
    uint32_t getTotalHeap() const override { return 80000; }  // Typical ESP8266
    uint32_t getFlashSize() const override { return ESP.getFlashChipSize(); }
    
    int getGPIOCount() const override { return 17; }
    
    bool isValidPin(int pin) const override {
        return pin >= 0 && pin <= 16;
    }
    
    bool isSafePin(int pin) const override {
        // Avoid GPIO 6-11 (flash), GPIO 0, 2, 15 (strapping)
        if (pin >= 6 && pin <= 11) return false;
        if (pin == 0 || pin == 2 || pin == 15) return false;
        return isValidPin(pin);
    }
    
    bool hasNVS() const override { return false; }
    bool hasEEPROM() const override { return true; }
    bool hasFilesystem() const override { return true; }
    
    bool supportsSleep() const override { return true; }
    
    void enterLightSleep(uint32_t ms) override {
        // ESP8266 light sleep (stub)
        delay(ms);
    }
    
    void enterDeepSleep(uint32_t ms) override {
        ESP.deepSleep(ms * 1000);
    }
    
    void softReset() override {
        ESP.restart();
    }
    
    String getResetReason() const override {
        return "ESP8266";  // Simplified
    }
};
#endif

// RP2040 Platform Pack (stub implementation)
#ifdef ARDUINO_ARCH_RP2040
class RP2040PlatformPack : public PlatformPack {
public:
    PlatformType getType() const override { return PlatformType::RP2040; }
    const char* getName() const override { return "RP2040"; }
    const char* getVersion() const override { return "1.0.0-stub"; }
    
    bool supportsWiFi() const override { return false; }  // Pico W has WiFi
    bool supportsBluetooth() const override { return false; }
    bool supportsI2C() const override { return true; }
    bool supportsSPI() const override { return true; }
    bool supportsADC() const override { return true; }
    bool supportsPWM() const override { return true; }
    
    uint32_t getFreeHeap() const override { return 256000; }  // Approximate
    uint32_t getTotalHeap() const override { return 264000; }  // 264KB RAM
    uint32_t getFlashSize() const override { return 2097152; }  // 2MB flash
    
    int getGPIOCount() const override { return 30; }
    
    bool isValidPin(int pin) const override {
        return pin >= 0 && pin < 30;
    }
    
    bool isSafePin(int pin) const override {
        // All GPIOs generally safe on RP2040
        return isValidPin(pin);
    }
    
    bool hasNVS() const override { return false; }
    bool hasEEPROM() const override { return false; }
    bool hasFilesystem() const override { return true; }
    
    bool supportsSleep() const override { return false; }  // Stub
    
    void enterLightSleep(uint32_t ms) override {
        delay(ms);
    }
    
    void enterDeepSleep(uint32_t ms) override {
        delay(ms);  // No deep sleep in stub
    }
    
    void softReset() override {
        // RP2040 reset (stub)
        while(1);
    }
    
    String getResetReason() const override {
        return "Power-on";
    }
};
#endif

// Factory function
PlatformPack* createPlatformPack() {
    #ifdef ESP32
    return new ESP32PlatformPack();
    #elif defined(ESP8266)
    return new ESP8266PlatformPack();
    #elif defined(ARDUINO_ARCH_RP2040)
    return new RP2040PlatformPack();
    #else
    return nullptr;
    #endif
}

} // namespace PocketOS
