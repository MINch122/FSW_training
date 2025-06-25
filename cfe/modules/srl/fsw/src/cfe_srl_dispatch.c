#include "cfe_srl_module_all.h"



/*----------------------------------------------------------------
 *
 * Internal helper routine only, not part of API.
 *
 * Verifies the length of incoming SRL command packets, returns true if acceptable
 *
 *-----------------------------------------------------------------*/
bool CFE_SRL_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength) {
    bool result = true;
    CFE_MSG_Size_t ActualLength = 0;
    CFE_MSG_FcnCode_t FcnCode = 0;
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    /**
     * Verify the command packet length
     */
    if (ExpectedLength != ActualLength) {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(CFE_SRL_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                        "Invalid msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                        (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                        (unsigned int)ExpectedLength);
        result = false;
        CFE_SRL_TaskData.HKTlmMsg.Payload.CommandErrorCounter++;
    }

    return result;
}


/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
void CFE_SRL_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr) {
    CFE_SB_MsgId_t MessageID = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MessageID);
    switch (CFE_SB_MsgIdToValue(MessageID))
    {
    /*
    ** Housekeeping telemetry request
    */
    case CFE_SRL_SEND_HK_MID:
        CFE_SRL_SendHkCmd((const CFE_SRL_SendHkCmd_t *)SBBufPtr);
        break;
    
    case CFE_SRL_CMD_MID:
        CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);
        switch (CommandCode)
        {
        case CFE_SRL_NOOP_CC:
            if (CFE_SRL_VerifyCmdLength(&SBBufPtr->Msg, sizeof(CFE_SRL_NoopCmd_t))) {
                CFE_SRL_NoopCmd((const CFE_SRL_NoopCmd_t *)SBBufPtr);
            }
            break;

        case CFE_SRL_RESET_COUNTERS_CC:
            if (CFE_SRL_VerifyCmdLength(&SBBufPtr->Msg, sizeof(CFE_SRL_ResetCounterCmd_t))) {
                CFE_SRL_ResetCounterCmd((const CFE_SRL_ResetCounterCmd_t *)SBBufPtr);
            }
            break;

        case CFE_SRL_GET_HANDLE_STATUS_CC:
            if (CFE_SRL_VerifyCmdLength(&SBBufPtr->Msg, sizeof(CFE_SRL_GetHandleStatusCmd_t))) {
                CFE_SRL_GetHandleStatusCmd((const CFE_SRL_GetHandleStatusCmd_t *)SBBufPtr);
            }
            break;
        
        case CFE_SRL_INIT_HANDLE_CC:
            if (CFE_SRL_VerifyCmdLength(&SBBufPtr->Msg, sizeof(CFE_SRL_InitHandleCmd_t))) {
                CFE_SRL_InitHandleCmd((const CFE_SRL_InitHandleCmd_t *)SBBufPtr);
            }
            break;

        case CFE_SRL_CLOSE_HANDLE_CC:
            if (CFE_SRL_VerifyCmdLength(&SBBufPtr->Msg, sizeof(CFE_SRL_CloseHandleCmd_t))) {
                CFE_SRL_CloseHandleCmd((const CFE_SRL_CloseHandleCmd_t *)SBBufPtr);
            }
            break;
        
        default:
            CFE_EVS_SendEvent(CFE_SRL_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                                "%s: Invalid CC, Unexpected Command Code %u", __func__, CommandCode);
            CFE_SRL_TaskData.HKTlmMsg.Payload.CommandErrorCounter++;
            break;
        }
    default:
        CFE_EVS_SendEvent(CFE_SRL_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                            "%s: Invalid MID, Unexpected Msg ID: 0x%X", __func__, 
                            (unsigned int)CFE_SB_MsgIdToValue(MessageID));
        CFE_SRL_TaskData.HKTlmMsg.Payload.CommandErrorCounter++;
        break;
    }
}