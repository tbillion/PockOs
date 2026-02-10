#ifndef POCKETOS_ILI9341_DRIVER_H
#define POCKETOS_ILI9341_DRIVER_H

#include <Arduino.h>
#include "../driver_config.h"
#include "spi_driver_base.h"
#include "register_types.h"

namespace PocketOS {

// ILI9341 320x240 TFT LCD Display Driver
// Endpoint format: spi0:cs=5,dc=16,rst=17 (dc and rst required)

class ILI9341Driver : public SPIDriverBase {
public:
    ILI9341Driver();
    ~ILI9341Driver();
    
    // Initialize from endpoint descriptor
    bool init(const String& endpoint);
    
    // Valid endpoint configurations
    static bool validEndpoints(const String& endpoint);
    
    // Identification probe - reads Display ID (0x04 command)
    static bool identifyProbe(const String& endpoint);
    
#if POCKETOS_ILI9341_ENABLE_BASIC_READ
    // Tier 0: Basic display operations
    bool begin();
    bool setPixel(uint16_t x, uint16_t y, uint16_t color);
    bool fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    bool fillScreen(uint16_t color);
    bool setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    bool pushColor(uint16_t color);
    bool pushColors(const uint16_t* colors, uint16_t len);
    uint16_t width() const { return _width; }
    uint16_t height() const { return _height; }
#endif

#if POCKETOS_ILI9341_ENABLE_ERROR_HANDLING
    // Tier 1: Rotation, scrolling, color modes
    bool setRotation(uint8_t rotation);
    bool invertDisplay(bool invert);
    bool setScrollArea(uint16_t top, uint16_t bottom);
    bool setScroll(uint16_t offset);
    bool setSleep(bool sleep);
    bool setIdleMode(bool idle);
    uint8_t getRotation() const { return _rotation; }
    uint32_t readID();
    uint8_t readStatus();
#endif

#if POCKETOS_ILI9341_ENABLE_REGISTER_ACCESS
    // Tier 2: Complete register/command access
    const RegisterDesc* registers(size_t& count) const override;
    bool writeCommand(uint8_t cmd);
    bool writeData(uint8_t data);
    bool writeData16(uint16_t data);
    bool readData(uint8_t cmd, uint8_t* buf, size_t len);
    const RegisterDesc* findRegisterByName(const String& name) const override;
#endif

private:
    bool initialized_;
    uint16_t _width;
    uint16_t _height;
    uint8_t _rotation;
    
    // Helper methods
    bool hardwareReset();
    bool sendCommand(uint8_t cmd);
    bool sendData(uint8_t data);
    bool sendData16(uint16_t data);
};

} // namespace PocketOS

#endif // POCKETOS_ILI9341_DRIVER_H
