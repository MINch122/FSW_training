/*
** File: coveragetest_ttc_timeline.c
**
** Purpose:
** Coverage Unit Test cases for ttc_timeline.c.
**
*/

#include "ttc_coveragetest_common.h"
#include "ttc_timeline.h"

/* -----------------------------------------------------------------------
 * Helpers
 * ----------------------------------------------------------------------- */

/** Snapshot the timeline's internal housekeeping counters. */
static TTC_TimelineHousekeeping_t GetHk(void)
{
    TTC_TimelineHousekeeping_t Hk;
    memset(&Hk, 0, sizeof(Hk));
    TTC_TimelineGetHousekeeping(&Hk);
    return Hk;
}

/** Prime CFE_MSG_GetSize to report the given message size once.
 *  AllocateCopy=true is required: the value must outlive this helper so the
 *  stub still has it when the code under test later calls CFE_MSG_GetSize. */
static void SetMsgSize(CFE_MSG_Size_t Size)
{
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &Size, sizeof(Size), true);
}

/** Prime CFE_TIME_GetTime to return the given MET second count once. */
static void SetCurrentTime(uint32 Seconds)
{
    CFE_TIME_SysTime_t T = {.Seconds = Seconds, .Subseconds = 0};
    UT_SetDataBuffer(UT_KEY(CFE_TIME_GetTime), &T, sizeof(T), true);
}

/**
 * Add a complete one-shot entry (TTC_TIMETAG_TYPE_ABSOLUTE) and prime
 * CFE_MSG_GetSize so CommitSlot accepts it.
 */
static CFE_Status_t AddEntry(uint16 EntryId, uint16 GroupId,
                              uint32 TimeTag,  uint16 Staleness,
                              uint8  ExecType, uint16 CmdSize)
{
    uint8 Cmd[TTC_PLATFORM_MAX_COMMAND_SIZE];
    memset(Cmd, 0, CmdSize);
    SetMsgSize(CmdSize);
    return TTC_TimelineAddEntry(TTC_TIMETAG_TYPE_ABSOLUTE,
                                EntryId, GroupId, TimeTag, Staleness,
                                ExecType, Cmd, CmdSize);
}


/* -----------------------------------------------------------------------
 * TTC_TimelineAddEntry
 * ----------------------------------------------------------------------- */

void Test_TTC_TimelineAddEntry_Nominal(void)
{
    TTC_TimelineInitialize();
    UtAssert_INT32_EQ(AddEntry(1, 0, 1000, 5, TTC_EXECTYPE_SKIP_LATE, 12),
                      CFE_SUCCESS);
}

void Test_TTC_TimelineAddEntry_SizeTooLarge(void)
{
    uint8 Cmd[1];

    TTC_TimelineInitialize();
    UtAssert_INT32_EQ(
        TTC_TimelineAddEntry(TTC_TIMETAG_TYPE_ABSOLUTE, 1, 0, 1000, 5,
                             TTC_EXECTYPE_SKIP_LATE,
                             Cmd, TTC_PLATFORM_MAX_COMMAND_SIZE + 1),
        ERR_CMD_SIZE_TOO_LARGE);
}

void Test_TTC_TimelineAddEntry_Duplicate(void)
{
    uint8 Cmd[12] = {0};

    TTC_TimelineInitialize();
    UtAssert_INT32_EQ(AddEntry(1, 0, 1000, 5, TTC_EXECTYPE_SKIP_LATE, 12),
                      CFE_SUCCESS);

    /* Same EntryId+GroupId must be rejected regardless of TimeTag */
    UtAssert_INT32_EQ(
        TTC_TimelineAddEntry(TTC_TIMETAG_TYPE_ABSOLUTE, 1, 0, 2000, 5,
                             TTC_EXECTYPE_SKIP_LATE, Cmd, sizeof(Cmd)),
        ERR_DUPLICATE_ENTRY);
}

