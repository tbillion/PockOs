#include "intent_api.h"
#include "hal.h"
#include "logger.h"
#include "resource_manager.h"
#include "endpoint_registry.h"
#include "device_registry.h"
#include "persistence.h"
#include "device_identifier.h"
#include "../drivers/bme280_driver.h"

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
    } else if (request.intent == "identify") {
        return handleIdentify(request);
    } else if (request.intent == "dev.read") {
        return handleDeviceRead(request);
    } else if (request.intent == "dev.stream") {
        return handleDeviceStream(request);
    } else if (request.intent == "factory_reset") {
        return handleFactoryReset(request);
    } else if (request.intent == "config.validate") {
        return handleConfigValidate(request);
    } else if (request.intent == "reg.list") {
        return handleRegList(request);
    } else if (request.intent == "reg.read") {
        return handleRegRead(request);
    } else if (request.intent == "reg.write") {
        return handleRegWrite(request);
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
        return IntentResponse(IntentError::ERR_BAD_ARGS, "Usage: bus.config <bus_name> [param=value...]");
    }
    
    String busName = req.args[0];
    
    if (busName == "i2c0") {
        // Parse I2C configuration parameters
        int sda = -1, scl = -1;
        uint32_t speedHz = 100000;  // Default 100kHz
        
        for (int i = 1; i < req.argCount; i++) {
            String param = req.args[i];
            int eqPos = param.indexOf('=');
            if (eqPos > 0) {
                String key = param.substring(0, eqPos);
                String value = param.substring(eqPos + 1);
                
                if (key == "sda") {
                    sda = value.toInt();
                } else if (key == "scl") {
                    scl = value.toInt();
                } else if (key == "speed_hz" || key == "speed") {
                    speedHz = value.toInt();
                }
            }
        }
        
        if (HAL::i2cInit(0, sda, scl, speedHz)) {
            IntentResponse resp;
            resp.data = "bus=i2c0\n";
            resp.data += "sda=" + String(sda < 0 ? 21 : sda) + "\n";
            resp.data += "scl=" + String(scl < 0 ? 22 : scl) + "\n";
            resp.data += "speed_hz=" + String(speedHz) + "\n";
            resp.data += "status=configured\n";
            return resp;
        } else {
            return IntentResponse(IntentError::ERR_IO, "Failed to configure I2C bus");
        }
    }
    
    return IntentResponse(IntentError::ERR_NOT_FOUND, "Unknown bus: " + busName);
}

IntentResponse IntentAPI::handleIdentify(const IntentRequest& req) {
    if (req.argCount < 1) {
        return IntentResponse(IntentError::ERR_BAD_ARGS, "Usage: identify <endpoint>");
    }
    
    String endpoint = req.args[0];
    DeviceIdentification id = DeviceIdentifier::identifyEndpoint(endpoint);
    
    IntentResponse resp;
    resp.data = "endpoint=" + endpoint + "\n";
    resp.data += "identified=" + String(id.identified ? "true" : "false") + "\n";
    resp.data += "device_class=" + id.deviceClass + "\n";
    resp.data += "confidence=" + id.confidence + "\n";
    if (id.details.length() > 0) {
        resp.data += "details=" + id.details + "\n";
    }
    
    return resp;
}

IntentResponse IntentAPI::handleDeviceRead(const IntentRequest& req) {
    if (req.argCount < 1) {
        return IntentResponse(IntentError::ERR_BAD_ARGS, "Usage: dev.read <device_id>");
    }
    
    int deviceId = req.args[0].toInt();
    Device* device = DeviceRegistry::getDevice(deviceId);
    
    if (!device || !device->active) {
        return IntentResponse(IntentError::ERR_NOT_FOUND, "Device not found");
    }
    
    // Check if device is a BME280
    if (device->driverId == "bme280") {
        // Get the driver instance (in a real implementation, we'd store driver instances)
        // For now, create a temporary driver to read
        BME280Driver driver;
        
        // Parse I2C address from endpoint (e.g., "i2c0:0x76")
        String endpoint = device->endpointRef;
        int colonPos = endpoint.indexOf(':');
        if (colonPos > 0) {
            String addrStr = endpoint.substring(colonPos + 1);
            uint8_t address = (uint8_t)strtol(addrStr.c_str(), nullptr, 16);
            
            if (driver.init(address)) {
                BME280Data data = driver.readData();
                driver.deinit();
                
                if (data.valid) {
                    IntentResponse resp;
                    resp.data = "device_id=" + String(deviceId) + "\n";
                    resp.data += "driver=" + device->driverId + "\n";
                    resp.data += "temperature=" + String(data.temperature, 2) + "\n";
                    resp.data += "humidity=" + String(data.humidity, 2) + "\n";
                    resp.data += "pressure=" + String(data.pressure, 2) + "\n";
                    resp.data += "temp_unit=°C\n";
                    resp.data += "hum_unit=%RH\n";
                    resp.data += "press_unit=hPa\n";
                    return resp;
                } else {
                    return IntentResponse(IntentError::ERR_IO, "Failed to read sensor data");
                }
            } else {
                return IntentResponse(IntentError::ERR_IO, "Failed to initialize driver");
            }
        }
    }
    
    return IntentResponse(IntentError::ERR_UNSUPPORTED, "Device driver does not support read operation");
}

