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
#if EPS_MISSION_CFG_DEVICE_p80_ENABLED
#include "eps_cmds_p80.h"
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool EPS_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool              result       = true;
    CFE_SB_MsgId_t    MsgId        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode      = 0;

    struct {
        size_t Actual;
        size_t Expected;
    } Lengths;

    CFE_MSG_GetSize(MsgPtr, &Lengths.Actual);

    /*
    ** Verify the command packet length.
    */
    if (ExpectedLength != Lengths.Actual) {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(EPS_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)Lengths.Actual,
                          (unsigned int)ExpectedLength);

        result = false;

        EPS_AppData.Counters.ErrCounter++;
        Lengths.Expected = ExpectedLength;

        EPS_SendReport(MsgPtr, &Lengths, sizeof(Lengths), CFE_STATUS_WRONG_MSG_LENGTH, RPT_RETTYPE_APP);
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
        /*
        ** Basic Commands
        */
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

        case EPS_REPORT_APPDATA_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_ReportAppDataCmd_t)))
            {
                // EPS_ReportAppDataCmd((const EPS_ReportAppDataCmd_t *)SBBufPtr);
            }
            break;

        /*
        ** Power Interface Commands
        */
        case EPS_POWER_IF_GET_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_Power_If_Get_Cmd_t)))
            {
                EPS_Power_If_Get_Cmd((const EPS_Power_If_Get_Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_POWER_IF_SET_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_Power_If_Set_Cmd_t)))
            {
                EPS_Power_If_Set_Cmd((const EPS_Power_If_Set_Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_POWER_IF_LIST_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_Power_If_List_Cmd_t)))
            {
                EPS_Power_If_List_Cmd((const EPS_Power_If_List_Cmd_t *)SBBufPtr);
            }
            break;

        /*
        ** Housekeeping Command
        */
        case EPS_GET_HK_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_Get_Hk_Cmd_t)))
            {
                EPS_Get_Hk_Cmd((const EPS_Get_Hk_Cmd_t *)SBBufPtr);
            }
            break;

        /*
        ** Watchdog Command
        */
        case EPS_GND_WDT_CLEAR_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_Gnd_Watchdog_Clear_Cmd_t)))
            {
                EPS_Gnd_Watchdog_Clear_Cmd((const EPS_Gnd_Watchdog_Clear_Cmd_t *)SBBufPtr);
            }
            break;

        /*
        ** Remote Parameter Commands
        */
        case EPS_PARAM_GET_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_Param_Get_Cmd_t)))
            {
                EPS_Param_Get_Cmd((const EPS_Param_Get_Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_PARAM_SET_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_Param_Set_Cmd_t)))
            {
                EPS_Param_Set_Cmd((const EPS_Param_Set_Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_GET_FULL_TABLE_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_Get_Full_Table_Cmd_t)))
            {
                EPS_Get_Full_Table_Cmd((const EPS_Get_Full_Table_Cmd_t *)SBBufPtr);
            }
            break;

        /*
        ** Table Save/Load Commands
        */

        case EPS_TABLE_SAVE_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_Table_Save_Cmd_t)))
            {
                EPS_Table_Save_Cmd((const EPS_Table_Save_Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_TABLE_LOAD_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_Table_Load_Cmd_t)))
            {
                EPS_Table_Load_Cmd((const EPS_Table_Load_Cmd_t *)SBBufPtr);
            }
            break;

        /*
        ** Device-specific commands (handled separately)
        */
        default:
            CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "EPS: invalid command packet");
            break;
    }
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

        case EPS_SEND_BCN_MID:
            EPS_SendBcnCmd((const EPS_SendBcnCmd_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(EPS_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "EPS: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
