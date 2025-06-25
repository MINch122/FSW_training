/**
 * @file
 *
 *  Purpose:
 *  cFE Serial Services (SRL) Command and Telemetry packet definition file.
 *
 *  References:
 *     Flight Software Branch C Coding Standard Version 1.0a
 *     cFE Flight Software Application Developers Guide
 *
 *  Notes:
 *
 */
#ifndef CFE_SRL_MSGSTRUCT_H
#define CFE_SRL_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/
#include "cfe_srl_msgdefs.h"
#include "cfe_msg_hdr.h"
#include "cfe_mission_cfg.h"
#include "cfe_srl_extern_typedefs.h"
#include "cfe_srl_mission_cfg.h"

/****************************
**  SRL Command Formats     **
*****************************/
typedef struct  {

    CFE_MSG_CommandHeader_t CommandHeader;

} CFE_SRL_NoopCmd_t;

typedef struct  {

    CFE_MSG_CommandHeader_t CommandHeader;

} CFE_SRL_ResetCounterCmd_t;

typedef struct  {

    CFE_MSG_CommandHeader_t CommandHeader;

} CFE_SRL_SendHkCmd_t;

typedef struct {

    CFE_MSG_CommandHeader_t CommandHeader;

    CFE_SRL_ResetHandleCounterCmd_Payload_t Payload;

} CFE_SRL_ResetHandleCounterCmd_t;

typedef struct {

    CFE_MSG_CommandHeader_t CommandHeader;
    
    CFE_SRL_GetHandleStatusCmd_Payload_t Payload;

} CFE_SRL_GetHandleStatusCmd_t;

typedef struct {

    CFE_MSG_CommandHeader_t CommandHeader;
    
    CFE_SRL_CloseHandleCmd_Payload_t Payload;

} CFE_SRL_CloseHandleCmd_t;


typedef struct {

    CFE_MSG_CommandHeader_t CommandHeader;
    
    CFE_SRL_HandleInitCmd_Payload_t Payload;

} CFE_SRL_InitHandleCmd_t;


/****************************
**  SB Telemetry Formats   **
*****************************/
typedef struct CFE_SRL_HousekeepingTlm {

    CFE_MSG_TelemetryHeader_t TelemetryHeader;

    CFE_SRL_HousekeepingTlm_Payload_t Payload;

} CFE_SRL_HousekeepingTlm_t;


#endif /* CFE_SRL_MSGSTRUCT_H */