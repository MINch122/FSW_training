/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * @file
 *
 * Main header file for the UELYSYS Payload Roma-SP
 */

#ifndef PAYUEL_ROMA_H
#define PAYUEL_ROMA_H

/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_config.h"

#include "payuel_roma_mission_cfg.h"
#include "payuel_roma_platform_cfg.h"

#include "payuel_roma_perfids.h"
#include "payuel_roma_msgids.h"
#include "payuel_roma_msg.h"

/************************************************************************
** Type Definitions
*************************************************************************/

/*
** Global Data
*/
typedef struct
{
    /*
    ** Command interface counters...
    */
    uint8 CmdCounter;
    uint8 ErrCounter;
    
    //beacon telemetry packet
    PAYUEL_ROMA_BcnTlm_t bcn;

    /*
    ** Housekeeping telemetry packet...
    */
    PAYUEL_ROMA_HkTlm_t HkTlm;

    PAYUEL_ROMA_ReportTlm_t rpt;

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

    CFE_TBL_Handle_t TblHandles[PAYUEL_ROMA_NUMBER_OF_TABLES];
} PAYUEL_ROMA_Data_t;

/*
** Global data structure
*/
extern PAYUEL_ROMA_Data_t PAYUEL_ROMA_Data;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (PAYUEL_ROMA_Main), these
**       functions are not called from any other source module.
*/
void         PAYUEL_ROMA_Main(void);
CFE_Status_t PAYUEL_ROMA_Init(void);

#endif /* PAYUEL_ROMA_H */
