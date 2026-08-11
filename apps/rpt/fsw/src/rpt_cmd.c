#include "rpt_task.h"
#include "rpt_cmd.h"
#include "rpt_eventids.h"
#include "rpt_tbl.h"
#include "rpt_utils.h"
#include "rpt_msg.h"


/**
 * @deprecated Not used. RPT only occupy Beacon
 */
CFE_Status_t RPT_SendHKCmd(void) {

    return CFE_SUCCESS;
}

CFE_Status_t RPT_SendBeaconCmd(void) {

    OS_MutSemTake(RPT_Data.OpsMutexID);
    RPT_Data.HkTlm.Payload.ResetCause = RPT_Data.OpsData.ResetCause;
    RPT_Data.HkTlm.Payload.BootCount = RPT_Data.OpsData.BootCount;
    RPT_Data.HkTlm.Payload.Sequence = RPT_Data.OpsData.Sequence;
    OS_MutSemGive(RPT_Data.OpsMutexID);

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(RPT_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(RPT_Data.HkTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

CFE_Status_t RPT_NoopCmd(const RPT_NoopCmd_t *Msg) {

    RPT_Data.CmdCounter ++;

    CFE_EVS_SendEvent(RPT_NOOP_CMD_INF_EID, CFE_EVS_EventType_INFORMATION, "RPT: Noop command Received.");

    return CFE_SUCCESS;
}

CFE_Status_t RPT_ResetCounterCmd(const RPT_ResetCounterCmd_t *Msg) {
    RPT_Data.CmdCounter = 0;
    RPT_Data.ErrCounter = 0;

    return CFE_SUCCESS;
}

CFE_Status_t RPT_ReportCmd(const RPT_ReportCmd_t *Msg) {
    int32 Status;
    RPT_Report_Payload_t Payload = Msg->Payload;

    RPT_Data.CmdCounter ++;

    if (Payload.IsCritical) Status = RPT_MultipleCritical(Payload.StartIdx, Payload.TotalNumber);
    else Status = RPT_MultipleReport(Payload.StartIdx, Payload.TotalNumber);
    
    if (Status != CFE_SUCCESS) RPT_Data.ErrCounter ++;

    return CFE_SUCCESS;
}

CFE_Status_t RPT_ClearQueueCmd(const RPT_ClearQueueCmd_t *Msg) {
    
    RPT_Data.CmdCounter ++;

    if (Msg->Payload.IsCritical) {
        OS_MutSemTake(RPT_Data.CritMutexID);
        memset(&RPT_Data.CritQueue, 0, sizeof(RPT_Data.CritQueue));
        OS_MutSemGive(RPT_Data.CritMutexID);
    }
    else {
        OS_MutSemTake(RPT_Data.ReportMutexID);
        memset(&RPT_Data.RptQueue, 0, sizeof(RPT_Data.RptQueue));
        OS_MutSemGive(RPT_Data.ReportMutexID);
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* RPT Update Operation Data                                                  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t RPT_GetOpsDataCmd(const RPT_GetOpsDataCmd_t *Msg) {
    RPT_Report_t Report = {0,};

    RPT_Data.CmdCounter ++;

    Report.MsgID = RPT_CMD_MID;
    Report.CommandCode = RPT_GET_OPS_DATA_CC;
    Report.ReturnType = RPT_RETTYPE_SUCCESS;
    Report.ReturnCode = CFE_SUCCESS;
    Report.ReturnDataSize = (uint16)sizeof(RPT_Data.OpsData);

    OS_MutSemTake(RPT_Data.OpsMutexID);
    memcpy(Report.ReturnValue, &RPT_Data.OpsData, sizeof(RPT_Data.OpsData));
    OS_MutSemGive(RPT_Data.OpsMutexID);

    if (RPT_Report(&Report, false) != CFE_SUCCESS) {
        RPT_Data.ErrCounter ++;
    }

    return CFE_SUCCESS;
}

void RPT_UpdateOperationData(void) {

    CFE_TIME_SysTime_t Time = CFE_TIME_GetTime();
    RPT_OperationData_t BackupData;
    osal_id_t BackupHandle;
    int32 Status;
    int32 CloseStatus;

    OS_MutSemTake(RPT_Data.OpsMutexID);

    RPT_Data.OpsData.TimeSec = Time.Seconds;
    RPT_Data.OpsData.TimeSubsec = Time.Subseconds;
    RPT_Data.OpsData.CRC = RPT_CalculateCRC(&RPT_Data.OpsData, sizeof(RPT_OperationData_t) - sizeof(uint32_t));

    Status = RPT_WriteToFile(RPT_Data.OpsDataHandle, &RPT_Data.OpsData, sizeof(RPT_OperationData_t));
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(RPT_DATA_WRITE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "RPT operation data write failed, RC=0x%08lX", (unsigned long)Status);
        OS_MutSemGive(RPT_Data.OpsMutexID);
        return;
    }

    if (RPT_Data.OpsCount < RPT_OPS_STORE_BACKUP_COUNT) {
        RPT_Data.OpsCount++;
    }

    if (RPT_Data.OpsCount < RPT_OPS_STORE_BACKUP_COUNT) {
        OS_MutSemGive(RPT_Data.OpsMutexID);
        return;
    }

    BackupData = RPT_Data.OpsData;
    BackupData.Sequence++;
    BackupData.CRC = RPT_CalculateCRC(&BackupData, sizeof(RPT_OperationData_t) - sizeof(uint32_t));

    BackupHandle = RPT_OpenOpsBackupFile(BackupData.Sequence);
    if (BackupHandle == OS_OBJECT_ID_UNDEFINED) {
        CFE_EVS_SendEvent(RPT_DATA_BACKUP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "RPT operation data backup open failed, sequence=%lu",
                          (unsigned long)BackupData.Sequence);
        RPT_Data.OpsCount = 0;
        OS_MutSemGive(RPT_Data.OpsMutexID);
        return;
    }

    Status = RPT_WriteToFile(BackupHandle, &BackupData, sizeof(BackupData));
    CloseStatus = RPT_CloseFile(BackupHandle);
    if (Status != CFE_SUCCESS || CloseStatus != CFE_SUCCESS) {
        CFE_EVS_SendEvent(RPT_DATA_BACKUP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "RPT operation data backup failed, sequence=%lu write=0x%08lX close=0x%08lX",
                          (unsigned long)BackupData.Sequence, (unsigned long)Status,
                          (unsigned long)CloseStatus);
        RPT_Data.OpsCount = 0;
        OS_MutSemGive(RPT_Data.OpsMutexID);
        return;
    }

    RPT_Data.OpsData = BackupData;
    RPT_Data.OpsCount = 0;

    Status = RPT_WriteToFile(RPT_Data.OpsDataHandle, &RPT_Data.OpsData, sizeof(RPT_OperationData_t));
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(RPT_DATA_WRITE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "RPT backup sequence persist failed, sequence=%lu RC=0x%08lX",
                          (unsigned long)RPT_Data.OpsData.Sequence, (unsigned long)Status);
    }
    else {
        CFE_EVS_SendEvent(RPT_DATA_BACKUP_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "RPT operation data backup stored, sequence=%lu",
                          (unsigned long)RPT_Data.OpsData.Sequence);
    }

    OS_MutSemGive(RPT_Data.OpsMutexID);
}
