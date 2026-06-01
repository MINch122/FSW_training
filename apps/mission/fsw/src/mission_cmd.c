#include "mission_task.h"
#include "mission_cmd.h"
#include "mission_eventids.h"
#include "mission_tbl.h"
#include "mission_utils.h"
#include "mission_msg.h"

#include "fm_msgids.h"
#include "fm_msg.h"
#include "fm_msgdefs.h"

static void MISSION_UpdateLeopTlmPayload(MISSION_HkTlm_Payload_t *Payload)
{
    Payload->CmdCounter    = MISSION_Data.CmdCounter;
    Payload->CmdErrCounter = MISSION_Data.ErrCounter;

    if (MISSION_LEOP_Lock() != CFE_SUCCESS)
    {
        return;
    }

    Payload->LeopWaitComplete       = MISSION_Data.LEOPWaitComplete;
    Payload->LeopWaitElapsedSec     = MISSION_Data.LEOPWaitElapsedSec;
    Payload->LeopWaitRemainingSec   = MISSION_Data.LEOPWaitRemainingSec;
    Payload->LeopUartDeployTryCount = MISSION_Data.LEOPUartDeployTryCount;
    Payload->LeopGpioBurnTryCount   = MISSION_Data.LEOPGpioBurnTryCount;
    Payload->LeopState              = (uint8_t)MISSION_Data.LEOPState;
    Payload->LeopUtrxRxBytesInitialized = MISSION_Data.LEOPUtrxRxBytesInitialized;
    Payload->LeopUtrxRxBytesIncreased   = MISSION_Data.LEOPUtrxRxBytesIncreased;
    Payload->LeopUtrxInitRxBytes        = MISSION_Data.LEOPUtrxInitRxBytes;
    Payload->LeopUtrxRxData             = MISSION_Data.LEOPUtrxRxData;

    MISSION_LEOP_Unlock();
}

CFE_Status_t MISSION_SendHKCmd(const MISSION_SendHkCmd_t *Msg) {
    (void)Msg;

    MISSION_UpdateLeopTlmPayload(&MISSION_Data.HkTlm.Payload);

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(MISSION_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(MISSION_Data.HkTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

CFE_Status_t MISSION_SendBeaconCmd(void) {
    MISSION_UpdateLeopTlmPayload(&MISSION_Data.BcnTlm.Payload);

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(MISSION_Data.BcnTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(MISSION_Data.BcnTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

CFE_Status_t MISSION_NoopCmd(const MISSION_NoopCmd_t *Msg) {
    (void)Msg;
    MISSION_Data.CmdCounter ++;
    return CFE_SUCCESS;
}

CFE_Status_t MISSION_ResetCounterCmd(const MISSION_ResetCounterCmd_t *Msg) {
    (void)Msg;
    MISSION_Data.CmdCounter = 0;
    MISSION_Data.ErrCounter = 0;

    return CFE_SUCCESS;
}

CFE_Status_t MISSION_AppsPermOffCmd(const MISSION_AppsPermOffCmd_t *Msg) {
    (void)Msg;
    const char *AppName = "/cf/mission.so";

    FM_DeleteFileCmd_t Cmd;
    CFE_MSG_Init(CFE_MSG_PTR(Cmd.CommandHeader), CFE_SB_ValueToMsgId(FM_CMD_MID), sizeof(Cmd));
    CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), FM_DELETE_FILE_CC);

    memcpy(Cmd.Payload.Filename, AppName, strlen(AppName) + 1);
    CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
    OS_TaskDelay(500);
    
    return CFE_SUCCESS;
}
