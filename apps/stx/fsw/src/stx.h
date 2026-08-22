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

#ifndef STX_H
#define STX_H

/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_config.h"

#include "stx_mission_cfg.h"
#include "stx_platform_cfg.h"

#include "stx_perfids.h"
#include "stx_msgids.h"
#include "stx_msg.h"

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
    ** telemetry packet...
    */

    STX_HkTlm_t HkTlm;

    STX_ReportTlm_t RptPkt;

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

    CFE_TBL_Handle_t TblHandles[STX_NUMBER_OF_TABLES];
} STX_Data_t;

/*
** Global data structure
*/
extern STX_Data_t STX_Data;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (STX_Main), these
**       functions are not called from any other source module.
*/
void         STX_Main(void);
CFE_Status_t STX_Init(void);

void STX_ProcessCommandPacket(CFE_MSG_Message_t *MsgPtr);

#endif /* STX_H */
