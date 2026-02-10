#include "st7789_driver.h"
#include "../core/logger.h"
#include "../core/resource_manager.h"
#include <SPI.h>

namespace PocketOS {

// ST7789 Command Definitions (0x00-0xFF)
#define ST7789_NOP         0x00  // No Operation
#define ST7789_SWRESET     0x01  // Software Reset
#define ST7789_RDDID       0x04  // Read Display ID
#define ST7789_RDDST       0x09  // Read Display Status
#define ST7789_RDDPM       0x0A  // Read Display Power Mode
#define ST7789_RDDMADCTL   0x0B  // Read Display MADCTL
#define ST7789_RDDCOLMOD   0x0C  // Read Display Pixel Format
#define ST7789_RDDIM       0x0D  // Read Display Image Mode
#define ST7789_RDDSM       0x0E  // Read Display Signal Mode
#define ST7789_RDDSDR      0x0F  // Read Display Self-Diagnostic Result
#define ST7789_SLPIN       0x10  // Sleep In
#define ST7789_SLPOUT      0x11  // Sleep Out
#define ST7789_PTLON       0x12  // Partial Mode On
#define ST7789_NORON       0x13  // Normal Display Mode On
#define ST7789_INVOFF      0x20  // Display Inversion Off
#define ST7789_INVON       0x21  // Display Inversion On
#define ST7789_GAMSET      0x26  // Gamma Set
#define ST7789_DISPOFF     0x28  // Display Off
#define ST7789_DISPON      0x29  // Display On
#define ST7789_CASET       0x2A  // Column Address Set
#define ST7789_RASET       0x2B  // Row Address Set
#define ST7789_RAMWR       0x2C  // Memory Write
#define ST7789_RAMRD       0x2E  // Memory Read
#define ST7789_PTLAR       0x30  // Partial Area
#define ST7789_VSCRDEF     0x33  // Vertical Scrolling Definition
#define ST7789_TEOFF       0x34  // Tearing Effect Line OFF
#define ST7789_TEON        0x35  // Tearing Effect Line ON
#define ST7789_MADCTL      0x36  // Memory Data Access Control
#define ST7789_VSCSAD      0x37  // Vertical Scroll Start Address
#define ST7789_IDMOFF      0x38  // Idle Mode Off
#define ST7789_IDMON       0x39  // Idle Mode On
#define ST7789_COLMOD      0x3A  // Color Mode
#define ST7789_RAMWRC      0x3C  // Memory Write Continue
#define ST7789_RAMRDC      0x3E  // Memory Read Continue
#define ST7789_TESCAN      0x44  // Set Tear Scanline
#define ST7789_RDTESCAN    0x45  // Get Scanline
#define ST7789_WRDISBV     0x51  // Write Display Brightness
#define ST7789_RDDISBV     0x52  // Read Display Brightness
#define ST7789_WRCTRLD     0x53  // Write CTRL Display
#define ST7789_RDCTRLD     0x54  // Read CTRL Display
#define ST7789_WRCACE      0x55  // Write Content Adaptive Brightness Control and Color Enhancement
#define ST7789_RDCABC      0x56  // Read Content Adaptive Brightness Control
#define ST7789_WRCABCMB    0x5E  // Write CABC Minimum Brightness
#define ST7789_RDCABCMB    0x5F  // Read CABC Minimum Brightness
#define ST7789_RDABCSDR    0x68  // Read Automatic Brightness Control Self-Diagnostic Result
#define ST7789_RDID1       0xDA  // Read ID1
#define ST7789_RDID2       0xDB  // Read ID2
#define ST7789_RDID3       0xDC  // Read ID3
#define ST7789_RAMCTRL     0xB0  // RAM Control
#define ST7789_RGBCTRL     0xB1  // RGB Control
#define ST7789_PORCTRL     0xB2  // Porch Control
#define ST7789_FRCTRL1     0xB3  // Frame Rate Control 1 (In partial mode/ idle colors)
#define ST7789_PARCTRL     0xB5  // Partial Control
#define ST7789_GCTRL       0xB7  // Gate Control
#define ST7789_GTADJ       0xB8  // Gate On Timing Adjustment
#define ST7789_DGMEN       0xBA  // Digital Gamma Enable
#define ST7789_VCOMS       0xBB  // VCOM Setting
#define ST7789_POWSAVE     0xBC  // Power Saving Mode
#define ST7789_DLPOFFSAVE  0xBD  // Display off power save
#define ST7789_LCMCTRL     0xC0  // LCM Control
#define ST7789_IDSET       0xC1  // ID Code Setting
#define ST7789_VDVVRHEN    0xC2  // VDV and VRH Command Enable
#define ST7789_VRHS        0xC3  // VRH Set
#define ST7789_VDVS        0xC4  // VDV Set
#define ST7789_VCMOFSET    0xC5  // VCOM Offset Set
#define ST7789_FRCTRL2     0xC6  // Frame Rate Control in Normal Mode
#define ST7789_CABCCTRL    0xC7  // CABC Control
#define ST7789_REGSEL1     0xC8  // Register Value Selection 1
#define ST7789_REGSEL2     0xCA  // Register Value Selection 2
#define ST7789_PWMFRSEL    0xCC  // PWM Frequency Selection
#define ST7789_PWCTRL1     0xD0  // Power Control 1
#define ST7789_VAPVANEN    0xD2  // Enable VAP/VAN signal output
#define ST7789_CMD2EN      0xDF  // Command 2 Enable
#define ST7789_PVGAMCTRL   0xE0  // Positive Voltage Gamma Control
#define ST7789_NVGAMCTRL   0xE1  // Negative Voltage Gamma Control
#define ST7789_DGMLUTR     0xE2  // Digital Gamma Look-up Table for Red
#define ST7789_DGMLUTB     0xE3  // Digital Gamma Look-up Table for Blue
#define ST7789_GATECTRL    0xE4  // Gate Control
#define ST7789_SPI2EN      0xE7  // SPI2 Enable
#define ST7789_PWCTRL2     0xE8  // Power Control 2
#define ST7789_EQCTRL      0xE9  // Equalize time control
#define ST7789_PROMCTRL    0xEC  // Program Mode Control
#define ST7789_PROMEN      0xFA  // Program Mode Enable
#define ST7789_NVMSET      0xFC  // NVM Setting
#define ST7789_PROMACT     0xFE  // Program action

// Color definitions
#define ST7789_BLACK       0x0000
#define ST7789_BLUE        0x001F
#define ST7789_RED         0xF800
#define ST7789_GREEN       0x07E0
#define ST7789_CYAN        0x07FF
#define ST7789_MAGENTA     0xF81F
#define ST7789_YELLOW      0xFFE0
#define ST7789_WHITE       0xFFFF

#if POCKETOS_ST7789_ENABLE_REGISTER_ACCESS
// Complete ST7789 Register Map (Commands 0x00-0xFF)
static const RegisterDesc ST7789_REGISTERS[] = {
    RegisterDesc(0x00, "NOP", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x01, "SWRESET", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x04, "RDDID", 4, RegisterAccess::RO, 0x00),
    RegisterDesc(0x09, "RDDST", 5, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0A, "RDDPM", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0B, "RDDMADCTL", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0C, "RDDCOLMOD", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0D, "RDDIM", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0E, "RDDSM", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0F, "RDDSDR", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x10, "SLPIN", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x11, "SLPOUT", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x12, "PTLON", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x13, "NORON", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x20, "INVOFF", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x21, "INVON", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x26, "GAMSET", 2, RegisterAccess::WO, 0x01),
    RegisterDesc(0x28, "DISPOFF", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x29, "DISPON", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x2A, "CASET", 5, RegisterAccess::WO, 0x00),
    RegisterDesc(0x2B, "RASET", 5, RegisterAccess::WO, 0x00),
    RegisterDesc(0x2C, "RAMWR", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x2E, "RAMRD", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x30, "PTLAR", 5, RegisterAccess::WO, 0x00),
    RegisterDesc(0x33, "VSCRDEF", 7, RegisterAccess::WO, 0x00),
    RegisterDesc(0x34, "TEOFF", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x35, "TEON", 2, RegisterAccess::WO, 0x00),
    RegisterDesc(0x36, "MADCTL", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0x37, "VSCSAD", 3, RegisterAccess::WO, 0x00),
    RegisterDesc(0x38, "IDMOFF", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x39, "IDMON", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x3A, "COLMOD", 2, RegisterAccess::RW, 0x05),
    RegisterDesc(0x3C, "RAMWRC", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x3E, "RAMRDC", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x44, "TESCAN", 3, RegisterAccess::WO, 0x00),
    RegisterDesc(0x45, "RDTESCAN", 3, RegisterAccess::RO, 0x00),
    RegisterDesc(0x51, "WRDISBV", 2, RegisterAccess::WO, 0x00),
    RegisterDesc(0x52, "RDDISBV", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x53, "WRCTRLD", 2, RegisterAccess::WO, 0x00),
    RegisterDesc(0x54, "RDCTRLD", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x55, "WRCACE", 2, RegisterAccess::WO, 0x00),
    RegisterDesc(0x56, "RDCABC", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x5E, "WRCABCMB", 2, RegisterAccess::WO, 0x00),
    RegisterDesc(0x5F, "RDCABCMB", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x68, "RDABCSDR", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0xB0, "RAMCTRL", 3, RegisterAccess::RW, 0x00),
    RegisterDesc(0xB1, "RGBCTRL", 4, RegisterAccess::RW, 0x00),
    RegisterDesc(0xB2, "PORCTRL", 6, RegisterAccess::RW, 0x00),
    RegisterDesc(0xB3, "FRCTRL1", 7, RegisterAccess::RW, 0x00),
    RegisterDesc(0xB5, "PARCTRL", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xB7, "GCTRL", 2, RegisterAccess::RW, 0x35),
    RegisterDesc(0xB8, "GTADJ", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xBA, "DGMEN", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xBB, "VCOMS", 2, RegisterAccess::RW, 0x20),
    RegisterDesc(0xBC, "POWSAVE", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xBD, "DLPOFFSAVE", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xC0, "LCMCTRL", 2, RegisterAccess::RW, 0x2C),
    RegisterDesc(0xC1, "IDSET", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xC2, "VDVVRHEN", 2, RegisterAccess::RW, 0x01),
    RegisterDesc(0xC3, "VRHS", 2, RegisterAccess::RW, 0x0B),
    RegisterDesc(0xC4, "VDVS", 2, RegisterAccess::RW, 0x20),
    RegisterDesc(0xC5, "VCMOFSET", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xC6, "FRCTRL2", 2, RegisterAccess::RW, 0x0F),
    RegisterDesc(0xC7, "CABCCTRL", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xC8, "REGSEL1", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xCA, "REGSEL2", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xCC, "PWMFRSEL", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xD0, "PWCTRL1", 3, RegisterAccess::RW, 0xA4),
    RegisterDesc(0xD2, "VAPVANEN", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xDA, "RDID1", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0xDB, "RDID2", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0xDC, "RDID3", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0xDF, "CMD2EN", 4, RegisterAccess::WO, 0x00),
    RegisterDesc(0xE0, "PVGAMCTRL", 14, RegisterAccess::WO, 0x00),
    RegisterDesc(0xE1, "NVGAMCTRL", 14, RegisterAccess::WO, 0x00),
    RegisterDesc(0xE2, "DGMLUTR", 65, RegisterAccess::WO, 0x00),
    RegisterDesc(0xE3, "DGMLUTB", 65, RegisterAccess::WO, 0x00),
    RegisterDesc(0xE4, "GATECTRL", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xE7, "SPI2EN", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xE8, "PWCTRL2", 9, RegisterAccess::RW, 0x00),
    RegisterDesc(0xE9, "EQCTRL", 7, RegisterAccess::RW, 0x00),
    RegisterDesc(0xEC, "PROMCTRL", 8, RegisterAccess::RW, 0x00),
    RegisterDesc(0xFA, "PROMEN", 2, RegisterAccess::WO, 0x00),
    RegisterDesc(0xFC, "NVMSET", 3, RegisterAccess::RW, 0x00),
    RegisterDesc(0xFE, "PROMACT", 2, RegisterAccess::WO, 0x00),
};

#define ST7789_REGISTER_COUNT (sizeof(ST7789_REGISTERS) / sizeof(RegisterDesc))
#endif

ST7789Driver::ST7789Driver() 
    : initialized_(false), _width(240), _height(240), _rotation(0) {
}

ST7789Driver::~ST7789Driver() {
    deinit();
}

bool ST7789Driver::init(const String& endpoint) {
    if (!initFromEndpoint(endpoint)) {
        Logger::error("ST7789: Failed to init SPI base");
        return false;
    }
    
    // Check for required DC pin
    if (getPinConfig().dc < 0) {
        Logger::error("ST7789: DC pin required");
        deinit();
        return false;
    }
    
    // Check for required RST pin
    if (getPinConfig().rst < 0) {
        Logger::error("ST7789: RST pin required");
        deinit();
        return false;
    }
    
    initialized_ = true;
    return true;
}

bool ST7789Driver::validEndpoints(const String& endpoint) {
    // Must have spi0 prefix, cs, dc, and rst pins
    if (!endpoint.startsWith("spi0:")) return false;
    if (endpoint.indexOf("cs=") < 0) return false;
    if (endpoint.indexOf("dc=") < 0) return false;
    if (endpoint.indexOf("rst=") < 0) return false;
    return true;
}

bool ST7789Driver::identifyProbe(const String& endpoint) {
    ST7789Driver driver;
    if (!driver.init(endpoint)) {
        return false;
    }
    
    if (!driver.hardwareReset()) {
        return false;
    }
    
    delay(120);
    
    // Read Display ID (0x04 command)
    uint8_t id[4] = {0};
    if (!driver.readData(ST7789_RDDID, id, 4)) {
        return false;
    }
    
    // Check if at least one byte is non-zero
    return (id[0] != 0 || id[1] != 0 || id[2] != 0 || id[3] != 0);
}

#if POCKETOS_ST7789_ENABLE_BASIC_READ
bool ST7789Driver::begin() {
    if (!initialized_) return false;
    
    // Hardware reset
    if (!hardwareReset()) return false;
    delay(150);
    
    // Software reset
    sendCommand(ST7789_SWRESET);
    delay(150);
    
    // Sleep out
    sendCommand(ST7789_SLPOUT);
    delay(120);
    
    // Memory data access control
    sendCommand(ST7789_MADCTL);
    sendData(0x00);
    
    // Color mode - 16-bit color
    sendCommand(ST7789_COLMOD);
    sendData(0x55);
    
    // Porch control
    sendCommand(ST7789_PORCTRL);
    sendData(0x0C);
    sendData(0x0C);
    sendData(0x00);
    sendData(0x33);
    sendData(0x33);
    
    // Gate control
    sendCommand(ST7789_GCTRL);
    sendData(0x35);
    
    // VCOM setting
    sendCommand(ST7789_VCOMS);
    sendData(0x28);
    
    // LCM control
    sendCommand(ST7789_LCMCTRL);
    sendData(0x2C);
    
    // VDV and VRH command enable
    sendCommand(ST7789_VDVVRHEN);
    sendData(0x01);
    sendData(0xFF);
    
    // VRH set
    sendCommand(ST7789_VRHS);
    sendData(0x10);
    
    // VDV set
    sendCommand(ST7789_VDVS);
    sendData(0x20);
    
    // Frame rate control
    sendCommand(ST7789_FRCTRL2);
    sendData(0x0F);
    
    // Power control 1
    sendCommand(ST7789_PWCTRL1);
    sendData(0xA4);
    sendData(0xA1);
    
    // Positive voltage gamma control
    sendCommand(ST7789_PVGAMCTRL);
    sendData(0xD0);
    sendData(0x00);
    sendData(0x02);
    sendData(0x07);
    sendData(0x0A);
    sendData(0x28);
    sendData(0x32);
    sendData(0x44);
    sendData(0x42);
    sendData(0x06);
    sendData(0x0E);
    sendData(0x12);
    sendData(0x14);
    sendData(0x17);
    
    // Negative voltage gamma control
    sendCommand(ST7789_NVGAMCTRL);
    sendData(0xD0);
    sendData(0x00);
    sendData(0x02);
    sendData(0x07);
    sendData(0x0A);
    sendData(0x28);
    sendData(0x31);
    sendData(0x54);
    sendData(0x47);
    sendData(0x0E);
    sendData(0x1C);
    sendData(0x17);
    sendData(0x1B);
    sendData(0x1E);
    
    // Display inversion on
    sendCommand(ST7789_INVON);
    
    // Normal display mode on
    sendCommand(ST7789_NORON);
    delay(10);
    
    // Display on
    sendCommand(ST7789_DISPON);
    delay(100);
    
    return true;
}

bool ST7789Driver::setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    if (!initialized_) return false;
    
