/**
 * @file
 *   Report (RPT) library
 */
#include "rpt_task.h"
#include "common_types.h"
#include "rpt_mission_cfg.h"
#include "rpt_msgids.h"
#include "rpt_utils.h"
#include "rpt_mission_cfg.h"
#include "rpt_eventids.h"
#include "cfe_msgids.h"

#include <unistd.h>
#include <fcntl.h>


void RPT_Subscribe(void) {
    CFE_Status_t Status;

    /* Subscription */
    for (uint8_t i = 0; i < RPT_MAX_TBL_ENTRY; i ++) {
        if (RPT_Data.SubsTblPtr->UsedState == RPT_DISABLED) {RPT_Data.SubsTblPtr ++; continue;}

        if (CFE_SB_IsValidMsgId(RPT_Data.SubsTblPtr->Entry.MessageID) == false) {RPT_Data.SubsTblPtr ++; continue;}

        /**
         * Subscribe each entry
         */
        if (RPT_Data.SubsTblPtr->Entry.IsCritical) {
            Status = CFE_SB_SubscribeEx(RPT_Data.SubsTblPtr->Entry.MessageID, 
                                        RPT_Data.CritPipe, CFE_SB_DEFAULT_QOS, RPT_CRITICAL_MSG_DEPTH);
        }
        else {
            Status = CFE_SB_SubscribeEx(RPT_Data.SubsTblPtr->Entry.MessageID,
                                        RPT_Data.RptPipe, CFE_SB_DEFAULT_QOS, RPT_REPORT_MSG_DEPTH);
        }

        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(RPT_REPORT_SUB_ERR_EID, CFE_EVS_EventType_ERROR,
                                "L%d RPT Can't subscribe to stream 0x%X status %i", __LINE__,
                                (unsigned int)CFE_SB_MsgIdToValue(RPT_Data.SubsTblPtr->Entry.MessageID), (int)Status);
        }
        RPT_Data.SubsTblPtr ++;
    }
}


void RPT_Enqueue(const RPT_Report_t *Report, bool IsCritical) {

    if (IsCritical) {
        OS_MutSemTake(RPT_Data.CritMutexID);
        OS_MutSemTake(RPT_Data.OpsMutexID);

        RPT_Data.CritQueue.Entry[RPT_Data.CritQueue.Head].Report = *Report;
        RPT_Data.CritQueue.Head = (RPT_Data.CritQueue.Head + 1) % RPT_CRITICAL_QUEUE_LEN;
        
        /**
         * Append Time info - Only for critical
         */
        RPT_Data.CritQueue.Entry[RPT_Data.CritQueue.Head].Time.Seconds = RPT_Data.OpsData.TimeSec;
        RPT_Data.CritQueue.Entry[RPT_Data.CritQueue.Head].Time.Subseconds = RPT_Data.OpsData.TimeSubsec;

        if (RPT_Data.CritQueue.Count < RPT_CRITICAL_QUEUE_LEN) RPT_Data.CritQueue.Count ++;
        OS_printf("Critical Q Head: %u || Count: %u\n", RPT_Data.CritQueue.Head, RPT_Data.CritQueue.Count);

        OS_MutSemGive(RPT_Data.CritMutexID);
        OS_MutSemGive(RPT_Data.OpsMutexID);

        /**
         * Update: Write data to file
         */
        OS_MutSemTake(RPT_Data.CritMutexID);
        RPT_Data.CritQueue.CRC = RPT_CalculateCRC(&RPT_Data.CritQueue, (sizeof(RPT_CriticalQueue_t) - sizeof(uint32_t)));
        RPT_WriteToFile(RPT_Data.CritDataHandle, &RPT_Data.CritQueue, sizeof(RPT_CriticalQueue_t));
        OS_MutSemGive(RPT_Data.CritMutexID);

    }
    else {
        OS_MutSemTake(RPT_Data.ReportMutexID);

        RPT_Data.RptQueue.Entry[RPT_Data.RptQueue.Head] = *Report;
        RPT_Data.RptQueue.Head = (RPT_Data.RptQueue.Head +1) % RPT_REPORT_QUEUE_LEN;
        if (RPT_Data.RptQueue.Count < RPT_REPORT_QUEUE_LEN) RPT_Data.RptQueue.Count ++;
        OS_printf("Report Q Head: %u || Count:%u\n", RPT_Data.RptQueue.Head, RPT_Data.RptQueue.Count);
        
        OS_MutSemGive(RPT_Data.ReportMutexID);
    }
    CFE_ES_WriteToSysLog("%s: Report Enqueued. MID: 0x%04X || CC: %u\n", __func__, Report->MsgID, Report->CommandCode);

}

