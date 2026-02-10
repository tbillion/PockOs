#include "st7735_driver.h"
#include "../core/logger.h"
#include "../core/resource_manager.h"
#include <SPI.h>

namespace PocketOS {

// ST7735 Command Definitions (0x00-0xFF)
#define ST7735_NOP         0x00  // No Operation
#define ST7735_SWRESET     0x01  // Software Reset
#define ST7735_RDDID       0x04  // Read Display ID
#define ST7735_RDDST       0x09  // Read Display Status
#define ST7735_RDDPM       0x0A  // Read Display Power Mode
#define ST7735_RDDMADCTL   0x0B  // Read Display MADCTL
#define ST7735_RDDCOLMOD   0x0C  // Read Display Pixel Format
#define ST7735_RDDIM       0x0D  // Read Display Image Mode
#define ST7735_RDDSM       0x0E  // Read Display Signal Mode
#define ST7735_SLPIN       0x10  // Sleep In
#define ST7735_SLPOUT      0x11  // Sleep Out
#define ST7735_PTLON       0x12  // Partial Mode On
#define ST7735_NORON       0x13  // Normal Display Mode On
#define ST7735_INVOFF      0x20  // Display Inversion Off
#define ST7735_INVON       0x21  // Display Inversion On
#define ST7735_GAMSET      0x26  // Gamma Set
#define ST7735_DISPOFF     0x28  // Display Off
#define ST7735_DISPON      0x29  // Display On
#define ST7735_CASET       0x2A  // Column Address Set
#define ST7735_RASET       0x2B  // Row Address Set
#define ST7735_RAMWR       0x2C  // Memory Write
#define ST7735_RAMRD       0x2E  // Memory Read
#define ST7735_PTLAR       0x30  // Partial Area
#define ST7735_TEOFF       0x34  // Tearing Effect Line OFF
#define ST7735_TEON        0x35  // Tearing Effect Line ON
#define ST7735_MADCTL      0x36  // Memory Data Access Control
#define ST7735_IDMOFF      0x38  // Idle Mode Off
#define ST7735_IDMON       0x39  // Idle Mode On
#define ST7735_COLMOD      0x3A  // Color Mode
#define ST7735_RDID1       0xDA  // Read ID1
#define ST7735_RDID2       0xDB  // Read ID2
#define ST7735_RDID3       0xDC  // Read ID3
#define ST7735_FRMCTR1     0xB1  // Frame Rate Control (In normal mode/ Full colors)
#define ST7735_FRMCTR2     0xB2  // Frame Rate Control (In Idle mode/ 8-colors)
#define ST7735_FRMCTR3     0xB3  // Frame Rate Control (In Partial mode/ full colors)
#define ST7735_INVCTR      0xB4  // Display Inversion Control
#define ST7735_DISSET5     0xB6  // Display Function set 5
#define ST7735_PWCTR1      0xC0  // Power Control 1
#define ST7735_PWCTR2      0xC1  // Power Control 2
#define ST7735_PWCTR3      0xC2  // Power Control 3 (in Normal mode/ Full colors)
#define ST7735_PWCTR4      0xC3  // Power Control 4 (in Idle mode/ 8-colors)
#define ST7735_PWCTR5      0xC4  // Power Control 5 (in Partial mode/ full-colors)
#define ST7735_VMCTR1      0xC5  // VCOM Control 1
#define ST7735_VMOFCTR     0xC7  // VCOM Offset Control
#define ST7735_WRID2       0xD1  // Write ID2 Value
#define ST7735_WRID3       0xD2  // Write ID3 Value
#define ST7735_NVCTR1      0xD9  // NVM Control Status
#define ST7735_NVCTR2      0xDE  // NVM Read Command
#define ST7735_NVCTR3      0xDF  // NVM Write Command
#define ST7735_GMCTRP1     0xE0  // Gamma ('+'polarity) Correction Characteristics Setting
#define ST7735_GMCTRN1     0xE1  // Gamma ('-'polarity) Correction Characteristics Setting
#define ST7735_GCV         0xFC  // Gate Pump Clock Frequency Variable

// Color definitions
#define ST7735_BLACK       0x0000
#define ST7735_BLUE        0x001F
#define ST7735_RED         0xF800
#define ST7735_GREEN       0x07E0
#define ST7735_CYAN        0x07FF
#define ST7735_MAGENTA     0xF81F
#define ST7735_YELLOW      0xFFE0
#define ST7735_WHITE       0xFFFF

#if POCKETOS_ST7735_ENABLE_REGISTER_ACCESS
// Complete ST7735 Register Map (Commands 0x00-0xFF)
static const RegisterDesc ST7735_REGISTERS[] = {
    RegisterDesc(0x00, "NOP", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x01, "SWRESET", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x04, "RDDID", 4, RegisterAccess::RO, 0x00),
    RegisterDesc(0x09, "RDDST", 5, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0A, "RDDPM", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0B, "RDDMADCTL", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0C, "RDDCOLMOD", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0D, "RDDIM", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x0E, "RDDSM", 2, RegisterAccess::RO, 0x00),
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
    RegisterDesc(0x34, "TEOFF", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x35, "TEON", 2, RegisterAccess::WO, 0x00),
    RegisterDesc(0x36, "MADCTL", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0x38, "IDMOFF", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x39, "IDMON", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x3A, "COLMOD", 2, RegisterAccess::RW, 0x05),
    RegisterDesc(0xB1, "FRMCTR1", 4, RegisterAccess::RW, 0x00),
    RegisterDesc(0xB2, "FRMCTR2", 4, RegisterAccess::RW, 0x00),
    RegisterDesc(0xB3, "FRMCTR3", 7, RegisterAccess::RW, 0x00),
    RegisterDesc(0xB4, "INVCTR", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xB6, "DISSET5", 3, RegisterAccess::RW, 0x00),
    RegisterDesc(0xC0, "PWCTR1", 4, RegisterAccess::RW, 0x00),
    RegisterDesc(0xC1, "PWCTR2", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xC2, "PWCTR3", 3, RegisterAccess::RW, 0x00),
    RegisterDesc(0xC3, "PWCTR4", 3, RegisterAccess::RW, 0x00),
    RegisterDesc(0xC4, "PWCTR5", 3, RegisterAccess::RW, 0x00),
    RegisterDesc(0xC5, "VMCTR1", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xC7, "VMOFCTR", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xD1, "WRID2", 4, RegisterAccess::WO, 0x00),
    RegisterDesc(0xD2, "WRID3", 4, RegisterAccess::WO, 0x00),
    RegisterDesc(0xD9, "NVCTR1", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xDA, "RDID1", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0xDB, "RDID2", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0xDC, "RDID3", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0xDE, "NVCTR2", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xDF, "NVCTR3", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xE0, "GMCTRP1", 16, RegisterAccess::WO, 0x00),
    RegisterDesc(0xE1, "GMCTRN1", 16, RegisterAccess::WO, 0x00),
    RegisterDesc(0xFC, "GCV", 2, RegisterAccess::RW, 0x00),
};

#define ST7735_REGISTER_COUNT (sizeof(ST7735_REGISTERS) / sizeof(RegisterDesc))
#endif

ST7735Driver::ST7735Driver() 
    : initialized_(false), _width(128), _height(160), _rotation(0) {
}

ST7735Driver::~ST7735Driver() {
    deinit();
}

bool ST7735Driver::init(const String& endpoint) {
    if (!initFromEndpoint(endpoint)) {
        Logger::error("ST7735: Failed to init SPI base");
        return false;
    }
    
    // Check for required DC pin
    if (getPinConfig().dc < 0) {
        Logger::error("ST7735: DC pin required");
        deinit();
        return false;
    }
    
    // Check for required RST pin
    if (getPinConfig().rst < 0) {
        Logger::error("ST7735: RST pin required");
        deinit();
        return false;
    }
    
    initialized_ = true;
    return true;
}

bool ST7735Driver::validEndpoints(const String& endpoint) {
    // Must have spi0 prefix, cs, dc, and rst pins
    if (!endpoint.startsWith("spi0:")) return false;
    if (endpoint.indexOf("cs=") < 0) return false;
    if (endpoint.indexOf("dc=") < 0) return false;
    if (endpoint.indexOf("rst=") < 0) return false;
    return true;
}

bool ST7735Driver::identifyProbe(const String& endpoint) {
    ST7735Driver driver;
    if (!driver.init(endpoint)) {
        return false;
    }
    
    if (!driver.hardwareReset()) {
        return false;
    }
    
    delay(120);
    
    // Read Display Status (0x09 command)
    uint8_t status[5] = {0};
    if (!driver.readData(ST7735_RDDST, status, 5)) {
        return false;
    }
    
    // Check if at least one byte is non-zero
    return (status[0] != 0 || status[1] != 0 || status[2] != 0 || status[3] != 0 || status[4] != 0);
}

#if POCKETOS_ST7735_ENABLE_BASIC_READ
bool ST7735Driver::begin() {
    if (!initialized_) return false;
    
    // Hardware reset
    if (!hardwareReset()) return false;
    delay(150);
    
    // Software reset
    sendCommand(ST7735_SWRESET);
    delay(150);
    
    // Sleep out
    sendCommand(ST7735_SLPOUT);
    delay(150);
    
    // Frame rate control - normal mode
    sendCommand(ST7735_FRMCTR1);
    sendData(0x01);  // Rate = fosc/(1x2+40) * (LINE+2C+2D)
    sendData(0x2C);
    sendData(0x2D);
    
    // Frame rate control - idle mode
    sendCommand(ST7735_FRMCTR2);
    sendData(0x01);
    sendData(0x2C);
    sendData(0x2D);
    
    // Frame rate control - partial mode
    sendCommand(ST7735_FRMCTR3);
    sendData(0x01);
    sendData(0x2C);
    sendData(0x2D);
    sendData(0x01);
    sendData(0x2C);
    sendData(0x2D);
    
    // Display inversion control
    sendCommand(ST7735_INVCTR);
    sendData(0x07);
    
    // Power control 1
    sendCommand(ST7735_PWCTR1);
    sendData(0xA2);
    sendData(0x02);
    sendData(0x84);
    
    // Power control 2
    sendCommand(ST7735_PWCTR2);
    sendData(0xC5);
    
    // Power control 3
    sendCommand(ST7735_PWCTR3);
    sendData(0x0A);
    sendData(0x00);
    
    // Power control 4
    sendCommand(ST7735_PWCTR4);
    sendData(0x8A);
    sendData(0x2A);
    
    // Power control 5
    sendCommand(ST7735_PWCTR5);
    sendData(0x8A);
    sendData(0xEE);
    
    // VCOM control
    sendCommand(ST7735_VMCTR1);
    sendData(0x0E);
    
    // Display inversion off
    sendCommand(ST7735_INVOFF);
    
    // Memory access control
    sendCommand(ST7735_MADCTL);
    sendData(0xC8);
    
    // Color mode - 16-bit color
    sendCommand(ST7735_COLMOD);
    sendData(0x05);
    
    // Positive gamma correction
    sendCommand(ST7735_GMCTRP1);
    sendData(0x02);
    sendData(0x1c);
    sendData(0x07);
    sendData(0x12);
    sendData(0x37);
    sendData(0x32);
    sendData(0x29);
    sendData(0x2d);
    sendData(0x29);
    sendData(0x25);
    sendData(0x2B);
    sendData(0x39);
    sendData(0x00);
    sendData(0x01);
    sendData(0x03);
    sendData(0x10);
    
    // Negative gamma correction
    sendCommand(ST7735_GMCTRN1);
    sendData(0x03);
    sendData(0x1d);
    sendData(0x07);
    sendData(0x06);
    sendData(0x2E);
    sendData(0x2C);
    sendData(0x29);
    sendData(0x2D);
    sendData(0x2E);
    sendData(0x2E);
    sendData(0x37);
    sendData(0x3F);
    sendData(0x00);
    sendData(0x00);
    sendData(0x02);
    sendData(0x10);
    
    // Normal display mode on
    sendCommand(ST7735_NORON);
    delay(10);
    
    // Display on
    sendCommand(ST7735_DISPON);
    delay(100);
    
    return true;
}

bool ST7735Driver::setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    if (!initialized_) return false;
    
