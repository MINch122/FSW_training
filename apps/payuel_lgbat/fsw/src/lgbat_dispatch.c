#include "lgbat_task.h"
#include "lgbat_dispatch.h"
#include "lgbat_cmds.h"
#include "lgbat_eventids.h"
#include "lgbat_msgids.h"


// LGBAT_VerifyCmdLength

bool LGBAT_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool   result = true;
    size_t ActualLength = 0;
    CFE_SB_MsgId_t    MsgId   = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    if (ExpectedLength != ActualLength)
    {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(LGBAT_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LGBAT: Invalid Msg len: ID=0x%X CC=%u Len=%u Exp=%u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId),
                          (unsigned int)FcnCode,
                          (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;
        LGBAT_Data.ErrCounter++;

        // Send RPT with actual and expected lengths in the return value buffer
        LGBAT_ReportTlm_t *BufPtr =
            (LGBAT_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(LGBAT_ReportTlm_t));
        if (BufPtr == NULL) goto cleanup;

        if (CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader),
                         CFE_SB_ValueToMsgId(LGBAT_REPORT_TLM_MID),
                         sizeof(LGBAT_ReportTlm_t)) != CFE_SUCCESS)
        {
            CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
            goto cleanup;
        }

        BufPtr->Payload.MsgID          = (uint16_t)CFE_SB_MsgIdToValue(MsgId);
        BufPtr->Payload.CommandCode    = (uint8_t)FcnCode;
        BufPtr->Payload.ReturnType     = RPT_RETTYPE_APP;
        BufPtr->Payload.ReturnCode     = CFE_STATUS_WRONG_MSG_LENGTH;
        BufPtr->Payload.ReturnDataSize = 2 * sizeof(uint32_t);

        uint32_t tmp = (uint32_t)ActualLength;
        memcpy(BufPtr->Payload.ReturnValue, &tmp, sizeof(uint32_t));
        tmp = (uint32_t)ExpectedLength;
        memcpy(BufPtr->Payload.ReturnValue + sizeof(uint32_t), &tmp, sizeof(uint32_t));

        CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
        if (CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true) != CFE_SUCCESS)
            CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
    }
cleanup:
    return result;
}


// LGBAT_ProcessGroundCommand

void LGBAT_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CC = 0xFF;
    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CC);

    switch (CC)
    {
        case LGBAT_NOOP_CC:
            if (LGBAT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(LGBAT_NoopCmd_t)))
                LGBAT_NoopCmd((const LGBAT_NoopCmd_t *)SBBufPtr);
            break;

        case LGBAT_RESET_COUNTER_CC:
            if (LGBAT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(LGBAT_ResetCounterCmd_t)))
                LGBAT_ResetCounterCmd((const LGBAT_ResetCounterCmd_t *)SBBufPtr);
            break;

        case LGBAT_SEND_BCN_CC:
            if (LGBAT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(LGBAT_SendBcnCmd_t)))
                LGBAT_SendBeaconCmd();
            break;

        case LGBAT_REQUEST_DATA_CC:
            if (LGBAT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(LGBAT_RequestDataCmd_t)))
                LGBAT_RequestDataCmd((const LGBAT_RequestDataCmd_t *)SBBufPtr);
            break;

        case LGBAT_REQUEST_ALL_DATA_CC:
            if (LGBAT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(LGBAT_RequestAllDataCmd_t)))
                LGBAT_RequestAllDataCmd((const LGBAT_RequestAllDataCmd_t *)SBBufPtr);
            break;

        case LGBAT_SET_POWER_CC:
            if (LGBAT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(LGBAT_SetPowerCmd_t)))
                LGBAT_SetPowerCmd((const LGBAT_SetPowerCmd_t *)SBBufPtr);
            break;

        case LGBAT_RESET_BMS_CC:
            if (LGBAT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(LGBAT_ResetBmsCmd_t)))
                LGBAT_ResetBmsCmd((const LGBAT_ResetBmsCmd_t *)SBBufPtr);
            break;

        default:
            // Unknown CC: count the error and send RPT
            LGBAT_Data.ErrCounter++;
            CFE_EVS_SendEvent(LGBAT_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                              "LGBAT: Invalid CC=%d", (int)CC);
            {
                CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
                CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);
                LGBAT_ReportTlm_t *BufPtr =
                    (LGBAT_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(
                        sizeof(LGBAT_ReportTlm_t));
                if (BufPtr != NULL)
                {
                    if (CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader),
                                     CFE_SB_ValueToMsgId(LGBAT_REPORT_TLM_MID),
                                     sizeof(LGBAT_ReportTlm_t)) == CFE_SUCCESS)
                    {
                        BufPtr->Payload.MsgID          = (uint16_t)CFE_SB_MsgIdToValue(MsgId);
                        BufPtr->Payload.CommandCode    = (uint8_t)CC;
                        BufPtr->Payload.ReturnType     = RPT_RETTYPE_APP;
                        BufPtr->Payload.ReturnCode     = CFE_STATUS_BAD_COMMAND_CODE;
                        BufPtr->Payload.ReturnDataSize = 0;
                        CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
                        if (CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true)
                                != CFE_SUCCESS)
                            CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
                    }
                    else
                        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
                }
            }
            break;
    }
}


