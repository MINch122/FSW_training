#include "payuel_cam_child.h"

#include "payuel_cam.h"
#include "payuel_cam_eventids.h"
#include "payuel_cam_msgids.h"
#include "payuel_cam_utils.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/*
 * The child task owns long-running full-image downloads.
 * Command handlers only enqueue work; the child performs the blocking
 * hardware transactions and file I/O in the background.
 */
static CFE_Status_t PAYUEL_CAM_TakeChildMutex(void)
{
    int32 OsStatus;

    /* Guard the child mailbox/state flags because both the SB command side and child task update them. */
    OsStatus = OS_MutSemTake(PAYUEL_CAM_Data.ChildMutex);
    if (OsStatus != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYUEL_CAM_MUTEX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_CAM: Child mutex take failed, RC=%ld", (long)OsStatus);
        PAYUEL_CAM_Data.ErrCounter++;
    }

    return OsStatus;
}

static void PAYUEL_CAM_GiveChildMutex(void)
{
    int32 OsStatus;

    /* Release the mailbox/state lock after queue state inspection or update completes. */
    OsStatus = OS_MutSemGive(PAYUEL_CAM_Data.ChildMutex);
    if (OsStatus != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYUEL_CAM_MUTEX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_CAM: Child mutex give failed, RC=%ld", (long)OsStatus);
        PAYUEL_CAM_Data.ErrCounter++;
    }
}

