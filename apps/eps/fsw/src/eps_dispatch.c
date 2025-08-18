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
 *   This file contains the source code for the EPS App.
 */

/*
** Include Files:
*/
#include "eps_app.h"
#include "eps_dispatch.h"
#include "eps_eventids.h"
#include "eps_msgids.h"
#include "eps_msg.h"

#include "eps_cmds.h"
#include "eps_cmds_p31u.h"


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool EPS_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
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

        CFE_EVS_SendEvent(EPS_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        EPS_AppData.ErrCounter++;
    }

    return result;
}


void EPS_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /*
    ** Process EPS app ground commands
    */
    switch (CommandCode)
    {
        case EPS_NOOP_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_NoopCmd_t)))
            {
                EPS_NoopCmd((const EPS_NoopCmd_t *)SBBufPtr);
            }
            break;

        case EPS_RESET_COUNTERS_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_ResetCountersCmd_t)))
            {
                EPS_ResetCountersCmd((const EPS_ResetCountersCmd_t *)SBBufPtr);
            }
            break;

        /* default case already found during FC vs length test */
        default:
            CFE_EVS_SendEvent(EPS_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid ground command code: CC = %d",
                              CommandCode);
            break;
    }
}

void EPS_ProcessDeviceCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /*
    ** Process EPS app ground commands
    */
    switch (CommandCode)
    {
        case EPS_P31U_SET_OUT_SINGLE_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_SetOutputSingleCmd_t)))
            {
                EPS_P31U_SetOutputSingleCmd((const EPS_P31U_SetOutputSingleCmd_t *)SBBufPtr);
            }
            break;

        case EPS_P31U_SET_OUTPUTS_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_SetOutputsCmd_t)))
            {
                EPS_P31U_SetOutputsCmd((const EPS_P31U_SetOutputsCmd_t *)SBBufPtr);
            }
            break;

        case EPS_P31U_RESET_WDT_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_ResetWdtCmd_t)))
            {
                EPS_P31U_ResetWdtCmd((const EPS_P31U_ResetWdtCmd_t *)SBBufPtr);
            }
            break;

        case EPS_P31U_RESET_COUNTERS_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_ResetCountersCmd_t)))
            {
                EPS_P31U_ResetCountersCmd((const EPS_P31U_ResetCountersCmd_t *)SBBufPtr);
            }
            break;

        case EPS_P31U_HARD_RESET_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_HardResetCmd_t)))
            {
                EPS_P31U_HardResetCmd((const EPS_P31U_HardResetCmd_t *)SBBufPtr);
            }
            break;

        case EPS_P31U_GETHK_ALL_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_GetHkAllCmd_t)))
            {
                EPS_P31U_GetHkAllCmd((const EPS_P31U_GetHkAllCmd_t *)SBBufPtr);
            }
            break;

        case EPS_P31U_GETHK_OUT_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_GetHkOutCmd_t)))
            {
                EPS_P31U_GetHkOutCmd((const EPS_P31U_GetHkOutCmd_t *)SBBufPtr);
            }
            break;

        case EPS_P31U_GETHK_VI_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_GetHkViCmd_t)))
            {
                EPS_P31U_GetHkViCmd((const EPS_P31U_GetHkViCmd_t *)SBBufPtr);
            }
            break;

        case EPS_P31U_GETHK_WDT_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_GetHkWdtCmd_t)))
            {
                EPS_P31U_GetHkWdtCmd((const EPS_P31U_GetHkWdtCmd_t *)SBBufPtr);
            }
            break;

        case EPS_P31U_GETHK_BASIC_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_GetHkBasicCmd_t)))
            {
                EPS_P31U_GetHkBasicCmd((const EPS_P31U_GetHkBasicCmd_t *)SBBufPtr);
            }
            break;

        case EPS_P31U_GETHK_OLD_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_GetHkOldCmd_t)))
            {
                EPS_P31U_GetHkOldCmd((const EPS_P31U_GetHkOldCmd_t *)SBBufPtr);
            }
            break;

        case EPS_P31U_GETHK_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_GetHkCmd_t)))
            {
                EPS_P31U_GetHkCmd((const EPS_P31U_GetHkCmd_t *)SBBufPtr);
            }
            break;

        case EPS_P31U_SET_PV_VOLT_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_SetPvVoltCmd_t)))
            {
                EPS_P31U_SetPvVoltCmd((const EPS_P31U_SetPvVoltCmd_t *)SBBufPtr);
            }
            break;

        case EPS_P31U_SET_PV_AUTO_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_SetPvAutoCmd_t)))
            {
                EPS_P31U_SetPvAutoCmd((const EPS_P31U_SetPvAutoCmd_t *)SBBufPtr);
            }
            break;

        case EPS_P31U_SET_HEATER_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_SetHeaterCmd_t)))
            {
                EPS_P31U_SetHeaterCmd((const EPS_P31U_SetHeaterCmd_t *)SBBufPtr);
            }
            break;

        case EPS_P31U_GET_CONFIG_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_GetConfigCmd_t)))
            {
                EPS_P31U_GetConfigCmd((const EPS_P31U_GetConfigCmd_t *)SBBufPtr);
            }
            break;

        case EPS_P31U_SET_CONFIG_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_SetConfigCmd_t)))
            {
                EPS_P31U_SetConfigCmd((const EPS_P31U_SetConfigCmd_t *)SBBufPtr);
            }
            break;

        case EPS_P31U_CONFIG_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_ConfigCmd_t)))
            {
                EPS_P31U_ConfigCmd((const EPS_P31U_ConfigCmd_t *)SBBufPtr);
            }
            break;

        case EPS_P31U_GET_CONFIG2_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_GetConfig2Cmd_t)))
            {
                EPS_P31U_GetConfig2Cmd((const EPS_P31U_GetConfig2Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_P31U_SET_CONFIG2_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_SetConfig2Cmd_t)))
            {
                EPS_P31U_SetConfig2Cmd((const EPS_P31U_SetConfig2Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_P31U_CONFIG2_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_Config2Cmd_t)))
            {
                EPS_P31U_Config2Cmd((const EPS_P31U_Config2Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_P31U_SET_CONFIG3_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_SetConfig3Cmd_t)))
            {
                EPS_P31U_SetConfig3Cmd((const EPS_P31U_SetConfig3Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_P31U_TRANSACTION_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P31U_TransactionCmd_t)))
            {
                EPS_P31U_TransactionCmd((const EPS_P31U_TransactionCmd_t *)SBBufPtr);
            }
            break;
        
        default:
            CFE_EVS_SendEvent(EPS_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid device command code: CC = %d",
                              CommandCode);
            break;
    }

    // todo: get the retcode and increment counters. But in the cmd functions (hk handling must be atomic. see the noop cmd.)
}

void EPS_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case EPS_CMD_MID:
            EPS_ProcessGroundCommand(SBBufPtr);
            break;

        case EPS_SEND_HK_MID:
            EPS_SendHkCmd((const EPS_SendHkCmd_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(EPS_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "EPS: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