// LGBAT_SendHkHandler

static void LGBAT_SendHkHandler(void)
{
    // Check whether the 2-week mission window has expired
    CFE_TIME_SysTime_t Now     = CFE_TIME_GetTime();
    CFE_TIME_SysTime_t Elapsed = CFE_TIME_Subtract(Now, LGBAT_Data.MissionStartTime);

    if (Elapsed.Seconds > LGBAT_MAX_MISSION_DURATION_SEC)
    {
        if (LGBAT_Data.MissionActive)
        {
            CFE_EVS_SendEvent(LGBAT_MISSION_END_INF_EID, CFE_EVS_EventType_INFORMATION,
                              "LGBAT: Mission duration exceeded (%u sec). Shutting down.",
                              LGBAT_MAX_MISSION_DURATION_SEC);
            LGBAT_Data.MissionActive = false;
        }
        LGBAT_Data.RunStatus = CFE_ES_RunStatus_APP_EXIT;
        return;
    }

    // Skip I2C reads if 3.3V is not applied
    if (!LGBAT_Data.PowerApplied)
    {
        OS_printf("LGBAT HK: skipped I2C poll. PowerApplied=0\n");
        return;
    }

    // Read all 12 Data IDs: 0x01 through 0x0C
    bool CycleOk = true;
    for (uint8_t id = LGBAT_BMS_DATA_ID_MIN; id <= LGBAT_BMS_DATA_ID_MAX; id++)
    {
        CFE_Status_t Status = LGBAT_I2C_ReadBmsData(id);
        if (Status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(LGBAT_I2C_READ_ERR_EID, CFE_EVS_EventType_ERROR,
                              "LGBAT: HK I2C fail. DataID=0x%02X", id);
            LGBAT_Data.ErrCounter++;
            CycleOk = false;
        }
    }

    // Build and transmit HK TLM (0x08C6) with summary fields
    LGBAT_HkTlm_Payload_t *HK = &LGBAT_Data.HkTlm.Payload;
    memset(HK, 0, sizeof(*HK));
    HK->CmdCounter          = LGBAT_Data.CmdCounter;
    HK->CmdErrCounter       = LGBAT_Data.ErrCounter;
    HK->PowerApplied        = 1u;
    HK->MissionActive       = LGBAT_Data.MissionActive ? 1u : 0u;
    HK->FirstCommDone       = LGBAT_Data.FirstCommSuccess ? 1u : 0u;
    HK->Pack_Voltage_mV     = LGBAT_Data.BmsData.Data01.Pack_Voltage;
    HK->Pack_Current_mA     = LGBAT_Data.BmsData.Data01.Pack_Current;
    HK->SOC_x100            = LGBAT_Data.BmsData.Data02.SOC;
    HK->SOH_pct             = LGBAT_Data.BmsData.Data02.SOH;
    HK->Power_Supply_Status = LGBAT_Data.BmsData.Data02.Power_Supply_Status;
    HK->Failure_Level       = LGBAT_0x09_GET_FAILURE_LEVEL(
                                  LGBAT_Data.BmsData.Data09.TempFailLevel);
    HK->BMS_Wakeup          = LGBAT_Data.BmsData.Data09.BMS_Wakeup;
    HK->FailStatus2_Raw     = LGBAT_Data.BmsData.Data0A.FailStatus2;
    HK->FailStatus3_Raw     = LGBAT_Data.BmsData.Data0A.FailStatus3;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(LGBAT_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(LGBAT_Data.HkTlm.TelemetryHeader), true);

    // Run health check after every full poll cycle
    LGBAT_CheckBmsHealth();

    // Record first successful full cycle
    if (CycleOk && !LGBAT_Data.FirstCommSuccess)
    {
        LGBAT_Data.FirstCommSuccess = true;
        CFE_EVS_SendEvent(LGBAT_MISSION_START_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "LGBAT: First BMS comm cycle SUCCESS. Mission Criteria met.");
    }
}


// LGBAT_TaskPipe

void LGBAT_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case LGBAT_CMD_MID:      // 0x18C6 Ground command
            LGBAT_ProcessGroundCommand(SBBufPtr);
            break;

        case LGBAT_SEND_HK_MID: // 0x18C7 SCH periodic HK/I2C poll trigger
            LGBAT_SendHkHandler();
            break;

        case LGBAT_SEND_BCN_MID: // 0x18C8 SCH beacon send request
            LGBAT_SendBeaconCmd();
            break;

        default:
            CFE_EVS_SendEvent(LGBAT_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "LGBAT: Invalid MID=0x%X",
                              (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
