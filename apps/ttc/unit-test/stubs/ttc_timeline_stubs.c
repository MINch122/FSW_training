/*
** File: ttc_timeline_stubs.c
**
** Purpose:
** Auto-generated stub implementations for functions defined in ttc_timeline.h.
** Linked into the coverage-ttc-stubs library so other test runners (e.g.
** coveragetest_ttc, coveragetest_ttc_cmds) can satisfy references to timeline
** functions without compiling ttc_timeline.c itself.
**
** NOTE: This must stay in sync with the public API in ttc_timeline.h.  Every
** non-inline function declared there needs a stub here.
*/

#include "ttc_timeline.h"
#include "utgenstub.h"

void TTC_TimelineInitialize(void)
{
    UT_GenStub_Execute(TTC_TimelineInitialize, Basic, NULL);
}

void TTC_TimelineGetHousekeeping(TTC_TimelineHousekeeping_t *HkBuffer)
{
    UT_GenStub_AddParam(TTC_TimelineGetHousekeeping, TTC_TimelineHousekeeping_t *, HkBuffer);

    UT_GenStub_Execute(TTC_TimelineGetHousekeeping, Basic, NULL);
}

void TTC_TimelineResetHousekeeping(void)
{
    UT_GenStub_Execute(TTC_TimelineResetHousekeeping, Basic, NULL);
}

uint16 TTC_TimelineGetPendingEntryCount(void)
{
    UT_GenStub_SetupReturnBuffer(TTC_TimelineGetPendingEntryCount, uint16);

    UT_GenStub_Execute(TTC_TimelineGetPendingEntryCount, Basic, NULL);

    return UT_GenStub_GetReturnValue(TTC_TimelineGetPendingEntryCount, uint16);
}

bool TTC_TimelineGetNextEntryId(uint16 *EntryId, uint16 *GroupId)
{
    UT_GenStub_SetupReturnBuffer(TTC_TimelineGetNextEntryId, bool);

    UT_GenStub_AddParam(TTC_TimelineGetNextEntryId, uint16 *, EntryId);
    UT_GenStub_AddParam(TTC_TimelineGetNextEntryId, uint16 *, GroupId);

    UT_GenStub_Execute(TTC_TimelineGetNextEntryId, Basic, NULL);

    return UT_GenStub_GetReturnValue(TTC_TimelineGetNextEntryId, bool);
}

bool TTC_TimelineGetNextExecutionTime(uint32 *TimeTag)
{
    UT_GenStub_SetupReturnBuffer(TTC_TimelineGetNextExecutionTime, bool);

    UT_GenStub_AddParam(TTC_TimelineGetNextExecutionTime, uint32 *, TimeTag);

    UT_GenStub_Execute(TTC_TimelineGetNextExecutionTime, Basic, NULL);

    return UT_GenStub_GetReturnValue(TTC_TimelineGetNextExecutionTime, bool);
}

CFE_Status_t TTC_TimelineAddEntry(uint8        TimeTagType,
                                  uint16       EntryId,
                                  uint16       GroupId,
                                  uint32       TimeTag,
                                  uint16       StalenessThreshold,
                                  uint8        ExecutionType,
                                  const uint8 *CmdData,
                                  uint16       CmdDataSize)
{
    UT_GenStub_SetupReturnBuffer(TTC_TimelineAddEntry, CFE_Status_t);

    UT_GenStub_AddParam(TTC_TimelineAddEntry, uint8,         TimeTagType);
    UT_GenStub_AddParam(TTC_TimelineAddEntry, uint16,        EntryId);
    UT_GenStub_AddParam(TTC_TimelineAddEntry, uint16,        GroupId);
    UT_GenStub_AddParam(TTC_TimelineAddEntry, uint32,        TimeTag);
    UT_GenStub_AddParam(TTC_TimelineAddEntry, uint16,        StalenessThreshold);
    UT_GenStub_AddParam(TTC_TimelineAddEntry, uint8,         ExecutionType);
    UT_GenStub_AddParam(TTC_TimelineAddEntry, const uint8 *, CmdData);
    UT_GenStub_AddParam(TTC_TimelineAddEntry, uint16,        CmdDataSize);

    UT_GenStub_Execute(TTC_TimelineAddEntry, Basic, NULL);

    return UT_GenStub_GetReturnValue(TTC_TimelineAddEntry, CFE_Status_t);
}

