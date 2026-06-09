#include "mission_task.h"
#include "mission_dispatch.h"
#include "mission_cmd.h"
#include "../inc/mission_eventids.h"
#include "mission_msgids.h"
#include "mission_msg.h"
#include "cfe_msgids.h"
#include "mission_utils.h"
#include "utrx_msgids.h"
#include "utrx_msg.h"

bool MISSION_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength) {
    bool Result = true;
    size_t ActualLength = 0;
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    if (ExpectedLength != ActualLength) {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(MISSION_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        Result = false;

        MISSION_Data.ErrCounter ++;
    }

    return Result;
}

static void MISSION_ProcessUtrxHkTlm(const CFE_SB_Buffer_t *SBBufPtr)
{
    const UTRX_HkTlm_t *HkTlm = (const UTRX_HkTlm_t *)SBBufPtr;
    uint32 RxBytes = HkTlm->Payload.TotRxBytes;

    if (MISSION_LEOP_Lock() != CFE_SUCCESS)
    {
        MISSION_Data.ErrCounter++;
        return;
    }

    if (!MISSION_Data.LEOPUtrxRxBytesInitialized)
    {
        if (RxBytes == 0)
        {
            MISSION_APP_printf("MISSION LEOP: ignoring initial UTRX RxBytes=0\n");
            MISSION_LEOP_Unlock();
            return;
        }

        MISSION_Data.LEOPUtrxInitRxBytes = RxBytes;
        MISSION_Data.LEOPUtrxRxData = RxBytes;
        MISSION_Data.LEOPUtrxRxBytesInitialized = true;
        MISSION_Data.LEOPUtrxRxBytesIncreased = false;
        MISSION_APP_printf("MISSION LEOP: initial UTRX RxBytes=%lu\n", (unsigned long)RxBytes);
        MISSION_LEOP_Unlock();
        return;
    }

    if (RxBytes > MISSION_Data.LEOPUtrxRxData)
    {
        MISSION_Data.LEOPUtrxRxData = RxBytes;
        MISSION_Data.LEOPUtrxRxBytesIncreased = (RxBytes > MISSION_Data.LEOPUtrxInitRxBytes);
        MISSION_APP_printf("MISSION LEOP: UTRX RxBytes updated current=%lu initial=%lu increased=%u\n",
                           (unsigned long)RxBytes, (unsigned long)MISSION_Data.LEOPUtrxInitRxBytes,
                           (unsigned int)MISSION_Data.LEOPUtrxRxBytesIncreased);
    }

    MISSION_LEOP_Unlock();
}

void MISSION_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr) {
    CFE_MSG_FcnCode_t CC = 0xFF;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CC);

    switch (CC)
    {
    case MISSION_NOOP_CC:
        if (MISSION_VerifyCmdLength(&SBBufPtr->Msg, sizeof(MISSION_NoopCmd_t))) {
            MISSION_NoopCmd((const MISSION_NoopCmd_t *)SBBufPtr);
        }
        break;
    
    case MISSION_RESET_COUNTER_CC:
        if (MISSION_VerifyCmdLength(&SBBufPtr->Msg, sizeof(MISSION_ResetCounterCmd_t))) {
            MISSION_ResetCounterCmd((const MISSION_ResetCounterCmd_t *)SBBufPtr);
        }
        break;

    case MISSION_APPS_PERM_OFF_CC:
        if (MISSION_VerifyCmdLength(&SBBufPtr->Msg, sizeof(MISSION_AppsPermOffCmd_t))) {
            MISSION_AppsPermOffCmd((const MISSION_AppsPermOffCmd_t *)SBBufPtr);
        }
        break;
        
    default:
        MISSION_Data.ErrCounter ++;    
        CFE_EVS_SendEvent(MISSION_CC_ERR_EID, CFE_EVS_EventType_ERROR, "MISSION: Invalid command code. CC = %d", CC);    
        break;
    }
}
void MISSION_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr) {
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId)) {
        case MISSION_CMD_MID:
            MISSION_ProcessGroundCommand(SBBufPtr);
            break;
        
        case MISSION_SEND_BCN_MID:
            MISSION_SendBeaconCmd();
            break;

        case MISSION_SEND_HK_MID:
            MISSION_SendHKCmd((const MISSION_SendHkCmd_t *)SBBufPtr);
            break;

        case UTRX_HK_TLM_MID:
            MISSION_ProcessUtrxHkTlm(SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(MISSION_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                                "MISSION: Invalid Message ID. MID = 0x%X", (uint32_t)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
