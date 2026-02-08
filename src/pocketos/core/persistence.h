#ifndef POCKETOS_PERSISTENCE_H
#define POCKETOS_PERSISTENCE_H

#include <Arduino.h>

#ifdef ESP32
#include <Preferences.h>
#endif

namespace PocketOS {

class Persistence {
public:
    static void init();
    static bool save(const char* key, const char* value);
    static bool load(const char* key, char* value, size_t maxLen);
    static bool remove(const char* key);
    static void clear();
    
    // High-level functions
    static bool saveAll();
    static bool loadAll();
    static bool saveDeviceBindings();
    static bool loadDeviceBindings();
    
private:
    static bool initialized;
    
#ifdef ESP32
    static Preferences prefs;
#endif
};

} // namespace PocketOS

#endif // POCKETOS_PERSISTENCE_H