CFE_Status_t TTC_TimelineDeleteEntry(uint16 EntryId, uint16 GroupId)
{
    UT_GenStub_SetupReturnBuffer(TTC_TimelineDeleteEntry, CFE_Status_t);

    UT_GenStub_AddParam(TTC_TimelineDeleteEntry, uint16, EntryId);
    UT_GenStub_AddParam(TTC_TimelineDeleteEntry, uint16, GroupId);

    UT_GenStub_Execute(TTC_TimelineDeleteEntry, Basic, NULL);

    return UT_GenStub_GetReturnValue(TTC_TimelineDeleteEntry, CFE_Status_t);
}

CFE_Status_t TTC_TimelineDeleteGroup(uint16 GroupId, uint16 *DeleteCount)
{
    UT_GenStub_SetupReturnBuffer(TTC_TimelineDeleteGroup, CFE_Status_t);

    UT_GenStub_AddParam(TTC_TimelineDeleteGroup, uint16,   GroupId);
    UT_GenStub_AddParam(TTC_TimelineDeleteGroup, uint16 *, DeleteCount);

    UT_GenStub_Execute(TTC_TimelineDeleteGroup, Basic, NULL);

    return UT_GenStub_GetReturnValue(TTC_TimelineDeleteGroup, CFE_Status_t);
}

CFE_Status_t TTC_TimelineDeleteAllEntries(void)
{
    UT_GenStub_SetupReturnBuffer(TTC_TimelineDeleteAllEntries, CFE_Status_t);

    UT_GenStub_Execute(TTC_TimelineDeleteAllEntries, Basic, NULL);

    return UT_GenStub_GetReturnValue(TTC_TimelineDeleteAllEntries, CFE_Status_t);
}

CFE_Status_t TTC_TimelineExecuteEntry(uint16 EntryId,
                                      uint16 GroupId,
                                      bool   PersistentExecution)
{
    UT_GenStub_SetupReturnBuffer(TTC_TimelineExecuteEntry, CFE_Status_t);

    UT_GenStub_AddParam(TTC_TimelineExecuteEntry, uint16, EntryId);
    UT_GenStub_AddParam(TTC_TimelineExecuteEntry, uint16, GroupId);
    UT_GenStub_AddParam(TTC_TimelineExecuteEntry, bool,   PersistentExecution);

    UT_GenStub_Execute(TTC_TimelineExecuteEntry, Basic, NULL);

    return UT_GenStub_GetReturnValue(TTC_TimelineExecuteEntry, CFE_Status_t);
}

CFE_Status_t TTC_TimelineExecuteGroup(uint16 GroupId,
                                      bool   PersistentExecution)
{
    UT_GenStub_SetupReturnBuffer(TTC_TimelineExecuteGroup, CFE_Status_t);

    UT_GenStub_AddParam(TTC_TimelineExecuteGroup, uint16, GroupId);
    UT_GenStub_AddParam(TTC_TimelineExecuteGroup, bool,   PersistentExecution);

    UT_GenStub_Execute(TTC_TimelineExecuteGroup, Basic, NULL);

    return UT_GenStub_GetReturnValue(TTC_TimelineExecuteGroup, CFE_Status_t);
}

void TTC_TimelinePauseProcessing(void)
{
    UT_GenStub_Execute(TTC_TimelinePauseProcessing, Basic, NULL);
}

void TTC_TimelineResumeProcessing(void)
{
    UT_GenStub_Execute(TTC_TimelineResumeProcessing, Basic, NULL);
}

CFE_Status_t TTC_Plumb_TimelineAddEntryInit(uint16 EntryId, uint16 GroupId)
{
    UT_GenStub_SetupReturnBuffer(TTC_Plumb_TimelineAddEntryInit, CFE_Status_t);

    UT_GenStub_AddParam(TTC_Plumb_TimelineAddEntryInit, uint16, EntryId);
    UT_GenStub_AddParam(TTC_Plumb_TimelineAddEntryInit, uint16, GroupId);

    UT_GenStub_Execute(TTC_Plumb_TimelineAddEntryInit, Basic, NULL);

    return UT_GenStub_GetReturnValue(TTC_Plumb_TimelineAddEntryInit, CFE_Status_t);
}

