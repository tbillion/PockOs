#include "logger.h"
#include <Arduino.h>

namespace PocketOS {

void Logger::init() {
    Serial.println("[INFO] Logger initialized");
}

void Logger::info(const char* message) {
    Serial.print("[INFO] ");
    Serial.println(message);
}

void Logger::warning(const char* message) {
    Serial.print("[WARN] ");
    Serial.println(message);
}

void Logger::error(const char* message) {
    Serial.print("[ERROR] ");
    Serial.println(message);
}

void Logger::debug(const char* message) {
    Serial.print("[DEBUG] ");
    Serial.println(message);
}

} // namespace PocketOS