IntentResponse IntentAPI::handleDeviceStream(const IntentRequest& req) {
    if (req.argCount < 3) {
        return IntentResponse(IntentError::ERR_BAD_ARGS, "Usage: dev.stream <device_id> <interval_ms> <count>");
    }
    
    int deviceId = req.args[0].toInt();
    int intervalMs = req.args[1].toInt();
    int count = req.args[2].toInt();
    
    if (intervalMs < 100) intervalMs = 100;  // Minimum 100ms
    if (count < 1 || count > 100) count = 10;  // Limit to reasonable range
    
    Device* device = DeviceRegistry::getDevice(deviceId);
    
    if (!device || !device->active) {
        return IntentResponse(IntentError::ERR_NOT_FOUND, "Device not found");
    }
    
    // Check if device is a BME280
    if (device->driverId == "bme280") {
        String endpoint = device->endpointRef;
        int colonPos = endpoint.indexOf(':');
        if (colonPos > 0) {
            String addrStr = endpoint.substring(colonPos + 1);
            uint8_t address = (uint8_t)strtol(addrStr.c_str(), nullptr, 16);
            
            BME280Driver driver;
            if (driver.init(address)) {
                IntentResponse resp;
                resp.data = "device_id=" + String(deviceId) + "\n";
                resp.data += "interval_ms=" + String(intervalMs) + "\n";
                resp.data += "count=" + String(count) + "\n";
                resp.data += "streaming=start\n";
                
                // Perform the streaming reads
                for (int i = 0; i < count; i++) {
                    BME280Data data = driver.readData();
                    if (data.valid) {
                        resp.data += "sample=" + String(i + 1) + " ";
                        resp.data += "temp=" + String(data.temperature, 2) + "°C ";
                        resp.data += "hum=" + String(data.humidity, 1) + "%RH ";
                        resp.data += "press=" + String(data.pressure, 1) + "hPa\n";
                    } else {
                        resp.data += "sample=" + String(i + 1) + " ERROR\n";
                    }
                    
                    if (i < count - 1) {
                        delay(intervalMs);
                    }
                }
                
                resp.data += "streaming=complete\n";
                driver.deinit();
                return resp;
            } else {
                return IntentResponse(IntentError::ERR_IO, "Failed to initialize driver");
            }
        }
    }
    
    return IntentResponse(IntentError::ERR_UNSUPPORTED, "Device driver does not support stream operation");
}

IntentResponse IntentAPI::handleFactoryReset(const IntentRequest& req) {
    #include "pcf1_config.h"
    
    if (PCF1Config::factoryReset()) {
        IntentResponse resp;
        resp.data = "status=reset_complete\n";
        resp.data += "message=All configuration cleared\n";
        resp.message = "Factory reset complete";
        return resp;
    }
    
    return IntentResponse(IntentError::ERR_INTERNAL, "Factory reset failed");
}

IntentResponse IntentAPI::handleConfigValidate(const IntentRequest& req) {
    #include "pcf1_config.h"
    
    if (req.args.size() < 1) {
        return IntentResponse(IntentError::ERR_BAD_ARGS, "Config data required");
    }
    
    String config = req.args[0];
    for (size_t i = 1; i < req.args.size(); i++) {
        config += " " + req.args[i];
    }
    
    if (PCF1Config::validateConfig(config)) {
        IntentResponse resp;
        resp.data = "valid=true\n";
        resp.message = "Configuration is valid";
        return resp;
    } else {
        IntentResponse resp(IntentError::ERR_BAD_ARGS, "Configuration validation failed");
        resp.data = "valid=false\n";
        resp.data += "errors=" + PCF1Config::getValidationErrors() + "\n";
        return resp;
    }
}

IntentResponse IntentAPI::handleRegList(const IntentRequest& req) {
    // reg.list <device_id>
    if (req.argCount < 1) {
        return IntentResponse(IntentError::ERR_BAD_ARGS, "Usage: reg.list <device_id>");
    }
    
    int deviceId = req.args[0].toInt();
    if (!DeviceRegistry::deviceExists(deviceId)) {
        return IntentResponse(IntentError::ERR_NOT_FOUND, "Device not found");
    }
    
    if (!DeviceRegistry::deviceSupportsRegisters(deviceId)) {
        return IntentResponse(IntentError::ERR_UNSUPPORTED, 
            "Device does not support register access. Enable POCKETOS_DRIVER_TIER=2 and use Tier 2 driver.");
    }
    
    String regList = DeviceRegistry::getDeviceRegisters(deviceId);
    if (regList.length() == 0) {
        return IntentResponse(IntentError::ERR_INTERNAL, "Failed to retrieve register list");
    }
    
    IntentResponse resp;
    resp.data = regList;
    return resp;
}

