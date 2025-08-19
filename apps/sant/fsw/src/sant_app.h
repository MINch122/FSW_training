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
 * Main header file for the SANT application
 */

#ifndef SANT_APP_H
#define SANT_APP_H

/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_config.h"
#include "cfe_msg.h"

#include "sant_mission_cfg.h"
#include "sant_platform_cfg.h"

#include "sant_perfids.h"
#include "sant_msgids.h"
#include "sant_msg.h"

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
    SANT_HkTlm_t HkTlm;
    SANT_BcnTlm_t BcnTlm;
    SANT_OperationTlm_t OperationTlm;

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


} SANT_Data_t;

/*
** Global data structure
*/
extern SANT_Data_t SANT_Data; // 메모리를 한 번만 할당하기 위해

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (SANT_Main), these
**       functions are not called from any other source module.
*/
void         SANT_AppMain(void);
CFE_Status_t SANT_AppInit(void);

#endif /* SANT_H */
