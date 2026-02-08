#ifndef POCKETOS_SCHEMA_H
#define POCKETOS_SCHEMA_H

namespace PocketOS {

class Schema {
public:
    static void init();
    static bool validate(const char* schema, const char* data);
    static void registerSchema(const char* name, const char* schema);
};

} // namespace PocketOS

#endif // POCKETOS_SCHEMA_H
