/**
 * @file
 *
 * Main header file for the MISSION application
 */

#ifndef MISSION_TASK_H
#define MISSION_TASK_H

/**
 * Required header file
 */
#include "cfe.h"

#include "mission_mission_cfg.h"

#include "mission_perfids.h"
#include "mission_msgids.h"
#include "mission_msg.h"
#include "mission_tbl.h"
#include "utrx_msgids.h"
#include "utrx_msg.h"

/************************************************************************
** Type Definitions
*************************************************************************/

/*
** Global Data
*/
typedef struct {
    uint8 CmdCounter;
    uint8 ErrCounter;

    uint32 RunStatus;

    CFE_SB_PipeId_t CmdPipe; // Command pipe

    char CmdPipeName[CFE_MISSION_MAX_API_LEN];

    uint16 PipeDepth;

    /**
     * MISSION Tlm struct
     */
    MISSION_HkTlm_t HkTlm;
    MISSION_BcnTlm_t BcnTlm;

    osal_id_t            LEOPDataHandle;
    osal_id_t            LEOPFileMutex;
    osal_id_t            LEOPMutex;
    CFE_ES_TaskId_t      LEOPTaskId;
    CFE_TIME_SysTime_t   LEOPStartTime;
    uint32               LEOPWaitElapsedSec;
    uint32               LEOPWaitRemainingSec;
    uint8                LEOPUartDeployTryCount;
    uint8                LEOPGpioBurnTryCount;
    MISSION_LEOP_State_t LEOPState;
    bool                 LEOPProcessStarted;
    bool                 LEOPWaitComplete;
    bool                 LEOPToEnabled;
    bool                 LEOPGpioDeployIssued;
    bool                 LEOPUtrxRxBytesInitialized;
    bool                 LEOPUtrxRxBytesIncreased;
    uint32               LEOPUtrxInitRxBytes;
    uint32               LEOPUtrxRxData;

} MISSION_Data_t;

extern MISSION_Data_t MISSION_Data;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (MISSION_Main), these
**       functions are not called from any other source module.
*/
void MISSION_Main(void);
CFE_Status_t MISSION_Init(void);

#endif
