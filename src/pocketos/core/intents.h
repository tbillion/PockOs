#ifndef POCKETOS_INTENTS_H
#define POCKETOS_INTENTS_H

namespace PocketOS {

class Intents {
public:
    static void init();
    static void sendIntent(const char* intent);
    static void registerHandler(const char* intent, void (*handler)());
    static void process();
};

} // namespace PocketOS

#endif // POCKETOS_INTENTS_H
