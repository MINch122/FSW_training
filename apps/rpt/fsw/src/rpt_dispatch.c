/**
 * Include files
 */
#include "rpt_task.h"
#include "rpt_dispatch.h"
#include "rpt_cmd.h"
#include "../inc/rpt_eventids.h"
#include "rpt_msgids.h"
#include "rpt_msg.h"
#include "cfe_msgids.h"
#include "rpt_utils.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool RPT_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength) {
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

        CFE_EVS_SendEvent(RPT_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        Result = false;

        RPT_Data.ErrCounter ++;
    }

    return Result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* RPT ground commands                                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void RPT_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr) {
    CFE_MSG_FcnCode_t CC = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CC);

    /**
     * Process ground command
     */
    switch (CC)
    {
    case RPT_NOOP_CC:
        if (RPT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(RPT_NoopCmd_t))) {
            RPT_NoopCmd((const RPT_NoopCmd_t *)SBBufPtr);
        }
        break;
    
    case RPT_RESET_COUNTER_CC:
        if (RPT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(RPT_ResetCounterCmd_t))) {
            RPT_ResetCounterCmd((const RPT_ResetCounterCmd_t *)SBBufPtr);
        }
        break;

    case RPT_REPORT_CC:
        if (RPT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(RPT_ReportCmd_t))) {
            RPT_ReportCmd((const RPT_ReportCmd_t *)SBBufPtr);
        }
        break;
    
    case RPT_CLEAR_QUEUE_CC:
        if (RPT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(RPT_ClearQueueCmd_t))) {
            RPT_ClearQueueCmd((const RPT_ClearQueueCmd_t *)SBBufPtr);
        }
        break;

    case RPT_GET_OPS_DATA_CC:
        if (RPT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(RPT_GetOpsDataCmd_t))) {

        }
        break;
        

    default:
        CFE_EVS_SendEvent(RPT_CC_ERR_EID, CFE_EVS_EventType_ERROR, "RPT: Invalid command code. CC = %d", CC);
        break;
    }
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* RPT Update Operation Data                                                  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void RPT_UpdateOperationData(void) {

    CFE_TIME_SysTime_t Time = CFE_TIME_GetTime();

    OS_MutSemTake(RPT_Data.OpsMutexID);

    RPT_Data.OpsData.TimeSec = Time.Seconds;
    RPT_Data.OpsData.TimeSubsec = Time.Subseconds;
    RPT_Data.OpsData.CRC = RPT_CalculateCRC(&RPT_Data.OpsData, sizeof(RPT_OperationData_t) - sizeof(uint32_t));

    RPT_WriteToFile(RPT_Data.OpsDataHandle, &RPT_Data.OpsData, sizeof(RPT_OperationData_t));
    
    RPT_Data.OpsCount ++;

    if (RPT_Data.OpsCount == RPT_OPS_STORE_BACKUP_COUNT) {
        /* Write Back up data to External SD */
        int FD;
        FD = RPT_OpenOpsFile(true);
        RPT_WriteToFile(FD, &RPT_Data.OpsData, sizeof(RPT_OperationData_t));
        RPT_CloseFile(FD);

        RPT_Data.OpsData.Sequence ++;
        RPT_Data.OpsCount = 0;
    }
    
    OS_MutSemGive(RPT_Data.OpsMutexID);
}




void RPT_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr) {
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId)) {
        case RPT_CMD_MID:
            RPT_ProcessGroundCommand(SBBufPtr);
            break;
        
        case RPT_SEND_BCN_MID:
            RPT_SendBeaconCmd();
            break;

        case RPT_SEND_HK_MID:
            // hk function
            // @deprecated
            break;

        case CFE_TIME_ONEHZ_CMD_MID:
            /* Update Operation State */
            RPT_UpdateOperationData();
            break;
            

        default:
            CFE_EVS_SendEvent(RPT_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                                "RPT: Invalid Message ID. MID = 0x%X", (uint32_t)CFE_SB_MsgIdToValue(MsgId));
    }
}