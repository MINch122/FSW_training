#include "ttc_timeline.h"
#include "ttc.h"
#include "ttc_eventids.h"

#include <string.h>


typedef struct {
    /**
     * Ignores execution of timeline entries when true.
     */
    bool ExecutionPaused;

    /**
     * Number of pending entries.
     */
    uint16 Count;

    /**
     * Indices of entries in the timeline sorted by TimeTag in descending
     * order. Index 0 is the farthest in the future, index Count-1 is the
     * next to execute.
     */
    uint16 SortedIndex[TTC_PLATFORM_MAX_COMMAND_ENTRIES];
} TTC_TimelineState_t;

/**
 * Represents a single command entry in the timeline.
 */
typedef struct {
    uint32 TimeTag; /* Absolute execution time in seconds. */
    uint16 StalenessThreshold; /* Maximum allowed staleness in seconds. */
    uint16 EntryId; /* Unique identifier for the entry. */
    uint16 GroupId; /* Group identifier for the entry. */
    uint8  ExecutionType; /* Type of execution for this entry. */
    uint8  Status; /* Current status of the entry. */
    uint16 CmdSize; /* Size of the command in bytes. */
    uint8  Command[TTC_PLATFORM_MAX_COMMAND_SIZE]; /* Command data. */
} TTC_TimelineEntry_t;

/**
 * Per-cycle context.
 */
typedef struct {
    uint32 PreviousTime;        /* Previous cycle time */
    uint16 Executed;            /* Executed entries this cycle */
    uint16 Skipped;             /* Skipped entries this cycle */
    uint16 Aborted;             /* Aborted entries this cycle */
    uint16 ProcessingErrors;    /* Runtime bugs encountered this cycle */
    bool ForwardJumpDetected;   /* Forward time jump detected */
    bool BackwardJumpDetected;  /* Backward time jump detected */
} TTC_TimelineCycleContext_t;

typedef struct {
    TTC_TimelineState_t State;
    TTC_TimelineEntry_t Entries[TTC_PLATFORM_MAX_COMMAND_ENTRIES];
    TTC_TimelineCycleContext_t CycleContext;
} TTC_Timeline_t;

static TTC_Timeline_t Timeline;

static TTC_TimelineHousekeeping_t TimelineHk;

/**
 * Helper to check if the ID pair is the anonymous pair.
 * Anonymous entries are not addressable nor subject to duplicate checks.
 */
static inline bool IsAnonymousEntry(uint16 EntryId, uint16 GroupId)
{
    return EntryId == TTC_PLATFORM_ANONYMOUS_ENTRY_ID &&
           GroupId == TTC_PLATFORM_ANONYMOUS_GROUP_ID;
}

/**
 * Return the index of the first empty slot in timeline - or
 * TTC_PLATFORM_MAX_COMMAND_ENTRIES if full.
 */
static uint16 FindEmptySlot(void)
{
    for (uint16 i = 0; i < TTC_PLATFORM_MAX_COMMAND_ENTRIES; ++i)
        if (Timeline.Entries[i].Status == TTC_STATUS_EMPTY)
            return i;
    
    return TTC_PLATFORM_MAX_COMMAND_ENTRIES;
}

/**
 * Return the index of a RESERVED slot whose EntryId+GroupId match,
 * or TTC_PLATFORM_MAX_COMMAND_ENTRIES if not found.
 * Used by plumb-write and plumb-finalize to locate the in-progress slot.
 */
static uint16 FindReservedSlot(uint16 EntryId, uint16 GroupId)
{
    if (IsAnonymousEntry(EntryId, GroupId))
        return TTC_PLATFORM_MAX_COMMAND_ENTRIES;

    for (uint16 i = 0; i < TTC_PLATFORM_MAX_COMMAND_ENTRIES; ++i)
        if (Timeline.Entries[i].Status == TTC_STATUS_RESERVED &&
            Timeline.Entries[i].EntryId == EntryId &&
            Timeline.Entries[i].GroupId == GroupId)
                return i;

    return TTC_PLATFORM_MAX_COMMAND_ENTRIES;
}

