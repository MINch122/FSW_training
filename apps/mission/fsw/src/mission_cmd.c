#include "mission_task.h"
#include "mission_cmd.h"
#include "mission_eventids.h"
#include "mission_tbl.h"
#include "mission_utils.h"
#include "mission_msg.h"

#include "fm_msgids.h"
#include "fm_msg.h"
#include "fm_msgdefs.h"

#include <string.h>

static void MISSION_SendReport(const void *Msg, const void *Data, uint16 DataSize, int32 ReturnCode, uint8 ReturnType)
{
    CFE_SB_MsgId_t   CmdMid;
    CFE_MSG_FcnCode_t CmdCode;
    uint16 CopySize = DataSize > RPT_RET_VALUE_BUF_SIZE ? RPT_RET_VALUE_BUF_SIZE : DataSize;

    CFE_MSG_GetMsgId(Msg, &CmdMid);
    CFE_MSG_GetFcnCode(Msg, &CmdCode);

    CFE_MSG_Init(CFE_MSG_PTR(MISSION_Data.Report.TelemetryHeader), CFE_SB_ValueToMsgId(MISSION_REPORT_TLM_MID),
                 sizeof(MISSION_Data.Report));
    MISSION_Data.Report.Payload.MsgID = (uint16)CFE_SB_MsgIdToValue(CmdMid);
    MISSION_Data.Report.Payload.CommandCode = (uint8)CmdCode;
    MISSION_Data.Report.Payload.ReturnType = ReturnType;
    MISSION_Data.Report.Payload.ReturnCode = ReturnCode;
    MISSION_Data.Report.Payload.ReturnDataSize = CopySize;
    if (Data != NULL && CopySize > 0)
    {
        memcpy(MISSION_Data.Report.Payload.ReturnValue, Data, CopySize);
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(MISSION_Data.Report.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(MISSION_Data.Report.TelemetryHeader), true);
}

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

    MISSION_SendReport(Msg, &MISSION_Data.HkTlm.Payload, sizeof(MISSION_Data.HkTlm.Payload), CFE_SUCCESS, RPT_RETTYPE_SUCCESS);

    return CFE_SUCCESS;
}

CFE_Status_t MISSION_SendBeaconCmd(void) {
    MISSION_UpdateLeopTlmPayload(&MISSION_Data.BcnTlm.Payload);

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(MISSION_Data.BcnTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(MISSION_Data.BcnTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

CFE_Status_t MISSION_NoopCmd(const MISSION_NoopCmd_t *Msg) {
    MISSION_Data.CmdCounter++;

    static const char NoopReport[] = "Yosi In Space";
    MISSION_SendReport(Msg, NoopReport, sizeof(NoopReport), CFE_SUCCESS, RPT_RETTYPE_SUCCESS);

    return CFE_SUCCESS;
}

CFE_Status_t MISSION_ResetCounterCmd(const MISSION_ResetCounterCmd_t *Msg) {
    MISSION_Data.CmdCounter = 0;
    MISSION_Data.ErrCounter = 0;

    uint8 Counters[2] = {MISSION_Data.CmdCounter, MISSION_Data.ErrCounter};
    MISSION_SendReport(Msg, Counters, sizeof(Counters), CFE_SUCCESS, RPT_RETTYPE_SUCCESS);

    return CFE_SUCCESS;
}

CFE_Status_t MISSION_AppsPermOffCmd(const MISSION_AppsPermOffCmd_t *Msg) {
    const char *AppName = "/cf/mission.so";

    FM_DeleteFileCmd_t Cmd;
    CFE_MSG_Init(CFE_MSG_PTR(Cmd.CommandHeader), CFE_SB_ValueToMsgId(FM_CMD_MID), sizeof(Cmd));
    CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), FM_DELETE_FILE_CC);

    memcpy(Cmd.Payload.Filename, AppName, strlen(AppName) + 1);
    CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
    MISSION_SendReport(Msg, AppName, (uint16)(strlen(AppName) + 1), CFE_SUCCESS, RPT_RETTYPE_SUCCESS);
    OS_TaskDelay(500);
    
    return CFE_SUCCESS;
}
