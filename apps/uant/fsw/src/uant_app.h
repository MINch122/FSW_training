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
 * Main header file for the Sample application
 */

#ifndef UANT_APP_H
#define UANT_APP_H

/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_config.h"
#include "cfe_msg.h"

#include "uant_app_mission_cfg.h"
#include "uant_app_platform_cfg.h"

#include "uant_app_perfids.h"
#include "uant_app_msgids.h"
#include "uant_app_msg.h"

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
    UANT_APP_bcnTlm_t bcn;

    /*
    ** Housekeeping telemetry packet...
    */
    UANT_APP_HkTlm_t HkTlm;

    UANT_RPT_Tlm_t rpt;

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

    CFE_SRL_IO_Handle_t *Handle;
} UANT_APP_Data_t;

/*
** Global data structure
*/
extern UANT_APP_Data_t UANT_APP_Data;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (UANT_APP_Main), these
**       functions are not called from any other source module.
*/
void         UANT_APP_Main(void);
CFE_Status_t UANT_APP_Init(void);

#endif /* UANT_APP_H */
