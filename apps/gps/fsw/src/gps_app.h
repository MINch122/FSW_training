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
 * Main header file for the GPS application
 */

#ifndef GPS_APP_H
#define GPS_APP_H

/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_config.h"

#include "gps_mission_cfg.h"
#include "gps_platform_cfg.h"

#include "gps_perfids.h"
#include "gps_msgids.h"
#include "gps_msg.h"

/************************************************************************
** Type Definitions
*************************************************************************/

typedef struct {
    uint32 CmdCounter;
    uint32 ErrCounter;
    uint32 GetBcnErrCounter;
} GPS_AppData_Counters_t;

/*
** Global Data
*/
typedef struct {
    /*
    ** Command interface counters...
    */
    GPS_AppData_Counters_t Counters;

    /*
    ** Housekeeping telemetry packet...
    */
    GPS_HkTlm_t HkTlm;

    /*
    ** Run Status variable used in the main processing loop
    */
    uint32 RunStatus;

    GPS_ReportTlm_t Report;

    /*
    ** Operational data (not reported in housekeeping)...
    */
    CFE_SB_PipeId_t CommandPipe;

} GPS_AppData_t;

/*
** Global data structure
*/
extern GPS_AppData_t GPS_AppData;

void         GPS_AppMain(void);
CFE_Status_t GPS_AppInit(void);

void GPS_SendReport(const void* cmd,
                    const void* data,
                    uint16 dataSize,
                    int32 retCode,
                    uint8 retType);

#endif
