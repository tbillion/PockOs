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
    Serial.println("PocketOS CLI Commands:");
    Serial.println("  help                           - Show this help");
    Serial.println("  sys info                       - System information");
    Serial.println("  hal caps                       - Hardware capabilities");
    Serial.println("  ep list                        - List endpoints");
    Serial.println("  ep probe <endpoint>            - Probe endpoint (e.g., i2c0)");
    Serial.println("  dev list                       - List devices");
    Serial.println("  bind <driver> <endpoint>       - Bind device (e.g., bind gpio.dout gpio.dout.2)");
    Serial.println("  unbind <device_id>             - Unbind device");
    Serial.println("  param get <dev_id> <param>     - Get parameter");
    Serial.println("  param set <dev_id> <param> <val> - Set parameter");
    Serial.println("  schema <device_id>             - Get device schema");
    Serial.println("  log tail [n]                   - Show last n log lines");
    Serial.println("  log clear                      - Clear log");
    Serial.println("  persist save                   - Save configuration");
    Serial.println("  persist load                   - Load configuration");
    Serial.println();
}

} // namespace PocketOS
