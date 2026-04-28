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
 *   This file contains the source code for the PAYUEL_LGPM App.
 */

/*
** Include Files:
*/
#include "payuel_lgpm_app.h"
#include "payuel_lgpm_dispatch.h"
#include "payuel_lgpm_cmds.h"
#include "payuel_lgpm_eventids.h"
#include "payuel_lgpm_msgids.h"
#include "payuel_lgpm_msg.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool PAYUEL_LGPM_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
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

        CFE_EVS_SendEvent(PAYUEL_LGPM_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        PAYUEL_LGPM_Data.ErrCounter++;
    }

    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUEL_LGPM ground commands                                                     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void PAYUEL_LGPM_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /*
    ** Process PAYUEL_LGPM app ground commands
    */
    switch (CommandCode)
    {
        case PAYUEL_LGPM_NOOP_CC:
            if (PAYUEL_LGPM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_LGPM_NoopCmd_t)))
            {
                PAYUEL_LGPM_NoopCmd((const PAYUEL_LGPM_NoopCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_LGPM_RESET_COUNTERS_CC:
            if (PAYUEL_LGPM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_LGPM_ResetCountersCmd_t)))
            {
                PAYUEL_LGPM_ResetCountersCmd((const PAYUEL_LGPM_ResetCountersCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_LGPM_MCU_ALIVE_CHECK_CC: 
            if (PAYUEL_LGPM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_LGPM_MCU_ALIVE_CHECK_Cmd_t)))
            {
                PAYUEL_LGPM_MCU_ALIVE_CHECK_Cmd((const PAYUEL_LGPM_MCU_ALIVE_CHECK_Cmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_LGPM_3V3_PWR_ON_CC:
            if (PAYUEL_LGPM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_LGPM_3V3_PWR_ON_Cmd_t)))
            {
                PAYUEL_LGPM_3V3_PWR_ON_Cmd((const PAYUEL_LGPM_3V3_PWR_ON_Cmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_LGPM_3V3_PWR_OFF_CC:
            if (PAYUEL_LGPM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_LGPM_3V3_PWR_OFF_Cmd_t)))
            {
                PAYUEL_LGPM_3V3_PWR_OFF_Cmd((const PAYUEL_LGPM_3V3_PWR_OFF_Cmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_LGPM_MAIN_BOOST_SW_ON_CC:
            if (PAYUEL_LGPM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_LGPM_MAIN_BOOST_SW_ON_Cmd_t)))
            {
                PAYUEL_LGPM_MAIN_BOOST_SW_ON_Cmd((const PAYUEL_LGPM_MAIN_BOOST_SW_ON_Cmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_LGPM_MAIN_BOOST_SW_OFF_CC:
            if (PAYUEL_LGPM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_LGPM_MAIN_BOOST_SW_OFF_Cmd_t)))
            {
                PAYUEL_LGPM_MAIN_BOOST_SW_OFF_Cmd((const PAYUEL_LGPM_MAIN_BOOST_SW_OFF_Cmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_LGPM_SUB_BOOST_SW_ON_CC:
            if (PAYUEL_LGPM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_LGPM_SUB_BOOST_SW_ON_Cmd_t)))
            {
                PAYUEL_LGPM_SUB_BOOST_SW_ON_Cmd((const PAYUEL_LGPM_SUB_BOOST_SW_ON_Cmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_LGPM_SUB_BOOST_SW_OFF_CC:
            if (PAYUEL_LGPM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_LGPM_SUB_BOOST_SW_OFF_Cmd_t)))
            {
                PAYUEL_LGPM_SUB_BOOST_SW_OFF_Cmd((const PAYUEL_LGPM_SUB_BOOST_SW_OFF_Cmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_LGPM_V28_MAIN_ON_CC:
            if (PAYUEL_LGPM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_LGPM_V28_MAIN_ON_Cmd_t)))
            {
                PAYUEL_LGPM_V28_MAIN_ON_Cmd((const PAYUEL_LGPM_V28_MAIN_ON_Cmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_LGPM_V28_MAIN_OFF_CC:
            if (PAYUEL_LGPM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_LGPM_V28_MAIN_OFF_Cmd_t)))
            {
                PAYUEL_LGPM_V28_MAIN_OFF_Cmd((const PAYUEL_LGPM_V28_MAIN_OFF_Cmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_LGPM_V28_SUB_ON_CC:
            if (PAYUEL_LGPM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_LGPM_V28_SUB_ON_Cmd_t)))
            {
                PAYUEL_LGPM_V28_SUB_ON_Cmd((const PAYUEL_LGPM_V28_SUB_ON_Cmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_LGPM_V28_SUB_OFF_CC:
            if (PAYUEL_LGPM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_LGPM_V28_SUB_OFF_Cmd_t)))
            {
                PAYUEL_LGPM_V28_SUB_OFF_Cmd((const PAYUEL_LGPM_V28_SUB_OFF_Cmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_LGPM_V12_MAIN_ON_CC:
            if (PAYUEL_LGPM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_LGPM_V12_MAIN_ON_Cmd_t)))
            {
                PAYUEL_LGPM_V12_MAIN_ON_Cmd((const PAYUEL_LGPM_V12_MAIN_ON_Cmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_LGPM_V12_MAIN_OFF_CC:
            if (PAYUEL_LGPM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_LGPM_V12_MAIN_OFF_Cmd_t)))
            {
                PAYUEL_LGPM_V12_MAIN_OFF_Cmd((const PAYUEL_LGPM_V12_MAIN_OFF_Cmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_LGPM_PWR_SENSE_INFO_CC:
            if (PAYUEL_LGPM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_LGPM_PWR_SENSE_INFO_Cmd_t)))
            {
                PAYUEL_LGPM_PWR_SENSE_INFO_Cmd((const PAYUEL_LGPM_PWR_SENSE_INFO_Cmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_LGPM_PWR_SEQ_ON_CC:
            if (PAYUEL_LGPM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_LGPM_PWR_SEQ_ON_Cmd_t)))
            {
                PAYUEL_LGPM_PWR_SEQ_ON_Cmd((const PAYUEL_LGPM_PWR_SEQ_ON_Cmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_LGPM_PWR_SEQ_OFF_CC:
            if (PAYUEL_LGPM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_LGPM_PWR_SEQ_OFF_Cmd_t)))
            {
                PAYUEL_LGPM_PWR_SEQ_OFF_Cmd((const PAYUEL_LGPM_PWR_SEQ_OFF_Cmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_LGPM_RWA_CONTROL_CC:
            if (PAYUEL_LGPM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_LGPM_RWA_CONTROL_Cmd_t)))
            {
                PAYUEL_LGPM_RWA_CONTROL_Cmd((const PAYUEL_LGPM_RWA_CONTROL_Cmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_LGPM_RWA_PWR_ON_CC:
            if (PAYUEL_LGPM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_LGPM_RWA_PWR_ON_Cmd_t)))
            {
                PAYUEL_LGPM_RWA_PWR_ON_Cmd((const PAYUEL_LGPM_RWA_PWR_ON_Cmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_LGPM_RWA_PWR_OFF_CC:
            if (PAYUEL_LGPM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_LGPM_RWA_PWR_OFF_Cmd_t)))
            {
                PAYUEL_LGPM_RWA_PWR_OFF_Cmd((const PAYUEL_LGPM_RWA_PWR_OFF_Cmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_LGPM_RWA_SENSE_INFO_CC:
            if (PAYUEL_LGPM_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_LGPM_RWA_SENSE_INFO_Cmd_t)))
            {
                PAYUEL_LGPM_RWA_SENSE_INFO_Cmd((const PAYUEL_LGPM_RWA_SENSE_INFO_Cmd_t *)SBBufPtr);
            }
            break;

        /* default case already found during FC vs length test */
        default:
            CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid ground command code: CC = %d",
                              CommandCode);
            break;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*  This routine will process any packet that is received on the PAYUEL_LGPM  */
/*  command pipe.                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void PAYUEL_LGPM_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case PAYUEL_LGPM_CMD_MID:
            PAYUEL_LGPM_ProcessGroundCommand(SBBufPtr);
            break;

        case PAYUEL_LGPM_SEND_HK_MID:
            PAYUEL_LGPM_SendHkCmd((const PAYUEL_LGPM_SendHkCmd_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(PAYUEL_LGPM_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_LGPM: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
