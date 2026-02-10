#include "ili9341_driver.h"
#include "../core/logger.h"
#include "../core/resource_manager.h"
#include <SPI.h>

namespace PocketOS {

// ILI9341 Command Definitions (0x00-0xFF)
#define ILI9341_NOP         0x00  // No Operation
#define ILI9341_SWRESET     0x01  // Software Reset
#define ILI9341_RDDID       0x04  // Read Display ID
#define ILI9341_RDDST       0x09  // Read Display Status
#define ILI9341_RDDPM       0x0A  // Read Display Power Mode
#define ILI9341_RDDMADCTL   0x0B  // Read Display MADCTL
#define ILI9341_RDDCOLMOD   0x0C  // Read Display Pixel Format
#define ILI9341_RDDIM       0x0D  // Read Display Image Mode
#define ILI9341_RDDSM       0x0E  // Read Display Signal Mode
#define ILI9341_RDDSDR      0x0F  // Read Display Self-Diagnostic Result
#define ILI9341_SLPIN       0x10  // Sleep In
#define ILI9341_SLPOUT      0x11  // Sleep Out
#define ILI9341_PTLON       0x12  // Partial Mode On
#define ILI9341_NORON       0x13  // Normal Display Mode On
#define ILI9341_INVOFF      0x20  // Display Inversion Off
#define ILI9341_INVON       0x21  // Display Inversion On
#define ILI9341_GAMMASET    0x26  // Gamma Set
#define ILI9341_DISPOFF     0x28  // Display Off
#define ILI9341_DISPON      0x29  // Display On
#define ILI9341_CASET       0x2A  // Column Address Set
#define ILI9341_PASET       0x2B  // Page Address Set
#define ILI9341_RAMWR       0x2C  // Memory Write
#define ILI9341_RAMRD       0x2E  // Memory Read
#define ILI9341_PTLAR       0x30  // Partial Area
#define ILI9341_VSCRDEF     0x33  // Vertical Scrolling Definition
#define ILI9341_MADCTL      0x36  // Memory Access Control
#define ILI9341_VSCRSADD    0x37  // Vertical Scrolling Start Address
#define ILI9341_IDMOFF      0x38  // Idle Mode Off
#define ILI9341_IDMON       0x39  // Idle Mode On
#define ILI9341_PIXFMT      0x3A  // Pixel Format Set
#define ILI9341_RAMWRC      0x3C  // Memory Write Continue
#define ILI9341_RAMRDC      0x3E  // Memory Read Continue
#define ILI9341_TESLOUT     0x44  // Set Tear Scanline
#define ILI9341_TESCAN      0x45  // Get Scanline
#define ILI9341_WRDISBV     0x51  // Write Display Brightness
#define ILI9341_RDDISBV     0x52  // Read Display Brightness
#define ILI9341_WRCTRLD     0x53  // Write CTRL Display
#define ILI9341_RDCTRLD     0x54  // Read CTRL Display
#define ILI9341_WRCABC      0x55  // Write Content Adaptive Brightness Control
#define ILI9341_RDCABC      0x56  // Read Content Adaptive Brightness Control
#define ILI9341_WRCABCMB    0x5E  // Write CABC Minimum Brightness
#define ILI9341_RDCABCMB    0x5F  // Read CABC Minimum Brightness
#define ILI9341_RDID1       0xDA  // Read ID1
#define ILI9341_RDID2       0xDB  // Read ID2
#define ILI9341_RDID3       0xDC  // Read ID3
#define ILI9341_IFMODE      0xB0  // RGB Interface Signal Control
#define ILI9341_FRMCTR1     0xB1  // Frame Rate Control (In Normal Mode)
#define ILI9341_FRMCTR2     0xB2  // Frame Rate Control (In Idle Mode)
#define ILI9341_FRMCTR3     0xB3  // Frame Rate Control (In Partial Mode)
#define ILI9341_INVCTR      0xB4  // Display Inversion Control
#define ILI9341_DFUNCTR     0xB6  // Display Function Control
#define ILI9341_ETMOD       0xB7  // Entry Mode Set
#define ILI9341_PWCTR1      0xC0  // Power Control 1
#define ILI9341_PWCTR2      0xC1  // Power Control 2
#define ILI9341_PWCTR3      0xC2  // Power Control 3
#define ILI9341_PWCTR4      0xC3  // Power Control 4
#define ILI9341_PWCTR5      0xC4  // Power Control 5
#define ILI9341_VMCTR1      0xC5  // VCOM Control 1
#define ILI9341_VMCTR2      0xC7  // VCOM Control 2
#define ILI9341_RDID4       0xD3  // Read ID4
#define ILI9341_GMCTRP1     0xE0  // Positive Gamma Correction
#define ILI9341_GMCTRN1     0xE1  // Negative Gamma Correction
#define ILI9341_DGMCTR1     0xE2  // Digital Gamma Control 1
#define ILI9341_DGMCTR2     0xE3  // Digital Gamma Control 2
#define ILI9341_IFCTL       0xF6  // Interface Control

// Color definitions
#define ILI9341_BLACK       0x0000
#define ILI9341_BLUE        0x001F
#define ILI9341_RED         0xF800
#define ILI9341_GREEN       0x07E0
#define ILI9341_CYAN        0x07FF
#define ILI9341_MAGENTA     0xF81F
#define ILI9341_YELLOW      0xFFE0
#define ILI9341_WHITE       0xFFFF

#if POCKETOS_ILI9341_ENABLE_REGISTER_ACCESS
// Complete ILI9341 Register Map (Commands 0x00-0xFF)
static const RegisterDesc ILI9341_REGISTERS[] = {
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
    RegisterDesc(0x26, "GAMMASET", 2, RegisterAccess::WO, 0x01),
    RegisterDesc(0x28, "DISPOFF", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x29, "DISPON", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x2A, "CASET", 5, RegisterAccess::WO, 0x00),
    RegisterDesc(0x2B, "PASET", 5, RegisterAccess::WO, 0x00),
    RegisterDesc(0x2C, "RAMWR", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x2E, "RAMRD", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x30, "PTLAR", 5, RegisterAccess::WO, 0x00),
    RegisterDesc(0x33, "VSCRDEF", 7, RegisterAccess::WO, 0x00),
    RegisterDesc(0x36, "MADCTL", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0x37, "VSCRSADD", 3, RegisterAccess::WO, 0x00),
    RegisterDesc(0x38, "IDMOFF", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x39, "IDMON", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x3A, "PIXFMT", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0x3C, "RAMWRC", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0x3E, "RAMRDC", 1, RegisterAccess::RO, 0x00),
    RegisterDesc(0x44, "TESLOUT", 3, RegisterAccess::WO, 0x00),
    RegisterDesc(0x45, "TESCAN", 3, RegisterAccess::RO, 0x00),
    RegisterDesc(0x51, "WRDISBV", 2, RegisterAccess::WO, 0x00),
    RegisterDesc(0x52, "RDDISBV", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x53, "WRCTRLD", 2, RegisterAccess::WO, 0x00),
    RegisterDesc(0x54, "RDCTRLD", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x55, "WRCABC", 2, RegisterAccess::WO, 0x00),
    RegisterDesc(0x56, "RDCABC", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0x5E, "WRCABCMB", 2, RegisterAccess::WO, 0x00),
    RegisterDesc(0x5F, "RDCABCMB", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0xB0, "IFMODE", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xB1, "FRMCTR1", 3, RegisterAccess::RW, 0x00),
    RegisterDesc(0xB2, "FRMCTR2", 3, RegisterAccess::RW, 0x00),
    RegisterDesc(0xB3, "FRMCTR3", 3, RegisterAccess::RW, 0x00),
    RegisterDesc(0xB4, "INVCTR", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xB6, "DFUNCTR", 5, RegisterAccess::RW, 0x00),
    RegisterDesc(0xB7, "ETMOD", 2, RegisterAccess::RW, 0x00),
    RegisterDesc(0xC0, "PWCTR1", 2, RegisterAccess::RW, 0x23),
    RegisterDesc(0xC1, "PWCTR2", 2, RegisterAccess::RW, 0x10),
    RegisterDesc(0xC2, "PWCTR3", 3, RegisterAccess::RW, 0x00),
    RegisterDesc(0xC3, "PWCTR4", 3, RegisterAccess::RW, 0x00),
    RegisterDesc(0xC4, "PWCTR5", 3, RegisterAccess::RW, 0x00),
    RegisterDesc(0xC5, "VMCTR1", 3, RegisterAccess::RW, 0x31),
    RegisterDesc(0xC7, "VMCTR2", 2, RegisterAccess::RW, 0xC0),
    RegisterDesc(0xD3, "RDID4", 4, RegisterAccess::RO, 0x00),
    RegisterDesc(0xDA, "RDID1", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0xDB, "RDID2", 2, RegisterAccess::RO, 0x80),
    RegisterDesc(0xDC, "RDID3", 2, RegisterAccess::RO, 0x00),
    RegisterDesc(0xE0, "GMCTRP1", 15, RegisterAccess::WO, 0x00),
    RegisterDesc(0xE1, "GMCTRN1", 15, RegisterAccess::WO, 0x00),
    RegisterDesc(0xE2, "DGMCTR1", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0xE3, "DGMCTR2", 1, RegisterAccess::WO, 0x00),
    RegisterDesc(0xF6, "IFCTL", 4, RegisterAccess::RW, 0x00),
};

#define ILI9341_REGISTER_COUNT (sizeof(ILI9341_REGISTERS) / sizeof(RegisterDesc))
#endif

ILI9341Driver::ILI9341Driver() 
    : initialized_(false), _width(240), _height(320), _rotation(0) {
}

ILI9341Driver::~ILI9341Driver() {
    deinit();
}

bool ILI9341Driver::init(const String& endpoint) {
    if (!initFromEndpoint(endpoint)) {
        Logger::error("ILI9341: Failed to init SPI base");
        return false;
    }
    
    // Check for required DC pin
    if (getPinConfig().dc < 0) {
        Logger::error("ILI9341: DC pin required");
        deinit();
        return false;
    }
    
    // Check for required RST pin
    if (getPinConfig().rst < 0) {
        Logger::error("ILI9341: RST pin required");
        deinit();
        return false;
    }
    
    initialized_ = true;
    return true;
}

bool ILI9341Driver::validEndpoints(const String& endpoint) {
    // Must have spi0 prefix, cs, dc, and rst pins
    if (!endpoint.startsWith("spi0:")) return false;
    if (endpoint.indexOf("cs=") < 0) return false;
    if (endpoint.indexOf("dc=") < 0) return false;
    if (endpoint.indexOf("rst=") < 0) return false;
    return true;
}

bool ILI9341Driver::identifyProbe(const String& endpoint) {
    ILI9341Driver driver;
    if (!driver.init(endpoint)) {
        return false;
    }
    
    if (!driver.hardwareReset()) {
        return false;
    }
    
    delay(120);
    
    // Read Display ID (0x04 command)
    uint8_t id[4] = {0};
    if (!driver.readData(ILI9341_RDDID, id, 4)) {
        return false;
    }
    
    // ILI9341 should return manufacturer ID in byte 1
    // Typical values: 0x00, 0x93, 0x41 or similar
    // Check if at least one byte is non-zero
    return (id[0] != 0 || id[1] != 0 || id[2] != 0 || id[3] != 0);
}

#if POCKETOS_ILI9341_ENABLE_BASIC_READ
bool ILI9341Driver::begin() {
    if (!initialized_) return false;
    
    // Hardware reset
    if (!hardwareReset()) return false;
    delay(150);
    
    // Software reset
    sendCommand(ILI9341_SWRESET);
    delay(120);
    
    // Power control A
    sendCommand(0xCB);
    sendData(0x39);
    sendData(0x2C);
    sendData(0x00);
    sendData(0x34);
    sendData(0x02);
    
    // Power control B
    sendCommand(0xCF);
    sendData(0x00);
    sendData(0xC1);
    sendData(0x30);
    
    // Driver timing control A
    sendCommand(0xE8);
    sendData(0x85);
    sendData(0x00);
    sendData(0x78);
    
    // Driver timing control B
    sendCommand(0xEA);
    sendData(0x00);
    sendData(0x00);
    
    // Power on sequence control
    sendCommand(0xED);
    sendData(0x64);
    sendData(0x03);
    sendData(0x12);
    sendData(0x81);
    
    // Pump ratio control
    sendCommand(0xF7);
    sendData(0x20);
    
    // Power control 1
    sendCommand(ILI9341_PWCTR1);
    sendData(0x23);
    
    // Power control 2
    sendCommand(ILI9341_PWCTR2);
    sendData(0x10);
    
    // VCOM control 1
    sendCommand(ILI9341_VMCTR1);
    sendData(0x3e);
    sendData(0x28);
    
    // VCOM control 2
    sendCommand(ILI9341_VMCTR2);
    sendData(0x86);
    
    // Memory access control (rotation)
    sendCommand(ILI9341_MADCTL);
    sendData(0x48);
    
    // Pixel format
    sendCommand(ILI9341_PIXFMT);
    sendData(0x55);  // 16-bit color
    
    // Frame rate
    sendCommand(ILI9341_FRMCTR1);
    sendData(0x00);
    sendData(0x18);
    
    // Display function control
    sendCommand(ILI9341_DFUNCTR);
    sendData(0x08);
    sendData(0x82);
    sendData(0x27);
    
    // Enable 3G
    sendCommand(0xF2);
    sendData(0x00);
    
    // Gamma set
    sendCommand(ILI9341_GAMMASET);
    sendData(0x01);
    
    // Positive gamma correction
    sendCommand(ILI9341_GMCTRP1);
    sendData(0x0F);
    sendData(0x31);
    sendData(0x2B);
    sendData(0x0C);
    sendData(0x0E);
    sendData(0x08);
    sendData(0x4E);
    sendData(0xF1);
    sendData(0x37);
    sendData(0x07);
    sendData(0x10);
    sendData(0x03);
    sendData(0x0E);
    sendData(0x09);
    sendData(0x00);
    
    // Negative gamma correction
    sendCommand(ILI9341_GMCTRN1);
    sendData(0x00);
    sendData(0x0E);
    sendData(0x14);
    sendData(0x03);
    sendData(0x11);
    sendData(0x07);
    sendData(0x31);
    sendData(0xC1);
    sendData(0x48);
    sendData(0x08);
    sendData(0x0F);
    sendData(0x0C);
    sendData(0x31);
    sendData(0x36);
    sendData(0x0F);
    
    // Sleep out
    sendCommand(ILI9341_SLPOUT);
    delay(120);
    
    // Display on
    sendCommand(ILI9341_DISPON);
    delay(100);
    
    return true;
}

bool ILI9341Driver::setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    if (!initialized_) return false;
    
