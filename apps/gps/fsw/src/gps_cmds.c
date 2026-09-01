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
 * @file  GPS ground command handlers
 */
#include "gps_app.h"
#include "gps_cmds.h"
#include "gps_msgids.h"
#include "gps_eventids.h"
#include "gps_version.h"
#include "gps_msg.h"
#include "gps_report.h"
#include "gps_service.h"

/** Bring the housekeeping payload up to date in place. */
static void GPS_RefreshHk(void)
{
    GPS_AppData.HkTlm.Payload.CmdErrorCounter = GPS_AppData.ErrCounter;
    GPS_AppData.HkTlm.Payload.CmdCounter      = GPS_AppData.CmdCounter;

    /* The receive task owns the rest of the payload. */
    GPS_ServiceGetCounters(&GPS_AppData.HkTlm.Payload);
}

CFE_Status_t GPS_SendHkCmd(const GPS_SendHkCmd_t *Msg)
{
    GPS_RefreshHk();

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(GPS_AppData.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(GPS_AppData.HkTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

CFE_Status_t GPS_NoopCmd(const GPS_NoopCmd_t *Msg)
{
    GPS_AppData.CmdCounter++;

    CFE_EVS_SendEvent(GPS_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "GPS: NOOP command %s",
                      GPS_VERSION);
    GPS_SendReport(Msg, NULL, 0, CFE_SUCCESS, GPS_MISSION_REPORT_RETTYPE_HW);

    return CFE_SUCCESS;
}

CFE_Status_t GPS_ResetCountersCmd(const GPS_ResetCountersCmd_t *Msg)
{
    GPS_AppData.CmdCounter = 0;
    GPS_AppData.ErrCounter = 0;

    GPS_ServiceResetCounters();

    CFE_EVS_SendEvent(GPS_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "GPS: RESET command");
    GPS_SendReport(Msg, NULL, 0, CFE_SUCCESS, GPS_MISSION_REPORT_RETTYPE_HW);

    return CFE_SUCCESS;
}

CFE_Status_t GPS_DriverReportHkCmd(const GPS_DriverReportHkCmd_t *Msg)
{
    GPS_AppData.CmdCounter++;

    GPS_RefreshHk();

    CFE_EVS_SendEvent(GPS_HK_RPT_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "GPS: housekeeping reported on request");

    GPS_SendReport(Msg, &GPS_AppData.HkTlm.Payload, sizeof(GPS_AppData.HkTlm.Payload),
                   CFE_SUCCESS, GPS_MISSION_REPORT_RETTYPE_APP);

    return CFE_SUCCESS;
}