/**
 * Return the index of any non-EMPTY slot whose EntryId+GroupId match,
 * or TTC_PLATFORM_MAX_COMMAND_ENTRIES if not found.
 * Only checks if the entry is empty. Used for duplicate detection.
 */
static uint16 FindSlotByEntryId(uint16 EntryId, uint16 GroupId)
{
    if (IsAnonymousEntry(EntryId, GroupId))
        return TTC_PLATFORM_MAX_COMMAND_ENTRIES;

    for (uint16 i = 0; i < TTC_PLATFORM_MAX_COMMAND_ENTRIES; ++i)
        if (Timeline.Entries[i].Status != TTC_STATUS_EMPTY &&
            Timeline.Entries[i].EntryId == EntryId &&
            Timeline.Entries[i].GroupId == GroupId)
                return i;

    return TTC_PLATFORM_MAX_COMMAND_ENTRIES;
}

/**
 * A simple binary search helper to find the slot where @a TimeTag should be
 * (decreasing order).
 */
static uint16 FindSortedIndex(uint32 TimeTag)
{
    uint16 Top = Timeline.State.Count;
    uint16 Bot = 0;

    while (Bot < Top) {
        uint16 Mid = (Bot + Top) / 2;
        uint16 MidIndex = Timeline.State.SortedIndex[Mid];
        if (TimeTag < Timeline.Entries[MidIndex].TimeTag)
            Bot = Mid + 1;
        else
            Top = Mid;
    }
    return Bot;
}

/**
 * Insert a new slot into the SortedIndex by TimeTag order. Shift later
 * entries up _AND_ increment Count.
 * 
 * Returns true if successful, false if the Slot is out of bounds or the
 * Count is already at the max (TTC_PLATFORM_MAX_COMMAND_ENTRIES).
 */
static bool InsertSlotToSortedIndex(uint32 TimeTag, uint16 Slot)
{
    if (Slot >= TTC_PLATFORM_MAX_COMMAND_ENTRIES)
        return false;

    uint16 Index = FindSortedIndex(TimeTag);

    if (Index > Timeline.State.Count /* This one is a binary search bug and should never happen. */
        ||
        Timeline.State.Count >= TTC_PLATFORM_MAX_COMMAND_ENTRIES /* No more room in the timeline. */)
        return false;

    for (uint16 i = Timeline.State.Count; i > Index; --i)
        Timeline.State.SortedIndex[i] = Timeline.State.SortedIndex[i-1];

    Timeline.State.SortedIndex[Index] = Slot;
    Timeline.State.Count++;

    return true;
}

/**
 * Copy a chunk of command data into a slot's Command buffer at the given byte offset.
 * 
 * Returns false if the write would go out of bounds, true on success.
 */
static CFE_Status_t WriteToSlot(uint16 Slot, const uint8* Data, uint16 Size, uint16 Offset)
{
    if ((uint32)Offset + Size > TTC_PLATFORM_MAX_COMMAND_SIZE)
        return ERR_WRITE_OUT_OF_BOUNDS;

    memcpy(&Timeline.Entries[Slot].Command[Offset], Data, Size);
    return CFE_SUCCESS;
}

/**
 * Mark a slot PENDING and populate its execution metadata.
 * TimeTag must already be expressed as absolute seconds by the caller.
 * 
 * After the commit the command is registered in SortedIndex as a pending entry.
 * Returns CFE_SUCCESS on success, ERR_CMD_SIZE_MISMATCH if CmdSize does not
 * match the actual cmd size, ERR_CMD_SIZE_TOO_LARGE if CmdSize exceeds the maximum,
 * ERR_INSERT_SORTED_INDEX if SortedIndex insertion fails (which should only happen
 * if Count hits the entry limit).
 */
