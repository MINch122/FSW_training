#include "sp_task.h"
#include "sp_dispatch.h"
#include "sp_cmds.h"
#include "sp_eventids.h"
#include "sp_msgids.h"
#include "sp_msg.h"
#include "sp_fcncodes.h"

bool SP_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength){
    bool result = true;
    size_t ActualLength = 0;
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    if (ExpectedLength != ActualLength){
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(SP_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
        "Invalid Msg length : ID = 0x%X, CC = %u, Len = %u, Expected = %u",
        (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength, 
        (unsigned int)ExpectedLength);

        result = false;

        SP_AppData.ErrCounter++;
    }

    return result;
}

void SP_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr){
    CFE_MSG_FcnCode_t CommandCode = 0;
    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    switch (CommandCode)
    {
        case SP_NOOP_CC :
            if (SP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SP_NoopCmd_t))){
                SP_NoopCmd((const SP_NoopCmd_t*)SBBufPtr);
            }
            break;
        
        case SP_RESET_COUNTERS_CC :
            if (SP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SP_ResetCountersCmd_t))){
                SP_ResetCounterCmd((const SP_ResetCountersCmd_t*)SBBufPtr);
            }
            break;
        
        case SP_DEPLOY_CC :
            if (SP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SP_DeployCmd_t))){
                SP_DeployCmd((const SP_DeployCmd_t*)SBBufPtr);
            }
            break;
        
        case SP_GET_DEPLOY_CC :
            if (SP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SP_Get_DeployCmd_t))){
                SP_Get_DeployCmd((const SP_Get_DeployCmd_t*)SBBufPtr);
            }
            break;
        
        default : 
            CFE_EVS_SendEvent(SP_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalud ground command code : CC = %d", CommandCode);
            break;
    }
}

void SP_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr){
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch(CFE_SB_MsgIdToValue(MsgId)){
        case (SP_CMD_MID) :
            SP_ProcessGroundCommand(SBBufPtr);
            break;

        case SP_SEND_BCN_MID:
        
            break;
            
        default :
            CFE_EVS_SendEvent(SP_MID_ERR_EID, CFE_EVS_EventType_ERROR, "SP : invalid command packet, MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
    }
}