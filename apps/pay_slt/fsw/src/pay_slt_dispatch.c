/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 ************************************************************************/

#include "pay_slt_app.h"
#include "pay_slt_cmds.h"
#include "pay_slt_dispatch.h"
#include "pay_slt_eventids.h"
#include "pay_slt_msg.h"
#include "pay_slt_msgids.h"

bool PAY_SLT_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
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
        CFE_EVS_SendEvent(PAY_SLT_DISPATCH_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X, CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(msg_id), (unsigned int)fcn_code,
                          (unsigned int)actual_length, (unsigned int)ExpectedLength);
        PAY_SLT_Data.ErrCounter++;
        PAY_SLT_Data.AppErrCounter++;
        result = false;
    }

    return result;
}

void PAY_SLT_ProcessCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t command_code = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &command_code);

    switch (command_code)
    {
        case PAY_SLT_NOOP_CC:
            if (PAY_SLT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_NoopCmd_t)))
            {
                PAY_SLT_NoopCmd((const PAY_SLT_NoopCmd_t *)SBBufPtr);
            }
            break;
        case PAY_SLT_RESET_COUNTERS_CC:
            if (PAY_SLT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_ResetCountersCmd_t)))
            {
                PAY_SLT_ResetCountersCmd((const PAY_SLT_ResetCountersCmd_t *)SBBufPtr);
            }
            break;
        case PAY_SLT_REPORT_BCN_CC:
            if (PAY_SLT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_SendBcnCmd_t)))
            {
                PAY_SLT_SendBeaconCmd((const PAY_SLT_SendBcnCmd_t *)SBBufPtr);
            }
            break;
        case PAY_SLT_OUTPUT_ENABLED_CC:
            if (PAY_SLT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_OutputEnabledCmd_t)))
            {
                PAY_SLT_OutputEnabledCmd((const PAY_SLT_OutputEnabledCmd_t *)SBBufPtr);
            }
            break;

            
        case PAY_SLT_RS422_PING_CC:
            if (PAY_SLT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_RS422PingCmd_t)))
            {
                PAY_SLT_RS422PingCmd((const PAY_SLT_RS422PingCmd_t *)SBBufPtr);
            }
            break;
        case PAY_SLT_PAR_GET_CC:
            if (PAY_SLT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_ParGetCmd_t))) {
                PAY_SLT_ParGetCmd((const PAY_SLT_ParGetCmd_t *)SBBufPtr);
            }
            break;
        case PAY_SLT_PAR_SET_CC:
            if (PAY_SLT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_ParSetCmd_t))) {
                PAY_SLT_ParSetCmd((const PAY_SLT_ParSetCmd_t *)SBBufPtr);
            }
            break;
        case PAY_SLT_SCAN_FILES_CC:
            if (PAY_SLT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_ScanFilesCmd_t))) {
                PAY_SLT_ScanFilesCmd((const PAY_SLT_ScanFilesCmd_t *)SBBufPtr);
            }
            break;
        case PAY_SLT_DOWNLOAD_FILE_I2C_CC:
            if (PAY_SLT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_DownloadFileCmd_t))) {
                PAY_SLT_DownloadFileI2CCmd((const PAY_SLT_DownloadFileCmd_t *)SBBufPtr);
            }
            break;
        case PAY_SLT_DOWNLOAD_FILE_RS422_CC:
            if (PAY_SLT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_DownloadFileCmd_t))) {
                PAY_SLT_DownloadFileRS422Cmd((const PAY_SLT_DownloadFileCmd_t *)SBBufPtr);
            }
            break;
        case PAY_SLT_GET_FULL_TABLE_CC:
            if (PAY_SLT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_GetFullTableCmd_t))) {
                PAY_SLT_GetFullTableCmd((const PAY_SLT_GetFullTableCmd_t *)SBBufPtr);
            }
            break;
        

        default:
            CFE_EVS_SendEvent(PAY_SLT_DISPATCH_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAY_SLT: invalid command code, CC = %u", (unsigned int)command_code);
            PAY_SLT_Data.ErrCounter++;
            PAY_SLT_Data.AppErrCounter++;
            break;
    }
}

void PAY_SLT_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t msg_id = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &msg_id);

    switch (CFE_SB_MsgIdToValue(msg_id))
    {
        case PAY_SLT_CMD_MID:
            PAY_SLT_ProcessCommand(SBBufPtr);
            break;
        case PAY_SLT_SEND_HK_MID:
            if (PAY_SLT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_SendHkCmd_t)))
            {
                PAY_SLT_SendHkCmd((const PAY_SLT_SendHkCmd_t *)SBBufPtr);
            }
            break;
        case PAY_SLT_SEND_BCN_MID:
            if (PAY_SLT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_SendBcnCmd_t)))
            {
                PAY_SLT_SendBeaconCmd((const PAY_SLT_SendBcnCmd_t *)SBBufPtr);
            }
            break;
        default:
            CFE_EVS_SendEvent(PAY_SLT_DISPATCH_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAY_SLT: invalid command packet, MID = 0x%x",
                              (unsigned int)CFE_SB_MsgIdToValue(msg_id));
            break;
    }
}
