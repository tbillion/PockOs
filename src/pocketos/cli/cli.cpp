#include "cli.h"
#include <Arduino.h>
#include "../core/logger.h"

namespace PocketOS {

static String commandBuffer = "";

void CLI::init() {
    Logger::info("CLI initialized");
    printHelp();
}

void CLI::process() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (commandBuffer.length() > 0) {
                commandBuffer.trim();
                
                // Parse and execute command
                if (commandBuffer == "help") {
                    printHelp();
                } else if (commandBuffer == "status") {
                    Logger::info("PocketOS is running");
                } else if (commandBuffer == "version") {
                    Serial.println("PocketOS v1.0.0");
                } else {
                    Serial.print("Unknown command: ");
                    Serial.println(commandBuffer);
                    printHelp();
                }
                
                commandBuffer = "";
                Serial.print("> ");
            }
        } else if (c == '\b' || c == 127) {
            // Backspace
            if (commandBuffer.length() > 0) {
                commandBuffer.remove(commandBuffer.length() - 1);
            }
        } else {
            commandBuffer += c;
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
