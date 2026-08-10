#ifndef _TTC_TIMELINE_H_
#define _TTC_TIMELINE_H_

#include "cfe.h"
#include "ttc_internal_cfg.h"


/* Sentinel stored in TimeTag of a freshly-reserved slot */
#define TTC_TIMETAG_MAX 0xFFFFFFFFU

/* TimeTagType values for TTC_TimelineAddEntry and TTC_Plumb_TimelineAddEntryFinalize */
#define TTC_TIMETAG_TYPE_ABSOLUTE 0  /* TimeTag is MET seconds */
#define TTC_TIMETAG_TYPE_RELATIVE 1  /* TimeTag is offset from current MET seconds */


/**
 * Determines how the pending command is processed if the current
 * time is late than the tagged time (e.g., time jump by sync).
 */
typedef enum {
    TTC_EXECTYPE_SKIP_LATE  = 0,    /* Skip if late.                         */
    TTC_EXECTYPE_FORCE      = 1,    /* Always execute no matter how late.    */
    TTC_EXECTYPE_ABORT_LATE = 2,    /* Skip, and abort all remaining commands if late. */
} TTC_CommandExecutionType_t;

/**
 * @brief Internal command entry status. 
 */
typedef enum {
    TTC_STATUS_EMPTY        = 0,    /* Entry is empty and ready for use.    */
    TTC_STATUS_PENDING      = 1,    /* Entry is pending execution.          */
    TTC_STATUS_RESERVED     = 2,    /* Entry is reserved for plumbing but not ready.  */
} TTC_CommandStatus_t;

typedef enum {
    ERR_OK = 0,
    ERR_NO_EMPTY_SLOT,
    ERR_CMD_SIZE_TOO_LARGE,
    ERR_INVALID_EXEC_TYPE,
    ERR_INVALID_TIME_TAG_TYPE,
    ERR_ENTRY_NOT_FOUND,
    ERR_DUPLICATE_ENTRY,
    ERR_WRITE_OUT_OF_BOUNDS,
    ERR_CMD_SIZE_MISMATCH,
    ERR_TIME_TAG_IN_PAST,
    ERR_ANONYMOUS_ID,
    ERR_INSERT_SORTED_INDEX,
    ERR_ENTRY_NOT_PENDING,
} TTC_Error_t;

typedef struct {
    uint32 NextExecutionTime;
    uint16 NextEntryId;
    uint16 NextEntryGroupId;
    uint16 CmdPending;

    uint32 CmdExecuted;
    uint32 CmdSkipped;
    uint32 CmdAborted;
    uint32 ForwardJumpDetectionCount;
    uint32 BackwardJumpDetectionCount;
    uint32 ProcessingErrorCount;

    bool ExecutionPaused;
} TTC_TimelineHousekeeping_t;


/**
 * @brief Initialize the timeline data structure.
 */
void TTC_TimelineInitialize(void);

/**
 * @brief Copy the current internal timeline housekeeping data.
 * 
 * @param HkBuffer Housekeeing buffer.
 */
void TTC_TimelineGetHousekeeping(TTC_TimelineHousekeeping_t* HkBuffer);

/**
 * @brief Reset the internal timeline housekeeping data to zeros.
 */
void TTC_TimelineResetHousekeeping(void);

/**
 * @brief Return the number of pending entries in the timeline.
 */
uint16 TTC_TimelineGetPendingEntryCount(void);

/**
 * @brief Get the EntryId and GroupId of the next pending entry in the timeline.
 *        Pointer arguments are null-allowed.
 * 
 * @return true if a pending entry exists, false if no pending entry exists.
 */
bool TTC_TimelineGetNextEntryId(uint16* EntryId, uint16* GroupId);

/**
 * @brief Get the TimeTag of the next pending entry in the timeline.
 * 
 *        If the execution has been paused by TTC_TimelinePauseExecution,
 *        this will still return the next entry's TimeTag that might be in the past.
 * 
 * @return true if a pending entry exists, false if no pending entry exists.
 */
bool TTC_TimelineGetNextExecutionTime(uint32* TimeTag);

