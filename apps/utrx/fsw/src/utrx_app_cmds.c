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
#include "utrx_app_cmds.h"
#include "utrx_app_msgids.h"
#include "utrx_app_eventids.h"
#include "utrx_app_utils.h"
#include "utrx_app_msg.h"
#include "utrx_app_dispatch.h"

/* The utrx_lib module provides the UTRX_Function() prototype */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t UTRX_APP_SendHkCmd(const CFE_SB_Buffer_t *SBBufPtr)
{
    UTRX_ReportHousekeeping();
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* UTRX NOOP commands                                                         */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t UTRX_APP_NoopCmd(const UTRX_APP_NoopCmd_t *Msg)
{
    UTRX_APP_Data.CmdCounter++;

    CFE_EVS_SendEvent(UTRX_APP_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "UTRX: NOOP command");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t UTRX_APP_ResetCountersCmd(const UTRX_APP_ResetCountersCmd_t *Msg)
{
    UTRX_APP_Data.CmdCounter = 0;
    UTRX_APP_Data.AppErrCounter = 0;
    UTRX_APP_Data.DeviceErrCounter = 0;

    CFE_EVS_SendEvent(UTRX_APP_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "UTRX: RESET command");

    return CFE_SUCCESS;
}


/**********************************************************************************/
CFE_Status_t UTRX_APP_ResetDeviceCmdCountersCmd(const UTRX_APP_ResetDeviceCmdCountersCmd_t *Msg)
{
    UTRX_APP_Data.CmdCounter = 0;
    UTRX_APP_Data.DeviceErrCounter = 0;

    CFE_EVS_SendEvent(UTRX_APP_RESET_DEVICE_COUNTER_EID, CFE_EVS_EventType_INFORMATION,
                      "UTRX: Reset Device Cmd Counters");

    return CFE_SUCCESS;
}

CFE_Status_t UTRX_APP_ResetAppCmdCountersCmd(const UTRX_APP_ResetAppCmdCountersCmd_t *Msg)
{
    UTRX_APP_Data.CmdCounter = 0;
    UTRX_APP_Data.AppErrCounter = 0;

    CFE_EVS_SendEvent(UTRX_APP_RESET_APP_COUNTER_EID, CFE_EVS_EventType_INFORMATION,
                      "UTRX: Reset App Cmd Counters");

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
    int32 Status;
    
    UTRX_HkTlm_t *BufPtr = (UTRX_HkTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(UTRX_HkTlm_t));
    if (BufPtr == NULL) return;

    Status = CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader),
                        CFE_SB_ValueToMsgId(UTRX_APP_HK_TLM_MID), sizeof(UTRX_HkTlm_t));
    if (Status != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        return;
    }

    /* Check the alive signal via PING */
    int32 ping_rc = csp_checkstate_ping(CSP_NODE_UTRX);
    if (ping_rc < 0) {
        OS_printf("[UTRX][HK] radio ping failed, skip HK (rc=%d)\n", (int)ping_rc);
        return;
    }

    UTRX_HkTlm_Payload_t *hk = &BufPtr->Payload;

    uint32 errmask = 0;
    if (UTRX_TLM_GetTempBrd(&hk->TempBrd)       != DEVICE_SUCCESS) errmask |= (1u << 0);
    if (UTRX_TLM_GetLastRssi(&hk->LastRssi)     != DEVICE_SUCCESS) errmask |= (1u << 1);
    if (UTRX_TLM_GetLastRferr(&hk->LastRferr)   != DEVICE_SUCCESS) errmask |= (1u << 2);
    if (UTRX_TLM_GetActiveConf(&hk->ActiveConf) != DEVICE_SUCCESS) errmask |= (1u << 3);
    if (UTRX_TLM_GetBootCount(&hk->BootCount)   != DEVICE_SUCCESS) errmask |= (1u << 4);
    if (UTRX_TLM_GetBootCause(&hk->BootCause)   != DEVICE_SUCCESS) errmask |= (1u << 5);
    if (UTRX_TLM_GetLastContact(&hk->LastContact)!= DEVICE_SUCCESS) errmask |= (1u << 6);
    if (UTRX_TLM_GetTotTxBytes(&hk->TotTxBytes) != DEVICE_SUCCESS) errmask |= (1u << 7);
    if (UTRX_TLM_GetTotRxBytes(&hk->TotRxBytes) != DEVICE_SUCCESS) errmask |= (1u << 8);

    if (errmask != 0u) {
        OS_printf("[UTRX][HK] collected with errors mask=0x%08X\n", (unsigned)errmask);
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
    Status = CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true);
    if (Status != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        return;
    }

    /* Debugging */
    OS_printf("[UTRX][HK] temp=%d, rssi=%d, rferr=%d, act=%u, boot_cnt=%u, cause=0x%08X, "
              "last=%u, tx=%u, rx=%u\n",
              (int)hk->TempBrd, (int)hk->LastRssi, (int)hk->LastRferr,
              (unsigned)hk->ActiveConf, (unsigned)hk->BootCount,
              (unsigned)hk->BootCause, (unsigned)hk->LastContact,
              (unsigned)hk->TotTxBytes, (unsigned)hk->TotRxBytes);
}


void UTRX_ReportBeacon(void)
{
    int32 Status;
    UTRX_BcnTlm_t *BufPtr = (UTRX_BcnTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(UTRX_BcnTlm_t));
    if (BufPtr == NULL) return;

    Status = CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader),
                        CFE_SB_ValueToMsgId(UTRX_APP_BCN_TLM_MID), sizeof(UTRX_BcnTlm_t));
    if (Status != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        return;
    }

    /* Check the alive signal via PING */
    int32 ping_rc = csp_checkstate_ping(CSP_NODE_UTRX);
     if (ping_rc < 0) {
        OS_printf("[UTRX][BCN] radio ping failed, skip beacon (rc=%d)\n", (int)ping_rc);
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
    // if (UTRX_TLM_GetActiveConf(&bcn->ActiveConf) != DEVICE_SUCCESS) errmask |= (1u << 0);
    // if (UTRX_TLM_GetBootCount(&bcn->BootCount)   != DEVICE_SUCCESS) errmask |= (1u << 1);
    // if (UTRX_TLM_GetBootCause(&bcn->BootCause)   != DEVICE_SUCCESS) errmask |= (1u << 2);

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