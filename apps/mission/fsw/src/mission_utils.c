#include "mission_task.h"
#include "mission_msgids.h"
#include "mission_utils.h"
#include "mission_dispatch.h"
#include "cfe_error.h"

static uint32 MISSION_LEOP_CalculateCRC(const void *Data, size_t Size)
{
    return CFE_ES_CalculateCRC(Data, Size, 0, CFE_MISSION_ES_DEFAULT_CRC);
}

static void MISSION_LEOP_SetDefaults(void)
{
    MISSION_Data.LEOPProcessStarted     = false;
    MISSION_Data.LEOPWaitComplete       = false;
    MISSION_Data.LEOPWaitElapsedSec     = 0;
    MISSION_Data.LEOPWaitRemainingSec   = MISSION_LEOP_INITIAL_WAIT_SEC;
    MISSION_Data.LEOPCycleCount         = 0;
    MISSION_Data.LEOPState              = MISSION_LEOP_STATE_INITIAL_WAIT;
}

static uint32 MISSION_LEOP_GetWaitDuration(MISSION_LEOP_State_t State)
{
    switch (State)
    {
        case MISSION_LEOP_STATE_INITIAL_WAIT:
            return MISSION_LEOP_INITIAL_WAIT_SEC;
        case MISSION_LEOP_STATE_BURN:
            return MISSION_LEOP_BURN_DURATION_SEC;
        case MISSION_LEOP_STATE_POST_WAIT:
            return MISSION_LEOP_POST_BURN_WAIT_SEC;
        default:
            return 0;
    }
}

static void MISSION_LEOP_CopyFileToRuntime(const MISSION_LEOP_FileData_t *FileData)
{
    uint32 WaitDurationSec;

    MISSION_LEOP_SetDefaults();
    MISSION_Data.LEOPCycleCount       = FileData->LeopCycleCount;
    MISSION_Data.LEOPState            = (MISSION_LEOP_State_t)FileData->LeopState;
    MISSION_Data.LEOPWaitElapsedSec   = FileData->LeopWaitElapsedSec;
    WaitDurationSec                   = MISSION_LEOP_GetWaitDuration(MISSION_Data.LEOPState);

    if (WaitDurationSec == 0u || MISSION_Data.LEOPWaitElapsedSec >= WaitDurationSec)
    {
        MISSION_Data.LEOPWaitComplete     = true;
        MISSION_Data.LEOPWaitRemainingSec = 0;
    }
    else
    {
        MISSION_Data.LEOPWaitRemainingSec = WaitDurationSec - MISSION_Data.LEOPWaitElapsedSec;
    }
}

static void MISSION_LEOP_CopyRuntimeToFile(MISSION_LEOP_FileData_t *FileData)
{
    memset(FileData, 0, sizeof(*FileData));
    FileData->Signature            = MISSION_LEOP_FILE_SIGNATURE;
    FileData->Version              = MISSION_LEOP_FILE_VERSION;
    FileData->LeopCycleCount       = MISSION_Data.LEOPCycleCount;
    FileData->LeopState            = (uint8)MISSION_Data.LEOPState;
    FileData->LeopWaitElapsedSec   = MISSION_Data.LEOPWaitElapsedSec;
    FileData->CRC                  = MISSION_LEOP_CalculateCRC(FileData, sizeof(*FileData) - sizeof(FileData->CRC));
}

CFE_Status_t MISSION_LEOP_Lock(void)
{
    int32 OsStatus;

    if (MISSION_Data.LEOPMutex == OS_OBJECT_ID_UNDEFINED)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OsStatus = OS_MutSemTake(MISSION_Data.LEOPMutex);
    if (OsStatus != OS_SUCCESS)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}

void MISSION_LEOP_Unlock(void)
{
    if (MISSION_Data.LEOPMutex != OS_OBJECT_ID_UNDEFINED)
    {
        (void)OS_MutSemGive(MISSION_Data.LEOPMutex);
    }
}

static CFE_Status_t MISSION_LEOP_LockFile(void)
{
    int32 OsStatus;

    if (MISSION_Data.LEOPFileMutex == OS_OBJECT_ID_UNDEFINED)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OsStatus = OS_MutSemTake(MISSION_Data.LEOPFileMutex);
    if (OsStatus != OS_SUCCESS)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}

static void MISSION_LEOP_UnlockFile(void)
{
    if (MISSION_Data.LEOPFileMutex != OS_OBJECT_ID_UNDEFINED)
    {
        (void)OS_MutSemGive(MISSION_Data.LEOPFileMutex);
    }
}

