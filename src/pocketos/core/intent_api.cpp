#include "intent_api.h"
#include "hal.h"
#include "logger.h"
#include "resource_manager.h"
#include "endpoint_registry.h"
#include "device_registry.h"
#include "persistence.h"

namespace PocketOS {

bool IntentAPI::initialized = false;

void IntentAPI::init() {
    if (!initialized) {
        Logger::info("Intent API v" INTENT_API_VERSION " initialized");
        initialized = true;
    }
}

IntentResponse IntentAPI::dispatch(const IntentRequest& request) {
    if (request.intent == "sys.info") {
        return handleSysInfo(request);
    } else if (request.intent == "hal.caps") {
        return handleHalCaps(request);
    } else if (request.intent == "ep.list") {
        return handleEpList(request);
    } else if (request.intent == "ep.probe") {
        return handleEpProbe(request);
    } else if (request.intent == "dev.list") {
        return handleDevList(request);
    } else if (request.intent == "dev.bind") {
        return handleDevBind(request);
    } else if (request.intent == "dev.unbind") {
        return handleDevUnbind(request);
    } else if (request.intent == "dev.enable") {
        return handleDevEnable(request);
    } else if (request.intent == "dev.disable") {
        return handleDevDisable(request);
    } else if (request.intent == "dev.status") {
        return handleDevStatus(request);
    } else if (request.intent == "param.get") {
        return handleParamGet(request);
    } else if (request.intent == "param.set") {
        return handleParamSet(request);
    } else if (request.intent == "schema.get") {
        return handleSchemaGet(request);
    } else if (request.intent == "log.tail") {
        return handleLogTail(request);
    } else if (request.intent == "log.clear") {
        return handleLogClear(request);
    } else if (request.intent == "persist.save") {
        return handlePersistSave(request);
    } else if (request.intent == "persist.load") {
        return handlePersistLoad(request);
    } else if (request.intent == "config.export") {
        return handleConfigExport(request);
    } else if (request.intent == "config.import") {
        return handleConfigImport(request);
    } else if (request.intent == "bus.list") {
        return handleBusList(request);
    } else if (request.intent == "bus.info") {
        return handleBusInfo(request);
    } else if (request.intent == "bus.config") {
        return handleBusConfig(request);
    }
    
    return IntentResponse(IntentError::ERR_NOT_FOUND, "Unknown intent");
}

IntentResponse IntentAPI::handleSysInfo(const IntentRequest& req) {
    IntentResponse resp;
    resp.data = "version=" INTENT_API_VERSION "\n";
    resp.data += "board=" + String(HAL::getBoardName()) + "\n";
    resp.data += "chip=" + String(HAL::getChipFamily()) + "\n";
    resp.data += "flash_size=" + String(HAL::getFlashSize()) + "\n";
    resp.data += "heap_size=" + String(HAL::getHeapSize()) + "\n";
    resp.data += "free_heap=" + String(HAL::getFreeHeap()) + "\n";
    return resp;
}

IntentResponse IntentAPI::handleHalCaps(const IntentRequest& req) {
    IntentResponse resp;
    resp.data = "gpio_count=" + String(HAL::getGPIOCount()) + "\n";
    resp.data += "adc_count=" + String(HAL::getADCCount()) + "\n";
    resp.data += "pwm_count=" + String(HAL::getPWMCount()) + "\n";
    resp.data += "i2c_count=" + String(HAL::getI2CCount()) + "\n";
    resp.data += "spi_count=" + String(HAL::getSPICount()) + "\n";
    resp.data += "uart_count=" + String(HAL::getUARTCount()) + "\n";
    return resp;
}

IntentResponse IntentAPI::handleEpList(const IntentRequest& req) {
    IntentResponse resp;
    resp.data = EndpointRegistry::listEndpoints();
    return resp;
}

IntentResponse IntentAPI::handleEpProbe(const IntentRequest& req) {
    if (req.argCount < 1) {
        return IntentResponse(IntentError::ERR_BAD_ARGS, "Usage: ep.probe <endpoint>");
    }
    
    String result = EndpointRegistry::probeEndpoint(req.args[0]);
    if (result.length() > 0) {
        IntentResponse resp;
        resp.data = result;
        return resp;
    }
    return IntentResponse(IntentError::ERR_NOT_FOUND, "Endpoint not found or probe not supported");
}

IntentResponse IntentAPI::handleDevList(const IntentRequest& req) {
    IntentResponse resp;
    resp.data = DeviceRegistry::listDevices();
    return resp;
}

IntentResponse IntentAPI::handleDevBind(const IntentRequest& req) {
    if (req.argCount < 2) {
        return IntentResponse(IntentError::ERR_BAD_ARGS, "Usage: dev.bind <driver_id> <endpoint>");
    }
    
    int deviceId = DeviceRegistry::bindDevice(req.args[0], req.args[1]);
    if (deviceId >= 0) {
        IntentResponse resp;
        resp.data = "device_id=" + String(deviceId) + "\n";
        return resp;
    }
    return IntentResponse(IntentError::ERR_CONFLICT, "Failed to bind device");
}

IntentResponse IntentAPI::handleDevUnbind(const IntentRequest& req) {
    if (req.argCount < 1) {
        return IntentResponse(IntentError::ERR_BAD_ARGS, "Usage: dev.unbind <device_id>");
    }
    
    int deviceId = req.args[0].toInt();
    if (DeviceRegistry::unbindDevice(deviceId)) {
        return IntentResponse();
    }
    return IntentResponse(IntentError::ERR_NOT_FOUND, "Device not found");
}

IntentResponse IntentAPI::handleDevEnable(const IntentRequest& req) {
    if (req.argCount < 1) {
        return IntentResponse(IntentError::ERR_BAD_ARGS, "Usage: dev.enable <device_id>");
    }
    
    int deviceId = req.args[0].toInt();
    if (DeviceRegistry::setDeviceEnabled(deviceId, true)) {
        return IntentResponse();
    }
    return IntentResponse(IntentError::ERR_NOT_FOUND, "Device not found");
}

IntentResponse IntentAPI::handleDevDisable(const IntentRequest& req) {
    if (req.argCount < 1) {
        return IntentResponse(IntentError::ERR_BAD_ARGS, "Usage: dev.disable <device_id>");
    }
    
    int deviceId = req.args[0].toInt();
    if (DeviceRegistry::setDeviceEnabled(deviceId, false)) {
        return IntentResponse();
    }
    return IntentResponse(IntentError::ERR_NOT_FOUND, "Device not found");
}

IntentResponse IntentAPI::handleParamGet(const IntentRequest& req) {
    if (req.argCount < 2) {
        return IntentResponse(IntentError::ERR_BAD_ARGS, "Usage: param.get <device_id> <param_name>");
    }
    
    int deviceId = req.args[0].toInt();
    String value = DeviceRegistry::getDeviceParam(deviceId, req.args[1]);
    if (value.length() > 0) {
        IntentResponse resp;
        resp.data = req.args[1] + "=" + value + "\n";
        return resp;
    }
    return IntentResponse(IntentError::ERR_NOT_FOUND, "Parameter not found");
}

IntentResponse IntentAPI::handleParamSet(const IntentRequest& req) {
    if (req.argCount < 3) {
        return IntentResponse(IntentError::ERR_BAD_ARGS, "Usage: param.set <device_id> <param_name> <value>");
    }
    
    int deviceId = req.args[0].toInt();
    if (DeviceRegistry::setDeviceParam(deviceId, req.args[1], req.args[2])) {
        return IntentResponse();
    }
    return IntentResponse(IntentError::ERR_NOT_FOUND, "Device not found or parameter invalid");
}

IntentResponse IntentAPI::handleSchemaGet(const IntentRequest& req) {
    if (req.argCount < 1) {
        return IntentResponse(IntentError::ERR_BAD_ARGS, "Usage: schema.get <device_id>");
    }
    
    int deviceId = req.args[0].toInt();
    String schema = DeviceRegistry::getDeviceSchema(deviceId);
    if (schema.length() > 0) {
        IntentResponse resp;
        resp.data = schema;
        return resp;
    }
    return IntentResponse(IntentError::ERR_NOT_FOUND, "Device not found");
}

IntentResponse IntentAPI::handleLogTail(const IntentRequest& req) {
    int lines = 10;
    if (req.argCount > 0) {
        lines = req.args[0].toInt();
    }
    
    IntentResponse resp;
    resp.data = Logger::tail(lines);
    return resp;
}

IntentResponse IntentAPI::handleLogClear(const IntentRequest& req) {
    Logger::clear();
    return IntentResponse();
}

IntentResponse IntentAPI::handlePersistSave(const IntentRequest& req) {
    if (Persistence::saveAll()) {
        return IntentResponse();
    }
    return IntentResponse(IntentError::ERR_IO, "Failed to save");
}

IntentResponse IntentAPI::handlePersistLoad(const IntentRequest& req) {
    if (Persistence::loadAll()) {
        return IntentResponse();
    }
    return IntentResponse(IntentError::ERR_IO, "Failed to load");
}

IntentResponse IntentAPI::handleDevStatus(const IntentRequest& req) {
    if (req.argCount < 1) {
        return IntentResponse(IntentError::ERR_BAD_ARGS, "Usage: dev.status <device_id>");
    }
    
    int deviceId = req.args[0].toInt();
    String status = DeviceRegistry::getDeviceStatus(deviceId);
    if (status.length() > 0) {
        IntentResponse resp;
        resp.data = status;
        return resp;
    }
    return IntentResponse(IntentError::ERR_NOT_FOUND, "Device not found");
}

IntentResponse IntentAPI::handleConfigExport(const IntentRequest& req) {
    // Export configuration in text format
    String config = "# PocketOS Configuration Export\n";
    config += "# Generated: " + String(millis()) + "ms\n\n";
    
    // Export device bindings
    config += "# Device Bindings\n";
    config += DeviceRegistry::exportConfig();
    config += "\n";
    
    // Export persistence data
    config += "# Persistence Data\n";
    config += Persistence::exportConfig();
    
    IntentResponse resp;
    resp.data = config;
    return resp;
}

IntentResponse IntentAPI::handleConfigImport(const IntentRequest& req) {
    if (req.argCount < 1) {
        return IntentResponse(IntentError::ERR_BAD_ARGS, "Usage: config.import <config_data>");
    }
    
    // Import configuration from text format
    // This is a placeholder - full implementation would parse the config data
    return IntentResponse(IntentError::ERR_UNSUPPORTED, "Config import not yet implemented");
}

IntentResponse IntentAPI::handleBusList(const IntentRequest& req) {
    // List available buses
    IntentResponse resp;
    resp.data = "";
    
    // List I2C buses
    int i2cCount = HAL::getI2CCount();
    for (int i = 0; i < i2cCount; i++) {
        resp.data += "i2c" + String(i) + " (I2C Bus " + String(i) + ")\n";
    }
    
    // List SPI buses
    int spiCount = HAL::getSPICount();
    for (int i = 0; i < spiCount; i++) {
        resp.data += "spi" + String(i) + " (SPI Bus " + String(i) + ")\n";
    }
    
    // List UART ports
    int uartCount = HAL::getUARTCount();
    for (int i = 0; i < uartCount; i++) {
        resp.data += "uart" + String(i) + " (UART Port " + String(i) + ")\n";
    }
    
    if (resp.data.length() == 0) {
        resp.data = "No buses available\n";
    }
    
    return resp;
}

IntentResponse IntentAPI::handleBusInfo(const IntentRequest& req) {
    if (req.argCount < 1) {
        return IntentResponse(IntentError::ERR_BAD_ARGS, "Usage: bus.info <bus_name>");
    }
    
    String busName = req.args[0];
    IntentResponse resp;
    
    if (busName.startsWith("i2c")) {
        int busNum = busName.substring(3).toInt();
        resp.data = "Bus: " + busName + "\n";
        resp.data += "Type: I2C\n";
        resp.data += "Status: ";
        resp.data += (busNum < HAL::getI2CCount()) ? "Available\n" : "Not available\n";
        resp.data += "Frequency: 100kHz (default)\n";
    } else if (busName.startsWith("spi")) {
        int busNum = busName.substring(3).toInt();
        resp.data = "Bus: " + busName + "\n";
        resp.data += "Type: SPI\n";
        resp.data += "Status: ";
        resp.data += (busNum < HAL::getSPICount()) ? "Available\n" : "Not available\n";
    } else if (busName.startsWith("uart")) {
        int busNum = busName.substring(4).toInt();
        resp.data = "Bus: " + busName + "\n";
        resp.data += "Type: UART\n";
        resp.data += "Status: ";
        resp.data += (busNum < HAL::getUARTCount()) ? "Available\n" : "Not available\n";
    } else {
        return IntentResponse(IntentError::ERR_NOT_FOUND, "Bus not found");
    }
    
    return resp;
}

IntentResponse IntentAPI::handleBusConfig(const IntentRequest& req) {
    if (req.argCount < 1) {
        return IntentResponse(IntentError::ERR_BAD_ARGS, "Usage: bus.config <bus_name> [params...]");
    }
    
    // Bus configuration is a placeholder for future implementation
    // Would configure I2C frequency, SPI mode, UART baud rate, etc.
    return IntentResponse(IntentError::ERR_UNSUPPORTED, "Bus configuration not yet implemented");
}

} // namespace PocketOS
