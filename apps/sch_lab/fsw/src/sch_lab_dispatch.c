#include "sch_lab_dispatch.h"

#include "sch_lab_app.h"
#include "sch_lab_cmds.h"
#include "sch_lab_eventids.h"

#include "cfe_msgids.h"

bool SCH_LAB_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool              Result       = true;
    size_t            ActualLength = 0;
    CFE_SB_MsgId_t    MsgId        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode      = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    if (ExpectedLength != ActualLength)
    {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(SCH_LAB_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SCH_LAB: Invalid msg length: ID=0x%X CC=%u Len=%u Expected=%u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode,
                          (unsigned int)ActualLength, (unsigned int)ExpectedLength);

        SCH_LAB_Global.ErrCounter++;
        Result = false;
    }

    return Result;
}

void SCH_LAB_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    switch (CommandCode)
    {
        case SCH_LAB_NOOP_CC:
            if (SCH_LAB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SCH_LAB_NoopCmd_t)))
            {
                SCH_LAB_NoopCmd((const SCH_LAB_NoopCmd_t *)SBBufPtr);
            }
            break;

        case SCH_LAB_RESET_COUNTERS_CC:
            if (SCH_LAB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SCH_LAB_ResetCountersCmd_t)))
            {
                SCH_LAB_ResetCountersCmd((const SCH_LAB_ResetCountersCmd_t *)SBBufPtr);
            }
            break;

        case SCH_LAB_SET_ENTRY_STATE_CC:
            if (SCH_LAB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SCH_LAB_SetEntryStateCmd_t)))
            {
                SCH_LAB_SetEntryStateCmd((const SCH_LAB_SetEntryStateCmd_t *)SBBufPtr);
            }
            break;

        default:
            SCH_LAB_Global.ErrCounter++;
            CFE_EVS_SendEvent(SCH_LAB_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SCH_LAB: Invalid command code CC=%u", (unsigned int)CommandCode);
            break;
    }
}

bool SCH_LAB_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case SCH_LAB_CMD_MID:
            SCH_LAB_ProcessGroundCommand(SBBufPtr);
            return false;

        case CFE_TIME_ONEHZ_CMD_MID:
            return true;

        default:
            SCH_LAB_Global.ErrCounter++;
            CFE_EVS_SendEvent(SCH_LAB_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SCH_LAB: Invalid Message ID MID=0x%X",
                              (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            return false;
    }
}
