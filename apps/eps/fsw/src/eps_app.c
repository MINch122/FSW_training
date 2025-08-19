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
 *   This file contains the source code for the EPS App.
 */

/*
** Include Files:
*/
#include "eps_app.h"
#include "eps_cmds.h"
#include "eps_eventids.h"
#include "eps_dispatch.h"

/*
** global data
*/
EPS_AppData_t EPS_AppData;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void EPS_Main(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(EPS_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = EPS_Init();
    if (status != CFE_SUCCESS)
    {
        EPS_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** App Runloop
    */
    while (CFE_ES_RunLoop(&EPS_AppData.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(EPS_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, EPS_AppData.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(EPS_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            EPS_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(EPS_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "EPS APP: SB Pipe Read Error, App Will Exit");

            EPS_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(EPS_PERF_ID);

    CFE_ES_ExitApp(EPS_AppData.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t EPS_Init(void)
{
    CFE_Status_t status;

    /* Zero out the global data structure */
    memset(&EPS_AppData, 0, sizeof(EPS_AppData));

    EPS_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("EPS: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
    }
    else
    {
        /*
         ** Initialize housekeeping packet (clear user data area).
         */
        CFE_MSG_Init(CFE_MSG_PTR(EPS_AppData.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(EPS_HK_TLM_MID),
                     sizeof(EPS_AppData.HkTlm));

        /*
         ** Create Software Bus message pipe.
         */
        status = CFE_SB_CreatePipe(&EPS_AppData.CommandPipe, EPS_PIPE_DEPTH, EPS_PIPE_NAME);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(EPS_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "EPS: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to Housekeeping request commands
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(EPS_SEND_HK_MID), EPS_AppData.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(EPS_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "EPS: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(EPS_CMD_MID), EPS_AppData.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(EPS_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "EPS: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS) {

        CFE_EVS_SendEvent(EPS_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "EPS P31u Successfully Initialized");
    }

    /**
     * I2C1 Handle Init
     */
    // EPS_AppData.Handle = CFE_SRL_ApiGetHandle(CFE_SRL_I2C1_HANDLE_INDEXER);

    return status;
}
