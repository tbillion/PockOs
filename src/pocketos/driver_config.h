#ifndef POCKETOS_DRIVER_CONFIG_H
#define POCKETOS_DRIVER_CONFIG_H

/**
 * PocketOS Driver Configuration and Tiering System
 * 
 * This file defines the driver tiering system that allows compile-time
 * selection of driver feature sets to optimize for code size vs features.
 * 
 * Driver Tiers:
 * - Tier 1 (MINIMAL): Basic functionality only, minimal code size
 * - Tier 2 (STANDARD): Production-ready with error handling (default)
 * - Tier 3 (FULL): Complete feature set with all optimizations
 */

// ============================================================================
// GLOBAL DRIVER PACKAGE CONFIGURATION
// ============================================================================

/**
 * POCKETOS_DRIVER_PACKAGE: Global driver tier setting
 * 
 * 1 = MINIMAL   - Basic functionality, ~30-40% code size
 * 2 = STANDARD  - Production ready, ~70-80% code size (DEFAULT)
 * 3 = FULL      - Complete features, 100% code size
 * 
 * Can be overridden via build flags: -DPOCKETOS_DRIVER_PACKAGE=1
 */
#ifndef POCKETOS_DRIVER_PACKAGE
#define POCKETOS_DRIVER_PACKAGE 2  // Default to STANDARD
#endif

// Tier level definitions
#define POCKETOS_TIER_MINIMAL  1
#define POCKETOS_TIER_STANDARD 2
#define POCKETOS_TIER_FULL     3

// ============================================================================
// PER-DRIVER TIER OVERRIDES
// ============================================================================

/**
 * Per-driver tier overrides allow individual drivers to use a different
 * tier than the global package setting.
 * 
 * Example: Use full BME280 driver even in minimal package:
 *   -DPOCKETOS_DRIVER_TIER_BME280=3
 */

// BME280 Driver Tier (Temperature/Humidity/Pressure sensor)
#ifndef POCKETOS_DRIVER_TIER_BME280
#define POCKETOS_DRIVER_TIER_BME280 POCKETOS_DRIVER_PACKAGE
#endif

// GPIO Digital Output Driver Tier
#ifndef POCKETOS_DRIVER_TIER_GPIO_DOUT
#define POCKETOS_DRIVER_TIER_GPIO_DOUT POCKETOS_DRIVER_PACKAGE
#endif

// Add more driver tier overrides as drivers are created
// #ifndef POCKETOS_DRIVER_TIER_<DRIVER_NAME>
// #define POCKETOS_DRIVER_TIER_<DRIVER_NAME> POCKETOS_DRIVER_PACKAGE
// #endif

// ============================================================================
// FEATURE FLAGS BASED ON TIERS
// ============================================================================

/**
 * These macros make it easy to conditionally compile features based on tier.
 * Usage in driver code:
 * 
 * #if POCKETOS_BME280_ENABLE_ADVANCED_DIAGNOSTICS
 *     // Full-tier only code
 * #endif
 */

// BME280 Feature Flags
#if POCKETOS_DRIVER_TIER_BME280 >= POCKETOS_TIER_MINIMAL
#define POCKETOS_BME280_ENABLE_BASIC_READ 1
#else
#define POCKETOS_BME280_ENABLE_BASIC_READ 0
#endif

#if POCKETOS_DRIVER_TIER_BME280 >= POCKETOS_TIER_STANDARD
#define POCKETOS_BME280_ENABLE_ERROR_HANDLING 1
#define POCKETOS_BME280_ENABLE_LOGGING 1
#define POCKETOS_BME280_ENABLE_CONFIGURATION 1
#define POCKETOS_BME280_ENABLE_FULL_SCHEMA 1
#else
#define POCKETOS_BME280_ENABLE_ERROR_HANDLING 0
#define POCKETOS_BME280_ENABLE_LOGGING 0
#define POCKETOS_BME280_ENABLE_CONFIGURATION 0
#define POCKETOS_BME280_ENABLE_FULL_SCHEMA 0
#endif

