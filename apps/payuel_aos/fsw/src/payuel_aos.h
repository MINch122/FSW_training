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
 * Main header file for the PayuelAos application
 */

#ifndef PAYUEL_AOS_H
#define PAYUEL_AOS_H

/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_config.h"

#include "payuel_aos_mission_cfg.h"
#include "payuel_aos_platform_cfg.h"

#include "payuel_aos_perfids.h"
#include "payuel_aos_msgids.h"
#include "payuel_aos_msg.h"

extern CFE_SRL_IO_Handle_t *i2c;

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

    /*
    ** Housekeeping telemetry packet...
    */
    PAYUEL_AOS_HkTlm_t HkTlm;

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

    CFE_TBL_Handle_t TblHandles[PAYUEL_AOS_NUMBER_OF_TABLES];
} PAYUEL_AOS_Data_t;

/*
** Global data structure
*/
extern PAYUEL_AOS_Data_t PAYUEL_AOS_Data;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (PAYUEL_AOS_Main), these
**       functions are not called from any other source module.
*/
void         PAYUEL_AOS_Main(void);
CFE_Status_t PAYUEL_AOS_Init(void);

#endif /* PAYUEL_AOS_H */