/**
 * @brief Add a command entry to the timeline.
 * 
 *        Upon successful return, the command is added to the timeline and will be
 *        executed when CFE_TIME reaches the specified TimeTag.
 * 
 *        EntryID and GroupId are a unique, user-specified ID pair to be referenced
 *        by other calls (e.g., delete).
 *        "Anonymous" entries with EntryId = TTC_PLATFORM_ANONYMOUS_ENTRY_ID and
 *        GroupId = TTC_PLATFORM_ANONYMOUS_GROUP_ID are also supported, which skip the
 *        duplicate ID check but cannot be referenced by later calls.
 * 
 * @param TimeTagType   TTC_TIMETAG_TYPE_ABSOLUTE or TTC_TIMETAG_TYPE_RELATIVE
 * @param EntryId       User-specified entry ID.
 * @param GroupId       User-specified group ID.
 * @param TimeTag       Absolute or relative time tag in seconds.
 * @param StalenessThreshold Number of seconds after the TimeTag that the command is still valid. 
 *                           If the command is pending before TimeTag + StalenessThreshold
 *                           (e.g., due to a time jump), it will be considered stale and processed
 *                           according to ExecutionType.
 * @param ExecutionType TTC_EXECTYPE_SKIP_LATE, TTC_EXECTYPE_FORCE, or TTC_EXECTYPE_ABORT_LATE.
 *                      See #TTC_CommandExecutionType_t for details.
 * @param CmdData       A full CCSDS command packet to add.
 * @param CmdDataSize   Size of @a CmdData in bytes.
 * 
 * @return CFE_SUCCESS: Success.
 *         ERR_CMD_SIZE_TOO_LARGE: CmdDataSize exceeds TTC_PLATFORM_MAX_COMMAND_SIZE.
 *         ERR_DUPLICATE_ENTRY: An entry with the same EntryId and GroupId already exists (unless anonymous IDs).
 *         ERR_NO_EMPTY_SLOT: Timeline is full and cannot accept more entries.
 *         ERR_TIME_TAG_IN_PAST: TimeTagType is TTC_TIMETAG_TYPE_ABSOLUTE and TimeTag is in the past.
 */
CFE_Status_t TTC_TimelineAddEntry(uint8  TimeTagType,
                                  uint16 EntryId,
                                  uint16 GroupId,
                                  uint32 TimeTag,
                                  uint16 StalenessThreshold,
                                  uint8 ExecutionType,
                                  const uint8* CmdData,
                                  uint16 CmdDataSize);

/**
 * @brief Add a command entry with absolute time tag. See #TTC_TimelineAddEntry for details.
 */
static inline CFE_Status_t TTC_TimelineAddAbsoluteEntry(uint16 EntryId,
                                            uint16 GroupId,
                                            uint32 TimeTag,
                                            uint16 StalenessThreshold,
                                            uint8  ExecutionType,
                                            const uint8* CmdData,
                                            uint16 CmdDataSize)
{
    return TTC_TimelineAddEntry(TTC_TIMETAG_TYPE_ABSOLUTE, EntryId, GroupId, TimeTag, StalenessThreshold, ExecutionType, CmdData, CmdDataSize);
}

/**
 * @brief Add a command entry with relative time tag. See #TTC_TimelineAddEntry for details.
 */
static inline CFE_Status_t TTC_TimelineAddRelativeEntry(uint16 EntryId,
                                            uint16 GroupId,
                                            uint32 TimeOffset,
                                            uint16 StalenessThreshold,
                                            uint8  ExecutionType,
                                            const uint8* CmdData,
                                            uint16 CmdDataSize)
{
    return TTC_TimelineAddEntry(TTC_TIMETAG_TYPE_RELATIVE, EntryId, GroupId, TimeOffset, StalenessThreshold, ExecutionType, CmdData, CmdDataSize);
}

/**
 * @brief Delete a pending entry with the given EntryId and GroupId.
 *        Cannot delete anonymous entries (TTC_PLATFORM_ANONYMOUS_ENTRY_ID
 *        and TTC_PLATFORM_ANONYMOUS_GROUP_ID pair) or reserved entries.
 * 
 * @param EntryId  Entry ID to delete.
 * @param GroupId  Group ID to delete.
 * @return CFE_SUCCESS: Success.
 *         ERR_ENTRY_NOT_FOUND: No matching entry was found.
 *         ERR_ANONYMOUS_ID: Target entry is anonymous.
 */
