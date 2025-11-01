#include "eo_task.h"
#include "eo_cmd.h"
#include "eo_eventids.h"
#include "eo_tbl.h"
#include "eo_utils.h"
#include "eo_msg.h"

#include "fm_msgids.h"
#include "fm_msg.h"
#include "fm_msgdefs.h"

/**
 * @deprecated Not used. EO only occupy Beacon
 */
CFE_Status_t EO_SendHKCmd(void) {

    return CFE_SUCCESS;
}

CFE_Status_t EO_SendBeaconCmd(void) {
    
    EO_Data.BcnTlm.Payload.CmdCounter = EO_Data.CmdCounter;
    EO_Data.BcnTlm.Payload.CmdErrCounter = EO_Data.ErrCounter;

    OS_MutSemTake(EO_Data.EOMutex);
    EO_Data.BcnTlm.Payload.PhaseInfo = EO_Data.CurrentStep;
    OS_MutSemGive(EO_Data.EOMutex);

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(EO_Data.BcnTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(EO_Data.BcnTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

CFE_Status_t EO_NoopCmd(const EO_NoopCmd_t *Msg) {

    EO_Data.CmdCounter ++;
    uint8_t CurrentPhase;

    OS_MutSemTake(EO_Data.EOMutex);
    CurrentPhase = EO_Data.CurrentStep.CurrentPhase;
    OS_MutSemGive(EO_Data.EOMutex);
    
    if (CurrentPhase == EO_TC_WAIT_PHASE) {
        /* Report the message */
        char WAM[] = "I LOVE MOZART";
        EO_ReportTlm_t Report;
        CFE_MSG_Init(CFE_MSG_PTR(Report.TelemetryHeader), CFE_SB_ValueToMsgId(EO_REPORT_TLM_MID), sizeof(Report));

        Report.Payload.MsgID = EO_CMD_MID;
        Report.Payload.CommandCode = 0;
        Report.Payload.ReturnType = RPT_RETTYPE_SUCCESS;
        Report.Payload.ReturnCode = CFE_SUCCESS;
        Report.Payload.ReturnDataSize = sizeof(WAM);
        memcpy(Report.Payload.ReturnValue, WAM, sizeof(WAM));

        CFE_SB_TimeStampMsg(CFE_MSG_PTR(Report.TelemetryHeader));
        CFE_SB_TransmitMsg(CFE_MSG_PTR(Report.TelemetryHeader), true);

        EO_Data.CurrentStep.IsTC = true;
        
        EO_PRINTF("%s: I LOVE MOZART\n", __func__);
    }
    
    return CFE_SUCCESS;
}

CFE_Status_t EO_ResetCounterCmd(const EO_ResetCounterCmd_t *Msg) {
    EO_Data.CmdCounter = 0;
    EO_Data.ErrCounter = 0;

    return CFE_SUCCESS;
}

CFE_Status_t EO_ResetPhaseCmd(const EO_ResetPhaseCmd_t *Msg) {
    OS_MutSemTake(EO_Data.EOMutex);
    EO_Data.CurrentStep.CurrentPhase = EO_SANT_DEPLOY_PHASE;
    OS_MutSemGive(EO_Data.EOMutex);

    return CFE_SUCCESS;
}

CFE_Status_t EO_NextPhaseCmd(const EO_NextPhaseCmd_t *Msg) {
    
    OS_MutSemTake(EO_Data.EOMutex);
    if (EO_Data.CurrentStep.CurrentPhase < EO_ATTITUDE_CONTROL_PHASE)
        EO_Data.CurrentStep.CurrentPhase ++;
    OS_MutSemGive(EO_Data.EOMutex);

    return CFE_SUCCESS;
}

CFE_Status_t EO_FinishPhaseCmd(const EO_FinishPhaseCmd_t *Msg) {
    
    OS_MutSemTake(EO_Data.EOMutex);
    EO_Data.CurrentStep.CurrentPhase = EO_DONE;
    OS_MutSemTake(EO_Data.EOMutex);

    return CFE_SUCCESS;
}

CFE_Status_t EO_ExitChildTaskCmd(const EO_ExitChildTaskCmd_t *Msg) {

    CFE_ES_DeleteChildTask(EO_Data.ChildTaskId);

    return CFE_SUCCESS;
}

CFE_Status_t EO_StartChildTaskCmd(const EO_StartChildTaskCmd_t *Msg) {

    CFE_ES_CreateChildTask(&EO_Data.ChildTaskId, EO_CHILD_TASK_NAME, EO_ChildTask, 0, EO_CHILD_TASK_STACK_SIZE,
                                   EO_CHILD_TASK_PRIORITY, 0);

    return CFE_SUCCESS;
}

CFE_Status_t EO_AppsPermOffCmd(const EO_AppsPermOffCmd_t *Msg) {
    /* Delete Apps library file */
    const char *AppName[] = {"/cf/sant.so", "/cf/sp.so", "/cf/eo.so"};

    FM_DeleteFileCmd_t Cmd;
    CFE_MSG_Init(CFE_MSG_PTR(Cmd.CommandHeader), CFE_SB_ValueToMsgId(FM_CMD_MID), sizeof(Cmd));
    CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), FM_DELETE_FILE_CC);
    for (uint8_t i = 0; i < 3; i ++) {
        memcpy(Cmd.Payload.Filename, AppName[i], strlen(AppName[i]) + 1);
        CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
        OS_TaskDelay(500);
    }
    
    return CFE_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* EO Wakeup Task                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void EO_WakeupTask(void) {

    // if(EO_Data.CurrentStep.CurrentPhase == EO_DONE) {
    //     /* If Early Orbit Phase done, Exit several app */
    //     EO_ExitApps();
    //     return;
    // }
    
    // EO_PhaseDispatch();

    return;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* EO Update values                                                           */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void EO_UpdateDataEPS(const EPS_Vi_Tlm_t *Msg) {
    EO_PRINTF("%s: EPS VI arrived.\n", __func__);
    EO_Data.Vbatt = Msg->Vbatt;
    EO_Data.CurIn[0] = Msg->CurIn[0];
    EO_Data.CurIn[1] = Msg->CurIn[1];

    EO_PRINTF("%s: EPS Vbatt Sem Give.\n", __func__);
    OS_BinSemGive(EO_Data.EPS_ViSemId);

}

void EO_UpdateOutEPS(const EPS_Output_Tlm_t *Msg) {
    memcpy(EO_Data.Output, Msg->Output, sizeof(EO_Data.Output));

    EO_PRINTF("%s: EPS Output Sem Give.\n", __func__);
    OS_BinSemGive(EO_Data.EPS_OutSemId);

}

void EO_UpdateDataSANT(const SANT_OperationTlm_t *Msg) {
    EO_Data.State = Msg->Payload.ReleaseStatus.State;
    EO_Data.Status = Msg->Payload.ReleaseStatus.Status;
    EO_Data.BurnTimeLeft = Msg->Payload.ReleaseStatus.BurnTimeLeft;
    EO_Data.BurnTries = Msg->Payload.ReleaseStatus.BurnTries;

    if (EO_Data.WaitingSANT) {
        EO_Data.WaitingSANT = false;
        EO_PRINTF("%s: SANT Sem Give.\n", __func__);
        OS_BinSemGive(EO_Data.SANT_SemId);
    }
}

void EO_ValidateOperationData(const RPT_OpsTlm_t *Msg) {
    /* Check Boot Count */
    uint16_t BootCount = Msg->Payload.BootCount;

    /* Update Epoch */
    EO_Data.Epoch.Seconds = Msg->Payload.EpochSec;
    EO_Data.Epoch.Subseconds = Msg->Payload.EpochSubsec;

    EO_PRINTF("%s: Received Boot Count : %u\n", __func__, BootCount);
    EO_PRINTF("%s: Received Epoch sec : %u\n", __func__, EO_Data.Epoch.Seconds);
    EO_PRINTF("%s: Received Epoch Subsec : %u\n", __func__, EO_Data.Epoch.Subseconds);

    /* If Boot Count is `1`, Start EO sequence after 45 min */
    if (BootCount == 1) {
        /* SC RTS 2 shall handle this */
        EO_EnableRTS2();
        EO_StartRTS2();
    }

    /* If not, Start EO sequence immediately */
    else {
        CFE_ES_CreateChildTask(&EO_Data.ChildTaskId, EO_CHILD_TASK_NAME, EO_ChildTask, 0, EO_CHILD_TASK_STACK_SIZE,
                                   EO_CHILD_TASK_PRIORITY, 0);
    }
}

void EO_UpdateDataADCS(const ADCS_MMTTlm_t *Msg) {
    EO_Data.MagDeployPinState = Msg->Payload.Mag0DeployPinState;
    EO_Data.MagBurnPinState = Msg->Payload.Mag0BurnPinState;
    EO_Data.MagDeployTimeout = Msg->Payload.Mag0DeployTimeout;

    if (EO_Data.WaitingADCS) {
        EO_Data.WaitingADCS = false;
        EO_PRINTF("%s: ADCS Sem Give.\n", __func__);
        OS_BinSemGive(EO_Data.ADCS_SemId);
    }
}