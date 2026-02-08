#ifndef POCKETOS_INTENT_API_H
#define POCKETOS_INTENT_API_H

#include <Arduino.h>

namespace PocketOS {

// Intent API version
#define INTENT_API_VERSION "1.0.0"

// Error codes - stable v1 error model
enum class IntentError {
    OK = 0,
    ERR_BAD_ARGS = 1,
    ERR_NOT_FOUND = 2,
    ERR_CONFLICT = 3,
    ERR_IO = 4,
    ERR_UNSUPPORTED = 5,
    ERR_INTERNAL = 6
};

// Intent response structure (line-oriented)
struct IntentResponse {
    IntentError error;
    String message;
    String data;  // Line-oriented data (newline-separated fields)
    
    IntentResponse() : error(IntentError::OK), message(""), data("") {}
    IntentResponse(IntentError err, const char* msg = "") : error(err), message(msg), data("") {}
    
    bool isOk() const { return error == IntentError::OK; }
    
    // Get error string
    const char* getErrorString() const {
        switch (error) {
            case IntentError::OK: return "OK";
            case IntentError::ERR_BAD_ARGS: return "ERR_BAD_ARGS";
            case IntentError::ERR_NOT_FOUND: return "ERR_NOT_FOUND";
            case IntentError::ERR_CONFLICT: return "ERR_CONFLICT";
            case IntentError::ERR_IO: return "ERR_IO";
            case IntentError::ERR_UNSUPPORTED: return "ERR_UNSUPPORTED";
            case IntentError::ERR_INTERNAL: return "ERR_INTERNAL";
            default: return "UNKNOWN";
        }
    }
};

// Intent request structure
struct IntentRequest {
    String intent;       // Intent opcode (e.g., "sys.info", "hal.caps")
    String args[8];      // Up to 8 arguments
    int argCount;
    
    IntentRequest() : intent(""), argCount(0) {}
    
    void clear() {
        intent = "";
        argCount = 0;
        for (int i = 0; i < 8; i++) {
            args[i] = "";
        }
    }
};

// Intent API Dispatcher
class IntentAPI {
public:
    static void init();
    
    // Main dispatch function - routes intent to appropriate handler
    static IntentResponse dispatch(const IntentRequest& request);
    
    // Intent handlers (v1 opcodes)
    static IntentResponse handleSysInfo(const IntentRequest& req);
    static IntentResponse handleHalCaps(const IntentRequest& req);
    static IntentResponse handleEpList(const IntentRequest& req);
    static IntentResponse handleEpProbe(const IntentRequest& req);
    static IntentResponse handleDevList(const IntentRequest& req);
    static IntentResponse handleDevBind(const IntentRequest& req);
    static IntentResponse handleDevUnbind(const IntentRequest& req);
    static IntentResponse handleDevEnable(const IntentRequest& req);
    static IntentResponse handleDevDisable(const IntentRequest& req);
    static IntentResponse handleDevStatus(const IntentRequest& req);
    static IntentResponse handleParamGet(const IntentRequest& req);
    static IntentResponse handleParamSet(const IntentRequest& req);
    static IntentResponse handleSchemaGet(const IntentRequest& req);
    static IntentResponse handleLogTail(const IntentRequest& req);
    static IntentResponse handleLogClear(const IntentRequest& req);
    static IntentResponse handlePersistSave(const IntentRequest& req);
    static IntentResponse handlePersistLoad(const IntentRequest& req);
    static IntentResponse handleConfigExport(const IntentRequest& req);
    static IntentResponse handleConfigImport(const IntentRequest& req);
    static IntentResponse handleBusList(const IntentRequest& req);
    static IntentResponse handleBusInfo(const IntentRequest& req);
    static IntentResponse handleBusConfig(const IntentRequest& req);
    static IntentResponse handleIdentify(const IntentRequest& req);
    static IntentResponse handleDeviceRead(const IntentRequest& req);
    static IntentResponse handleDeviceStream(const IntentRequest& req);
    static IntentResponse handleFactoryReset(const IntentRequest& req);
    static IntentResponse handleConfigValidate(const IntentRequest& req);
    
private:
    static bool initialized;
};

} // namespace PocketOS

#endif // POCKETOS_INTENT_API_H