static CFE_Status_t CommitSlot(uint16 Slot,
                               uint32 TimeTag,
                               uint16 StalenessThreshold,
                               uint8  ExecutionType,
                               uint16 CmdSize)
{
    CFE_MSG_Size_t MsgSize;

    CFE_MSG_GetSize((CFE_MSG_Message_t*)Timeline.Entries[Slot].Command, &MsgSize);
    if (MsgSize != CmdSize)
        return ERR_CMD_SIZE_MISMATCH;

    if (CmdSize > TTC_PLATFORM_MAX_COMMAND_SIZE)
        return ERR_CMD_SIZE_TOO_LARGE;

    if (InsertSlotToSortedIndex(TimeTag, Slot) == false)
        /* Pending entry limit reached. */
        return ERR_INSERT_SORTED_INDEX;

    Timeline.Entries[Slot].TimeTag            = TimeTag;
    Timeline.Entries[Slot].StalenessThreshold = StalenessThreshold;
    Timeline.Entries[Slot].ExecutionType      = ExecutionType;
    Timeline.Entries[Slot].CmdSize            = CmdSize;
    Timeline.Entries[Slot].Status             = TTC_STATUS_PENDING;

    return CFE_SUCCESS;
}

/**
 * Just clear the slot (no state changes).
 */
static void CancelSlot(uint16 Slot)
{
    memset(&Timeline.Entries[Slot], 0, sizeof(TTC_TimelineEntry_t));

}

/**
 * Clear the slot and remove it from the sorted index. Decrement Count.
 * 
 * Returns true if the slot was found in SortedIndex and deleted, false if not found.
 */
static bool DeleteSlot(uint16 Slot)
{
    for (int i = Timeline.State.Count - 1; i >= 0; --i) {
        if (Timeline.State.SortedIndex[i] == Slot) {
            /* Remove this slot from the sorted index */
            CancelSlot(Slot);
            for (int j = i; j < Timeline.State.Count - 1; ++j)
                Timeline.State.SortedIndex[j] = Timeline.State.SortedIndex[j + 1];
            Timeline.State.Count--;
            return true;
        }
    }
    return false;
}

/**
 * Helper to execute a pending entry. The entry is identified by its slot and assumed to be pending.
 */
static void ExecuteEntry(uint16 Slot, bool PersistentExecution)
{
    if (Slot >= TTC_PLATFORM_MAX_COMMAND_ENTRIES)
        return;

    TTC_TimelineEntry_t* Entry = &Timeline.Entries[Slot];

    if (Entry->Status != TTC_STATUS_PENDING) {
        /* Shouldn't happen. Corrupted entry or invalid call point. */
        Timeline.CycleContext.ProcessingErrors++;
        CFE_EVS_SendEvent(TTC_INVALID_ENTRY_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                      "TTC: entry %u:%u has invalid status %u during exec. Cleaning.",
                      Entry->GroupId, Entry->EntryId, Entry->Status);
        if (DeleteSlot(Slot) == false)
            /* This slot was not in the sorted index, just cancel it. */
            CancelSlot(Slot);
        return;
    }

    CFE_SB_TransmitMsg((CFE_MSG_Message_t*) Entry->Command, true);

    if (!PersistentExecution)
        DeleteSlot(Slot);
    Timeline.CycleContext.Executed++;
}

static void SkipEntry(uint16 Slot)
{
    if (Slot >= TTC_PLATFORM_MAX_COMMAND_ENTRIES)
        return;

    TTC_TimelineEntry_t* Entry = &Timeline.Entries[Slot];

    CFE_EVS_SendEvent(TTC_SKIPPING_ENTRY_INF_EID, CFE_EVS_EventType_INFORMATION,
                  "TTC: Skipping entry %u:%u sched for %u",
                  Entry->GroupId, Entry->EntryId, Entry->TimeTag);

    DeleteSlot(Slot);
    Timeline.CycleContext.Skipped++;
}

static void AbortPendingEntries(void)
{
    CFE_EVS_SendEvent(TTC_ABORTING_ENTRIES_INF_EID, CFE_EVS_EventType_INFORMATION,
                  "TTC: Aborting %u pending entries",
                  Timeline.State.Count);

    for (int i = Timeline.State.Count - 1; i >= 0; --i)
        CancelSlot(Timeline.State.SortedIndex[i]);

    Timeline.CycleContext.Aborted += Timeline.State.Count;
    Timeline.State.Count = 0;
}

/**
 * Retrieve the next pending entry's index in the timeline.
 */
