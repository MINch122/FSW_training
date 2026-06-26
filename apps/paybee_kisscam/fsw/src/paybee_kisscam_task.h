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

#include "paybee_kisscam_mission_cfg.h"
#include "paybee_kisscam_platform_cfg.h"

#include "paybee_kisscam_perfids.h"
#include "paybee_kisscam_msgids.h"
#include "paybee_kisscam_msg.h"
#include "paybee_kisscam_tblstruct.h"

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
    // paybee_kisscam_BcnTlm_t BcnTlm;

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
    paybee_kisscam_ReportTlm_t *ReprotTlm;

    /**
     * paybee_kisscam Table Handle
     * Not use cFE TBL service
     */
    int TblHandle;

    paybee_kisscam_Memory_Status_t MemSlotStatus;

    /**
     * Arguments of Download Task Arguments
     */
    // CFE_ES_TaskId_t DownTaskId;
    // paybee_kisscam_DownloadAll_Payload_t DownTaskArg;  /* <\brief Arguments used `Download All` Command */
    // uint16_t ErrCount;                          /* <\brief Error count during `Download All` Command */

    osal_id_t MutId;

} paybee_kisscam_Data_t;

/**
 * Global Data structure declaration
 */
extern paybee_kisscam_Data_t paybee_kisscam_Data;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (SAMPLE_APP_Main), these
**       functions are not called from any other source module.
*/
void         paybee_kisscam_Main(void);
CFE_Status_t paybee_kisscam_Init(void);


#endif /* paybee_kisscam_TASK_H */