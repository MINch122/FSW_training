/*
 *  Copyright (c) 2015, United States government as represented by the
 *  administrator of the National Aeronautics Space Administration.
 *  All rights reserved. This software was created at NASA Glenn
 *  Research Center pursuant to government contracts.
 */

/**
 * \file
 *
 * I/O adapter for discrete (digitial gpio) interfaces
 */

#ifndef CFE_PSP_IODRIVER_DISCRETE_IO_H
#define CFE_PSP_IODRIVER_DISCRETE_IO_H

/* Include all base definitions */
#include "iodriver_base.h"

/**
 * Type abstraction for expressing digital logic levels.
 *
 * This value will be filled starting with the LSB. A typical GPIO logic channel is 1 bit, so
 * only the LSB is signficiant and the other bits are not used.
 *
 * This allows single channels up to 8 bits wide, but multiple "channels" could be concatenated
 * using a multi-read/write opcode to allow atomic access to any number of bits.
 */
typedef uint8 CFE_PSP_IODriver_GpioLevel_t;

/**
 * Enumerated names for typical digital 1-bit logic channel states.
 *
 * For convenience / code readability.
 */
enum
{
    CFE_PSP_IODriver_GPIO_LOGIC_LOW  = 0,
    CFE_PSP_IODriver_GPIO_LOGIC_HIGH = 1
};

/**
 * Complete API container for gpio read/write commands.
 * This allows reading/writing multiple channels at once with a single entry into the API.
 * As each entry into the API needs to acquire a mutex for serialization, this can be much
 * more efficient to read channels through this means rather than single channel read/write.
 */
typedef struct
{
    uint16 NumChannels;                    /**<  Number of channels in the i/o structure (length of "samples" array) */
    CFE_PSP_IODriver_GpioLevel_t *Samples; /**<  Array for digital logic levels */
} CFE_PSP_IODriver_GpioRdWr_t;

/**
 * GPIO configuration for initialization
 * Only used for init(i.e. open subsystem)
 * If configuration change needed, use `CFE_PSP_IODriver_Gpio_config_t` instead.
 */
typedef struct  {
    const char *path;
    unsigned int line;
    const char *name;
    bool default_val;
    bool isout;
} CFE_PSP_IODriver_Gpio_init_t;

/**
 * GPIO configuration payload for change config
 * Should not use directly.
 */
typedef struct  {
    const char *name;
    bool default_val;
    bool isout;
} CFE_PSP_IODriver_Gpio_cfg_t;
/**
 * GPIO configuration struct for change config
 * Only used for config subsystem
 * If initialization needed, use `CFE_PSP_IODriver_Gpio_init_t` instead
 */
typedef struct {
    int handle;
    CFE_PSP_IODriver_Gpio_cfg_t cfg;
} CFE_PSP_IODriver_Gpio_config_t;

/**
 * GPIO I/O struct for change logic level
 * Used for write/read subsystems
 * Should not used at other subsystems
 */
typedef struct {
    int handle;                            /* <\brief handle for gpio table indexing */
    CFE_PSP_IODriver_GpioLevel_t level;    /* <\brief logical level. `1` for high, `0` for low */
} CFE_PSP_IODriver_GpioVal_t;

/**
 * Opcodes specific to digital GPIO devices
 */
enum
{
    CFE_PSP_IODriver_DISCRETE_IO_NOOP = CFE_PSP_IODriver_DISCRETE_IO_CLASS_BASE,

    CFE_PSP_IODriver_DISCRETE_IO_READ_CHANNELS,  /**< CFE_PSP_IODriver_GpioRdWr_t argument */
    CFE_PSP_IODriver_DISCRETE_IO_WRITE_CHANNELS, /**< CFE_PSP_IODriver_GpioRdWr_t argument */

    CFE_PSP_IODriver_DISCRETE_IO_MAX
};

/**
 * Additional error codes specific to GPIO
 *
 * These are based from the CFE_PSP_IODriver_SERIAL_IO_CLASS_BASE so as to not conflict with other classes of I/O
 */
enum {
    CFE_PSP_IODriver_DISCRETE_IO_ERROR_NONE = 0, // Use CFE_PSP_SUCCESS instead

    CFE_PSP_IODriver_DISCRETE_IO_ERROR_BASE = -(CFE_PSP_IODriver_DISCRETE_IO_CLASS_BASE + 0xFFFF),
    CFE_PSP_IODriver_DISCRETE_IO_INVALID_TYPE_ERROR,
    CFE_PSP_IODriver_DISCRETE_IO_TABLE_FULL_ERROR,
    CFE_PSP_IODriver_DISCRETE_IO_CHIP_OPEN_ERROR,
    CFE_PSP_IODriver_DISCRETE_IO_GET_LINE_ERROR,
    CFE_PSP_IODriver_DISCRETE_IO_REQUEST_OUTPUT_ERROR,
    CFE_PSP_IODriver_DISCRETE_IO_REQUEST_INPUT_ERROR,

    CFE_PSP_IODriver_DISCRETE_IO_NOT_OPEN_ERROR,
    CFE_PSP_IODriver_DISCRETE_IO_INVALID_DIRECTION_ERROR,

    CFE_PSP_IODriver_DISCRETE_IO_SET_ERROR,
    CFE_PSP_IODriver_DISCRETE_IO_GET_ERROR,

    CFE_PSP_IODriver_DISCRETE_IO_INVALID_HANDLE,

};
#endif /* CFE_PSP_IODRIVER_DISCRETE_IO_H */