CFE_Status_t TTC_Plumb_TimelineAddEntryWrite(uint16       EntryId,
                                             uint16       GroupId,
                                             const uint8 *CmdChunk,
                                             uint16       ChunkSize,
                                             uint16       Offset)
{
    UT_GenStub_SetupReturnBuffer(TTC_Plumb_TimelineAddEntryWrite, CFE_Status_t);

    UT_GenStub_AddParam(TTC_Plumb_TimelineAddEntryWrite, uint16,        EntryId);
    UT_GenStub_AddParam(TTC_Plumb_TimelineAddEntryWrite, uint16,        GroupId);
    UT_GenStub_AddParam(TTC_Plumb_TimelineAddEntryWrite, const uint8 *, CmdChunk);
    UT_GenStub_AddParam(TTC_Plumb_TimelineAddEntryWrite, uint16,        ChunkSize);
    UT_GenStub_AddParam(TTC_Plumb_TimelineAddEntryWrite, uint16,        Offset);

    UT_GenStub_Execute(TTC_Plumb_TimelineAddEntryWrite, Basic, NULL);

    return UT_GenStub_GetReturnValue(TTC_Plumb_TimelineAddEntryWrite, CFE_Status_t);
}

CFE_Status_t TTC_Plumb_TimelineAddEntryFinalize(uint8  TimeTagType,
                                                uint16 EntryId,
                                                uint16 GroupId,
                                                uint32 TimeTag,
                                                uint16 StalenessThreshold,
                                                uint8  ExecutionType,
                                                uint16 CmdSize)
{
    UT_GenStub_SetupReturnBuffer(TTC_Plumb_TimelineAddEntryFinalize, CFE_Status_t);

    UT_GenStub_AddParam(TTC_Plumb_TimelineAddEntryFinalize, uint8,  TimeTagType);
    UT_GenStub_AddParam(TTC_Plumb_TimelineAddEntryFinalize, uint16, EntryId);
    UT_GenStub_AddParam(TTC_Plumb_TimelineAddEntryFinalize, uint16, GroupId);
    UT_GenStub_AddParam(TTC_Plumb_TimelineAddEntryFinalize, uint32, TimeTag);
    UT_GenStub_AddParam(TTC_Plumb_TimelineAddEntryFinalize, uint16, StalenessThreshold);
    UT_GenStub_AddParam(TTC_Plumb_TimelineAddEntryFinalize, uint8,  ExecutionType);
    UT_GenStub_AddParam(TTC_Plumb_TimelineAddEntryFinalize, uint16, CmdSize);

    UT_GenStub_Execute(TTC_Plumb_TimelineAddEntryFinalize, Basic, NULL);

    return UT_GenStub_GetReturnValue(TTC_Plumb_TimelineAddEntryFinalize, CFE_Status_t);
}

CFE_Status_t TTC_Plumb_TimelineDeleteReservedEntry(uint16 EntryId, uint16 GroupId)
{
    UT_GenStub_SetupReturnBuffer(TTC_Plumb_TimelineDeleteReservedEntry, CFE_Status_t);

    UT_GenStub_AddParam(TTC_Plumb_TimelineDeleteReservedEntry, uint16, EntryId);
    UT_GenStub_AddParam(TTC_Plumb_TimelineDeleteReservedEntry, uint16, GroupId);

    UT_GenStub_Execute(TTC_Plumb_TimelineDeleteReservedEntry, Basic, NULL);

    return UT_GenStub_GetReturnValue(TTC_Plumb_TimelineDeleteReservedEntry, CFE_Status_t);
}

CFE_Status_t TTC_Plumb_PurgeTimeline(void)
{
    UT_GenStub_SetupReturnBuffer(TTC_Plumb_PurgeTimeline, CFE_Status_t);

    UT_GenStub_Execute(TTC_Plumb_PurgeTimeline, Basic, NULL);

    return UT_GenStub_GetReturnValue(TTC_Plumb_PurgeTimeline, CFE_Status_t);
}

void TTC_ProcessTimeline(void)
{
    UT_GenStub_Execute(TTC_ProcessTimeline, Basic, NULL);
}

void TTC_Debug_PrintTimeline(void)
{
    UT_GenStub_Execute(TTC_Debug_PrintTimeline, Basic, NULL);
}
