#ifndef PAYUEL_CAM_H
#define PAYUEL_CAM_H

#include "cfe.h"
#include "cfe_config.h"

#include "payuel_cam_mission_cfg.h"
#include "payuel_cam_platform_cfg.h"
#include "payuel_cam_perfids.h"
#include "payuel_cam_msgids.h"
#include "payuel_cam_msg.h"

#include "common_types.h"
#include "rpt_interface_cfg.h"
#include "osapi.h"

#include <stdbool.h>

typedef struct
{
    uint8 ImageSlot;
    uint8 ImageNumber;
} PAYUEL_CAM_DownloadRequest_t;

typedef struct
{
    uint8 CmdCounter;
    uint8 ErrCounter;

    PAYUEL_CAM_BcnTlm_t bcn;
    PAYUEL_CAM_RptTlm_t rpt;

    uint32          RunStatus;
    CFE_SB_PipeId_t CommandPipe;

    char   PipeName[CFE_MISSION_MAX_API_LEN];
    uint16 PipeDepth;

    CFE_ES_TaskId_t ChildTaskId;    /**< Background worker that performs long full-download jobs outside the SB command loop */
    osal_id_t       ChildSemaphore; /**< Wake-up signal for the child task when a new download request is queued */
    osal_id_t       ChildMutex;     /**< Protects PendingDownload/DownloadRequestPending/DownloadInProgress state */
    osal_id_t       ReportMutex;    /**< Prevents concurrent commands and child task from corrupting shared RPT telemetry */
    osal_id_t       HardwareMutex;  /**< Serializes CSP/CAN hardware access so only one transaction flow touches the payload at a time */

    PAYUEL_CAM_DownloadRequest_t PendingDownload;       /**< One-slot mailbox consumed by the child task */
    bool                         DownloadRequestPending; /**< True after the parent queues work and before the child picks it up */
    bool                         DownloadInProgress;     /**< True while the child is actively downloading chunks/files */

    uint8  ImageMetaSlot;
    uint8  ImageMetaNumber;
    uint16 ImageTotalChunks;
    uint8  ImageLastChunkSize;
    uint32 ImageFileCrc32;

    /* Legacy SPI handle kept commented for reference.
     * CFE_SRL_IO_Handle_t *SpiHandle;
     */
} PAYUEL_CAM_Data_t;

extern PAYUEL_CAM_Data_t PAYUEL_CAM_Data;

void         PAYUEL_CAM_Main(void);
CFE_Status_t PAYUEL_CAM_Init(void);

#endif /* PAYUEL_CAM_H */
