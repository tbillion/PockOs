#ifndef POCKETOS_LOGGER_H
#define POCKETOS_LOGGER_H

namespace PocketOS {

class Logger {
public:
    static void init();
    static void info(const char* message);
    static void warning(const char* message);
    static void error(const char* message);
    static void debug(const char* message);
};

} // namespace PocketOS

#endif // POCKETOS_LOGGER_H
