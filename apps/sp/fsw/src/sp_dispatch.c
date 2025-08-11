#include "sp_task.h"
#include "sp_dispatch.h"
#include "sp_cmds.h"
#include "sp_eventids.h"
#include "sp_msgids.h"
#include "sp_msg.h"
#include "sp_fcncodes.h"

bool SP_APP_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength){
    bool result = true;
    size_t ActualLength = 0;
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    if (ExpectedLength != ActualLength){
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(SP_APP_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
        "Invalid Msg length : ID = 0x%X, CC = %u, Len = %u, Expected = %u",
        (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength, 
        (unsigned int)ExpectedLength);

        result = false;

        SP_APP_Data.ErrCounter++;
    }

    return result;
}

void SP_APP_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr){
    CFE_MSG_FcnCode_t CommandCode = 0;
    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    switch (CommandCode)
    {
        case SP_APP_NOOP_CC :
            if (SP_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SP_APP_NoopCmd_t))){
                SP_APP_NoopCmd((const SP_APP_NoopCmd_t*)SBBufPtr);
            }
            break;
        
        case SP_APP_RESET_COUNTERS_CC :
            if (SP_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SP_APP_ResetCountersCmd_t))){
                SP_APP_ResetCounterCmd((const SP_APP_ResetCountersCmd_t*)SBBufPtr);
            }
            break;
        
        case SP_APP_DEPLOY_CC :
            if (SP_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SP_APP_DeployCmd_t))){
                SP_APP_DeployCmd((const SP_APP_DeployCmd_t*)SBBufPtr);
            }
            break;
        
        case SP_APP_GET_DEPLOY_CC :
            if (SP_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SP_APP_Get_DeployCmd_t))){
                SP_APP_Get_DeployCmd((const SP_APP_Get_DeployCmd_t*)SBBufPtr);
            }
            break;
        
        default : 
            CFE_EVS_SendEvent(SP_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalud ground command code : CC = %d", CommandCode);
            break;
    }
}

void SP_APP_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr){
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch(CFE_SB_MsgIdToValue(MsgId)){
        case (SP_APP_CMD_MID) :
            SP_APP_ProcessGroundCommand(SBBufPtr);
            break;

        case SP_APP_SEND_BCN_MID:
        
            break;
            
        default :
            CFE_EVS_SendEvent(SP_APP_MID_ERR_EID, CFE_EVS_EventType_ERROR, "SP : invalid command packet, MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
    }
}