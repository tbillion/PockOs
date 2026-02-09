#ifndef POCKETOS_REGISTER_TYPES_H
#define POCKETOS_REGISTER_TYPES_H

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>

namespace PocketOS {

/**
 * Register access type
 */
enum class RegisterAccess {
    RO = 0,  // Read-only
    WO = 1,  // Write-only
    RW = 2,  // Read-write
    RC = 3   // Read-clear (reading clears the value)
};

/**
 * Bus type (for register access routing)
 */
enum class BusType {
    I2C = 0,
    SPI = 1,
    UNKNOWN = 255
};

/**
 * Register descriptor structure
 * 
 * This structure describes a single hardware register in a device.
 * Used by Tier 2 drivers to expose complete register maps.
 */
struct RegisterDesc {
    uint16_t addr;              // Register address
    const char* name;           // Register name (for CLI access)
    uint8_t width;              // Width in bytes (1, 2, 3, or 4)
    RegisterAccess access;      // Access type (RO/WO/RW/RC)
    uint32_t reset;             // Reset value (if known, 0 if unknown)
    
    RegisterDesc() : addr(0), name(""), width(1), access(RegisterAccess::RO), reset(0) {}
    
    RegisterDesc(uint16_t a, const char* n, uint8_t w, RegisterAccess acc, uint32_t rst = 0)
        : addr(a), name(n), width(w), access(acc), reset(rst) {}
};

/**
 * Register utilities
 */
class RegisterUtils {
public:
    /**
     * Find register by address
     */
    static const RegisterDesc* findByAddr(const RegisterDesc* regs, size_t count, uint16_t addr) {
        for (size_t i = 0; i < count; i++) {
            if (regs[i].addr == addr) {
                return &regs[i];
            }
        }
        return nullptr;
    }
    
    /**
     * Find register by name (case-insensitive)
     */
    static const RegisterDesc* findByName(const RegisterDesc* regs, size_t count, const String& name) {
        String searchName = name;
        searchName.toUpperCase();
        
        for (size_t i = 0; i < count; i++) {
            String regName = String(regs[i].name);
            regName.toUpperCase();
            if (regName == searchName) {
                return &regs[i];
            }
        }
        return nullptr;
    }
    
    /**
     * Get access type as string
     */
    static const char* accessToString(RegisterAccess access) {
        switch (access) {
            case RegisterAccess::RO: return "RO";
            case RegisterAccess::WO: return "WO";
            case RegisterAccess::RW: return "RW";
            case RegisterAccess::RC: return "RC";
            default: return "??";
        }
    }
    
    /**
     * Check if register is readable
     */
    static bool isReadable(RegisterAccess access) {
        return access == RegisterAccess::RO || 
               access == RegisterAccess::RW || 
               access == RegisterAccess::RC;
    }
    
    /**
     * Check if register is writable
     */
    static bool isWritable(RegisterAccess access) {
        return access == RegisterAccess::WO || 
               access == RegisterAccess::RW;
    }
    
    /**
     * Get bus type as string
     */
    static const char* busTypeToString(BusType type) {
        switch (type) {
            case BusType::I2C: return "I2C";
            case BusType::SPI: return "SPI";
            default: return "UNKNOWN";
        }
    }
};

} // namespace PocketOS

#endif // POCKETOS_REGISTER_TYPES_H