    sendCommand(ST7735_CASET);
    sendData16(x0);
    sendData16(x1);
    
    sendCommand(ST7735_RASET);
    sendData16(y0);
    sendData16(y1);
    
    sendCommand(ST7735_RAMWR);
    return true;
}

bool ST7735Driver::setPixel(uint16_t x, uint16_t y, uint16_t color) {
    if (!initialized_) return false;
    if (x >= _width || y >= _height) return false;
    
    setWindow(x, y, x, y);
    sendData16(color);
    return true;
}

bool ST7735Driver::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
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

bool ST7735Driver::fillScreen(uint16_t color) {
    return fillRect(0, 0, _width, _height, color);
}

bool ST7735Driver::pushColor(uint16_t color) {
    if (!initialized_) return false;
    sendData16(color);
    return true;
}

bool ST7735Driver::pushColors(const uint16_t* colors, uint16_t len) {
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

#if POCKETOS_ST7735_ENABLE_ERROR_HANDLING
bool ST7735Driver::setRotation(uint8_t rotation) {
    if (!initialized_) return false;
    
    _rotation = rotation % 4;
    uint8_t madctl = 0;
    
    switch (_rotation) {
        case 0:
            madctl = 0xC8;
            _width = 128;
            _height = 160;
            break;
        case 1:
            madctl = 0xA8;
            _width = 160;
            _height = 128;
            break;
        case 2:
            madctl = 0x08;
            _width = 128;
            _height = 160;
            break;
        case 3:
            madctl = 0x68;
            _width = 160;
            _height = 128;
            break;
    }
    
    sendCommand(ST7735_MADCTL);
    sendData(madctl);
    return true;
}

bool ST7735Driver::invertDisplay(bool invert) {
    if (!initialized_) return false;
    sendCommand(invert ? ST7735_INVON : ST7735_INVOFF);
    return true;
}

bool ST7735Driver::setScrollArea(uint16_t top, uint16_t bottom) {
    if (!initialized_) return false;
    
    sendCommand(ST7735_PTLAR);
    sendData16(top);
    sendData16(_height - top - bottom);
    return true;
}

bool ST7735Driver::setScroll(uint16_t offset) {
    if (!initialized_) return false;
    
    // ST7735 doesn't have a direct scroll command like ST7789
    // This is a simplified implementation
    return false;
}

bool ST7735Driver::setSleep(bool sleep) {
    if (!initialized_) return false;
    sendCommand(sleep ? ST7735_SLPIN : ST7735_SLPOUT);
    delay(120);
    return true;
}

bool ST7735Driver::setIdleMode(bool idle) {
    if (!initialized_) return false;
    sendCommand(idle ? ST7735_IDMON : ST7735_IDMOFF);
    return true;
}

uint32_t ST7735Driver::readID() {
    uint8_t id[3] = {0};
    readData(ST7735_RDID1, &id[0], 1);
    readData(ST7735_RDID2, &id[1], 1);
    readData(ST7735_RDID3, &id[2], 1);
    return ((uint32_t)id[0] << 16) | ((uint32_t)id[1] << 8) | id[2];
}

uint8_t ST7735Driver::readStatus() {
    uint8_t status[5] = {0};
    readData(ST7735_RDDST, status, 5);
    return status[0];
}
#endif

#if POCKETOS_ST7735_ENABLE_REGISTER_ACCESS
const RegisterDesc* ST7735Driver::registers(size_t& count) const {
    count = ST7735_REGISTER_COUNT;
    return ST7735_REGISTERS;
}

bool ST7735Driver::writeCommand(uint8_t cmd) {
    return sendCommand(cmd);
}

bool ST7735Driver::writeData(uint8_t data) {
    return sendData(data);
}

bool ST7735Driver::writeData16(uint16_t data) {
    return sendData16(data);
}

bool ST7735Driver::readData(uint8_t cmd, uint8_t* buf, size_t len) {
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

const RegisterDesc* ST7735Driver::findRegisterByName(const String& name) const {
    size_t count;
    const RegisterDesc* regs = registers(count);
    return RegisterUtils::findByName(regs, count, name);
}
#endif

bool ST7735Driver::hardwareReset() {
    if (getPinConfig().rst < 0) return false;
    
    setRST(true);
    delay(10);
    setRST(false);
    delay(20);
    setRST(true);
    delay(150);
    
    return true;
}

bool ST7735Driver::sendCommand(uint8_t cmd) {
    if (!initialized_) return false;
    
    setDCCommand();
    setCS(true);
    SPI.transfer(cmd);
    setCS(false);
    
    return true;
}

bool ST7735Driver::sendData(uint8_t data) {
    if (!initialized_) return false;
    
    setDCData();
    setCS(true);
    SPI.transfer(data);
    setCS(false);
    
    return true;
}

bool ST7735Driver::sendData16(uint16_t data) {
    if (!initialized_) return false;
    
    setDCData();
    setCS(true);
    SPI.write16(data);
    setCS(false);
    
    return true;
}

} // namespace PocketOS
