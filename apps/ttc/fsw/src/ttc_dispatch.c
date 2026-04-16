/************************************************************************
 * NASA Docket No. GSC-19,200-1, and identified as "cFS Draco"
 *
 * Copyright (c) 2023 United States Government as represented by the
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
 *   This file contains the source code for the Ttc.
 */

/*
** Include Files:
*/
#include "ttc.h"
#include "ttc_dispatch.h"
#include "ttc_cmds.h"
#include "ttc_eventids.h"
#include "ttc_msgids.h"
#include "ttc_msg.h"
#include "ttc_timeline.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool TTC_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
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

        CFE_EVS_SendEvent(TTC_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        TTC_AppData.ErrCounter++;
    }

    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* TTC ground commands                                                     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void TTC_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /*
    ** Process TTC app ground commands
    */
    switch (CommandCode)
    {
        case TTC_NOOP_CC:
            if (TTC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(TTC_NoopCmd_t)))
            {
                TTC_NoopCmd((const TTC_NoopCmd_t *)SBBufPtr);
            }
            break;

        case TTC_RESET_COUNTERS_CC:
            if (TTC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(TTC_ResetCountersCmd_t)))
            {
                TTC_ResetCountersCmd((const TTC_ResetCountersCmd_t *)SBBufPtr);
            }
            break;

        case TTC_REPORT_CC:
            // TODO: implement a report interface.
            OS_printf("TTC: Report command received (not implemented)\n");
            break;

        case TTC_GET_TIMELINE_HK_CC:
            if (TTC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(TTC_GetTimelineHkCmd_t)))
            {
                TTC_GetTimelineHkCmd((const TTC_GetTimelineHkCmd_t *)SBBufPtr);
            }
            break;
        
        case TTC_RESET_TIMELINE_HK_CC:
            if (TTC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(TTC_ResetTimelineHkCmd_t)))
            {
                TTC_ResetTimelineHkCmd((const TTC_ResetTimelineHkCmd_t *)SBBufPtr);
            }
            break;

        case TTC_GET_PENDING_ENTRY_COUNT_CC:
            if (TTC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(TTC_GetPendingEntryCountCmd_t)))
            {
                TTC_GetPendingEntryCountCmd((const TTC_GetPendingEntryCountCmd_t *)SBBufPtr);
            }
            break;

        case TTC_GET_NEXT_ENTRY_ID_CC:
            if (TTC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(TTC_GetNextEntryIdCmd_t)))
            {
                TTC_GetNextEntryIdCmd((const TTC_GetNextEntryIdCmd_t *)SBBufPtr);
            }
            break;

        case TTC_GET_NEXT_EXEC_TIME_CC:
            if (TTC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(TTC_GetNextExecutionTimeCmd_t)))
            {
                TTC_GetNextExecutionTimeCmd((const TTC_GetNextExecutionTimeCmd_t *)SBBufPtr);
            }
            break;

        case TTC_INSERT_ABS_CMD_ENTRY_CC:
            if (TTC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(TTC_InsertAbsCmdEntryCmd_t)))
            {
                TTC_InsertAbsCmdEntryCmd((const TTC_InsertAbsCmdEntryCmd_t *)SBBufPtr);
                TTC_Debug_PrintTimeline();
            }
            break;

        case TTC_INSERT_REL_CMD_ENTRY_CC:
            if (TTC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(TTC_InsertRelCmdEntryCmd_t)))
            {
                TTC_InsertRelCmdEntryCmd((const TTC_InsertRelCmdEntryCmd_t *)SBBufPtr);
                TTC_Debug_PrintTimeline();
            }
            break;

        case TTC_DELETE_CMD_ENTRY_CC:
            if (TTC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(TTC_DeleteEntryCmd_t)))
            {
                TTC_DeleteEntryCmd((const TTC_DeleteEntryCmd_t *)SBBufPtr);
                TTC_Debug_PrintTimeline();
            }
            break;

        case TTC_DELETE_CMD_GROUP_CC:
            if (TTC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(TTC_DeleteGroupCmd_t)))
            {
                TTC_DeleteGroupCmd((const TTC_DeleteGroupCmd_t *)SBBufPtr);
                TTC_Debug_PrintTimeline();
            }
            break;

        case TTC_DELETE_ALL_ENTRIES_CC:
            if (TTC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(TTC_DeleteAllEntriesCmd_t)))
            {
                TTC_DeleteAllEntriesCmd((const TTC_DeleteAllEntriesCmd_t *)SBBufPtr);
                TTC_Debug_PrintTimeline();
            }
            break;

        case TTC_EXECUTE_CMD_ENTRY_CC:
            if (TTC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(TTC_ExecuteEntryCmd_t)))
            {
                TTC_ExecuteEntryCmd((const TTC_ExecuteEntryCmd_t *)SBBufPtr);
                TTC_Debug_PrintTimeline();
            }
            break;

        case TTC_EXECUTE_CMD_GROUP_CC:
            if (TTC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(TTC_ExecuteGroupCmd_t)))
            {
                TTC_ExecuteGroupCmd((const TTC_ExecuteGroupCmd_t *)SBBufPtr);
                TTC_Debug_PrintTimeline();
            }
            break;

        case TTC_PAUSE_TIMELINE_PROCESSING_CC:
            if (TTC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(TTC_PauseTimelineProcessingCmd_t)))
            {
                TTC_PauseTimelineProcessingCmd((const TTC_PauseTimelineProcessingCmd_t *)SBBufPtr);
            }
            break;

        case TTC_RESUME_TIMELINE_PROCESSING_CC:
            if (TTC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(TTC_ResumeTimelineProcessingCmd_t)))
            {
                TTC_ResumeTimelineProcessingCmd((const TTC_ResumeTimelineProcessingCmd_t *)SBBufPtr);
            }
            break;

        case TTC_PLUMB_INIT_ENTRY_CC:
            if (TTC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(TTC_PlumbEntryInitCmd_t)))
            {
                TTC_PlumbEntryInitCmd((const TTC_PlumbEntryInitCmd_t *)SBBufPtr);
            }
            break;

        case TTC_PLUMB_WRITE_ENTRY_CC:
            if (TTC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(TTC_PlumbEntryWriteCmd_t)))
            {
                TTC_PlumbEntryWriteCmd((const TTC_PlumbEntryWriteCmd_t *)SBBufPtr);
            }
            break;

        case TTC_PLUMB_FINALIZE_ENTRY_CC:
            if (TTC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(TTC_PlumbEntryFinalizeCmd_t)))
            {
                TTC_PlumbEntryFinalizeCmd((const TTC_PlumbEntryFinalizeCmd_t *)SBBufPtr);
            }
            break;

        case TTC_PLUMB_DELETE_RESERVED_ENTRY_CC:
            if (TTC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(TTC_PlumbDeleteReservedEntryCmd_t)))
            {
                TTC_PlumbDeleteReservedEntryCmd((const TTC_PlumbDeleteReservedEntryCmd_t *)SBBufPtr);
            }
            break;

        case TTC_PLUMB_PURGE_TIMELINE_CC:
            if (TTC_VerifyCmdLength(&SBBufPtr->Msg, sizeof(TTC_PlumbPurgeTimelineCmd_t)))
            {
                TTC_PlumbPurgeTimelineCmd((const TTC_PlumbPurgeTimelineCmd_t *)SBBufPtr);
            }
            break;

        /* default case already found during FC vs length test */
        default:
            CFE_EVS_SendEvent(TTC_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid ground command code: CC = %d",
                              CommandCode);
            break;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the TTC    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void TTC_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    static CFE_SB_MsgId_t CMD_MID     = CFE_SB_MSGID_RESERVED;
    static CFE_SB_MsgId_t SEND_HK_MID = CFE_SB_MSGID_RESERVED;
    static CFE_SB_MsgId_t ONEHZ_WAKEUP = CFE_SB_MSGID_RESERVED;

    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    /* cache the local MID Values here, this avoids repeat lookups */
    if (!CFE_SB_IsValidMsgId(CMD_MID))
    {
        CMD_MID     = CFE_SB_ValueToMsgId(TTC_CMD_MID);
        SEND_HK_MID = CFE_SB_ValueToMsgId(TTC_SEND_HK_MID);
        ONEHZ_WAKEUP = CFE_SB_ValueToMsgId(TTC_ONEHZ_WAKEUP_MID);
    }

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    /* Process all SB messages */
    if (CFE_SB_MsgId_Equal(MsgId, SEND_HK_MID))
    {
        /* Housekeeping request */
        TTC_SendHkCmd((const TTC_SendHkCmd_t *)SBBufPtr);
    }
    else if (CFE_SB_MsgId_Equal(MsgId, CMD_MID))
    {
        /* Ground command */
        TTC_ProcessGroundCommand(SBBufPtr);
    }
    else if (CFE_SB_MsgId_Equal(MsgId, ONEHZ_WAKEUP))
    {
        /* 1Hz wakeup - process timeline */
        TTC_ProcessTimeline();
    }
    else
    {
        /* Unknown command */
        CFE_EVS_SendEvent(TTC_MID_ERR_EID, CFE_EVS_EventType_ERROR, "TTC: invalid command packet,MID = 0x%x",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId));
    }
}
