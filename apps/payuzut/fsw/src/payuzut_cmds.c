/**
 * \file
 *   This file contains the source code for the PAY UZURO CAM Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "payuzut_task.h"
#include "payuzut_cmds.h"
#include "payuzut_msgids.h"
#include "payuzut_eventids.h"
#include "payuzut_msg.h"

#include "payuzut_internal_cfg.h"
#include "payuzut_interface_cfg.h"
#include "payuzut_utils.h"
#include "cfe.h"
#include <unistd.h>


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t PAYUZUT_SendHkCmd(const PAYUZUT_SendHkCmd_t *Msg) {
    PAYUZUT_Data.HkTlm.Payload.CommandCounter = PAYUZUT_Data.CmdCounter;
    PAYUZUT_Data.HkTlm.Payload.CommandErrorCounter = PAYUZUT_Data.ErrCounter;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(PAYUZUT_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(PAYUZUT_Data.HkTlm.TelemetryHeader), true);

    // CFE_EVS_SendEvent(PAYUZUT_SEND_HK_INF_EID, CFE_EVS_EventType_INFORMATION, "PAYUZUT Send HK Cmd Received.");

    return CFE_SUCCESS;
}

CFE_Status_t PAYUZUT_SendBcnCmd(const PAYUZUT_SendBcnCmd_t *Msg) {
    PAYUZUT_Data.BcnTlm.Payload.CommandCounter = PAYUZUT_Data.CmdCounter;
    PAYUZUT_Data.BcnTlm.Payload.CommandErrorCounter = PAYUZUT_Data.ErrCounter;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(PAYUZUT_Data.BcnTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(PAYUZUT_Data.BcnTlm.TelemetryHeader), true);

    // CFE_EVS_SendEvent(PAYUZUT_SEND_HK_INF_EID, CFE_EVS_EventType_INFORMATION, "PAYUZUT Send HK Cmd Received.");

    return CFE_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUT NOOP commands                                                      */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUT_NoopCmd(const PAYUZUT_NoopCmd_t *Msg) {
    PAYUZUT_Data.CmdCounter++;

    CFE_EVS_SendEvent(PAYUZUT_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "PAYUZUT Noop Command Received");

    return CFE_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t PAYUZUT_ResetCountersCmd(const PAYUZUT_ResetCountersCmd_t *Msg) {
    PAYUZUT_Data.CmdCounter = 0;
    PAYUZUT_Data.ErrCounter = 0;
    PAYUZUT_Data.DeviceErrCounter = 0;

    CFE_EVS_SendEvent(PAYUZUT_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "PAYUZUT Reset Counters Command Received");

    return CFE_SUCCESS;
}
