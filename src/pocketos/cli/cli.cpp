#include "cli.h"
#include <Arduino.h>
#include "../core/logger.h"

namespace PocketOS {

#define MAX_COMMAND_LENGTH 128
static char commandBuffer[MAX_COMMAND_LENGTH];
static int commandPos = 0;

void CLI::init() {
    commandPos = 0;
    commandBuffer[0] = '\0';
    Logger::info("CLI initialized");
    printHelp();
}

void CLI::process() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (commandPos > 0) {
                commandBuffer[commandPos] = '\0';
                
                // Trim leading/trailing spaces
                char* cmd = commandBuffer;
                while (*cmd == ' ') cmd++;
                
                // Parse and execute command
                if (strcmp(cmd, "help") == 0) {
                    printHelp();
                } else if (strcmp(cmd, "status") == 0) {
                    Logger::info("PocketOS is running");
                } else if (strcmp(cmd, "version") == 0) {
                    Serial.println("PocketOS v1.0.0");
                } else if (*cmd != '\0') {
                    Serial.print("Unknown command: ");
                    Serial.println(cmd);
                    printHelp();
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
            }
        } else if (commandPos < MAX_COMMAND_LENGTH - 1) {
            commandBuffer[commandPos++] = c;
        }
    }
}

void CLI::printHelp() {
    Serial.println("\nPocketOS CLI");
    Serial.println("Commands:");
    Serial.println("  help    - Show this help message");
    Serial.println("  status  - Show system status");
    Serial.println("  version - Show PocketOS version");
    Serial.print("> ");
}

void CLI::registerCommand(const char* name, void (*handler)(const char* args)) {
    Logger::debug("Command registered");
}

} // namespace PocketOS