#if POCKETOS_DRIVER_TIER_BME280 >= POCKETOS_TIER_FULL
#define POCKETOS_BME280_ENABLE_ADVANCED_DIAGNOSTICS 1
#define POCKETOS_BME280_ENABLE_FORCED_MODE 1
#define POCKETOS_BME280_ENABLE_IIR_FILTER 1
#define POCKETOS_BME280_ENABLE_STANDBY_CONFIG 1
#define POCKETOS_BME280_ENABLE_OVERSAMPLING_CONFIG 1
#else
#define POCKETOS_BME280_ENABLE_ADVANCED_DIAGNOSTICS 0
#define POCKETOS_BME280_ENABLE_FORCED_MODE 0
#define POCKETOS_BME280_ENABLE_IIR_FILTER 0
#define POCKETOS_BME280_ENABLE_STANDBY_CONFIG 0
#define POCKETOS_BME280_ENABLE_OVERSAMPLING_CONFIG 0
#endif

// GPIO DOUT Feature Flags
#if POCKETOS_DRIVER_TIER_GPIO_DOUT >= POCKETOS_TIER_MINIMAL
#define POCKETOS_GPIO_DOUT_ENABLE_BASIC 1
#else
#define POCKETOS_GPIO_DOUT_ENABLE_BASIC 0
#endif

#if POCKETOS_DRIVER_TIER_GPIO_DOUT >= POCKETOS_TIER_STANDARD
#define POCKETOS_GPIO_DOUT_ENABLE_ERROR_HANDLING 1
#define POCKETOS_GPIO_DOUT_ENABLE_LOGGING 1
#else
#define POCKETOS_GPIO_DOUT_ENABLE_ERROR_HANDLING 0
#define POCKETOS_GPIO_DOUT_ENABLE_LOGGING 0
#endif

#if POCKETOS_DRIVER_TIER_GPIO_DOUT >= POCKETOS_TIER_FULL
#define POCKETOS_GPIO_DOUT_ENABLE_PWM 1
#define POCKETOS_GPIO_DOUT_ENABLE_BLINK 1
#else
#define POCKETOS_GPIO_DOUT_ENABLE_PWM 0
#define POCKETOS_GPIO_DOUT_ENABLE_BLINK 0
#endif

// ============================================================================
// TIER INFORMATION MACROS
// ============================================================================

// Helper macros to get tier name as string
#define POCKETOS_TIER_NAME(tier) \
    ((tier) == POCKETOS_TIER_MINIMAL ? "MINIMAL" : \
     (tier) == POCKETOS_TIER_STANDARD ? "STANDARD" : \
     (tier) == POCKETOS_TIER_FULL ? "FULL" : "UNKNOWN")

// Get current package tier name
#define POCKETOS_PACKAGE_TIER_NAME POCKETOS_TIER_NAME(POCKETOS_DRIVER_PACKAGE)

// Get specific driver tier name
#define POCKETOS_BME280_TIER_NAME POCKETOS_TIER_NAME(POCKETOS_DRIVER_TIER_BME280)
#define POCKETOS_GPIO_DOUT_TIER_NAME POCKETOS_TIER_NAME(POCKETOS_DRIVER_TIER_GPIO_DOUT)

// ============================================================================
// VALIDATION
// ============================================================================

// Validate tier values are in range
#if POCKETOS_DRIVER_PACKAGE < 1 || POCKETOS_DRIVER_PACKAGE > 3
#error "POCKETOS_DRIVER_PACKAGE must be 1 (MINIMAL), 2 (STANDARD), or 3 (FULL)"
#endif

#if POCKETOS_DRIVER_TIER_BME280 < 1 || POCKETOS_DRIVER_TIER_BME280 > 3
#error "POCKETOS_DRIVER_TIER_BME280 must be 1, 2, or 3"
#endif

#if POCKETOS_DRIVER_TIER_GPIO_DOUT < 1 || POCKETOS_DRIVER_TIER_GPIO_DOUT > 3
#error "POCKETOS_DRIVER_TIER_GPIO_DOUT must be 1, 2, or 3"
#endif

#endif // POCKETOS_DRIVER_CONFIG_H