int32 RPT_Report(const RPT_Report_t *Report, bool IsCritical) {
    /**
     * Invoked by SB Msg
     * 1. Get report.
     * 2. Msg init with `RPT_REPORT_TLM_MID` or `RPT_CRITICAL_TLM_MID`
     * 3. Publish Msg
     * 4. TO will ingest this.
     */
    int32 Status;

    if (IsCritical) {
        RPT_CriticalTlm_t RPT_Tlm = {0,};
        Status = CFE_MSG_Init(CFE_MSG_PTR(RPT_Tlm.TelemetryHeader), CFE_SB_ValueToMsgId(RPT_CRITICAL_TLM_MID), sizeof(RPT_CriticalTlm_t));
        if (Status == CFE_SUCCESS) {
            RPT_Tlm.Payload.Report = *Report;
            RPT_Tlm.Payload.Time.Seconds = RPT_Data.OpsData.TimeSec;
            RPT_Tlm.Payload.Time.Seconds = RPT_Data.OpsData.TimeSubsec;

            CFE_SB_TimeStampMsg(CFE_MSG_PTR(RPT_Tlm.TelemetryHeader));
            Status = CFE_SB_TransmitMsg(CFE_MSG_PTR(RPT_Tlm.TelemetryHeader), false);
        }
    }
    else {
        RPT_ReportTlm_t RPT_Tlm = {0,};
        Status = CFE_MSG_Init(CFE_MSG_PTR(RPT_Tlm.TelemetryHeader), CFE_SB_ValueToMsgId(RPT_REPORT_TLM_MID), sizeof(RPT_ReportTlm_t));
        if (Status == CFE_SUCCESS) {
            RPT_Tlm.Payload = *Report;
            CFE_SB_TimeStampMsg(CFE_MSG_PTR(RPT_Tlm.TelemetryHeader));
            Status = CFE_SB_TransmitMsg(CFE_MSG_PTR(RPT_Tlm.TelemetryHeader), false);
        }
    }
    
    RPT_Enqueue(Report, IsCritical);

    return Status;
}

int32 RPT_MultipleReport(uint8_t StartIdx, uint8_t TotNum) {
    /**
     * Invoked by GS command
     */
    int32 Status;
    uint8_t Start;
    RPT_Report_t *Entry = RPT_Data.RptQueue.Entry;
    size_t TlmSize = sizeof(RPT_MultipleReportTlm_t) + TotNum * sizeof(RPT_Report_t);
    CFE_SB_Buffer_t *BufPtr;

    OS_MutSemTake(RPT_Data.ReportMutexID);

    /**
     * Parameter validation
     */
    if (TotNum == 0) {
        Status = CFE_STATUS_VALIDATION_FAILURE;
        goto cleanup;
    }

    if (StartIdx > RPT_Data.RptQueue.Count) {
        OS_printf("Invalid StartIdx.\n");
        StartIdx = 0;
    }
    if (TotNum > RPT_Data.RptQueue.Count) {
        OS_printf("Invalid TotNum.\n");
        TotNum = RPT_Data.RptQueue.Count;
        StartIdx = 0;
    }
    if (StartIdx + TotNum > RPT_Data.RptQueue.Count) {
        OS_printf("Invalid Parameters.\n");
        TotNum = RPT_Data.RptQueue.Count - StartIdx;
    }
    
    BufPtr = CFE_SB_AllocateMessageBuffer(TlmSize);
    RPT_MultipleReportTlm_t *RPT_MultipleTlm = (RPT_MultipleReportTlm_t *)BufPtr;
    if (RPT_MultipleTlm == NULL) {
        Status = -1; // Revise Later
        goto cleanup;
    }

    Status = CFE_MSG_Init(CFE_MSG_PTR(RPT_MultipleTlm->TelemetryHeader), CFE_SB_ValueToMsgId(RPT_REPORT_TLM_MID), TlmSize);
    if (Status != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer(BufPtr);
        goto cleanup;
    }

    /* Start from oldest one */
    uint8_t OldIdx = (RPT_Data.RptQueue.Head + RPT_REPORT_QUEUE_LEN - RPT_Data.RptQueue.Count) % RPT_REPORT_QUEUE_LEN;

    Start = (OldIdx + StartIdx) % RPT_REPORT_QUEUE_LEN;
    for (uint8_t i = 0; i < TotNum; i++) {
        RPT_MultipleTlm->Payload[i] = Entry[(Start + i) % RPT_REPORT_QUEUE_LEN];
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(RPT_MultipleTlm->TelemetryHeader));
    Status = CFE_SB_TransmitBuffer(BufPtr, true);
    if (Status != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer(BufPtr);
    }

cleanup:
    OS_MutSemGive(RPT_Data.ReportMutexID);

    return Status;
}