    sendCommand(ST7789_CASET);
    sendData16(x0);
    sendData16(x1);
    
    sendCommand(ST7789_RASET);
    sendData16(y0);
    sendData16(y1);
    
    sendCommand(ST7789_RAMWR);
    return true;
}

bool ST7789Driver::setPixel(uint16_t x, uint16_t y, uint16_t color) {
    if (!initialized_) return false;
    if (x >= _width || y >= _height) return false;
    
    setWindow(x, y, x, y);
    sendData16(color);
    return true;
}

bool ST7789Driver::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (!initialized_) return false;
    if (x >= _width || y >= _height) return false;
    if (x + w > _width) w = _width - x;
    if (y + h > _height) h = _height - y;
    
    setWindow(x, y, x + w - 1, y + h - 1);
    
    uint32_t pixels = (uint32_t)w * h;
    setDCData();
    setCS(true);
    for (uint32_t i = 0; i < pixels; i++) {
        SPI.write16(color);
    }
    setCS(false);
    
    return true;
}

bool ST7789Driver::fillScreen(uint16_t color) {
    return fillRect(0, 0, _width, _height, color);
}

bool ST7789Driver::pushColor(uint16_t color) {
    if (!initialized_) return false;
    sendData16(color);
    return true;
}

bool ST7789Driver::pushColors(const uint16_t* colors, uint16_t len) {
    if (!initialized_) return false;
    
    setDCData();
    setCS(true);
    for (uint16_t i = 0; i < len; i++) {
        SPI.write16(colors[i]);
    }
    setCS(false);
    
    return true;
}
#endif

