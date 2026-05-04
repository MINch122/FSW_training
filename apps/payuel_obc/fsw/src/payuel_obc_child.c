#include "payuel_obc_child.h"

#include "payuel_obc.h"
#include "payuel_obc_eventids.h"
#include "payuel_obc_msgids.h"
#include "payuel_obc_utils.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/*
 * The child task owns long-running full-file downloads.
 * Command handlers only enqueue image or sensor requests; the child performs
 * the blocking hardware transactions and file I/O in the background.
 */
static CFE_Status_t PAYUEL_OBC_TakeChildMutex(void)
{
    int32 OsStatus;

    /* Guard the child mailbox/state flags because both the SB command side and child task update them. */
    OsStatus = OS_MutSemTake(PAYUEL_OBC_Data.ChildMutex);
    if (OsStatus != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYUEL_OBC_MUTEX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Child mutex take failed, RC=%ld", (long)OsStatus);
        PAYUEL_OBC_Data.ErrCounter++;
    }

    return OsStatus;
}

static void PAYUEL_OBC_GiveChildMutex(void)
{
    int32 OsStatus;

    /* Release the mailbox/state lock after queue state inspection or update completes. */
    OsStatus = OS_MutSemGive(PAYUEL_OBC_Data.ChildMutex);
    if (OsStatus != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYUEL_OBC_MUTEX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Child mutex give failed, RC=%ld", (long)OsStatus);
        PAYUEL_OBC_Data.ErrCounter++;
    }
}

