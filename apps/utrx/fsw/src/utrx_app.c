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
 *   This file contains the source code for the Utrx App.
 */

/*
** Include Files:
*/
#include "utrx_app.h"
#include "utrx_cmds.h"
#include "utrx_utils.h"
#include "utrx_eventids.h"
#include "utrx_dispatch.h"

/*
** global data
*/
UTRX_AppData_t UTRX_AppData;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void UTRX_AppMain(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr; 

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(UTRX_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = UTRX_AppInit();
    if (status != CFE_SUCCESS)
    {
        UTRX_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** Utrx App Runloop
    */
    while (CFE_ES_RunLoop(&UTRX_AppData.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(UTRX_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, UTRX_AppData.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(UTRX_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            UTRX_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(UTRX_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "UTRX APP: SB Pipe Read Error, App Will Exit");

            UTRX_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(UTRX_PERF_ID);

    CFE_ES_ExitApp(UTRX_AppData.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t UTRX_AppInit(void)
{
    CFE_Status_t status;

    /* Zero out the global data structure */
    memset(&UTRX_AppData, 0, sizeof(UTRX_AppData));

    UTRX_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app configuration data
    */
    UTRX_AppData.PipeDepth = UTRX_PIPE_DEPTH;

    strncpy(UTRX_AppData.PipeName, "UTRX_CMD_PIPE", sizeof(UTRX_AppData.PipeName));
    UTRX_AppData.PipeName[sizeof(UTRX_AppData.PipeName) - 1] = 0;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Utrx App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
    }
    else
    {
        /*
         ** Create Software Bus message pipe.
         */
        status = CFE_SB_CreatePipe(&UTRX_AppData.CommandPipe, UTRX_AppData.PipeDepth, UTRX_AppData.PipeName);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(UTRX_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Utrx App: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to Housekeeping request commands
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(UTRX_SEND_HK_MID), UTRX_AppData.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(UTRX_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Utrx App: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)status);
        }
    }
  
    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(UTRX_CMD_MID), UTRX_AppData.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(UTRX_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Utrx App: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(UTRX_INIT_INF_EID, CFE_EVS_EventType_INFORMATION,
                            "UTRX App Successfully Initialized.");
    }

    return status;
}