    sendCommand(ILI9341_CASET);
    sendData16(x0);
    sendData16(x1);
    
    sendCommand(ILI9341_PASET);
    sendData16(y0);
    sendData16(y1);
    
    sendCommand(ILI9341_RAMWR);
    return true;
}

bool ILI9341Driver::setPixel(uint16_t x, uint16_t y, uint16_t color) {
    if (!initialized_) return false;
    if (x >= _width || y >= _height) return false;
    
    setWindow(x, y, x, y);
    sendData16(color);
    return true;
}

bool ILI9341Driver::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
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

bool ILI9341Driver::fillScreen(uint16_t color) {
    return fillRect(0, 0, _width, _height, color);
}

bool ILI9341Driver::pushColor(uint16_t color) {
    if (!initialized_) return false;
    sendData16(color);
    return true;
}

bool ILI9341Driver::pushColors(const uint16_t* colors, uint16_t len) {
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

#if POCKETOS_ILI9341_ENABLE_ERROR_HANDLING
bool ILI9341Driver::setRotation(uint8_t rotation) {
    if (!initialized_) return false;
    
    _rotation = rotation % 4;
    uint8_t madctl = 0;
    
    switch (_rotation) {
        case 0:
            madctl = 0x48;
            _width = 240;
            _height = 320;
            break;
        case 1:
            madctl = 0x28;
            _width = 320;
            _height = 240;
            break;
        case 2:
            madctl = 0x88;
            _width = 240;
            _height = 320;
            break;
        case 3:
            madctl = 0xE8;
            _width = 320;
            _height = 240;
            break;
    }
    
    sendCommand(ILI9341_MADCTL);
    sendData(madctl);
    return true;
}

bool ILI9341Driver::invertDisplay(bool invert) {
    if (!initialized_) return false;
    sendCommand(invert ? ILI9341_INVON : ILI9341_INVOFF);
    return true;
}

bool ILI9341Driver::setScrollArea(uint16_t top, uint16_t bottom) {
    if (!initialized_) return false;
    
    sendCommand(ILI9341_VSCRDEF);
    sendData16(top);
    sendData16(_height - top - bottom);
    sendData16(bottom);
    return true;
}

bool ILI9341Driver::setScroll(uint16_t offset) {
    if (!initialized_) return false;
    
    sendCommand(ILI9341_VSCRSADD);
    sendData16(offset);
    return true;
}

bool ILI9341Driver::setSleep(bool sleep) {
    if (!initialized_) return false;
    sendCommand(sleep ? ILI9341_SLPIN : ILI9341_SLPOUT);
    delay(120);
    return true;
}

bool ILI9341Driver::setIdleMode(bool idle) {
    if (!initialized_) return false;
    sendCommand(idle ? ILI9341_IDMON : ILI9341_IDMOFF);
    return true;
}

uint32_t ILI9341Driver::readID() {
    uint8_t id[4] = {0};
    readData(ILI9341_RDDID, id, 4);
    return ((uint32_t)id[0] << 24) | ((uint32_t)id[1] << 16) | ((uint32_t)id[2] << 8) | id[3];
}

uint8_t ILI9341Driver::readStatus() {
    uint8_t status[5] = {0};
    readData(ILI9341_RDDST, status, 5);
    return status[0];
}
#endif

#if POCKETOS_ILI9341_ENABLE_REGISTER_ACCESS
const RegisterDesc* ILI9341Driver::registers(size_t& count) const {
    count = ILI9341_REGISTER_COUNT;
    return ILI9341_REGISTERS;
}

bool ILI9341Driver::writeCommand(uint8_t cmd) {
    return sendCommand(cmd);
}

bool ILI9341Driver::writeData(uint8_t data) {
    return sendData(data);
}

bool ILI9341Driver::writeData16(uint16_t data) {
    return sendData16(data);
}

bool ILI9341Driver::readData(uint8_t cmd, uint8_t* buf, size_t len) {
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

const RegisterDesc* ILI9341Driver::findRegisterByName(const String& name) const {
    size_t count;
    const RegisterDesc* regs = registers(count);
    return RegisterUtils::findByName(regs, count, name);
}
#endif

bool ILI9341Driver::hardwareReset() {
    if (getPinConfig().rst < 0) return false;
    
    setRST(true);
    delay(10);
    setRST(false);
    delay(20);
    setRST(true);
    delay(150);
    
    return true;
}

bool ILI9341Driver::sendCommand(uint8_t cmd) {
    if (!initialized_) return false;
    
    setDCCommand();
    setCS(true);
    SPI.transfer(cmd);
    setCS(false);
    
    return true;
}

bool ILI9341Driver::sendData(uint8_t data) {
    if (!initialized_) return false;
    
    setDCData();
    setCS(true);
    SPI.transfer(data);
    setCS(false);
    
    return true;
}

bool ILI9341Driver::sendData16(uint16_t data) {
    if (!initialized_) return false;
    
    setDCData();
    setCS(true);
    SPI.write16(data);
    setCS(false);
    
    return true;
}

} // namespace PocketOS
