/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 ************************************************************************/

#include <string.h>
#include <gs/param/rparam.h>

#include "pay_slt_app.h"
#include "pay_slt_cmds.h"
#include "pay_slt_eventids.h"
#include "pay_slt_msgids.h"
#include "pay_slt_version.h"

typedef struct
{
    uint8 Node;
    uint8 BoardTable;
    uint8 TlmTable;
    uint16 AddrBrdUid;
    uint16 AddrBrdRev;
    uint16 AddrCspAddr;
    uint16 AddrCanSpeed;
    uint16 AddrI2cAddr;
    uint16 AddrI2cSpeed;
    uint16 AddrWdtVal;
    uint16 AddrCspRtable;
    uint16 AddrSysStatus;
    uint16 AddrSysUptime;
    uint16 AddrBootCnt;
    uint16 AddrBootCause;
    uint16 AddrRebootCause;
    uint16 AddrWdtLeft;
    uint16 AddrBrdTemp;
    uint16 AddrPwrCurrent;
    uint16 AddrImuData;
    uint16 AddrNtcData;
} PAY_SLT_TargetConfig_t;

static const PAY_SLT_TargetConfig_t PAY_SLT_IFB_CFG = {
    CSP_NODE_PAY_IFB, BOARD_PARAMETER_TABLE, TELEMETRY_TABLE,
    BRD_UID_ADDRESS, BRD_REV_ADDRESS, CSP_ADDR_ADDRESS, CAN_SPEED_ADDRESS, I2C_ADDR_ADDRESS, I2C_SPEED_ADDRESS,
    WDT_VAL_ADDRESS, CSP_RTABLE_ADDRESS, SYS_STATUS_ADDRESS, SYS_UPTIME_ADDRESS, BOOT_CNT_ADDRESS,
    BOOT_CAUSE_ADDRESS, REBOOT_CAUSE_ADDRESS, WDT_LEFT_ADDRESS, BRD_TEMP_ADDRESS, PWR_CURRENT_ADDRESS,
    IMU_DATA_ADDRESS, NTC_DATA_ADDRESS};

static const PAY_SLT_TargetConfig_t PAY_SLT_EXP_CFG = {
    CSP_NODE_PAY_EXP, BOARD_PARAMETER_TABLE, TELEMETRY_TABLE,
    BRD_UID_ADDRESS, BRD_REV_ADDRESS, EXP_CSP_ADDR_MPU_ADDRESS, EXP_CAN_SPEED_ADDRESS, EXP_I2C_ADDR_ADDRESS,
    EXP_I2C_SPEED_ADDRESS, EXP_WDT_VAL_ADDRESS, EXP_CSP_RTABLE_ADDRESS, EXP_SYS_STATUS_ADDRESS,
    EXP_SYS_UPTIME_ADDRESS, EXP_BOOT_CNT_ADDRESS, EXP_BOOT_CAUSE_ADDRESS, EXP_REBOOT_CAUSE_ADDRESS,
    EXP_WDT_LEFT_ADDRESS, EXP_BRD_TEMP_ADDRESS, EXP_PWR_CURRENT_ADDRESS, EXP_IMU_DATA_ADDRESS,
    EXP_NTC_DATA_ADDRESS};

static CFE_Status_t PAY_SLT_HandleReport(int32 status, uint8 command_code, bool device_error, const void *read_data,
                                         uint16 read_size)
{
    CFE_SB_Buffer_t *BufPtr;
    SLT_IFB_RPT_t *Report;
    bool success = ((device_error && (status > 0)) || (!device_error && (status == CFE_SUCCESS)));
    size_t copy_size = read_size;

    BufPtr = CFE_SB_AllocateMessageBuffer(sizeof(SLT_IFB_RPT_t));
    if (BufPtr == NULL)
    {
        return CFE_SB_BUF_ALOC_ERR;
    }

    Report = (SLT_IFB_RPT_t *)BufPtr;
    memset(Report, 0, sizeof(*Report));
    if (CFE_MSG_Init(CFE_MSG_PTR(Report->TelemetryHeader), CFE_SB_ValueToMsgId(PAY_SLT_RPT_TLM_MID), sizeof(*Report)) !=
        CFE_SUCCESS)
    {
        CFE_SB_ReleaseMessageBuffer(BufPtr);
        return CFE_MSG_BAD_ARGUMENT;
    }

    Report->Report.MsgID = PAY_SLT_CMD_MID;
    Report->Report.CommandCode = command_code;
    Report->Report.ReturnCode = success ? CFE_SUCCESS : status;

    if (success)
    {
        Report->Report.ReturnType = RPT_RETTYPE_SUCCESS;
        SLT_IFB_Data.CmdCounter++;
    }
    else
    {
        Report->Report.ReturnType = device_error ? RPT_RETTYPE_HW : RPT_RETTYPE_APP;
        SLT_IFB_Data.CmdCounter++;
        SLT_IFB_Data.ErrCounter++;
        if (device_error)
        {
            SLT_IFB_Data.DeviceErrCounter++;
        }
        else
        {
            SLT_IFB_Data.AppErrCounter++;
        }
    }

    if ((read_data == NULL) || (read_size == 0U))
    {
        goto send_report;
    }

    if (copy_size > sizeof(Report->Report.ReturnValue))
    {
        copy_size = sizeof(Report->Report.ReturnValue);
    }

    memcpy(Report->Report.ReturnValue, read_data, copy_size);
    Report->Report.ReturnDataSize = (uint16)copy_size;

send_report:
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(Report->TelemetryHeader));
    if (CFE_SB_TransmitBuffer(BufPtr, true) != CFE_SUCCESS)
    {
        CFE_SB_ReleaseMessageBuffer(BufPtr);
        return CFE_SB_BUF_ALOC_ERR;
    }

    return success ? CFE_SUCCESS : status;
}

