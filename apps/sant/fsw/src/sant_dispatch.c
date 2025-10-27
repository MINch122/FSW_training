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


#include "sant_app.h"
#include "sant_dispatch.h"
#include "sant_cmds.h"
#include "sant_eventids.h"
#include "sant_msgids.h"
#include "sant_msg.h"
#include "cfe_msg.h"
#include <gs/gssb/gssb_ar6.h>

#include <gs/gssb/internal/gssb_common.h>   /* gs_gssb_common_* 함수 선언 */
#include <gs/gssb/internal/gssb_cmd_id.h>   /* GSSB_CMD_ 열거형 */
#include <gs/gssb/gssb_autodeploy.h>



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool SANT_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
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

        CFE_EVS_SendEvent(SANT_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        SANT_Data.ErrCounter++;


        /* RPT */
        SANT_ReportTlm_t *BufPtr = (SANT_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(SANT_ReportTlm_t));
        if (BufPtr == NULL) goto cleanup;

        if (CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader), CFE_SB_ValueToMsgId(SANT_REPORT_TLM_MID),
        sizeof(SANT_ReportTlm_t)) != CFE_SUCCESS) {
            CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
            goto cleanup;
        }
        BufPtr->Report.MsgID = (uint16_t)CFE_SB_MsgIdToValue(MsgId);
        BufPtr->Report.CommandCode = (uint8_t)FcnCode;
        BufPtr->Report.ReturnType = RPT_RETTYPE_APP;
        BufPtr->Report.ReturnCode = CFE_STATUS_WRONG_MSG_LENGTH; // Error code of `Length error`
        BufPtr->Report.ReturnDataSize = 2 * sizeof(uint32_t);
        
        uint32_t Temp32 = (uint32_t)ActualLength;
        memcpy(BufPtr->Report.ReturnValue, &Temp32, sizeof(uint32_t));
        Temp32 = (uint32_t)ExpectedLength;
        memcpy(BufPtr->Report.ReturnValue + sizeof(uint32_t), &Temp32, sizeof(uint32_t));

        CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
        if (CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true) != CFE_SUCCESS) {
            CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
            goto cleanup;
        }
        /* End of RPT */
    }
cleanup:
    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */

/* SANT ground commands or scheduled commands(from sch app/ ref. sch's table)                                                    */

