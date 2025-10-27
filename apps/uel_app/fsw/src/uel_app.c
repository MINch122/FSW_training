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
 *   This file contains the source code for the UEL.
 */

/*
** Include Files:
*/
#include "uel_app.h"
#include "uel_app_cmds.h"
#include "uel_app_utils.h"
#include "uel_app_eventids.h"
#include "uel_app_dispatch.h"
#include "uel_app_tbl.h"
#include "uel_app_version.h"

/*
** global data
*/
UEL_APP_Data_t UEL_APP_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void UEL_APP_Main(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(UEL_APP_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = UEL_APP_Init();
    if (status != CFE_SUCCESS)
    {
        UEL_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** UEL Runloop
    */
    while (CFE_ES_RunLoop(&UEL_APP_Data.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(UEL_APP_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, UEL_APP_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(UEL_APP_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            UEL_APP_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(UEL_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "UEL APP: SB Pipe Read Error, App Will Exit");

            UEL_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(UEL_APP_PERF_ID);

    CFE_ES_ExitApp(UEL_APP_Data.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t UEL_APP_Init(void)
{
    CFE_Status_t status;
    //char VersionString[UEL_APP_CFG_MAX_VERSION_STR_LEN];

    /* Zero out the global data structure */
    memset(&UEL_APP_Data, 0, sizeof(UEL_APP_Data));

    UEL_APP_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app configuration data
    */
    UEL_APP_Data.PipeDepth = UEL_APP_PIPE_DEPTH;

    strncpy(UEL_APP_Data.PipeName, "UEL_APP_CMD_PIPE", sizeof(UEL_APP_Data.PipeName));
    UEL_APP_Data.PipeName[sizeof(UEL_APP_Data.PipeName) - 1] = 0;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("UEL: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
    }
    else
    {
        /*
         ** Initialize housekeeping packet (clear user data area).
         */

        CFE_MSG_Init(CFE_MSG_PTR(UEL_APP_Data.bcn.TelemetryHeader), CFE_SB_ValueToMsgId(UEL_APP_BCN_TLM_MID),
                     sizeof(UEL_APP_Data.bcn));

        memset(&UEL_APP_Data.rpt, 0, sizeof(UEL_APP_Data.rpt));
        CFE_MSG_Init(CFE_MSG_PTR(UEL_APP_Data.rpt.TelemetryHeader),CFE_SB_ValueToMsgId(UEL_APP_RPT_TLM_MID),
                     sizeof(UEL_APP_Data.rpt));

        /*
         ** Create Software Bus message pipe.
         */
        status = CFE_SB_CreatePipe(&UEL_APP_Data.CommandPipe, UEL_APP_Data.PipeDepth, UEL_APP_Data.PipeName);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(UEL_APP_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "UEL: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if(status == CFE_SUCCESS)
    {
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(UEL_APP_SEND_BCN_MID), UEL_APP_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(UEL_APP_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                                "UEL: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    
    }

    if (status == CFE_SUCCESS)
    {
       
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(UEL_APP_CMD_MID), UEL_APP_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(UEL_APP_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "UEL: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    OS_MutSemCreate(&UEL_APP_Data.MutexId);

   

    return status;
}