int32 RPT_MultipleCritical(uint8_t StartIdx, uint8_t TotNum) {
    /**
     * Invoked by GS command
     */
    int32 Status;
    uint8_t Start;
    RPT_Critical_t *Entry = RPT_Data.CritQueue.Entry;
    size_t TlmSize = sizeof(RPT_MultipleCriticalTlm_t) + TotNum * sizeof(RPT_Critical_t);
    CFE_SB_Buffer_t *BufPtr;

    OS_MutSemTake(RPT_Data.CritMutexID);

    /**
     * Parameter validation
     */
    if (TotNum == 0) {
        Status = CFE_STATUS_VALIDATION_FAILURE;
        goto cleanup;
    }

    if (StartIdx > RPT_Data.CritQueue.Count) StartIdx = 0;
    if (TotNum > RPT_Data.CritQueue.Count) {
        TotNum = RPT_Data.CritQueue.Count;
        StartIdx = 0;
    }
    if (StartIdx + TotNum > RPT_Data.CritQueue.Count) TotNum = RPT_Data.CritQueue.Count - StartIdx;
    
    BufPtr = CFE_SB_AllocateMessageBuffer(TlmSize);
    RPT_MultipleCriticalTlm_t *RPT_MultipleTlm = (RPT_MultipleCriticalTlm_t *)BufPtr;
    if (RPT_MultipleTlm == NULL) {
        Status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        goto cleanup;
    }

    Status = CFE_MSG_Init(CFE_MSG_PTR(RPT_MultipleTlm->TelemetryHeader), CFE_SB_ValueToMsgId(RPT_CRITICAL_TLM_MID), TlmSize);
    if (Status != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer(BufPtr);
        goto cleanup;
    }

    /* Start from oldest one */
    uint8_t OldIdx = RPT_Data.CritQueue.Head + RPT_CRITICAL_QUEUE_LEN - RPT_Data.CritQueue.Count;

    Start = (OldIdx + StartIdx) % RPT_CRITICAL_QUEUE_LEN;
    for (uint8_t i = 0; i < TotNum; i++) {
        RPT_MultipleTlm->Payload[i] = Entry[(Start + i) % RPT_CRITICAL_QUEUE_LEN];
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(RPT_MultipleTlm->TelemetryHeader));
    Status = CFE_SB_TransmitBuffer(BufPtr, true);
    if (Status != CFE_SUCCESS) {
        CFE_SB_ReleaseMessageBuffer(BufPtr);
    }

cleanup:
    OS_MutSemGive(RPT_Data.CritMutexID);

    return Status;
}