static inline uint16 GetNextSlot(void)
{
    if (Timeline.State.Count == 0)
        return TTC_PLATFORM_MAX_COMMAND_ENTRIES;

    return Timeline.State.SortedIndex[Timeline.State.Count - 1];
}

/* -----------------------------------------------------------------------
 * Public API
 * ----------------------------------------------------------------------- */

void TTC_TimelineInitialize(void)
{
    memset(&TimelineHk, 0, sizeof(TimelineHk));
    memset(&Timeline, 0, sizeof(Timeline));
    Timeline.CycleContext.PreviousTime = CFE_TIME_GetTime().Seconds;
}

void TTC_TimelineGetHousekeeping(TTC_TimelineHousekeeping_t* HkBuffer)
{
    if (!HkBuffer)
        return;

    memcpy(HkBuffer, &TimelineHk, sizeof(TTC_TimelineHousekeeping_t));
}

void TTC_TimelineResetHousekeeping(void)
{
    memset(&TimelineHk, 0, sizeof(TimelineHk));
}

uint16 TTC_TimelineGetPendingEntryCount(void)
{
    return Timeline.State.Count;
}

bool TTC_TimelineGetNextEntryId(uint16* EntryId, uint16* GroupId)
{
    uint16 NextSlot = GetNextSlot();
    if (NextSlot >= TTC_PLATFORM_MAX_COMMAND_ENTRIES)
        return false;

    if (EntryId)
        *EntryId = Timeline.Entries[NextSlot].EntryId;
    if (GroupId)
        *GroupId = Timeline.Entries[NextSlot].GroupId;

    return true;
}

bool TTC_TimelineGetNextExecutionTime(uint32* TimeTag)
{
    uint16 NextSlot = GetNextSlot();
    if (NextSlot >= TTC_PLATFORM_MAX_COMMAND_ENTRIES)
        return false;

    if (TimeTag)
        *TimeTag = Timeline.Entries[NextSlot].TimeTag;

    return true;
}

CFE_Status_t TTC_TimelineAddEntry(uint8        TimeTagType,
                                  uint16       EntryId,
                                  uint16       GroupId,
                                  uint32       TimeTag,
                                  uint16       StalenessThreshold,
                                  uint8        ExecutionType,
                                  const uint8* CmdData,
                                  uint16       CmdDataSize)
{
    CFE_Status_t status;

    if (CmdDataSize > TTC_PLATFORM_MAX_COMMAND_SIZE)
        return ERR_CMD_SIZE_TOO_LARGE;

    if (!IsAnonymousEntry(EntryId, GroupId) &&
        FindSlotByEntryId(EntryId, GroupId) != TTC_PLATFORM_MAX_COMMAND_ENTRIES)
        return ERR_DUPLICATE_ENTRY;

    uint16 Slot = FindEmptySlot();
    if (Slot == TTC_PLATFORM_MAX_COMMAND_ENTRIES)
        return ERR_NO_EMPTY_SLOT;

    Timeline.Entries[Slot].Status  = TTC_STATUS_RESERVED;
    Timeline.Entries[Slot].EntryId = EntryId;
    Timeline.Entries[Slot].GroupId = GroupId;

    uint32 AbsTimeTag = TimeTag;
    uint32 Now = CFE_TIME_GetTime().Seconds;

    if (TimeTagType == TTC_TIMETAG_TYPE_ABSOLUTE && AbsTimeTag < Now) {
        CancelSlot(Slot);
        return ERR_TIME_TAG_IN_PAST;
    }

    if (TimeTagType == TTC_TIMETAG_TYPE_RELATIVE)
        AbsTimeTag = Now + TimeTag;

    status = WriteToSlot(Slot, CmdData, CmdDataSize, 0);
    if (status != CFE_SUCCESS) {
        CancelSlot(Slot);
        return status;
    }

    status = CommitSlot(Slot, AbsTimeTag, StalenessThreshold, ExecutionType, CmdDataSize);

    if (status != CFE_SUCCESS)
        CancelSlot(Slot);

    return status;
}