/* Build a deterministic file name so each download lands in a stable location on disk. */
static CFE_Status_t PAYUEL_OBC_MakeDownloadPath(char *Path, size_t PathSize,
                                                const char *Prefix,
                                                uint8_t Slot, uint8_t Number,
                                                const char *Suffix)
{
    int Result;

    Result = snprintf(Path, PathSize, "./cf/sdcard/%s_%u_%u%s", Prefix, Slot, Number, Suffix);
    if (Result < 0 || (size_t)Result >= PathSize)
    {
        CFE_EVS_SendEvent(PAYUEL_OBC_FILE_OPEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Download path build failed for %s (%u,%u)",
                          Prefix, Slot, Number);
        PAYUEL_OBC_Data.ErrCounter++;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}

static CFE_Status_t PAYUEL_OBC_DownloadImageInChild(uint8_t CameraID, uint8_t ImageNumber)
{
    PAYUEL_OBC_ImageMetaInfo_t Meta = {0};
    uint8_t                    ChunkData[PAYUEL_OBC_CHUNK_DATA_SIZE];
    uint8_t                    ReportData[9] = {CameraID, ImageNumber, 0};
    CFE_Status_t               status = CFE_SUCCESS;
    uint8_t                    return_type = RPT_RETTYPE_SUCCESS;
    char                       temp_path[PAYUEL_OBC_DOWNLOAD_PATH_MAX];
    char                       final_path[PAYUEL_OBC_DOWNLOAD_PATH_MAX];
    uint32_t                   expected_size = 0;
    uint32_t                   actual_size = 0;
    uint32_t                   actual_crc32 = 0;
    uint16_t                   chunk = 0;
    size_t                     chunk_size;
    int                        fd = -1;
    ssize_t                    written;

    /* Download into a temporary file first so incomplete data never looks final. */
    status = PAYUEL_OBC_MakeDownloadPath(temp_path, sizeof(temp_path), "uel_img", CameraID, ImageNumber, ".part");
    if (status != CFE_SUCCESS)
    {
        return_type = RPT_RETTYPE_APP;
        goto report_status;
    }

    /* The final .bin name is published only after every chunk and verification step succeeds. */
    status = PAYUEL_OBC_MakeDownloadPath(final_path, sizeof(final_path), "uel_img", CameraID, ImageNumber, ".bin");
    if (status != CFE_SUCCESS)
    {
        return_type = RPT_RETTYPE_APP;
        goto report_status;
    }

    /* Hold the hardware lock for the whole meta+chunk sequence so no other command can disturb ordering. */
    status = PAYUEL_OBC_LockHardware("DownloadImage");
    if (status != CFE_SUCCESS)
    {
        return_type = RPT_RETTYPE_OSAL;
        goto report_status;
    }

    /*
     * First ask the payload for metadata.
     * That tells us how many chunks to fetch, how large the last chunk is,
     * and what CRC32 the finished file must match.
     */
    status = PAYUEL_OBC_RequestImageMeta(CameraID, ImageNumber, &Meta, &return_type, NULL);
    if (status != CFE_SUCCESS)
    {
        PAYUEL_OBC_UnlockHardware("DownloadImage");
        goto report_status;
    }

    ReportData[0] = Meta.CameraID;
    ReportData[1] = Meta.ImageIndex;
    PAYUEL_OBC_WriteU16BE(&ReportData[2], Meta.ChunkCount);
    ReportData[4] = Meta.LastChunkSize;
    PAYUEL_OBC_WriteU32BE(&ReportData[5], Meta.ImageFileCRC32);

    /* Start with a clean temporary file so retries do not append to old contents. */
    fd = open(temp_path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0)
    {
        PAYUEL_OBC_UnlockHardware("DownloadImage");
        CFE_EVS_SendEvent(PAYUEL_OBC_FILE_OPEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Failed to open %s", temp_path);
        PAYUEL_OBC_Data.ErrCounter++;
        status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        return_type = RPT_RETTYPE_CFE;
        goto report_status;
    }

    for (chunk = 0; chunk < Meta.ChunkCount; ++chunk)
    {
        /* Metadata tells us whether this is a full-sized chunk or the shorter last chunk. */
        chunk_size = PAYUEL_OBC_GetChunkDataLenFromImageMeta(&Meta, chunk);
        status = PAYUEL_OBC_RequestImageChunk(CameraID, ImageNumber, chunk, chunk_size, ChunkData, &return_type,
                                              NULL);
        if (status != CFE_SUCCESS)
        {
            break;
        }

        /* Persist each verified chunk immediately so RAM usage stays bounded. */
        written = write(fd, ChunkData, chunk_size);
        if (written != (ssize_t)chunk_size)
        {
            CFE_EVS_SendEvent(PAYUEL_OBC_FILE_WRITE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_OBC: File write incomplete (%ld/%u) for %s",
                              (long)written, (unsigned int)chunk_size, temp_path);
            PAYUEL_OBC_Data.ErrCounter++;
            status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            return_type = RPT_RETTYPE_CFE;
            break;
        }

        actual_size += (uint32_t)chunk_size;
        /* Progress logging helps operators see that long image downloads are still advancing. */
        OS_printf("PAYUEL_OBC: DownloadImage progress camera=0x%02X image=%u chunk=%u/%u\n",
                  CameraID, ImageNumber, (unsigned int)chunk + 1U, Meta.ChunkCount);
    }

    /* Release the shared hardware path before local filesystem verification work begins. */
    PAYUEL_OBC_UnlockHardware("DownloadImage");

    if (fd >= 0)
    {
        close(fd);
        fd = -1;
    }

    if (status != CFE_SUCCESS)
    {
        goto cleanup_temp;
    }

    /* Recompute the expected final size from metadata and compare with what we actually wrote. */
    expected_size = ((uint32_t)Meta.ChunkCount - 1U) * PAYUEL_OBC_CHUNK_DATA_SIZE + Meta.LastChunkSize;
    if (actual_size != expected_size)
    {
        CFE_EVS_SendEvent(PAYUEL_OBC_FILE_WRITE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Downloaded image size mismatch actual=%lu expected=%lu",
                          (unsigned long)actual_size, (unsigned long)expected_size);
        PAYUEL_OBC_Data.ErrCounter++;
        status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        return_type = RPT_RETTYPE_APP;
        goto cleanup_temp;
    }

    /* Verify the reconstructed file content matches the CRC advertised by the payload. */
    status = PAYUEL_OBC_ComputeFileCrc32(temp_path, &actual_crc32);
    if (status != CFE_SUCCESS)
    {
        return_type = RPT_RETTYPE_CFE;
        goto cleanup_temp;
    }

    if (actual_crc32 != Meta.ImageFileCRC32)
    {
        CFE_EVS_SendEvent(PAYUEL_OBC_FILE_CRC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Image file CRC32 mismatch actual=0x%08X expected=0x%08X for %s",
                          actual_crc32, Meta.ImageFileCRC32, temp_path);
        PAYUEL_OBC_Data.ErrCounter++;
        status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        return_type = RPT_RETTYPE_HW;
        goto cleanup_temp;
    }

    /* Promote the temporary artifact to the final name only after all checks pass. */
    if (rename(temp_path, final_path) != 0)
    {
        CFE_EVS_SendEvent(PAYUEL_OBC_FILE_RENAME_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Failed to rename %s -> %s (errno=%d)",
                          temp_path, final_path, errno);
        PAYUEL_OBC_Data.ErrCounter++;
        status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        return_type = RPT_RETTYPE_CFE;
        goto cleanup_temp;
    }

    CFE_EVS_SendEvent(PAYUEL_OBC_DOWNLOAD_DONE_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "PAYUEL_OBC: Image download complete camera=0x%02X image=%u chunks=%u file=%s crc32=0x%08X",
                      Meta.CameraID, Meta.ImageIndex, Meta.ChunkCount, final_path, Meta.ImageFileCRC32);
    OS_printf("PAYUEL_OBC: Downloaded image -> %s\n", final_path);
    goto report_status;

cleanup_temp:
    /* Best-effort cleanup of any partial file left by a failed download attempt. */
    unlink(temp_path);

report_status:
    /* Always emit one completion report so the command side can observe final status. */
    PAYUEL_OBC_ReportCmdStatus(CFE_SB_ValueToMsgId(PAYUEL_OBC_CMD_MID), PAYUEL_OBC_DOWNLOAD_IMAGE_CC,
                               status, ReportData, sizeof(ReportData), return_type);
    return status;
}

static CFE_Status_t PAYUEL_OBC_DownloadSensorInChild(uint8_t DataSlot, uint8_t DataNumber)
{
    PAYUEL_OBC_SensorMetaInfo_t Meta = {0};
    uint8_t                     ChunkData[PAYUEL_OBC_CHUNK_DATA_SIZE];
    uint8_t                     ReportData[9] = {DataSlot, DataNumber, 0};
    CFE_Status_t                status = CFE_SUCCESS;
    uint8_t                     return_type = RPT_RETTYPE_SUCCESS;
    char                        temp_path[PAYUEL_OBC_DOWNLOAD_PATH_MAX];
    char                        final_path[PAYUEL_OBC_DOWNLOAD_PATH_MAX];
    uint32_t                    expected_size = 0;
    uint32_t                    actual_size = 0;
    uint32_t                    actual_crc32 = 0;
    uint16_t                    chunk = 0;
    size_t                      chunk_size;
    int                         fd = -1;
    ssize_t                     written;

    /* Download into a temporary file first so incomplete data never looks final. */
    status = PAYUEL_OBC_MakeDownloadPath(temp_path, sizeof(temp_path), "sens", DataSlot, DataNumber, ".part");
    if (status != CFE_SUCCESS)
    {
        return_type = RPT_RETTYPE_APP;
        goto report_status;
    }

    /* The final .bin name is published only after every chunk and size checks succeed. */
    status = PAYUEL_OBC_MakeDownloadPath(final_path, sizeof(final_path), "sens", DataSlot, DataNumber, ".bin");
    if (status != CFE_SUCCESS)
    {
        return_type = RPT_RETTYPE_APP;
        goto report_status;
    }

    /* Hold the hardware lock for the whole meta+chunk sequence so no other command can disturb ordering. */
    status = PAYUEL_OBC_LockHardware("DownloadSensor");
    if (status != CFE_SUCCESS)
    {
        return_type = RPT_RETTYPE_OSAL;
        goto report_status;
    }

    /*
     * First ask the payload for sensor metadata.
     * That tells us how many chunks to fetch and how large the last chunk is.
     */
    status = PAYUEL_OBC_RequestSensorMeta(DataSlot, DataNumber, &Meta, &return_type, NULL);
    if (status != CFE_SUCCESS)
    {
        PAYUEL_OBC_UnlockHardware("DownloadSensor");
        goto report_status;
    }

    ReportData[0] = Meta.DataSlot;
    ReportData[1] = Meta.DataIndex;
    PAYUEL_OBC_WriteU16BE(&ReportData[2], Meta.ChunkCount);
    ReportData[4] = Meta.LastChunkSize;
    PAYUEL_OBC_WriteU32BE(&ReportData[5], Meta.BinFileCRC32);

    /* Start with a clean temporary file so retries do not append to old contents. */
    fd = open(temp_path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0)
    {
        PAYUEL_OBC_UnlockHardware("DownloadSensor");
        CFE_EVS_SendEvent(PAYUEL_OBC_FILE_OPEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Failed to open %s", temp_path);
        PAYUEL_OBC_Data.ErrCounter++;
        status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        return_type = RPT_RETTYPE_CFE;
        goto report_status;
    }

    for (chunk = 0; chunk < Meta.ChunkCount; ++chunk)
    {
        /* Metadata tells us whether this is a full-sized chunk or the shorter last chunk. */
        chunk_size = PAYUEL_OBC_GetChunkDataLenFromSensorMeta(&Meta, chunk);
        status = PAYUEL_OBC_RequestSensorChunk(DataSlot, DataNumber, chunk, chunk_size, ChunkData, &return_type,
                                               NULL);
        if (status != CFE_SUCCESS)
        {
            break;
        }

        /* Persist each verified chunk immediately so RAM usage stays bounded. */
        written = write(fd, ChunkData, chunk_size);
        if (written != (ssize_t)chunk_size)
        {
            CFE_EVS_SendEvent(PAYUEL_OBC_FILE_WRITE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_OBC: Sensor file write incomplete (%ld/%u) for %s",
                              (long)written, (unsigned int)chunk_size, temp_path);
            PAYUEL_OBC_Data.ErrCounter++;
            status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            return_type = RPT_RETTYPE_CFE;
            break;
        }

        actual_size += (uint32_t)chunk_size;
        /* Progress logging helps operators see that long sensor downloads are still advancing. */
        OS_printf("PAYUEL_OBC: DownloadSensor progress slot=%u data=%u chunk=%u/%u\n",
                  DataSlot, DataNumber, (unsigned int)chunk + 1U, Meta.ChunkCount);
    }

    /* Release the shared hardware path before local filesystem verification work begins. */
    PAYUEL_OBC_UnlockHardware("DownloadSensor");

    if (fd >= 0)
    {
        close(fd);
        fd = -1;
    }

    if (status != CFE_SUCCESS)
    {
        goto cleanup_temp;
    }

    /* Recompute the expected final size from metadata and compare with what we actually wrote. */
    expected_size = ((uint32_t)Meta.ChunkCount - 1U) * PAYUEL_OBC_CHUNK_DATA_SIZE + Meta.LastChunkSize;
    if (actual_size != expected_size)
    {
        CFE_EVS_SendEvent(PAYUEL_OBC_FILE_WRITE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Downloaded sensor size mismatch actual=%lu expected=%lu",
                          (unsigned long)actual_size, (unsigned long)expected_size);
        PAYUEL_OBC_Data.ErrCounter++;
        status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        return_type = RPT_RETTYPE_APP;
        goto cleanup_temp;
    }

    status = PAYUEL_OBC_ComputeFileCrc32(temp_path, &actual_crc32);
    if (status != CFE_SUCCESS)
    {
        return_type = RPT_RETTYPE_CFE;
        goto cleanup_temp;
    }

    if (actual_crc32 != Meta.BinFileCRC32)
    {
        CFE_EVS_SendEvent(PAYUEL_OBC_FILE_CRC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Sensor file CRC32 mismatch actual=0x%08X expected=0x%08X for %s",
                          actual_crc32, Meta.BinFileCRC32, temp_path);
        PAYUEL_OBC_Data.ErrCounter++;
        status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        return_type = RPT_RETTYPE_HW;
        goto cleanup_temp;
    }

    /* Promote the temporary artifact to the final name only after all checks pass. */
    if (rename(temp_path, final_path) != 0)
    {
        CFE_EVS_SendEvent(PAYUEL_OBC_FILE_RENAME_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Failed to rename %s -> %s (errno=%d)",
                          temp_path, final_path, errno);
        PAYUEL_OBC_Data.ErrCounter++;
        status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        return_type = RPT_RETTYPE_CFE;
        goto cleanup_temp;
    }

    CFE_EVS_SendEvent(PAYUEL_OBC_DOWNLOAD_DONE_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "PAYUEL_OBC: Sensor download complete slot=%u data=%u chunks=%u file=%s crc32=0x%08X",
                      Meta.DataSlot, Meta.DataIndex, Meta.ChunkCount, final_path, Meta.BinFileCRC32);
    OS_printf("PAYUEL_OBC: Downloaded sensor data -> %s\n", final_path);
    goto report_status;

cleanup_temp:
    /* Best-effort cleanup of any partial file left by a failed download attempt. */
    unlink(temp_path);

report_status:
    /* Always emit one completion report so the command side can observe final status. */
    PAYUEL_OBC_ReportCmdStatus(CFE_SB_ValueToMsgId(PAYUEL_OBC_CMD_MID), PAYUEL_OBC_DOWNLOAD_SENSOR_CC,
                               status, ReportData, sizeof(ReportData), return_type);
    return status;
}

void PAYUEL_OBC_ChildTask(void)
{
    PAYUEL_OBC_DownloadRequest_t Request;
    int32                        OsStatus;

    CFE_EVS_SendEvent(PAYUEL_OBC_CHILD_INIT_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "PAYUEL_OBC: Child task initialized");

    while (true)
    {
        /* Sleep until the command side posts a new download request into the one-slot mailbox. */
        OsStatus = OS_CountSemTake(PAYUEL_OBC_Data.ChildSemaphore);
        if (OsStatus != OS_SUCCESS)
        {
            CFE_EVS_SendEvent(PAYUEL_OBC_CHILD_TERM_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_OBC: Child semaphore take failed, RC=%ld", (long)OsStatus);
            break;
        }

        if (PAYUEL_OBC_TakeChildMutex() != CFE_SUCCESS)
        {
            break;
        }

        if (!PAYUEL_OBC_Data.DownloadRequestPending)
        {
            PAYUEL_OBC_GiveChildMutex();
            CFE_EVS_SendEvent(PAYUEL_OBC_CHILD_TERM_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_OBC: Child wake-up without pending request");
            PAYUEL_OBC_Data.ErrCounter++;
            continue;
        }

        /* Copy the mailbox locally so the child can work without holding the state mutex for minutes. */
        Request = PAYUEL_OBC_Data.PendingDownload;
        memset(&PAYUEL_OBC_Data.PendingDownload, 0, sizeof(PAYUEL_OBC_Data.PendingDownload));
        PAYUEL_OBC_Data.DownloadRequestPending = false;
        PAYUEL_OBC_Data.DownloadInProgress = true;
        PAYUEL_OBC_GiveChildMutex();

        /* Dispatch based on request type because the same child handles image and sensor downloads. */
        if (Request.Type == PAYUEL_OBC_DOWNLOAD_REQ_IMAGE)
        {
            (void)PAYUEL_OBC_DownloadImageInChild(Request.Slot, Request.Number);
        }
        else if (Request.Type == PAYUEL_OBC_DOWNLOAD_REQ_SENSOR)
        {
            (void)PAYUEL_OBC_DownloadSensorInChild(Request.Slot, Request.Number);
        }
        else
        {
            CFE_EVS_SendEvent(PAYUEL_OBC_CHILD_TERM_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_OBC: Unknown child request type %u", Request.Type);
            PAYUEL_OBC_Data.ErrCounter++;
        }

        if (PAYUEL_OBC_TakeChildMutex() != CFE_SUCCESS)
        {
            break;
        }

        /* Clear the busy flag only after the whole file download path has completely returned. */
        PAYUEL_OBC_Data.DownloadInProgress = false;
        PAYUEL_OBC_GiveChildMutex();
    }

    /* Mark the child as unusable before exiting so new queue attempts fail cleanly. */
    PAYUEL_OBC_Data.ChildSemaphore = OS_OBJECT_ID_UNDEFINED;
    CFE_ES_ExitChildTask();
}

CFE_Status_t PAYUEL_OBC_ChildInit(void)
{
    CFE_Status_t status;

    /* Counting semaphore works as the child's doorbell: parent gives, child wakes and handles one request. */
    status = OS_CountSemCreate(&PAYUEL_OBC_Data.ChildSemaphore, PAYUEL_OBC_CHILD_SEM_NAME, 0, 0);
    if (status != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYUEL_OBC_CHILD_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Child semaphore create failed, RC=%ld", (long)status);
        return status;
    }

    /* Mutex protects the one-slot mailbox and busy flags from parent/child races. */
    status = OS_MutSemCreate(&PAYUEL_OBC_Data.ChildMutex, PAYUEL_OBC_CHILD_MUTEX_NAME, 0);
    if (status != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYUEL_OBC_CHILD_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Child mutex create failed, RC=%ld", (long)status);
        return status;
    }

    /* Child task keeps long file I/O and chunk loops off the main SB command processing thread. */
    status = CFE_ES_CreateChildTask(&PAYUEL_OBC_Data.ChildTaskId, PAYUEL_OBC_CHILD_TASK_NAME,
                                    PAYUEL_OBC_ChildTask, NULL, PAYUEL_OBC_CHILD_TASK_STACK_SIZE,
                                    PAYUEL_OBC_CHILD_TASK_PRIORITY, 0);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYUEL_OBC_CHILD_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Child task create failed, RC=0x%08lX", (unsigned long)status);
    }

    return status;
}

static CFE_Status_t PAYUEL_OBC_QueueDownload(uint8_t RequestType, uint8_t Slot, uint8_t Number,
                                             const char *CmdName, uint8_t *ReturnTypeOut)
{
    CFE_Status_t status = CFE_SUCCESS;
    int32        OsStatus;

    if (ReturnTypeOut != NULL)
    {
        /* Queueing succeeded unless one of the mailbox or OS steps fails below. */
        *ReturnTypeOut = RPT_RETTYPE_SUCCESS;
    }

    if (!OS_ObjectIdDefined(PAYUEL_OBC_Data.ChildSemaphore))
    {
        CFE_EVS_SendEvent(PAYUEL_OBC_CHILD_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Child task unavailable");
        PAYUEL_OBC_Data.ErrCounter++;
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_APP;
        }
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    /* Parent side locks the mailbox before checking busy flags and posting a new child request. */
    status = PAYUEL_OBC_TakeChildMutex();
    if (status != CFE_SUCCESS)
    {
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_OSAL;
        }
        return status;
    }

    if (PAYUEL_OBC_Data.DownloadRequestPending || PAYUEL_OBC_Data.DownloadInProgress)
    {
        PAYUEL_OBC_GiveChildMutex();
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_APP;
        }
        CFE_EVS_SendEvent(PAYUEL_OBC_CHILD_BUSY_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: %s rejected while download task is busy", CmdName);
        PAYUEL_OBC_Data.ErrCounter++;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    /* Populate the one-slot mailbox that the child will copy after it wakes up. */
    PAYUEL_OBC_Data.PendingDownload.Type = RequestType;
    PAYUEL_OBC_Data.PendingDownload.Slot = Slot;
    PAYUEL_OBC_Data.PendingDownload.Number = Number;
    PAYUEL_OBC_Data.DownloadRequestPending = true;
    PAYUEL_OBC_GiveChildMutex();

    /* Ring the child's doorbell after the mailbox is fully populated. */
    OsStatus = OS_CountSemGive(PAYUEL_OBC_Data.ChildSemaphore);
    if (OsStatus != OS_SUCCESS)
    {
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_OSAL;
        }

        if (PAYUEL_OBC_TakeChildMutex() == CFE_SUCCESS)
        {
            /* Roll back the mailbox so the next request starts from a clean state. */
            memset(&PAYUEL_OBC_Data.PendingDownload, 0, sizeof(PAYUEL_OBC_Data.PendingDownload));
            PAYUEL_OBC_Data.DownloadRequestPending = false;
            PAYUEL_OBC_GiveChildMutex();
        }

        CFE_EVS_SendEvent(PAYUEL_OBC_CHILD_TERM_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_OBC: Child semaphore give failed, RC=%ld", (long)OsStatus);
        PAYUEL_OBC_Data.ErrCounter++;
        return OsStatus;
    }

    CFE_EVS_SendEvent(PAYUEL_OBC_DOWNLOAD_REQ_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "PAYUEL_OBC: Queued %s slot=%u number=%u",
                      CmdName, Slot, Number);
    return CFE_SUCCESS;
}

CFE_Status_t PAYUEL_OBC_QueueDownloadImage(uint8_t CameraID, uint8_t ImageNumber, uint8_t *ReturnTypeOut)
{
    return PAYUEL_OBC_QueueDownload(PAYUEL_OBC_DOWNLOAD_REQ_IMAGE, CameraID, ImageNumber,
                                    "DownloadImage", ReturnTypeOut);
}

CFE_Status_t PAYUEL_OBC_QueueDownloadSensor(uint8_t DataSlot, uint8_t DataNumber, uint8_t *ReturnTypeOut)
{
    return PAYUEL_OBC_QueueDownload(PAYUEL_OBC_DOWNLOAD_REQ_SENSOR, DataSlot, DataNumber,
                                    "DownloadSensor", ReturnTypeOut);
}

bool PAYUEL_OBC_ChildIsBusy(void)
{
    bool Busy = true;

    if (PAYUEL_OBC_TakeChildMutex() != CFE_SUCCESS)
    {
        return true;
    }

    /* Busy means either a request is queued or the child is actively downloading. */
    Busy = PAYUEL_OBC_Data.DownloadRequestPending || PAYUEL_OBC_Data.DownloadInProgress;
    PAYUEL_OBC_GiveChildMutex();
    return Busy;
}
