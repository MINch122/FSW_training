/**
 * @file
 *
 * Main header file for the BATT (NanoPower BP8) application
 */

#ifndef BATT_APP_H
#define BATT_APP_H

/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_config.h"

#include "batt_mission_cfg.h"
#include "batt_platform_cfg.h"

#include "batt_perfids.h"
#include "batt_msgids.h"
#include "batt_msg.h"

/************************************************************************
** Type Definitions
*************************************************************************/

/*
** Global Data
*/
typedef struct
{
    /*
    ** Command interface counters
    */
    uint8 CmdCounter;
    uint8 ErrCounter;

    /*
    ** Housekeeping telemetry packet
    */
    BATT_HkTlm_t HkTlm;

    /*
    ** Run Status variable used in the main processing loop
    */
    uint32 RunStatus;

    /*
    ** Operational data (not reported in housekeeping)
    */
    CFE_SB_PipeId_t CommandPipe;

    /*
    ** Initialization data
    */
    char   PipeName[CFE_MISSION_MAX_API_LEN];
    uint16 PipeDepth;

} BATT_Data_t;

/*
** Global data structure
*/
extern BATT_Data_t BATT_Data;

/****************************************************************************/
/*
** Local function prototypes.
*/
void         BATT_Main(void);
CFE_Status_t BATT_Init(void);

#endif /* BATT_APP_H */
