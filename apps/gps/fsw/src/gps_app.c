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
 * @file  GPS entry point, initialization, and main loop
 */
#include "gps_app.h"
#include "gps_cmds.h"
#include "gps_eventids.h"
#include "gps_dispatch.h"
#include "gps_version.h"
#include "gps_service.h"

GPS_AppData_t GPS_AppData;

void GPS_AppMain(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    CFE_ES_PerfLogEntry(GPS_PERF_ID);

    status = GPS_AppInit();
    if (status != CFE_SUCCESS)
    {
        GPS_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while (CFE_ES_RunLoop(&GPS_AppData.RunStatus) == true)
    {
        CFE_ES_PerfLogExit(GPS_PERF_ID);

        status = CFE_SB_ReceiveBuffer(&SBBufPtr, GPS_AppData.CommandPipe, CFE_SB_PEND_FOREVER);

        CFE_ES_PerfLogEntry(GPS_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            GPS_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(GPS_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "GPS: SB Pipe Read Error, App Will Exit");

            GPS_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    CFE_ES_PerfLogExit(GPS_PERF_ID);

    GPS_ServiceShutdown();

    CFE_ES_ExitApp(GPS_AppData.RunStatus);
}

CFE_Status_t GPS_AppInit(void)
{
    CFE_Status_t status;
    char         VersionString[GPS_CFG_MAX_VERSION_STR_LEN];

    memset(&GPS_AppData, 0, sizeof(GPS_AppData));

    GPS_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("GPS: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
    }
    else
    {
        CFE_MSG_Init(CFE_MSG_PTR(GPS_AppData.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(GPS_HK_TLM_MID),
                     sizeof(GPS_AppData.HkTlm));

        CFE_MSG_Init(CFE_MSG_PTR(GPS_AppData.Report.TelemetryHeader), CFE_SB_ValueToMsgId(GPS_REPORT_TLM_MID),
                     sizeof(GPS_AppData.Report));


        status = CFE_SB_CreatePipe(&GPS_AppData.CommandPipe, GPS_PLATFORM_PIPE_DEPTH,
                                   GPS_PLATFORM_PIPE_NAME);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(GPS_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "GPS: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(GPS_SEND_HK_MID), GPS_AppData.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(GPS_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "GPS: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(GPS_CMD_MID), GPS_AppData.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(GPS_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "GPS: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    /* A dead receiver must not stop the app: ground still needs the command
     * path to diagnose and re-open it. */
    if (status == CFE_SUCCESS)
    {
        GPS_ServiceInit();
    }

    if (status == CFE_SUCCESS)
    {
        CFE_Config_GetVersionString(VersionString, GPS_CFG_MAX_VERSION_STR_LEN, "GPS",
                                    GPS_VERSION, GPS_BUILD_CODENAME, GPS_LAST_OFFICIAL);

        CFE_EVS_SendEvent(GPS_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "GPS Initialized.%s",
                          VersionString);
    }

    return status;
}
