#ifndef POCKETOS_RESOURCES_H
#define POCKETOS_RESOURCES_H

namespace PocketOS {

class Resources {
public:
    static void init();
    static void release();
    static bool allocate(const char* resourceId);
    static bool deallocate(const char* resourceId);
};

} // namespace PocketOS

#endif // POCKETOS_RESOURCES_H