static CFE_Status_t PAY_SLT_ReportTxn(uint8 command_code, int32 status)
{
    return PAY_SLT_HandleReport(status, command_code, true, NULL, 0);
}

static CFE_Status_t PAY_SLT_ReportQuery(uint8 command_code, int32 status, const void *data, uint16 size)
{
    return PAY_SLT_HandleReport(status, command_code, true, data, size);
}

static void PAY_SLT_UpdateIfbCache(void)
{
    (void)PAY_SLT_GetRparam(GS_PARAM_INT16, PAY_SLT_IFB_CFG.Node, PAY_SLT_IFB_CFG.TlmTable,
                            PAY_SLT_IFB_CFG.AddrSysStatus, &SLT_IFB_Data.sys_status);
    (void)PAY_SLT_GetRparam(GS_PARAM_UINT32, PAY_SLT_IFB_CFG.Node, PAY_SLT_IFB_CFG.TlmTable,
                            PAY_SLT_IFB_CFG.AddrSysUptime, &SLT_IFB_Data.sys_uptime);
    (void)PAY_SLT_GetRparam(GS_PARAM_UINT16, PAY_SLT_IFB_CFG.Node, PAY_SLT_IFB_CFG.TlmTable,
                            PAY_SLT_IFB_CFG.AddrBootCnt, &SLT_IFB_Data.boot_cnt);
    (void)PAY_SLT_GetRparam(GS_PARAM_UINT16, PAY_SLT_IFB_CFG.Node, PAY_SLT_IFB_CFG.TlmTable,
                            PAY_SLT_IFB_CFG.AddrBootCause, &SLT_IFB_Data.boot_cause);
    (void)PAY_SLT_GetRparam(GS_PARAM_UINT16, PAY_SLT_IFB_CFG.Node, PAY_SLT_IFB_CFG.TlmTable,
                            PAY_SLT_IFB_CFG.AddrRebootCause, &SLT_IFB_Data.reboot_cause);
    (void)PAY_SLT_GetRparam(GS_PARAM_UINT32, PAY_SLT_IFB_CFG.Node, PAY_SLT_IFB_CFG.TlmTable,
                            PAY_SLT_IFB_CFG.AddrWdtLeft, &SLT_IFB_Data.wdt_left);
    (void)PAY_SLT_GetRparam(GS_PARAM_INT16, PAY_SLT_IFB_CFG.Node, PAY_SLT_IFB_CFG.TlmTable,
                            PAY_SLT_IFB_CFG.AddrBrdTemp, &SLT_IFB_Data.brd_temp);
    (void)PAY_SLT_GetRparam(GS_PARAM_UINT16, PAY_SLT_IFB_CFG.Node, PAY_SLT_IFB_CFG.TlmTable,
                            PAY_SLT_IFB_CFG.AddrPwrCurrent, &SLT_IFB_Data.pwr_current);
    (void)PAY_SLT_GetRparam(GS_PARAM_UINT16, PAY_SLT_IFB_CFG.Node, PAY_SLT_IFB_CFG.TlmTable,
                            PAY_SLT_IFB_CFG.AddrImuData, SLT_IFB_Data.imu_data);
    (void)PAY_SLT_GetRparam(GS_PARAM_INT16, PAY_SLT_IFB_CFG.Node, PAY_SLT_IFB_CFG.TlmTable,
                            PAY_SLT_IFB_CFG.AddrNtcData, SLT_IFB_Data.ntc_data);
}

