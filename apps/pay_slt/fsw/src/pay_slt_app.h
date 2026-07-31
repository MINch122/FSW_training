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

#ifndef PAY_SLT_APP_H
#define PAY_SLT_APP_H

/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_config.h"
#include "cfe_srl.h"

#include "pay_slt_mission_cfg.h"
#include "pay_slt_platform_cfg.h"
#include "pay_slt_perfids.h"
#include "pay_slt_msgids.h"
#include "pay_slt_msg.h"

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
    uint8 AppErrCounter;
    uint8 DeviceErrCounter;

    int16 sys_status;
    uint32 sys_uptime;
    uint32 sys_now;
    
    uint16 boot_cnt;
    uint8 boot_his[8];

    uint8 HkEnabled;
    uint8 BcnEnabled;

    PAY_SLT_HkTlm_t HkTlm;
    PAY_SLT_BcnTlm_t BcnTlm;
    PAY_SLT_RPT_t RptPkt;

    /*
    ** Run Status variable used in the main processing loop
    */
    uint32 RunStatus;

    /*
    ** Operational data (not reported in housekeeping)...
    */
    CFE_SB_PipeId_t CommandPipe;
    CFE_SRL_IO_Handle_t *I2c1Handle;
    CFE_SRL_IO_Handle_t *RS422Handle;

    /*
    ** Initialization data (not reported in housekeeping)...
    */
    char   PipeName[CFE_MISSION_MAX_API_LEN];
    uint16 PipeDepth;

} PAY_SLT_Data_t;

/*
** Global data structure
*/
extern PAY_SLT_Data_t PAY_SLT_Data;

/************************************************************************
** Application function prototypes
**
** PAY_SLT_Main is the cFS application entry point. PAY_SLT_Init
** initializes application state, telemetry messages, and the command pipe.
************************************************************************/
void         PAY_SLT_Main(void);

CFE_Status_t PAY_SLT_Init(void);

#endif /* PAY_SLT_APP_H */