static CFE_Status_t MISSION_LEOP_OpenDataFile(void)
{
    int32 OsStatus;

    if (MISSION_Data.LEOPDataHandle != OS_OBJECT_ID_UNDEFINED)
    {
        return CFE_SUCCESS;
    }

    OsStatus = OS_OpenCreate(&MISSION_Data.LEOPDataHandle, MISSION_LEOP_DATA_PATH, OS_FILE_FLAG_CREATE, OS_READ_WRITE);
    if (OsStatus != OS_SUCCESS)
    {
        MISSION_Data.LEOPDataHandle = OS_OBJECT_ID_UNDEFINED;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}

static CFE_Status_t MISSION_LEOP_LoadState(void)
{
    CFE_Status_t            Status;
    MISSION_LEOP_FileData_t FileData;
    int32                   OsStatus;
    uint32                  ExpectedCRC;

    Status = MISSION_LEOP_LockFile();
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    Status = MISSION_LEOP_OpenDataFile();
    if (Status != CFE_SUCCESS)
    {
        MISSION_LEOP_UnlockFile();
        return Status;
    }

    OsStatus = OS_lseek(MISSION_Data.LEOPDataHandle, 0, OS_SEEK_SET);
    if (OsStatus < OS_SUCCESS)
    {
        (void)OS_close(MISSION_Data.LEOPDataHandle);
        MISSION_Data.LEOPDataHandle = OS_OBJECT_ID_UNDEFINED;
        MISSION_LEOP_UnlockFile();
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    memset(&FileData, 0, sizeof(FileData));
    OsStatus = OS_read(MISSION_Data.LEOPDataHandle, &FileData, sizeof(FileData));
    (void)OS_close(MISSION_Data.LEOPDataHandle);
    MISSION_Data.LEOPDataHandle = OS_OBJECT_ID_UNDEFINED;
    MISSION_LEOP_UnlockFile();
    if (OsStatus != (int32)sizeof(FileData))
    {
        MISSION_LEOP_SetDefaults();
        MISSION_APP_printf("MISSION LEOP: state file missing, short, or invalid; defaults applied\n");
        return MISSION_LEOP_SaveState();
    }

    ExpectedCRC = MISSION_LEOP_CalculateCRC(&FileData, sizeof(FileData) - sizeof(FileData.CRC));
    if (FileData.Signature != MISSION_LEOP_FILE_SIGNATURE || FileData.Version != MISSION_LEOP_FILE_VERSION ||
        FileData.CRC != ExpectedCRC || FileData.LeopCycleCount > MISSION_LEOP_SEQUENCE_REPEAT_COUNT ||
        FileData.LeopState > MISSION_LEOP_STATE_COMPLETE ||
        (FileData.LeopState == MISSION_LEOP_STATE_INITIAL_WAIT && FileData.LeopCycleCount != 0u) ||
        (FileData.LeopState == MISSION_LEOP_STATE_COMPLETE &&
         FileData.LeopCycleCount != MISSION_LEOP_SEQUENCE_REPEAT_COUNT) ||
        (FileData.LeopState != MISSION_LEOP_STATE_COMPLETE &&
         FileData.LeopCycleCount >= MISSION_LEOP_SEQUENCE_REPEAT_COUNT) ||
        FileData.LeopWaitElapsedSec >
            MISSION_LEOP_GetWaitDuration((MISSION_LEOP_State_t)FileData.LeopState))
    {
        MISSION_LEOP_SetDefaults();
        MISSION_APP_printf("MISSION LEOP: state file missing, short, or invalid; defaults applied\n");
        return MISSION_LEOP_SaveState();
    }

    MISSION_LEOP_CopyFileToRuntime(&FileData);
    MISSION_APP_printf("MISSION LEOP: state loaded state=%u cycle_count=%u wait_elapsed=%lu sec\n",
                       (unsigned int)MISSION_Data.LEOPState,
                       (unsigned int)MISSION_Data.LEOPCycleCount,
                       (unsigned long)MISSION_Data.LEOPWaitElapsedSec);

    return CFE_SUCCESS;
}

static void MISSION_LEOP_Wait(uint32 WaitTimeSec)
{
    CFE_TIME_SysTime_t Now;
    CFE_TIME_SysTime_t SessionElapsed;
    uint32             SavedElapsedSec;
    uint32             TotalElapsedSec;
    uint32             NextSaveElapsedSec;
    uint32             NextLogElapsedSec;

    if (WaitTimeSec == 0u || MISSION_Data.LEOPCompleteRequested)
    {
        MISSION_Data.LEOPProcessStarted   = false;
        MISSION_Data.LEOPWaitComplete     = false;
        MISSION_Data.LEOPWaitElapsedSec   = 0;
        MISSION_Data.LEOPWaitRemainingSec = 0;
        return;
    }

    if (MISSION_Data.LEOPWaitElapsedSec >= WaitTimeSec)
    {
        MISSION_Data.LEOPProcessStarted   = false;
        MISSION_Data.LEOPWaitComplete     = false;
        MISSION_Data.LEOPWaitElapsedSec   = 0;
        MISSION_Data.LEOPWaitRemainingSec = 0;
        return;
    }

    SavedElapsedSec                     = MISSION_Data.LEOPWaitElapsedSec;
    MISSION_Data.LEOPProcessStarted   = true;
    MISSION_Data.LEOPWaitComplete     = false;
    MISSION_Data.LEOPWaitRemainingSec = WaitTimeSec - SavedElapsedSec;
    MISSION_Data.LEOPStartTime        = CFE_TIME_GetTime();
    NextSaveElapsedSec                = SavedElapsedSec + MISSION_LEOP_SAVE_INTERVAL_SEC;
    NextLogElapsedSec                 = SavedElapsedSec + MISSION_LEOP_WAIT_LOG_SEC;

    while (!MISSION_Data.LEOPWaitComplete)
    {
        OS_TaskDelay(1000);
        if (MISSION_Data.LEOPCompleteRequested)
        {
            break;
        }

        Now              = CFE_TIME_GetTime();
        SessionElapsed   = CFE_TIME_Subtract(Now, MISSION_Data.LEOPStartTime);
        TotalElapsedSec  = SavedElapsedSec + SessionElapsed.Seconds;

        if (TotalElapsedSec >= WaitTimeSec)
        {
            MISSION_Data.LEOPWaitComplete     = true;
            MISSION_Data.LEOPWaitElapsedSec   = WaitTimeSec;
            MISSION_Data.LEOPWaitRemainingSec = 0;
        }
        else
        {
            MISSION_Data.LEOPWaitElapsedSec   = TotalElapsedSec;
            MISSION_Data.LEOPWaitRemainingSec = WaitTimeSec - TotalElapsedSec;
        }

        if (MISSION_Data.LEOPWaitComplete || MISSION_Data.LEOPWaitElapsedSec >= NextSaveElapsedSec)
        {
            (void)MISSION_LEOP_SaveState();
            NextSaveElapsedSec = MISSION_Data.LEOPWaitElapsedSec + MISSION_LEOP_SAVE_INTERVAL_SEC;
        }

        if (MISSION_Data.LEOPWaitComplete || MISSION_Data.LEOPWaitElapsedSec >= NextLogElapsedSec)
        {
            MISSION_APP_printf("MISSION LEOP: wait remaining=%lu sec elapsed=%lu sec\n",
                               (unsigned long)MISSION_Data.LEOPWaitRemainingSec,
                               (unsigned long)MISSION_Data.LEOPWaitElapsedSec);
            NextLogElapsedSec = MISSION_Data.LEOPWaitElapsedSec + MISSION_LEOP_WAIT_LOG_SEC;
        }
    }

    MISSION_Data.LEOPProcessStarted   = false;
    MISSION_Data.LEOPWaitComplete     = false;
    MISSION_Data.LEOPWaitElapsedSec   = 0;
    MISSION_Data.LEOPWaitRemainingSec = 0;
}

static CFE_Status_t MISSION_LEOP_EnableTo(void)
{
    CFE_Status_t            Status;
    TO_LAB_EnableOutputCmd_t Cmd;

    memset(&Cmd, 0, sizeof(Cmd));
    Status = CFE_MSG_Init(CFE_MSG_PTR(Cmd.CommandHeader), CFE_SB_ValueToMsgId(TO_LAB_CMD_MID), sizeof(Cmd));
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    Status = CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), TO_LAB_OUTPUT_ENABLE_CC);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    MISSION_APP_printf("MISSION LEOP: TO enable transmit\n");

    Status = CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    MISSION_APP_printf("MISSION LEOP: TO enabled\n");

    return Status;
}

