/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * \file
 *   This file contains the source code for the Utrx App Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "utrx_app.h"
#include "utrx_cmds.h"
#include "utrx_msgids.h"
#include "utrx_eventids.h"
#include "utrx_utils.h"
#include "utrx_msg.h"
#include "utrx_dispatch.h"
#include <string.h>

/* The utrx_lib module provides the UTRX_Function() prototype */

static void UTRX_SendCmdReport(uint8 command_code, int32 status, const void *data, uint16 data_size,
                               uint8 return_type)
{
    uint16 copy_size = 0;

    if (data != NULL && data_size > 0)
    {
        copy_size = (data_size <= sizeof(UTRX_AppData.RptPkt.Report.ReturnValue))
                        ? data_size
                        : (uint16)sizeof(UTRX_AppData.RptPkt.Report.ReturnValue);
    }

    memset(&UTRX_AppData.RptPkt, 0, sizeof(UTRX_AppData.RptPkt));
    CFE_MSG_Init(CFE_MSG_PTR(UTRX_AppData.RptPkt.TelemetryHeader), CFE_SB_ValueToMsgId(UTRX_RPT_TLM_MID),
                 sizeof(UTRX_AppData.RptPkt));
    UTRX_AppData.RptPkt.Report.MsgID          = UTRX_CMD_MID;
    UTRX_AppData.RptPkt.Report.CommandCode    = command_code;
    UTRX_AppData.RptPkt.Report.ReturnType     = return_type;
    UTRX_AppData.RptPkt.Report.ReturnCode     = status;
    UTRX_AppData.RptPkt.Report.ReturnDataSize = copy_size;
    if (copy_size > 0)
    {
        memcpy(UTRX_AppData.RptPkt.Report.ReturnValue, data, copy_size);
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UTRX_AppData.RptPkt.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(UTRX_AppData.RptPkt.TelemetryHeader), true);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t UTRX_SendHkCmd(const CFE_SB_Buffer_t *SBBufPtr)
{
    UTRX_ReportHousekeeping();
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* UTRX NOOP commands                                                         */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t UTRX_NoopCmd(const UTRX_NoopCmd_t *Msg)
{
    static const char NoopReport[] = "Yosi In Space";

    UTRX_AppData.CmdCounter++;

    CFE_EVS_SendEvent(UTRX_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "UTRX: NOOP command");
    UTRX_SendCmdReport(UTRX_NOOP_CC, CFE_SUCCESS, NoopReport, sizeof(NoopReport), RPT_RETTYPE_SUCCESS);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t UTRX_ResetCountersCmd(const UTRX_ResetCountersCmd_t *Msg)
{
    uint8 counters[3];

    UTRX_AppData.CmdCounter = 0;
    UTRX_AppData.AppErrCounter = 0;
    UTRX_AppData.DeviceErrCounter = 0;

    CFE_EVS_SendEvent(UTRX_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "UTRX: RESET command");
    counters[0] = UTRX_AppData.CmdCounter;
    counters[1] = UTRX_AppData.AppErrCounter;
    counters[2] = UTRX_AppData.DeviceErrCounter;
    UTRX_SendCmdReport(UTRX_RESET_COUNTERS_CC, CFE_SUCCESS, counters, sizeof(counters), RPT_RETTYPE_SUCCESS);

    return CFE_SUCCESS;
}


/**********************************************************************************/
CFE_Status_t UTRX_ResetDeviceCmdCountersCmd(const UTRX_ResetDeviceCmdCountersCmd_t *Msg)
{
    uint8 counters[3];

    UTRX_AppData.CmdCounter = 0;
    UTRX_AppData.DeviceErrCounter = 0;

    CFE_EVS_SendEvent(UTRX_RESET_DEVICE_COUNTER_EID, CFE_EVS_EventType_INFORMATION,
                      "UTRX: Reset Device Cmd Counters");
    counters[0] = UTRX_AppData.CmdCounter;
    counters[1] = UTRX_AppData.AppErrCounter;
    counters[2] = UTRX_AppData.DeviceErrCounter;
    UTRX_SendCmdReport(UTRX_RESET_DEVICE_CMD_COUNTERS_CC, CFE_SUCCESS, counters, sizeof(counters),
                       RPT_RETTYPE_SUCCESS);

    return CFE_SUCCESS;
}

CFE_Status_t UTRX_ResetAppCmdCountersCmd(const UTRX_ResetAppCmdCountersCmd_t *Msg)
{
    uint8 counters[3];

    UTRX_AppData.CmdCounter = 0;
    UTRX_AppData.AppErrCounter = 0;

    CFE_EVS_SendEvent(UTRX_RESET_APP_COUNTER_EID, CFE_EVS_EventType_INFORMATION,
                      "UTRX: Reset App Cmd Counters");
    counters[0] = UTRX_AppData.CmdCounter;
    counters[1] = UTRX_AppData.AppErrCounter;
    counters[2] = UTRX_AppData.DeviceErrCounter;
    UTRX_SendCmdReport(UTRX_RESET_APP_CMD_COUNTERS_CC, CFE_SUCCESS, counters, sizeof(counters),
                       RPT_RETTYPE_SUCCESS);

    return CFE_SUCCESS;
}

/* retType: CMD_RETCODE_TYPE_DRIVER 이면 드라이버(하드웨어) 커맨드, 그 외는 앱/CFEs 레벨 커맨드
   retCode: 드라이버면 DEVICE_SUCCESS와 비교, 앱/CFEs면 CFE_SUCCESS와 비교 */
void CmdErrCounter(uint8 *CmdCounter,
                   uint8 *AppErrCounter,
                   uint8 *DeviceErrCounter,
                   uint8  retType,
                   int32  retCode)
{
    if (!CmdCounter || !AppErrCounter || !DeviceErrCounter)
        return;

    /* 모든 커맨드에 대해 공통 커맨드 카운터 증가 */
    (*CmdCounter)++;

    // 순서 1. TYPE 검증을 먼저 했다 retType == CMD_RETCODE_TYPE_DRIVER면 Device 함수에서 오류난겨
    // 그 외 retType == CMD_RETCODE_TYPE_APP 으로 간주하고 APP에서 오류났다고 생각하는겨
    // 순서 2. 함수 호출했을때 status가 0 이 아니면 오류가 난거니까, return code 즉 retcode가 0 이 아니면 ErrCounter를 하는거제

     if (retType == CMD_RETCODE_TYPE_DRIVER) {
        /* 드라이버/하드웨어 커맨드 실패만 집계 */
        if (retCode != DEVICE_SUCCESS) {
            (*DeviceErrCounter)++;
        }
    } else {
        /* 앱/CFEs 레벨 실패 집계 (예: CMD_RETCODE_TYPE_APP 등) */
        if (retCode != CFE_SUCCESS) {
            (*AppErrCounter)++;
        }
    }
}




/*****************Housekeeping, Beacon *****************************/

void UTRX_ReportHousekeeping(void)
{
    UTRX_HkTlm_Payload_t hk;
    uint32 errmask = 0;

    memset(&hk, 0, sizeof(hk));

    if (UTRX_TLM_GetTempBrd(&hk.TempBrd)       != DEVICE_SUCCESS) errmask |= (1u << 0);
    if (UTRX_TLM_GetLastRssi(&hk.LastRssi)     != DEVICE_SUCCESS) errmask |= (1u << 1);
    if (UTRX_TLM_GetLastRferr(&hk.LastRferr)   != DEVICE_SUCCESS) errmask |= (1u << 2);
    if (UTRX_TLM_GetActiveConf(&hk.ActiveConf) != DEVICE_SUCCESS) errmask |= (1u << 3);
    if (UTRX_TLM_GetBootCount(&hk.BootCount)   != DEVICE_SUCCESS) errmask |= (1u << 4);
    if (UTRX_TLM_GetBootCause(&hk.BootCause)   != DEVICE_SUCCESS) errmask |= (1u << 5);
    if (UTRX_TLM_GetLastContact(&hk.LastContact)!= DEVICE_SUCCESS) errmask |= (1u << 6);
    if (UTRX_TLM_GetTotTxBytes(&hk.TotTxBytes) != DEVICE_SUCCESS) errmask |= (1u << 7);
    if (UTRX_TLM_GetTotRxBytes(&hk.TotRxBytes) != DEVICE_SUCCESS) errmask |= (1u << 8);

    if (errmask != 0u) {
        OS_printf("[UTRX][HK] collected with errors mask=0x%08X\n", (unsigned)errmask);
    }

    UTRX_SendCmdReport(0, CFE_SUCCESS, &hk, sizeof(hk), RPT_RETTYPE_SUCCESS);

    OS_printf("[UTRX][HK] temp=%d, rssi=%d, rferr=%d, act=%u, boot_cnt=%u, cause=0x%08X, "
              "last=%u, tx=%u, rx=%u\n",
              (int)hk.TempBrd, (int)hk.LastRssi, (int)hk.LastRferr,
              (unsigned)hk.ActiveConf, (unsigned)hk.BootCount,
              (unsigned)hk.BootCause, (unsigned)hk.LastContact,
              (unsigned)hk.TotTxBytes, (unsigned)hk.TotRxBytes);
}


void UTRX_ReportBeacon(void)
{
    int32 Status;
    UTRX_BcnTlm_t *BufPtr = (UTRX_BcnTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(UTRX_BcnTlm_t));
    if (BufPtr == NULL) return;

    Status = CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader),
                        CFE_SB_ValueToMsgId(UTRX_BCN_TLM_MID), sizeof(UTRX_BcnTlm_t));
    if (Status != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        return;
    }

    UTRX_BcnTlm_Payload_t *bcn = &BufPtr->Payload;

    uint8 errmask = 0;
    if (CFE_PUT_VALUE_TO_STRUCT(uint8, &bcn->ActiveConf, UTRX_TLM_GetActiveConf, DEVICE_SUCCESS) != DEVICE_SUCCESS) {
        errmask |= (1u << 0);
    }
    if (CFE_PUT_VALUE_TO_STRUCT(uint16, &bcn->BootCount, UTRX_TLM_GetBootCount, DEVICE_SUCCESS) != DEVICE_SUCCESS) {
        errmask |= (1u << 1);
    }
    if (CFE_PUT_VALUE_TO_STRUCT(uint32, &bcn->BootCause, UTRX_TLM_GetBootCause, DEVICE_SUCCESS) != DEVICE_SUCCESS) {
        errmask |= (1u << 2);
    }
    if (CFE_PUT_VALUE_TO_STRUCT(int16, &bcn->TempBrd, UTRX_TLM_GetTempBrd, DEVICE_SUCCESS) != DEVICE_SUCCESS) {
        errmask |= (1u << 3);
    }

  if (errmask != 0u) {
    OS_printf("[UTRX][BCN] collected with errors mask=0x%02X\n",
              (unsigned)errmask);
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
    Status = CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true);
    if (Status != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        return;
    }

    OS_printf("[UTRX][BCN] act=%u, boot_cnt=%u, cause=0x%08X\n",
            bcn->ActiveConf, bcn->BootCount, 
            bcn->BootCause);

}
