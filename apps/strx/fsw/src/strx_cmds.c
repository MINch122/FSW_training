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
 *   This file contains the source code for the Strx App Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "strx_app.h"
#include "strx.h"
#include "strx_cmds.h"
#include "strx_msgids.h"
#include "strx_eventids.h"
#include "strx_msg.h"

#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include "rpt_interface_cfg.h"


/* The strx_lib module provides the STRX_Function() prototype */
/* RPT */
static inline void STRX_RptBegin(void)
{
    CFE_MSG_Init(CFE_MSG_PTR(STRX_AppData.RptPkt.TelemetryHeader),
                 CFE_SB_ValueToMsgId(STRX_RPT_TLM_MID),  /* <- 앱에서 정의한 TLM MID 사용 */
                 sizeof(STRX_AppData.RptPkt));             /* 통째로 전송 */
}

static inline void STRX_RptSetStatusAuto(uint8_t cc, int32_t status)
{
    STRX_AppData.RptPkt.Report.CommandCode = cc;

    if (status == DEVICE_SUCCESS) {
        STRX_AppData.RptPkt.Report.ReturnType = RPT_RETTYPE_SUCCESS;
        STRX_AppData.RptPkt.Report.ReturnCode = DEVICE_SUCCESS;
    } else {
        STRX_AppData.RptPkt.Report.ReturnType = RPT_RETTYPE_HW;
        STRX_AppData.RptPkt.Report.ReturnCode = status;
    }
}

/* 스칼라/가변 모두 사용 가능: 길이 상한 및 ReturnDataSize 설정 포함 */
static inline void STRX_RptCopy(const void *src, size_t len)
{
    size_t n = len;

    /* 필수: ReturnValue 버퍼 크기 이하로 자르기 (오버런 방지) */
    size_t maxbuf = sizeof(STRX_AppData.RptPkt.Report.ReturnValue);
    if (n > maxbuf) n = maxbuf;

    memcpy(STRX_AppData.RptPkt.Report.ReturnValue, src, n);
    STRX_AppData.RptPkt.Report.ReturnDataSize = (uint16_t)n;
}


static inline void STRX_RptEnd(void)
{
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(STRX_AppData.RptPkt.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(STRX_AppData.RptPkt.TelemetryHeader), true);
}

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


