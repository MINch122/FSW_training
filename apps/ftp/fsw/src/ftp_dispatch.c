/**
 * \file
 *   This file contains the source code for the FTP App.
 */

/*
** Include Files:
*/
#include "ftp_task.h"
#include "ftp_dispatch.h"
#include "ftp_cmds.h"
#include "ftp_eventids.h"
#include "ftp_msgids.h"
#include "ftp_msg.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool FTP_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength) {
    bool Result = true;
    size_t ActualLength = 0;
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    /**
     * Verify the command packet length
     */
    if (ExpectedLength != ActualLength) {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(FTP_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                            "Invalid Msg Length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                            (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode,
                            (unsigned int)ActualLength, (unsigned int)ExpectedLength);
        
        Result = false;

        FTP_Data.ErrCounter ++;
    }

    return Result;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SLT FTP ground commands                                                    */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void FTP_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr) {
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /**
     * Process ground commands
     */
    switch (CommandCode)
    {
    case FTP_NOOP_CC:
        if (FTP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(FTP_NoopCmd_t))) {
            FTP_NoopCmd((const FTP_NoopCmd_t *)SBBufPtr);
        }
        break;

    case FTP_RESET_COUNTER_CC:
        if (FTP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(FTP_ResetCountersCmd_t))) {
            FTP_ResetCountersCmd((const FTP_ResetCountersCmd_t *)SBBufPtr);
            break;
        }

    case FTP_SEND_FILE_CC:
        if (FTP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(FTP_SendFileCmd_t))) {
            FTP_SendFileCmd((const FTP_SendFileCmd_t *)SBBufPtr);
        }
        break;
    
    default:
        CFE_EVS_SendEvent(FTP_CC_ERR_EID, CFE_EVS_EventType_ERROR, "%s: Invalid ground command code - CC = %d",
                            __func__, CommandCode);
        break;
    }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the FTP       */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void FTP_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr) {
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
    case FTP_CMD_MID:
        FTP_ProcessGroundCommand(SBBufPtr);
        break;

    case FTP_SEND_HK_MID:
        FTP_SendHkCmd((const FTP_SendHkCmd_t *)SBBufPtr);
        break;

    case FTP_SEND_BCN_MID:
        FTP_SendBcnCmd((const FTP_SendBcnCmd_t *)SBBufPtr);
        break;

    default:
        CFE_EVS_SendEvent(FTP_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                            "FTPUZUC: Invalid command packet, MID = 0x%X",
                            (unsigned int)CFE_SB_MsgIdToValue(MsgId));
        break;
    }
}