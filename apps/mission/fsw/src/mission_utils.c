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
    MISSION_Data.LEOPWaitComplete       = false;
    MISSION_Data.LEOPWaitElapsedSec     = 0;
    MISSION_Data.LEOPWaitRemainingSec   = MISSION_LEOP_WAIT_DURATION_SEC;
    MISSION_Data.LEOPUartDeployTryCount = 0;
    MISSION_Data.LEOPGpioBurnTryCount   = 0;
    MISSION_Data.LEOPToEnabled          = false;
    MISSION_Data.LEOPGpioDeployIssued   = false;
    MISSION_Data.LEOPUtrxRxBytesInitialized = false;
    MISSION_Data.LEOPUtrxRxBytesIncreased   = false;
    MISSION_Data.LEOPUtrxInitRxBytes        = 0;
    MISSION_Data.LEOPUtrxRxData             = 0;
    MISSION_Data.LEOPState              = MISSION_LEOP_STATE_WAITING;
}

static void MISSION_LEOP_CopyFileToRuntime(const MISSION_LEOP_FileData_t *FileData)
{
    MISSION_Data.LEOPWaitComplete       = (FileData->LeopWaitComplete != 0u);
    MISSION_Data.LEOPWaitElapsedSec     = FileData->LeopWaitElapsedSec;
    MISSION_Data.LEOPWaitRemainingSec   = FileData->LeopWaitRemainingSec;
    MISSION_Data.LEOPUartDeployTryCount = FileData->LeopUartDeployTryCount;
    MISSION_Data.LEOPGpioBurnTryCount   = FileData->LeopGpioBurnTryCount;
    MISSION_Data.LEOPState              = (MISSION_LEOP_State_t)FileData->LeopState;
    MISSION_Data.LEOPToEnabled          = (FileData->LeopToEnabled != 0u);
    MISSION_Data.LEOPGpioDeployIssued   = (FileData->LeopGpioDeployIssued != 0u);
}

static void MISSION_LEOP_CopyRuntimeToFile(MISSION_LEOP_FileData_t *FileData)
{
    memset(FileData, 0, sizeof(*FileData));
    FileData->Signature              = MISSION_LEOP_FILE_SIGNATURE;
    FileData->Version                = MISSION_LEOP_FILE_VERSION;
    FileData->LeopWaitComplete       = (uint8)MISSION_Data.LEOPWaitComplete;
    FileData->LeopWaitElapsedSec     = MISSION_Data.LEOPWaitElapsedSec;
    FileData->LeopWaitRemainingSec   = MISSION_Data.LEOPWaitRemainingSec;
    FileData->LeopUartDeployTryCount = MISSION_Data.LEOPUartDeployTryCount;
    FileData->LeopGpioBurnTryCount   = MISSION_Data.LEOPGpioBurnTryCount;
    FileData->LeopState              = (uint8)MISSION_Data.LEOPState;
    FileData->LeopToEnabled          = (uint8)MISSION_Data.LEOPToEnabled;
    FileData->LeopGpioDeployIssued   = (uint8)MISSION_Data.LEOPGpioDeployIssued;
    FileData->CRC                    = MISSION_LEOP_CalculateCRC(FileData, sizeof(*FileData) - sizeof(FileData->CRC));
}

