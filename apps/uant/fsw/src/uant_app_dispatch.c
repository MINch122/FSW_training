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

//메뉴얼 정독하고 걸러야할 야규먼트 체크, 안테나에서 resp가 뭐뭐있는지 get제외하고 공통적으로 오는 것이 있는지 확인
#include "uant.h" 
#include "uant_app.h"
#include "uant_app_dispatch.h"
#include "uant_app_cmds.h"
#include "uant_app_eventids.h"
#include "uant_app_msgids.h"
#include "uant_app_msg.h"
#include "cfe_msg.h"


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
/* UANT ground commands                                                     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void UANT_APP_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{   
    CFE_MSG_FcnCode_t CommandCode = 0;
    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);
    //printf("good");
    switch (CommandCode)
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

        /* ───────── 장치 리셋 ───────── */
        case UANT_APP_RESET_CC:
            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_ISIS_ResetCmd_t)))
            {
                UANT_APP_Reset((const UANT_APP_ISIS_ResetCmd_t *)SBBufPtr);
            }
            break;

        /* ───────── ARM / DISARM ───────── */
        case UANT_APP_ARM_ANTENNA_SYSTEMS_CC:
            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_ISIS_ArmAntennaSystemsCmd_t)))
            {
                UANT_APP_Arm((const UANT_APP_ISIS_ArmAntennaSystemsCmd_t *)SBBufPtr);
            }
            break;

        case UANT_APP_DISARM_CC:
            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_ISIS_DisarmCmd_t)))
            {
                UANT_APP_Disarm((const UANT_APP_ISIS_DisarmCmd_t *)SBBufPtr);
            }
            break;

        /* ───────── 자동 전개 ───────── */
        case UANT_APP_AUTOMATED_DEPLOYMENT_CC:
            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_ISIS_AutomatedDeploymentCmd_t)))
            {
                UANT_APP_AutomatedDeployment((const UANT_APP_ISIS_AutomatedDeploymentCmd_t *)SBBufPtr);
            }
            break;

        /* ───────── 단일 안테나 전개 ───────── */
        case UANT_APP_DEPLOY_ANT1_CC:
            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_ISIS_DeployAnt1Cmd_t)))
            {
                UANT_APP_DeployAnt1((const UANT_APP_ISIS_DeployAnt1Cmd_t *)SBBufPtr);
            }
            break;
        case UANT_APP_DEPLOY_ANT2_CC:
            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_ISIS_DeployAnt2Cmd_t)))
            {
                UANT_APP_DeployAnt2((const UANT_APP_ISIS_DeployAnt2Cmd_t *)SBBufPtr);
            }
            break;
        case UANT_APP_DEPLOY_ANT3_CC:
            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_ISIS_DeployAnt3Cmd_t)))
            {
                UANT_APP_DeployAnt3((const UANT_APP_ISIS_DeployAnt3Cmd_t *)SBBufPtr);
            }
            break;
        case UANT_APP_DEPLOY_ANT4_CC:
            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_ISIS_DeployAnt4Cmd_t)))
            {
                UANT_APP_DeployAnt4((const UANT_APP_ISIS_DeployAnt4Cmd_t *)SBBufPtr);
            }
            break;
            

        /* ───────── Override 전개 ───────── */
        case UANT_APP_DEPLOY_ANT1_OVERRIDE_CC:
            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_ISIS_DeployAnt1OverrideCmd_t)))
            {
                UANT_APP_DeployAnt1_Override((const UANT_APP_ISIS_DeployAnt1OverrideCmd_t *)SBBufPtr);
            }
            break;
        case UANT_APP_DEPLOY_ANT2_OVERRIDE_CC:
            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_ISIS_DeployAnt2OverrideCmd_t)))
            {
                UANT_APP_DeployAnt2_Override((const UANT_APP_ISIS_DeployAnt2OverrideCmd_t *)SBBufPtr);
            }
            break;
        case UANT_APP_DEPLOY_ANT3_OVERRIDE_CC:
            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_ISIS_DeployAnt3OverrideCmd_t)))
            {
                UANT_APP_DeployAnt3_Override((const UANT_APP_ISIS_DeployAnt3OverrideCmd_t *)SBBufPtr);
            }
            break;
        case UANT_APP_DEPLOY_ANT4_OVERRIDE_CC:
            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_ISIS_DeployAnt4OverrideCmd_t)))
            {
                UANT_APP_DeployAnt4_Override((const UANT_APP_ISIS_DeployAnt4OverrideCmd_t *)SBBufPtr);
            }
            break;

        /* ───────── 전개 취소 ───────── */
        case UANT_APP_CANCEL_DEPLOYMENT_ACTIVATION_CC:
            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_ISIS_CancelDeploymentActivationCmd_t)))
            {
                UANT_APP_CancleDeployment((const UANT_APP_ISIS_CancelDeploymentActivationCmd_t *)SBBufPtr);
            }
            break;

        /* ───────── 상태 조회 ───────── */
        case UANT_APP_GET_DEPLOYMENT_STATUS_CC:
            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_ISIS_ReportDeploymentStatusCmd_t)))
            {
                UANT_APP_GetDeploymentStatus((const UANT_APP_ISIS_ReportDeploymentStatusCmd_t *)SBBufPtr);
            }
            break;

        /* ───────── 온도 조회 ───────── */
        case UANT_APP_MEASURE_SYSTEM_TEMPERATURE_CC:
            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_ISIS_MeasureSystemTemperatureCmd_t)))
            {
                UANT_APP_MeasureAntSystemTemperature((const UANT_APP_ISIS_MeasureSystemTemperatureCmd_t *)SBBufPtr);
            }
            break;

        /* ───────── 활성화 횟수 보고 ───────── */
        case UANT_APP_REPORT_ANT_ACTIVATION_CNT_CC:
            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_ISIS_ReportAntActivationCntCmd_t)))
            {
                UANT_APP_ReportAntActivationCnt((const UANT_APP_ISIS_ReportAntActivationCntCmd_t *)SBBufPtr);
            }
            break;

        /* ───────── 활성화 시간 보고 ───────── */
        case UANT_APP_REPORT_ANT_ACTIVATION_TIME_CC:
            if (UANT_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UANT_APP_ISIS_ReportAntActivationTimeCmd_t)))
            {
                UANT_APP_ReportAntActivationTime((const UANT_APP_ISIS_ReportAntActivationTimeCmd_t *)SBBufPtr);
            }
            break;

        default:
            CFE_EVS_SendEvent(UANT_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid ground command code: CC = %d", CommandCode);
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
    //printf("good");
    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case UANT_APP_CMD_MID:
            UANT_APP_ProcessGroundCommand(SBBufPtr);
            break;
        //sch 추가?
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
