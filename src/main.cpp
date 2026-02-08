#include <Arduino.h>
#include "pocketos/core/logger.h"
#include "pocketos/core/hal.h"
#include "pocketos/cli/cli.h"

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    PocketOS::Logger::init();
    PocketOS::Logger::info("PocketOS Starting...");
    
    PocketOS::HAL::init();
    PocketOS::CLI::init();
    
    PocketOS::Logger::info("PocketOS Ready");
}

void loop() {
    PocketOS::CLI::process();
    delay(10);
}
