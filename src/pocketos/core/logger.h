#ifndef POCKETOS_LOGGER_H
#define POCKETOS_LOGGER_H

#include <Arduino.h>

namespace PocketOS {

#define LOG_BUFFER_LINES 128
#define LOG_LINE_LENGTH 96

enum class LogLevel {
    INFO,
    WARN,
    ERROR
};

class Logger {
public:
    static void init();
    static void info(const char* message);
    static void warning(const char* message);
    static void error(const char* message);
    static void debug(const char* message);
    
    // Ring buffer functions
    static String tail(int lines);
    static void clear();
    
private:
    static char logBuffer[LOG_BUFFER_LINES][LOG_LINE_LENGTH];
    static int logHead;
    static int logCount;
    static bool initialized;
    
    static void addToBuffer(LogLevel level, const char* message);
    static const char* levelToString(LogLevel level);
};

} // namespace PocketOS

#endif // POCKETOS_LOGGER_H