CFE_Status_t TTC_TimelineDeleteEntry(uint16 EntryId, uint16 GroupId)
{
    if (IsAnonymousEntry(EntryId, GroupId))
        return ERR_ANONYMOUS_ID;

    uint16 Slot = FindSlotByEntryId(EntryId, GroupId);
    if (Slot == TTC_PLATFORM_MAX_COMMAND_ENTRIES)
        return ERR_ENTRY_NOT_FOUND;

    return DeleteSlot(Slot) ? CFE_SUCCESS : ERR_ENTRY_NOT_FOUND;
}

CFE_Status_t TTC_TimelineDeleteGroup(uint16 GroupId, uint16* Deleted)
{
    int DelCount = 0;
    bool AnonymousGroup = (GroupId == TTC_PLATFORM_ANONYMOUS_GROUP_ID);

    /**
     * Must iterate backwards since DeleteSlot decrements Count.
     */
    for (int i = Timeline.State.Count - 1; i >= 0; --i) {
        uint16 Slot = Timeline.State.SortedIndex[i];
        if (Timeline.Entries[Slot].GroupId == GroupId) {
            if (AnonymousGroup && Timeline.Entries[Slot].EntryId == TTC_PLATFORM_ANONYMOUS_ENTRY_ID)
                continue;
            DeleteSlot(Slot);
            DelCount++; 
        }
    }

    if (Deleted)
        *Deleted = DelCount;

    return DelCount > 0 ? CFE_SUCCESS : ERR_ENTRY_NOT_FOUND;
}

CFE_Status_t TTC_TimelineDeleteAllEntries(void)
{
    for (uint16 i = 0; i < Timeline.State.Count; ++i)
        CancelSlot(Timeline.State.SortedIndex[i]);

    Timeline.State.Count = 0;

    return CFE_SUCCESS;
}

CFE_Status_t TTC_TimelineExecuteEntry(uint16 EntryId,
                                      uint16 GroupId,
                                      bool PersistentExecution)
{
    if (IsAnonymousEntry(EntryId, GroupId))
        return ERR_ANONYMOUS_ID;

    uint16 Slot = FindSlotByEntryId(EntryId, GroupId);
    if (Slot == TTC_PLATFORM_MAX_COMMAND_ENTRIES)
        return ERR_ENTRY_NOT_FOUND;

    if (Timeline.Entries[Slot].Status != TTC_STATUS_PENDING)
        return ERR_ENTRY_NOT_PENDING;

    ExecuteEntry(Slot, PersistentExecution);

    return CFE_SUCCESS;
}

CFE_Status_t TTC_TimelineExecuteGroup(uint16 GroupId,
                                      bool PersistentExecution)
{
    bool AnonymousGroup = (GroupId == TTC_PLATFORM_ANONYMOUS_GROUP_ID);
    bool Found = false;

    /**
     * Must iterate backwards since ExecuteEntry/DeleteSlot modifies Count.
     */
    for (int i = Timeline.State.Count - 1; i >= 0; --i) {
        uint16 Slot = Timeline.State.SortedIndex[i];
        if (Timeline.Entries[Slot].GroupId == GroupId) {
            if (AnonymousGroup && Timeline.Entries[Slot].EntryId == TTC_PLATFORM_ANONYMOUS_ENTRY_ID)
                continue;
            if (Timeline.Entries[Slot].Status != TTC_STATUS_PENDING)
                continue;
            Found = true;
            ExecuteEntry(Slot, PersistentExecution);
        }
    }

    return Found ? CFE_SUCCESS : ERR_ENTRY_NOT_FOUND;
}

void TTC_TimelinePauseProcessing(void)
{
    Timeline.State.ExecutionPaused = true;
}

void TTC_TimelineResumeProcessing(void)
{
    Timeline.State.ExecutionPaused = false;
}