void Test_TTC_TimelineAddEntry_NoEmptySlot(void)
{
    uint8 Cmd[12];

    TTC_TimelineInitialize();

    /* Fill all slots via Plumb_Init — no CFE_MSG_GetSize priming needed */
    /* IDs start at 1: (0,0) is the anonymous pair and would be rejected,
     * leaving a slot free and defeating the "timeline full" precondition. */
    for (uint16 i = 1; i <= TTC_PLATFORM_MAX_COMMAND_ENTRIES; i++)
        TTC_Plumb_TimelineAddEntryInit(i, 0);

    SetMsgSize(sizeof(Cmd));
    UtAssert_INT32_EQ(
        TTC_TimelineAddEntry(TTC_TIMETAG_TYPE_ABSOLUTE, 200, 0, 1000, 5,
                             TTC_EXECTYPE_SKIP_LATE, Cmd, sizeof(Cmd)),
        ERR_NO_EMPTY_SLOT);
}

void Test_TTC_TimelineAddEntry_CmdSizeMismatch(void)
{
    uint8 Cmd[12] = {0};

    TTC_TimelineInitialize();
    /* CFE_MSG_GetSize reports a size that doesn't match the CmdDataSize arg */
    SetMsgSize(99);
    UtAssert_INT32_EQ(
        TTC_TimelineAddEntry(TTC_TIMETAG_TYPE_ABSOLUTE, 1, 0, 1000, 5,
                             TTC_EXECTYPE_SKIP_LATE, Cmd, sizeof(Cmd)),
        ERR_CMD_SIZE_MISMATCH);
}

void Test_TTC_TimelineAddEntry_RelativeTimeTag(void)
{
    uint8 Cmd[12] = {0};

    /* Now=500; relative offset=100 → stored internally as absolute 600 */
    TTC_TimelineInitialize();

    SetCurrentTime(500); /* consumed by AddEntry as the relative base */
    SetMsgSize(sizeof(Cmd));
    UtAssert_INT32_EQ(
        TTC_TimelineAddEntry(TTC_TIMETAG_TYPE_RELATIVE, 1, 0, 100, 0,
                             TTC_EXECTYPE_SKIP_LATE, Cmd, sizeof(Cmd)),
        CFE_SUCCESS);

    /* Staleness=0: 600-600=0 which is NOT > 0 → executes on time */
    SetCurrentTime(600);
    TTC_ProcessTimeline();
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);
}


/* -----------------------------------------------------------------------
 * Plumb (chunked-uplink) flow
 * ----------------------------------------------------------------------- */

void Test_TTC_PlumbFlow_Nominal(void)
{
    /* Build a 20-byte command in two 10-byte chunks */
    uint8 Chunk1[10], Chunk2[10];
    memset(Chunk1, 0x11, sizeof(Chunk1));
    memset(Chunk2, 0x22, sizeof(Chunk2));

    TTC_TimelineInitialize();

    UtAssert_INT32_EQ(TTC_Plumb_TimelineAddEntryInit(1, 0), CFE_SUCCESS);
    UtAssert_INT32_EQ(
        TTC_Plumb_TimelineAddEntryWrite(1, 0, Chunk1, 10, 0), CFE_SUCCESS);
    UtAssert_INT32_EQ(
        TTC_Plumb_TimelineAddEntryWrite(1, 0, Chunk2, 10, 10), CFE_SUCCESS);

    SetMsgSize(20);
    UtAssert_INT32_EQ(
        TTC_Plumb_TimelineAddEntryFinalize(TTC_TIMETAG_TYPE_ABSOLUTE,
                                           1, 0, 1000, 5,
                                           TTC_EXECTYPE_SKIP_LATE, 20),
        CFE_SUCCESS);
}

void Test_TTC_PlumbFlow_InitDuplicate(void)
{
    TTC_TimelineInitialize();
    UtAssert_INT32_EQ(TTC_Plumb_TimelineAddEntryInit(1, 0), CFE_SUCCESS);
    UtAssert_INT32_EQ(TTC_Plumb_TimelineAddEntryInit(1, 0), ERR_DUPLICATE_ENTRY);
}

void Test_TTC_PlumbFlow_InitNoEmptySlot(void)
{
    TTC_TimelineInitialize();
    /* IDs start at 1: (0,0) is the anonymous pair and would be rejected,
     * leaving a slot free and defeating the "timeline full" precondition. */
    for (uint16 i = 1; i <= TTC_PLATFORM_MAX_COMMAND_ENTRIES; i++)
        TTC_Plumb_TimelineAddEntryInit(i, 0);

    UtAssert_INT32_EQ(TTC_Plumb_TimelineAddEntryInit(200, 0), ERR_NO_EMPTY_SLOT);
}

