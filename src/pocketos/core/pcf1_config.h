#ifndef POCKETOS_PCF1_CONFIG_H
#define POCKETOS_PCF1_CONFIG_H

#include <Arduino.h>

namespace PocketOS {

/**
 * PCF1 (PocketOS Configuration Format 1)
 * 
 * Human-readable text configuration format for complete system state.
 * Supports validation, import, export, and persistence.
 * 
 * Format: Line-oriented key=value with sections
 * Example:
 *   [system]
 *   version=1.0.0
 *   platform=esp32
 *   
 *   [i2c0]
 *   sda=21
 *   scl=22
 *   speed_hz=400000
 *   
 *   [device:1]
 *   endpoint=i2c0:0x76
 *   driver=bme280
 *   state=enabled
 */

class PCF1Config {
public:
    static void init();
    
    // Export current configuration to text format
    static String exportConfig();
    
    // Import configuration from text format
    // Returns: true if successful, false if validation failed
    static bool importConfig(const String& config, bool validateOnly = false);
    
    // Validate configuration without importing
    static bool validateConfig(const String& config);
    
    // Factory reset - clear all configuration
    static bool factoryReset();
    
    // Get validation errors from last operation
    static String getValidationErrors();
    
private:
    static String _validationErrors;
    
    static bool parseLine(const String& line, String& section, String& key, String& value);
    static bool validateSection(const String& section, const String& key, const String& value);
    static bool applyConfiguration(const String& config);
};

} // namespace PocketOS

#endif // POCKETOS_PCF1_CONFIG_H
