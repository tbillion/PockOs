#include "cli.h"
#include <Arduino.h>
#include "../core/logger.h"
#include "../core/intent_api.h"

namespace PocketOS {

char CLI::commandBuffer[CLI_MAX_COMMAND_LENGTH];
int CLI::commandPos = 0;

void CLI::init() {
    commandPos = 0;
    commandBuffer[0] = '\0';
    Logger::info("CLI initialized");
    printBanner();
    printHelp();
}

void CLI::printBanner() {
    Serial.println();
    Serial.println("=====================================");
    Serial.println("       PocketOS v1.0");
    Serial.println("  Embedded OS for Microcontrollers");
    Serial.println("=====================================");
    Serial.println();
}

void CLI::process() {
    while (Serial.available()) {
        char c = Serial.read();
        
        if (c == '\n' || c == '\r') {
            if (commandPos > 0) {
                commandBuffer[commandPos] = '\0';
                String cmdLine = String(commandBuffer);
                cmdLine.trim();
                
                if (cmdLine.length() > 0) {
                    Serial.println(); // Echo newline
                    executeCommand(cmdLine);
                }
                
                commandPos = 0;
                commandBuffer[0] = '\0';
                Serial.print("> ");
            }
        } else if (c == '\b' || c == 127) {
            // Backspace
            if (commandPos > 0) {
                commandPos--;
                commandBuffer[commandPos] = '\0';
                Serial.print("\b \b"); // Erase character on terminal
            }
        } else if (c >= 32 && c < 127) {
            // Printable character
            if (commandPos < CLI_MAX_COMMAND_LENGTH - 1) {
                commandBuffer[commandPos++] = c;
                Serial.print(c); // Echo character
            }
        }
    }
}

void CLI::executeCommand(const String& cmdLine) {
    // Handle built-in commands first
    if (cmdLine == "help" || cmdLine == "?") {
        printHelp();
        return;
    }
    
    // Parse and dispatch via Intent API
    IntentRequest request;
    parseCommand(cmdLine, request);
    
    if (request.intent.length() > 0) {
        IntentResponse response = IntentAPI::dispatch(request);
        printResponse(response);
    } else {
        Serial.println("Unknown command. Type 'help' for available commands.");
    }
}

void CLI::parseCommand(const String& cmdLine, IntentRequest& request) {
    request.clear();
    
    // Split command line into tokens
    String tokens[CLI_MAX_ARGS + 1];
    int tokenCount = 0;
    int start = 0;
    
    for (int i = 0; i <= cmdLine.length(); i++) {
        if (i == cmdLine.length() || cmdLine[i] == ' ') {
            if (i > start && tokenCount < CLI_MAX_ARGS + 1) {
                tokens[tokenCount++] = cmdLine.substring(start, i);
            }
            start = i + 1;
        }
    }
    
    if (tokenCount == 0) return;
    
    // Map CLI commands to Intent opcodes
    String cmd = tokens[0];
    
    if (cmd == "sys" && tokenCount > 1 && tokens[1] == "info") {
        request.intent = "sys.info";
    } else if (cmd == "hal" && tokenCount > 1 && tokens[1] == "caps") {
        request.intent = "hal.caps";
    } else if (cmd == "ep" && tokenCount > 1) {
        if (tokens[1] == "list") {
            request.intent = "ep.list";
        } else if (tokens[1] == "probe" && tokenCount > 2) {
            request.intent = "ep.probe";
            request.args[0] = tokens[2];
            request.argCount = 1;
        }
    } else if (cmd == "dev" && tokenCount > 1) {
        if (tokens[1] == "list") {
            request.intent = "dev.list";
        }
    } else if (cmd == "bind" && tokenCount > 2) {
        // bind <driver_id> <endpoint>
        request.intent = "dev.bind";
        request.args[0] = tokens[1];
        request.args[1] = tokens[2];
        request.argCount = 2;
    } else if (cmd == "unbind" && tokenCount > 1) {
        request.intent = "dev.unbind";
        request.args[0] = tokens[1];
        request.argCount = 1;
    } else if (cmd == "param" && tokenCount > 2) {
        if (tokens[1] == "get" && tokenCount > 3) {
            request.intent = "param.get";
            request.args[0] = tokens[2];
            request.args[1] = tokens[3];
            request.argCount = 2;
        } else if (tokens[1] == "set" && tokenCount > 4) {
            request.intent = "param.set";
            request.args[0] = tokens[2];
            request.args[1] = tokens[3];
            request.args[2] = tokens[4];
            request.argCount = 3;
        }
    } else if (cmd == "schema" && tokenCount > 1) {
        request.intent = "schema.get";
        request.args[0] = tokens[1];
        request.argCount = 1;
    } else if (cmd == "status" && tokenCount > 1) {
        request.intent = "dev.status";
        request.args[0] = tokens[1];
        request.argCount = 1;
    } else if (cmd == "log") {
        if (tokenCount > 1 && tokens[1] == "tail") {
            request.intent = "log.tail";
            if (tokenCount > 2) {
                request.args[0] = tokens[2];
                request.argCount = 1;
            }
        } else if (tokenCount > 1 && tokens[1] == "clear") {
            request.intent = "log.clear";
        }
    } else if (cmd == "persist") {
        if (tokenCount > 1 && tokens[1] == "save") {
            request.intent = "persist.save";
        } else if (tokenCount > 1 && tokens[1] == "load") {
            request.intent = "persist.load";
        }
    } else if (cmd == "config") {
        if (tokenCount > 1 && tokens[1] == "export") {
            request.intent = "config.export";
        } else if (tokenCount > 1 && tokens[1] == "import") {
            request.intent = "config.import";
            // Would need to handle multi-line config data
        }
    } else if (cmd == "bus") {
        if (tokenCount > 1) {
            if (tokens[1] == "list") {
                request.intent = "bus.list";
            } else if (tokens[1] == "info" && tokenCount > 2) {
                request.intent = "bus.info";
                request.args[0] = tokens[2];
                request.argCount = 1;
            } else if (tokens[1] == "config" && tokenCount > 2) {
                request.intent = "bus.config";
                request.args[0] = tokens[2];
                // Pass all remaining tokens as args
                for (int i = 3; i < tokenCount && request.argCount < MAX_INTENT_ARGS - 1; i++) {
                    request.args[request.argCount++] = tokens[i];
                }
            }
        }
    } else if (cmd == "identify" && tokenCount > 1) {
        request.intent = "identify";
        request.args[0] = tokens[1];
        request.argCount = 1;
    } else if (cmd == "read" && tokenCount > 1) {
        request.intent = "dev.read";
        request.args[0] = tokens[1];
        request.argCount = 1;
    } else if (cmd == "stream" && tokenCount > 3) {
        request.intent = "dev.stream";
        request.args[0] = tokens[1];  // device_id
        request.args[1] = tokens[2];  // interval
        request.args[2] = tokens[3];  // count
        request.argCount = 3;
    }
}

void CLI::printResponse(const IntentResponse& response) {
    if (response.isOk()) {
        if (response.data.length() > 0) {
            Serial.print(response.data);
        } else {
            Serial.println("OK");
        }
    } else {
        Serial.print("Error: ");
        Serial.print(response.getErrorString());
        if (response.message.length() > 0) {
            Serial.print(" - ");
            Serial.print(response.message);
        }
        Serial.println();
    }
}

void CLI::printHelp() {
    Serial.println("PocketOS Device Manager CLI Commands:");
    Serial.println();
    Serial.println("System & Hardware:");
    Serial.println("  help                           - Show this help");
    Serial.println("  sys info                       - System information");
    Serial.println("  hal caps                       - Hardware capabilities");
    Serial.println();
    Serial.println("Bus Management:");
    Serial.println("  bus list                       - List available buses");
    Serial.println("  bus info <bus>                 - Bus information (e.g., bus info i2c0)");
    Serial.println("  bus config <bus> [params]      - Configure bus (e.g., bus config i2c0 sda=21 scl=22 speed_hz=400000)");
    Serial.println();
    Serial.println("Endpoints:");
    Serial.println("  ep list                        - List endpoints");
    Serial.println("  ep probe <endpoint>            - Probe endpoint (e.g., ep probe i2c0)");
    Serial.println();
    Serial.println("Device Identification:");
    Serial.println("  identify <endpoint>            - Identify device at endpoint (e.g., identify i2c0:0x76)");
    Serial.println();
    Serial.println("Device Management:");
    Serial.println("  dev list                       - List devices");
    Serial.println("  bind <driver> <endpoint>       - Bind device (e.g., bind bme280 i2c0:0x76)");
    Serial.println("  unbind <device_id>             - Unbind device");
    Serial.println("  status <device_id>             - Device status and health");
    Serial.println();
    Serial.println("Device Operations:");
    Serial.println("  read <device_id>               - Read current sensor data");
    Serial.println("  stream <device_id> <interval_ms> <count> - Stream sensor data");
    Serial.println();
    Serial.println("Device Configuration:");
    Serial.println("  schema <device_id>             - Show device schema");
    Serial.println("  param get <dev_id> <param>     - Get parameter");
    Serial.println("  param set <dev_id> <param> <val> - Set parameter");
    Serial.println();
    Serial.println("Persistence & Config:");
    Serial.println("  persist save                   - Save configuration");
    Serial.println("  persist load                   - Load configuration");
    Serial.println("  config export                  - Export configuration");
    Serial.println("  config import <data>           - Import configuration (future)");
    Serial.println();
    Serial.println("Logging:");
    Serial.println("  log tail [n]                   - Show last n log lines");
    Serial.println("  log clear                      - Clear log");
    Serial.println();
}

} // namespace PocketOS
