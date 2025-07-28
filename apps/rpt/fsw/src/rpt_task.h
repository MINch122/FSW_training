/**
 * @file
 *
 * Main header file for the RPT application
 */

#ifndef RPT_TASK_H
#define RPT_TASK_H

/**
 * Required header file
 */
#include "cfe.h"

#include "rpt_mission_cfg.h"

#include "rpt_perfids.h"
#include "rpt_msgids.h"
#include "rpt_msg.h"
#include "rpt_tbl.h"

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

    CFE_SB_PipeId_t CmdPipe; // TC pipe
    CFE_SB_PipeId_t RptPipe; // Report pipe
    CFE_SB_PipeId_t CritPipe; // Critical pipe

    char CmdPipeName[CFE_MISSION_MAX_API_LEN];
    char RptPipeName[CFE_MISSION_MAX_API_LEN];
    char CritPipeName[CFE_MISSION_MAX_API_LEN];

    uint16 PipeDepth;

    RPT_Table_t *SubsTblPtr;
    CFE_TBL_Handle_t SubsTblHandle;

    /**
     * RPT Tlm struct
     */
    RPT_HkTlm_t HkTlm;

    /**
     * RPT Report Queue
     */
    RPT_ReportQueue_t RptQueue;

    /**
     * RPT Critical Queue
     */
    RPT_CriticalQueue_t CritQueue;
    int CritDataHandle;

    /**
     * RPT Operation data
     */
    int OpsDataHandle;
    RPT_OperationData_t OpsData;
    uint8 OpsCount; /* If this count reached to `RPT_OPS_STORE_BACKUP_COUNT`, store backup data */

    /**
     * RPT Mutex ID
     */
    osal_id_t ReportMutexID;
    osal_id_t CritMutexID;
    osal_id_t OpsMutexID;
    

} RPT_Data_t;

extern RPT_Data_t RPT_Data;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (RPT_Main), these
**       functions are not called from any other source module.
*/
void RPT_Main(void);
CFE_Status_t RPT_Init(void);
CFE_Status_t RPT_OpsDataInit(void);
CFE_Status_t RPT_CriticalQInit(void);

void RPT_ForwardReport(void);
void RPT_FowardCritical(void);
void RPT_ForwardCommand(void);

#endif