CFE_Status_t TTC_Plumb_TimelineAddEntryInit(uint16 EntryId,
                                            uint16 GroupId)
{
    if (IsAnonymousEntry(EntryId, GroupId))
        return ERR_ANONYMOUS_ID;

    if (FindSlotByEntryId(EntryId, GroupId) != TTC_PLATFORM_MAX_COMMAND_ENTRIES)
        return ERR_DUPLICATE_ENTRY;

    uint16 Slot = FindEmptySlot();
    if (Slot == TTC_PLATFORM_MAX_COMMAND_ENTRIES)
        return ERR_NO_EMPTY_SLOT;

    memset(&Timeline.Entries[Slot], 0, sizeof(TTC_TimelineEntry_t));
    Timeline.Entries[Slot].EntryId = EntryId;
    Timeline.Entries[Slot].GroupId = GroupId;
    Timeline.Entries[Slot].TimeTag = TTC_TIMETAG_MAX;
    Timeline.Entries[Slot].Status  = TTC_STATUS_RESERVED;

    return CFE_SUCCESS;
}

CFE_Status_t TTC_Plumb_TimelineAddEntryWrite(uint16       EntryId,
                                             uint16       GroupId,
                                             const uint8* CmdChunk,
                                             uint16       ChunkSize,
                                             uint16       Offset)
{
    if (IsAnonymousEntry(EntryId, GroupId))
        return ERR_ANONYMOUS_ID;

    uint16 Slot = FindReservedSlot(EntryId, GroupId);
    if (Slot == TTC_PLATFORM_MAX_COMMAND_ENTRIES)
        return ERR_ENTRY_NOT_FOUND;

    return WriteToSlot(Slot, CmdChunk, ChunkSize, Offset);
}

CFE_Status_t TTC_Plumb_TimelineAddEntryFinalize(uint8  TimeTagType,
                                                uint16 EntryId,
                                                uint16 GroupId,
                                                uint32 TimeTag,
                                                uint16 StalenessThreshold,
                                                uint8  ExecutionType,
                                                uint16 CmdSize)
{
    if (IsAnonymousEntry(EntryId, GroupId))
        return ERR_ANONYMOUS_ID;

    if (CmdSize == 0 || CmdSize > TTC_PLATFORM_MAX_COMMAND_SIZE)
        return ERR_CMD_SIZE_TOO_LARGE;

    uint16 Slot = FindReservedSlot(EntryId, GroupId);
    if (Slot == TTC_PLATFORM_MAX_COMMAND_ENTRIES)
        return ERR_ENTRY_NOT_FOUND;

    uint32 AbsTimeTag = TimeTag;
    if (TimeTagType == TTC_TIMETAG_TYPE_RELATIVE)
        AbsTimeTag = CFE_TIME_GetTime().Seconds + TimeTag;

    return CommitSlot(Slot, AbsTimeTag, StalenessThreshold, ExecutionType, CmdSize);
}

CFE_Status_t TTC_Plumb_TimelineDeleteReservedEntry(uint16 EntryId, uint16 GroupId)
{
    if (IsAnonymousEntry(EntryId, GroupId))
        return ERR_ANONYMOUS_ID;

    uint16 Slot = FindReservedSlot(EntryId, GroupId);
    if (Slot == TTC_PLATFORM_MAX_COMMAND_ENTRIES)
        return ERR_ENTRY_NOT_FOUND;

    CancelSlot(Slot);
    return CFE_SUCCESS;
}

/**
 * Completely clear the timeline including reserved entries and cycle context.
 */
static void PurgeTimeline(void)
{
    bool ExecutionPaused = Timeline.State.ExecutionPaused;
    memset(&Timeline, 0, sizeof(Timeline));
    Timeline.CycleContext.PreviousTime = CFE_TIME_GetTime().Seconds;
    Timeline.State.ExecutionPaused = ExecutionPaused;
}

CFE_Status_t TTC_Plumb_PurgeTimeline(void)
{
    PurgeTimeline();
    return CFE_SUCCESS;
}

