/**
 * \file
 *   This file contains the source code for the PAY UZURO CAM App.
 */

/*
** Include Files:
*/
#include "payuzuc_task.h"
#include "payuzuc_dispatch.h"
#include "payuzuc_cmds.h"
#include "payuzuc_eventids.h"
#include "payuzuc_msgids.h"
#include "payuzuc_msg.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool PAYUZUC_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength) {
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

        CFE_EVS_SendEvent(PAYUZUC_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                            "Invalid Msg Length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                            (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode,
                            (unsigned int)ActualLength, (unsigned int)ExpectedLength);
        
        Result = false;

        PAYUZUC_Data.ErrCounter ++;

        /* RPT */
        PAYUZUC_ReportTlm_t *BufPtr = (PAYUZUC_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(PAYUZUC_ReportTlm_t));
        if (BufPtr == NULL) goto cleanup;

        if (CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader), CFE_SB_ValueToMsgId(PAYUZUC_REPORT_TLM_MID),
        sizeof(PAYUZUC_ReportTlm_t)) != CFE_SUCCESS) {
            CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
            goto cleanup;
        }
        BufPtr->Report.MsgID = (uint16_t)CFE_SB_MsgIdToValue(MsgId);
        BufPtr->Report.CommandCode = (uint8_t)FcnCode;
        BufPtr->Report.ReturnType = RPT_RETTYPE_APP;
        BufPtr->Report.ReturnCode = CFE_STATUS_WRONG_MSG_LENGTH; // Error code of `Length error`
        BufPtr->Report.ReturnDataSize = 2 * sizeof(uint32_t);
        
        uint32_t Temp32 = (uint32_t)ActualLength;
        memcpy(BufPtr->Report.ReturnValue, &Temp32, sizeof(uint32_t));
        Temp32 = (uint32_t)ExpectedLength;
        memcpy(BufPtr->Report.ReturnValue + sizeof(uint32_t), &Temp32, sizeof(uint32_t));

        CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
        if (CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true) != CFE_SUCCESS) {
            CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
            goto cleanup;
        }
        /* End of RPT */
    }