/* Build a deterministic file name so each downloaded image has a stable location on disk. */
static CFE_Status_t PAYUEL_CAM_MakeDownloadPath(char *Path, size_t PathSize,
                                                uint8_t ImageSlot, uint8_t ImageNumber,
                                                const char *Suffix)
{
    int Result;

    Result = snprintf(Path, PathSize, "./cf/sdcard/cam_img_%u_%u%s",
                      ImageSlot, ImageNumber, Suffix);

    if (Result < 0 || (size_t)Result >= PathSize)
    {
        CFE_EVS_SendEvent(PAYUEL_CAM_FILE_OPEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_CAM: Download path build failed for (%u,%u)",
                          ImageSlot, ImageNumber);
        PAYUEL_CAM_Data.ErrCounter++;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}

static CFE_Status_t PAYUEL_CAM_DownloadImageInChild(uint8_t ImageSlot, uint8_t ImageNumber)
{
    PAYUEL_CAM_ImageMetaInfo_t Meta = {0};
    uint8_t                    ChunkData[PAYUEL_CAM_CHUNK_DATA_SIZE];
    uint8_t                    ReportData[9] = {ImageSlot, ImageNumber, 0};
    CFE_Status_t               status = CFE_SUCCESS;
    uint8_t                    return_type = RPT_RETTYPE_SUCCESS;
    char                       temp_path[PAYUEL_CAM_DOWNLOAD_PATH_MAX];
    char                       final_path[PAYUEL_CAM_DOWNLOAD_PATH_MAX];
    uint32_t                   expected_size = 0;
    uint32_t                   actual_size = 0;
    uint32_t                   actual_crc32 = 0;
    uint16_t                   chunk = 0;
    size_t                     chunk_size;
    int                        fd = -1;
    ssize_t                    written;

    /* Download into a temporary file first so incomplete data never looks final. */
    status = PAYUEL_CAM_MakeDownloadPath(temp_path, sizeof(temp_path), ImageSlot, ImageNumber, ".part");
    if (status != CFE_SUCCESS)
    {
        return_type = RPT_RETTYPE_APP;
        goto report_status;
    }

    /* The final .bin name is published only after every chunk and verification step succeeds. */
    status = PAYUEL_CAM_MakeDownloadPath(final_path, sizeof(final_path), ImageSlot, ImageNumber, ".bin");
    if (status != CFE_SUCCESS)
    {
        return_type = RPT_RETTYPE_APP;
        goto report_status;
    }

    /* Hold the hardware lock for the whole meta+chunk sequence so no other command can disturb ordering. */
    status = PAYUEL_CAM_LockHardware("DownloadImage");
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
    status = PAYUEL_CAM_RequestImageMeta(ImageSlot, ImageNumber, &Meta, &return_type, NULL);
    if (status != CFE_SUCCESS)
    {
        PAYUEL_CAM_UnlockHardware("DownloadImage");
        goto report_status;
    }

    ReportData[0] = Meta.ImageSlot;
    ReportData[1] = Meta.ImageNumber;
    PAYUEL_CAM_WriteU16BE(&ReportData[2], Meta.TotalChunks);
    ReportData[4] = Meta.LastChunkSize;
    PAYUEL_CAM_WriteU32BE(&ReportData[5], Meta.FileCrc32);

    /* Start with a clean temporary file so retries do not append to old contents. */
    fd = open(temp_path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0)
    {
        PAYUEL_CAM_UnlockHardware("DownloadImage");
        CFE_EVS_SendEvent(PAYUEL_CAM_FILE_OPEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_CAM: Failed to open %s", temp_path);
        PAYUEL_CAM_Data.ErrCounter++;
        status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        return_type = RPT_RETTYPE_CFE;
        goto report_status;
    }

    for (chunk = 0; chunk < Meta.TotalChunks; ++chunk)
    {
        /* Metadata tells us whether this is a full-sized chunk or the shorter last chunk. */
        chunk_size = PAYUEL_CAM_GetChunkDataLenFromMeta(&Meta, chunk);
        status = PAYUEL_CAM_RequestImageChunk(ImageSlot, ImageNumber, chunk, chunk_size, ChunkData, &return_type,
                                              NULL);
        if (status != CFE_SUCCESS)
        {
            break;
        }

        /* Persist each verified chunk immediately so RAM usage stays bounded. */
        written = write(fd, ChunkData, chunk_size);
        if (written != (ssize_t)chunk_size)
        {
            CFE_EVS_SendEvent(PAYUEL_CAM_FILE_WRITE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_CAM: File write incomplete (%ld/%u) for %s",
                              (long)written, (unsigned int)chunk_size, temp_path);
            PAYUEL_CAM_Data.ErrCounter++;
            status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            return_type = RPT_RETTYPE_CFE;
            break;
        }

        actual_size += (uint32_t)chunk_size;
    }

    /* Release the shared hardware path before local filesystem verification work begins. */
    PAYUEL_CAM_UnlockHardware("DownloadImage");

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
    expected_size = ((uint32_t)Meta.TotalChunks - 1U) * PAYUEL_CAM_CHUNK_DATA_SIZE + Meta.LastChunkSize;
    if (actual_size != expected_size)
    {
        CFE_EVS_SendEvent(PAYUEL_CAM_FILE_WRITE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_CAM: Downloaded size mismatch actual=%lu expected=%lu",
                          (unsigned long)actual_size, (unsigned long)expected_size);
        PAYUEL_CAM_Data.ErrCounter++;
        status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        return_type = RPT_RETTYPE_APP;
        goto cleanup_temp;
    }

    /* Verify the reconstructed file content matches the CRC advertised by the payload. */
    status = PAYUEL_CAM_ComputeFileCrc32(temp_path, &actual_crc32);
    if (status != CFE_SUCCESS)
    {
        return_type = RPT_RETTYPE_CFE;
        goto cleanup_temp;
    }

    if (actual_crc32 != Meta.FileCrc32)
    {
        CFE_EVS_SendEvent(PAYUEL_CAM_FILE_CRC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_CAM: File CRC32 mismatch actual=0x%08X expected=0x%08X for %s",
                          actual_crc32, Meta.FileCrc32, temp_path);
        PAYUEL_CAM_Data.ErrCounter++;
        status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        return_type = RPT_RETTYPE_HW;
        goto cleanup_temp;
    }

    /* Promote the temporary artifact to the final name only after all checks pass. */
    if (rename(temp_path, final_path) != 0)
    {
        CFE_EVS_SendEvent(PAYUEL_CAM_FILE_RENAME_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_CAM: Failed to rename %s -> %s (errno=%d)",
                          temp_path, final_path, errno);
        PAYUEL_CAM_Data.ErrCounter++;
        status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        return_type = RPT_RETTYPE_CFE;
        goto cleanup_temp;
    }

    CFE_EVS_SendEvent(PAYUEL_CAM_DOWNLOAD_DONE_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "PAYUEL_CAM: Download complete slot=%u image=%u chunks=%u file=%s crc32=0x%08X",
                      Meta.ImageSlot, Meta.ImageNumber, Meta.TotalChunks, final_path, Meta.FileCrc32);
    OS_printf("PAYUEL_CAM: Downloaded image -> %s\n", final_path);
    goto report_status;

cleanup_temp:
    /* Best-effort cleanup of any partial file left by a failed download attempt. */
    unlink(temp_path);

report_status:
    /* Always emit one completion report so the command side can observe final status. */
    PAYUEL_CAM_ReportCmdStatus(CFE_SB_ValueToMsgId(PAYUEL_CAM_CMD_MID), PAYUEL_CAM_DOWNLOAD_IMAGE_CC,
                               status, ReportData, sizeof(ReportData), return_type);
    return status;
}

void PAYUEL_CAM_ChildTask(void)
{
    PAYUEL_CAM_DownloadRequest_t Request;
    int32                       OsStatus;

    CFE_EVS_SendEvent(PAYUEL_CAM_CHILD_INIT_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "PAYUEL_CAM: Child task initialized");

    while (true)
    {
        /* Sleep until the command side posts a new download request into the one-slot mailbox. */
        OsStatus = OS_CountSemTake(PAYUEL_CAM_Data.ChildSemaphore);
        if (OsStatus != OS_SUCCESS)
        {
            CFE_EVS_SendEvent(PAYUEL_CAM_CHILD_TERM_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_CAM: Child semaphore take failed, RC=%ld", (long)OsStatus);
            break;
        }

        if (PAYUEL_CAM_TakeChildMutex() != CFE_SUCCESS)
        {
            break;
        }

        if (!PAYUEL_CAM_Data.DownloadRequestPending)
        {
            PAYUEL_CAM_GiveChildMutex();
            CFE_EVS_SendEvent(PAYUEL_CAM_CHILD_TERM_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_CAM: Child wake-up without pending request");
            PAYUEL_CAM_Data.ErrCounter++;
            continue;
        }

        /* Copy the mailbox locally so the child can work without holding the state mutex for minutes. */
        Request = PAYUEL_CAM_Data.PendingDownload;
        memset(&PAYUEL_CAM_Data.PendingDownload, 0, sizeof(PAYUEL_CAM_Data.PendingDownload));
        PAYUEL_CAM_Data.DownloadRequestPending = false;
        PAYUEL_CAM_Data.DownloadInProgress = true;
        PAYUEL_CAM_GiveChildMutex();

        /* Process the queued request outside the state lock because it can take a long time. */
        (void)PAYUEL_CAM_DownloadImageInChild(Request.ImageSlot, Request.ImageNumber);

        if (PAYUEL_CAM_TakeChildMutex() != CFE_SUCCESS)
        {
            break;
        }

        /* Clear the busy flag only after the whole file download path has completely returned. */
        PAYUEL_CAM_Data.DownloadInProgress = false;
        PAYUEL_CAM_GiveChildMutex();
    }

    /* Mark the child as unusable before exiting so new queue attempts fail cleanly. */
    PAYUEL_CAM_Data.ChildSemaphore = OS_OBJECT_ID_UNDEFINED;
    CFE_ES_ExitChildTask();
}

CFE_Status_t PAYUEL_CAM_ChildInit(void)
{
    CFE_Status_t status;

    /* Counting semaphore works as the child's doorbell: parent gives, child wakes and handles one request. */
    status = OS_CountSemCreate(&PAYUEL_CAM_Data.ChildSemaphore, PAYUEL_CAM_CHILD_SEM_NAME, 0, 0);
    if (status != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYUEL_CAM_CHILD_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_CAM: Child semaphore create failed, RC=%ld", (long)status);
        return status;
    }

    /* Mutex protects the one-slot mailbox and busy flags from parent/child races. */
    status = OS_MutSemCreate(&PAYUEL_CAM_Data.ChildMutex, PAYUEL_CAM_CHILD_MUTEX_NAME, 0);
    if (status != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYUEL_CAM_CHILD_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_CAM: Child mutex create failed, RC=%ld", (long)status);
        return status;
    }

    /* Child task keeps long file I/O and chunk loops off the main SB command processing thread. */
    status = CFE_ES_CreateChildTask(&PAYUEL_CAM_Data.ChildTaskId, PAYUEL_CAM_CHILD_TASK_NAME,
                                    PAYUEL_CAM_ChildTask, NULL, PAYUEL_CAM_CHILD_TASK_STACK_SIZE,
                                    PAYUEL_CAM_CHILD_TASK_PRIORITY, 0);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYUEL_CAM_CHILD_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_CAM: Child task create failed, RC=0x%08lX", (unsigned long)status);
    }

    return status;
}

CFE_Status_t PAYUEL_CAM_QueueDownloadImage(uint8_t ImageSlot, uint8_t ImageNumber, uint8_t *ReturnTypeOut)
{
    CFE_Status_t status = CFE_SUCCESS;
    int32        OsStatus;

    if (ReturnTypeOut != NULL)
    {
        /* Queueing succeeded unless one of the mailbox or OS steps fails below. */
        *ReturnTypeOut = RPT_RETTYPE_SUCCESS;
    }

    if (!OS_ObjectIdDefined(PAYUEL_CAM_Data.ChildSemaphore))
    {
        CFE_EVS_SendEvent(PAYUEL_CAM_CHILD_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_CAM: Child task unavailable");
        PAYUEL_CAM_Data.ErrCounter++;
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_APP;
        }
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    /* Parent side locks the mailbox before checking busy flags and posting a new child request. */
    status = PAYUEL_CAM_TakeChildMutex();
    if (status != CFE_SUCCESS)
    {
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_OSAL;
        }
        return status;
    }

    if (PAYUEL_CAM_Data.DownloadRequestPending || PAYUEL_CAM_Data.DownloadInProgress)
    {
        PAYUEL_CAM_GiveChildMutex();
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_APP;
        }
        CFE_EVS_SendEvent(PAYUEL_CAM_CHILD_BUSY_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_CAM: DownloadImage rejected while download task is busy");
        PAYUEL_CAM_Data.ErrCounter++;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    /* Populate the one-slot mailbox that the child will copy after it wakes up. */
    PAYUEL_CAM_Data.PendingDownload.ImageSlot = ImageSlot;
    PAYUEL_CAM_Data.PendingDownload.ImageNumber = ImageNumber;
    PAYUEL_CAM_Data.DownloadRequestPending = true;
    PAYUEL_CAM_GiveChildMutex();

    /* Ring the child's doorbell after the mailbox is fully populated. */
    OsStatus = OS_CountSemGive(PAYUEL_CAM_Data.ChildSemaphore);
    if (OsStatus != OS_SUCCESS)
    {
        if (ReturnTypeOut != NULL)
        {
            *ReturnTypeOut = RPT_RETTYPE_OSAL;
        }

        if (PAYUEL_CAM_TakeChildMutex() == CFE_SUCCESS)
        {
            /* Roll back the mailbox so the next request starts from a clean state. */
            memset(&PAYUEL_CAM_Data.PendingDownload, 0, sizeof(PAYUEL_CAM_Data.PendingDownload));
            PAYUEL_CAM_Data.DownloadRequestPending = false;
            PAYUEL_CAM_GiveChildMutex();
        }

        CFE_EVS_SendEvent(PAYUEL_CAM_CHILD_TERM_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYUEL_CAM: Child semaphore give failed, RC=%ld", (long)OsStatus);
        PAYUEL_CAM_Data.ErrCounter++;
        return OsStatus;
    }

    CFE_EVS_SendEvent(PAYUEL_CAM_DOWNLOAD_REQ_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "PAYUEL_CAM: Queued download slot=%u image=%u", ImageSlot, ImageNumber);
    return CFE_SUCCESS;
}

bool PAYUEL_CAM_ChildIsBusy(void)
{
    bool Busy = true;

    if (PAYUEL_CAM_TakeChildMutex() != CFE_SUCCESS)
    {
        return true;
    }

    /* Busy means either a request is queued or the child is actively downloading. */
    Busy = PAYUEL_CAM_Data.DownloadRequestPending || PAYUEL_CAM_Data.DownloadInProgress;
    PAYUEL_CAM_GiveChildMutex();
    return Busy;
}
