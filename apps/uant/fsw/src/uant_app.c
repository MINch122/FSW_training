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
 *   This file contains the source code for the Uant App.
 */

/*
** Include Files(추후수정):
*/
#include <stdio.h>
#include "uant_app.h"
#include "uant_app_cmds.h"
#include "uant_app_eventids.h"
#include "uant_app_dispatch.h"
#include "cfe_msg.h"


/*
** global data
*/
UANT_APP_Data_t UANT_APP_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/


void UANT_APP_Main(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(UANT_APP_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = UANT_APP_Init();
    if (status != CFE_SUCCESS)
    {
        UANT_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** Uant App Runloop
    */
    while (CFE_ES_RunLoop(&UANT_APP_Data.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(UANT_APP_PERF_ID);
        
        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, UANT_APP_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(UANT_APP_PERF_ID);

        if (status == CFE_SUCCESS)
        {   
            
            UANT_APP_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(UANT_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "UANT APP: SB Pipe Read Error, App Will Exit");

            UANT_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(UANT_APP_PERF_ID);

    CFE_ES_ExitApp(UANT_APP_Data.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t UANT_APP_Init(void)
{
    CFE_Status_t status;
    
    /* Zero out the global data structure */
    memset(&UANT_APP_Data, 0, sizeof(UANT_APP_Data));

    UANT_APP_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app configuration data
    */
    UANT_APP_Data.PipeDepth = UANT_APP_PIPE_DEPTH;

    strncpy(UANT_APP_Data.PipeName, "UANT_APP_CMD_PIPE", sizeof(UANT_APP_Data.PipeName));
    UANT_APP_Data.PipeName[sizeof(UANT_APP_Data.PipeName) - 1] = 0;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Uant App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
        CFE_EVS_SendEvent(UANT_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                            "UANT App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
                            
    }
    else
    {
        /*
         ** Initialize housekeeping packet
         */
        CFE_MSG_Init(CFE_MSG_PTR(UANT_APP_Data.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(UANT_APP_HK_TLM_MID),
                     sizeof(UANT_APP_Data.HkTlm));

        CFE_MSG_Init(CFE_MSG_PTR(UANT_APP_Data.bcn.TelemetryHeader), CFE_SB_ValueToMsgId(UANT_APP_BCN_TLM_MID),
                     sizeof(UANT_APP_Data.bcn));

        CFE_MSG_Init(CFE_MSG_PTR(UANT_APP_Data.rpt.TelemetryHeader),CFE_SB_ValueToMsgId(UANT_APP_RPT_TLM_MID),
                     sizeof(UANT_APP_Data.rpt));
        

        /*
         ** Create Software Bus message pipe.
         */
        status = CFE_SB_CreatePipe(&UANT_APP_Data.CommandPipe, UANT_APP_Data.PipeDepth, UANT_APP_Data.PipeName);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(UANT_APP_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Uant App: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to Housekeeping request commands
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(UANT_APP_SEND_HK_MID), UANT_APP_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(UANT_APP_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Uant App: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(UANT_APP_CMD_MID), UANT_APP_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(UANT_APP_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Uant App: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(UANT_APP_SEND_BCN_MID), UANT_APP_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(UANT_APP_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Uant App: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    /**
     * Get I2C2 Handle
     */
    // UANT_APP_Data.Handle = CFE_SRL_ApiGetHandle(CFE_SRL_I2C2_HANDLE_INDEXER);

    if (status == CFE_SUCCESS) {
        CFE_EVS_SendEvent(UANT_APP_INIT_EID,CFE_EVS_EventType_INFORMATION,
                            "UANT app Successfully Initialized.");
    }
    
    return status;
}