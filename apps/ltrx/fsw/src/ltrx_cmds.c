/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 ************************************************************************/

/**
 * \file
 *   LTRX App command handlers + HK/Status sender
 */

#include "ltrx_app.h"
#include "ltrx_cmds.h"
#include "ltrx_cmds_beacon.h"
#include "ltrx_msgids.h"
#include "ltrx_fcncodes.h"
#include "ltrx_eventids.h"
#include "ltrx_msg.h"
#include "ltrx_session.h"

#include "cfe.h"

#include <string.h>

void LTRX_HandleReport(CFE_Status_t Status, uint8 CC, const void *Data, uint16 DataSize)
{
    CFE_SB_Buffer_t *BufPtr = CFE_SB_AllocateMessageBuffer(sizeof(LTRX_ReportTlm_t));
    LTRX_ReportTlm_t *Report;
    uint16 CopySize;

    if (BufPtr == NULL)
    {
        return;
    }

    Report = (LTRX_ReportTlm_t *)BufPtr;
    memset(Report, 0, sizeof(*Report));

    if (CFE_MSG_Init(CFE_MSG_PTR(Report->TelemetryHeader), CFE_SB_ValueToMsgId(LTRX_RPT_TLM_MID),
                     sizeof(LTRX_ReportTlm_t)) != CFE_SUCCESS)
    {
        CFE_SB_ReleaseMessageBuffer(BufPtr);
        return;
    }

    CopySize = (DataSize > RPT_RET_VALUE_BUF_SIZE) ? RPT_RET_VALUE_BUF_SIZE : DataSize;

    Report->Report.MsgID          = LTRX_CMD_MID;
    Report->Report.CommandCode    = CC;
    Report->Report.ReturnType     = (Status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_APP;
    Report->Report.ReturnCode     = (uint32)Status;
    Report->Report.ReturnDataSize = CopySize;

    if (CopySize > 0 && Data != NULL)
    {
        memcpy(Report->Report.ReturnValue, Data, CopySize);
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(Report->TelemetryHeader));
    (void)CFE_SB_TransmitBuffer(BufPtr, true);
}

/* Helpers */

static uint8 LTRX_SatU32ToU8(uint32 v)
{
    return (v > 255) ? 255 : (uint8)v;
}

/* ---------------- HK sender ---------------- */

static void LTRX_ReportHousekeeping(void)
{
    LTRX_HkTlm_Payload_t Payload;

    memset(&Payload, 0, sizeof(Payload));

    Payload.CmdCounter    = LTRX_SatU32ToU8(LTRX_AppData.CmdCounter);
    Payload.CmdErrCounter = LTRX_SatU32ToU8(LTRX_AppData.AppErrCounter);
    Payload.HaveGnss = LTRX_AppData.HaveGnss ? 1 : 0;
    Payload.HaveBeaconStatus = LTRX_AppData.HaveBeaconStatus ? 1 : 0;
    Payload.DownstreamEnabled = LTRX_Downstream_IsEnabled() ? 1 : 0;
    Payload.BusBeaconPeriod = LTRX_Downstream_GetBeaconPeriod();
    Payload.BusBeaconCount  = LTRX_Downstream_GetBeaconCount();

    if (LTRX_AppData.HaveGnss)
    {
        memcpy(&Payload.LastGnss, &LTRX_AppData.LastGnss, sizeof(Payload.LastGnss));
    }

    if (LTRX_AppData.HaveBeaconStatus)
    {
        memcpy(&Payload.LastBeaconStatus, &LTRX_AppData.LastBeaconStatus, sizeof(Payload.LastBeaconStatus));
    }

    LTRX_APP_printf("LTRX: HK report requested\n");
    LTRX_HandleReport(CFE_SUCCESS, 0, &Payload, sizeof(Payload));
}

/* BCN/Status telemetry sender */
void LTRX_SendBcnTlm(void)
{
    LTRX_BcnTlm_t *BufPtr = (LTRX_BcnTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(LTRX_BcnTlm_t));
    if (BufPtr == NULL)
    {
        CFE_EVS_SendEvent(LTRX_ALLOC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LTRX: BCN TLM alloc failed");
        return;
    }

    if (CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader),
                     CFE_SB_ValueToMsgId(LTRX_BCN_TLM_MID),
                     sizeof(LTRX_BcnTlm_t)) != CFE_SUCCESS)
    {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        CFE_EVS_SendEvent(LTRX_ALLOC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LTRX: BCN TLM CFE_MSG_Init failed");
        return;
    }

    memset(&BufPtr->Payload, 0, sizeof(BufPtr->Payload));

    if (LTRX_AppData.HaveBeaconStatus)
    {
        BufPtr->Payload.Temperature       = LTRX_AppData.LastBeaconStatus.Temperature;
        BufPtr->Payload.ConnectionQuality = LTRX_AppData.LastBeaconStatus.ConnectionQuality;
        BufPtr->Payload.BatteryCapacity   = LTRX_AppData.LastBeaconStatus.BatteryCapacity;
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));

    if (CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true) != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(LTRX_HK_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LTRX: BCN TLM transmit failed");
        return;
    }

}

