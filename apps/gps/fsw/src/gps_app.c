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
 *   This file contains the source code for the GPS App.
 */

/*
** Include Files:
*/
#include "gps_app.h"
#include "gps_cmds.h"
#include "gps_eventids.h"
#include "gps_dispatch.h"
#include "gps_version.h"

#include "gps_dev_oem.h"

/*
** global data
*/
GPS_AppData_t GPS_AppData;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void GPS_AppMain(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(GPS_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = GPS_AppInit();
    if (status != CFE_SUCCESS)
    {
        GPS_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** App Runloop
    */
    while (CFE_ES_RunLoop(&GPS_AppData.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(GPS_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, GPS_AppData.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(GPS_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            GPS_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(GPS_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "GPS APP: SB Pipe Read Error, App Will Exit");

            GPS_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(GPS_PERF_ID);

    CFE_ES_ExitApp(GPS_AppData.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t GPS_AppInit(void)
{
    CFE_Status_t status;
    char         VersionString[GPS_CFG_MAX_VERSION_STR_LEN];

    /* Zero out the global data structure */
    memset(&GPS_AppData, 0, sizeof(GPS_AppData));

    GPS_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("GPS: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
    }
    else
    {
        /*
         ** Initialize housekeeping packet (clear user data area).
         */
        CFE_MSG_Init(CFE_MSG_PTR(GPS_AppData.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(GPS_HK_TLM_MID),
                     sizeof(GPS_AppData.HkTlm));

        /*
         ** Create Software Bus message pipe.
         */
        status = CFE_SB_CreatePipe(&GPS_AppData.CommandPipe, GPS_PIPE_DEPTH, GPS_PIPE_NAME);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(GPS_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "GPS: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to Housekeeping request commands
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(GPS_SEND_HK_MID), GPS_AppData.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(GPS_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "GPS: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(GPS_CMD_MID), GPS_AppData.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(GPS_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "GPS: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS) {
        CFE_Config_GetVersionString(VersionString, GPS_CFG_MAX_VERSION_STR_LEN, "GPS", GPS_VERSION,
                                    GPS_BUILD_CODENAME, GPS_LAST_OFFICIAL);

        CFE_EVS_SendEvent(GPS_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "GPS Initialized.%s",
                          VersionString);
    }

    if (status == CFE_SUCCESS) {
        status = GPS_Device_Init();
    }


    return status;
}

void GPS_SendReport(const void* cmd,
                    const void* data,
                    uint16 dataSize,
                    int32 retCode,
                    uint8 retType)
{
    CFE_SB_MsgId_t cmdMid;
    CFE_MSG_FcnCode_t cmdCode;

    CFE_MSG_GetMsgId(cmd, &cmdMid);
    CFE_MSG_GetFcnCode(cmd, &cmdCode);

    CFE_MSG_Init(CFE_MSG_PTR(GPS_AppData.Report.TelemetryHeader),
                 CFE_SB_ValueToMsgId(GPS_HK_TLM_MID), // todo: define gps report mid.
                 sizeof(GPS_AppData.Report));
    GPS_AppData.Report.Payload.MsgID = (uint16_t)CFE_SB_MsgIdToValue(cmdMid);
    GPS_AppData.Report.Payload.CommandCode = cmdCode;
    GPS_AppData.Report.Payload.ReturnType = retType;
    GPS_AppData.Report.Payload.ReturnCode = retCode;
    GPS_AppData.Report.Payload.ReturnDataSize = dataSize;
    if (data && dataSize)
        memcpy(GPS_AppData.Report.Payload.ReturnValue,
               data,
               dataSize > RPT_RET_VALUE_BUF_SIZE 
                        ? RPT_RET_VALUE_BUF_SIZE
                        : dataSize);
   CFE_SB_TransmitMsg(CFE_MSG_PTR(GPS_AppData.Report.TelemetryHeader), true);
}
