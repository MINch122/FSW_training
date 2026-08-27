/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as "core Flight System: Bootes"
 *
 * Licensed under the Apache License, Version 2.0
 ************************************************************************/

/**
 * \file
 *   This file contains the source code for the Thrust App ground command-handling functions.
 */

/*
** Include Files:
*/
#include "thrust_app.h"
#include "thrust_cmds.h"
#include "thrust_cmds_list.h"
#include "thrust_eventids.h"

#include <string.h>

static CFE_Status_t THRUST_LogCmdResult(const char *CmdName, CFE_Status_t status)
{
    if (status == CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(THRUST_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "THRUST: %s command succeeded", CmdName);
    }
    else
    {
        CFE_EVS_SendEvent(THRUST_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "THRUST: %s command failed, RC = 0x%08lX",
                          CmdName, (unsigned long)status);
    }

    return status;
}

static void THRUST_HandleDeviceReportForMid(uint16_t msgId, int32 status, uint8_t cc)
{
    const uint8_t *rawResponse = NULL;
    uint16_t rawResponseSize = 0;

    THRUST_GetLastResponse(&rawResponse, &rawResponseSize);
    THRUST_HandleReportForMid(msgId, status, cc, rawResponse, rawResponseSize);
}

static void THRUST_HandleDeviceReport(int32 status, uint8_t cc)
{
    THRUST_HandleDeviceReportForMid((uint16_t)THRUST_CMD_MID, status, cc);
}

/* ===================================================================
 * PostCmdProcess: DefaultResponse → AppData 반영 + EVS 이벤트
 *
 * DefaultResponse를 수신하는 모든 Cmd 함수에서 호출.(cmd_list.c의 RecvDefaultResponse함수 참고)
 * CurrentStatus / FaultFlags / InterlockFlags를 AppData에 동기화하고
 * 이상 상태 감지 시 EVS 이벤트를 발행한다.
 * =================================================================== */
static void THRUST_PostCmdProcess(const THRUST_DefaultResponse_t *resp)
{
    THRUST_AppData.CurrentStatus  = resp->CurrentStatus;
    THRUST_AppData.FaultFlags     = resp->FaultFlags;
    THRUST_AppData.InterlockFlags = resp->InterlockFlags;

    if (resp->FaultFlags != 0)
    {
        CFE_EVS_SendEvent(THRUST_FAULT_EID, CFE_EVS_EventType_ERROR,
                          "iG4U Fault detected: FaultFlags=0x%04X",
                          resp->FaultFlags);
    }
    if (resp->InterlockFlags != 0)
    {
        CFE_EVS_SendEvent(THRUST_INTERLOCK_EID, CFE_EVS_EventType_ERROR,
                          "iG4U Interlock active: InterlockFlags=0x%04X",
                          resp->InterlockFlags);
    }
}

static void THRUST_UpdateModeTracking(uint32_t currentMode, uint8_t lastMsgId,
                                      uint8_t lastResult)
{
    THRUST_AppData.CurrentMode = currentMode;
    THRUST_AppData.LastMsgId   = lastMsgId;
    THRUST_AppData.LastResult  = lastResult;
}

static void THRUST_BuildHkTlm(THRUST_HkTlm_Payload_t *tlm,
                              const THRUST_HKData_Payload_t *reply)
{
    tlm->Pressure_CH0 = reply->Pressure_CH0;
    tlm->Pressure_CH1 = reply->Pressure_CH1;
    tlm->Pressure_CH2 = reply->Pressure_CH2;
    tlm->Pressure_CH3 = reply->Pressure_CH3;
    tlm->Temp_CH0     = reply->Temp_CH0;
    tlm->Temp_CH1     = reply->Temp_CH1;
    tlm->Temp_CH2     = reply->Temp_CH2;
    tlm->Temp_CH3     = reply->Temp_CH3;
    tlm->Temp_CH4     = reply->Temp_CH4;
    tlm->Temp_CH5     = reply->Temp_CH5;
    tlm->Status       = reply->Status;
    tlm->CmdCounter   = THRUST_AppData.CmdCounter;
    tlm->ErrCounter   = THRUST_AppData.ErrCounter;
}

