#include "platform_pack.h"

namespace PocketOS {

// Global platform pack instance
PlatformPack* g_platformPack = nullptr;

// Forward declarations for platform-specific factory functions
#ifdef ESP32
extern PlatformPack* createESP32PlatformPack();
#endif

#ifdef ESP8266
extern PlatformPack* createESP8266PlatformPack();
#endif

#ifdef ARDUINO_ARCH_RP2040
extern PlatformPack* createRP2040PlatformPack();
#endif

// Factory function to create appropriate platform pack
PlatformPack* createPlatformPack() {
    #ifdef ESP32
    return createESP32PlatformPack();
    #elif defined(ESP8266)
    return createESP8266PlatformPack();
    #elif defined(ARDUINO_ARCH_RP2040)
    return createRP2040PlatformPack();
    #else
    #error "Unsupported platform - PocketOS requires ESP32, ESP8266, or RP2040"
    #endif
}

// Initialize platform pack (call once at startup)
bool initializePlatformPack() {
    if (g_platformPack == nullptr) {
        g_platformPack = createPlatformPack();
        if (g_platformPack) {
            return g_platformPack->initStorage();
        }
    }
    return g_platformPack != nullptr;
}

} // namespace PocketOS