void STRX_ReportHousekeeping(void)
{
    int32 Status;

    STRX_HkTlm_t *BufPtr = (STRX_HkTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(STRX_HkTlm_t));
    if (BufPtr == NULL) return;

    Status = CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader),
                        CFE_SB_ValueToMsgId(STRX_HK_TLM_MID), sizeof(STRX_HkTlm_t));
    if (Status != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        return;
    }

    STRX_HkTlm_Payload_t* hk = &BufPtr->Payload;

    if (STRX_RXCONF_GetFreq(&hk->rx_freq) != DEVICE_SUCCESS) STRX_AppData.AppCnt.DeviceErrCounter++;
    if (STRX_RXCONF_GetBaud(&hk->rx_baud) != DEVICE_SUCCESS) STRX_AppData.AppCnt.DeviceErrCounter++;
    if (STRX_RXCONF_GetGuard(&hk->rx_guard) != DEVICE_SUCCESS) STRX_AppData.AppCnt.DeviceErrCounter++;
    if (STRX_TLM_GetLastRssi(&hk->LastRssi) != DEVICE_SUCCESS) STRX_AppData.AppCnt.DeviceErrCounter++;
    if (STRX_TLM_GetBootCount(&hk->BootCount) != DEVICE_SUCCESS) STRX_AppData.AppCnt.DeviceErrCounter++;
    if (STRX_TLM_GetBootCause(&hk->BootCause) != DEVICE_SUCCESS) STRX_AppData.AppCnt.DeviceErrCounter++;
    if (STRX_TLM_GetTotTxBytes(&hk->TotTxBytes) != DEVICE_SUCCESS) STRX_AppData.AppCnt.DeviceErrCounter++;
    if (STRX_TLM_GetTotRxBytes(&hk->TotRxBytes) != DEVICE_SUCCESS) STRX_AppData.AppCnt.DeviceErrCounter++;
    if (STRX_TLM_GET_HWDET(&hk->hw_det) != DEVICE_SUCCESS) STRX_AppData.AppCnt.DeviceErrCounter++;
    if (STRX_TLM_GET_RXMODE(&hk->rxmode) != DEVICE_SUCCESS) STRX_AppData.AppCnt.DeviceErrCounter++;
    if (STRX_TLM_GET_GND_WDT_CNT(&hk->gnd_wdt_cnt) != DEVICE_SUCCESS) STRX_AppData.AppCnt.DeviceErrCounter++;
    if (STRX_TLM_GET_GND_WDT_LEFT(&hk->gnd_wdt_left) != DEVICE_SUCCESS) STRX_AppData.AppCnt.DeviceErrCounter++;
    if (STRX_TXCONF_GetFreq(&hk->tx_freq) != DEVICE_SUCCESS) STRX_AppData.AppCnt.DeviceErrCounter++;
    if (STRX_TXCONF_GetBaud(&hk->tx_baud) != DEVICE_SUCCESS) STRX_AppData.AppCnt.DeviceErrCounter++;
    if (STRX_TXCONF_GetGuard(&hk->tx_guard) != DEVICE_SUCCESS) STRX_AppData.AppCnt.DeviceErrCounter++;
    if (STRX_TLM_GetRssibusy(&hk->rssibusy) != DEVICE_SUCCESS) STRX_AppData.AppCnt.DeviceErrCounter++;

    OS_printf("[STRX] RX FREQ: %u\n", hk->rx_freq);
    OS_printf("[STRX] TX FREQ: %u\n", hk->tx_freq);
    OS_printf("[STRX] HK succes\n");

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
    Status = CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true);
    if (Status != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        return;
    }
}

