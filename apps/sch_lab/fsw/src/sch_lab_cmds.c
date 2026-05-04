#include "sch_lab_cmds.h"

#include <string.h>

#include "sch_lab_app.h"
#include "sch_lab_eventids.h"

static SCH_LAB_StateEntry_t *SCH_LAB_GetEntry(uint16 EntryIndex)
{
    if (EntryIndex >= SCH_LAB_MAX_SCHEDULE_ENTRIES)
    {
        SCH_LAB_Global.ErrCounter++;
        CFE_EVS_SendEvent(SCH_LAB_ENTRY_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SCH_LAB: Invalid entry index %u (max %u)", EntryIndex,
                          SCH_LAB_MAX_SCHEDULE_ENTRIES - 1);
        return NULL;
    }

    return &SCH_LAB_Global.State[EntryIndex];
}

static void SCH_LAB_ClearEntry(SCH_LAB_StateEntry_t *Entry)
{
    memset(Entry, 0, sizeof(*Entry));
}

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
    bool                  EnableRequested = (Cmd->Enabled != 0);
    SCH_LAB_StateEntry_t *Entry;
    CFE_SB_MsgId_t        MsgId = CFE_SB_INVALID_MSG_ID;

    Entry = SCH_LAB_GetEntry(Cmd->EntryIndex);
    if (Entry == NULL)
    {
        return;
    }

    if (EnableRequested && Entry->PacketRate == 0)
    {
        SCH_LAB_Global.ErrCounter++;
        CFE_EVS_SendEvent(SCH_LAB_ENTRY_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SCH_LAB: Entry %u has no configured schedule", Cmd->EntryIndex);
        return;
    }

    Entry->Enabled = EnableRequested;
    Entry->Counter = 0;

    SCH_LAB_Global.CmdCounter++;

    CFE_MSG_GetMsgId(CFE_MSG_PTR(Entry->CommandHeader), &MsgId);
    CFE_EVS_SendEvent(SCH_LAB_TOGGLE_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "SCH_LAB: Entry %u %s (MID=0x%04X, Rate=%lu)", Cmd->EntryIndex,
                      EnableRequested ? "enabled" : "disabled",
                      (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned long)Entry->PacketRate);
}

void SCH_LAB_AddEntryCmd(const SCH_LAB_AddEntryCmd_t *Cmd)
{
    SCH_LAB_StateEntry_t *Entry;
    CFE_SB_MsgId_t        MsgId;
    CFE_Status_t          Status;
    size_t                MessageSize;
    size_t                MaxPayloadLength;

    Entry = SCH_LAB_GetEntry(Cmd->EntryIndex);
    if (Entry == NULL)
    {
        return;
    }

    MsgId = CFE_SB_ValueToMsgId(Cmd->MessageID);
    if (!CFE_SB_IsValidMsgId(MsgId))
    {
        SCH_LAB_Global.ErrCounter++;
        CFE_EVS_SendEvent(SCH_LAB_ENTRY_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SCH_LAB: Entry %u invalid MID=0x%04lX", Cmd->EntryIndex,
                          (unsigned long)Cmd->MessageID);
        return;
    }

    if (Cmd->PacketRate == 0)
    {
        SCH_LAB_Global.ErrCounter++;
        CFE_EVS_SendEvent(SCH_LAB_ENTRY_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SCH_LAB: Entry %u invalid rate 0", Cmd->EntryIndex);
        return;
    }

    MaxPayloadLength = sizeof(Entry->MessageBuffer);
    if (Cmd->PayloadLength > MaxPayloadLength)
    {
        SCH_LAB_Global.ErrCounter++;
        CFE_EVS_SendEvent(SCH_LAB_ENTRY_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SCH_LAB: Entry %u payload length %u exceeds max %u", Cmd->EntryIndex,
                          Cmd->PayloadLength, (unsigned int)MaxPayloadLength);
        return;
    }

    SCH_LAB_ClearEntry(Entry);

    MessageSize = sizeof(Entry->CommandHeader) + Cmd->PayloadLength;
    Status = CFE_MSG_Init(CFE_MSG_PTR(Entry->CommandHeader), MsgId, MessageSize);
    if (Status == CFE_SUCCESS)
    {
        Status = CFE_MSG_SetFcnCode(CFE_MSG_PTR(Entry->CommandHeader), Cmd->FcnCode);
    }

    if (Status != CFE_SUCCESS)
    {
        SCH_LAB_ClearEntry(Entry);
        SCH_LAB_Global.ErrCounter++;
        CFE_EVS_SendEvent(SCH_LAB_ENTRY_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SCH_LAB: Entry %u message init failed MID=0x%04lX status=0x%08lX",
                          Cmd->EntryIndex, (unsigned long)Cmd->MessageID, (unsigned long)Status);
        return;
    }

    Entry->PacketRate    = Cmd->PacketRate;
    Entry->PayloadLength = Cmd->PayloadLength;
    Entry->Enabled       = (Cmd->Enabled != 0);
    Entry->Counter       = 0;
    memcpy(Entry->MessageBuffer, Cmd->MessageBuffer, Cmd->PayloadLength);

    SCH_LAB_Global.CmdCounter++;
    CFE_EVS_SendEvent(SCH_LAB_ADD_ENTRY_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "SCH_LAB: Entry %u added %s MID=0x%04lX CC=%u Rate=%lu PayloadLen=%u",
                      Cmd->EntryIndex, Entry->Enabled ? "enabled" : "disabled",
                      (unsigned long)Cmd->MessageID, (unsigned int)Cmd->FcnCode,
                      (unsigned long)Entry->PacketRate, Entry->PayloadLength);
}

void SCH_LAB_DeleteEntryCmd(const SCH_LAB_DeleteEntryCmd_t *Cmd)
{
    SCH_LAB_StateEntry_t *Entry;
    CFE_SB_MsgId_t        MsgId = CFE_SB_INVALID_MSG_ID;
    uint32                PacketRate;

    Entry = SCH_LAB_GetEntry(Cmd->EntryIndex);
    if (Entry == NULL)
    {
        return;
    }

    PacketRate = Entry->PacketRate;
    CFE_MSG_GetMsgId(CFE_MSG_PTR(Entry->CommandHeader), &MsgId);
    SCH_LAB_ClearEntry(Entry);

    SCH_LAB_Global.CmdCounter++;
    CFE_EVS_SendEvent(SCH_LAB_DELETE_ENTRY_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "SCH_LAB: Entry %u deleted (MID=0x%04X, Rate=%lu)", Cmd->EntryIndex,
                      (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned long)PacketRate);
}