#if POCKETOS_ST7789_ENABLE_ERROR_HANDLING
bool ST7789Driver::setRotation(uint8_t rotation) {
    if (!initialized_) return false;
    
    _rotation = rotation % 4;
    uint8_t madctl = 0;
    
    switch (_rotation) {
        case 0:
            madctl = 0x00;
            _width = 240;
            _height = 240;
            break;
        case 1:
            madctl = 0x60;
            _width = 240;
            _height = 240;
            break;
        case 2:
            madctl = 0xC0;
            _width = 240;
            _height = 240;
            break;
        case 3:
            madctl = 0xA0;
            _width = 240;
            _height = 240;
            break;
    }
    
    sendCommand(ST7789_MADCTL);
    sendData(madctl);
    return true;
}

bool ST7789Driver::invertDisplay(bool invert) {
    if (!initialized_) return false;
    sendCommand(invert ? ST7789_INVON : ST7789_INVOFF);
    return true;
}

bool ST7789Driver::setScrollArea(uint16_t top, uint16_t bottom) {
    if (!initialized_) return false;
    
    sendCommand(ST7789_VSCRDEF);
    sendData16(top);
    sendData16(_height - top - bottom);
    sendData16(bottom);
    return true;
}

bool ST7789Driver::setScroll(uint16_t offset) {
    if (!initialized_) return false;
    
    sendCommand(ST7789_VSCSAD);
    sendData16(offset);
    return true;
}

