/**
 * \file
 *   This file contains the source code for the BATT App message dispatcher.
 */

/*
** Include Files:
*/
#include "batt_app.h"
#include "batt_dispatch.h"
#include "batt_cmds.h"
#include "batt_eventids.h"
#include "batt_msgids.h"
#include "batt_msg.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool BATT_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool              result       = true;
    size_t            ActualLength = 0;
    CFE_SB_MsgId_t    MsgId        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode      = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    /*
    ** Verify the command packet length.
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(BATT_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        BATT_Data.ErrCounter++;
    }

    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* BATT ground commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void BATT_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /*
    ** Process BATT app ground commands
    */
    switch (CommandCode)
    {
        case BATT_NOOP_CC:
            if (BATT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(BATT_NoopCmd_t)))
            {
                BATT_NoopCmd((const BATT_NoopCmd_t *)SBBufPtr);
            }
            break;

        case BATT_RESET_COUNTERS_CC:
            if (BATT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(BATT_ResetCountersCmd_t)))
            {
                BATT_ResetCountersCmd((const BATT_ResetCountersCmd_t *)SBBufPtr);
            }
            break;

        case BATT_GET_HK_CC:
            if (BATT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(BATT_GetHkCmd_t)))
            {
                BATT_GetHkCmd((const BATT_GetHkCmd_t *)SBBufPtr);
            }
            break;

        case BATT_SET_HEATER_CC:
            if (BATT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(BATT_SetHeaterCmd_t)))
            {
                BATT_SetHeaterCmd((const BATT_SetHeaterCmd_t *)SBBufPtr);
            }
            break;

        case BATT_RESET_FAULT_CC:
            if (BATT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(BATT_ResetFaultCmd_t)))
            {
                BATT_ResetFaultCmd((const BATT_ResetFaultCmd_t *)SBBufPtr);
            }
            break;

        default:
            CFE_EVS_SendEvent(BATT_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid ground command code: CC = %d",
                              CommandCode);
            break;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the BATT      */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void BATT_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case BATT_CMD_MID:
            BATT_ProcessGroundCommand(SBBufPtr);
            break;

        case BATT_SEND_HK_MID:
            BATT_SendHkCmd((const BATT_SendHkCmd_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(BATT_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "BATT: invalid command packet, MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