void STRX_ReportBeacon(void)
{
    int32 Status;
    STRX_BcnTlm_t *BufPtr = (STRX_BcnTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(STRX_BcnTlm_t));
    if (BufPtr == NULL) return;

    Status = CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader),
                        CFE_SB_ValueToMsgId(STRX_BCN_TLM_MID), sizeof(STRX_BcnTlm_t));
    if (Status != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        return;
    }
    STRX_BcnTlm_Payload_t* bcn = &BufPtr->Payload;

    if (CFE_PUT_VALUE_TO_STRUCT(int16, &bcn->LastRssi, STRX_TLM_GetLastRssi, DEVICE_SUCCESS) != DEVICE_SUCCESS) {
        STRX_AppData.AppCnt.DeviceErrCounter++;
    }
    if (CFE_PUT_VALUE_TO_STRUCT(uint16, &bcn->BootCount, STRX_TLM_GetBootCount, DEVICE_SUCCESS) != DEVICE_SUCCESS) {
        STRX_AppData.AppCnt.DeviceErrCounter++;
    }
    if (CFE_PUT_VALUE_TO_STRUCT(uint32, &bcn->BootCause, STRX_TLM_GetBootCause, DEVICE_SUCCESS) != DEVICE_SUCCESS) {
        STRX_AppData.AppCnt.DeviceErrCounter++;
    }
    if (CFE_PUT_VALUE_TO_STRUCT(uint8, &bcn->rxmode, STRX_TLM_GET_RXMODE, DEVICE_SUCCESS) != DEVICE_SUCCESS) {
        STRX_AppData.AppCnt.DeviceErrCounter++;
    }
    if (CFE_PUT_VALUE_TO_STRUCT(uint16, &bcn->gnd_wdt_cnt, STRX_TLM_GET_GND_WDT_CNT, DEVICE_SUCCESS) != DEVICE_SUCCESS) {
        STRX_AppData.AppCnt.DeviceErrCounter++;
    }
    if (CFE_PUT_VALUE_TO_STRUCT(uint32, &bcn->gnd_wdt_left, STRX_TLM_GET_GND_WDT_LEFT, DEVICE_SUCCESS) != DEVICE_SUCCESS) {
        STRX_AppData.AppCnt.DeviceErrCounter++;
    }
    if (CFE_PUT_VALUE_TO_STRUCT(int16, &bcn->TempBrd, STRX_TLM_GetTempBrd, DEVICE_SUCCESS) != DEVICE_SUCCESS) {
        STRX_AppData.AppCnt.DeviceErrCounter++;
    }
    // if (STRX_TLM_GetLastRssi(&bcn->LastRssi)      != DEVICE_SUCCESS) STRX_AppData.AppCnt.DeviceErrCounter++;
    // if (STRX_TLM_GetBootCount(&bcn->BootCount)    != DEVICE_SUCCESS) STRX_AppData.AppCnt.DeviceErrCounter++;
    // if (STRX_TLM_GetBootCause(&bcn->BootCause)    != DEVICE_SUCCESS) STRX_AppData.AppCnt.DeviceErrCounter++;
    // if (STRX_TLM_GET_RXMODE(&bcn->rxmode)         != DEVICE_SUCCESS) STRX_AppData.AppCnt.DeviceErrCounter++;
    // if (STRX_TLM_GET_GND_WDT_CNT(&bcn->gnd_wdt_cnt)   != DEVICE_SUCCESS) STRX_AppData.AppCnt.DeviceErrCounter++;
    // if (STRX_TLM_GET_GND_WDT_LEFT(&bcn->gnd_wdt_left) != DEVICE_SUCCESS) STRX_AppData.AppCnt.DeviceErrCounter++;


    OS_printf("[STRX] RX FREQ: %u\n", bcn->BootCount);
    OS_printf("[STRX] TX FREQ: %u\n", bcn->BootCause);
    OS_printf("[STRX] bcn succes\n");


    CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
    Status = CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true);
    if (Status != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
        return;
    }
}
    
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* STRX NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t STRX_NoopCmd(const STRX_NoopCmd_t *Msg)
{

    CFE_EVS_SendEvent(STRX_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "STRX: NOOP command");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t STRX_ResetCountersCmd(const STRX_ResetCountersCmd_t *Msg)
{

    STRX_AppData.AppCnt.AppCmdCounter = 0;
    STRX_AppData.AppCnt.AppErrCounter = 0;
    STRX_AppData.AppCnt.DeviceErrCounter = 0;

    CFE_EVS_SendEvent(STRX_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "STRX: RESET count command");

    return CFE_SUCCESS;
}

/**********************************************************************************/
CFE_Status_t STRX_ResetDeviceCmdCountersCmd(const STRX_ResetDeviceCmdCountersCmd_t *Msg)
{


    STRX_AppData.AppCnt.DeviceErrCounter = 0;
    CFE_EVS_SendEvent(STRX_RESET_DEVICE_COUNTER_EID, CFE_EVS_EventType_INFORMATION,
                      "STRX: Reset Device Cmd Counters");

    return CFE_SUCCESS;
}

CFE_Status_t STRX_ResetAppCmdCountersCmd(const STRX_ResetAppCmdCountersCmd_t *Msg)
{
    STRX_AppData.AppCnt.AppCmdCounter = 0;
    STRX_AppData.AppCnt.AppErrCounter = 0;

    CFE_EVS_SendEvent(STRX_RESET_APP_COUNTER_EID, CFE_EVS_EventType_INFORMATION,
                      "STRX: Reset App Cmd Counters");

    return CFE_SUCCESS;
}

/*
STRX GROUND COMMAND
*/

void STRX_GndwdtClearCmd(void){

    STRX_RptBegin();

    int status     = STRX_GndwdtClear();

    STRX_RptSetStatusAuto(STRX_GNDWDT_CLEAR_CC, status);
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_GNDWDT_CLEAR_CC, status);
    }
    STRX_RptEnd();

}

void STRX_RebootCmd(void){
    STRX_RptBegin();
    int status     = STRX_Reboot();

    STRX_RptSetStatusAuto(STRX_REBOOT_CC, status);
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_REBOOT_CC, status);
    }
    STRX_RptEnd();
}