bool ST7789Driver::setSleep(bool sleep) {
    if (!initialized_) return false;
    sendCommand(sleep ? ST7789_SLPIN : ST7789_SLPOUT);
    delay(120);
    return true;
}

bool ST7789Driver::setIdleMode(bool idle) {
    if (!initialized_) return false;
    sendCommand(idle ? ST7789_IDMON : ST7789_IDMOFF);
    return true;
}

uint32_t ST7789Driver::readID() {
    uint8_t id[4] = {0};
    readData(ST7789_RDDID, id, 4);
    return ((uint32_t)id[0] << 24) | ((uint32_t)id[1] << 16) | ((uint32_t)id[2] << 8) | id[3];
}

uint8_t ST7789Driver::readStatus() {
    uint8_t status[5] = {0};
    readData(ST7789_RDDST, status, 5);
    return status[0];
}
#endif

#if POCKETOS_ST7789_ENABLE_REGISTER_ACCESS
const RegisterDesc* ST7789Driver::registers(size_t& count) const {
    count = ST7789_REGISTER_COUNT;
    return ST7789_REGISTERS;
}

bool ST7789Driver::writeCommand(uint8_t cmd) {
    return sendCommand(cmd);
}

bool ST7789Driver::writeData(uint8_t data) {
    return sendData(data);
}

