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

#ifndef STRX_APP_H
#define STRX_APP_H

/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_config.h"

#include "strx_mission_cfg.h"
#include "strx_platform_cfg.h"

#include "strx_perfids.h"
#include "strx_msgids.h"
#include "strx_msg.h"

/************************************************************************
** Type Definitions
*************************************************************************/

/*
** Global Data
*/
typedef struct
{

    /*
    ** Housekeeping telemetry packet...
    */

    STRX_AppCount_t AppCnt;

    STRX_ReportTlm_t RptPkt;
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

} STRX_AppData_t;

/*
** Global data structure
*/
extern STRX_AppData_t STRX_AppData;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (STRX_APP_Main), these
**       functions are not called from any other source module.
*/
void         STRX_AppMain(void);
CFE_Status_t STRX_AppInit(void);

#endif /* STRX_APP_H */
