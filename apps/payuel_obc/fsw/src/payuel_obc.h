#ifndef PAYUEL_OBC_H
#define PAYUEL_OBC_H

#include "cfe.h"
#include "cfe_config.h"

#include "payuel_obc_mission_cfg.h"
#include "payuel_obc_platform_cfg.h"
#include "payuel_obc_perfids.h"
#include "payuel_obc_msgids.h"
#include "payuel_obc_msg.h"

#include "common_types.h"
#include "rpt_interface_cfg.h"
#include "osapi.h"

#include <stdbool.h>

#define PAYUEL_OBC_DOWNLOAD_REQ_NONE   0U
#define PAYUEL_OBC_DOWNLOAD_REQ_IMAGE  1U
#define PAYUEL_OBC_DOWNLOAD_REQ_SENSOR 2U

typedef struct
{
    uint8 Type;
    uint8 Slot;
    uint8 Number;
} PAYUEL_OBC_DownloadRequest_t;

typedef struct
{
    uint8 CmdCounter;
    uint8 ErrCounter;

    PAYUEL_OBC_ObcBcnTlm_t obc_bcn;   /**< 0x20 UEL OBC Beacon telemetry */
    PAYUEL_OBC_RptTlm_t    rpt;

    uint32          RunStatus;
    CFE_SB_PipeId_t CommandPipe;

    char   PipeName[CFE_MISSION_MAX_API_LEN];
    uint16 PipeDepth;

    CFE_ES_TaskId_t ChildTaskId;    /**< Background worker that performs long full-download jobs outside the SB command loop */
    osal_id_t       ChildSemaphore; /**< Wake-up signal for the child task when a new download request is queued */
    osal_id_t       ChildMutex;     /**< Protects PendingDownload/DownloadRequestPending/DownloadInProgress state */
    osal_id_t       ReportMutex;    /**< Prevents concurrent commands and child task from corrupting shared RPT telemetry */
    osal_id_t       HardwareMutex;  /**< Serializes CSP/CAN hardware access so only one transaction flow touches the payload at a time */

    PAYUEL_OBC_DownloadRequest_t PendingDownload;       /**< One-slot mailbox consumed by the child task */
    bool                         DownloadRequestPending; /**< True after the parent queues work and before the child picks it up */
    bool                         DownloadInProgress;     /**< True while the child is actively downloading chunks/files */

    uint8  ImageMetaSlot;
    uint8  ImageMetaNumber;
    uint16 ImageTotalChunks;
    uint8  ImageLastChunkSize;
    uint32 ImageFileCrc32;

    uint8  SensorMetaSlot;
    uint8  SensorMetaNumber;
    uint16 SensorTotalChunks;
    uint8  SensorLastChunkSize;
    uint32 SensorFileCrc32;

    /* CFE_SRL_IO_Handle_t *SpiHandle; */ /**< Legacy SPI handle kept commented for chunk download reference */
} PAYUEL_OBC_Data_t;

extern PAYUEL_OBC_Data_t PAYUEL_OBC_Data;

void         PAYUEL_OBC_Main(void);
CFE_Status_t PAYUEL_OBC_Init(void);

#endif /* PAYUEL_OBC_H */