/* -------------------------------------------------- */
/* NoopCmd: 아무것도 하지 않음 — 통신 확인용          */
/* -------------------------------------------------- */
CFE_Status_t THRUST_NoopCmd(const THRUST_NoopCmd_t *Msg) {
    (void)Msg;
    THRUST_AppData.CmdCounter++;
    CFE_EVS_SendEvent(THRUST_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "THRUST: NOOP command");
    THRUST_HandleReport(CFE_SUCCESS, THRUST_NOOP_CC, NULL, 0);
    return CFE_SUCCESS;
}

/* -------------------------------------------------- */
/* ResetCountersCmd: CmdCounter, ErrCounter 초기화    */
/* -------------------------------------------------- */
CFE_Status_t THRUST_ResetCountersCmd(const THRUST_ResetCountersCmd_t *Msg) {
    (void)Msg;
    THRUST_AppData.CmdCounter = 0;
    THRUST_AppData.ErrCounter = 0;
    CFE_EVS_SendEvent(THRUST_RESET_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "THRUST: Reset Counters command");
    THRUST_HandleReport(CFE_SUCCESS, THRUST_RESET_COUNTERS_CC, NULL, 0);
    return CFE_SUCCESS;
}

/* -------------------------------------------------- */
/* HandleReport: 모든 명령 결과를 SB 텔레메트리로 전송 */
/* ADCS_HandleReport()와 동일 역할                    */
/* 입력: status(성공/실패), CC(어떤 명령), data, size */
/* 출력: SB Report TLM → 지상국                       */
/* -------------------------------------------------- */
void THRUST_HandleReportForMid(uint16_t msgId, int32 status, uint8_t cc, const void *data, uint16_t dataSize)
{
    THRUST_ReportTlm_t *BufPtr =
        (THRUST_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(THRUST_ReportTlm_t));

    if (BufPtr == NULL)
    {
        return;
    }

    memset(BufPtr, 0, sizeof(*BufPtr));

    if (CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader), CFE_SB_ValueToMsgId(THRUST_REPORT_TLM_MID),
                     sizeof(THRUST_ReportTlm_t)) != CFE_SUCCESS)
    {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        return;
    }

    BufPtr->Report.MsgID          = msgId;
    BufPtr->Report.CommandCode    = cc;
    if (status == CFE_SUCCESS)
    {
        BufPtr->Report.ReturnType = RPT_RETTYPE_SUCCESS;
    }
    else if ((status <= THRUST_ERR_WRITE) && (status >= THRUST_ERR_PACKET))
    {
        BufPtr->Report.ReturnType = RPT_RETTYPE_HW;
    }
    else
    {
        BufPtr->Report.ReturnType = RPT_RETTYPE_APP;
    }
    BufPtr->Report.ReturnCode     = status;
    if (data == NULL)
    {
        dataSize = 0;
    }

    BufPtr->Report.ReturnDataSize = (dataSize > sizeof(BufPtr->Report.ReturnValue)) ?
                                        sizeof(BufPtr->Report.ReturnValue) : dataSize;

    if (data != NULL && BufPtr->Report.ReturnDataSize > 0)
    {
        memcpy(BufPtr->Report.ReturnValue, data, BufPtr->Report.ReturnDataSize);
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
    (void)CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true);
}

void THRUST_HandleReport(int32 status, uint8_t cc, const void *data, uint16_t dataSize)
{
    THRUST_HandleReportForMid((uint16_t)THRUST_CMD_MID, status, cc, data, dataSize);
}

