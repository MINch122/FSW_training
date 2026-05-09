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
        case PAY_SLT_IFB_CSP_CMP_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_CSP_CMP_Cmd_t))) { PAY_SLT_IFB_CSP_CMP_Cmd((const PAY_SLT_IFB_CSP_CMP_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_CSP_PING_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_CSP_PING_Cmd_t))) { PAY_SLT_IFB_CSP_PING_Cmd((const PAY_SLT_IFB_CSP_PING_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_CSP_PS_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_CSP_PS_Cmd_t))) { PAY_SLT_IFB_CSP_PS_Cmd((const PAY_SLT_IFB_CSP_PS_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_CSP_MEM_FREE_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_CSP_MEM_FREE_Cmd_t))) { PAY_SLT_IFB_CSP_MEM_FREE_Cmd((const PAY_SLT_IFB_CSP_MEM_FREE_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_CSP_REBOOT_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_CSP_REBOOT_Cmd_t))) { PAY_SLT_IFB_CSP_REBOOT_Cmd((const PAY_SLT_IFB_CSP_REBOOT_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_CSP_BUF_FREE_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_CSP_BUF_FREE_Cmd_t))) { PAY_SLT_IFB_CSP_BUF_FREE_Cmd((const PAY_SLT_IFB_CSP_BUF_FREE_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_CSP_UPTIME_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_CSP_UPTIME_Cmd_t))) { PAY_SLT_IFB_CSP_UPTIME_Cmd((const PAY_SLT_IFB_CSP_UPTIME_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_CSP_GNDWDT_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_CSP_GNDWDT_Cmd_t))) { PAY_SLT_IFB_CSP_GNDWDT_Cmd((const PAY_SLT_IFB_CSP_GNDWDT_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_GET_BRD_UID_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_GET_BRD_UID_Cmd_t))) { PAY_SLT_IFB_GET_BRD_UID_Cmd((const PAY_SLT_IFB_GET_BRD_UID_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_GET_BRD_REV_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_GET_BRD_REV_Cmd_t))) { PAY_SLT_IFB_GET_BRD_REV_Cmd((const PAY_SLT_IFB_GET_BRD_REV_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_GET_CSP_ADDR_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_GET_CSP_ADDR_Cmd_t))) { PAY_SLT_IFB_GET_CSP_ADDR_Cmd((const PAY_SLT_IFB_GET_CSP_ADDR_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_GET_CAN_SPEED_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_GET_CAN_SPEED_Cmd_t))) { PAY_SLT_IFB_GET_CAN_SPEED_Cmd((const PAY_SLT_IFB_GET_CAN_SPEED_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_GET_I2C_ADDR_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_GET_I2C_ADDR_Cmd_t))) { PAY_SLT_IFB_GET_I2C_ADDR_Cmd((const PAY_SLT_IFB_GET_I2C_ADDR_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_GET_I2C_SPEED_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_GET_I2C_SPEED_Cmd_t))) { PAY_SLT_IFB_GET_I2C_SPEED_Cmd((const PAY_SLT_IFB_GET_I2C_SPEED_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_GET_WDT_VAL_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_GET_WDT_VAL_Cmd_t))) { PAY_SLT_IFB_GET_WDT_VAL_Cmd((const PAY_SLT_IFB_GET_WDT_VAL_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_GET_CSP_RTABLE_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_GET_CSP_RTABLE_Cmd_t))) { PAY_SLT_IFB_GET_CSP_RTABLE_Cmd((const PAY_SLT_IFB_GET_CSP_RTABLE_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_GET_SYS_STATUS_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_GET_SYS_STATUS_Cmd_t))) { PAY_SLT_IFB_GET_SYS_STATUS_Cmd((const PAY_SLT_IFB_GET_SYS_STATUS_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_GET_SYS_UPTIME_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_GET_SYS_UPTIME_Cmd_t))) { PAY_SLT_IFB_GET_SYS_UPTIME_Cmd((const PAY_SLT_IFB_GET_SYS_UPTIME_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_GET_BOOT_CNT_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_GET_BOOT_CNT_Cmd_t))) { PAY_SLT_IFB_GET_BOOT_CNT_Cmd((const PAY_SLT_IFB_GET_BOOT_CNT_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_GET_BOOT_CAUSE_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_GET_BOOT_CAUSE_Cmd_t))) { PAY_SLT_IFB_GET_BOOT_CAUSE_Cmd((const PAY_SLT_IFB_GET_BOOT_CAUSE_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_GET_REBOOT_CAUSE_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_GET_REBOOT_CAUSE_Cmd_t))) { PAY_SLT_IFB_GET_REBOOT_CAUSE_Cmd((const PAY_SLT_IFB_GET_REBOOT_CAUSE_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_GET_WDT_LEFT_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_GET_WDT_LEFT_Cmd_t))) { PAY_SLT_IFB_GET_WDT_LEFT_Cmd((const PAY_SLT_IFB_GET_WDT_LEFT_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_GET_BRD_TEMP_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_GET_BRD_TEMP_Cmd_t))) { PAY_SLT_IFB_GET_BRD_TEMP_Cmd((const PAY_SLT_IFB_GET_BRD_TEMP_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_GET_PWR_CURRENT_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_GET_PWR_CURRENT_Cmd_t))) { PAY_SLT_IFB_GET_PWR_CURRENT_Cmd((const PAY_SLT_IFB_GET_PWR_CURRENT_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_GET_IMU_DATA_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_GET_IMU_DATA_Cmd_t))) { PAY_SLT_IFB_GET_IMU_DATA_Cmd((const PAY_SLT_IFB_GET_IMU_DATA_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_GET_NTC_DATA_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_GET_NTC_DATA_Cmd_t))) { PAY_SLT_IFB_GET_NTC_DATA_Cmd((const PAY_SLT_IFB_GET_NTC_DATA_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_SAVE_TABLE0_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_SAVE_TABLE0_Cmd_t))) { PAY_SLT_IFB_SAVE_TABLE0_Cmd((const PAY_SLT_IFB_SAVE_TABLE0_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_SAVE_TABLE1_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_SAVE_TABLE1_Cmd_t))) { PAY_SLT_IFB_SAVE_TABLE1_Cmd((const PAY_SLT_IFB_SAVE_TABLE1_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_SAVE_TABLE4_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_SAVE_TABLE4_Cmd_t))) { PAY_SLT_IFB_SAVE_TABLE4_Cmd((const PAY_SLT_IFB_SAVE_TABLE4_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_SAVE_ALL_TABLE_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_SAVE_ALL_TABLE_Cmd_t))) { PAY_SLT_IFB_SAVE_ALL_TABLE_Cmd((const PAY_SLT_IFB_SAVE_ALL_TABLE_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_SET_CSP_ADDR_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_SET_CSP_ADDR_Cmd_t))) { PAY_SLT_IFB_SET_CSP_ADDR_Cmd((const PAY_SLT_IFB_SET_CSP_ADDR_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_SET_CAN_SPEED_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_SET_CAN_SPEED_Cmd_t))) { PAY_SLT_IFB_SET_CAN_SPEED_Cmd((const PAY_SLT_IFB_SET_CAN_SPEED_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_SET_I2C_ADDR_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_SET_I2C_ADDR_Cmd_t))) { PAY_SLT_IFB_SET_I2C_ADDR_Cmd((const PAY_SLT_IFB_SET_I2C_ADDR_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_SET_I2C_SPEED_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_SET_I2C_SPEED_Cmd_t))) { PAY_SLT_IFB_SET_I2C_SPEED_Cmd((const PAY_SLT_IFB_SET_I2C_SPEED_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_SET_WDT_VAL_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_SET_WDT_VAL_Cmd_t))) { PAY_SLT_IFB_SET_WDT_VAL_Cmd((const PAY_SLT_IFB_SET_WDT_VAL_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_IFB_SET_CSP_RTABLE_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_IFB_SET_CSP_RTABLE_Cmd_t))) { PAY_SLT_IFB_SET_CSP_RTABLE_Cmd((const PAY_SLT_IFB_SET_CSP_RTABLE_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_CSP_CMP_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_CSP_CMP_Cmd_t))) { PAY_SLT_EXP_CSP_CMP_Cmd((const PAY_SLT_EXP_CSP_CMP_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_CSP_PING_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_CSP_PING_Cmd_t))) { PAY_SLT_EXP_CSP_PING_Cmd((const PAY_SLT_EXP_CSP_PING_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_CSP_PS_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_CSP_PS_Cmd_t))) { PAY_SLT_EXP_CSP_PS_Cmd((const PAY_SLT_EXP_CSP_PS_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_CSP_MEM_FREE_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_CSP_MEM_FREE_Cmd_t))) { PAY_SLT_EXP_CSP_MEM_FREE_Cmd((const PAY_SLT_EXP_CSP_MEM_FREE_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_CSP_REBOOT_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_CSP_REBOOT_Cmd_t))) { PAY_SLT_EXP_CSP_REBOOT_Cmd((const PAY_SLT_EXP_CSP_REBOOT_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_CSP_BUF_FREE_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_CSP_BUF_FREE_Cmd_t))) { PAY_SLT_EXP_CSP_BUF_FREE_Cmd((const PAY_SLT_EXP_CSP_BUF_FREE_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_CSP_UPTIME_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_CSP_UPTIME_Cmd_t))) { PAY_SLT_EXP_CSP_UPTIME_Cmd((const PAY_SLT_EXP_CSP_UPTIME_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_CSP_GNDWDT_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_CSP_GNDWDT_Cmd_t))) { PAY_SLT_EXP_CSP_GNDWDT_Cmd((const PAY_SLT_EXP_CSP_GNDWDT_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_GET_BRD_UID_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_GET_BRD_UID_Cmd_t))) { PAY_SLT_EXP_GET_BRD_UID_Cmd((const PAY_SLT_EXP_GET_BRD_UID_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_GET_BRD_REV_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_GET_BRD_REV_Cmd_t))) { PAY_SLT_EXP_GET_BRD_REV_Cmd((const PAY_SLT_EXP_GET_BRD_REV_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_GET_CSP_ADDR_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_GET_CSP_ADDR_Cmd_t))) { PAY_SLT_EXP_GET_CSP_ADDR_Cmd((const PAY_SLT_EXP_GET_CSP_ADDR_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_GET_CAN_SPEED_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_GET_CAN_SPEED_Cmd_t))) { PAY_SLT_EXP_GET_CAN_SPEED_Cmd((const PAY_SLT_EXP_GET_CAN_SPEED_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_GET_I2C_ADDR_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_GET_I2C_ADDR_Cmd_t))) { PAY_SLT_EXP_GET_I2C_ADDR_Cmd((const PAY_SLT_EXP_GET_I2C_ADDR_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_GET_I2C_SPEED_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_GET_I2C_SPEED_Cmd_t))) { PAY_SLT_EXP_GET_I2C_SPEED_Cmd((const PAY_SLT_EXP_GET_I2C_SPEED_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_GET_WDT_VAL_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_GET_WDT_VAL_Cmd_t))) { PAY_SLT_EXP_GET_WDT_VAL_Cmd((const PAY_SLT_EXP_GET_WDT_VAL_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_GET_CSP_RTABLE_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_GET_CSP_RTABLE_Cmd_t))) { PAY_SLT_EXP_GET_CSP_RTABLE_Cmd((const PAY_SLT_EXP_GET_CSP_RTABLE_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_GET_SYS_STATUS_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_GET_SYS_STATUS_Cmd_t))) { PAY_SLT_EXP_GET_SYS_STATUS_Cmd((const PAY_SLT_EXP_GET_SYS_STATUS_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_GET_SYS_UPTIME_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_GET_SYS_UPTIME_Cmd_t))) { PAY_SLT_EXP_GET_SYS_UPTIME_Cmd((const PAY_SLT_EXP_GET_SYS_UPTIME_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_GET_BOOT_CNT_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_GET_BOOT_CNT_Cmd_t))) { PAY_SLT_EXP_GET_BOOT_CNT_Cmd((const PAY_SLT_EXP_GET_BOOT_CNT_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_GET_BOOT_CAUSE_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_GET_BOOT_CAUSE_Cmd_t))) { PAY_SLT_EXP_GET_BOOT_CAUSE_Cmd((const PAY_SLT_EXP_GET_BOOT_CAUSE_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_GET_REBOOT_CAUSE_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_GET_REBOOT_CAUSE_Cmd_t))) { PAY_SLT_EXP_GET_REBOOT_CAUSE_Cmd((const PAY_SLT_EXP_GET_REBOOT_CAUSE_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_GET_WDT_LEFT_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_GET_WDT_LEFT_Cmd_t))) { PAY_SLT_EXP_GET_WDT_LEFT_Cmd((const PAY_SLT_EXP_GET_WDT_LEFT_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_GET_BRD_TEMP_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_GET_BRD_TEMP_Cmd_t))) { PAY_SLT_EXP_GET_BRD_TEMP_Cmd((const PAY_SLT_EXP_GET_BRD_TEMP_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_GET_PWR_CURRENT_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_GET_PWR_CURRENT_Cmd_t))) { PAY_SLT_EXP_GET_PWR_CURRENT_Cmd((const PAY_SLT_EXP_GET_PWR_CURRENT_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_GET_IMU_DATA_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_GET_IMU_DATA_Cmd_t))) { PAY_SLT_EXP_GET_IMU_DATA_Cmd((const PAY_SLT_EXP_GET_IMU_DATA_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_GET_NTC_DATA_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_GET_NTC_DATA_Cmd_t))) { PAY_SLT_EXP_GET_NTC_DATA_Cmd((const PAY_SLT_EXP_GET_NTC_DATA_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_SAVE_TABLE0_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_SAVE_TABLE0_Cmd_t))) { PAY_SLT_EXP_SAVE_TABLE0_Cmd((const PAY_SLT_EXP_SAVE_TABLE0_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_SAVE_TABLE1_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_SAVE_TABLE1_Cmd_t))) { PAY_SLT_EXP_SAVE_TABLE1_Cmd((const PAY_SLT_EXP_SAVE_TABLE1_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_SAVE_TABLE4_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_SAVE_TABLE4_Cmd_t))) { PAY_SLT_EXP_SAVE_TABLE4_Cmd((const PAY_SLT_EXP_SAVE_TABLE4_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_SAVE_ALL_TABLE_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_SAVE_ALL_TABLE_Cmd_t))) { PAY_SLT_EXP_SAVE_ALL_TABLE_Cmd((const PAY_SLT_EXP_SAVE_ALL_TABLE_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_SET_CSP_ADDR_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_SET_CSP_ADDR_Cmd_t))) { PAY_SLT_EXP_SET_CSP_ADDR_Cmd((const PAY_SLT_EXP_SET_CSP_ADDR_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_SET_CAN_SPEED_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_SET_CAN_SPEED_Cmd_t))) { PAY_SLT_EXP_SET_CAN_SPEED_Cmd((const PAY_SLT_EXP_SET_CAN_SPEED_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_SET_I2C_ADDR_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_SET_I2C_ADDR_Cmd_t))) { PAY_SLT_EXP_SET_I2C_ADDR_Cmd((const PAY_SLT_EXP_SET_I2C_ADDR_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_SET_I2C_SPEED_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_SET_I2C_SPEED_Cmd_t))) { PAY_SLT_EXP_SET_I2C_SPEED_Cmd((const PAY_SLT_EXP_SET_I2C_SPEED_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_SET_WDT_VAL_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_SET_WDT_VAL_Cmd_t))) { PAY_SLT_EXP_SET_WDT_VAL_Cmd((const PAY_SLT_EXP_SET_WDT_VAL_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_SET_CSP_RTABLE_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_SET_CSP_RTABLE_Cmd_t))) { PAY_SLT_EXP_SET_CSP_RTABLE_Cmd((const PAY_SLT_EXP_SET_CSP_RTABLE_Cmd_t *)SBBufPtr); }
            break;
        case PAY_SLT_EXP_I2C_READ_CHUNK_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAY_SLT_EXP_I2C_READ_CHUNK_Cmd_t))) { PAY_SLT_EXP_I2C_READ_CHUNK_Cmd((const PAY_SLT_EXP_I2C_READ_CHUNK_Cmd_t *)SBBufPtr); }
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