void STRX_RXCONF_SetBaudCmd(const STRX_U32ArgsCmd_t * SBBufPtr){
    STRX_RptBegin();

    int status     = STRX_RXCONF_SetBaud(SBBufPtr->arg);

    STRX_RptSetStatusAuto(STRX_RXCONF_SET_BAUD_CC, status);
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed (arg=%u), Status=%" PRId32,
                          STRX_RXCONF_SET_BAUD_CC, (unsigned)SBBufPtr->arg, status);
    }
    STRX_RptEnd();
}

void STRX_TXCONF_SetBaudCmd(const STRX_U32ArgsCmd_t * SBBufPtr){
    STRX_RptBegin();


    int status     = STRX_TXCONF_SetBaud(SBBufPtr->arg);

    STRX_RptSetStatusAuto(STRX_TXCONF_SET_BAUD_CC, status);
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed (arg=%u), Status=%" PRId32,
                          STRX_TXCONF_SET_BAUD_CC, (unsigned)SBBufPtr->arg, status);
    }
    STRX_RptEnd();
}

void STRX_RXCONF_SetFreqCmd(const STRX_U32ArgsCmd_t * SBBufPtr){
    STRX_RptBegin();
 
    int status     = STRX_RXCONF_SetFreq( SBBufPtr->arg);

    STRX_RptSetStatusAuto(STRX_RXCONF_SET_FREQ_CC, status);
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed (arg=%u), Status=%" PRId32,
                          STRX_RXCONF_SET_FREQ_CC, (unsigned)SBBufPtr->arg, status);
    }
    STRX_RptEnd();
 
}

void STRX_TXCONF_SetFreqCmd(const STRX_U32ArgsCmd_t * SBBufPtr){
    STRX_RptBegin();
 
    int status     = STRX_TXCONF_SetFreq( SBBufPtr->arg);
  
    STRX_RptSetStatusAuto(STRX_TXCONF_SET_FREQ_CC, status);
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed (arg=%u), Status=%" PRId32,
                          STRX_TXCONF_SET_FREQ_CC, (unsigned)SBBufPtr->arg, status);
    }
    STRX_RptEnd();    

}

void STRX_TLM_SET_KISS_USARTCmd(const STRX_8ArgsCmd_t * SBBufPtr){
    STRX_RptBegin();

    int status     = STRX_TLM_SET_KISS_USART( SBBufPtr->arg);

    STRX_RptSetStatusAuto(STRX_TLM_SET_KISS_USART_CC, status);
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_TLM_SET_KISS_USART_CC, status);
    }
    STRX_RptEnd();

}

void STRX_TLM_SET_GOSH_USARTTCmd(const STRX_U8ArgsCmd_t * SBBufPtr){
    STRX_RptBegin();

    int status     = STRX_TLM_SET_GOSH_USART(SBBufPtr->arg);

    STRX_RptSetStatusAuto(STRX_TLM_SET_GOSH_USART_CC, status);
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_TLM_SET_GOSH_USART_CC, status);
    }
    STRX_RptEnd();
    
}

void STRX_SetDefaultBaudCmd(void){
    STRX_RptBegin();
    int status     = STRX_SetDefaultBaud();

    STRX_RptSetStatusAuto(STRX_SET_DEFAULT_BAUD_CC, status);
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_SET_DEFAULT_BAUD_CC, status);
    }
    STRX_RptEnd();

}

void STRX_RparamSave0Cmd(void){
    STRX_RptBegin();

    int status     = STRX_RparamSave0();

    STRX_RptSetStatusAuto(STRX_RPARAM_SAVE_0_CC, status);
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_RPARAM_SAVE_0_CC, status);
    }
    STRX_RptEnd();

}

void STRX_RparamSave1Cmd(void){
    STRX_RptBegin();

    int status     = STRX_RparamSave1();

    STRX_RptSetStatusAuto(STRX_RPARAM_SAVE_1_CC, status);
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_RPARAM_SAVE_1_CC, status);
    }
    STRX_RptEnd();

}

