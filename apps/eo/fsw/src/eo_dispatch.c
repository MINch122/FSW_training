/**
 * Include files
 */
#include "eo_task.h"
#include "eo_dispatch.h"
#include "eo_cmd.h"
#include "../inc/eo_eventids.h"
#include "eo_msgids.h"
#include "eo_msg.h"
#include "cfe_msgids.h"
#include "eo_utils.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool EO_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength) {
    bool Result = true;
    size_t ActualLength = 0;
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    /**
     * Verify the command packet length.
     */
    if (ExpectedLength != ActualLength) {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(EO_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        Result = false;

        EO_Data.ErrCounter ++;
    }

    return Result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* EO ground commands                                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void EO_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr) {
    CFE_MSG_FcnCode_t CC = 0xFF;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CC);

    /**
     * Process ground command
     */
    switch (CC)
    {
    case EO_NOOP_CC:
        if (EO_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EO_NoopCmd_t))) {
            EO_NoopCmd((const EO_NoopCmd_t *)SBBufPtr);
        }
        break;
    
    case EO_RESET_COUNTER_CC:
        if (EO_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EO_ResetCounterCmd_t))) {
            EO_ResetCounterCmd((const EO_ResetCounterCmd_t *)SBBufPtr);
        }
        break;

    case EO_RESET_PHASE_CC:
        if (EO_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EO_ResetPhaseCmd_t))) {
            EO_ResetPhaseCmd((const EO_ResetPhaseCmd_t *)SBBufPtr);
        }
        break;
    
    case EO_NEXT_PHASE_CC:
        if (EO_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EO_NextPhaseCmd_t))) {
            EO_NextPhaseCmd((const EO_NextPhaseCmd_t *)SBBufPtr);
        }
        break;

    case EO_FINISH_PHASE_CC:
        if (EO_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EO_FinishPhaseCmd_t))) {
            EO_FinishPhaseCmd((const EO_FinishPhaseCmd_t *)SBBufPtr);
        }
        break;

    case EO_EXIT_CHILD_TASK_CC:
        if (EO_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EO_ExitChildTaskCmd_t))) {
            EO_ExitChildTaskCmd((const EO_ExitChildTaskCmd_t *)SBBufPtr);
        }
        break;
    
    case EO_START_CHILD_TASK_CC:
        if (EO_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EO_StartChildTaskCmd_t))) {
            EO_StartChildTaskCmd((const EO_StartChildTaskCmd_t *)SBBufPtr);
        }
        break;

    case EO_APPS_PERM_OFF_CC:
        if (EO_VerifyCmdLength(&SBBufPtr->Msg, sizeof(EO_AppsPermOffCmd_t))) {
            EO_AppsPermOffCmd((const EO_AppsPermOffCmd_t *)SBBufPtr);
        }
        break;
        
    default:
        EO_Data.ErrCounter ++;    
        CFE_EVS_SendEvent(EO_CC_ERR_EID, CFE_EVS_EventType_ERROR, "EO: Invalid command code. CC = %d", CC);    
        break;
    }
}


void EO_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr) {
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId)) {
        case EO_CMD_MID:
            EO_ProcessGroundCommand(SBBufPtr);
            break;
        
        case EO_SEND_BCN_MID:
            EO_SendBeaconCmd();
            break;

        /* Other H/W Apps Msg */
        case EPS_VI_TLM_MID:
            /* Update Vbatt */
            EO_UpdateDataEPS((const EPS_Vi_Tlm_t *)SBBufPtr);
            break;
        
        case EPS_OUT_TLM_MID:
            /* Update PCDU output */
            EO_UpdateOutEPS((const EPS_Output_Tlm_t *)SBBufPtr);

        case SANT_OP_TLM_MID:
            /* Check SANT state */
            EO_UpdateDataSANT((const SANT_OperationTlm_t *)SBBufPtr);
            break;

        // case SP_BCN_TLM_MID:
            /* Check SP state */
            /* Not used. EO directly call GPIO via SRL */
            // break;

        case RPT_OPS_TLM_MID:
            /* Check Spacecraft Operation Data from RPT */
            EO_ValidateOperationData((const RPT_OpsTlm_t *)SBBufPtr);
            break;

        case ADCS_MMT_TLM_MID:
            /* Check MMT deploy data */
            EO_UpdateDataADCS((const ADCS_MMTTlm_t *)SBBufPtr);
            break;
        
        default:
            CFE_EVS_SendEvent(EO_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                                "EO: Invalid Message ID. MID = 0x%X", (uint32_t)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}