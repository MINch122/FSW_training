/**
 * \file
 *   This file contains the source code for the PAY UZURO CAM App.
 */

/*
** Include Files:
*/
#include "paybee_kisscam_task.h"
#include "paybee_kisscam_dispatch.h"
#include "paybee_kisscam_cmds.h"
#include "paybee_kisscam_eventids.h"
#include "paybee_kisscam_msgids.h"
#include "paybee_kisscam_msg.h"
#include "paybee_kisscam_utils.h"

#include <string.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool paybee_kisscam_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength) {
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

        CFE_EVS_SendEvent(paybee_kisscam_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                            "Invalid Msg Length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                            (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode,
                            (unsigned int)ActualLength, (unsigned int)ExpectedLength);
        
        Result = false;

        paybee_kisscam_Data.ErrCounter ++;

        uint32_t Lengths[2] = {(uint32_t)ActualLength, (uint32_t)ExpectedLength};
        paybee_kisscam_SendReport((uint8_t)FcnCode, RPT_RETTYPE_APP,
                                  CFE_STATUS_WRONG_MSG_LENGTH, Lengths, sizeof(Lengths));
    }
    return Result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* paybee_kisscam ground commands                                                    */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void paybee_kisscam_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr) {
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /**
     * Process ground commands
     */
    switch (CommandCode)
    {
    case paybee_kisscam_NOOP_CC:
        if (paybee_kisscam_VerifyCmdLength(&SBBufPtr->Msg, sizeof(paybee_kisscam_NoopCmd_t))) {
            paybee_kisscam_NoopCmd((const paybee_kisscam_NoopCmd_t *)SBBufPtr);
        }
        break;

    case paybee_kisscam_RESET_COUNTERS_CC:
        if (paybee_kisscam_VerifyCmdLength(&SBBufPtr->Msg, sizeof(paybee_kisscam_ResetCountersCmd_t))) {
            paybee_kisscam_ResetCountersCmd((const paybee_kisscam_ResetCountersCmd_t *)SBBufPtr);
        }
        break;

    case paybee_kisscam_PING_CC:
        if (paybee_kisscam_VerifyCmdLength(&SBBufPtr->Msg, sizeof(paybee_kisscam_PingCmd_t))) {
            paybee_kisscam_PingCmd((const paybee_kisscam_PingCmd_t *)SBBufPtr);
        }
        break;

    case paybee_kisscam_SET_MODE_CC:
        if (paybee_kisscam_VerifyCmdLength(&SBBufPtr->Msg, sizeof(paybee_kisscam_SetModeCmd_t))) {
            paybee_kisscam_SetModeCmd((const paybee_kisscam_SetModeCmd_t *)SBBufPtr);
        }
        break;

    case paybee_kisscam_MEMORY_STATUS_CC:
        if (paybee_kisscam_VerifyCmdLength(&SBBufPtr->Msg, sizeof(paybee_kisscam_MemoryStatusCmd_t))) {
            paybee_kisscam_MemoryStatusCmd((const paybee_kisscam_MemoryStatusCmd_t *)SBBufPtr);
        }
        break;

    case paybee_kisscam_SET_EXPOSURE_CC:
        if (paybee_kisscam_VerifyCmdLength(&SBBufPtr->Msg, sizeof(paybee_kisscam_SetExposureCmd_t))) {
            paybee_kisscam_SetExposureCmd((const paybee_kisscam_SetExposureCmd_t *)SBBufPtr);
        }
        break;

    case paybee_kisscam_CAPTURE_CC:
        if (paybee_kisscam_VerifyCmdLength(&SBBufPtr->Msg, sizeof(paybee_kisscam_CaptureCmd_t))) {
            paybee_kisscam_CaptureCmd((const paybee_kisscam_CaptureCmd_t *)SBBufPtr);
        }
        break;

    case paybee_kisscam_DOWNLOAD_CC:
        if (paybee_kisscam_VerifyCmdLength(&SBBufPtr->Msg, sizeof(paybee_kisscam_DownloadCmd_t))) {
            paybee_kisscam_DownloadCmd((const paybee_kisscam_DownloadCmd_t *)SBBufPtr);
        }
        break;

    case paybee_kisscam_READ_REGISTER_CC:
        if (paybee_kisscam_VerifyCmdLength(&SBBufPtr->Msg, sizeof(paybee_kisscam_ReadRegisterCmd_t))) {
            paybee_kisscam_ReadRegisterCmd((const paybee_kisscam_ReadRegisterCmd_t *)SBBufPtr);
        }
        break;

    case paybee_kisscam_WRITE_REGISTER_CC:
        if (paybee_kisscam_VerifyCmdLength(&SBBufPtr->Msg, sizeof(paybee_kisscam_WriteRegisterCmd_t))) {
            paybee_kisscam_WriteRegisterCmd((const paybee_kisscam_WriteRegisterCmd_t *)SBBufPtr);
        }
        break;
    case paybee_kisscam_DOWNLOAD_ALL_CC:
        if (paybee_kisscam_VerifyCmdLength(&SBBufPtr->Msg, sizeof(paybee_kisscam_DownloadAllCmd_t))) {
            paybee_kisscam_DownloadAllCmd((const paybee_kisscam_DownloadAllCmd_t *)SBBufPtr);
        }
        break;
    case paybee_kisscam_IMAGE_COMPRESS_CC:
        if (paybee_kisscam_VerifyCmdLength(&SBBufPtr->Msg, sizeof(paybee_kisscam_ImageCompressCmd_t))) {
            paybee_kisscam_ImageCompressCmd((const paybee_kisscam_ImageCompressCmd_t *)SBBufPtr);
        }
        break;
    // case paybee_kisscam_MOSAIC_CC:
    //     if (paybee_kisscam_VerifyCmdLength(&SBBufPtr->Msg, sizeof(paybee_kisscam_MosaicCmd_t))) {
    //         paybee_kisscam_MosaicCmd((const paybee_kisscam_MosaicCmd_t *)SBBufPtr);
    //     }
    //     break;

    // case paybee_kisscam_DOWNLOAD_ALL_CHILD_CC:
    //     if (paybee_kisscam_VerifyCmdLength(&SBBufPtr->Msg, sizeof(paybee_kisscam_DownloadAllCmd_t))) {
    //         paybee_kisscam_DownloadAll2Cmd((const paybee_kisscam_DownloadAllCmd_t *)SBBufPtr);
    //     }
    //     break;

    default:
        CFE_EVS_SendEvent(paybee_kisscam_CC_ERR_EID, CFE_EVS_EventType_ERROR, "%s: Invalid ground command code - CC = %d",
                            __func__, CommandCode);
        paybee_kisscam_Data.ErrCounter++;
        paybee_kisscam_SendReport((uint8_t)CommandCode, RPT_RETTYPE_APP,
                                  CFE_STATUS_BAD_COMMAND_CODE, NULL, 0);
        break;
    }
    
    return;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the paybee_kisscam   */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void paybee_kisscam_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr) {
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
    case paybee_kisscam_CMD_MID:
        paybee_kisscam_ProcessGroundCommand(SBBufPtr);
        break;
    
    // case paybee_kisscam_SEND_HK_MID:
    //     paybee_kisscam_SendHkCmd((const paybee_kisscam_SendHkCmd_t *)SBBufPtr);
    //     break;

    // case paybee_kisscam_SEND_BCN_MID:
    //     paybee_kisscam_SendBcnCmd((const paybee_kisscam_SendBcnCmd_t *)SBBufPtr);
    //     break;

    default:
        CFE_EVS_SendEvent(paybee_kisscam_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                            "paybee_kisscam: Invalid command packet, MID = 0x%X",
                            (unsigned int)CFE_SB_MsgIdToValue(MsgId));
        paybee_kisscam_Data.ErrCounter++;
        paybee_kisscam_SendReport(0, RPT_RETTYPE_APP, CFE_STATUS_UNKNOWN_MSG_ID,
                                  &MsgId.Value, sizeof(MsgId.Value));
        break;
    }
}