void STRX_RparamSave4Cmd(void){
    STRX_RptBegin();
    int status     = STRX_RparamSave4();

    STRX_RptSetStatusAuto(STRX_RPARAM_SAVE_4_CC, status);
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_RPARAM_SAVE_4_CC, status);
    }
    STRX_RptEnd();

}

void STRX_RparamSave5Cmd(void){
    STRX_RptBegin();
    int status     = STRX_RparamSave5();

    STRX_RptSetStatusAuto(STRX_RPARAM_SAVE_5_CC, status);
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_RPARAM_SAVE_5_CC, status);
    }
    STRX_RptEnd();

}

void STRX_RparamSaveAllCmd(void){
    STRX_RptBegin();
    int status     = STRX_RparamSaveAll();

    STRX_RptSetStatusAuto(STRX_RPARAM_SAVE_ALL_CC, status);
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_RPARAM_SAVE_ALL_CC, status);
    }
    STRX_RptEnd();

}

void csp_checkstate_pingCmd(void){
    STRX_RptBegin();
    int status     = csp_checkstate_ping(CSP_NODE_STRX);

    STRX_RptSetStatusAuto(STRX_CHECK_STATE_PING_CC, status);
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_CHECK_STATE_PING_CC, status);
    }
    STRX_RptEnd();

}
/*
STRX GET COMMAND
*/
void STRX_RXCONF_GetBaudCmd(void){

    STRX_RptBegin();

    uint32  baud = 0;

    int status = STRX_RXCONF_GetBaud(&baud);

    STRX_RptSetStatusAuto(STRX_RXCONF_GET_BAUD_CC, status);
    if (status == DEVICE_SUCCESS) STRX_RptCopy(&baud, sizeof(baud));
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_RXCONF_GET_BAUD_CC, status);
    } else {
        OS_printf("[STRX] RX Baud: %u\n", (unsigned)baud);
    }
    STRX_RptEnd();

}

void STRX_RXCONF_GetGuardCmd(void){
    STRX_RptBegin();

    uint16  val = 0;
    int status = STRX_RXCONF_GetGuard(&val);

    STRX_RptSetStatusAuto(STRX_RXCONF_GET_GUARD_CC, status);
    if (status == DEVICE_SUCCESS) STRX_RptCopy(&val, sizeof(val));
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_RXCONF_GET_GUARD_CC, status);
    } else {
        OS_printf("[STRX] RX GUARD: %u\n", (unsigned)val);
    }
    STRX_RptEnd();
  
}

void STRX_RXCONF_GetFreqCmd(void){
    STRX_RptBegin();

    uint32  val = 0;
    int status = STRX_RXCONF_GetFreq(&val);

    STRX_RptSetStatusAuto(STRX_RXCONF_GET_FREQ_CC, status);
    if (status == DEVICE_SUCCESS) STRX_RptCopy(&val, sizeof(val));
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_RXCONF_GET_FREQ_CC, status);
    } else {
        OS_printf("[STRX] RX Freq: %u\n", (unsigned)val);
    }
    STRX_RptEnd();

}

void STRX_TXCONF_GetBaudCmd(void){
    STRX_RptBegin();

    uint32  val = 0;
    int status = STRX_TXCONF_GetBaud(&val);

    STRX_RptSetStatusAuto(STRX_TXCONF_GET_BAUD_CC, status);
    if (status == DEVICE_SUCCESS) STRX_RptCopy(&val, sizeof(val));
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_TXCONF_GET_BAUD_CC, status);
    } else {
        OS_printf("[STRX] TX Baud: %u\n", (unsigned)val);
    }
    STRX_RptEnd();

}

void STRX_TXCONF_GetFreqCmd(void){
        STRX_RptBegin();
    
    uint32  val = 0;
    int status = STRX_TXCONF_GetFreq(&val);

    STRX_RptSetStatusAuto(STRX_TXCONF_GET_FREQ_CC, status);
    if (status == DEVICE_SUCCESS) STRX_RptCopy(&val, sizeof(val));
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_TXCONF_GET_FREQ_CC, status);
    } else {
        OS_printf("[STRX] TX Freq: %u\n", (unsigned)val);
    }
    STRX_RptEnd();    

}

