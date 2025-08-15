/**
 * \file
 *   This file contains the source code for the PAY UZURO CAM App.
 */

/*
** Include Files:
*/
#include "payuzut_task.h"
#include "payuzut_dispatch.h"
#include "payuzut_cmds.h"
#include "payuzut_eventids.h"
#include "payuzut_msgids.h"
#include "payuzut_msg.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool PAYUZUT_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength) {
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

        CFE_EVS_SendEvent(PAYUZUT_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                            "Invalid Msg Length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                            (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode,
                            (unsigned int)ActualLength, (unsigned int)ExpectedLength);
        
        Result = false;

        PAYUZUT_Data.ErrCounter ++;

        /* RPT */
        PAYUZUT_ReportTlm_t *BufPtr = (PAYUZUT_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(PAYUZUT_ReportTlm_t));
        if (BufPtr == NULL) goto cleanup;

        if (CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader), CFE_SB_ValueToMsgId(PAYUZUT_REPORT_TLM_MID),
        sizeof(PAYUZUT_ReportTlm_t)) != CFE_SUCCESS) {
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
/* PAYUZUT ground commands                                                    */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void PAYUZUT_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr) {
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /**
     * Process ground commands
     */
    switch (CommandCode)
    {
    case PAYUZUT_NOOP_CC:
        if (PAYUZUT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUZUT_NoopCmd_t))) {
            PAYUZUT_NoopCmd((const PAYUZUT_NoopCmd_t *)SBBufPtr);
        }
        break;

    case PAYUZUT_RESET_COUNTERS_CC:
        if (PAYUZUT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUZUT_ResetCountersCmd_t))) {
            PAYUZUT_ResetCountersCmd((const PAYUZUT_ResetCountersCmd_t *)SBBufPtr);
        }
        break;

    case PAYUZUT_GET_TEMP_CC:
        if (PAYUZUT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUZUT_GetTempCmd_t))) {
            PAYUZUT_GetTempCmd((const PAYUZUT_GetTempCmd_t *)SBBufPtr);
        }
        break;
    
    case PAYUZUT_THRUSTER_ON_CC:
        if (PAYUZUT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUZUT_ThrusterOnCmd_t))) {
            PAYUZUT_ThrusterOnCmd((const PAYUZUT_ThrusterOnCmd_t *)SBBufPtr);
        }
        break;

    case PAYUZUT_THRUSTER_OFF_CC:
        if (PAYUZUT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUZUT_ThrusterOffCmd_t))) {
            PAYUZUT_ThrusterOffCmd((const PAYUZUT_ThrusterOffCmd_t *)SBBufPtr);
        }
        break;

    case PAYUZUT_CUMULATE_TEMP_CC:
        if (PAYUZUT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUZUT_CumulateTempCmd_t))) {
            PAYUZUT_CumulateTempCmd((const PAYUZUT_CumulateTempCmd_t *)SBBufPtr);
        }
        break;

    default:
        CFE_EVS_SendEvent(PAYUZUT_CC_ERR_EID, CFE_EVS_EventType_ERROR, "%s: Invalid ground command code - CC = %d",
                            __func__, CommandCode);
        /* RPT */
        PAYUZUT_ReportTlm_t *BufPtr = (PAYUZUT_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(PAYUZUT_ReportTlm_t));
        if (BufPtr == NULL) break;
        if(CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader), CFE_SB_ValueToMsgId(PAYUZUT_REPORT_TLM_MID), sizeof(PAYUZUT_ReportTlm_t) != CFE_SUCCESS)) {
            CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
            break;
        }
        BufPtr->Report.MsgID = PAYUZUT_CMD_MID;
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
/*     This routine will process any packet that is received on the PAYUZUT   */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void PAYUZUT_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr) {
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
    case PAYUZUT_CMD_MID:
        PAYUZUT_ProcessGroundCommand(SBBufPtr);
        break;
    
    case PAYUZUT_SEND_HK_MID:
        PAYUZUT_SendHkCmd((const PAYUZUT_SendHkCmd_t *)SBBufPtr);
        break;
    
    case PAYUZUT_SEND_BCN_MID:
        PAYUZUT_SendBcnCmd((const PAYUZUT_SendBcnCmd_t *)SBBufPtr);
        break;

    default:
        CFE_EVS_SendEvent(PAYUZUT_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                            "PAYUZUT: Invalid command packet, MID = 0x%X",
                            (unsigned int)CFE_SB_MsgIdToValue(MsgId));

        /* RPT */
        PAYUZUT_ReportTlm_t *BufPtr = (PAYUZUT_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(PAYUZUT_ReportTlm_t));
        if (BufPtr == NULL) break;
        if(CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader), CFE_SB_ValueToMsgId(PAYUZUT_REPORT_TLM_MID), sizeof(PAYUZUT_ReportTlm_t) != CFE_SUCCESS)) {
            CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
            break;
        }
        BufPtr->Report.MsgID = PAYUZUT_CMD_MID;
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