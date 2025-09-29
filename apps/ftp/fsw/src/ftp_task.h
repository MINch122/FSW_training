/**
 * \file
 *
 * Main header file for the FTP App
 */
#ifndef FTP_TASK_H
#define FTP_TASK_H

/**
 * Required Header files,
 */
#include "cfe.h"
#include "cfe_config.h"

#include "ftp_mission_cfg.h"
#include "ftp_platform_cfg.h"

#include "ftp_perfids.h"
#include "ftp_msgids.h"
#include "ftp_msg.h"
#include "ftp_tblstruct.h"

#include "rpt_interface_cfg.h"


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
    FTP_HkTlm_t HkTlm;

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
     * File transfer Msg
     */
    FTP_File_t Chunk;
    osal_id_t FileId;

    osal_id_t TimerId;
    bool RunFlag;

    /**
     * Report telemetry packet
     */
    FTP_ReportTlm_t Report;

} FTP_Data_t;

/**
 * Global Data structure declaration
 */
extern FTP_Data_t FTP_Data;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (SAMPLE_APP_Main), these
**       functions are not called from any other source module.
*/
void         FTP_Main(void);
CFE_Status_t FTP_Init(void);


#endif