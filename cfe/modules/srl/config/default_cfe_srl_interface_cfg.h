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
#define CFE_SRL_HANDLE_NAME_LENGTH  16

#endif /* CFE_SRL_INTERFACE_CFG_H */