/* -------------------------------------------------- */
/* SendHkCmd: HK 데이터 읽기 → HandleReport로 SB 전송 */
/* 입력: 지상국 HK 요청 메시지                        */
/* 출력: SB Report TLM → 지상국                       */
/* -------------------------------------------------- */
CFE_Status_t THRUST_SendHkCmd(const THRUST_SendHkCmd_t *Msg) {
    THRUST_HKData_Payload_t hkData = {0};
    CFE_SB_MsgId_t msgId = CFE_SB_INVALID_MSG_ID;
    uint16 reportMsgId = (uint16)THRUST_CMD_MID;
    uint8 reportCc = THRUST_REQ_HK_CC;
    int32 status = THRUST_GetHKData(&hkData);

    if (CFE_MSG_GetMsgId(CFE_MSG_PTR(Msg->CommandHeader), &msgId) == CFE_SUCCESS)
    {
        reportMsgId = (uint16)CFE_SB_MsgIdToValue(msgId);
        if (reportMsgId == (uint16)THRUST_SEND_HK_MID)
        {
            reportCc = 0;
        }
    }

    THRUST_HandleDeviceReportForMid(reportMsgId, status, reportCc);
    THRUST_LogCmdResult("Send HK", status);
    if (status != CFE_SUCCESS) { THRUST_AppData.ErrCounter++; return status; }
    THRUST_AppData.CmdCounter++;
    return CFE_SUCCESS;
}

/* -------------------------------------------------- */
/* SCH Send HK: 장치 HK 읽기 -> HK TLM 전송 (RPT 없음) */
/* -------------------------------------------------- */
CFE_Status_t THRUST_SendScheduledHkCmd(const THRUST_SendHkCmd_t *Msg)
{
    THRUST_HKData_Payload_t hkData = {0};
    CFE_Status_t status;

    (void)Msg;

    status = THRUST_GetHKData(&hkData);
    if (status != CFE_SUCCESS)
    {
        THRUST_AppData.ErrCounter++;
        return THRUST_LogCmdResult("Scheduled HK", status);
    }

    THRUST_BuildHkTlm(&THRUST_AppData.HkTlm.Payload, &hkData);
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(THRUST_AppData.HkTlm.TelemetryHeader));

    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(THRUST_AppData.HkTlm.TelemetryHeader), true);
    if (status != CFE_SUCCESS)
    {
        THRUST_AppData.ErrCounter++;
        return THRUST_LogCmdResult("Scheduled HK telemetry", status);
    }

    return CFE_SUCCESS;
}

/* -------------------------------------------------- */
/* SetModeCmd: payload에서 TargetMode 추출 → SetMode  */
/* -------------------------------------------------- */
CFE_Status_t THRUST_SetModeCmd(const THRUST_SetModeCmd_t *Msg) {
    THRUST_DefaultResponse_t resp = {0};
    CFE_Status_t status = THRUST_SetMode(Msg->Payload.TargetMode, &resp);
    if (status == CFE_SUCCESS) {
        THRUST_PostCmdProcess(&resp);
        THRUST_UpdateModeTracking(Msg->Payload.TargetMode, THRUST_SET_MODE_CC,
                                  THRUST_RESULT_ACK);
    }
    THRUST_HandleDeviceReport(status, THRUST_SET_MODE_CC);
    THRUST_LogCmdResult("Set Mode", status);
    if (status != CFE_SUCCESS) { THRUST_AppData.ErrCounter++; return status; }
    THRUST_AppData.CmdCounter++;
    return CFE_SUCCESS;
}

