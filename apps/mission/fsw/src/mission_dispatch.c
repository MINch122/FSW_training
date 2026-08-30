#include "mission_task.h"
#include "mission_dispatch.h"
#include "mission_cmd.h"
#include "mission_eventids.h"
#include "mission_msgids.h"
#include "mission_msg.h"

void MISSION_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr) {
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId)) {
        case MISSION_SET_COMPLETE_MID:
            MISSION_SetCompleteCmd((const MISSION_SetCompleteCmd_t *)SBBufPtr);
            break;

        case MISSION_SEND_HK_MID:
            MISSION_SendHKCmd((const MISSION_SendHkCmd_t *)SBBufPtr);
            break;

        case MISSION_READ_LEOP_FILE_MID:
            MISSION_ReadLeopFileCmd((const MISSION_ReadLeopFileCmd_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(MISSION_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                                "MISSION: Invalid Message ID. MID = 0x%X", (uint32_t)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
