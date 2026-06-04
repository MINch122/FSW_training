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

#ifndef SLT_PAY_APP_H
#define SLT_PAY_APP_H

/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_config.h"

#include "SLT_IFB_mission_cfg.h"
#include "SLT_IFB_platform_cfg.h"
#include "SLT_IFB_perfids.h"
#include "SLT_IFB_msgids.h"
#include "SLT_IFB_msg.h"

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
    uint16 sys_status;
    uint32 sys_uptime;
    uint16 boot_cnt;
    uint16 boot_cause;
    uint16 reboot_cause;
    uint32 wdt_left;
    int16 brd_temp;
    uint16 pwr_current;
    uint16 imu_data[6];
    int16 ntc_data[4];

    SLT_IFB_SendHkCmd_t HkTlm;
    SLT_IFB_RPT_t RptPkt;

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

    CFE_TBL_Handle_t TblHandles[SLT_IFB_NUMBER_OF_TABLES];

} SLT_IFB_Data_t;

/*
** Global data structure
*/
extern SLT_IFB_Data_t SLT_IFB_Data;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (SAMPLE_APP_Main), these
**       functions are not called from any other source module.
*/
void         SLT_IFB_Main(void);
CFE_Status_t SLT_IFB_Init(void);

#endif /* SAMPLE_APP_H */