#include "sch_lab_cmds.h"

#include "sch_lab_app.h"
#include "sch_lab_eventids.h"

void SCH_LAB_NoopCmd(const SCH_LAB_NoopCmd_t *Cmd)
{
    (void)Cmd;

    SCH_LAB_Global.CmdCounter++;

    CFE_EVS_SendEvent(SCH_LAB_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "SCH_LAB: NOOP received. CmdCnt=%u ErrCnt=%u", SCH_LAB_Global.CmdCounter,
                      SCH_LAB_Global.ErrCounter);
}

void SCH_LAB_ResetCountersCmd(const SCH_LAB_ResetCountersCmd_t *Cmd)
{
    (void)Cmd;

    SCH_LAB_Global.CmdCounter = 0;
    SCH_LAB_Global.ErrCounter = 0;

    CFE_EVS_SendEvent(SCH_LAB_RESET_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "SCH_LAB: Counters reset");
}

void SCH_LAB_SetEntryStateCmd(const SCH_LAB_SetEntryStateCmd_t *Cmd)
{
    uint16            EntryIndex      = Cmd->EntryIndex;
    bool              EnableRequested = (Cmd->Enabled != 0);
    SCH_LAB_StateEntry_t *Entry;
    CFE_SB_MsgId_t    MsgId = CFE_SB_INVALID_MSG_ID;

    if (EntryIndex >= SCH_LAB_MAX_SCHEDULE_ENTRIES)
    {
        SCH_LAB_Global.ErrCounter++;
        CFE_EVS_SendEvent(SCH_LAB_ENTRY_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SCH_LAB: Invalid entry index %u (max %u)", EntryIndex,
                          SCH_LAB_MAX_SCHEDULE_ENTRIES - 1);
        return;
    }

    Entry = &SCH_LAB_Global.State[EntryIndex];

    if (EnableRequested && Entry->PacketRate == 0)
    {
        SCH_LAB_Global.ErrCounter++;
        CFE_EVS_SendEvent(SCH_LAB_ENTRY_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SCH_LAB: Entry %u has no configured schedule", EntryIndex);
        return;
    }

    Entry->Enabled = EnableRequested;
    Entry->Counter = 0;

    SCH_LAB_Global.CmdCounter++;

    CFE_MSG_GetMsgId(CFE_MSG_PTR(Entry->CommandHeader), &MsgId);
    CFE_EVS_SendEvent(SCH_LAB_TOGGLE_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "SCH_LAB: Entry %u %s (MID=0x%04X, Rate=%lu)", EntryIndex,
                      EnableRequested ? "enabled" : "disabled",
                      (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned long)Entry->PacketRate);
}