static void UpdateTimelineHousekeeping(void)
{
    uint16 NextSlot = GetNextSlot();
    TimelineHk.CmdExecuted += Timeline.CycleContext.Executed;
    TimelineHk.CmdSkipped  += Timeline.CycleContext.Skipped;
    TimelineHk.CmdAborted  += Timeline.CycleContext.Aborted;
    TimelineHk.ForwardJumpDetectionCount += Timeline.CycleContext.ForwardJumpDetected;
    TimelineHk.BackwardJumpDetectionCount += Timeline.CycleContext.BackwardJumpDetected;
    TimelineHk.ProcessingErrorCount += Timeline.CycleContext.ProcessingErrors;
    if (NextSlot < TTC_PLATFORM_MAX_COMMAND_ENTRIES) {
        TimelineHk.NextExecutionTime = Timeline.Entries[NextSlot].TimeTag;
        TimelineHk.NextEntryId = Timeline.Entries[NextSlot].EntryId;
        TimelineHk.NextEntryGroupId = Timeline.Entries[NextSlot].GroupId;
    }
    else {
        TimelineHk.NextExecutionTime = TTC_TIMETAG_MAX;
        TimelineHk.NextEntryId = 0;
        TimelineHk.NextEntryGroupId = 0;
    }
    TimelineHk.CmdPending = Timeline.State.Count;
    TimelineHk.ExecutionPaused = Timeline.State.ExecutionPaused;
}

static void InitCycleContext(void)
{
    Timeline.CycleContext.Executed = 0;
    Timeline.CycleContext.Skipped = 0;
    Timeline.CycleContext.Aborted = 0;
    Timeline.CycleContext.ProcessingErrors = 0;
    Timeline.CycleContext.ForwardJumpDetected = false;
    Timeline.CycleContext.BackwardJumpDetected = false;
}

void TTC_ProcessTimeline(void)
{
    uint32 PreviousTime;
    uint32 CurrentTime;

    InitCycleContext();

    CurrentTime = CFE_TIME_GetTime().Seconds;
    PreviousTime = Timeline.CycleContext.PreviousTime;

    if (CurrentTime < PreviousTime) {
        /**
         * We had a backward time jump.
         */
        CFE_EVS_SendEvent(TTC_TIME_JUMP_BACKWARD_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "TTC: Bwd jmp detected: now %u, prev %u",
                      CurrentTime, PreviousTime);
        Timeline.CycleContext.BackwardJumpDetected = true;
    }

    if (CurrentTime > PreviousTime + 1) {
        /**
         * We had a forward time jump.
         */
        CFE_EVS_SendEvent(TTC_TIME_JUMP_FORWARD_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "TTC: Fwd jmp detected: now %u, prev %u",
                      CurrentTime, PreviousTime);
        Timeline.CycleContext.ForwardJumpDetected = true;
    }

    if (Timeline.State.ExecutionPaused) {
        Timeline.CycleContext.PreviousTime = CurrentTime;
        UpdateTimelineHousekeeping();
        return;
    }

    while (Timeline.State.Count > 0) {
        /**
         * Retrieve the next pending entry.
         */
        uint16 NextSlot = GetNextSlot();
        if (NextSlot >= TTC_PLATFORM_MAX_COMMAND_ENTRIES) {
            /**
             * If this happens we're in hell and nothing can be trusted.
             */
            Timeline.CycleContext.ProcessingErrors++;
            CFE_EVS_SendEvent(TTC_INVALID_NEXT_SLOT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "TTC: Inval next slot ind %u. Clearing timeline.", NextSlot);
            PurgeTimeline();
            break;
        }

        const TTC_TimelineEntry_t* NextEntry = &Timeline.Entries[NextSlot];
        uint32 NextExecutionTime = NextEntry->TimeTag;

        if (CurrentTime < NextExecutionTime) {
            /**
             * No more entries are ready to execute.
             */
            break;
        }

        if (CurrentTime - NextExecutionTime > NextEntry->StalenessThreshold) {
            /**
             * This command is stale.
             */
            switch (NextEntry->ExecutionType) {
            case TTC_EXECTYPE_SKIP_LATE:
                SkipEntry(NextSlot);
                break;
            case TTC_EXECTYPE_ABORT_LATE:
                AbortPendingEntries();
                break;
            case TTC_EXECTYPE_FORCE:
                ExecuteEntry(NextSlot, false);
                break;
            default:
                /* Invalid execution type. Shouldn't happen. */
                Timeline.CycleContext.Skipped++;
                Timeline.CycleContext.ProcessingErrors++;
                CFE_EVS_SendEvent(TTC_INVALID_EXEC_TYPE_ERR_EID, CFE_EVS_EventType_ERROR,
                                "TTC: Inv exec type %u for entry %u:%u",
                                NextEntry->ExecutionType, NextEntry->GroupId, NextEntry->EntryId);
                DeleteSlot(NextSlot);
                break;
            }
        }
        else {
            /**
             * This command is ready to execute.
             */
            ExecuteEntry(NextSlot, false);
        }
    }
    
    /**
     * Update states.
     */
    Timeline.CycleContext.PreviousTime = CurrentTime;

    UpdateTimelineHousekeeping();

    return;
}