/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void SANT_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{   //CFE_Status_t     status; // 내부 판단용
    CFE_MSG_FcnCode_t CC = 0;
    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CC);

    switch (CC)
    {
        /* ───────── NOOP ───────── */
        case SANT_NOOP_CC:
            if (SANT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SANT_NoopCmd_t))) //지상에서 보내준 것과 정의한 구조가 일치하는지 검증
            {
                SANT_NoopCmd((const SANT_NoopCmd_t *)SBBufPtr);
            }
            break;

        /* ───────── 카운터 리셋 ───────── */

        case SANT_RESET_COUNTERS_CC:

            if (SANT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SANT_ResetCountersCmd_t)))
            {
                SANT_ResetCountersCmd((const SANT_ResetCountersCmd_t *)SBBufPtr);
            }
            
            break;



         /* ------------ Board Soft‑Reboot ------------ */
        case SANT_SOFT_REBOOT_CC:

            if (SANT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SANT_SoftRebootCmd_t)))
            {
                SANT_SoftRebootCmd((const SANT_SoftRebootCmd_t *)SBBufPtr);
            }
            break;


        /* ------------ Burn / Stop ------------ */

        case SANT_BURN_CC:

            if (SANT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SANT_BurnCmd_t)))
            {
                SANT_BurnCmd((const SANT_BurnCmd_t *)SBBufPtr);
            }
            break;

        case SANT_STOP_BURN_CC:

            if (SANT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SANT_StopBurnCmd_t)))
            {
                SANT_StopBurnCmd((const SANT_StopBurnCmd_t *)SBBufPtr);
            }
            break;

        /* ---------- BOARD‑STATUS (uptime, reboot‑cnt) ---------- */

        case SANT_GET_BOARD_STATUS_CC:

            if (SANT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SANT_GetBoardStatusCmd_t)))
            {
                SANT_GetBoardStatusCmd((const SANT_GetBoardStatusCmd_t *)SBBufPtr);
            }
            break;

        /* ---------- MCU 내부 온도 ---------- */
        case SANT_GET_TEMPERATURE_CC:
            
            if (SANT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SANT_GetTemperatureCmd_t)))
            {
                SANT_GetTemperatureCmd((const SANT_GetTemperatureCmd_t *)SBBufPtr);
            }
            break;


       /* ------------ Telemetry pulls ------------ */

        case SANT_GET_STATUS_CC:

            if (SANT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SANT_GetStatusCmd_t)))
            {
                SANT_GetStatusCmd((const SANT_GetStatusCmd_t *)SBBufPtr);
            }
            break;

        case SANT_GET_BACKUP_STATUS_CC:

            if (SANT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SANT_GetBackupStatusCmd_t)))
            {
                SANT_GetBackupStatusCmd((const SANT_GetBackupStatusCmd_t *)SBBufPtr);
            }
            break;
        

        case SANT_GET_SETTINGS_CC:

            if (SANT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SANT_GetSettingsCmd_t)))
            {
                SANT_GetBackupSettingsCmd((const SANT_GetSettingsCmd_t *)SBBufPtr);
            }
            break;

        /* ---------- 백업‑설정 WRITE ---------- */

        case SANT_SET_SETTINGS_CC:

            if (SANT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SANT_SetSettingsCmd_t)))
            {
                SANT_SetBackupSettingsCmd((const SANT_SetSettingsCmd_t *)SBBufPtr);
            }
            break;

        case SANT_BURN_INTERNAL_CC:

            if (SANT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SANT_BurnCmd_t)))
            {
                SANT_BurnInternalCmd((const SANT_BurnCmd_t *)SBBufPtr);
            }
            break;

        default:
            CFE_EVS_SendEvent(SANT_CC_ERR_EID, CFE_EVS_EventType_ERROR,

                              "Invalid ground command code: CC = %d", CC);


            /* RPT */
            SANT_ReportTlm_t *BufPtr = (SANT_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(SANT_ReportTlm_t));
            if (BufPtr == NULL) break;
            if(CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader), CFE_SB_ValueToMsgId(SANT_REPORT_TLM_MID), sizeof(SANT_ReportTlm_t) != CFE_SUCCESS)) {
                CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
                break;
            }
            BufPtr->Report.MsgID = SANT_CMD_MID;
            BufPtr->Report.CommandCode = (uint8_t)CC;
            BufPtr->Report.ReturnType = RPT_RETTYPE_APP;
            BufPtr->Report.ReturnCode = CFE_STATUS_BAD_COMMAND_CODE;
            BufPtr->Report.ReturnDataSize = 0;
            CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
            if(CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true) != CFE_SUCCESS) {
                CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
                break;
            }
            /* End of RPT */
            break;
    }

   
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the SANT    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void SANT_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case SANT_CMD_MID:
            SANT_ProcessGroundCommand(SBBufPtr);
            break;

        case SANT_SEND_HK_MID:
            SANT_SendHkCmd((const SANT_SendHkCmd_t *)SBBufPtr);
            break;

        case SANT_SEND_BCN_MID:
            SANT_SendBcnCmd((const SANT_SendBcnCmd_t *)SBBufPtr);
            break;
        
        case SANT_SEND_OP_MID:
            SANT_SendOpCmd((const SANT_SendOpCmd_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(SANT_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SANT: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));


            /* RPT */
            SANT_ReportTlm_t *BufPtr = (SANT_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(SANT_ReportTlm_t));
            if (BufPtr == NULL) break;
            if(CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader), CFE_SB_ValueToMsgId(SANT_REPORT_TLM_MID), sizeof(SANT_ReportTlm_t) != CFE_SUCCESS)) {
                CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
                break;
            }
            BufPtr->Report.MsgID = SANT_CMD_MID;
            BufPtr->Report.CommandCode = 0;
            BufPtr->Report.ReturnType = RPT_RETTYPE_APP;
            BufPtr->Report.ReturnCode = CFE_STATUS_UNKNOWN_MSG_ID;
            BufPtr->Report.ReturnDataSize = sizeof(CFE_SB_MsgId_Atom_t);
            memcpy(BufPtr->Report.ReturnValue, &MsgId.Value, sizeof(CFE_SB_MsgId_Atom_t));
            CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
            if(CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true) != CFE_SUCCESS) {
                CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
                break;
            }
            /* End of RPT */
            break;
    }
}