void STRX_TLM_GetTempBrdCmd(void){
    STRX_RptBegin();
    
    int16_t val = 0;
    int status = STRX_TLM_GetTempBrd(&val);

    STRX_RptSetStatusAuto(STRX_TLM_GET_TEMP_BRD_CC, status);
    if (status == DEVICE_SUCCESS) STRX_RptCopy(&val, sizeof(val));
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_TLM_GET_TEMP_BRD_CC, status);
    } else {
        OS_printf("[STRX] TEMPERATURE: %d\n", (int)val);
    }
    STRX_RptEnd();

}

void STRX_TLM_GetLastRssiCmd(void){
    STRX_RptBegin();
    
    int16_t val = 0;
    int status = STRX_TLM_GetLastRssi(&val);

    STRX_RptSetStatusAuto(STRX_TLM_GET_LAST_RSSI_CC, status);
    if (status == DEVICE_SUCCESS) STRX_RptCopy(&val, sizeof(val));
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_TLM_GET_LAST_RSSI_CC, status);
    } else {
        OS_printf("[STRX] LAST RSSI: %d\n", (int)val);
    }
    STRX_RptEnd();

}

void STRX_TLM_GetLastRferrCmd(void){
        STRX_RptBegin();
   
    int16_t val = 0;
    int status = STRX_TLM_GetLastRferr(&val);

    STRX_RptSetStatusAuto(STRX_TLM_GET_LAST_RFERR_CC, status);
    if (status == DEVICE_SUCCESS) STRX_RptCopy(&val, sizeof(val));
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_TLM_GET_LAST_RFERR_CC, status);
    } else {
        OS_printf("[STRX] LAST RFERR: %d\n", (int)val);
    }
    STRX_RptEnd();

}

void STRX_TLM_GetBootCountCmd(void){
    STRX_RptBegin();
    
    uint16  val = 0;
    int status = STRX_TLM_GetBootCount(&val);

    STRX_RptSetStatusAuto(STRX_TLM_GET_BOOT_COUNT_CC, status);
    if (status == DEVICE_SUCCESS) STRX_RptCopy(&val, sizeof(val));
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_TLM_GET_BOOT_COUNT_CC, status);
    } else {
        OS_printf("[STRX] BOOT COUNT: %u\n", (unsigned)val);
    }
    STRX_RptEnd();

}

void STRX_TLM_GetBootCauseCmd(void){
    STRX_RptBegin();
    
    uint32  val = 0;
    int status = STRX_TLM_GetBootCause(&val);

    STRX_RptSetStatusAuto(STRX_TLM_GET_BOOT_CAUSE_CC, status);
    if (status == DEVICE_SUCCESS) STRX_RptCopy(&val, sizeof(val));
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_TLM_GET_BOOT_CAUSE_CC, status);
    } else {
        OS_printf("[STRX] BOOT CAUSE: %u\n", (unsigned)val);
    }
    STRX_RptEnd();

}

void STRX_TLM_GetLastContactCmd(void){
    STRX_RptBegin();
    
    uint32  val = 0;
    int status = STRX_TLM_GetLastContact(&val);

    STRX_RptSetStatusAuto(STRX_TLM_GET_LAST_CONTACT_CC, status);
    if (status == DEVICE_SUCCESS) STRX_RptCopy(&val, sizeof(val));
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_TLM_GET_LAST_CONTACT_CC, status);
    } else {
        OS_printf("[STRX] LAST CONTACT: %u\n", (unsigned)val);
    }
    STRX_RptEnd();    

}