cleanup:
    return Result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUC ground commands                                                    */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void PAYUZUC_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr) {
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /**
     * Process ground commands
     */
    switch (CommandCode)
    {
    case PAYUZUC_NOOP_CC:
        if (PAYUZUC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUZUC_NoopCmd_t))) {
            PAYUZUC_NoopCmd((const PAYUZUC_NoopCmd_t *)SBBufPtr);
        }
        break;

    case PAYUZUC_RESET_COUNTERS_CC:
        if (PAYUZUC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUZUC_ResetCountersCmd_t))) {
            PAYUZUC_ResetCountersCmd((const PAYUZUC_ResetCountersCmd_t *)SBBufPtr);
        }
        break;

    case PAYUZUC_PING_CC:
        if (PAYUZUC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUZUC_PingCmd_t))) {
            PAYUZUC_PingCmd((const PAYUZUC_PingCmd_t *)SBBufPtr);
        }
        break;

    case PAYUZUC_SET_MODE_CC:
        if (PAYUZUC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUZUC_SetModeCmd_t))) {
            PAYUZUC_SetModeCmd((const PAYUZUC_SetModeCmd_t *)SBBufPtr);
        }
        break;

    case PAYUZUC_MEMORY_STATUS_CC:
        if (PAYUZUC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUZUC_MemoryStatusCmd_t))) {
            PAYUZUC_MemoryStatusCmd((const PAYUZUC_MemoryStatusCmd_t *)SBBufPtr);
        }
        break;

    case PAYUZUC_SET_EXPOSURE_CC:
        if (PAYUZUC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUZUC_SetExposureCmd_t))) {
            PAYUZUC_SetExposureCmd((const PAYUZUC_SetExposureCmd_t *)SBBufPtr);
        }
        break;

    case PAYUZUC_CAPTURE_CC:
        if (PAYUZUC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUZUC_CaptureCmd_t))) {
            PAYUZUC_CaptureCmd((const PAYUZUC_CaptureCmd_t *)SBBufPtr);
        }
        break;

    case PAYUZUC_DOWNLOAD_CC:
        if (PAYUZUC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUZUC_DownloadCmd_t))) {
            PAYUZUC_DownloadCmd((const PAYUZUC_DownloadCmd_t *)SBBufPtr);
        }
        break;

    case PAYUZUC_READ_REGISTER_CC:
        if (PAYUZUC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUZUC_ReadRegisterCmd_t))) {
            PAYUZUC_ReadRegisterCmd((const PAYUZUC_ReadRegisterCmd_t *)SBBufPtr);
        }
        break;

    case PAYUZUC_WRITE_REGISTER_CC:
        if (PAYUZUC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUZUC_WriteRegisterCmd_t))) {
            PAYUZUC_WriteRegisterCmd((const PAYUZUC_WriteRegisterCmd_t *)SBBufPtr);
        }
        break;
    case PAYUZUC_DOWNLOAD_ALL_CC:
        if (PAYUZUC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUZUC_DownloadAllCmd_t))) {
            PAYUZUC_DownloadAllCmd((const PAYUZUC_DownloadAllCmd_t *)SBBufPtr);
        }
        break;
    case PAYUZUC_MOSAIC_CC:
        if (PAYUZUC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUZUC_MosaicCmd_t))) {
            PAYUZUC_MosaicCmd((const PAYUZUC_MosaicCmd_t *)SBBufPtr);
        }
        break;

    default:
        CFE_EVS_SendEvent(PAYUZUC_CC_ERR_EID, CFE_EVS_EventType_ERROR, "%s: Invalid ground command code - CC = %d",
                            __func__, CommandCode);
        /* RPT */
        PAYUZUC_ReportTlm_t *BufPtr = (PAYUZUC_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(PAYUZUC_ReportTlm_t));
        if (BufPtr == NULL) break;
        if(CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader), CFE_SB_ValueToMsgId(PAYUZUC_REPORT_TLM_MID), sizeof(PAYUZUC_ReportTlm_t) != CFE_SUCCESS)) {
            CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
            break;
        }
        BufPtr->Report.MsgID = PAYUZUC_CMD_MID;
        BufPtr->Report.CommandCode = (uint8_t)CommandCode;
        BufPtr->Report.ReturnType = RPT_RETTYPE_APP;
        BufPtr->Report.ReturnCode = CFE_STATUS_BAD_COMMAND_CODE;
        BufPtr->Report.ReturnDataSize = 0;
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
        if(CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true) != CFE_SUCCESS) {
            CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
            break;
        }
        /* End of RPT */
        break;
    }
    
    return;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the PAYUZUC   */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void PAYUZUC_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr) {
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
    case PAYUZUC_CMD_MID:
        PAYUZUC_ProcessGroundCommand(SBBufPtr);
        break;
    
    case PAYUZUC_SEND_HK_MID:
        PAYUZUC_SendHkCmd((const PAYUZUC_SendHkCmd_t *)SBBufPtr);
        break;

    default:
        CFE_EVS_SendEvent(PAYUZUC_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                            "PAYUZUC: Invalid command packet, MID = 0x%X",
                            (unsigned int)CFE_SB_MsgIdToValue(MsgId));
        /* RPT */
        PAYUZUC_ReportTlm_t *BufPtr = (PAYUZUC_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(PAYUZUC_ReportTlm_t));
        if (BufPtr == NULL) break;
        if(CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader), CFE_SB_ValueToMsgId(PAYUZUC_REPORT_TLM_MID), sizeof(PAYUZUC_ReportTlm_t) != CFE_SUCCESS)) {
            CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
            break;
        }
        BufPtr->Report.MsgID = PAYUZUC_CMD_MID;
        BufPtr->Report.CommandCode = 0;
        BufPtr->Report.ReturnType = RPT_RETTYPE_APP;
        BufPtr->Report.ReturnCode = CFE_STATUS_UNKNOWN_MSG_ID;
        BufPtr->Report.ReturnDataSize = sizeof(CFE_SB_MsgId_Atom_t);
        memcpy(BufPtr->Report.ReturnValue, &MsgId.Value, sizeof(CFE_SB_MsgId_Atom_t));
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
        if(CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true) != CFE_SUCCESS) {
            CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
            break;
        }
        /* End of RPT */
        break;
    }
}