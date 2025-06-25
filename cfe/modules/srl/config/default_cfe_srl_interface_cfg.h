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
#define CFE_SRL_GLOBAL_HANDLE_NUM   20
#define CFE_SRL_DEVICE_TABLE_NUM    20

/* Max Handle name length */
#define CFE_SRL_HANDLE_NAME_LENGTH  16

#endif /* CFE_SRL_INTERFACE_CFG_H */