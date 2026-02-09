#include "pcf1_config.h"
#include "logger.h"
#include "hal.h"
#include "device_registry.h"
#include "endpoint_registry.h"
#include "persistence.h"

namespace PocketOS {

String PCF1Config::_validationErrors = "";

void PCF1Config::init() {
    Logger::info("PCF1Config initialized");
}

String PCF1Config::exportConfig() {
    String config = "";
    
    // System section
    config += "[system]\n";
    config += "version=1.0.0\n";
    config += "platform=" + String(HAL::getBoardName()) + "\n";
    config += "chip=" + String(HAL::getChipFamily()) + "\n";
    config += "\n";
    
    // HAL capabilities
    config += "[hal]\n";
    config += "gpio_count=" + String(HAL::getGPIOCount()) + "\n";
    config += "adc_channels=" + String(HAL::getADCChannels()) + "\n";
    config += "pwm_channels=" + String(HAL::getPWMChannels()) + "\n";
    config += "i2c_count=" + String(HAL::getI2CCount()) + "\n";
    config += "\n";
    
    // I2C bus configuration (if configured)
    config += "[i2c0]\n";
    config += "sda=21\n";
    config += "scl=22\n";
    config += "speed_hz=400000\n";
    config += "\n";
    
    // Device configurations
    config += DeviceRegistry::exportConfig();
    
    return config;
}

bool PCF1Config::importConfig(const String& config, bool validateOnly) {
    _validationErrors = "";
    
    if (!validateConfig(config)) {
        return false;
    }
    
    if (validateOnly) {
        return true;
    }
    
    return applyConfiguration(config);
}

bool PCF1Config::validateConfig(const String& config) {
    _validationErrors = "";
    
    if (config.length() == 0) {
        _validationErrors = "Empty configuration";
        return false;
    }
    
    String currentSection = "";
    int lineNum = 0;
    int startIdx = 0;
    
    while (startIdx < config.length()) {
        int endIdx = config.indexOf('\n', startIdx);
        if (endIdx == -1) endIdx = config.length();
        
        String line = config.substring(startIdx, endIdx);
        line.trim();
        lineNum++;
        
        if (line.length() > 0 && !line.startsWith("#")) {
            if (line.startsWith("[") && line.endsWith("]")) {
                // Section header
                currentSection = line.substring(1, line.length() - 1);
            } else {
                // Key=value pair
                String key, value;
                String section = currentSection;
                if (!parseLine(line, section, key, value)) {
                    _validationErrors += "Line " + String(lineNum) + ": Invalid format\n";
                    return false;
                }
                
                if (!validateSection(currentSection, key, value)) {
                    _validationErrors += "Line " + String(lineNum) + ": Validation failed for " + 
                                        currentSection + "." + key + "=" + value + "\n";
                    return false;
                }
            }
        }
        
        startIdx = endIdx + 1;
    }
    
    return true;
}

bool PCF1Config::factoryReset() {
    Logger::info("Factory reset initiated");
    
    // Unbind all devices
    DeviceRegistry::unbindAll();
    
    // Clear persistence
    Persistence::clear();
    
    Logger::info("Factory reset complete");
    return true;
}

String PCF1Config::getValidationErrors() {
    return _validationErrors;
}

bool PCF1Config::parseLine(const String& line, String& section, String& key, String& value) {
    int eqIdx = line.indexOf('=');
    if (eqIdx == -1) {
        return false;
    }
    
    key = line.substring(0, eqIdx);
    key.trim();
    value = line.substring(eqIdx + 1);
    value.trim();
    
    return key.length() > 0;
}

bool PCF1Config::validateSection(const String& section, const String& key, const String& value) {
    if (section == "system") {
        if (key == "version" || key == "platform" || key == "chip") {
            return true;
        }
    } else if (section == "hal") {
        if (key == "gpio_count" || key == "adc_channels" || 
            key == "pwm_channels" || key == "i2c_count") {
            return value.toInt() >= 0;
        }
    } else if (section == "i2c0") {
        if (key == "sda" || key == "scl") {
            int pin = value.toInt();
            return pin >= 0 && pin < 40;  // ESP32 pin range
        }
        if (key == "speed_hz") {
            int speed = value.toInt();
            return speed > 0 && speed <= 1000000;  // Valid I2C speeds
        }
    } else if (section.startsWith("device:")) {
        if (key == "endpoint" || key == "driver" || key == "state") {
            return true;
        }
    }
    
    // Unknown sections/keys are allowed (future compatibility)
    return true;
}

bool PCF1Config::applyConfiguration(const String& config) {
    Logger::info("Applying configuration");
    
    // Parse and apply configuration
    String currentSection = "";
    int startIdx = 0;
    
    while (startIdx < config.length()) {
        int endIdx = config.indexOf('\n', startIdx);
        if (endIdx == -1) endIdx = config.length();
        
        String line = config.substring(startIdx, endIdx);
        line.trim();
        
        if (line.length() > 0 && !line.startsWith("#")) {
            if (line.startsWith("[") && line.endsWith("]")) {
                currentSection = line.substring(1, line.length() - 1);
            } else {
                String key, value;
                String section = currentSection;
                if (parseLine(line, section, key, value)) {
                    // Apply configuration based on section
                    // (Would integrate with device binding, bus config, etc.)
                    Logger::info("Config: " + currentSection + "." + key + "=" + value);
                }
            }
        }
        
        startIdx = endIdx + 1;
    }
    
    return true;
}

} // namespace PocketOS
