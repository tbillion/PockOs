#include "capability_schema.h"

namespace PocketOS {

bool CapabilitySchema::addSetting(const String& name, ParamType type, bool rw,
                                  float minVal, float maxVal, float step,
                                  const String& units) {
    if (settingCount >= MAX_SCHEMA_SETTINGS) {
        return false;
    }
    
    settings[settingCount].active = true;
    settings[settingCount].name = name;
    settings[settingCount].type = type;
    settings[settingCount].readWrite = rw;
    settings[settingCount].minValue = minVal;
    settings[settingCount].maxValue = maxVal;
    settings[settingCount].stepValue = step;
    settings[settingCount].units = units;
    settingCount++;
    return true;
}

bool CapabilitySchema::addSignal(const String& name, ParamType type, bool rw, const String& units) {
    if (signalCount >= MAX_SCHEMA_SIGNALS) {
        return false;
    }
    
    signals[signalCount].active = true;
    signals[signalCount].name = name;
    signals[signalCount].type = type;
    signals[signalCount].readWrite = rw;
    signals[signalCount].units = units;
    signalCount++;
    return true;
}

bool CapabilitySchema::addCommand(const String& name, const String& argsSchema) {
    if (commandCount >= MAX_SCHEMA_COMMANDS) {
        return false;
    }
    
    commands[commandCount].active = true;
    commands[commandCount].name = name;
    commands[commandCount].argsSchema = argsSchema;
    commandCount++;
    return true;
}

String CapabilitySchema::serialize() const {
    String result = "";
    
    // Settings section
    if (settingCount > 0) {
        result += "[settings]\n";
        for (int i = 0; i < settingCount; i++) {
            if (settings[i].active) {
                result += settings[i].name + ":";
                result += paramTypeToString(settings[i].type);
                result += settings[i].readWrite ? ":rw" : ":ro";
                if (settings[i].minValue != settings[i].maxValue) {
                    result += ":" + String(settings[i].minValue, 2);
                    result += "-" + String(settings[i].maxValue, 2);
                }
                if (settings[i].units.length() > 0) {
                    result += ":" + settings[i].units;
                }
                result += "\n";
            }
        }
    }
    
    // Signals section
    if (signalCount > 0) {
        result += "[signals]\n";
        for (int i = 0; i < signalCount; i++) {
            if (signals[i].active) {
                result += signals[i].name + ":";
                result += paramTypeToString(signals[i].type);
                result += signals[i].readWrite ? ":rw" : ":ro";
                if (signals[i].units.length() > 0) {
                    result += ":" + signals[i].units;
                }
                result += "\n";
            }
        }
    }
    
    // Commands section
    if (commandCount > 0) {
        result += "[commands]\n";
        for (int i = 0; i < commandCount; i++) {
            if (commands[i].active) {
                result += commands[i].name;
                if (commands[i].argsSchema.length() > 0) {
                    result += " " + commands[i].argsSchema;
                }
                result += "\n";
            }
        }
    }
    
    return result;
}

const char* CapabilitySchema::paramTypeToString(ParamType type) {
    switch (type) {
        case ParamType::BOOL: return "bool";
        case ParamType::INT: return "int";
        case ParamType::FLOAT: return "float";
        case ParamType::ENUM: return "enum";
        case ParamType::STRING: return "string";
        case ParamType::EVENT: return "event";
        case ParamType::COUNTER: return "counter";
        case ParamType::BLOB: return "blob";
        default: return "unknown";
    }
}

} // namespace PocketOS
