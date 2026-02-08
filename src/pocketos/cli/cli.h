#ifndef POCKETOS_CLI_H
#define POCKETOS_CLI_H

#include <Arduino.h>
#include "../core/intent_api.h"

namespace PocketOS {

#define CLI_MAX_COMMAND_LENGTH 128
#define CLI_MAX_ARGS 8

class CLI {
public:
    static void init();
    static void process();
    static void printBanner();
    static void printHelp();
    
private:
    static char commandBuffer[CLI_MAX_COMMAND_LENGTH];
    static int commandPos;
    
    static void executeCommand(const String& cmdLine);
    static void parseCommand(const String& cmdLine, IntentRequest& request);
    static void printResponse(const IntentResponse& response);
};

} // namespace PocketOS

#endif // POCKETOS_CLI_H
