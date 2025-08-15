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
 *   This file contains the source code for the Strx App.
 */

/*
** Include Files:
*/
#include "strx_app.h"
#include "strx_app_cmds.h"
#include "strx_app_utils.h"
#include "strx_app_eventids.h"
#include "strx_app_dispatch.h"

/*
** global data
*/
STRX_APP_Data_t STRX_APP_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void STRX_APP_Main(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr; 

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(STRX_APP_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = STRX_APP_Init();
    if (status != CFE_SUCCESS)
    {
        STRX_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** Strx App Runloop
    */
    while (CFE_ES_RunLoop(&STRX_APP_Data.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(STRX_APP_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, STRX_APP_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(STRX_APP_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            STRX_APP_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(STRX_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "STRX APP: SB Pipe Read Error, App Will Exit");

            STRX_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(STRX_APP_PERF_ID);

    CFE_ES_ExitApp(STRX_APP_Data.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t STRX_APP_Init(void)
{
    CFE_Status_t status;

    /* Zero out the global data structure */
    memset(&STRX_APP_Data, 0, sizeof(STRX_APP_Data));

    STRX_APP_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app configuration data
    */
    STRX_APP_Data.PipeDepth = STRX_APP_PIPE_DEPTH;

    strncpy(STRX_APP_Data.PipeName, "STRX_APP_CMD_PIPE", sizeof(STRX_APP_Data.PipeName));
    STRX_APP_Data.PipeName[sizeof(STRX_APP_Data.PipeName) - 1] = 0;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Strx App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
    }
    else
    {
        /*
         ** Initialize housekeeping packet (clear user data area).
         */
    //    CFE_MSG_Init(CFE_MSG_PTR(STRX_APP_Data.ExecReportMsg.TlmHeader),
    //          CFE_SB_ValueToMsgId(STRX_APP_HK_TLM_MID),
    //          sizeof(STRX_APP_Data.ExecReportMsg));

              

        /*
         ** Create Software Bus message pipe.
         */
        status = CFE_SB_CreatePipe(&STRX_APP_Data.CommandPipe, STRX_APP_Data.PipeDepth, STRX_APP_Data.PipeName);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(STRX_APP_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Strx App: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to Housekeeping request commands
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(STRX_APP_SEND_HK_MID), STRX_APP_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(STRX_APP_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Strx App: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(STRX_APP_OIF_MID), STRX_APP_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
           CFE_EVS_SendEvent(STRX_APP_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Strx App: Error Subscribing to OIF MID, RC = 0x%08lX", (unsigned long)status);
        }
    }

  
    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(STRX_APP_CMD_MID), STRX_APP_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(STRX_APP_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Strx App: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {

        CFE_EVS_SendEvent(STRX_APP_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "Strx App Successfully Initialized");
    }

    return status;
}