void STRX_TLM_GetTotTxBytesCmd(void){
    STRX_RptBegin();
    
    uint32  val = 0;
    int status = STRX_TLM_GetTotTxBytes(&val);

    STRX_RptSetStatusAuto(STRX_TLM_GET_TOT_TX_BYTES_CC, status);
    if (status == DEVICE_SUCCESS) STRX_RptCopy(&val, sizeof(val));
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_TLM_GET_TOT_TX_BYTES_CC, status);
    } else {
        OS_printf("[STRX] TOTAL TX BYTES: %u\n", (unsigned)val);
    }
    STRX_RptEnd();

}

void STRX_TLM_GetTotRxBytesCmd(void){
    STRX_RptBegin();

    uint32  val = 0;
    int status = STRX_TLM_GetTotRxBytes(&val);

    STRX_RptSetStatusAuto(STRX_TLM_GET_TOT_RX_BYTES_CC, status);
    if (status == DEVICE_SUCCESS) STRX_RptCopy(&val, sizeof(val));
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_TLM_GET_TOT_RX_BYTES_CC, status);
    } else {
        OS_printf("[STRX] TOTAL RX BYTES: %u\n", (unsigned)val);
    }
    STRX_RptEnd();

}

void STRX_TLM_GET_RXMODECmd(void){
    STRX_RptBegin();

    uint8_t  val = 0;
    int status = STRX_TLM_GET_RXMODE(&val);

    STRX_RptSetStatusAuto(STRX_TLM_RXMODE_CC , status);
    if (status == DEVICE_SUCCESS) STRX_RptCopy(&val, sizeof(val));
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_TLM_RXMODE_CC, status);
    } else {
        OS_printf("[STRX] RXMODE: %u\n", (unsigned)val);
    }
    STRX_RptEnd();

}

void STRX_TLM_GET_GND_WDT_CNTCmd(void){
    STRX_RptBegin();

    uint16_t  val = 0;
    int status = STRX_TLM_GET_GND_WDT_CNT(&val);

    STRX_RptSetStatusAuto(STRX_TLM_GET_GNDWDT_CNT_CC , status);
    if (status == DEVICE_SUCCESS) STRX_RptCopy(&val, sizeof(val));
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_TLM_GET_GNDWDT_CNT_CC, status);
    } else {
        OS_printf("[STRX] WDT_CNT: %u\n", (unsigned)val);
    }
    STRX_RptEnd();
}

void STRX_TLM_GET_GND_WDT_LEFTCmd(void){
    STRX_RptBegin();

    uint32_t  val = 0;
    int status = STRX_TLM_GET_GND_WDT_LEFT(&val);

    STRX_RptSetStatusAuto(STRX_TLM_GET_GNDWDT_LEFT_CC , status);
    if (status == DEVICE_SUCCESS) STRX_RptCopy(&val, sizeof(val));
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_TLM_GET_GNDWDT_LEFT_CC, status);
    } else {
        OS_printf("[STRX] WDT_LEFT: %u\n", (unsigned)val);
    }
    STRX_RptEnd();

}

void STRX_TLM_GET_KISS_USARTCmd(void){
    STRX_RptBegin();

    int8_t  val = 0;
    int status = STRX_TLM_GET_KISS_USART(&val);

    STRX_RptSetStatusAuto(STRX_TLM_GET_KISS_USART_CC , status);
    if (status == DEVICE_SUCCESS) STRX_RptCopy(&val, sizeof(val));
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_TLM_GET_KISS_USART_CC, status);
    } else {
        OS_printf("[STRX] KISS USART: %u\n", (unsigned)val);
    }
    STRX_RptEnd();

}

void STRX_TLM_GET_GOSH_USARTCmd(void){
    STRX_RptBegin();

    uint8_t  val = 0;
    int status = STRX_TLM_GET_GOSH_USART(&val);

    STRX_RptSetStatusAuto(STRX_TLM_GET_GOSH_USART_CC , status);
    if (status == DEVICE_SUCCESS) STRX_RptCopy(&val, sizeof(val));
    STRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX AX2150: CC=%u failed, Status=%" PRId32,
                          STRX_TLM_GET_GOSH_USART_CC, status);
    } else {
        OS_printf("[STRX] GOSH USART: %u\n", (unsigned)val);
    }
    STRX_RptEnd();
}





