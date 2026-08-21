/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 ************************************************************************/

#include "pay_slt_app.h"
#include "pay_slt_cmds.h"
#include "pay_slt_dispatch.h"
#include "pay_slt_eventids.h"
#include "pay_slt_msg.h"
#include "pay_slt_msgids.h"
#include "pay_slt_utils.h"

bool PAY_SLT_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    CFE_Status_t status;
    size_t actual_length = 0;
    CFE_SB_MsgId_t msg_id = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t fcn_code = 0;
    uint16 msg_id_value = PAY_SLT_CMD_MID;

    if (MsgPtr == NULL)
    {
        (void)PAY_SLT_HandleReport(CFE_ES_BAD_ARGUMENT, 0, false, NULL, 0);
        return false;
    }

    status = CFE_MSG_GetMsgId(MsgPtr, &msg_id);
    if (status != CFE_SUCCESS)
    {
        (void)PAY_SLT_HandleReport(status, 0, false, NULL, 0);
        return false;
    }
    msg_id_value = (uint16)CFE_SB_MsgIdToValue(msg_id);

    status = CFE_MSG_GetFcnCode(MsgPtr, &fcn_code);
    if (status != CFE_SUCCESS)
    {
        (void)PAY_SLT_HandleReportForMid(msg_id_value, status, 0, false, NULL, 0);
        return false;
    }

    status = CFE_MSG_GetSize(MsgPtr, &actual_length);
    if (status != CFE_SUCCESS)
    {
        (void)PAY_SLT_HandleReportForMid(msg_id_value, status, (uint8)fcn_code, false, NULL, 0);
        return false;
    }

    if (ExpectedLength != actual_length)
    {
        uint32 lengths[2] = {(uint32)actual_length, (uint32)ExpectedLength};

        CFE_EVS_SendEvent(PAY_SLT_DISPATCH_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X, CC = %u, Len = %u, Expected = %u",
                          (unsigned int)msg_id_value, (unsigned int)fcn_code,
                          (unsigned int)actual_length, (unsigned int)ExpectedLength);
        (void)PAY_SLT_HandleReportForMid(msg_id_value, CFE_STATUS_WRONG_MSG_LENGTH, (uint8)fcn_code, false,
                                         lengths, sizeof(lengths));
        return false;
    }

    if (msg_id_value == PAY_SLT_CMD_MID)
    {
        PAY_SLT_Data.CmdCounter++;
    }

    return true;
}

void PAY_SLT_ProcessCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_Status_t status;
    CFE_MSG_FcnCode_t command_code = 0;

    status = CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &command_code);
    if (status != CFE_SUCCESS)
    {
        (void)PAY_SLT_HandleReport(status, 0, false, NULL, 0);
        return;
    }

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
                status = PAY_SLT_SendBeaconCmd((const PAY_SLT_SendBcnCmd_t *)SBBufPtr);
                (void)PAY_SLT_HandleReport(status, PAY_SLT_REPORT_BCN_CC, status != CFE_SUCCESS,
                                           &PAY_SLT_Data.BcnEnabled, sizeof(PAY_SLT_Data.BcnEnabled));
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
        case PAY_SLT_PAR_SET_ARRAY_CC:
            if (PAY_SLT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_ParSetArrayCmd_t))) {
                PAY_SLT_ParSetArrayCmd((const PAY_SLT_ParSetArrayCmd_t *)SBBufPtr);
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
            (void)PAY_SLT_HandleReport(CFE_STATUS_BAD_COMMAND_CODE, (uint8)command_code, false, NULL, 0);
            break;
    }
}

void PAY_SLT_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_Status_t status;
    CFE_SB_MsgId_t msg_id = CFE_SB_INVALID_MSG_ID;
    uint16 msg_id_value;

    if (SBBufPtr == NULL)
    {
        (void)PAY_SLT_HandleReport(CFE_ES_BAD_ARGUMENT, 0, false, NULL, 0);
        return;
    }

    status = CFE_MSG_GetMsgId(&SBBufPtr->Msg, &msg_id);
    if (status != CFE_SUCCESS)
    {
        (void)PAY_SLT_HandleReport(status, 0, false, NULL, 0);
        return;
    }
    msg_id_value = (uint16)CFE_SB_MsgIdToValue(msg_id);

    switch (msg_id_value)
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
                              (unsigned int)msg_id_value);
            (void)PAY_SLT_HandleReportForMid(msg_id_value, CFE_STATUS_UNKNOWN_MSG_ID, 0, false,
                                             &msg_id_value, sizeof(msg_id_value));
            break;
    }
}