void TTC_Debug_PrintTimeline(void)
{
    uint32 Now = CFE_TIME_GetTime().Seconds;
    uint16 Count = Timeline.State.Count;

    /* Header */
    printf("\n+------+--------+-------+------------+-----------+-----------+-----------+\n");
    printf("|         TTC TIMELINE   entries: %3u/%-3u   Now: %-10u   %s      |\n",
            Count, TTC_PLATFORM_MAX_COMMAND_ENTRIES, Now,
            Timeline.State.ExecutionPaused ? "PAUSED" : "RUN   ");
    printf("+------+--------+-------+------------+-----------+-----------+-----------+\n");
    printf("| Slot | Entry  | Group |  TimeTag   |   dt (s)  | ExecType  |  Status   |\n");
    printf("+------+--------+-------+------------+-----------+-----------+-----------+\n");

    if (Count == 0)
        printf("|                         (no entries)                                   |\n");
    else {
        /* SortedIndex is descending; index [Count-1] fires next */
        for (int i = Count - 1; i >= 0; --i) {
            uint16 Slot  = Timeline.State.SortedIndex[i];
            TTC_TimelineEntry_t* E = &Timeline.Entries[Slot];
            int32 dt = (int32)E->TimeTag - (int32)Now;

            const char* exec_str;
            switch (E->ExecutionType) {
            case TTC_EXECTYPE_SKIP_LATE:  exec_str = "SKIP_LATE"; break;
            case TTC_EXECTYPE_FORCE:      exec_str = "FORCE    "; break;
            case TTC_EXECTYPE_ABORT_LATE: exec_str = "ABORT_LT "; break;
            default:                      exec_str = "???      "; break;
            }

            const char* stat_str;
            switch (E->Status) {
                case TTC_STATUS_PENDING:  stat_str = "PENDING  "; break;
                case TTC_STATUS_RESERVED: stat_str = "RESERVED "; break;
                default:                  stat_str = "???      "; break;
            }

            printf("| %4u | %6u | %5u | %10u | %+9d | %s | %s |\n",
                    Slot, E->EntryId, E->GroupId, E->TimeTag, dt,
                    exec_str, stat_str);
        }
    }

    /* Footer with mini bar for the next 60 s */
    printf("+------+--------+-------+------------+-----------+-----------+-----------+\n");

    if (Count > 0) {
        /* Gantt bar: 60-character window, each char = 1 second */
        #define BAR_WIDTH 60
        char bar[BAR_WIDTH + 1];
        memset(bar, '.', BAR_WIDTH);
        bar[BAR_WIDTH] = '\0';

        for (uint16 i = 0; i < Count; ++i) {
            uint16 Slot = Timeline.State.SortedIndex[i];
            int32  dt   = (int32)Timeline.Entries[Slot].TimeTag - (int32)Now;
            if (dt >= 0 && dt < BAR_WIDTH)
                bar[dt] = '*';
            else if (dt < 0)
                bar[0] = '!';   /* overdue */
        }

        printf("| now [%s] +%ds\n", bar, BAR_WIDTH);
        uint16 NextSlot = GetNextSlot();
        if (NextSlot < TTC_PLATFORM_MAX_COMMAND_ENTRIES)
            printf("| next exec in: %+d s\n",
                    (int32)Timeline.Entries[NextSlot].TimeTag - (int32)Now);
    }

    printf("\n");
}