bool ST7789Driver::writeData16(uint16_t data) {
    return sendData16(data);
}

bool ST7789Driver::readData(uint8_t cmd, uint8_t* buf, size_t len) {
    if (!initialized_) return false;
    
    setDCCommand();
    setCS(true);
    SPI.transfer(cmd);
    setDCData();
    
    // Dummy read
    SPI.transfer(0x00);
    
    for (size_t i = 0; i < len; i++) {
        buf[i] = SPI.transfer(0x00);
    }
    
    setCS(false);
    return true;
}

const RegisterDesc* ST7789Driver::findRegisterByName(const String& name) const {
    size_t count;
    const RegisterDesc* regs = registers(count);
    return RegisterUtils::findByName(regs, count, name);
}
#endif

bool ST7789Driver::hardwareReset() {
    if (getPinConfig().rst < 0) return false;
    
    setRST(true);
    delay(10);
    setRST(false);
    delay(20);
    setRST(true);
    delay(150);
    
    return true;
}

bool ST7789Driver::sendCommand(uint8_t cmd) {
    if (!initialized_) return false;
    
    setDCCommand();
    setCS(true);
    SPI.transfer(cmd);
    setCS(false);
    
    return true;
}

bool ST7789Driver::sendData(uint8_t data) {
    if (!initialized_) return false;
    
    setDCData();
    setCS(true);
    SPI.transfer(data);
    setCS(false);
    
    return true;
}

bool ST7789Driver::sendData16(uint16_t data) {
    if (!initialized_) return false;
    
    setDCData();
    setCS(true);
    SPI.write16(data);
    setCS(false);
    
    return true;
}

} // namespace PocketOS
