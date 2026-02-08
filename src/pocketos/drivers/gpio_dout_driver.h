#ifndef POCKETOS_GPIO_DOUT_DRIVER_H
#define POCKETOS_GPIO_DOUT_DRIVER_H

#include "../core/device_registry.h"
#include "../core/capability_schema.h"
#include "../core/resource_manager.h"
#include "../core/hal.h"

namespace PocketOS {

class GPIODoutDriver : public IDriver {
public:
    GPIODoutDriver(const String& endpoint);
    virtual ~GPIODoutDriver();
    
    virtual bool init() override;
    virtual bool setParam(const String& name, const String& value) override;
    virtual String getParam(const String& name) override;
    virtual CapabilitySchema getSchema() override;
    virtual void update() override;
    
private:
    int pin;
    bool state;
    String endpoint;
};

} // namespace PocketOS

#endif // POCKETOS_GPIO_DOUT_DRIVER_H
