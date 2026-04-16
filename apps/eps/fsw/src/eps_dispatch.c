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
#include "eps_utils.h"
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



        /*
        ** Power Interface Commands
        */
        case EPS_P80_POWER_IF_GET_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P80_Power_If_Get_Cmd_t)))
            {
                EPS_P80_Power_If_Get_Cmd((const EPS_P80_Power_If_Get_Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_P80_POWER_IF_SET_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P80_Power_If_Set_Cmd_t)))
            {
                EPS_P80_Power_If_Set_Cmd((const EPS_P80_Power_If_Set_Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_P80_POWER_IF_LIST_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P80_Power_If_List_Cmd_t)))
            {
                EPS_P80_Power_If_List_Cmd((const EPS_P80_Power_If_List_Cmd_t *)SBBufPtr);
            }
            break;

        /*
        ** Housekeeping Command
        */
        case EPS_GET_HK_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_Get_HK_Cmd_t)))
            {
                EPS_Get_HK_Cmd((const EPS_Get_HK_Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_GET_HK_ALL_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_Get_HK_All_Cmd_t)))
            {
                EPS_Get_HK_All_Cmd((const EPS_Get_HK_All_Cmd_t *)SBBufPtr);
            }
            break;

        /*
        ** Watchdog Command
        */
        case EPS_P80_GND_WDT_CLEAR_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P80_Gnd_Watchdog_Clear_Cmd_t)))
            {
                EPS_P80_Gnd_Watchdog_Clear_Cmd((const EPS_P80_Gnd_Watchdog_Clear_Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_P80_GND_WDT_CLEAR_ALL_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_P80_Gnd_Watchdog_Clear_All_Cmd_t)))
            {
                EPS_P80_Gnd_Watchdog_Clear_All_Cmd((const EPS_P80_Gnd_Watchdog_Clear_All_Cmd_t *)SBBufPtr);
            }
            break;

        /*
        ** Remote Parameter Commands
        */
        case EPS_RPARAM_GET_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_RParam_Get_Cmd_t)))
            {
                EPS_RParam_Get_Cmd((const EPS_RParam_Get_Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_RPARAM_SET_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_RParam_Set_Cmd_t)))
            {
                EPS_RParam_Set_Cmd((const EPS_RParam_Set_Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_RPARAM_GET_FULL_TABLE_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_RParam_Get_Full_Table_Cmd_t)))
            {
                EPS_RParam_Get_Full_Table_Cmd((const EPS_RParam_Get_Full_Table_Cmd_t *)SBBufPtr);
            }
            break;

        /*
        ** Table Save/Load Commands
        */

        case EPS_RPARAM_TABLE_SAVE_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_RParam_Table_Save_Cmd_t)))
            {
                EPS_RParam_Table_Save_Cmd((const EPS_RParam_Table_Save_Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_RPARAM_TABLE_LOAD_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_RParam_Table_Load_Cmd_t)))
            {
                EPS_RParam_Table_Load_Cmd((const EPS_RParam_Table_Load_Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_RPARAM_SAVE_TO_STORE_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_RParam_Save_To_Store_Cmd_t)))
            {
                EPS_RParam_Save_To_Store_Cmd((const EPS_RParam_Save_To_Store_Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_RPARAM_LOAD_FROM_STORE_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_RParam_Load_From_Store_Cmd_t)))
            {
                EPS_RParam_Load_From_Store_Cmd((const EPS_RParam_Load_From_Store_Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_RPARAM_SAVE_ALL_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_RParam_Save_All_Cmd_t)))
            {
                EPS_RParam_Save_All_Cmd((const EPS_RParam_Save_All_Cmd_t *)SBBufPtr);
            }
            break;

        /*
        ** Beacon Report Command
        */
        case EPS_REPORT_BCN_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_ReportBcnCmd_t)))
            {
                EPS_ReportBcnCmd((const EPS_ReportBcnCmd_t *)SBBufPtr);
            }
            break;

        /*
        ** CSP Standard Service Commands
        */
        case EPS_CSP_PING_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_CSP_Ping_Cmd_t)))
            {
                EPS_CSP_Ping_Cmd((const EPS_CSP_Ping_Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_CSP_REBOOT_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_CSP_Reboot_Cmd_t)))
            {
                EPS_CSP_Reboot_Cmd((const EPS_CSP_Reboot_Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_CSP_PS_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_CSP_PS_Cmd_t)))
            {
                EPS_CSP_PS_Cmd((const EPS_CSP_PS_Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_CSP_MEMFREE_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_CSP_MemFree_Cmd_t)))
            {
                EPS_CSP_MemFree_Cmd((const EPS_CSP_MemFree_Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_CSP_BUF_FREE_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_CSP_BufFree_Cmd_t)))
            {
                EPS_CSP_BufFree_Cmd((const EPS_CSP_BufFree_Cmd_t *)SBBufPtr);
            }
            break;

        case EPS_CSP_UPTIME_CC:
            if (EPS_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EPS_CSP_Uptime_Cmd_t)))
            {
                EPS_CSP_Uptime_Cmd((const EPS_CSP_Uptime_Cmd_t *)SBBufPtr);
            }
            break;

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

        /*
        ** Disabled: EPS_SendHkCmd is not implemented.
        **
        ** case EPS_SEND_HK_MID:
        **     EPS_SendHkCmd((const EPS_SendHkCmd_t *)SBBufPtr);
        **     break;
        */

        case EPS_SEND_BCN_MID:
            EPS_SendBcnCmd((const EPS_SendBcnCmd_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(EPS_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "EPS: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
