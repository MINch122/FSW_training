#ifndef CFE_SRL_EXTERN_TYPEDEF_H
#define CFE_SRL_EXTERN_TYPEDEF_H

/**
 * @file
 *
 * Declarations and prototypes for cfe_srl_extern_typedefs module
 */
#include "common_types.h"
#include "cfe_mission_cfg.h"

#include "iodriver_serial_io.h"

/**
 * Re-definition of PSP iodriver serial transfer param struct
 * Higher layer should use the "re-defined" struct (i.e. `CFE_SRL_IO_Param_t`)
 */
typedef CFE_PSP_IODriver_SerialXferParam_t    CFE_SRL_IO_Param_t;

/**
 * Re-definition of PSP iodriver serial config struct
 * Higher Layer should use the "re-defined" struct 
 */
typedef CFE_PSP_IODriver_Serial_cfg_t         CFE_SRL_IO_Config_t;

#endif /* CFE_SRL_EXTERN_TYPEDEF_H */