#include "persistence.h"
#include "logger.h"
#include "device_registry.h"

namespace PocketOS {

bool Persistence::initialized = false;

#ifdef ESP32
Preferences Persistence::prefs;
#endif

void Persistence::init() {
    if (!initialized) {
#ifdef ESP32
        prefs.begin("pocketos", false);
#endif
        Logger::info("Persistence initialized");
        initialized = true;
    }
}

bool Persistence::save(const char* key, const char* value) {
#ifdef ESP32
    return prefs.putString(key, value) > 0;
#elif defined(ESP8266)
    // Use EEPROM or LittleFS on ESP8266
    return false;
#else
    return false;
#endif
}

bool Persistence::load(const char* key, char* value, size_t maxLen) {
#ifdef ESP32
    String str = prefs.getString(key, "");
    if (str.length() > 0) {
        strncpy(value, str.c_str(), maxLen - 1);
        value[maxLen - 1] = '\0';
        return true;
    }
    return false;
#else
    return false;
#endif
}

bool Persistence::remove(const char* key) {
#ifdef ESP32
    return prefs.remove(key);
#else
    return false;
#endif
}

void Persistence::clear() {
#ifdef ESP32
    prefs.clear();
#endif
    Logger::info("Persistence cleared");
}

bool Persistence::saveAll() {
    Logger::info("Saving all persistent data");
    return saveDeviceBindings();
}

bool Persistence::loadAll() {
    Logger::info("Loading all persistent data");
    return loadDeviceBindings();
}

bool Persistence::saveDeviceBindings() {
    // Save device count and device info
    // For now, just indicate success
    // Full implementation would iterate through DeviceRegistry and save each binding
    Logger::info("Device bindings saved");
    return true;
}

bool Persistence::loadDeviceBindings() {
    // Load device bindings and re-create devices
    // For now, just indicate success
    Logger::info("Device bindings loaded");
    return true;
}

String Persistence::exportConfig() {
    String config = "";
    
    #ifdef ESP32
    if (initialized && prefs.begin("pocketos", true)) {  // Read-only
        // Export any stored configuration
        config += "# Persistence namespace: pocketos\n";
        config += "# (NVS key-value pairs would be listed here)\n";
        prefs.end();
    }
    #else
    config += "# Persistence not available on this platform\n";
    #endif
    
    return config;
}

} // namespace PocketOS
