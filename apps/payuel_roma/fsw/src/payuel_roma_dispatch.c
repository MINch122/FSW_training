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
 *   This file contains the source code for the UELYSYS Payload Roma-SP
 */

/*
** Include Files:
*/
#include "payuel_roma.h"
#include "payuel_roma_dispatch.h"
#include "payuel_roma_cmds.h"
#include "payuel_roma_eventids.h"
#include "payuel_roma_msgids.h"
#include "payuel_roma_msg.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool PAYUEL_ROMA_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
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

        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        PAYUEL_ROMA_Data.ErrCounter++;

        /* RPT */
        PAYUEL_ROMA_ReportTlm_t *BufPtr = (PAYUEL_ROMA_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(PAYUEL_ROMA_ReportTlm_t));
        if (BufPtr == NULL) goto cleanup;

        if (CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader), CFE_SB_ValueToMsgId(PAYUEL_ROMA_REPORT_TLM_MID),
        sizeof(PAYUEL_ROMA_ReportTlm_t)) != CFE_SUCCESS) {
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
/* UELYSYS Payload Roma-SP ground commands                                    */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void PAYUEL_ROMA_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /*
    ** Process UELYSYS Payload Roma-SP ground commands
    */
    switch (CommandCode)
    {
        case PAYUEL_ROMA_NOOP_CC:

            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_NoopCmd_t)))
            {
                PAYUEL_ROMA_NoopCmd((const PAYUEL_ROMA_NoopCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_RESET_COUNTERS_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_ResetCountersCmd_t)))
            {
                PAYUEL_ROMA_ResetCountersCmd((const PAYUEL_ROMA_ResetCountersCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_COMM_TEST_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_CommTestCmd_t)))
            {
                PAYUEL_ROMA_CommTestCmd((const PAYUEL_ROMA_CommTestCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_CLOCK_SYNC_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_ClockSyncCmd_t)))
            {
                PAYUEL_ROMA_ClockSyncCmd((const PAYUEL_ROMA_ClockSyncCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_LOG_TEST_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_LogTestCmd_t)))
            {
                PAYUEL_ROMA_LogTestCmd((const PAYUEL_ROMA_LogTestCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_TRANS_TEST_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_TransTestCmd_t)))
            {
                PAYUEL_ROMA_TransTestCmd((const PAYUEL_ROMA_TransTestCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_GET_SPECIFIC_LINE_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_GetSpecificLineCmd_t)))
            {
                PAYUEL_ROMA_GetSpecificLineCmd((const PAYUEL_ROMA_GetSpecificLineCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_GET_MULTIPLE_LINES_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_GetMultipleLinesCmd_t)))
            {
                PAYUEL_ROMA_GetMultipleLinesCmd((const PAYUEL_ROMA_GetMultipleLinesCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_GET_LATEST_LINE_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_GetLatestLineCmd_t)))
            {
                PAYUEL_ROMA_GetLatestLineCmd((const PAYUEL_ROMA_GetLatestLineCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_GET_LATEST_N_LINES_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_GetLatestNLinesCmd_t)))
            {
                PAYUEL_ROMA_GetLatestNLinesCmd((const PAYUEL_ROMA_GetLatestNLinesCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_CLEAR_ALL_LINES_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_ClearAllLinesCmd_t)))
            {
                PAYUEL_ROMA_ClearAllLinesCmd((const PAYUEL_ROMA_ClearAllLinesCmd_t *)SBBufPtr);
            }
            break;
        
        case PAYUEL_ROMA_GET_SINGLE_ENTRY_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_GetSingleEntryCmd_t)))
            {
                PAYUEL_ROMA_GetSingleEntryCmd((const PAYUEL_ROMA_GetSingleEntryCmd_t *)SBBufPtr);
            }
            break;
        
        case PAYUEL_ROMA_GET_MULTIPLE_ENTRIES_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_GetMultipleEntriesCmd_t)))
            {
                PAYUEL_ROMA_GetMultipleEntriesCmd((const PAYUEL_ROMA_GetMultipleEntriesCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_ADD_ENTRY_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_AddEntryCmd_t)))
            {
                PAYUEL_ROMA_AddEntryCmd((const PAYUEL_ROMA_AddEntryCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_REMOVE_ENTRY_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_RemoveEntryCmd_t)))
            {
                PAYUEL_ROMA_RemoveEntryCmd((const PAYUEL_ROMA_RemoveEntryCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_GET_USED_SLOTS_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_GetUsedSlotsCmd_t)))
            {
                PAYUEL_ROMA_GetUsedSlotsCmd((const PAYUEL_ROMA_GetUsedSlotsCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_SET_ROUTE_DEFAULT_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_SetRouteDefaultCmd_t)))
            {
                PAYUEL_ROMA_SetRouteDefaultCmd((const PAYUEL_ROMA_SetRouteDefaultCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_RESET_ROUTE_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_ResetRouteCmd_t)))
            {
                PAYUEL_ROMA_ResetRouteCmd((const PAYUEL_ROMA_ResetRouteCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_LOAD_ROUTE_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_LoadRouteCmd_t)))
            {
                PAYUEL_ROMA_LoadRouteCmd((const PAYUEL_ROMA_LoadRouteCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_SAVE_ROUTE_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_SaveRouteCmd_t)))
            {
                PAYUEL_ROMA_SaveRouteCmd((const PAYUEL_ROMA_SaveRouteCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_SEND_ROUTE_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_SendRouteCmd_t)))
            {
                PAYUEL_ROMA_SendRouteCmd((const PAYUEL_ROMA_SendRouteCmd_t *)SBBufPtr);
            }
            break;
        
        case PAYUEL_ROMA_SET_ROUTE_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_SetRouteCmd_t)))
            {
                PAYUEL_ROMA_SetRouteCmd((const PAYUEL_ROMA_SetRouteCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_PAR_GET_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_ParGetCmd_t)))
            {
                PAYUEL_ROMA_ParGetCmd((const PAYUEL_ROMA_ParGetCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_PAR_SET_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_ParSetCmd_t)))
            {
                PAYUEL_ROMA_ParSetCmd((const PAYUEL_ROMA_ParSetCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_PAR_DEFAULTS_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_ParDefaultsCmd_t)))
            {
                PAYUEL_ROMA_ParDefaultsCmd((const PAYUEL_ROMA_ParDefaultsCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_PAR_SAVE_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_ParSaveCmd_t)))
            {
                PAYUEL_ROMA_ParSaveCmd((const PAYUEL_ROMA_ParSaveCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_PAR_RESTORE_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_ParRestoreCmd_t)))
            {
                PAYUEL_ROMA_ParRestoreCmd((const PAYUEL_ROMA_ParRestoreCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_PAR_LOAD_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_ParLoadCmd_t)))
            {
                PAYUEL_ROMA_ParLoadCmd((const PAYUEL_ROMA_ParLoadCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_PAR_SET_OOB_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_ParSetOobCmd_t)))
            {
                PAYUEL_ROMA_ParSetOobCmd((const PAYUEL_ROMA_ParSetOobCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_SEND_COMMAND_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_SendCommandCmd_t)))
            {
                PAYUEL_ROMA_SendCommandCmd((const PAYUEL_ROMA_SendCommandCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_SEND_MSG_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_SendMsgCmd_t)))
            {
                PAYUEL_ROMA_SendMsgCmd((const PAYUEL_ROMA_SendMsgCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_SYNC_RX_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_SyncRxCmd_t)))
            {
                PAYUEL_ROMA_SyncRxCmd((const PAYUEL_ROMA_SyncRxCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_SYNC_TX_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_SyncTxCmd_t)))
            {
                PAYUEL_ROMA_SyncTxCmd((const PAYUEL_ROMA_SyncTxCmd_t *)SBBufPtr);
            }
            break;

        case PAYUEL_ROMA_PAY_INIT_CC:
            if (PAYUEL_ROMA_VerifyCmdLength(&SBBufPtr->Msg, sizeof(PAYUEL_ROMA_PayInitCmd_t)))
            {
                PAYUEL_ROMA_PayInitCmd((const PAYUEL_ROMA_PayInitCmd_t *)SBBufPtr);
            }
            break;


        /* default case already found during FC vs length test */
        default:
            CFE_EVS_SendEvent(PAYUEL_ROMA_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid ground command code: CC = %d",
                              CommandCode);

            /* RPT */
            PAYUEL_ROMA_ReportTlm_t *BufPtr = (PAYUEL_ROMA_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(PAYUEL_ROMA_ReportTlm_t));
            if (BufPtr == NULL) break;
            if(CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader), CFE_SB_ValueToMsgId(PAYUEL_ROMA_REPORT_TLM_MID), sizeof(PAYUEL_ROMA_ReportTlm_t) != CFE_SUCCESS)) {
                CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
                break;
            }
            BufPtr->Report.MsgID = PAYUEL_ROMA_CMD_MID;
            BufPtr->Report.CommandCode = (uint8_t)CommandCode;
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
/*     This routine will process any packet that is received on the UELYSYS   */
/*     Payload Roma-SP command pipe.                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void PAYUEL_ROMA_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case PAYUEL_ROMA_CMD_MID:
            PAYUEL_ROMA_ProcessGroundCommand(SBBufPtr);
            break;

        case PAYUEL_ROMA_SEND_HK_MID:
            PAYUEL_ROMA_SendHkCmd((const PAYUEL_ROMA_SendHkCmd_t *)SBBufPtr);
            break;

        case PAYUEL_ROMA_SEND_BCN_MID:
            PAYUEL_ROMA_SendBcnCmd((const PAYUEL_ROMA_SendBcnCmd_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(PAYUEL_ROMA_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Roma-SP: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