void Test_TTC_PlumbFlow_WriteNotFound(void)
{
    uint8 Chunk[10] = {0};

    TTC_TimelineInitialize();
    /* No RESERVED slot exists → write must fail */
    UtAssert_INT32_EQ(
        TTC_Plumb_TimelineAddEntryWrite(1, 0, Chunk, 10, 0),
        ERR_ENTRY_NOT_FOUND);
}

void Test_TTC_PlumbFlow_WriteOutOfBounds(void)
{
    uint8 Chunk[10] = {0};

    TTC_TimelineInitialize();
    UtAssert_INT32_EQ(TTC_Plumb_TimelineAddEntryInit(1, 0), CFE_SUCCESS);

    /* Offset + Size straddles the end of the command buffer:
     * (MAX_COMMAND_SIZE-5) + 10 > MAX_COMMAND_SIZE */
    UtAssert_INT32_EQ(
        TTC_Plumb_TimelineAddEntryWrite(1, 0, Chunk, 10,
                                        TTC_PLATFORM_MAX_COMMAND_SIZE - 5),
        ERR_WRITE_OUT_OF_BOUNDS);
}

void Test_TTC_PlumbFlow_FinalizeNotFound(void)
{
    TTC_TimelineInitialize();
    /* No RESERVED slot exists */
    UtAssert_INT32_EQ(
        TTC_Plumb_TimelineAddEntryFinalize(TTC_TIMETAG_TYPE_ABSOLUTE,
                                           1, 0, 1000, 5,
                                           TTC_EXECTYPE_SKIP_LATE, 12),
        ERR_ENTRY_NOT_FOUND);
}

void Test_TTC_PlumbFlow_FinalizeZeroSize(void)
{
    TTC_TimelineInitialize();
    UtAssert_INT32_EQ(TTC_Plumb_TimelineAddEntryInit(1, 0), CFE_SUCCESS);
    UtAssert_INT32_EQ(
        TTC_Plumb_TimelineAddEntryFinalize(TTC_TIMETAG_TYPE_ABSOLUTE,
                                           1, 0, 1000, 5,
                                           TTC_EXECTYPE_SKIP_LATE, 0),
        ERR_CMD_SIZE_TOO_LARGE);
}


/* -----------------------------------------------------------------------
 * TTC_ProcessTimeline — dispatch logic
 * ----------------------------------------------------------------------- */

void Test_TTC_ProcessTimeline_Empty(void)
{
    TTC_TimelineInitialize();
    SetCurrentTime(1000);
    TTC_ProcessTimeline(); /* must not crash with an empty timeline */
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 0);
}

void Test_TTC_ProcessTimeline_NotYetReady(void)
{
    TTC_TimelineInitialize();

    UtAssert_INT32_EQ(AddEntry(1, 0, 2000, 9999, TTC_EXECTYPE_SKIP_LATE, 12),
                      CFE_SUCCESS);
    SetCurrentTime(1000); /* entry not due until T=2000 */
    TTC_ProcessTimeline();

    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 0);
}

void Test_TTC_ProcessTimeline_Paused(void)
{
    TTC_TimelineInitialize();

    UtAssert_INT32_EQ(AddEntry(1, 0, 900, 9999, TTC_EXECTYPE_SKIP_LATE, 12),
                      CFE_SUCCESS);
    TTC_TimelinePauseProcessing();
    SetCurrentTime(9999);
    TTC_ProcessTimeline();

    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 0);
}

void Test_TTC_ProcessTimeline_NominalExecution(void)
{
    TTC_TimelineInitialize();

    /* Staleness=200; 1000-900=100 is NOT > 200 → on-time execution */
    UtAssert_INT32_EQ(AddEntry(1, 0, 900, 200, TTC_EXECTYPE_SKIP_LATE, 12),
                      CFE_SUCCESS);
    SetCurrentTime(1000);
    TTC_ProcessTimeline();

    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);
    UtAssert_UINT32_EQ(GetHk().CmdExecuted, 1);
}

void Test_TTC_ProcessTimeline_StaleSkip(void)
{
    TTC_TimelineInitialize();

    /* Staleness=5; 1000-900=100 > 5 → stale, SKIP_LATE */
    UtAssert_INT32_EQ(AddEntry(1, 0, 900, 5, TTC_EXECTYPE_SKIP_LATE, 12),
                      CFE_SUCCESS);
    SetCurrentTime(1000);
    TTC_ProcessTimeline();

    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 0);
    UtAssert_UINT32_EQ(GetHk().CmdSkipped, 1);
}

