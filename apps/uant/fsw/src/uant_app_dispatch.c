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
 *   This file contains the source code for the Sample App.
 */

/*
** Include Files:
*/


#include "uant_app.h"
#include "uant_app_dispatch.h"
#include "uant_app_cmds.h"
#include "uant_app_eventids.h"
#include "uant_app_msgids.h"
#include "uant_app_msg.h"
#include "cfe_msg.h"
#include <gs/gssb/gssb_ant6.h>

#include <gs/gssb/internal/gssb_common.h>   /* gs_gssb_common_* 함수 선언 */
#include <gs/gssb/internal/gssb_cmd_id.h>   /* GSSB_CMD_ 열거형 */
#include <gs/gssb/gssb_autodeploy.h>




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool UANT_APP_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool              result       = true;
    size_t            ActualLength = 0;
    CFE_SB_MsgId_t    MsgId        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode      = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    /*
    ** Verify the command packet length.
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(UANT_APP_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        UANT_APP_Data.ErrCounter++;
    }

    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */

/* UANT ground commands or scheduled commands(from sch app/ ref. sch's table)                                                    */

/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void UANT_APP_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{   //CFE_Status_t     status; // 내부 판단용
    CFE_MSG_FcnCode_t cc = 0;
    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &cc);
    //gs_error_t   gs_st;

    switch (cc)
    {
        /* ───────── NOOP ───────── */
        case UANT_APP_NOOP_CC:
            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_NoopCmd_t))) //지상에서 보내준 것과 정의한 구조가 일치하는지 검증
            {
                UANT_APP_NoopCmd((const UANT_APP_NoopCmd_t *)SBBufPtr);
            }
            break;

        /* ───────── 카운터 리셋 ───────── */

        case UANT_APP_RESET_COUNTERS_CC:

            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_ResetCountersCmd_t)))
            {
                UANT_APP_ResetCountersCmd((const UANT_APP_ResetCountersCmd_t *)SBBufPtr);
            }
            break;


         /* ------------ Board Soft‑Reboot ------------ */
        case UANT_APP_SOFT_REBOOT_CC:

            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_SoftRebootCmd_t)))
            {
                UANT_APP_SoftReboot((const UANT_APP_SoftRebootCmd_t *)SBBufPtr);
            }
            break;


        /* ------------ Burn / Stop ------------ */

        case UANT_APP_BURN_CHANNEL_CC:

            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_BurnChannelCmd_t)))
            {
                UANT_APP_BurnChannel((const UANT_APP_BurnChannelCmd_t *)SBBufPtr);
            }
            break;

        case UANT_APP_STOP_BURN_CC:

            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_StopBurnCmd_t)))
            {
                 UANT_APP_StopBurn((const UANT_APP_StopBurnCmd_t *)SBBufPtr);
            }
            break;


                /* ---------- ANT‑6 자동 전개 ---------- */
        case UANT_APP_AUTODEPLOY_CC:

            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_AutodeployCmd_t)))
            {
                UANT_APP_AutoDeploy((const UANT_APP_AutodeployCmd_t *)SBBufPtr);
            }
            break;


        /* ---------- BOARD‑STATUS (uptime, reboot‑cnt) ---------- */
            //디스패치 , cmd 분리하기, 텔레메트리는 지상콜과 스케줄러가 주는 것 두가지가있음 << 분리해야함 각각 다른 버퍼에 담아서 뿌리기
        case UANT_APP_GET_BOARD_STATUS_CC:

            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_GetBoardStatusCmd_t)))
            {
                UANT_APP_GetBoardStatus((const UANT_APP_GetBoardStatusCmd_t *)SBBufPtr);
            }
            break;

        /* ---------- MCU 내부 온도 ---------- */
        case UANT_APP_GET_TEMPERATURE_CC:

            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_GetTemperatureCmd_t)))
            {
                UANT_APP_GetTemperature((const UANT_APP_GetTemperatureCmd_t *)SBBufPtr);
            }
            break;

       /* ------------ Telemetry pulls ------------ */

        case UANT_APP_GET_STATUS_CC:

            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_GetStatusCmd_t)))
            {
                UANT_APP_GetReleaseStatus((const UANT_APP_GetStatusCmd_t *)SBBufPtr);
            }
            break;

        case UANT_APP_GET_BACKUP_STATUS_CC:

            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_GetBackupStatusCmd_t)))
            {
                UANT_APP_GetBackupStatus((const UANT_APP_GetBackupStatusCmd_t *)SBBufPtr);
            }
            break;
        

        case UANT_APP_GET_SETTINGS_CC:

            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_GetSettingsCmd_t)))
            {
                UANT_APP_GetBackupSettings((const UANT_APP_GetSettingsCmd_t *)SBBufPtr);
            }
            break;

        /* ---------- 백업‑설정 WRITE ---------- */
        case UANT_APP_SET_SETTINGS_CC:

            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_SetSettingsCmd_t)))
            {
                UANT_APP_SetBackupSettings((const UANT_APP_SetSettingsCmd_t *)SBBufPtr);
            }
            break;

        case UANT_GET_STATUS_INTERNAL_CC:
            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_RequestRelease_t ))) {
                UANT_APP_IsReleased((const UANT_RequestRelease_t *)SBBufPtr);
            }
            break;

        case UANT_BURN_CHANNEL_INTERNAL_CC:
            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_BurnChannelCmd_t)))
            {
                UANT_APP_BurnChannelInternal((const UANT_APP_BurnChannelCmd_t *)SBBufPtr);
            }
            break;
            

        default:
            CFE_EVS_SendEvent(UANT_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
            "Invalid ground command code: CC = %d", cc);
            break;
    }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the UANT    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void UANT_APP_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case UANT_APP_CMD_MID:
            UANT_APP_ProcessGroundCommand(SBBufPtr);
            break;

        case UANT_APP_SEND_HK_MID:
            UANT_APP_SendHkCmd((const UANT_APP_SendHkCmd_t *)SBBufPtr);
            break;
        case UANT_APP_SEND_BCN_MID:
            UANT_APP_SendBcnCmd((const UANT_APP_SendBcnCmd_t *)SBBufPtr);
            break;
        default:
            CFE_EVS_SendEvent(UANT_APP_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "UANT: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