IntentResponse IntentAPI::handleRegRead(const IntentRequest& req) {
    // reg.read <device_id> <reg|name> [len]
    if (req.argCount < 2) {
        return IntentResponse(IntentError::ERR_BAD_ARGS, "Usage: reg.read <device_id> <reg|name> [len]");
    }
    
    int deviceId = req.args[0].toInt();
    if (!DeviceRegistry::deviceExists(deviceId)) {
        return IntentResponse(IntentError::ERR_NOT_FOUND, "Device not found");
    }
    
    if (!DeviceRegistry::deviceSupportsRegisters(deviceId)) {
        return IntentResponse(IntentError::ERR_UNSUPPORTED, 
            "Device does not support register access. Enable POCKETOS_DRIVER_TIER=2 and use Tier 2 driver.");
    }
    
    // Parse register address (hex or decimal)
    String regStr = req.args[1];
    uint16_t regAddr;
    
    if (regStr.startsWith("0x") || regStr.startsWith("0X")) {
        regAddr = (uint16_t)strtol(regStr.c_str() + 2, nullptr, 16);
    } else {
        // Try as decimal first
        regAddr = (uint16_t)regStr.toInt();
        // If it's 0 and not "0", might be a name - handle in driver layer
        // For now, support hex/decimal addressing
    }
    
    // Determine read length (default to 1)
    size_t len = 1;
    if (req.argCount >= 3) {
        len = req.args[2].toInt();
        if (len == 0 || len > 16) {
            return IntentResponse(IntentError::ERR_BAD_ARGS, "Length must be 1-16");
        }
    }
    
    // Read the register(s)
    uint8_t buf[16];
    if (!DeviceRegistry::deviceRegRead(deviceId, regAddr, buf, len)) {
        return IntentResponse(IntentError::ERR_IO, "Failed to read register");
    }
    
    // Format response
    IntentResponse resp;
    resp.data = "register=0x" + String(regAddr, HEX) + "\n";
    resp.data += "value=";
    for (size_t i = 0; i < len; i++) {
        if (i > 0) resp.data += ":";
        if (buf[i] < 16) resp.data += "0";
        resp.data += String(buf[i], HEX);
    }
    resp.data += "\n";
    resp.data += "length=" + String(len) + "\n";
    
    return resp;
}

IntentResponse IntentAPI::handleRegWrite(const IntentRequest& req) {
    // reg.write <device_id> <reg|name> <value> [len]
    if (req.argCount < 3) {
        return IntentResponse(IntentError::ERR_BAD_ARGS, "Usage: reg.write <device_id> <reg|name> <value> [len]");
    }
    
    int deviceId = req.args[0].toInt();
    if (!DeviceRegistry::deviceExists(deviceId)) {
        return IntentResponse(IntentError::ERR_NOT_FOUND, "Device not found");
    }
    
    if (!DeviceRegistry::deviceSupportsRegisters(deviceId)) {
        return IntentResponse(IntentError::ERR_UNSUPPORTED, 
            "Device does not support register access. Enable POCKETOS_DRIVER_TIER=2 and use Tier 2 driver.");
    }
    
    // Parse register address
    String regStr = req.args[1];
    uint16_t regAddr;
    
    if (regStr.startsWith("0x") || regStr.startsWith("0X")) {
        regAddr = (uint16_t)strtol(regStr.c_str() + 2, nullptr, 16);
    } else {
        regAddr = (uint16_t)regStr.toInt();
    }
    
    // Parse value
    String valueStr = req.args[2];
    uint32_t value;
    
    if (valueStr.startsWith("0x") || valueStr.startsWith("0X")) {
        value = (uint32_t)strtol(valueStr.c_str() + 2, nullptr, 16);
    } else {
        value = (uint32_t)valueStr.toInt();
    }
    
    // For now, only support single byte writes
    uint8_t buf[1];
    buf[0] = (uint8_t)(value & 0xFF);
    
    // Write the register
    if (!DeviceRegistry::deviceRegWrite(deviceId, regAddr, buf, 1)) {
        return IntentResponse(IntentError::ERR_IO, "Failed to write register (may be read-only)");
    }
    
    IntentResponse resp;
    resp.data = "register=0x" + String(regAddr, HEX) + "\n";
    resp.data += "value=0x" + String(buf[0], HEX) + "\n";
    return resp;
}

} // namespace PocketOS
