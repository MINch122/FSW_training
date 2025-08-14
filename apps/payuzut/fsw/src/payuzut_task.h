/**
 * @file
 *
 * Main header file for the PAY UZURO CAM App
 */
#ifndef PAYUZUT_TASK_H
#define PAYUZUT_TASK_H

/**
 * Required Header files,
 */
#include "cfe.h"
#include "cfe_config.h"

#include "payuzut_mission_cfg.h"
#include "payuzut_platform_cfg.h"

#include "payuzut_perfids.h"
#include "payuzut_msgids.h"
#include "payuzut_msg.h"
#include "payuzut_tblstruct.h"

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
    PAYUZUT_HkTlm_t HkTlm;
    PAYUZUT_BcnTlm_t BcnTlm;

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
     * I2C1. For temperature read
     */
    CFE_SRL_IO_Handle_t *Handle;

    /**
     * GPIO Handle
     * PC5. For Thruster control
     */
    CFE_SRL_GPIO_Handle_t *GpioHandle;

    /**
     * PAYUZUT Table Handle
     */
    int TblHandle;

    /**
     * PAYUZUT Child Task Id
     */
    CFE_ES_TaskId_t TaskId;

} PAYUZUT_Data_t;

/**
 * Global Data structure declaration
 */
extern PAYUZUT_Data_t PAYUZUT_Data;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (SAMPLE_APP_Main), these
**       functions are not called from any other source module.
*/
void         PAYUZUT_Main(void);
CFE_Status_t PAYUZUT_Init(void);


#endif
