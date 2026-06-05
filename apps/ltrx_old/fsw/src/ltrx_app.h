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
 * Main header file for the ltrx application
 */

#ifndef LTRX_APP_H
#define LTRX_APP_H

/*
** Required header files.
*/
#include "cfe.h"

#include "ltrx_mission_cfg.h"
#include "ltrx_platform_cfg.h"
#include "ltrx_perfids.h"
#include "ltrx_msgids.h"
#include "ltrx_msg.h"
#include "ltrx.h"

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
    uint32 CmdCounter;
    uint32 AppErrCounter;
    uint32 DeviceErrCounter;

    /**
    * Housekeeping telemetry packet...
    * Not defined. (use zero copy API)
    */
    LTRX_ReportTlm_t RptPkt;

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

    /* Session / last RX trace (for HK) */
    uint8 LastRxType;
    uint8 LastRxStatus;

    /* last received beacon telemetry (from ICD Type 21/23) */
    bool                    HaveGnss;
    LTRX_GNSSInfo_Payload_t  LastGnss;

    bool                    HaveBeaconStatus;
    LTRX_BeaconStatus_Payload_t LastBeaconStatus;

    bool                    HaveMsgStatus;      
    LTRX_MessageStatus_Payload_t LastMsgStatus;

    bool                            HaveBeaconStatusFull;
    LTRX_BeaconStatusFull_Payload_t LastBeaconStatusFull;

} LTRX_AppData_t;

/*
** Global data structure
*/
extern LTRX_AppData_t LTRX_AppData;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (LTRX_APP_Main), these
**       functions are not called from any other source module.
*/
void         LTRX_AppMain(void);
CFE_Status_t LTRX_AppInit(void);
CFE_Status_t LTRX_CFE_ListenInit(void);

#endif /* LTRX_APP_H */
