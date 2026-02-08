#ifndef POCKETOS_DEVICE_REGISTRY_H
#define POCKETOS_DEVICE_REGISTRY_H

#include <Arduino.h>
#include "capability_schema.h"

namespace PocketOS {

#define MAX_DEVICES 16

enum class DeviceState {
    READY,
    FAULT,
    DISABLED
};

// Forward declaration of base driver interface
class IDriver {
public:
    virtual ~IDriver() {}
    virtual bool init() = 0;
    virtual bool setParam(const String& name, const String& value) = 0;
    virtual String getParam(const String& name) = 0;
    virtual CapabilitySchema getSchema() = 0;
    virtual void update() = 0;
};

struct Device {
    bool active;
    int deviceId;
    String endpoint;
    String driverId;
    DeviceState state;
    IDriver* driver;  // Pointer to driver instance
    
    // Health counters
    int initFailCount;
    int ioFailCount;
    unsigned long lastOkMs;
    
    Device() : active(false), deviceId(-1), endpoint(""), driverId(""), 
               state(DeviceState::DISABLED), driver(nullptr),
               initFailCount(0), ioFailCount(0), lastOkMs(0) {}
};

class DeviceRegistry {
public:
    static void init();
    
    // Device lifecycle
    static int bindDevice(const String& driverId, const String& endpoint);
    static bool unbindDevice(int deviceId);
    static bool unbindAll();  // Unbind all devices
    static bool setDeviceEnabled(int deviceId, bool enabled);
    
    // Device queries
    static String listDevices();
    static bool deviceExists(int deviceId);
    static DeviceState getDeviceState(int deviceId);
    
    // Device parameters
    static bool setDeviceParam(int deviceId, const String& paramName, const String& value);
    static String getDeviceParam(int deviceId, const String& paramName);
    
    // Schema query
    static String getDeviceSchema(int deviceId);
    
    // Device status and health
    static String getDeviceStatus(int deviceId);
    
    // Config export
    static String exportConfig();
    
    // Update all devices
    static void updateAll();
    
    // Device count
    static int getDeviceCount() { return deviceCount; }
    
private:
    static Device devices[MAX_DEVICES];
    static int deviceCount;
    static int nextDeviceId;
    
    static int findDevice(int deviceId);
    static int findFreeSlot();
    static IDriver* createDriver(const String& driverId, const String& endpoint);
    static const char* deviceStateToString(DeviceState state);
};

} // namespace PocketOS

#endif // POCKETOS_DEVICE_REGISTRY_H
