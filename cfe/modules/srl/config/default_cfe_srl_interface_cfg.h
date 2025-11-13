/**
 * @file
 *   CFE Serial Services (CFE_SRL) Application Public Definitions
 *
 * This provides default values for configurable items that affect
 * the interface(s) of this module.  This includes the CMD/TLM message
 * interface, tables definitions, and any other data products that
 * serve to exchange information with other entities.
 *
 * @note This file may be overridden/superceded by mission-provided definitions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef CFE_SRL_INTERFACE_CFG_H
#define CFE_SRL_INTERFACE_CFG_H


/******************* Macro Definitions ***********************/

/**
 * The Maximum number of general (i.e. Native, non CSP) Device
 * Can be enlarged. If the general device is too many.
 */
#define CFE_SRL_GLOBAL_HANDLE_NUM   20

/**
 * The maximum number of CSP Device
 * Can **NOT** be enlarged, because of the libgscsp architecture
 */
#define CFE_SRL_CSP_MAX_DEVICE_NUM  32

/* Max Handle name length */
#define CFE_SRL_HANDLE_NAME_LENGTH  (OS_MAX_FILE_NAME - 4)

typedef enum {
    SRL_DEVTYPE_I2C = 1,
    SRL_DEVTYPE_SPI,
    SRL_DEVTYPE_CAN,
    SRL_DEVTYPE_UART,
    SRL_DEVTYPE_RS422
} CFE_SRL_DevType_t;


typedef enum {
    CFE_SRL_HANDLE_STATUS_NONE = 0x00,
    CFE_SRL_HANDLE_STATUS_ALLOCATE = 0x01,
    CFE_SRL_HANDLE_STATUS_FD_INIT = 0x02,
    CFE_SRL_HANDLE_STATUS_MUTEX_INIT = 0x04,
    CFE_SRL_HANDLE_STATUS_ALL = 0x07
} CFE_SRL_Handle_Status_t;

#endif /* CFE_SRL_INTERFACE_CFG_H */