static void MISSION_LEOP_RetryDelay(void)
{
    OS_TaskDelay(1000 * MISSION_LEOP_RETRY_DELAY_SEC);
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

static bool MISSION_LEOP_UtrxRxBytesIncreased(void)
{
    bool Increased;

    if (MISSION_LEOP_Lock() != CFE_SUCCESS)
    {
        return false;
    }

    Increased = MISSION_Data.LEOPUtrxRxBytesIncreased;
    MISSION_LEOP_Unlock();

    return Increased;
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
        MISSION_LEOP_UnlockFile();
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    memset(&FileData, 0, sizeof(FileData));
    OsStatus = OS_read(MISSION_Data.LEOPDataHandle, &FileData, sizeof(FileData));
    MISSION_LEOP_UnlockFile();
    if (OsStatus != (int32)sizeof(FileData))
    {
        MISSION_LEOP_SetDefaults();
        return MISSION_LEOP_SaveState();
    }

    ExpectedCRC = MISSION_LEOP_CalculateCRC(&FileData, sizeof(FileData) - sizeof(FileData.CRC));
    if (FileData.Signature != MISSION_LEOP_FILE_SIGNATURE || FileData.Version != MISSION_LEOP_FILE_VERSION ||
        FileData.CRC != ExpectedCRC)
    {
        MISSION_LEOP_SetDefaults();
        return MISSION_LEOP_SaveState();
    }

    MISSION_LEOP_CopyFileToRuntime(&FileData);

    return CFE_SUCCESS;
}

static void MISSION_LEOP_UpdateWaitStatus(void)
{
    CFE_TIME_SysTime_t Now;
    CFE_TIME_SysTime_t Elapsed;

    if (MISSION_Data.LEOPWaitComplete)
    {
        MISSION_Data.LEOPWaitElapsedSec   = MISSION_LEOP_WAIT_DURATION_SEC;
        MISSION_Data.LEOPWaitRemainingSec = 0;
        return;
    }

    Now = CFE_TIME_GetTime();
    Elapsed = CFE_TIME_Subtract(Now, MISSION_Data.LEOPStartTime);
    MISSION_Data.LEOPWaitElapsedSec = Elapsed.Seconds;

    if (Elapsed.Seconds >= MISSION_LEOP_WAIT_DURATION_SEC)
    {
        MISSION_Data.LEOPWaitComplete = true;
        MISSION_Data.LEOPWaitElapsedSec = MISSION_LEOP_WAIT_DURATION_SEC;
        MISSION_Data.LEOPWaitRemainingSec = 0;
    }
    else
    {
        MISSION_Data.LEOPWaitRemainingSec = MISSION_LEOP_WAIT_DURATION_SEC - Elapsed.Seconds;
    }
}

static void MISSION_LEOP_WaitUntilComplete(void)
{
    uint32 NextSaveElapsedSec;

    MISSION_Data.LEOPState = MISSION_LEOP_STATE_WAITING;
    (void)MISSION_LEOP_SaveState();
    NextSaveElapsedSec = MISSION_Data.LEOPWaitElapsedSec + MISSION_LEOP_SAVE_INTERVAL_SEC;

    while (!MISSION_Data.LEOPWaitComplete)
    {
        OS_TaskDelay(1000);
        MISSION_LEOP_UpdateWaitStatus();

        if (MISSION_Data.LEOPWaitComplete || MISSION_Data.LEOPWaitElapsedSec >= NextSaveElapsedSec)
        {
            (void)MISSION_LEOP_SaveState();
            NextSaveElapsedSec = MISSION_Data.LEOPWaitElapsedSec + MISSION_LEOP_SAVE_INTERVAL_SEC;
        }
    }
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

    MISSION_Data.LEOPUartDeployTryCount++;
    (void)MISSION_LEOP_SaveState();

    Status = CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
    if (Status != CFE_SUCCESS)
    {
        MISSION_Data.ErrCounter++;
        return Status;
    }

    MISSION_Data.LEOPToEnabled = true;
    MISSION_Data.LEOPState = MISSION_LEOP_STATE_TO_ENABLED;

    return Status;
}

static CFE_Status_t MISSION_LEOP_SendGpioDeployCmd(CFE_MSG_FcnCode_t FcnCode)
{
    CFE_Status_t       Status;
    GPIO_Dep1EnOnCmd_t Cmd;

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

    Status = CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
    if (Status != CFE_SUCCESS)
    {
        MISSION_Data.ErrCounter++;
    }

    return Status;
}

static CFE_Status_t MISSION_LEOP_GpioHigh(void)
{
    CFE_Status_t Status;

    MISSION_Data.LEOPGpioBurnTryCount++;
    (void)MISSION_LEOP_SaveState();

    Status = MISSION_LEOP_SendGpioDeployCmd(GPIO_DEP1_EN_ON_CC);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    OS_TaskDelay(90000);

    Status = MISSION_LEOP_SendGpioDeployCmd(GPIO_DEP1_EN_OFF_CC);
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    MISSION_Data.LEOPGpioDeployIssued = true;
    MISSION_Data.LEOPState = MISSION_LEOP_STATE_GPIO_HIGH_DONE;

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

    Status = MISSION_LEOP_OpenDataFile();
    if (Status != CFE_SUCCESS)
    {
        MISSION_LEOP_UnlockFile();
        return Status;
    }

    OsStatus = OS_lseek(MISSION_Data.LEOPDataHandle, 0, OS_SEEK_SET);
    if (OsStatus < OS_SUCCESS)
    {
        MISSION_LEOP_UnlockFile();
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OsStatus = OS_write(MISSION_Data.LEOPDataHandle, &FileData, sizeof(FileData));
    MISSION_LEOP_UnlockFile();
    if (OsStatus != (int32)sizeof(FileData))
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}

void MISSION_LEOP_Process(void)
{
    CFE_Status_t Status;
    int32        Burn_try_count = 0;

    while (true)
    {
        Status = MISSION_LEOP_LoadState();
        if (Status != CFE_SUCCESS)
        {
            MISSION_Data.ErrCounter++;
            MISSION_LEOP_RetryDelay();
            continue;
        }

        if (MISSION_Data.LEOPState == MISSION_LEOP_STATE_COMPLETE)
        {
            return;
        }

        if (!MISSION_Data.LEOPWaitComplete)
        {
            if (!MISSION_Data.LEOPProcessStarted)
            {
                MISSION_Data.LEOPStartTime = CFE_TIME_GetTime();
                MISSION_Data.LEOPProcessStarted = true;
            }

            MISSION_LEOP_UpdateWaitStatus();
            if (!MISSION_Data.LEOPWaitComplete)
            {
                MISSION_LEOP_WaitUntilComplete();
            }
        }

        MISSION_Data.LEOPState = MISSION_LEOP_STATE_WAIT_COMPLETE;
        (void)MISSION_LEOP_SaveState();

        if (!MISSION_LEOP_UtrxRxBytesIncreased() && Burn_try_count <= MISSION_LEOP_MAX_GPIO_BURN_TRY_COUNT)
        {
            Status = MISSION_LEOP_GpioHigh();
            Burn_try_count++;
            if (Status != CFE_SUCCESS)
            {
                MISSION_Data.ErrCounter++;
                (void)MISSION_LEOP_SaveState();
                MISSION_LEOP_RetryDelay();
                continue;
            }

            (void)MISSION_LEOP_SaveState();
            Status = MISSION_LEOP_EnableTo();

            if (Status != CFE_SUCCESS)
            {
                (void)MISSION_LEOP_SaveState();
                MISSION_LEOP_RetryDelay();
                continue;
            }

            (void)MISSION_LEOP_SaveState();
        
            OS_TaskDelay(3*90*60*1000);
            continue;
        }

        MISSION_Data.LEOPState = MISSION_LEOP_STATE_COMPLETE;
        (void)MISSION_LEOP_SaveState();
        return;
    }
}

void MISSION_LEOP_Task(void)
{
    MISSION_LEOP_Process();
    CFE_ES_ExitChildTask();
}
