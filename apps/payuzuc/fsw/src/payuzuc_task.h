/**
 * @file
 *
 * Main header file for the PAY UZURO CAM App
 */
#ifndef PAYUZUROC_TASK_H
#define PAYUZUROC_TASK_H

/**
 * Required Header files,
 */
#include "cfe.h"
#include "cfe_config.h"

#include "payuzuc_mission_cfg.h"
#include "payuzuc_platform_cfg.h"

#include "payuzuc_perfids.h"
#include "payuzuc_msgids.h"
#include "payuzuc_msg.h"
#include "payuzuc_tblstruct.h"

/************************************************************************
** Type Definitions
*************************************************************************/

/*
** Global Data
*/
typedef struct {
    uint8 CmdCounter;
    uint8 ErrCounter;
    uint8 DeviceErrCounter;

    /**
     * Houskeeping telemetry packet
     */
    PAYUZUC_BcnTlm_t HkTlm;

    /*
    ** Run Status variable used in the main processing loop
    */
    uint32 RunStatus;

    /*
    ** Operational data (not reported in housekeeping)...
    */
    CFE_SB_PipeId_t CommandPipe;

    /*
    ** Initialization data (not reported in housekeeping)...
    */
    char   PipeName[CFE_MISSION_MAX_API_LEN];
    uint16 PipeDepth;

    /**
     * Serial Handle
     */
    CFE_SRL_IO_Handle_t *Handle;
    
    /**
     * Report Data for RPT
     */
    PAYUZUC_ReportTlm_t *ReprotTlm;

    /**
     * PAYUZUC Table Handle
     * Not use cFE TBL service
     */
    int TblHandle;

    PAYUZUC_Memory_Status_t MemSlotStatus;

    /**
     * Arguments of Download Task Arguments
     */
    PAYUZUC_DownloadAll_Payload_t DownTaskArg;

} PAYUZUC_Data_t;

/**
 * Global Data structure declaration
 */
extern PAYUZUC_Data_t PAYUZUC_Data;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (SAMPLE_APP_Main), these
**       functions are not called from any other source module.
*/
void         PAYUZUC_Main(void);
CFE_Status_t PAYUZUC_Init(void);


#endif /* PAYUZUC_TASK_H */