/* -------------------------------------------------- */
/* ArmCmd: ArmKey/TimeoutSec 추출 → ArmPropulsion     */
/* -------------------------------------------------- */
CFE_Status_t THRUST_ArmCmd(const THRUST_ArmCmd_t *Msg) {
    THRUST_DefaultResponse_t resp = {0};
    CFE_Status_t status = THRUST_ArmPropulsion(Msg->Payload.ArmKey,
                                               Msg->Payload.TimeoutSec,
                                               &resp);
    if (status == CFE_SUCCESS) {
        THRUST_PostCmdProcess(&resp);
        if (THRUST_AppData.InterlockFlags != 0) {
            status = THRUST_ERR_INTERLOCK;
            CFE_EVS_SendEvent(THRUST_ARM_INTERLOCK_EID, CFE_EVS_EventType_ERROR,
                              "THRUST: Arm blocked, Interlock=0x%04X",
                              THRUST_AppData.InterlockFlags);
        }
        else {
            THRUST_UpdateModeTracking(THRUST_MODE_ARMED, THRUST_ARM_CC,
                                      THRUST_RESULT_ACK);
        }
    }

    THRUST_HandleDeviceReport(status, THRUST_ARM_CC);
    THRUST_LogCmdResult("Arm", status);
    if (status != CFE_SUCCESS) { THRUST_AppData.ErrCounter++; return status; }

    THRUST_AppData.CmdCounter++;
    return CFE_SUCCESS;
}

/* -------------------------------------------------- */
/* MainThrusterFireCmd: 유효성 검증 포함              */
/* IgnitionTime < FireTime 강제 (물리적 요구사항)     */
/* -------------------------------------------------- */
CFE_Status_t THRUST_MainThrusterFireCmd(const THRUST_MainFireCmd_t *Msg) {
    /* 상태 검증: ARMED 상태에서만 허용 (ICD 5.1절) */
    // if (THRUST_AppData.CurrentMode != THRUST_MODE_ARMED) {
    //     CFE_EVS_SendEvent(THRUST_FIRE_STATE_ERR_EID, CFE_EVS_EventType_ERROR,
    //                       "THRUST: Fire rejected, not ARMED (mode=%lu)",
    //                       (unsigned long)THRUST_AppData.CurrentMode);
    //     THRUST_LogCmdResult("Main Fire", THRUST_ERR_INVALID_STATE);
    //     THRUST_AppData.ErrCounter++;
    //     THRUST_HandleReport(THRUST_ERR_INVALID_STATE, THRUST_MAIN_FIRE_CC, NULL, 0);
    //     return THRUST_ERR_INVALID_STATE;
    // }

    /* 파라미터 검증 */
    if (Msg->Payload.IgnitionTime_ms >= Msg->Payload.FireTime_ms) {
        CFE_EVS_SendEvent(THRUST_FIRE_PARAM_ERR_EID, CFE_EVS_EventType_ERROR,
                          "THRUST: IgnitionTime(%u) >= FireTime(%u)",
                          Msg->Payload.IgnitionTime_ms, Msg->Payload.FireTime_ms);
        THRUST_LogCmdResult("Main Fire", THRUST_ERR_INVALID_PARAM);
        THRUST_AppData.ErrCounter++;
        THRUST_HandleReport(THRUST_ERR_INVALID_PARAM, THRUST_MAIN_FIRE_CC, NULL, 0);
        return THRUST_ERR_INVALID_PARAM;
    }

    THRUST_DefaultResponse_t resp = {0};
    CFE_Status_t status = THRUST_MainThrusterFire(
        Msg->Payload.FireTime_ms, Msg->Payload.IgnitionTime_ms,
        Msg->Payload.FireKey, &resp);
    if (status == CFE_SUCCESS) {
        THRUST_PostCmdProcess(&resp);
        THRUST_UpdateModeTracking(THRUST_MODE_FIRING, THRUST_MAIN_FIRE_CC,
                                  THRUST_RESULT_ACK);
    }
    THRUST_HandleDeviceReport(status, THRUST_MAIN_FIRE_CC);
    THRUST_LogCmdResult("Main Fire", status);
    if (status != CFE_SUCCESS) { THRUST_AppData.ErrCounter++; return status; }

    THRUST_AppData.CmdCounter++;
    return CFE_SUCCESS;
}

