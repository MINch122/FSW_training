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

    RPT_Data.HkTlm.Payload.CmdCounter = RPT_Data.CmdCounter;
    RPT_Data.HkTlm.Payload.CmdErrCounter = RPT_Data.ErrCounter;

    OS_MutSemTake(RPT_Data.ReportMutexID);
    RPT_Data.HkTlm.Payload.ReportQueueCnt = RPT_Data.RptQueue.Count;
    OS_MutSemGive(RPT_Data.ReportMutexID);
    
    OS_MutSemTake(RPT_Data.CritMutexID);
    RPT_Data.HkTlm.Payload.CriticalQueueCnt = RPT_Data.CritQueue.Count;
    OS_MutSemGive(RPT_Data.CritMutexID);

    OS_MutSemTake(RPT_Data.OpsMutexID);
    RPT_Data.HkTlm.Payload.BootCount = RPT_Data.OpsData.BootCount;
    RPT_Data.HkTlm.Payload.TimeSec = RPT_Data.OpsData.TimeSec;
    RPT_Data.HkTlm.Payload.TimeSubsec = RPT_Data.OpsData.TimeSubsec;
    RPT_Data.HkTlm.Payload.Sequence = RPT_Data.OpsData.Sequence;
    OS_MutSemGive(RPT_Data.OpsMutexID);

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(RPT_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(RPT_Data.HkTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

CFE_Status_t RPT_NoopCmd(const RPT_NoopCmd_t *Msg) {

    RPT_Data.CmdCounter ++;

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

    if (Payload.IsCritical) Status = RPT_MultipleCritical(Payload.StartIdx, Payload.TotalNumber);
    else Status = RPT_MultipleReport(Payload.StartIdx, Payload.TotalNumber);
    
    if (Status != CFE_SUCCESS) RPT_Data.ErrCounter ++;

    return CFE_SUCCESS;
}

CFE_Status_t RPT_ClearQueueCmd(const RPT_ClearQueueCmd_t *Msg) {

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