#ifndef POCKETOS_CLI_H
#define POCKETOS_CLI_H

namespace PocketOS {

class CLI {
public:
    static void init();
    static void process();
    static void printHelp();
    static void registerCommand(const char* name, void (*handler)(const char* args));
};

} // namespace PocketOS

#endif // POCKETOS_CLI_H