void Test_TTC_ProcessTimeline_StaleAbort(void)
{
    TTC_TimelineInitialize();

    /* Earlier entry (T=800) is ABORT_LATE; later entry (T=900) is NORMAL.
     * The T=800 entry is at the tail of SortedIndex and encountered first,
     * aborting both entries before the T=900 entry can execute. */
    UtAssert_INT32_EQ(AddEntry(1, 0, 800, 5, TTC_EXECTYPE_ABORT_LATE, 12),
                      CFE_SUCCESS);
    UtAssert_INT32_EQ(AddEntry(2, 0, 900, 5, TTC_EXECTYPE_SKIP_LATE, 12),
                      CFE_SUCCESS);
    SetCurrentTime(1000);
    TTC_ProcessTimeline();

    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 0);
    UtAssert_UINT32_EQ(GetHk().CmdExecuted, 0);
    UtAssert_UINT32_EQ(GetHk().CmdAborted, 2);
}

void Test_TTC_ProcessTimeline_StaleForce(void)
{
    TTC_TimelineInitialize();

    /* Staleness=5; 1000-900=100 > 5 → stale, FORCE → transmit anyway */
    UtAssert_INT32_EQ(AddEntry(1, 0, 900, 5, TTC_EXECTYPE_FORCE, 12),
                      CFE_SUCCESS);
    SetCurrentTime(1000);
    TTC_ProcessTimeline();

    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);
    UtAssert_UINT32_EQ(GetHk().CmdExecuted, 1);
}
/*
 * The stale-switch "default" branch in TTC_ProcessTimeline is only reachable
 * with an execution type outside the three valid enum values.  CommitSlot
 * rejects such entries up front (ERR_INVALID_EXEC_TYPE), so that branch is
 * unreachable through the public API — verify the guard that makes it so.
 */
void Test_TTC_TimelineAddEntry_InvalidExecType(void)
{
    uint8 Cmd[12] = {0};

    TTC_TimelineInitialize();
    SetMsgSize(sizeof(Cmd));
    UtAssert_INT32_EQ(
        TTC_TimelineAddEntry(TTC_TIMETAG_TYPE_ABSOLUTE, 1, 0, 1000, 5,
                             99 /* not a valid TTC_EXECTYPE_* value */,
                             Cmd, sizeof(Cmd)),
        ERR_INVALID_EXEC_TYPE);
}

void Test_TTC_ProcessTimeline_ForwardTimeJump(void)
{
    TTC_TimelineInitialize();

    /* Warm-up: establish PreviousTime=100 (the initial call from PreviousTime=0
     * itself triggers a jump, so reset the counter after). */
    SetCurrentTime(100);
    TTC_ProcessTimeline();
    TTC_TimelineResetHousekeeping();

    /* Advance by exactly 1 second → no jump */
    SetCurrentTime(101);
    TTC_ProcessTimeline();
    UtAssert_UINT32_EQ(GetHk().ForwardJumpDetectionCount, 0);

    /* Advance by 10 seconds → jump detected */
    SetCurrentTime(111);
    TTC_ProcessTimeline();
    UtAssert_UINT32_EQ(GetHk().ForwardJumpDetectionCount, 1);
}

void Test_TTC_ProcessTimeline_BackwardTimeJump(void)
{
    TTC_TimelineInitialize();

    SetCurrentTime(1000);
    TTC_ProcessTimeline();
    TTC_TimelineResetHousekeeping();

    /* Going backward must not crash, must be counted as a backward jump,
     * and must not count as a forward jump. */
    SetCurrentTime(500);
    TTC_ProcessTimeline();

    UtAssert_UINT32_EQ(GetHk().ForwardJumpDetectionCount, 0);
    UtAssert_UINT32_EQ(GetHk().BackwardJumpDetectionCount, 1);
}

void Test_TTC_ProcessTimeline_SortOrder(void)
{
    /* Three entries added out of time order; all should execute in one call. */
    TTC_TimelineInitialize();

    UtAssert_INT32_EQ(AddEntry(1, 0, 300, 9999, TTC_EXECTYPE_SKIP_LATE, 12), CFE_SUCCESS);
    UtAssert_INT32_EQ(AddEntry(2, 0, 100, 9999, TTC_EXECTYPE_SKIP_LATE, 12), CFE_SUCCESS);
    UtAssert_INT32_EQ(AddEntry(3, 0, 200, 9999, TTC_EXECTYPE_SKIP_LATE, 12), CFE_SUCCESS);

    SetCurrentTime(9999);
    TTC_ProcessTimeline();

    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 3);
}