CFE_Status_t SLT_IFB_SendHkCmd(const SLT_IFB_SendHkCmd_t *Msg)
{
    (void)Msg;

    PAY_SLT_UpdateIfbCache();

    SLT_IFB_Data.HkTlm.Payload.CommandErrorCounter = SLT_IFB_Data.ErrCounter;
    SLT_IFB_Data.HkTlm.Payload.CommandCounter = SLT_IFB_Data.CmdCounter;
    SLT_IFB_Data.HkTlm.Payload.sys_status = SLT_IFB_Data.sys_status;
    SLT_IFB_Data.HkTlm.Payload.sys_uptime = SLT_IFB_Data.sys_uptime;
    SLT_IFB_Data.HkTlm.Payload.boot_cnt = SLT_IFB_Data.boot_cnt;
    SLT_IFB_Data.HkTlm.Payload.boot_cause = SLT_IFB_Data.boot_cause;
    SLT_IFB_Data.HkTlm.Payload.reboot_cause = SLT_IFB_Data.reboot_cause;
    SLT_IFB_Data.HkTlm.Payload.wdt_left = SLT_IFB_Data.wdt_left;
    SLT_IFB_Data.HkTlm.Payload.brd_temp = SLT_IFB_Data.brd_temp;
    SLT_IFB_Data.HkTlm.Payload.pwr_current = SLT_IFB_Data.pwr_current;
    memcpy(SLT_IFB_Data.HkTlm.Payload.imu_data, SLT_IFB_Data.imu_data, sizeof(SLT_IFB_Data.imu_data));
    memcpy(SLT_IFB_Data.HkTlm.Payload.ntc_data, SLT_IFB_Data.ntc_data, sizeof(SLT_IFB_Data.ntc_data));

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(SLT_IFB_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(SLT_IFB_Data.HkTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

CFE_Status_t PAY_SLT_SendBeaconCmd(const SLT_IFB_SendBcnCmd_t *Msg)
{
    (void)Msg;

    PAY_SLT_UpdateIfbCache();

    SLT_IFB_Data.BcnTlm.Payload.CommandErrorCounter = SLT_IFB_Data.ErrCounter;
    SLT_IFB_Data.BcnTlm.Payload.CommandCounter = SLT_IFB_Data.CmdCounter;
    SLT_IFB_Data.BcnTlm.Payload.sys_status = SLT_IFB_Data.sys_status;
    SLT_IFB_Data.BcnTlm.Payload.boot_cnt = SLT_IFB_Data.boot_cnt;
    SLT_IFB_Data.BcnTlm.Payload.brd_temp = SLT_IFB_Data.brd_temp;
    SLT_IFB_Data.BcnTlm.Payload.pwr_current = SLT_IFB_Data.pwr_current;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(SLT_IFB_Data.BcnTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(SLT_IFB_Data.BcnTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

CFE_Status_t SLT_IFB_NoopCmd(const SLT_IFB_NoopCmd_t *Msg)
{
    (void)Msg;
    (void)PAY_SLT_HandleReport(CFE_SUCCESS, PAY_SLT_NOOP_CC, false, NULL, 0);
    CFE_EVS_SendEvent(SLT_IFB_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "PAY_SLT: NOOP command %s",
                      SLT_IFB_VERSION);
    return CFE_SUCCESS;
}

CFE_Status_t SLT_IFB_ResetCountersCmd(const SLT_IFB_ResetCountersCmd_t *Msg)
{
    (void)Msg;
    SLT_IFB_Data.CmdCounter = 0;
    SLT_IFB_Data.ErrCounter = 0;
    SLT_IFB_Data.AppErrCounter = 0;
    SLT_IFB_Data.DeviceErrCounter = 0;
    (void)PAY_SLT_HandleReport(CFE_SUCCESS, PAY_SLT_RESET_COUNTERS_CC, false, NULL, 0);
    return CFE_SUCCESS;
}

static CFE_Status_t PAY_SLT_RunNoArgTxn(const PAY_SLT_TargetConfig_t *cfg, uint8 cc, int32 (*fn)(uint8))
{
    return PAY_SLT_ReportTxn(cc, fn(cfg->Node));
}

static CFE_Status_t PAY_SLT_RunScalarGet(const PAY_SLT_TargetConfig_t *cfg, uint8 cc, uint8 param_type, uint8 table_id,
                                         uint16 addr, void *value, uint16 size)
{
    return PAY_SLT_ReportQuery(cc, PAY_SLT_GetRparam(param_type, cfg->Node, table_id, addr, value), value, size);
}

static CFE_Status_t PAY_SLT_RunStringGet(const PAY_SLT_TargetConfig_t *cfg, uint8 cc, uint8 table_id, uint16 addr,
                                         char *value, uint16 size)
{
    return PAY_SLT_ReportQuery(cc, PAY_SLT_GetRparam(GS_PARAM_STRING, cfg->Node, table_id, addr, value), value, size);
}

static CFE_Status_t PAY_SLT_RunSet(const PAY_SLT_TargetConfig_t *cfg, uint8 cc, uint8 param_type, uint16 addr,
                                   void *value)
{
    return PAY_SLT_ReportQuery(cc, PAY_SLT_SetRparam(param_type, cfg->Node, cfg->BoardTable, addr, value), NULL, 0);
}

static CFE_Status_t PAY_SLT_RunSave(const PAY_SLT_TargetConfig_t *cfg, uint8 cc, uint8 table_id)
{
    return PAY_SLT_ReportQuery(cc, PAY_SLT_SaveTable(cfg->Node, table_id), NULL, 0);
}

CFE_Status_t PAY_SLT_IFB_CSP_CMP_Cmd(const PAY_SLT_IFB_CSP_CMP_Cmd_t *Msg)
{
    (void)Msg;
    return PAY_SLT_RunNoArgTxn(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_CSP_CMP_CC, PAY_SLT_CSP_CMP);
}

CFE_Status_t PAY_SLT_IFB_CSP_PING_Cmd(const PAY_SLT_IFB_CSP_PING_Cmd_t *Msg)
{
    (void)Msg;
    return PAY_SLT_RunNoArgTxn(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_CSP_PING_CC, PAY_SLT_CSP_PING);
}

CFE_Status_t PAY_SLT_IFB_CSP_PS_Cmd(const PAY_SLT_IFB_CSP_PS_Cmd_t *Msg)
{
    (void)Msg;
    return PAY_SLT_RunNoArgTxn(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_CSP_PS_CC, PAY_SLT_CSP_PS);
}

CFE_Status_t PAY_SLT_IFB_CSP_MEM_FREE_Cmd(const PAY_SLT_IFB_CSP_MEM_FREE_Cmd_t *Msg)
{
    (void)Msg;
    return PAY_SLT_RunNoArgTxn(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_CSP_MEM_FREE_CC, PAY_SLT_CSP_MEM_FREE);
}

CFE_Status_t PAY_SLT_IFB_CSP_REBOOT_Cmd(const PAY_SLT_IFB_CSP_REBOOT_Cmd_t *Msg)
{
    (void)Msg;
    return PAY_SLT_RunNoArgTxn(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_CSP_REBOOT_CC, PAY_SLT_CSP_REBOOT);
}

CFE_Status_t PAY_SLT_IFB_CSP_BUF_FREE_Cmd(const PAY_SLT_IFB_CSP_BUF_FREE_Cmd_t *Msg)
{
    (void)Msg;
    return PAY_SLT_RunNoArgTxn(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_CSP_BUF_FREE_CC, PAY_SLT_CSP_BUF_FREE);
}

CFE_Status_t PAY_SLT_IFB_CSP_UPTIME_Cmd(const PAY_SLT_IFB_CSP_UPTIME_Cmd_t *Msg)
{
    (void)Msg;
    return PAY_SLT_RunNoArgTxn(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_CSP_UPTIME_CC, PAY_SLT_CSP_UPTIME);
}

CFE_Status_t PAY_SLT_IFB_CSP_GNDWDT_Cmd(const PAY_SLT_IFB_CSP_GNDWDT_Cmd_t *Msg)
{
    (void)Msg;
    return PAY_SLT_RunNoArgTxn(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_CSP_GNDWDT_CC, PAY_SLT_CSP_GNDWDT);
}

CFE_Status_t PAY_SLT_IFB_GET_BRD_UID_Cmd(const PAY_SLT_IFB_GET_BRD_UID_Cmd_t *Msg)
{
    char value[PAY_SLT_BRD_UID_SIZE] = {0};
    (void)Msg;
    return PAY_SLT_RunStringGet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_GET_BRD_UID_CC, PAY_SLT_IFB_CFG.BoardTable,
                                PAY_SLT_IFB_CFG.AddrBrdUid, value, sizeof(value));
}

CFE_Status_t PAY_SLT_IFB_GET_BRD_REV_Cmd(const PAY_SLT_IFB_GET_BRD_REV_Cmd_t *Msg)
{
    uint8 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_GET_BRD_REV_CC, GS_PARAM_UINT8,
                                PAY_SLT_IFB_CFG.BoardTable, PAY_SLT_IFB_CFG.AddrBrdRev, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_IFB_GET_CSP_ADDR_Cmd(const PAY_SLT_IFB_GET_CSP_ADDR_Cmd_t *Msg)
{
    uint8 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_GET_CSP_ADDR_CC, GS_PARAM_UINT8,
                                PAY_SLT_IFB_CFG.BoardTable, PAY_SLT_IFB_CFG.AddrCspAddr, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_IFB_GET_CAN_SPEED_Cmd(const PAY_SLT_IFB_GET_CAN_SPEED_Cmd_t *Msg)
{
    uint16 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_GET_CAN_SPEED_CC, GS_PARAM_UINT16,
                                PAY_SLT_IFB_CFG.BoardTable, PAY_SLT_IFB_CFG.AddrCanSpeed, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_IFB_GET_I2C_ADDR_Cmd(const PAY_SLT_IFB_GET_I2C_ADDR_Cmd_t *Msg)
{
    uint8 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_GET_I2C_ADDR_CC, GS_PARAM_UINT8,
                                PAY_SLT_IFB_CFG.BoardTable, PAY_SLT_IFB_CFG.AddrI2cAddr, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_IFB_GET_I2C_SPEED_Cmd(const PAY_SLT_IFB_GET_I2C_SPEED_Cmd_t *Msg)
{
    uint16 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_GET_I2C_SPEED_CC, GS_PARAM_UINT16,
                                PAY_SLT_IFB_CFG.BoardTable, PAY_SLT_IFB_CFG.AddrI2cSpeed, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_IFB_GET_WDT_VAL_Cmd(const PAY_SLT_IFB_GET_WDT_VAL_Cmd_t *Msg)
{
    uint32 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_GET_WDT_VAL_CC, GS_PARAM_UINT32,
                                PAY_SLT_IFB_CFG.BoardTable, PAY_SLT_IFB_CFG.AddrWdtVal, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_IFB_GET_CSP_RTABLE_Cmd(const PAY_SLT_IFB_GET_CSP_RTABLE_Cmd_t *Msg)
{
    char value[PAY_SLT_RTABLE_STR_SIZE] = {0};
    (void)Msg;
    return PAY_SLT_RunStringGet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_GET_CSP_RTABLE_CC, PAY_SLT_IFB_CFG.BoardTable,
                                PAY_SLT_IFB_CFG.AddrCspRtable, value, sizeof(value));
}

CFE_Status_t PAY_SLT_IFB_GET_SYS_STATUS_Cmd(const PAY_SLT_IFB_GET_SYS_STATUS_Cmd_t *Msg)
{
    int16 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_GET_SYS_STATUS_CC, GS_PARAM_INT16,
                                PAY_SLT_IFB_CFG.TlmTable, PAY_SLT_IFB_CFG.AddrSysStatus, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_IFB_GET_SYS_UPTIME_Cmd(const PAY_SLT_IFB_GET_SYS_UPTIME_Cmd_t *Msg)
{
    uint32 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_GET_SYS_UPTIME_CC, GS_PARAM_UINT32,
                                PAY_SLT_IFB_CFG.TlmTable, PAY_SLT_IFB_CFG.AddrSysUptime, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_IFB_GET_BOOT_CNT_Cmd(const PAY_SLT_IFB_GET_BOOT_CNT_Cmd_t *Msg)
{
    uint16 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_GET_BOOT_CNT_CC, GS_PARAM_UINT16,
                                PAY_SLT_IFB_CFG.TlmTable, PAY_SLT_IFB_CFG.AddrBootCnt, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_IFB_GET_BOOT_CAUSE_Cmd(const PAY_SLT_IFB_GET_BOOT_CAUSE_Cmd_t *Msg)
{
    uint16 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_GET_BOOT_CAUSE_CC, GS_PARAM_UINT16,
                                PAY_SLT_IFB_CFG.TlmTable, PAY_SLT_IFB_CFG.AddrBootCause, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_IFB_GET_REBOOT_CAUSE_Cmd(const PAY_SLT_IFB_GET_REBOOT_CAUSE_Cmd_t *Msg)
{
    uint16 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_GET_REBOOT_CAUSE_CC, GS_PARAM_UINT16,
                                PAY_SLT_IFB_CFG.TlmTable, PAY_SLT_IFB_CFG.AddrRebootCause, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_IFB_GET_WDT_LEFT_Cmd(const PAY_SLT_IFB_GET_WDT_LEFT_Cmd_t *Msg)
{
    uint32 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_GET_WDT_LEFT_CC, GS_PARAM_UINT32,
                                PAY_SLT_IFB_CFG.TlmTable, PAY_SLT_IFB_CFG.AddrWdtLeft, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_IFB_GET_BRD_TEMP_Cmd(const PAY_SLT_IFB_GET_BRD_TEMP_Cmd_t *Msg)
{
    int16 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_GET_BRD_TEMP_CC, GS_PARAM_INT16,
                                PAY_SLT_IFB_CFG.TlmTable, PAY_SLT_IFB_CFG.AddrBrdTemp, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_IFB_GET_PWR_CURRENT_Cmd(const PAY_SLT_IFB_GET_PWR_CURRENT_Cmd_t *Msg)
{
    uint16 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_GET_PWR_CURRENT_CC, GS_PARAM_UINT16,
                                PAY_SLT_IFB_CFG.TlmTable, PAY_SLT_IFB_CFG.AddrPwrCurrent, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_IFB_GET_IMU_DATA_Cmd(const PAY_SLT_IFB_GET_IMU_DATA_Cmd_t *Msg)
{
    uint16 value[6] = {0};
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_GET_IMU_DATA_CC, GS_PARAM_UINT16,
                                PAY_SLT_IFB_CFG.TlmTable, PAY_SLT_IFB_CFG.AddrImuData, value, sizeof(value));
}

CFE_Status_t PAY_SLT_IFB_GET_NTC_DATA_Cmd(const PAY_SLT_IFB_GET_NTC_DATA_Cmd_t *Msg)
{
    int16 value[4] = {0};
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_GET_NTC_DATA_CC, GS_PARAM_INT16,
                                PAY_SLT_IFB_CFG.TlmTable, PAY_SLT_IFB_CFG.AddrNtcData, value, sizeof(value));
}

CFE_Status_t PAY_SLT_IFB_SAVE_TABLE0_Cmd(const PAY_SLT_IFB_SAVE_TABLE0_Cmd_t *Msg)
{
    (void)Msg;
    return PAY_SLT_RunSave(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_SAVE_TABLE0_CC, BOARD_PARAMETER_TABLE);
}

CFE_Status_t PAY_SLT_IFB_SAVE_TABLE1_Cmd(const PAY_SLT_IFB_SAVE_TABLE1_Cmd_t *Msg)
{
    (void)Msg;
    return PAY_SLT_RunSave(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_SAVE_TABLE1_CC, CONFIGURATION_PARAMETER_TABLE);
}

CFE_Status_t PAY_SLT_IFB_SAVE_TABLE4_Cmd(const PAY_SLT_IFB_SAVE_TABLE4_Cmd_t *Msg)
{
    (void)Msg;
    return PAY_SLT_RunSave(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_SAVE_TABLE4_CC, TELEMETRY_TABLE);
}

CFE_Status_t PAY_SLT_IFB_SAVE_ALL_TABLE_Cmd(const PAY_SLT_IFB_SAVE_ALL_TABLE_Cmd_t *Msg)
{
    int32 status;
    (void)Msg;
    status = PAY_SLT_SaveTable(PAY_SLT_IFB_CFG.Node, BOARD_PARAMETER_TABLE);
    status |= PAY_SLT_SaveTable(PAY_SLT_IFB_CFG.Node, CONFIGURATION_PARAMETER_TABLE);
    status |= PAY_SLT_SaveTable(PAY_SLT_IFB_CFG.Node, TELEMETRY_TABLE);
    return PAY_SLT_ReportQuery(PAY_SLT_IFB_SAVE_ALL_TABLE_CC, status, NULL, 0);
}

CFE_Status_t PAY_SLT_IFB_SET_CSP_ADDR_Cmd(const PAY_SLT_IFB_SET_CSP_ADDR_Cmd_t *Msg)
{
    uint8 value = Msg->Arg;
    return PAY_SLT_RunSet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_SET_CSP_ADDR_CC, GS_PARAM_UINT8,
                          PAY_SLT_IFB_CFG.AddrCspAddr, &value);
}

CFE_Status_t PAY_SLT_IFB_SET_CAN_SPEED_Cmd(const PAY_SLT_IFB_SET_CAN_SPEED_Cmd_t *Msg)
{
    uint16 value = Msg->Arg;
    return PAY_SLT_RunSet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_SET_CAN_SPEED_CC, GS_PARAM_UINT16,
                          PAY_SLT_IFB_CFG.AddrCanSpeed, &value);
}

CFE_Status_t PAY_SLT_IFB_SET_I2C_ADDR_Cmd(const PAY_SLT_IFB_SET_I2C_ADDR_Cmd_t *Msg)
{
    uint8 value = Msg->Arg;
    return PAY_SLT_RunSet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_SET_I2C_ADDR_CC, GS_PARAM_UINT8,
                          PAY_SLT_IFB_CFG.AddrI2cAddr, &value);
}

CFE_Status_t PAY_SLT_IFB_SET_I2C_SPEED_Cmd(const PAY_SLT_IFB_SET_I2C_SPEED_Cmd_t *Msg)
{
    uint16 value = Msg->Arg;
    return PAY_SLT_RunSet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_SET_I2C_SPEED_CC, GS_PARAM_UINT16,
                          PAY_SLT_IFB_CFG.AddrI2cSpeed, &value);
}

CFE_Status_t PAY_SLT_IFB_SET_WDT_VAL_Cmd(const PAY_SLT_IFB_SET_WDT_VAL_Cmd_t *Msg)
{
    uint32 value = Msg->Arg;
    return PAY_SLT_RunSet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_SET_WDT_VAL_CC, GS_PARAM_UINT32,
                          PAY_SLT_IFB_CFG.AddrWdtVal, &value);
}

CFE_Status_t PAY_SLT_IFB_SET_CSP_RTABLE_Cmd(const PAY_SLT_IFB_SET_CSP_RTABLE_Cmd_t *Msg)
{
    char value[PAY_SLT_RTABLE_STR_SIZE] = {0};
    memcpy(value, Msg->Arg, sizeof(value));
    return PAY_SLT_RunSet(&PAY_SLT_IFB_CFG, PAY_SLT_IFB_SET_CSP_RTABLE_CC, GS_PARAM_STRING,
                          PAY_SLT_IFB_CFG.AddrCspRtable, value);
}

CFE_Status_t PAY_SLT_EXP_CSP_CMP_Cmd(const PAY_SLT_EXP_CSP_CMP_Cmd_t *Msg)
{
    (void)Msg;
    return PAY_SLT_RunNoArgTxn(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_CSP_CMP_CC, PAY_SLT_CSP_CMP);
}

CFE_Status_t PAY_SLT_EXP_CSP_PING_Cmd(const PAY_SLT_EXP_CSP_PING_Cmd_t *Msg)
{
    (void)Msg;
    return PAY_SLT_RunNoArgTxn(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_CSP_PING_CC, PAY_SLT_CSP_PING);
}

CFE_Status_t PAY_SLT_EXP_CSP_PS_Cmd(const PAY_SLT_EXP_CSP_PS_Cmd_t *Msg)
{
    (void)Msg;
    return PAY_SLT_RunNoArgTxn(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_CSP_PS_CC, PAY_SLT_CSP_PS);
}

CFE_Status_t PAY_SLT_EXP_CSP_MEM_FREE_Cmd(const PAY_SLT_EXP_CSP_MEM_FREE_Cmd_t *Msg)
{
    (void)Msg;
    return PAY_SLT_RunNoArgTxn(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_CSP_MEM_FREE_CC, PAY_SLT_CSP_MEM_FREE);
}

CFE_Status_t PAY_SLT_EXP_CSP_REBOOT_Cmd(const PAY_SLT_EXP_CSP_REBOOT_Cmd_t *Msg)
{
    (void)Msg;
    return PAY_SLT_RunNoArgTxn(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_CSP_REBOOT_CC, PAY_SLT_CSP_REBOOT);
}

CFE_Status_t PAY_SLT_EXP_CSP_BUF_FREE_Cmd(const PAY_SLT_EXP_CSP_BUF_FREE_Cmd_t *Msg)
{
    (void)Msg;
    return PAY_SLT_RunNoArgTxn(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_CSP_BUF_FREE_CC, PAY_SLT_CSP_BUF_FREE);
}

CFE_Status_t PAY_SLT_EXP_CSP_UPTIME_Cmd(const PAY_SLT_EXP_CSP_UPTIME_Cmd_t *Msg)
{
    (void)Msg;
    return PAY_SLT_RunNoArgTxn(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_CSP_UPTIME_CC, PAY_SLT_CSP_UPTIME);
}

CFE_Status_t PAY_SLT_EXP_CSP_GNDWDT_Cmd(const PAY_SLT_EXP_CSP_GNDWDT_Cmd_t *Msg)
{
    (void)Msg;
    return PAY_SLT_RunNoArgTxn(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_CSP_GNDWDT_CC, PAY_SLT_CSP_GNDWDT);
}

CFE_Status_t PAY_SLT_EXP_GET_BRD_UID_Cmd(const PAY_SLT_EXP_GET_BRD_UID_Cmd_t *Msg)
{
    char value[PAY_SLT_BRD_UID_SIZE] = {0};
    (void)Msg;
    return PAY_SLT_RunStringGet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_GET_BRD_UID_CC, PAY_SLT_EXP_CFG.BoardTable,
                                PAY_SLT_EXP_CFG.AddrBrdUid, value, sizeof(value));
}

CFE_Status_t PAY_SLT_EXP_GET_BRD_REV_Cmd(const PAY_SLT_EXP_GET_BRD_REV_Cmd_t *Msg)
{
    uint8 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_GET_BRD_REV_CC, GS_PARAM_UINT8,
                                PAY_SLT_EXP_CFG.BoardTable, PAY_SLT_EXP_CFG.AddrBrdRev, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_EXP_GET_CSP_ADDR_Cmd(const PAY_SLT_EXP_GET_CSP_ADDR_Cmd_t *Msg)
{
    uint8 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_GET_CSP_ADDR_CC, GS_PARAM_UINT8,
                                PAY_SLT_EXP_CFG.BoardTable, PAY_SLT_EXP_CFG.AddrCspAddr, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_EXP_GET_CAN_SPEED_Cmd(const PAY_SLT_EXP_GET_CAN_SPEED_Cmd_t *Msg)
{
    uint16 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_GET_CAN_SPEED_CC, GS_PARAM_UINT16,
                                PAY_SLT_EXP_CFG.BoardTable, PAY_SLT_EXP_CFG.AddrCanSpeed, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_EXP_GET_I2C_ADDR_Cmd(const PAY_SLT_EXP_GET_I2C_ADDR_Cmd_t *Msg)
{
    uint8 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_GET_I2C_ADDR_CC, GS_PARAM_UINT8,
                                PAY_SLT_EXP_CFG.BoardTable, PAY_SLT_EXP_CFG.AddrI2cAddr, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_EXP_GET_I2C_SPEED_Cmd(const PAY_SLT_EXP_GET_I2C_SPEED_Cmd_t *Msg)
{
    uint16 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_GET_I2C_SPEED_CC, GS_PARAM_UINT16,
                                PAY_SLT_EXP_CFG.BoardTable, PAY_SLT_EXP_CFG.AddrI2cSpeed, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_EXP_GET_WDT_VAL_Cmd(const PAY_SLT_EXP_GET_WDT_VAL_Cmd_t *Msg)
{
    uint32 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_GET_WDT_VAL_CC, GS_PARAM_UINT32,
                                PAY_SLT_EXP_CFG.BoardTable, PAY_SLT_EXP_CFG.AddrWdtVal, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_EXP_GET_CSP_RTABLE_Cmd(const PAY_SLT_EXP_GET_CSP_RTABLE_Cmd_t *Msg)
{
    char value[PAY_SLT_RTABLE_STR_SIZE] = {0};
    (void)Msg;
    return PAY_SLT_RunStringGet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_GET_CSP_RTABLE_CC, PAY_SLT_EXP_CFG.BoardTable,
                                PAY_SLT_EXP_CFG.AddrCspRtable, value, sizeof(value));
}

CFE_Status_t PAY_SLT_EXP_GET_SYS_STATUS_Cmd(const PAY_SLT_EXP_GET_SYS_STATUS_Cmd_t *Msg)
{
    int16 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_GET_SYS_STATUS_CC, GS_PARAM_INT16,
                                PAY_SLT_EXP_CFG.TlmTable, PAY_SLT_EXP_CFG.AddrSysStatus, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_EXP_GET_SYS_UPTIME_Cmd(const PAY_SLT_EXP_GET_SYS_UPTIME_Cmd_t *Msg)
{
    uint32 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_GET_SYS_UPTIME_CC, GS_PARAM_UINT32,
                                PAY_SLT_EXP_CFG.TlmTable, PAY_SLT_EXP_CFG.AddrSysUptime, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_EXP_GET_BOOT_CNT_Cmd(const PAY_SLT_EXP_GET_BOOT_CNT_Cmd_t *Msg)
{
    uint16 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_GET_BOOT_CNT_CC, GS_PARAM_UINT16,
                                PAY_SLT_EXP_CFG.TlmTable, PAY_SLT_EXP_CFG.AddrBootCnt, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_EXP_GET_BOOT_CAUSE_Cmd(const PAY_SLT_EXP_GET_BOOT_CAUSE_Cmd_t *Msg)
{
    uint16 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_GET_BOOT_CAUSE_CC, GS_PARAM_UINT16,
                                PAY_SLT_EXP_CFG.TlmTable, PAY_SLT_EXP_CFG.AddrBootCause, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_EXP_GET_REBOOT_CAUSE_Cmd(const PAY_SLT_EXP_GET_REBOOT_CAUSE_Cmd_t *Msg)
{
    uint16 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_GET_REBOOT_CAUSE_CC, GS_PARAM_UINT16,
                                PAY_SLT_EXP_CFG.TlmTable, PAY_SLT_EXP_CFG.AddrRebootCause, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_EXP_GET_WDT_LEFT_Cmd(const PAY_SLT_EXP_GET_WDT_LEFT_Cmd_t *Msg)
{
    uint32 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_GET_WDT_LEFT_CC, GS_PARAM_UINT32,
                                PAY_SLT_EXP_CFG.TlmTable, PAY_SLT_EXP_CFG.AddrWdtLeft, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_EXP_GET_BRD_TEMP_Cmd(const PAY_SLT_EXP_GET_BRD_TEMP_Cmd_t *Msg)
{
    int16 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_GET_BRD_TEMP_CC, GS_PARAM_INT16,
                                PAY_SLT_EXP_CFG.TlmTable, PAY_SLT_EXP_CFG.AddrBrdTemp, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_EXP_GET_PWR_CURRENT_Cmd(const PAY_SLT_EXP_GET_PWR_CURRENT_Cmd_t *Msg)
{
    uint16 value = 0;
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_GET_PWR_CURRENT_CC, GS_PARAM_UINT16,
                                PAY_SLT_EXP_CFG.TlmTable, PAY_SLT_EXP_CFG.AddrPwrCurrent, &value, sizeof(value));
}

CFE_Status_t PAY_SLT_EXP_GET_IMU_DATA_Cmd(const PAY_SLT_EXP_GET_IMU_DATA_Cmd_t *Msg)
{
    uint16 value[6] = {0};
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_GET_IMU_DATA_CC, GS_PARAM_UINT16,
                                PAY_SLT_EXP_CFG.TlmTable, PAY_SLT_EXP_CFG.AddrImuData, value, sizeof(value));
}

CFE_Status_t PAY_SLT_EXP_GET_NTC_DATA_Cmd(const PAY_SLT_EXP_GET_NTC_DATA_Cmd_t *Msg)
{
    int16 value[4] = {0};
    (void)Msg;
    return PAY_SLT_RunScalarGet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_GET_NTC_DATA_CC, GS_PARAM_INT16,
                                PAY_SLT_EXP_CFG.TlmTable, PAY_SLT_EXP_CFG.AddrNtcData, value, sizeof(value));
}

CFE_Status_t PAY_SLT_EXP_SAVE_TABLE0_Cmd(const PAY_SLT_EXP_SAVE_TABLE0_Cmd_t *Msg)
{
    (void)Msg;
    return PAY_SLT_RunSave(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_SAVE_TABLE0_CC, BOARD_PARAMETER_TABLE);
}

CFE_Status_t PAY_SLT_EXP_SAVE_TABLE1_Cmd(const PAY_SLT_EXP_SAVE_TABLE1_Cmd_t *Msg)
{
    (void)Msg;
    return PAY_SLT_RunSave(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_SAVE_TABLE1_CC, CONFIGURATION_PARAMETER_TABLE);
}

CFE_Status_t PAY_SLT_EXP_SAVE_TABLE4_Cmd(const PAY_SLT_EXP_SAVE_TABLE4_Cmd_t *Msg)
{
    (void)Msg;
    return PAY_SLT_RunSave(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_SAVE_TABLE4_CC, TELEMETRY_TABLE);
}

CFE_Status_t PAY_SLT_EXP_SAVE_ALL_TABLE_Cmd(const PAY_SLT_EXP_SAVE_ALL_TABLE_Cmd_t *Msg)
{
    int32 status;
    (void)Msg;
    status = PAY_SLT_SaveTable(PAY_SLT_EXP_CFG.Node, BOARD_PARAMETER_TABLE);
    status |= PAY_SLT_SaveTable(PAY_SLT_EXP_CFG.Node, CONFIGURATION_PARAMETER_TABLE);
    status |= PAY_SLT_SaveTable(PAY_SLT_EXP_CFG.Node, TELEMETRY_TABLE);
    return PAY_SLT_ReportQuery(PAY_SLT_EXP_SAVE_ALL_TABLE_CC, status, NULL, 0);
}

CFE_Status_t PAY_SLT_EXP_SET_CSP_ADDR_Cmd(const PAY_SLT_EXP_SET_CSP_ADDR_Cmd_t *Msg)
{
    uint8 value = Msg->Arg;
    return PAY_SLT_RunSet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_SET_CSP_ADDR_CC, GS_PARAM_UINT8,
                          PAY_SLT_EXP_CFG.AddrCspAddr, &value);
}

CFE_Status_t PAY_SLT_EXP_SET_CAN_SPEED_Cmd(const PAY_SLT_EXP_SET_CAN_SPEED_Cmd_t *Msg)
{
    uint16 value = Msg->Arg;
    return PAY_SLT_RunSet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_SET_CAN_SPEED_CC, GS_PARAM_UINT16,
                          PAY_SLT_EXP_CFG.AddrCanSpeed, &value);
}

CFE_Status_t PAY_SLT_EXP_SET_I2C_ADDR_Cmd(const PAY_SLT_EXP_SET_I2C_ADDR_Cmd_t *Msg)
{
    uint8 value = Msg->Arg;
    return PAY_SLT_RunSet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_SET_I2C_ADDR_CC, GS_PARAM_UINT8,
                          PAY_SLT_EXP_CFG.AddrI2cAddr, &value);
}

CFE_Status_t PAY_SLT_EXP_SET_I2C_SPEED_Cmd(const PAY_SLT_EXP_SET_I2C_SPEED_Cmd_t *Msg)
{
    uint16 value = Msg->Arg;
    return PAY_SLT_RunSet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_SET_I2C_SPEED_CC, GS_PARAM_UINT16,
                          PAY_SLT_EXP_CFG.AddrI2cSpeed, &value);
}

CFE_Status_t PAY_SLT_EXP_SET_WDT_VAL_Cmd(const PAY_SLT_EXP_SET_WDT_VAL_Cmd_t *Msg)
{
    uint32 value = Msg->Arg;
    return PAY_SLT_RunSet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_SET_WDT_VAL_CC, GS_PARAM_UINT32,
                          PAY_SLT_EXP_CFG.AddrWdtVal, &value);
}

CFE_Status_t PAY_SLT_EXP_SET_CSP_RTABLE_Cmd(const PAY_SLT_EXP_SET_CSP_RTABLE_Cmd_t *Msg)
{
    char value[PAY_SLT_RTABLE_STR_SIZE] = {0};
    memcpy(value, Msg->Arg, sizeof(value));
    return PAY_SLT_RunSet(&PAY_SLT_EXP_CFG, PAY_SLT_EXP_SET_CSP_RTABLE_CC, GS_PARAM_STRING,
                          PAY_SLT_EXP_CFG.AddrCspRtable, value);
}

CFE_Status_t PAY_SLT_EXP_I2C_READ_CHUNK_Cmd(const PAY_SLT_EXP_I2C_READ_CHUNK_Cmd_t *Msg)
{
    uint8 chunk[PAY_SLT_I2C_READ_CHUNK_SIZE] = {0};
    int32 status = PAY_SLT_ReadExpI2CChunk(SLT_IFB_Data.I2c2Handle, Msg->Arg, chunk, sizeof(chunk));

    if (status != CFE_SUCCESS)
    {
        return PAY_SLT_HandleReport(status, PAY_SLT_EXP_I2C_READ_CHUNK_CC, true, NULL, 0);
    }

    return PAY_SLT_HandleReport(CFE_SUCCESS, PAY_SLT_EXP_I2C_READ_CHUNK_CC, true, chunk, sizeof(chunk));
}
