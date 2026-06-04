/************************************************************************
 * NASA Docket No. GSC-19,200-1, and identified as "cFS Draco"
 *
 * Copyright (c) 2023 United States Government as represented by the
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
 * Main header file for the Ttc application
 */

#ifndef TTC_H
#define TTC_H

/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_config.h"

#include "ttc_mission_cfg.h"
#include "ttc_platform_cfg.h"

#include "ttc_perfids.h"
#include "ttc_msgids.h"
#include "ttc_msg.h"

/************************************************************************
** Type Definitions
*************************************************************************/

/*
** Global Data
*/
typedef struct {

    uint16 CmdCounter;
    uint16 ErrCounter;

    TTC_HkTlm_t     HkTlm;
    TTC_ReportTlm_t Report;

    uint32 RunStatus;

    CFE_SB_PipeId_t CommandPipe;

    CFE_TBL_Handle_t WorkingBufferTbl;

    CFE_TBL_Handle_t CmdSqBufferTbl;

} TTC_AppData_t;


/*
** Global data structure
*/
extern TTC_AppData_t TTC_AppData;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (TTC_Main), these
**       functions are not called from any other source module.
*/
void         TTC_Main(void);
CFE_Status_t TTC_Init(void);

#endif /* TTC_H */
