#include "mission_task.h"
#include "mission_cmd.h"
#include "mission_utils.h"
#include "mission_msg.h"

#include <string.h>

static void MISSION_SendReport(const void *Msg, const void *Data, uint16 DataSize, int32 ReturnCode, uint8 ReturnType)
{
    CFE_SB_MsgId_t   CmdMid;
    CFE_MSG_FcnCode_t CmdCode;
    uint16 CopySize = DataSize > RPT_RET_VALUE_BUF_SIZE ? RPT_RET_VALUE_BUF_SIZE : DataSize;

    memset(&MISSION_Data.Report, 0, sizeof(MISSION_Data.Report));

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

static uint8 MISSION_ReturnTypeFromStatus(CFE_Status_t Status)
{
    return (Status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_APP;
}

static CFE_Status_t MISSION_UpdateLeopTlmPayload(MISSION_HkTlm_Payload_t *Payload)
{
    Payload->CmdErrCounter = MISSION_Data.ErrCounter;

    if (MISSION_LEOP_Lock() != CFE_SUCCESS)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    Payload->LeopWaitComplete       = MISSION_Data.LEOPWaitComplete;
    Payload->LeopWaitElapsedSec     = MISSION_Data.LEOPWaitElapsedSec;
    Payload->LeopWaitRemainingSec   = MISSION_Data.LEOPWaitRemainingSec;
    Payload->LeopCycleCount = MISSION_Data.LEOPCycleCount;
    Payload->LeopState      = (uint8_t)MISSION_Data.LEOPState;

    MISSION_LEOP_Unlock();

    return CFE_SUCCESS;
}

CFE_Status_t MISSION_SendHKCmd(const MISSION_SendHkCmd_t *Msg) {
    CFE_Status_t Status;

    (void)Msg;

    Status = MISSION_UpdateLeopTlmPayload(&MISSION_Data.HkTlm.Payload);
    if (Status != CFE_SUCCESS)
    {
        MISSION_Data.ErrCounter++;
    }

    MISSION_SendReport(Msg, &MISSION_Data.HkTlm.Payload, sizeof(MISSION_Data.HkTlm.Payload), Status,
                       MISSION_ReturnTypeFromStatus(Status));

    return Status;
}

CFE_Status_t MISSION_SetCompleteCmd(const MISSION_SetCompleteCmd_t *Msg) {
    CFE_Status_t Status;

    Status = MISSION_LEOP_RequestComplete();
    if (Status != CFE_SUCCESS)
    {
        MISSION_Data.ErrCounter++;
    }

    (void)MISSION_UpdateLeopTlmPayload(&MISSION_Data.HkTlm.Payload);
    MISSION_SendReport(Msg, &MISSION_Data.HkTlm.Payload, sizeof(MISSION_Data.HkTlm.Payload), Status,
                       MISSION_ReturnTypeFromStatus(Status));

    return Status;
}

CFE_Status_t MISSION_ReadLeopFileCmd(const MISSION_ReadLeopFileCmd_t *Msg) {
    uint8        FileData[RPT_RET_VALUE_BUF_SIZE] = {0};
    uint16       BytesRead = 0;
    CFE_Status_t Status;

    Status = MISSION_LEOP_ReadStateFile(FileData, sizeof(FileData), &BytesRead);
    if (Status != CFE_SUCCESS)
    {
        MISSION_Data.ErrCounter++;
    }

    MISSION_SendReport(Msg, FileData, BytesRead, Status, MISSION_ReturnTypeFromStatus(Status));
    return Status;
}
