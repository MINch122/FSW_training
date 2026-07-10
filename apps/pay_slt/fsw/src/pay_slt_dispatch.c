/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 ************************************************************************/

#include "pay_slt_app.h"
#include "pay_slt_cmds.h"
#include "pay_slt_dispatch.h"
#include "pay_slt_eventids.h"
#include "pay_slt_msg.h"
#include "pay_slt_msgids.h"

bool SLT_IFB_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool result = true;
    size_t actual_length = 0;
    CFE_SB_MsgId_t msg_id = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t fcn_code = 0;

    CFE_MSG_GetSize(MsgPtr, &actual_length);
    if (ExpectedLength != actual_length)
    {
        CFE_MSG_GetMsgId(MsgPtr, &msg_id);
        CFE_MSG_GetFcnCode(MsgPtr, &fcn_code);
        CFE_EVS_SendEvent(SLT_IFB_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X, CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(msg_id), (unsigned int)fcn_code,
                          (unsigned int)actual_length, (unsigned int)ExpectedLength);
        SLT_IFB_Data.ErrCounter++;
        SLT_IFB_Data.AppErrCounter++;
        result = false;
    }

    return result;
}

void SLT_IFB_ProcessCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t command_code = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &command_code);

    switch (command_code)
    {
        case PAY_SLT_NOOP_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_NoopCmd_t)))
            {
                SLT_IFB_NoopCmd((const SLT_IFB_NoopCmd_t *)SBBufPtr);
            }
            break;
        case PAY_SLT_RESET_COUNTERS_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_ResetCountersCmd_t)))
            {
                SLT_IFB_ResetCountersCmd((const SLT_IFB_ResetCountersCmd_t *)SBBufPtr);
            }
            break;
        case PAY_SLT_REPORT_BCN_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_SendBcnCmd_t)))
            {
                PAY_SLT_SendBeaconCmd((const SLT_IFB_SendBcnCmd_t *)SBBufPtr);
            }
            break;
        case PAY_SLT_SET_BCN_ENABLED_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_SetBcnEnabledCmd_t)))
            {
                PAY_SLT_SetBcnEnabledCmd((const PAY_SLT_SetBcnEnabledCmd_t *)SBBufPtr);
            }
            break;

        case PAY_SLT_PAR_GET_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_ParGetCmd_t))) {
                PAY_SLT_ParGetCmd((const PAY_SLT_ParGetCmd_t *)SBBufPtr);
            }
            break;
        case PAY_SLT_PAR_SET_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_ParSetCmd_t))) {
                PAY_SLT_ParSetCmd((const PAY_SLT_ParSetCmd_t *)SBBufPtr);
            }
            break;
        case PAY_SLT_SCAN_FILES_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_ScanFilesCmd_t))) {
                PAY_SLT_ScanFilesCmd((const PAY_SLT_ScanFilesCmd_t *)SBBufPtr);
            }
            break;
        case PAY_SLT_DOWNLOAD_FILE_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_DownloadFileCmd_t))) {
                PAY_SLT_DownloadFileCmd((const PAY_SLT_DownloadFileCmd_t *)SBBufPtr);
            }
            break;
        case PAY_SLT_GET_FULL_TABLE_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_GetFullTableCmd_t))) {
                PAY_SLT_GetFullTableCmd((const PAY_SLT_GetFullTableCmd_t *)SBBufPtr);
            }
            break;
            
        default:
            CFE_EVS_SendEvent(SLT_IFB_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAY_SLT: invalid command code, CC = %u", (unsigned int)command_code);
            SLT_IFB_Data.ErrCounter++;
            SLT_IFB_Data.AppErrCounter++;
            break;
    }
}

void SLT_IFB_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t msg_id = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &msg_id);

    switch (CFE_SB_MsgIdToValue(msg_id))
    {
        case PAY_SLT_CMD_MID:
            SLT_IFB_ProcessCommand(SBBufPtr);
            break;
        case PAY_SLT_SEND_HK_MID:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_SendHkCmd_t)))
            {
                SLT_IFB_SendHkCmd((const SLT_IFB_SendHkCmd_t *)SBBufPtr);
            }
            break;
        case PAY_SLT_SEND_BCN_MID:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_SendBcnCmd_t)))
            {
                PAY_SLT_SendBeaconCmd((const SLT_IFB_SendBcnCmd_t *)SBBufPtr);
            }
            break;
        default:
            CFE_EVS_SendEvent(SLT_IFB_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAY_SLT: invalid command packet, MID = 0x%x",
                              (unsigned int)CFE_SB_MsgIdToValue(msg_id));
            break;
    }
}
