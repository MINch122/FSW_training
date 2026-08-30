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
 *   This file contains the source code for the Ttc Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "ttc.h"
#include "ttc_cmds.h"
#include "ttc_msgids.h"
#include "ttc_eventids.h"
#include "ttc_version.h"
#include "ttc_tbl.h"
#include "ttc_utils.h"
#include "ttc_msg.h"
#include "ttc_timeline.h"

#include <string.h>

static void TTC_SendReport(const void *Msg, const void *Data, uint16 DataSize, int32 ReturnCode, uint8 ReturnType)
{
    CFE_SB_MsgId_t    CmdMid;
    CFE_MSG_FcnCode_t CmdCode;
    uint16            CopySize = DataSize > TTC_MISSION_MAX_REPORT_LEN ? TTC_MISSION_MAX_REPORT_LEN : DataSize;

    CFE_MSG_GetMsgId(Msg, &CmdMid);
    CFE_MSG_GetFcnCode(Msg, &CmdCode);

    CFE_MSG_Init(CFE_MSG_PTR(TTC_AppData.Report.TelemetryHeader), CFE_SB_ValueToMsgId(TTC_REPORT_TLM_MID),
                 sizeof(TTC_AppData.Report));
    TTC_AppData.Report.Payload.MsgID          = (uint16)CFE_SB_MsgIdToValue(CmdMid);
    TTC_AppData.Report.Payload.CommandCode    = (uint8)CmdCode;
    TTC_AppData.Report.Payload.ReturnType     = ReturnType;
    TTC_AppData.Report.Payload.ReturnCode     = ReturnCode;
    TTC_AppData.Report.Payload.ReturnDataSize = CopySize;

    memset(TTC_AppData.Report.Payload.ReturnValue, 0, sizeof(TTC_AppData.Report.Payload.ReturnValue));
    if (Data != NULL && CopySize > 0)
    {
        memcpy(TTC_AppData.Report.Payload.ReturnValue, Data, CopySize);
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(TTC_AppData.Report.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(TTC_AppData.Report.TelemetryHeader), true);
}

void TTC_SendHkCmd(const TTC_SendHkCmd_t* Msg)
{
    (void)Msg;

    /*
    ** Get command execution counters...
    */
    TTC_AppData.HkTlm.Payload.CommandErrorCounter = TTC_AppData.ErrCounter;
    TTC_AppData.HkTlm.Payload.CommandCounter      = TTC_AppData.CmdCounter;

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(TTC_AppData.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(TTC_AppData.HkTlm.TelemetryHeader), true);
    TTC_SendReport(Msg, &TTC_AppData.HkTlm.Payload, sizeof(TTC_AppData.HkTlm.Payload), CFE_SUCCESS, 0);
}

void TTC_ReportCmd(const TTC_ReportCmd_t* Msg)
{
    uint16 Counters[2] = {TTC_AppData.CmdCounter, TTC_AppData.ErrCounter};

    TTC_AppData.CmdCounter++;
    TTC_SendReport(Msg, Counters, sizeof(Counters), CFE_SUCCESS, 0);
}

void TTC_NoopCmd(const TTC_NoopCmd_t* Msg)
{
    TTC_AppData.CmdCounter++;

    CFE_EVS_SendEvent(TTC_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "TTC: NOOP command %s",
                      TTC_VERSION);

    static const char NoopReport[] = "TTC NOOP CMD: YOSI IN SPACE";
    TTC_SendReport(Msg, NoopReport, sizeof(NoopReport), CFE_SUCCESS, 0);
}

void TTC_ResetCountersCmd(const TTC_ResetCountersCmd_t* Msg)
{
    TTC_AppData.CmdCounter = 0;
    TTC_AppData.ErrCounter = 0;

    uint16 Counters[2] = {TTC_AppData.CmdCounter, TTC_AppData.ErrCounter};
    TTC_SendReport(Msg, Counters, sizeof(Counters), CFE_SUCCESS, 0);

    CFE_EVS_SendEvent(TTC_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "TTC: RESET command");
}

void TTC_GetTimelineHkCmd(const TTC_GetTimelineHkCmd_t* Msg)
{
    TTC_AppData.CmdCounter++;

    TTC_TimelineHousekeeping_t TimelineHk;
    TTC_TimelineGetHousekeeping(&TimelineHk);
    TTC_SendReport(Msg, &TimelineHk, sizeof(TimelineHk), CFE_SUCCESS, 0);
}

void TTC_ResetTimelineHkCmd(const TTC_ResetTimelineHkCmd_t* Msg)
{
    TTC_AppData.CmdCounter++;

    TTC_TimelineResetHousekeeping();
    TTC_SendReport(Msg, NULL, 0, CFE_SUCCESS, 0);

    CFE_EVS_SendEvent(TTC_RESET_TIMELINE_HK_INF_EID, CFE_EVS_EventType_INFORMATION, "TTC: Timeline HK reset");
}

void TTC_GetPendingEntryCountCmd(const TTC_GetPendingEntryCountCmd_t* Msg)
{
    (void)Msg;

    TTC_AppData.CmdCounter++;

    uint16 PendingCount = TTC_TimelineGetPendingEntryCount();

    TTC_SendReport(Msg, &PendingCount, sizeof(PendingCount), CFE_SUCCESS, 0);

    CFE_EVS_SendEvent(TTC_PENDING_COUNT_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "TTC: Pending timeline entry count: %u", PendingCount);
}

void TTC_GetNextEntryIdCmd(const TTC_GetNextEntryIdCmd_t* Msg)
{
    TTC_AppData.CmdCounter++;

    uint16 EntryId, GroupId;
    uint16 Result[2] = {0, 0};
    CFE_Status_t Status = CFE_SUCCESS;

    if (!TTC_TimelineGetNextEntryId(&EntryId, &GroupId))
    {
        Status = CFE_STATUS_RANGE_ERROR;
        CFE_EVS_SendEvent(TTC_NEXT_ENTRY_ID_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "TTC: No pending entries in timeline");
    }
    else
    {
        Result[0] = EntryId;
        Result[1] = GroupId;
        CFE_EVS_SendEvent(TTC_NEXT_ENTRY_ID_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "TTC: Next entry ID: %u:%u", EntryId, GroupId);
    }

    TTC_SendReport(Msg, Result, sizeof(Result), Status, 0);
}

void TTC_GetNextExecutionTimeCmd(const TTC_GetNextExecutionTimeCmd_t* Msg)
{
    TTC_AppData.CmdCounter++;

    uint32 NextExecutionTime;
    CFE_Status_t Status = CFE_SUCCESS;

    if (!TTC_TimelineGetNextExecutionTime(&NextExecutionTime))
    {
        NextExecutionTime = 0;
        Status = CFE_STATUS_RANGE_ERROR;
        CFE_EVS_SendEvent(TTC_NEXT_EXEC_TIME_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "TTC: No pending entries in timeline");
    }
    else
    {
        CFE_EVS_SendEvent(TTC_NEXT_EXEC_TIME_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "TTC: Next exec time: %u", NextExecutionTime);
    }

    TTC_SendReport(Msg, &NextExecutionTime, sizeof(NextExecutionTime), Status, 0);
}

void TTC_InsertAbsCmdEntryCmd(const TTC_InsertAbsCmdEntryCmd_t* Msg) {
    CFE_Status_t status;
    
    TTC_AppData.CmdCounter++;

    if (Msg->Payload.CommandSize > TTC_PLATFORM_MAX_COMMAND_SIZE) {
        TTC_AppData.ErrCounter++;
        CFE_EVS_SendEvent(TTC_INSERT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "TTC: Cmd size exceeds max allowed (Size: %d, Max: %d)",
                          Msg->Payload.CommandSize, TTC_PLATFORM_MAX_COMMAND_SIZE);
        return;
    }

    status = TTC_TimelineAddAbsoluteEntry(Msg->Payload.EntryId,
                                  Msg->Payload.GroupId,
                                  Msg->Payload.ExecutionTimeAbsolute,
                                  Msg->Payload.StalenessThreshold,
                                  Msg->Payload.ExecutionType,
                                  Msg->Payload.Command,
                                  Msg->Payload.CommandSize);
    if (status != CFE_SUCCESS) {
        TTC_AppData.ErrCounter++;
        CFE_EVS_SendEvent(TTC_INSERT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "TTC: abs cmd entry add err. RC = 0x%08lX (ID %d:%d)",
                          (unsigned long)status, Msg->Payload.GroupId, Msg->Payload.EntryId);
    }
}

void TTC_InsertRelCmdEntryCmd(const TTC_InsertRelCmdEntryCmd_t* Msg)
{
    CFE_Status_t status;
    
    TTC_AppData.CmdCounter++;

    if (Msg->Payload.CommandSize > TTC_PLATFORM_MAX_COMMAND_SIZE) {
        TTC_AppData.ErrCounter++;
        CFE_EVS_SendEvent(TTC_INSERT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "TTC: Cmd size exceeds max (Size: %d, Max: %d)",
                          Msg->Payload.CommandSize, TTC_PLATFORM_MAX_COMMAND_SIZE);
        return;
    }

    status = TTC_TimelineAddRelativeEntry(Msg->Payload.EntryId,
                                  Msg->Payload.GroupId,
                                  Msg->Payload.ExecutionTimeRelative,
                                  Msg->Payload.StalenessThreshold,
                                  Msg->Payload.ExecutionType,
                                  Msg->Payload.Command,
                                  Msg->Payload.CommandSize);
    if (status != CFE_SUCCESS) {
        TTC_AppData.ErrCounter++;
        CFE_EVS_SendEvent(TTC_INSERT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "TTC: rel cmd entry add err. RC = 0x%08lX (ID %d:%d)",
                          (unsigned long)status, Msg->Payload.GroupId, Msg->Payload.EntryId);
    }
}

void TTC_DeleteEntryCmd(const TTC_DeleteEntryCmd_t* Msg)
{
    CFE_Status_t status;
    
    TTC_AppData.CmdCounter++;

    status = TTC_TimelineDeleteEntry(Msg->Payload.EntryId, Msg->Payload.GroupId);
    if (status != CFE_SUCCESS) {
        TTC_AppData.ErrCounter++;
        CFE_EVS_SendEvent(TTC_DELETE_ENTRY_ERR_EID, CFE_EVS_EventType_ERROR,
                          "TTC: delete entry err. RC = 0x%08lX (ID %d:%d)",
                          (unsigned long)status, Msg->Payload.GroupId, Msg->Payload.EntryId);
    }
    else {
        CFE_EVS_SendEvent(TTC_DELETE_ENTRY_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "TTC: Deleted entry %d:%d", Msg->Payload.GroupId, Msg->Payload.EntryId);
    }
}

void TTC_DeleteGroupCmd(const TTC_DeleteGroupCmd_t* Msg)
{
    uint16 DeleteCount;

    TTC_AppData.CmdCounter++;

    /**
     * Be careful with the branch here: we emit the info event if any were deleted.
     */
    if (TTC_TimelineDeleteGroup(Msg->Payload.GroupId, &DeleteCount) == CFE_SUCCESS)
        CFE_EVS_SendEvent(TTC_DELETE_GROUP_INF_EID, CFE_EVS_EventType_INFORMATION,
                        "TTC: Deleted %d entries from group %u", DeleteCount, Msg->Payload.GroupId);
    else
        TTC_AppData.ErrCounter++;
}

void TTC_DeleteAllEntriesCmd(const TTC_DeleteAllEntriesCmd_t* Msg)
{
    (void)Msg;

    TTC_AppData.CmdCounter++;

    TTC_TimelineDeleteAllEntries();


    CFE_EVS_SendEvent(TTC_DELETE_ALL_ENTRIES_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "TTC: Deleted all pending entries from timeline");
}

void TTC_ExecuteEntryCmd(const TTC_ExecuteEntryCmd_t* Msg)
{
    CFE_Status_t status;

    TTC_AppData.CmdCounter++;

    status = TTC_TimelineExecuteEntry(Msg->Payload.EntryId, Msg->Payload.GroupId, Msg->Payload.PersistentExecution);
    if (status != CFE_SUCCESS) {
        TTC_AppData.ErrCounter++;
        CFE_EVS_SendEvent(TTC_EXECUTE_ENTRY_ERR_EID, CFE_EVS_EventType_ERROR,
                          "TTC: execute entry err. RC = 0x%08lX (ID %d:%d, Persistent: %s)",
                          (unsigned long)status, Msg->Payload.GroupId, Msg->Payload.EntryId,
                          Msg->Payload.PersistentExecution ? "t" : "f");
    }
}

void TTC_ExecuteGroupCmd(const TTC_ExecuteGroupCmd_t* Msg)
{
    CFE_Status_t status;

    TTC_AppData.CmdCounter++;

    status = TTC_TimelineExecuteGroup(Msg->Payload.GroupId, Msg->Payload.PersistentExecution);
    if (status != CFE_SUCCESS) {
        TTC_AppData.ErrCounter++;
        CFE_EVS_SendEvent(TTC_EXECUTE_GROUP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "TTC: execute group err. RC = 0x%08lX (ID %d, Persistent: %s)",
                          (unsigned long)status, Msg->Payload.GroupId,
                          Msg->Payload.PersistentExecution ? "t" : "f");
    }
}

void TTC_PauseTimelineProcessingCmd(const TTC_PauseTimelineProcessingCmd_t* Msg)
{
    (void)Msg;

    TTC_AppData.CmdCounter++;

    TTC_TimelinePauseProcessing();

    CFE_EVS_SendEvent(TTC_PAUSE_EXECUTION_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "TTC: Timeline execution paused");
}

void TTC_ResumeTimelineProcessingCmd(const TTC_ResumeTimelineProcessingCmd_t* Msg)
{
    (void)Msg;

    TTC_AppData.CmdCounter++;

    TTC_TimelineResumeProcessing();

    CFE_EVS_SendEvent(TTC_RESUME_EXECUTION_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "TTC: Timeline execution resumed");
}

void TTC_PlumbEntryInitCmd(const TTC_PlumbEntryInitCmd_t* Msg)
{
    CFE_Status_t status;

    TTC_AppData.CmdCounter++;

    status = TTC_Plumb_TimelineAddEntryInit(Msg->Payload.EntryId, Msg->Payload.GroupId);
    if (status != CFE_SUCCESS) {
        TTC_AppData.ErrCounter++;
        CFE_EVS_SendEvent(TTC_PLUMB_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "TTC: plumb init err. RC = 0x%08lX (ID %d:%d)",
                          (unsigned long)status, Msg->Payload.GroupId, Msg->Payload.EntryId);
    }
}

void TTC_PlumbEntryWriteCmd(const TTC_PlumbEntryWriteCmd_t* Msg)
{
    CFE_Status_t status;

    TTC_AppData.CmdCounter++;

    status = TTC_Plumb_TimelineAddEntryWrite(Msg->Payload.EntryId,
                                            Msg->Payload.GroupId,
                                            Msg->Payload.Data,
                                            Msg->Payload.ChunkSize,
                                            Msg->Payload.Offset);
    if (status != CFE_SUCCESS) {
        TTC_AppData.ErrCounter++;
        CFE_EVS_SendEvent(TTC_PLUMB_WRITE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "TTC: plumb write err. RC = 0x%08lX (ID %d:%d)",
                          (unsigned long)status, Msg->Payload.GroupId, Msg->Payload.EntryId);
    }
}

void TTC_PlumbEntryFinalizeCmd(const TTC_PlumbEntryFinalizeCmd_t* Msg)
{
    CFE_Status_t status;

    TTC_AppData.CmdCounter++;

    status = TTC_Plumb_TimelineAddEntryFinalize(Msg->Payload.TimeTagType,
                                                Msg->Payload.EntryId,
                                                Msg->Payload.GroupId,
                                                Msg->Payload.TimeTag,
                                                Msg->Payload.StalenessThreshold,
                                                Msg->Payload.ExecutionType,
                                                Msg->Payload.CmdSize);
    if (status != CFE_SUCCESS) {
        TTC_AppData.ErrCounter++;
        CFE_EVS_SendEvent(TTC_PLUMB_FINALIZE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "TTC: plumb finalize err. RC = 0x%08lX (ID %d:%d)",
                          (unsigned long)status, Msg->Payload.GroupId, Msg->Payload.EntryId);
    }
}

void TTC_PlumbDeleteReservedEntryCmd(const TTC_PlumbDeleteReservedEntryCmd_t* Msg)
{
    CFE_Status_t status;

    TTC_AppData.CmdCounter++;

    status = TTC_Plumb_TimelineDeleteReservedEntry(Msg->Payload.EntryId, Msg->Payload.GroupId);
    if (status != CFE_SUCCESS) {
        TTC_AppData.ErrCounter++;
        CFE_EVS_SendEvent(TTC_PLUMB_DELETE_RESERVED_ERR_EID, CFE_EVS_EventType_ERROR,
                          "TTC: plumb delete reserved err. RC = 0x%08lX (ID %d:%d)",
                          (unsigned long)status, Msg->Payload.GroupId, Msg->Payload.EntryId);
    }
}

void TTC_PlumbPurgeTimelineCmd(const TTC_PlumbPurgeTimelineCmd_t* Msg)
{
    (void)Msg;

    TTC_AppData.CmdCounter++;

    TTC_Plumb_PurgeTimeline();

    CFE_EVS_SendEvent(TTC_PLUMB_PURGE_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "TTC: Timeline purged");
}