CFE_Status_t TTC_TimelineDeleteEntry(uint16 EntryId, uint16 GroupId);

/**
 * @brief Delete all entries with the given GroupId.
 *        If GroupId is TTC_PLATFORM_ANONYMOUS_GROUP_ID, delete all entries
 *        except those with EntryId TTC_PLATFORM_ANONYMOUS_ENTRY_ID.
 * 
 * @param GroupId  Group ID to delete.
 * @param[out] Deleted Number of entries deleted. NULL allowed.
 *
 * @return CFE_SUCCESS: if any entries were deleted,
 *         ERR_ENTRY_NOT_FOUND: no matching entries were found.
 */
CFE_Status_t TTC_TimelineDeleteGroup(uint16 GroupId, uint16* DeleteCount);

/**
 * @brief Delete all entries in the timeline.
 *        Does _NOT_ delete reserved entries that are being plumbed.
 * 
 * @return Always returns CFE_SUCCESS. 
 */
CFE_Status_t TTC_TimelineDeleteAllEntries(void);

/**
 * @brief Immediately execute a pending entry regardless of its TimeTag or ExecutionType.
 *        Execution by this interface does not increment the housekeeping counter.
 * 
 * @param EntryId Pending entry ID to execute.
 * @param GroupId Pending group ID to execute.
 * @param PersistentExecution If false, the entry will be deleted after execution.
 *                            Otherwise the entry is kept at the pending status.
 * @return CFE_SUCCESS: Success.
 *         ERR_ENTRY_NOT_FOUND: No pending entry matches the EntryId and GroupId.
 *         ERR_ANONYMOUS_ID: Target entry is anonymous.
 *         ERR_ENTRY_NOT_PENDING: Target entry not in pending status (perhaps reserved for plumbing).
 */
CFE_Status_t TTC_TimelineExecuteEntry(uint16 EntryId,
                                      uint16 GroupId,
                                      bool PersistentExecution);

/**
 * @brief Immediately execute all pending entries in the given group regardless 
 *        of their TimeTag or ExecutionType.
 *        Executions by this interface do not increment the housekeeping counter.
 *
 *        The execution is in increasing order of their TimeTags, not the entry IDs.
 *        entries If the given GroupId is TTC_PLATFORM_ANONYMOUS_GROUP_ID, execute
 *        all pending except those with EntryId TTC_PLATFORM_ANONYMOUS_ENTRY_ID
 *        (That is, only the anonymous entries are excluded).
 * 
 * @param GroupId Pending group ID to execute.
 * @param PersistentExecution If false, the entries will be deleted after execution.
 *                            Otherwise the entries are kept at the pending status.
 * @return CFE_SUCCESS: Success.
 *         ERR_ENTRY_NOT_FOUND: No pending entries match the GroupId.
 */
CFE_Status_t TTC_TimelineExecuteGroup(uint16 GroupId,
                                      bool PersistentExecution);

/**
 * @brief Pause timeline processing. Pending entries will not be executed
 *        until processing is resumed.
 */
void TTC_TimelinePauseProcessing(void);

/**
 * @brief Resume timeline processing. Pending entries with TimeTag expired will
 *        be processed according to their ExecutionType.
 */
void TTC_TimelineResumeProcessing(void);

/**
 * @brief Plumbing cmd add interface 1 of 3: initialize a reserved entry for later writing and committing.
 * 
 *        This interface allows plumbing a command entry in multiple steps, which is useful
 *        for limited uplink bandwidths where a command needs to be sent in multiple chunks.
 * 
 *        The reserved entry is not visible to the timeline until finalized, and can be
 *        referenced by EntryId and GroupId like a normal entry for writing and committing.
 * 
 * @param EntryId   User-specified entry ID. Must not be the anonymous pair.
 * @param GroupId   User-specified group ID. Must not be the anonymous pair.
 * 
 * @return CFE_SUCCESS: Success.
 *         ERR_ANONYMOUS_ID: EntryId and GroupId pair is the anonymous pair.
 *         ERR_DUPLICATE_ENTRY: An entry with the same EntryId and GroupId already exists.
 *         ERR_NO_EMPTY_SLOT: Timeline is full and cannot accept more entries.
 */