/* -------------------------------------------------- */
/* PingCmd                                            */
/* -------------------------------------------------- */
CFE_Status_t THRUST_PingCmd(const THRUST_PingCmd_t *Msg) {
    THRUST_DefaultResponse_t resp = {0};
    CFE_Status_t status = THRUST_Ping(&resp);
    if (status == CFE_SUCCESS) { THRUST_PostCmdProcess(&resp); }
    THRUST_HandleDeviceReport(status, THRUST_PING_CC);
    THRUST_LogCmdResult("Ping", status);
    if (status != CFE_SUCCESS) { THRUST_AppData.ErrCounter++; return status; }
    THRUST_AppData.CmdCounter++;
    return CFE_SUCCESS;
}

/* -------------------------------------------------- */
/* ResetModuleCmd                                     */
/* -------------------------------------------------- */
CFE_Status_t THRUST_ResetModuleCmd(const THRUST_ResetModuleCmd_t *Msg) {
    THRUST_DefaultResponse_t resp = {0};
    CFE_Status_t status = THRUST_ResetModule(Msg->Payload.TargetMode, &resp);
    if (status == CFE_SUCCESS) { THRUST_PostCmdProcess(&resp); }
    THRUST_HandleDeviceReport(status, THRUST_RESET_MODULE_CC);
    THRUST_LogCmdResult("Reset Module", status);
    if (status != CFE_SUCCESS) { THRUST_AppData.ErrCounter++; return status; }
    THRUST_AppData.CmdCounter++;
    return CFE_SUCCESS;
}

/* -------------------------------------------------- */
/* DisarmCmd                                          */
/* -------------------------------------------------- */
CFE_Status_t THRUST_DisarmCmd(const THRUST_DisarmCmd_t *Msg) {
    THRUST_DefaultResponse_t resp = {0};
    CFE_Status_t status = THRUST_DisarmPropulsion(&resp);
    if (status == CFE_SUCCESS) {
        THRUST_PostCmdProcess(&resp);
        THRUST_UpdateModeTracking(THRUST_MODE_STANDBY, THRUST_DISARM_CC,
                                  THRUST_RESULT_ACK);
    }
    THRUST_HandleDeviceReport(status, THRUST_DISARM_CC);
    THRUST_LogCmdResult("Disarm", status);
    if (status != CFE_SUCCESS) { THRUST_AppData.ErrCounter++; return status; }
    THRUST_AppData.CmdCounter++;
    return CFE_SUCCESS;//return값이 있는 경우에 대해서 report 함수를 재사용해서 보낼것.
}

/* -------------------------------------------------- */
/* ReqStatusCmd: Status 읽기 → HandleReport로 SB 전송 */
/* -------------------------------------------------- */
CFE_Status_t THRUST_ReqStatusCmd(const THRUST_ReqStatusCmd_t *Msg) {
    THRUST_StatusData_Payload_t statusData = {0};
    int32 status = THRUST_GetStatus(&statusData);
    THRUST_HandleDeviceReport(status, THRUST_REQ_STATUS_CC);
    THRUST_LogCmdResult("Request Status", status);
    if (status != CFE_SUCCESS) { THRUST_AppData.ErrCounter++; return status; }

    /* AppData 동기화 */
    THRUST_AppData.CurrentMode    = statusData.CurrentMode;
    THRUST_AppData.CurrentStatus  = statusData.CurrentStatus;
    THRUST_AppData.FaultFlags     = statusData.FaultFlags;
    THRUST_AppData.InterlockFlags = statusData.InterlockFlags;
    THRUST_AppData.LastMsgId      = statusData.LastMsgId;
    THRUST_AppData.LastResult     = statusData.LastResult;

    THRUST_AppData.CmdCounter++;
    return CFE_SUCCESS;
}