/* Public handlers */

CFE_Status_t LTRX_SendHkCmd(const CFE_SB_Buffer_t *SBBufPtr)
{
    (void)SBBufPtr;
    LTRX_ReportHousekeeping();
    return CFE_SUCCESS;
}

/* ---- Basic app CC ---- */

CFE_Status_t LTRX_NoopCmd(const LTRX_NoopCmd_t *Msg)
{
    (void)Msg;

    LTRX_AppData.CmdCounter++;

    CFE_EVS_SendEvent(LTRX_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "LTRX: NOOP (Cmd=%u AppErr=%u DevErr=%u)",
                      (unsigned)LTRX_AppData.CmdCounter,
                      (unsigned)LTRX_AppData.AppErrCounter,
                      (unsigned)LTRX_AppData.DeviceErrCounter);

    static const char NoopReport[] = "Yosi In Space";
    LTRX_HandleReport(CFE_SUCCESS, LTRX_NOOP_CC, NoopReport, sizeof(NoopReport));

    return CFE_SUCCESS;
}

CFE_Status_t LTRX_ResetCountersCmd(const LTRX_ResetCountersCmd_t *Msg)
{
    (void)Msg;

    LTRX_AppData.CmdCounter       = 0;
    LTRX_AppData.AppErrCounter    = 0;
    LTRX_AppData.DeviceErrCounter = 0;

    CFE_EVS_SendEvent(LTRX_RESET_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "LTRX: RESET_COUNTERS");
    LTRX_HandleReport(CFE_SUCCESS, LTRX_RESET_COUNTERS_CC, NULL, 0);
    return CFE_SUCCESS;
}

CFE_Status_t LTRX_ResetAppCmdCountersCmd(const LTRX_ResetAppCmdCountersCmd_t *Msg)
{
    (void)Msg;

    LTRX_AppData.CmdCounter    = 0;
    LTRX_AppData.AppErrCounter = 0;

    CFE_EVS_SendEvent(LTRX_RESET_APP_COUNTER_EID, CFE_EVS_EventType_INFORMATION,
                      "LTRX: RESET_APP_CMD_COUNTERS");
    LTRX_HandleReport(CFE_SUCCESS, LTRX_RESET_APP_CMD_COUNTERS_CC, NULL, 0);
    return CFE_SUCCESS;
}

CFE_Status_t LTRX_ResetDeviceCmdCountersCmd(const LTRX_ResetDeviceCmdCountersCmd_t *Msg)
{
    (void)Msg;

    LTRX_AppData.DeviceErrCounter = 0;

    CFE_EVS_SendEvent(LTRX_RESET_DEVICE_COUNTER_EID, CFE_EVS_EventType_INFORMATION,
                      "LTRX: RESET_DEVICE_CMD_COUNTERS");
    LTRX_HandleReport(CFE_SUCCESS, LTRX_RESET_DEVICE_CMD_COUNTERS_CC, NULL, 0);
    return CFE_SUCCESS;
}

