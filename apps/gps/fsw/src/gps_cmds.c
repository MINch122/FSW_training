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
 * \file
 *   This file contains the source code for the GPS App Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "gps_app.h"
#include "gps_cmds.h"
#include "gps_msgids.h"
#include "gps_eventids.h"
#include "gps_version.h"
#include "gps_msg.h"


CFE_Status_t GPS_SendHkCmd(const GPS_SendHkCmd_t *Msg)
{
    GPS_AppData.Counters.GetHkErrCounter++;

    /*
    ** Get command execution counters...
    */
    // GPS_AppData.HkTlm.Payload.CommandErrorCounter = GPS_AppData.ErrCounter;
    // GPS_AppData.HkTlm.Payload.CommandCounter      = GPS_AppData.CmdCounter;

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(GPS_AppData.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(GPS_AppData.HkTlm.TelemetryHeader), true);


    return CFE_SUCCESS;
}


CFE_Status_t GPS_NoopCmd(const GPS_NoopCmd_t* Msg)
{
    GPS_AppData_Counters_t Counters;

    GPS_AppData.Counters.CmdCounter++;
    Counters = GPS_AppData.Counters;

    CFE_EVS_SendEvent(GPS_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "GPS: NOOP command %s",
                      GPS_VERSION);
    GPS_SendReport(Msg, &Counters, sizeof(Counters), CFE_SUCCESS, RPT_RETTYPE_SUCCESS);

    return CFE_SUCCESS;
}


CFE_Status_t GPS_ResetCountersCmd(const GPS_ResetCountersCmd_t* Msg)
{
    GPS_AppData.Counters.CmdCounter = 0;
    GPS_AppData.Counters.ErrCounter = 0;
    GPS_AppData.Counters.GetBcnErrCounter = 0;
    GPS_AppData.Counters.GetHkErrCounter = 0;

    CFE_EVS_SendEvent(GPS_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "GPS: RESET command");
    GPS_SendReport(Msg, &GPS_AppData.Counters, sizeof(GPS_AppData.Counters), CFE_SUCCESS, RPT_RETTYPE_SUCCESS);

    return CFE_SUCCESS;
}


CFE_Status_t GPS_GetCountersCmd(const GPS_GetCountersCmd_t* Msg)
{
    GPS_AppData.Counters.CmdCounter++;

    CFE_EVS_SendEvent(GPS_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "GPS: COUNTERS report command");
    GPS_SendReport(Msg, &GPS_AppData.Counters, sizeof(GPS_AppData.Counters), CFE_SUCCESS, RPT_RETTYPE_SUCCESS);

    return CFE_SUCCESS;
}

CFE_Status_t GPS_GetAppDataCmd(const GPS_GetAppDataCmd_t* Msg)
{
    GPS_AppData.Counters.CmdCounter++;

    CFE_EVS_SendEvent(GPS_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "GPS: APP DATA report command");
    GPS_SendReport(Msg, &GPS_AppData.HkTlm.Payload, sizeof(GPS_AppData.HkTlm.Payload),
                   CFE_SUCCESS, RPT_RETTYPE_SUCCESS);

    return CFE_SUCCESS;
}
