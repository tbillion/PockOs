#ifndef POCKETOS_CAPABILITY_SCHEMA_H
#define POCKETOS_CAPABILITY_SCHEMA_H

#include <Arduino.h>

namespace PocketOS {

#define MAX_SCHEMA_SETTINGS 8
#define MAX_SCHEMA_SIGNALS 8
#define MAX_SCHEMA_COMMANDS 8

enum class ParamType {
    BOOL,
    INT,
    FLOAT,
    ENUM,
    STRING
};

struct SchemaParam {
    bool active;
    String name;
    ParamType type;
    bool readWrite;  // true = RW, false = RO
    String units;
    float minValue;
    float maxValue;
    float stepValue;
    
    SchemaParam() : active(false), name(""), type(ParamType::INT), 
                    readWrite(true), units(""), 
                    minValue(0), maxValue(0), stepValue(0) {}
};

struct SchemaSignal {
    bool active;
    String name;
    ParamType type;
    bool readWrite;
    String units;
    
    SchemaSignal() : active(false), name(""), type(ParamType::INT), 
                     readWrite(false), units("") {}
};

struct SchemaCommand {
    bool active;
    String name;
    String argsSchema;  // Simple string description of args
    
    SchemaCommand() : active(false), name(""), argsSchema("") {}
};

class CapabilitySchema {
public:
    SchemaParam settings[MAX_SCHEMA_SETTINGS];
    SchemaSignal signals[MAX_SCHEMA_SIGNALS];
    SchemaCommand commands[MAX_SCHEMA_COMMANDS];
    
    int settingCount;
    int signalCount;
    int commandCount;
    
    CapabilitySchema() : settingCount(0), signalCount(0), commandCount(0) {}
    
    // Add items to schema
    bool addSetting(const String& name, ParamType type, bool rw, 
                   float minVal = 0, float maxVal = 0, float step = 0, 
                   const String& units = "");
    bool addSignal(const String& name, ParamType type, bool rw, const String& units = "");
    bool addCommand(const String& name, const String& argsSchema = "");
    
    // Serialize to line-oriented format
    String serialize() const;
    
    // Helper to convert param type to string
    static const char* paramTypeToString(ParamType type);
};

} // namespace PocketOS

#endif // POCKETOS_CAPABILITY_SCHEMA_H