static CFE_Status_t MISSION_LEOP_StartAdcsDetumble(void)
{
    CFE_Status_t                    Status;
    ADCS_SequenceCmdDetumblingCmd_t Cmd;

    memset(&Cmd, 0, sizeof(Cmd));
    Status = CFE_MSG_Init(CFE_MSG_PTR(Cmd.CommandHeader), CFE_SB_ValueToMsgId(ADCS_CMD_MID), sizeof(Cmd));
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    Status = CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), ADCS_SEQ_DTUMB_CC);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    MISSION_APP_printf("MISSION LEOP: ADCS detumble transmit\n");
    return CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
}

static CFE_Status_t MISSION_LEOP_SendGpioDeployCmd(CFE_MSG_FcnCode_t FcnCode)
{
    CFE_Status_t         Status;
    GPIO_Dep1EnHighCmd_t Cmd;

    memset(&Cmd, 0, sizeof(Cmd));
    Status = CFE_MSG_Init(CFE_MSG_PTR(Cmd.CommandHeader), CFE_SB_ValueToMsgId(GPIO_CMD_MID), sizeof(Cmd));
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    Status = CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), FcnCode);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    return CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
}

static CFE_Status_t MISSION_LEOP_GpioHigh(void)
{
    CFE_Status_t Status;

    Status = MISSION_LEOP_SendGpioDeployCmd(GPIO_DEP1_EN_HIGH_CC);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    MISSION_LEOP_Wait(MISSION_LEOP_BURN_DURATION_SEC);

    Status = MISSION_LEOP_SendGpioDeployCmd(GPIO_DEP1_EN_LOW_CC);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t MISSION_LEOP_SaveState(void)
{
    MISSION_LEOP_FileData_t FileData;
    CFE_Status_t            Status;
    int32                   OsStatus;

    Status = MISSION_LEOP_Lock();
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    MISSION_LEOP_CopyRuntimeToFile(&FileData);
    MISSION_LEOP_Unlock();

    Status = MISSION_LEOP_LockFile();
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    if (MISSION_Data.LEOPDataHandle != OS_OBJECT_ID_UNDEFINED)
    {
        (void)OS_close(MISSION_Data.LEOPDataHandle);
        MISSION_Data.LEOPDataHandle = OS_OBJECT_ID_UNDEFINED;
    }

    OsStatus = OS_OpenCreate(&MISSION_Data.LEOPDataHandle, MISSION_LEOP_TEMP_DATA_PATH,
                             OS_FILE_FLAG_CREATE | OS_FILE_FLAG_TRUNCATE, OS_READ_WRITE);
    if (OsStatus != OS_SUCCESS)
    {
        MISSION_Data.LEOPDataHandle = OS_OBJECT_ID_UNDEFINED;
        MISSION_LEOP_UnlockFile();
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OsStatus = OS_write(MISSION_Data.LEOPDataHandle, &FileData, sizeof(FileData));
    (void)OS_close(MISSION_Data.LEOPDataHandle);
    MISSION_Data.LEOPDataHandle = OS_OBJECT_ID_UNDEFINED;
    if (OsStatus != (int32)sizeof(FileData))
    {
        MISSION_LEOP_UnlockFile();
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OsStatus = OS_rename(MISSION_LEOP_TEMP_DATA_PATH, MISSION_LEOP_DATA_PATH);
    MISSION_LEOP_UnlockFile();
    if (OsStatus != OS_SUCCESS)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}

CFE_Status_t MISSION_LEOP_RequestComplete(void)
{
    CFE_Status_t Status;

    Status = MISSION_LEOP_Lock();
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    MISSION_Data.LEOPCompleteRequested  = true;
    MISSION_Data.LEOPProcessStarted     = false;
    MISSION_Data.LEOPWaitComplete       = true;
    MISSION_Data.LEOPWaitElapsedSec     = 0;
    MISSION_Data.LEOPWaitRemainingSec   = 0;
    MISSION_Data.LEOPCycleCount         = MISSION_LEOP_SEQUENCE_REPEAT_COUNT;
    MISSION_Data.LEOPState              = MISSION_LEOP_STATE_COMPLETE;

    MISSION_LEOP_Unlock();

    MISSION_APP_printf("MISSION LEOP: complete requested by command\n");
    return MISSION_LEOP_SaveState();
}

void MISSION_LEOP_Process(void)
{
    CFE_Status_t Status;
    uint8        Cycle;

    Status = MISSION_LEOP_LoadState();
    if (Status != CFE_SUCCESS)
    {
        MISSION_Data.ErrCounter++;
        MISSION_LEOP_SetDefaults();
        MISSION_APP_printf("MISSION LEOP: state load failed status=0x%08lX; continuing with defaults\n",
                           (unsigned long)Status);
    }

    if (MISSION_ENABLE_ADCS_DETUMBLE)
    {
        Status = MISSION_LEOP_StartAdcsDetumble();
        if (Status != CFE_SUCCESS)
        {
            MISSION_Data.ErrCounter++;
            MISSION_APP_printf("MISSION LEOP: ADCS detumble transmit failed status=0x%08lX\n",
                               (unsigned long)Status);
        }
    }
    else
    {
        MISSION_APP_printf("MISSION LEOP: ADCS detumble disabled by config\n");
    }

    if (MISSION_Data.LEOPState == MISSION_LEOP_STATE_COMPLETE)
    {
        MISSION_LEOP_EnableTo();
        MISSION_APP_printf("MISSION LEOP: complete state loaded, exiting\n");
        return;
    }

    if (MISSION_Data.LEOPState == MISSION_LEOP_STATE_INITIAL_WAIT)
    {
        MISSION_APP_printf("MISSION LEOP: initial wait started duration=%lu sec\n",
                           (unsigned long)MISSION_LEOP_INITIAL_WAIT_SEC);
        MISSION_LEOP_Wait(MISSION_LEOP_INITIAL_WAIT_SEC);
        if (!MISSION_Data.LEOPCompleteRequested)
        {
            MISSION_Data.LEOPState = MISSION_LEOP_STATE_BURN;
            (void)MISSION_LEOP_SaveState();
        }
    }

    for (Cycle = MISSION_Data.LEOPCycleCount;
         Cycle < MISSION_LEOP_SEQUENCE_REPEAT_COUNT && !MISSION_Data.LEOPCompleteRequested; Cycle++)
    {
        MISSION_LEOP_Wait(0u);
        MISSION_Data.LEOPState = MISSION_LEOP_STATE_BURN;

        MISSION_APP_printf("MISSION LEOP: cycle=%u/%u issuing GPIO burn duration=%lu sec\n",
                           (unsigned int)(Cycle + 1), (unsigned int)MISSION_LEOP_SEQUENCE_REPEAT_COUNT,
                           (unsigned long)MISSION_LEOP_BURN_DURATION_SEC);

        (void)MISSION_LEOP_GpioHigh();
        if (MISSION_Data.LEOPCompleteRequested)
        {
            break;
        }
        (void)MISSION_LEOP_SaveState();

        if (MISSION_Data.LEOPCompleteRequested)
        {
            break;
        }
        MISSION_Data.LEOPState = MISSION_LEOP_STATE_TO_ENABLE;
        (void)MISSION_LEOP_EnableTo();
        if (MISSION_Data.LEOPCompleteRequested)
        {
            break;
        }
        (void)MISSION_LEOP_SaveState();

        if (MISSION_Data.LEOPCompleteRequested)
        {
            break;
        }
        MISSION_Data.LEOPState = MISSION_LEOP_STATE_POST_WAIT;
        (void)MISSION_LEOP_SaveState();
        MISSION_APP_printf("MISSION LEOP: cycle=%u/%u post-burn wait started duration=%lu sec\n",
                           (unsigned int)(Cycle + 1), (unsigned int)MISSION_LEOP_SEQUENCE_REPEAT_COUNT,
                           (unsigned long)MISSION_LEOP_POST_BURN_WAIT_SEC);
        MISSION_LEOP_Wait(MISSION_LEOP_POST_BURN_WAIT_SEC);

        if (MISSION_Data.LEOPCompleteRequested)
        {
            break;
        }

        MISSION_Data.LEOPCycleCount = Cycle + 1u;
        MISSION_Data.LEOPState = (MISSION_Data.LEOPCycleCount < MISSION_LEOP_SEQUENCE_REPEAT_COUNT)
                                     ? MISSION_LEOP_STATE_BURN
                                     : MISSION_LEOP_STATE_COMPLETE;
        (void)MISSION_LEOP_SaveState();
    }

    MISSION_LEOP_Wait(0u);
    MISSION_Data.LEOPWaitComplete = true;
    MISSION_Data.LEOPCycleCount   = MISSION_LEOP_SEQUENCE_REPEAT_COUNT;
    MISSION_Data.LEOPState        = MISSION_LEOP_STATE_COMPLETE;
    (void)MISSION_LEOP_SaveState();
    MISSION_APP_printf("MISSION LEOP: sequence complete cycle_count=%u repeat_count=%u\n",
                       (unsigned int)MISSION_Data.LEOPCycleCount,
                       (unsigned int)MISSION_LEOP_SEQUENCE_REPEAT_COUNT);
}

void MISSION_LEOP_Task(void)
{
    MISSION_LEOP_Process();
    CFE_ES_ExitChildTask();
}