CFE_Status_t LTRX_SetBusBeaconPeriodCmd(const LTRX_SetBusBeaconPeriodCmd_t *Msg)
{
    uint16 period = Msg->Payload.Period;

    if (period == 0u)
    {
        CFE_EVS_SendEvent(LTRX_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LTRX: SET_BUS_BEACON_PERIOD rejected, period must be at least 1");
        LTRX_HandleReport(CFE_ES_BAD_ARGUMENT, LTRX_SET_BUS_BEACON_PERIOD_CC, &period, sizeof(period));
        return CFE_ES_BAD_ARGUMENT;
    }

    LTRX_Downstream_SetBeaconPeriod(period);
    CFE_EVS_SendEvent(LTRX_BUS_BCN_PERIOD_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "LTRX: bus beacon period set to %u", (unsigned)period);
    LTRX_HandleReport(CFE_SUCCESS, LTRX_SET_BUS_BEACON_PERIOD_CC, &period, sizeof(period));

    return CFE_SUCCESS;
}

/* ---- Session/Child trigger CC ---- */

CFE_Status_t LTRX_SessionStartDownlinkCmd(const LTRX_SessionStartDownlinkCmd_t *Msg)
{
    CFE_Status_t Status;

    (void)Msg;

    if (LTRX_Downlink_IsInFlight())
    {
        CFE_EVS_SendEvent(LTRX_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LTRX: SESSION_START_DOWNLINK rejected - transfer already in progress");
        LTRX_HandleReport(CFE_STATUS_REQUEST_ALREADY_PENDING, LTRX_SESSION_START_DOWNLINK_CC, NULL, 0);
        return CFE_STATUS_REQUEST_ALREADY_PENDING;
    }

    if (!LTRX_Downlink_IsReady() && !LTRX_Downlink_HasPending())
    {
        CFE_EVS_SendEvent(LTRX_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LTRX: SESSION_START_DOWNLINK rejected - no message prepared");
        LTRX_HandleReport(CFE_STATUS_EXTERNAL_RESOURCE_FAIL, LTRX_SESSION_START_DOWNLINK_CC, NULL, 0);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    Status = LTRX_SessionRequestStartDownlink();
    LTRX_HandleReport(Status, LTRX_SESSION_START_DOWNLINK_CC, NULL, 0);
    return Status;
}

CFE_Status_t LTRX_SessionAbortCmd(const LTRX_SessionAbortCmd_t *Msg)
{
    CFE_Status_t Status;

    (void)Msg;
    Status = LTRX_SessionRequestAbort();
    LTRX_HandleReport(Status, LTRX_SESSION_ABORT_CC, NULL, 0);
    return Status;
}

CFE_Status_t LTRX_SessionResetStateCmd(const LTRX_SessionResetStateCmd_t *Msg)
{
    CFE_Status_t Status;

    (void)Msg;
    Status = LTRX_SessionRequestReset();
    LTRX_HandleReport(Status, LTRX_SESSION_RESET_STATE_CC, NULL, 0);
    return Status;
}

/* ---- Downstream gating ---- */

CFE_Status_t LTRX_DownstreamEnableCmd(const LTRX_DownstreamEnableCmd_t *Msg)
{
    (void)Msg;

    LTRX_Downstream_SetEnabled(true);

    CFE_EVS_SendEvent(LTRX_DOWNSTREAM_ENABLE_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "LTRX: DOWNSTREAM_ENABLE");
    LTRX_HandleReport(CFE_SUCCESS, LTRX_DOWNSTREAM_ENABLE_CC, NULL, 0);
    return CFE_SUCCESS;
}

CFE_Status_t LTRX_DownstreamDisableCmd(const LTRX_DownstreamDisableCmd_t *Msg)
{
    (void)Msg;

    LTRX_Downstream_SetEnabled(false);

    CFE_EVS_SendEvent(LTRX_DOWNSTREAM_DISABLE_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "LTRX: DOWNSTREAM_DISABLE");
    LTRX_HandleReport(CFE_SUCCESS, LTRX_DOWNSTREAM_DISABLE_CC, NULL, 0);
    return CFE_SUCCESS;
}
