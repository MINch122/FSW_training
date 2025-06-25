/**
 * @file
 *   Specification for the CFE Serial Services (CFE_SRL) command and telemetry
 *   message payload constant definitions.
 *
 *  For CFE_SRL this is only the function/command code definitions
 */
#ifndef CFE_SRL_MSGDEFS_H
#define CFE_SRL_MSGDEFS_H

#include "common_types.h"
#include "cfe_mission_cfg.h"
#include "cfe_srl_extern_typedefs.h"
#include "cfe_srl_mission_cfg.h"
#include "cfe_srl_interface_cfg.h"

typedef struct CFE_SRL_ResetHandleCounterCmd_Payload {

    CFE_SRL_Handle_Indexer_t Indexer;

} CFE_SRL_ResetHandleCounterCmd_Payload_t;

typedef struct CFE_SRL_GetHandleStatusCmd_Payload {

    CFE_SRL_Handle_Indexer_t Indexer;

} CFE_SRL_GetHandleStatusCmd_Payload_t;


typedef struct CFE_SRL_CloseHandleCmd_Payload {

    CFE_SRL_Handle_Indexer_t Indexer;

} CFE_SRL_CloseHandleCmd_Payload_t;


typedef struct CFE_SRL_HandleInitCmd_Payload {

    /**
     * Determine Handle itself & MutexID
     */
    CFE_SRL_Handle_Indexer_t Indexer;

    /**
     * Device Name
     */
    char Name[CFE_SRL_HANDLE_NAME_LENGTH];
    
    /**
     * Device file path
     */
    char DevName[CFE_SRL_HANDLE_NAME_LENGTH];

    /**
     * Device(= Protocol) type
     */
    uint8_t DevType;

    /**
     * Only need UART series init
     */
    uint32_t BaudRate;

    /**
     * Only need SPI init
     */
    uint8_t SPIMode;
    
} CFE_SRL_HandleInitCmd_Payload_t;


typedef struct CFE_SRL_HousekeepingTlm_Payload {
    uint8 CommandCounter;

    uint8 CommandErrorCounter;

    uint8 IOHandleStatus[CFE_SRL_GNRL_DEVICE_NUM];

    CFE_SRL_IO_Handle_t IOHandle[CFE_SRL_GNRL_DEVICE_NUM];

    CFE_SRL_GPIO_Handle_t GPIOHandle[CFE_SRL_TOT_GPIO_NUM];
    
} CFE_SRL_HousekeepingTlm_Payload_t;

#endif /* CFE_SRL_MSGDEFS_H */