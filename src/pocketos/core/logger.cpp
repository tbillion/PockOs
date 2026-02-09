#include "logger.h"
#include <Arduino.h>

namespace PocketOS {

char Logger::logBuffer[LOG_BUFFER_LINES][LOG_LINE_LENGTH];
int Logger::logHead = 0;
int Logger::logCount = 0;
bool Logger::initialized = false;

void Logger::init() {
    if (!initialized) {
        logHead = 0;
        logCount = 0;
        for (int i = 0; i < LOG_BUFFER_LINES; i++) {
            logBuffer[i][0] = '\0';
        }
        Serial.println("[INFO] Logger initialized");
        initialized = true;
    }
}

void Logger::info(const char* message) {
    Serial.print("[INFO] ");
    Serial.println(message);
    addToBuffer(LogLevel::INFO, message);
}

void Logger::warning(const char* message) {
    Serial.print("[WARN] ");
    Serial.println(message);
    addToBuffer(LogLevel::WARN, message);
}

void Logger::error(const char* message) {
    Serial.print("[ERROR] ");
    Serial.println(message);
    addToBuffer(LogLevel::ERROR, message);
}

void Logger::debug(const char* message) {
    Serial.print("[DEBUG] ");
    Serial.println(message);
    addToBuffer(LogLevel::INFO, message);
}

String Logger::tail(int lines) {
    if (lines > LOG_BUFFER_LINES) {
        lines = LOG_BUFFER_LINES;
    }
    if (lines > logCount) {
        lines = logCount;
    }
    
    String result = "";
    int startIdx = (logHead - lines + LOG_BUFFER_LINES) % LOG_BUFFER_LINES;
    
    for (int i = 0; i < lines; i++) {
        int idx = (startIdx + i) % LOG_BUFFER_LINES;
        if (logBuffer[idx][0] != '\0') {
            result += logBuffer[idx];
            result += "\n";
        }
    }
    
    return result;
}

void Logger::clear() {
    logHead = 0;
    logCount = 0;
    for (int i = 0; i < LOG_BUFFER_LINES; i++) {
        logBuffer[i][0] = '\0';
    }
    Serial.println("Log cleared");
}

void Logger::addToBuffer(LogLevel level, const char* message) {
    // Format: [LEVEL] message
    snprintf(logBuffer[logHead], LOG_LINE_LENGTH, "[%s] %s", 
             levelToString(level), message);
    
    logHead = (logHead + 1) % LOG_BUFFER_LINES;
    if (logCount < LOG_BUFFER_LINES) {
        logCount++;
    }
}

const char* Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARN: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

} // namespace PocketOS
