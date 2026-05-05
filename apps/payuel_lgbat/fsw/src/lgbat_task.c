#include "lgbat_task.h"
#include "lgbat_dispatch.h"
#include "lgbat_eventids.h"

LGBAT_Data_t LGBAT_Data;

void LGBAT_Main(void)
{
    CFE_Status_t    Status;
    CFE_SB_Buffer_t *SBBufPtr;

    CFE_ES_PerfLogEntry(LGBAT_PERF_ID);

    Status = LGBAT_Init();
    if (Status != CFE_SUCCESS)
        LGBAT_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;

    while (CFE_ES_RunLoop(&LGBAT_Data.RunStatus) == true)
    {
        CFE_ES_PerfLogExit(LGBAT_PERF_ID);

        Status = CFE_SB_ReceiveBuffer(&SBBufPtr, LGBAT_Data.CmdPipe, CFE_SB_PEND_FOREVER);

        CFE_ES_PerfLogEntry(LGBAT_PERF_ID);

        if (Status == CFE_SUCCESS)
        {
            LGBAT_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(LGBAT_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "LGBAT: SB Pipe Read Error. RC=0x%08lX",
                              (unsigned long)Status);
            LGBAT_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    CFE_ES_PerfLogExit(LGBAT_PERF_ID);
    CFE_ES_ExitApp(LGBAT_Data.RunStatus);
}

CFE_Status_t LGBAT_Init(void)
{
    CFE_Status_t Status;

    memset(&LGBAT_Data, 0, sizeof(LGBAT_Data));
    LGBAT_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;
    LGBAT_Data.PipeDepth = LGBAT_PIPE_DEPTH;

    strncpy(LGBAT_Data.CmdPipeName, "LGBAT_CMD_PIPE", sizeof(LGBAT_Data.CmdPipeName));
    LGBAT_Data.CmdPipeName[sizeof(LGBAT_Data.CmdPipeName) - 1] = '\0';

    Status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (Status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("LGBAT: EVS Register failed. RC=0x%08lX\n",
                             (unsigned long)Status);
        return Status;
    }

    CFE_MSG_Init(CFE_MSG_PTR(LGBAT_Data.HkTlm.TelemetryHeader),
                 CFE_SB_ValueToMsgId(LGBAT_HK_TLM_MID),
                 sizeof(LGBAT_Data.HkTlm));

    CFE_MSG_Init(CFE_MSG_PTR(LGBAT_Data.ReportTlm.TelemetryHeader),
                 CFE_SB_ValueToMsgId(LGBAT_REPORT_TLM_MID),
                 sizeof(LGBAT_Data.ReportTlm));

    CFE_MSG_Init(CFE_MSG_PTR(LGBAT_Data.BcnTlm.TelemetryHeader),
                 CFE_SB_ValueToMsgId(LGBAT_BCN_TLM_MID),
                 sizeof(LGBAT_Data.BcnTlm));

    Status = CFE_SB_CreatePipe(&LGBAT_Data.CmdPipe, LGBAT_Data.PipeDepth,
                                LGBAT_Data.CmdPipeName);
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(LGBAT_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LGBAT: Create Pipe failed. RC=0x%08lX", (unsigned long)Status);
        return Status;
    }

    Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(LGBAT_CMD_MID), LGBAT_Data.CmdPipe);
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(LGBAT_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LGBAT: Subscribe CMD (0x18C6) failed. RC=0x%08lX",
                          (unsigned long)Status);
        return Status;
    }

    Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(LGBAT_SEND_HK_MID), LGBAT_Data.CmdPipe);
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(LGBAT_SUB_WAKEUP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LGBAT: Subscribe SEND_HK (0x18C7) failed. RC=0x%08lX",
                          (unsigned long)Status);
        return Status;
    }

    Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(LGBAT_SEND_BCN_MID), LGBAT_Data.CmdPipe);
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(LGBAT_SUB_BCN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LGBAT: Subscribe SEND_BCN (0x18C8) failed. RC=0x%08lX",
                          (unsigned long)Status);
        return Status;
    }

    LGBAT_Data.MissionStartTime = CFE_TIME_GetTime();
    LGBAT_Data.MissionActive    = true;

    OS_printf("LGBAT: App initialized. I2C2 Slave=0x%02X MaxMission=%u sec.\n",
              LGBAT_BMS_I2C_SLAVE_ADDR, LGBAT_MAX_MISSION_DURATION_SEC);

    CFE_EVS_SendEvent(LGBAT_INIT_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "LGBAT App Initialized. I2C2 Addr=0x%02X MaxMission=%u sec.",
                      LGBAT_BMS_I2C_SLAVE_ADDR, LGBAT_MAX_MISSION_DURATION_SEC);

    return CFE_SUCCESS;
}