/* -------------------------------------------------- */
/* MainAbortCmd                                       */
/* -------------------------------------------------- */
CFE_Status_t THRUST_MainAbortCmd(const THRUST_MainAbortCmd_t *Msg) {
    THRUST_DefaultResponse_t resp = {0};
    CFE_Status_t status = THRUST_MainThrusterAbort(&resp);
    if (status == CFE_SUCCESS) { THRUST_PostCmdProcess(&resp); }
    THRUST_HandleDeviceReport(status, THRUST_MAIN_ABORT_CC);
    THRUST_LogCmdResult("Main Abort", status);
    if (status != CFE_SUCCESS) { THRUST_AppData.ErrCounter++; return status; }
    THRUST_AppData.CmdCounter++;
    return CFE_SUCCESS;
}

/* -------------------------------------------------- */
/* CGPulseCmd                                         */
/* -------------------------------------------------- */
CFE_Status_t THRUST_CGPulseCmd(const THRUST_CGPulseCmd_t *Msg) {
    THRUST_DefaultResponse_t resp = {0};
    CFE_Status_t status = THRUST_CGThrusterPulse(
        Msg->Payload.ThrusterId, Msg->Payload.PulseWidth_ms,
        Msg->Payload.FireKey, &resp);
    if (status == CFE_SUCCESS) { THRUST_PostCmdProcess(&resp); }
    THRUST_HandleDeviceReport(status, THRUST_CG_PULSE_CC);
    THRUST_LogCmdResult("CG Pulse", status);
    if (status != CFE_SUCCESS) { THRUST_AppData.ErrCounter++; return status; }
    THRUST_AppData.CmdCounter++;
    return CFE_SUCCESS;
}

/* -------------------------------------------------- */
/* CGAbortCmd                                         */
/* -------------------------------------------------- */
CFE_Status_t THRUST_CGAbortCmd(const THRUST_CGAbortCmd_t *Msg) {
    THRUST_DefaultResponse_t resp = {0};
    CFE_Status_t status = THRUST_CGThrusterAbort(&resp);
    if (status == CFE_SUCCESS) { THRUST_PostCmdProcess(&resp); }
    THRUST_HandleDeviceReport(status, THRUST_CG_ABORT_CC);
    THRUST_LogCmdResult("CG Abort", status);
    if (status != CFE_SUCCESS) { THRUST_AppData.ErrCounter++; return status; }
    THRUST_AppData.CmdCounter++;
    return CFE_SUCCESS;
}

/* -------------------------------------------------- */
/* ReqFaultLogCmd: FaultLog 읽기 → SB 전송            */
/* -------------------------------------------------- */
CFE_Status_t THRUST_ReqFaultLogCmd(const THRUST_ReqFaultLogCmd_t *Msg) {
    THRUST_FaultLogData_Payload_t faultLog = {0};
    CFE_Status_t status = THRUST_GetFaultLog(&faultLog);
    THRUST_HandleDeviceReport(status, THRUST_REQ_FAULT_LOG_CC);
    THRUST_LogCmdResult("Request Fault Log", status);
    if (status != CFE_SUCCESS) { THRUST_AppData.ErrCounter++; return status; }
    THRUST_AppData.CmdCounter++;
    return CFE_SUCCESS;
}

/* -------------------------------------------------- */
/* ClearFaultCmd                                      */
/* -------------------------------------------------- */
CFE_Status_t THRUST_ClearFaultCmd(const THRUST_ClearFaultCmd_t *Msg) {
    THRUST_DefaultResponse_t resp = {0};
    CFE_Status_t status = THRUST_ClearFault(&resp);
    if (status == CFE_SUCCESS) { THRUST_PostCmdProcess(&resp); }
    THRUST_HandleDeviceReport(status, THRUST_CLEAR_FAULT_CC);
    THRUST_LogCmdResult("Clear Fault", status);
    if (status != CFE_SUCCESS) { THRUST_AppData.ErrCounter++; return status; }
    THRUST_AppData.CmdCounter++;
    return CFE_SUCCESS;
}