void Test_TTC_InsertSortedIndex_SlotGuardBug(void)
{
    /*
     * Regression test for the InsertSlotToSortedIndex guard.  The guard must
     * bound the sorted *position* (Index/Pos), not the entry-*slot* index:
     *
     *   1. Plumb_Init reserves slot 0 → Count stays 0.
     *   2. AddEntry finds slot 1 as the first empty slot.
     *   3. CommitSlot calls InsertSlotToSortedIndex(TimeTag, Slot=1) which
     *      computes Index=0 (the correct sorted position).
     *   4. A slot-indexed guard ("Count < Slot") would wrongly reject the
     *      insert (0 < 1) and leave the entry out of SortedIndex, so
     *      ProcessTimeline would transmit nothing.
     *
     * With the correct position-indexed guard the entry is scheduled and
     * transmitted, which this test asserts.
     */
    TTC_TimelineInitialize();

    UtAssert_INT32_EQ(TTC_Plumb_TimelineAddEntryInit(1, 0), CFE_SUCCESS);
    UtAssert_INT32_EQ(AddEntry(2, 0, 500, 9999, TTC_EXECTYPE_SKIP_LATE, 12),
                      CFE_SUCCESS);

    SetCurrentTime(500);
    TTC_ProcessTimeline();

    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);
}


/* -----------------------------------------------------------------------
 * Pause / resume
 * ----------------------------------------------------------------------- */

void Test_TTC_PauseResume(void)
{
    TTC_TimelineInitialize();

    UtAssert_INT32_EQ(AddEntry(1, 0, 900, 9999, TTC_EXECTYPE_SKIP_LATE, 12),
                      CFE_SUCCESS);

    /* Paused: entry is due but must not be dispatched */
    TTC_TimelinePauseProcessing();
    SetCurrentTime(9999);
    TTC_ProcessTimeline();
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 0);

    /* Resumed: entry now dispatched */
    TTC_TimelineResumeProcessing();
    SetCurrentTime(9999);
    TTC_ProcessTimeline();
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);
}


/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    ADD_TEST(TTC_TimelineAddEntry_Nominal);
    ADD_TEST(TTC_TimelineAddEntry_SizeTooLarge);
    ADD_TEST(TTC_TimelineAddEntry_Duplicate);
    ADD_TEST(TTC_TimelineAddEntry_NoEmptySlot);
    ADD_TEST(TTC_TimelineAddEntry_CmdSizeMismatch);
    ADD_TEST(TTC_TimelineAddEntry_InvalidExecType);
    ADD_TEST(TTC_TimelineAddEntry_RelativeTimeTag);
    ADD_TEST(TTC_PlumbFlow_Nominal);
    ADD_TEST(TTC_PlumbFlow_InitDuplicate);
    ADD_TEST(TTC_PlumbFlow_InitNoEmptySlot);
    ADD_TEST(TTC_PlumbFlow_WriteNotFound);
    ADD_TEST(TTC_PlumbFlow_WriteOutOfBounds);
    ADD_TEST(TTC_PlumbFlow_FinalizeNotFound);
    ADD_TEST(TTC_PlumbFlow_FinalizeZeroSize);
    ADD_TEST(TTC_ProcessTimeline_Empty);
    ADD_TEST(TTC_ProcessTimeline_NotYetReady);
    ADD_TEST(TTC_ProcessTimeline_Paused);
    ADD_TEST(TTC_ProcessTimeline_NominalExecution);
    ADD_TEST(TTC_ProcessTimeline_StaleSkip);
    ADD_TEST(TTC_ProcessTimeline_StaleAbort);
    ADD_TEST(TTC_ProcessTimeline_StaleForce);
    ADD_TEST(TTC_ProcessTimeline_ForwardTimeJump);
    ADD_TEST(TTC_ProcessTimeline_BackwardTimeJump);
    ADD_TEST(TTC_ProcessTimeline_SortOrder);
    ADD_TEST(TTC_InsertSortedIndex_SlotGuardBug);
    ADD_TEST(TTC_PauseResume);
}
