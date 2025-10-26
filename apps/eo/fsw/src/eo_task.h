/**
 * @file
 *
 * Main header file for the EO application
 */

#ifndef EO_TASK_H
#define EO_TASK_H

/**
 * Required header file
 */
#include "cfe.h"

#include "eo_mission_cfg.h"

#include "eo_perfids.h"
#include "eo_msgids.h"
#include "eo_msg.h"
#include "eo_tbl.h"

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
     * EO Tlm struct
     */
    EO_BcnTlm_t BcnTlm;

    /**
     * EO Current Phase
     */
    osal_id_t CurrentStepHandle;
    EO_CurrentStep_t CurrentStep;

    /* EPS data */
    uint16_t Vbatt;     /*<\brief Battery Voltage */
    uint16_t CurIn[2];  /*<\brief Charged Current */

    uint8_t Output[8];  /* <\brief Output Channel status */

    /* SANT data */
    uint8_t State;
    uint8_t Status;
    uint8_t BurnTimeLeft;
    uint8_t BurnTries;

    /* ADCS data */
    uint8_t MagDeployPinState;
    uint8_t MagBurnPinState;
    uint8_t MagDeployTimeout;

    /* RPT data */
    CFE_TIME_SysTime_t Epoch; /* <\brief Received from RPT at beginning */
    

    /* Mut Sem object */
    osal_id_t EPS_SemId;
    osal_id_t SANT_SemId;
    osal_id_t ADCS_SemId;

    osal_id_t EOMutex;
    
    /* Child Task ID */
    CFE_ES_TaskId_t ChildTaskId;

    volatile bool WaitingEPS;
    volatile bool WaitingSANT;
    volatile bool WaitingADCS;

} EO_Data_t;

extern EO_Data_t EO_Data;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (EO_Main), these
**       functions are not called from any other source module.
*/
void EO_Main(void);
CFE_Status_t EO_Init(void);

#endif