CFE_Status_t TTC_Plumb_TimelineAddEntryInit(uint16 EntryId,
                                            uint16 GroupId);

/**
 * @brief Plumbing cmd add interface 2 of 3: write a chunk of command data to a reserved entry.
 *        This can be called multiple times until the full command is written.
 * 
 * @param EntryId   Target entry ID at the reserved status.
 * @param GroupId   Target group ID at the reserved status.
 * @param CmdChunk  Command data chunk.
 * @param ChunkSize Size of the command data chunk.
 * @param Offset    Byte offset to write the chunk.
 * 
 * @return CFE_SUCCESS: Success.
 *         ERR_ANONYMOUS_ID: EntryId and GroupId pair is the anonymous pair.
 *         ERR_ENTRY_NOT_FOUND: No reserved entry matches the EntryId and GroupId pair.
 *         ERR_WRITE_OUT_OF_BOUNDS: The write operation would go out of bounds of the command buffer. 
 */
CFE_Status_t TTC_Plumb_TimelineAddEntryWrite(uint16 EntryId,
                                             uint16 GroupId,
                                             const uint8* CmdChunk,
                                             uint16 ChunkSize,
                                             uint16 Offset);

/**
 * @brief Plumbing cmd add interface 3 of 3: finalize a reserved entry and commit it to the timeline.
 *        See #TTC_TimelineAddEntry for the meaning of the parameters.
 *        If the TimeTagType is relative, the absolute TimeTag will be from the time of this call
 *        (not the time of the init or write steps).
 * 
 * @param TimeTagType   TTC_TIMETAG_TYPE_ABSOLUTE or TTC_TIMETAG_TYPE_RELATIVE.
 * @param EntryId       Target entry ID at the reserved status.
 * @param GroupId       Target group ID at the reserved status.
 * @param TimeTag       Time tag in seconds.
 * @param StalenessThreshold Maximum allowed staleness for the entry.
 * @param ExecutionType Execution type for the entry.
 * @param CmdSize       Size of the command data.
 *
 * @return CFE_SUCCESS: Success.
 *         ERR_ANONYMOUS_ID: EntryId and GroupId pair is the anonymous pair.
 *         ERR_ENTRY_NOT_FOUND: No reserved entry matches the EntryId and GroupId pair.
 *         ERR_CMD_SIZE_MISMATCH: CmdSize does not match the CCSDS packet's size field.
 *         ERR_CMD_SIZE_TOO_LARGE: CmdSize exceeds TTC_PLATFORM_MAX_COMMAND_SIZE.
 *         ERR_INSERT_SORTED_INDEX: Insertion into the timeline failed.
 */
CFE_Status_t TTC_Plumb_TimelineAddEntryFinalize(uint8  TimeTagType,
                                                uint16 EntryId,
                                                uint16 GroupId,
                                                uint32 TimeTag,
                                                uint16 StalenessThreshold,
                                                uint8  ExecutionType,
                                                uint16 CmdSize);

/**
 * @brief Delete an entry reserved by TTC_Plumb_TimelineAddEntryInit() from the timeline.
 * 
 * @param EntryId   Target entry ID at the reserved status.
 * @param GroupId   Target group ID at the reserved status.
 * @return CFE_SUCCESS: Success.
 *         ERR_ENTRY_NOT_FOUND: No reserved entry matches the EntryId and GroupId pair.
 *         ERR_ANONYMOUS_ID: EntryId and GroupId pair is the anonymous pair
 */
CFE_Status_t TTC_Plumb_TimelineDeleteReservedEntry(uint16 EntryId, uint16 GroupId);

/**
 * @brief Plumbing interface: purge the entire timeline structure.
 * 
 *        This removes all pending and reserved entries, except for the execution pause state.
 * 
 * @return Always returns CFE_SUCCESS.
 */
CFE_Status_t TTC_Plumb_PurgeTimeline(void);


/**
 * @brief Run a single loop of the timeline processing engine. Must be called at 1Hz rate.
 */
void TTC_ProcessTimeline(void);


/**
 * @brief Debug function to print the current timeline table.
 */
void TTC_Debug_PrintTimeline(void);

#endif
