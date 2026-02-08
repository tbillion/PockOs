#ifndef POCKETOS_DEVICES_H
#define POCKETOS_DEVICES_H

namespace PocketOS {

class Devices {
public:
    static void init();
    static void registerDevice(const char* name);
    static void unregisterDevice(const char* name);
    static void listDevices();
};

} // namespace PocketOS

#endif // POCKETOS_DEVICES_H
