#ifndef POCKETOS_ENDPOINTS_H
#define POCKETOS_ENDPOINTS_H

namespace PocketOS {

class Endpoints {
public:
    static void init();
    static void registerEndpoint(const char* name, void (*handler)());
    static void unregisterEndpoint(const char* name);
    static void process();
};

} // namespace PocketOS

#endif // POCKETOS_ENDPOINTS_H
