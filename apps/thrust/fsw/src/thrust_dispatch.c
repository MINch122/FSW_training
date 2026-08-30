/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as "core Flight System: Bootes"
 *
 * Licensed under the Apache License, Version 2.0
 ************************************************************************/

/**
 * \file
 *   This file contains the source code for the Thrust App command dispatch.
 */

/*
** Include Files:
*/
#include "cfe.h"
#include "thrust_app.h"
#include "thrust_cmds.h"
#include "thrust_dispatch.h"
#include "thrust_eventids.h"
#include "thrust_msg.h"
#include "thrust_msgids.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* THRUST task pipe                                                           */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void THRUST_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case THRUST_CMD_MID:
            THRUST_ProcessGroundCommand(SBBufPtr);
            break;

        case THRUST_SEND_HK_MID:
            if (THRUST_VerifyCmdLength(&SBBufPtr->Msg, sizeof(THRUST_SendHkCmd_t)))
            {
                THRUST_SendScheduledHkCmd((const THRUST_SendHkCmd_t *)SBBufPtr);
            }
            break;

        default:
            CFE_EVS_SendEvent(THRUST_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "THRUST: Invalid MID=0x%X", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            THRUST_AppData.ErrCounter++;
            THRUST_HandleReportForMid((uint16_t)CFE_SB_MsgIdToValue(MsgId), CFE_STATUS_UNKNOWN_MSG_ID, 0,
                                      &MsgId.Value, sizeof(MsgId.Value));
            break;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* THRUST ground commands                                                     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void THRUST_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    switch (CommandCode)
    {
        case THRUST_NOOP_CC:
            if (THRUST_VerifyCmdLength(&SBBufPtr->Msg, sizeof(THRUST_NoopCmd_t)))
            {
                THRUST_NoopCmd((const THRUST_NoopCmd_t *)SBBufPtr);
            }
            break;

        case THRUST_RESET_COUNTERS_CC:
            if (THRUST_VerifyCmdLength(&SBBufPtr->Msg, sizeof(THRUST_ResetCountersCmd_t)))
            {
                THRUST_ResetCountersCmd((const THRUST_ResetCountersCmd_t *)SBBufPtr);
            }
            break;

        /* iG4U ICD commands */
        case THRUST_PING_CC:           /* MsgID=1 */
            if (THRUST_VerifyCmdLength(&SBBufPtr->Msg, sizeof(THRUST_PingCmd_t)))
            {
                THRUST_PingCmd((const THRUST_PingCmd_t *)SBBufPtr);
            }
            break;

        case THRUST_RESET_MODULE_CC:   /* MsgID=2 */
            if (THRUST_VerifyCmdLength(&SBBufPtr->Msg, sizeof(THRUST_ResetModuleCmd_t)))
            {
                THRUST_ResetModuleCmd((const THRUST_ResetModuleCmd_t *)SBBufPtr);
            }
            break;

        case THRUST_SET_MODE_CC:       /* MsgID=3 */
            if (THRUST_VerifyCmdLength(&SBBufPtr->Msg, sizeof(THRUST_SetModeCmd_t)))
            {
                THRUST_SetModeCmd((const THRUST_SetModeCmd_t *)SBBufPtr);
            }
            break;

        case THRUST_ARM_CC:            /* MsgID=4 */
            if (THRUST_VerifyCmdLength(&SBBufPtr->Msg, sizeof(THRUST_ArmCmd_t)))
            {
                THRUST_ArmCmd((const THRUST_ArmCmd_t *)SBBufPtr);
            }
            break;

        case THRUST_DISARM_CC:         /* MsgID=5 */
            if (THRUST_VerifyCmdLength(&SBBufPtr->Msg, sizeof(THRUST_DisarmCmd_t)))
            {
                THRUST_DisarmCmd((const THRUST_DisarmCmd_t *)SBBufPtr);
            }
            break;

        case THRUST_REQ_HK_CC:         /* MsgID=10 */
            if (THRUST_VerifyCmdLength(&SBBufPtr->Msg, sizeof(THRUST_SendHkCmd_t)))
            {
                THRUST_SendHkCmd((const THRUST_SendHkCmd_t *)SBBufPtr);
            }
            break;

        case THRUST_SCH_HK_ENABLE_CC:
            if (THRUST_VerifyCmdLength(&SBBufPtr->Msg, sizeof(THRUST_ScheduledHkEnableCmd_t)))
            {
                THRUST_ScheduledHkEnableCmd((const THRUST_ScheduledHkEnableCmd_t *)SBBufPtr);
            }
            break;

        case THRUST_REQ_STATUS_CC:     /* MsgID=11 */
            if (THRUST_VerifyCmdLength(&SBBufPtr->Msg, sizeof(THRUST_ReqStatusCmd_t)))
            {
                THRUST_ReqStatusCmd((const THRUST_ReqStatusCmd_t *)SBBufPtr);
            }
            break;

        case THRUST_MAIN_FIRE_CC:      /* MsgID=20 */
            if (THRUST_VerifyCmdLength(&SBBufPtr->Msg, sizeof(THRUST_MainFireCmd_t)))
            {
                THRUST_MainThrusterFireCmd((const THRUST_MainFireCmd_t *)SBBufPtr);
            }
            break;

        case THRUST_MAIN_ABORT_CC:     /* MsgID=21 */
            if (THRUST_VerifyCmdLength(&SBBufPtr->Msg, sizeof(THRUST_MainAbortCmd_t)))
            {
                THRUST_MainAbortCmd((const THRUST_MainAbortCmd_t *)SBBufPtr);
            }
            break;

        case THRUST_CG_PULSE_CC:       /* MsgID=30 */
            if (THRUST_VerifyCmdLength(&SBBufPtr->Msg, sizeof(THRUST_CGPulseCmd_t)))
            {
                THRUST_CGPulseCmd((const THRUST_CGPulseCmd_t *)SBBufPtr);
            }
            break;

        case THRUST_CG_ABORT_CC:       /* MsgID=31 */
            if (THRUST_VerifyCmdLength(&SBBufPtr->Msg, sizeof(THRUST_CGAbortCmd_t)))
            {
                THRUST_CGAbortCmd((const THRUST_CGAbortCmd_t *)SBBufPtr);
            }
            break;

        case THRUST_REQ_FAULT_LOG_CC:  /* MsgID=40 */
            if (THRUST_VerifyCmdLength(&SBBufPtr->Msg, sizeof(THRUST_ReqFaultLogCmd_t)))
            {
                THRUST_ReqFaultLogCmd((const THRUST_ReqFaultLogCmd_t *)SBBufPtr);
            }
            break;

        case THRUST_CLEAR_FAULT_CC:    /* MsgID=41 */
            if (THRUST_VerifyCmdLength(&SBBufPtr->Msg, sizeof(THRUST_ClearFaultCmd_t)))
            {
                THRUST_ClearFaultCmd((const THRUST_ClearFaultCmd_t *)SBBufPtr);
            }
            break;

        default:
            CFE_EVS_SendEvent(THRUST_CC_ERR_EID, CFE_EVS_EventType_ERROR, "THRUST: Invalid CC=%u",
                              (unsigned int)CommandCode);
            THRUST_AppData.ErrCounter++;
            THRUST_HandleReport(CFE_STATUS_BAD_COMMAND_CODE, (uint8_t)CommandCode, &CommandCode,
                                sizeof(CommandCode));
            break;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool THRUST_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool              result       = true;
    size_t            ActualLength = 0;
    CFE_SB_MsgId_t    MsgId        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode      = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    if (ExpectedLength != ActualLength)
    {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(THRUST_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X, CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode,
                          (unsigned int)ActualLength, (unsigned int)ExpectedLength);

        result = false;
        THRUST_AppData.ErrCounter++;

        {
            uint32_t Lengths[2] = {(uint32_t)ActualLength, (uint32_t)ExpectedLength};
            THRUST_HandleReportForMid((uint16_t)CFE_SB_MsgIdToValue(MsgId), CFE_STATUS_WRONG_MSG_LENGTH,
                                      (uint8_t)FcnCode, Lengths, sizeof(Lengths));
        }
    }

    return result;
}
