#include "device_registry.h"
#include "logger.h"
#include "resource_manager.h"
#include "endpoint_registry.h"
#include "../drivers/gpio_dout_driver.h"

namespace PocketOS {

Device DeviceRegistry::devices[MAX_DEVICES];
int DeviceRegistry::deviceCount = 0;
int DeviceRegistry::nextDeviceId = 1;

void DeviceRegistry::init() {
    deviceCount = 0;
    nextDeviceId = 1;
    for (int i = 0; i < MAX_DEVICES; i++) {
        devices[i].active = false;
        devices[i].driver = nullptr;
    }
    Logger::info("Device Registry initialized");
}

int DeviceRegistry::bindDevice(const String& driverId, const String& endpoint) {
    // Check if endpoint exists
    if (!EndpointRegistry::endpointExists(endpoint)) {
        // Try to register it dynamically for GPIO
        if (endpoint.startsWith("gpio.dout.")) {
            int pin = endpoint.substring(10).toInt();
            EndpointRegistry::registerEndpoint(endpoint, EndpointType::GPIO_DOUT, pin);
        } else {
            Logger::error("Endpoint not found");
            return -1;
        }
    }
    
    // Find free slot
    int slot = findFreeSlot();
    if (slot < 0) {
        Logger::error("No free device slots");
        return -1;
    }
    
    // Create driver instance
    IDriver* driver = createDriver(driverId, endpoint);
    if (!driver) {
        Logger::error("Failed to create driver");
        return -1;
    }
    
    // Initialize driver
    if (!driver->init()) {
        Logger::error("Driver init failed");
        delete driver;
        devices[slot].initFailCount++;
        return -1;
    }
    
    // Bind device
    int deviceId = nextDeviceId++;
    devices[slot].active = true;
    devices[slot].deviceId = deviceId;
    devices[slot].endpoint = endpoint;
    devices[slot].driverId = driverId;
    devices[slot].state = DeviceState::READY;
    devices[slot].driver = driver;
    devices[slot].lastOkMs = millis();
    deviceCount++;
    
    Logger::info(("Device " + String(deviceId) + " bound to " + endpoint).c_str());
    return deviceId;
}

bool DeviceRegistry::unbindDevice(int deviceId) {
    int idx = findDevice(deviceId);
    if (idx < 0) {
        return false;
    }
    
    // Clean up driver
    if (devices[idx].driver) {
        delete devices[idx].driver;
        devices[idx].driver = nullptr;
    }
    
    devices[idx].active = false;
    deviceCount--;
    Logger::info(("Device " + String(deviceId) + " unbound").c_str());
    return true;
}

bool DeviceRegistry::setDeviceEnabled(int deviceId, bool enabled) {
    int idx = findDevice(deviceId);
    if (idx < 0) {
        return false;
    }
    
    if (enabled) {
        devices[idx].state = DeviceState::READY;
    } else {
        devices[idx].state = DeviceState::DISABLED;
    }
    return true;
}

String DeviceRegistry::listDevices() {
    String result = "";
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (devices[i].active) {
            result += "dev" + String(devices[i].deviceId) + ": ";
            result += devices[i].driverId + " @ " + devices[i].endpoint;
            result += " [" + String(deviceStateToString(devices[i].state)) + "]";
            result += " fails:" + String(devices[i].initFailCount + devices[i].ioFailCount);
            result += "\n";
        }
    }
    if (result.length() == 0) {
        result = "No devices bound\n";
    }
    return result;
}

bool DeviceRegistry::deviceExists(int deviceId) {
    return findDevice(deviceId) >= 0;
}

DeviceState DeviceRegistry::getDeviceState(int deviceId) {
    int idx = findDevice(deviceId);
    if (idx >= 0) {
        return devices[idx].state;
    }
    return DeviceState::FAULT;
}

bool DeviceRegistry::setDeviceParam(int deviceId, const String& paramName, const String& value) {
    int idx = findDevice(deviceId);
    if (idx < 0 || !devices[idx].driver) {
        return false;
    }
    
    return devices[idx].driver->setParam(paramName, value);
}

String DeviceRegistry::getDeviceParam(int deviceId, const String& paramName) {
    int idx = findDevice(deviceId);
    if (idx < 0 || !devices[idx].driver) {
        return "";
    }
    
    return devices[idx].driver->getParam(paramName);
}

String DeviceRegistry::getDeviceSchema(int deviceId) {
    int idx = findDevice(deviceId);
    if (idx < 0 || !devices[idx].driver) {
        return "";
    }
    
    CapabilitySchema schema = devices[idx].driver->getSchema();
    return schema.serialize();
}

void DeviceRegistry::updateAll() {
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (devices[i].active && devices[i].driver && 
            devices[i].state == DeviceState::READY) {
            devices[i].driver->update();
        }
    }
}

int DeviceRegistry::findDevice(int deviceId) {
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (devices[i].active && devices[i].deviceId == deviceId) {
            return i;
        }
    }
    return -1;
}

int DeviceRegistry::findFreeSlot() {
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (!devices[i].active) {
            return i;
        }
    }
    return -1;
}

IDriver* DeviceRegistry::createDriver(const String& driverId, const String& endpoint) {
    if (driverId == "gpio.dout") {
        return new GPIODoutDriver(endpoint);
    }
    // Add more drivers here as needed
    return nullptr;
}

const char* DeviceRegistry::deviceStateToString(DeviceState state) {
    switch (state) {
        case DeviceState::READY: return "READY";
        case DeviceState::FAULT: return "FAULT";
        case DeviceState::DISABLED: return "DISABLED";
        default: return "UNKNOWN";
    }
}

String DeviceRegistry::getDeviceStatus(int deviceId) {
    int idx = findDevice(deviceId);
    if (idx < 0) {
        return "";
    }
    
    Device& dev = devices[idx];
    String status = "";
    status += "device_id=" + String(dev.deviceId) + "\n";
    status += "endpoint=" + dev.endpoint + "\n";
    status += "driver=" + dev.driverId + "\n";
    status += "state=" + String(deviceStateToString(dev.state)) + "\n";
    status += "init_failures=" + String(dev.initFailCount) + "\n";
    status += "io_failures=" + String(dev.ioFailCount) + "\n";
    status += "last_ok_ms=" + String(dev.lastOkMs) + "\n";
    status += "uptime_ms=" + String(millis() - dev.lastOkMs) + "\n";
    
    return status;
}

String DeviceRegistry::exportConfig() {
    String config = "";
    
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (devices[i].active) {
            config += "bind " + devices[i].driverId + " " + devices[i].endpoint + "\n";
            
            // Export device state
            if (devices[i].state == DeviceState::DISABLED) {
                config += "# dev.disable " + String(devices[i].deviceId) + "\n";
            }
        }
    }
    
    return config;
}

} // namespace PocketOS