bool RPT_VerifyReportLength(const CFE_MSG_Message_t *MsgPtr) {
    int32 Status;
    bool Result = true;
    size_t ActualSize;

    Status = CFE_MSG_GetSize(MsgPtr, &ActualSize);
    if (Status != CFE_SUCCESS) return false;

    if (ActualSize != sizeof(RPT_ReportTlm_t)) Result = false;

    return Result;
}



/**********************************
 * 
 * Operation Data function
 * 
 **********************************/
osal_id_t RPT_OpenOpsFile(uint8_t IsBackup) {
    osal_id_t FD = OS_OBJECT_ID_UNDEFINED;
    int32 OsStatus;
    char Path[64] = {0,};
    
    if (!IsBackup) strcpy(Path, RPT_OPS_DATA_PATH);
    else sprintf(Path, "%sOps-%032u", 
                RPT_OPS_BACKUP_PATH, 
                RPT_Data.OpsData.Sequence);

    // FD = open(Path, O_CREAT | O_RDWR, 0666);
    OsStatus = OS_OpenCreate(&FD, RPT_OPS_DATA_PATH, OS_FILE_FLAG_CREATE, OS_READ_WRITE);
    OS_printf("RPT Ops FD: %d\n", FD);

    /* If, error occur, return 0  NOTE: `osal_id_t` is unsigned */ 
    if (OsStatus != OS_SUCCESS) return OS_OBJECT_ID_UNDEFINED;

    return FD;
}


/**
 * Data is guaranteed to `RPT_OperationData_t`
 * Size is guaranteed to `sizeof(RPT_OperationData_t)`
 */
int32 RPT_WriteToFile(osal_id_t FD, const void *Data, size_t Size) {

    int32 OsStatus;
    
    OsStatus = OS_lseek(FD, 0, OS_SEEK_SET);
    if (OsStatus < OS_SUCCESS) return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;

    OsStatus = OS_write(FD, Data, Size);
    if (OsStatus != Size) return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;

    return CFE_SUCCESS;
}

int32 RPT_ReadFromFile(osal_id_t FD, void *Data, size_t Size) {
    int32 OsStatus;

    OsStatus = OS_read(FD, Data, Size);
    if (OsStatus < OS_SUCCESS) return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    
    /* Size of readed data */
    return OsStatus;
}

int32 RPT_CloseFile(osal_id_t FD) {
    int32 OsStatus;

    OsStatus = OS_close(FD);
    if (OsStatus != OS_SUCCESS) return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;

    return CFE_SUCCESS;
}


/**********************************
 * 
 * Critical Queue function
 * 
 **********************************/
osal_id_t RPT_OpenCriticalFile(void) {
    osal_id_t FD = OS_OBJECT_ID_UNDEFINED;
    int32 OsStatus;
    
    OsStatus = OS_OpenCreate(&FD, RPT_CRITICAL_DATA_PATH, OS_FILE_FLAG_CREATE, OS_READ_WRITE);
    OS_printf("RPT Critical FD: %d\n", FD);
    
    /* If, error occur, return 0  NOTE: `osal_id_t` is unsigned */ 
    if (OsStatus != OS_SUCCESS) return OS_OBJECT_ID_UNDEFINED;
    
    return FD;
}


/**********************************
 * 
 * Other Util function
 * 
 **********************************/
uint32 RPT_CalculateCRC(const void *Data, size_t Size) {
    return CFE_ES_CalculateCRC(Data, Size, 0, CFE_MISSION_ES_DEFAULT_CRC);
}

/****************************************************************
 * Calculate Reset Cause
 * Two value combined in bit operation manner.
 * Look up Lower diagram
 * 
 *   7  6  5  4  3  2  1  0
 *  +----------------------+
 *  |  R  |Type|  SubType  |
 *  +----------------------+
 * R - are reserved bits. Always `00`
 * Type - Must be `01` or `10`
 * SubType - Must be `0001` ~ `1001`
 * For more specification, refer `cfe_psp_watchdog_api.h`
 ****************************************************************/
uint8 RPT_CalculateResetCause(uint8 ResetType, uint8 ResetSubType) {
    uint8 TempVal = 0;

    TempVal = ((ResetType & 3u) << 4) | (ResetSubType & 15u);

    return TempVal;
}