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
 *   This file contains the source code for the Sample App.
 */

/*
** Include Files:
*/
#include "SLT_IFB_app.h"
#include "SLT_IFB_cmds.h"
#include "SLT_IFB_utils.h"
#include "SLT_IFB_eventids.h"
#include "SLT_IFB_dispatch.h"
#include "SLT_IFB_tbl.h"
#include "SLT_IFB_version.h"
#include "SLT_IFB_utils.h"

/*
** global data
*/
SLT_IFB_Data_t SLT_IFB_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void SLT_IFB_Main(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(SLT_IFB_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = SLT_IFB_Init();
    if (status != CFE_SUCCESS)
    {
        SLT_IFB_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** Sample App Runloop
    */
    while (CFE_ES_RunLoop(&SLT_IFB_Data.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(SLT_IFB_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, SLT_IFB_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(SLT_IFB_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            SLT_IFB_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(SLT_IFB_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SAMPLE APP: SB Pipe Read Error, App Will Exit");

            SLT_IFB_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(SLT_IFB_PERF_ID);

    CFE_ES_ExitApp(SLT_IFB_Data.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t SLT_IFB_Init(void)
{
    CFE_Status_t status;
    char         VersionString[SLT_IFB_CFG_MAX_VERSION_STR_LEN];

    /* Zero out the global data structure */
    memset(&SLT_IFB_Data, 0, sizeof(SLT_IFB_Data));

    SLT_IFB_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app configuration data
    */
    SLT_IFB_Data.PipeDepth = SLT_IFB_PIPE_DEPTH;

    strncpy(SLT_IFB_Data.PipeName, "SLT_IFB_CMD_PIPE", sizeof(SLT_IFB_Data.PipeName));
    SLT_IFB_Data.PipeName[sizeof(SLT_IFB_Data.PipeName) - 1] = 0;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("SLT IFB: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
    }
    else
    {
        /*
         ** Initialize housekeeping & report packet (clear user data area).
         */
        CFE_MSG_Init(CFE_MSG_PTR(SLT_IFB_Data.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(SLT_IFB_HK_TLM_MID),
                     sizeof(SLT_IFB_Data.HkTlm));

        CFE_MSG_Init(CFE_MSG_PTR(SLT_IFB_Data.RptPkt.TelemetryHeader), CFE_SB_ValueToMsgId(SLT_IFB_RPT_TLM_MID),
                    sizeof(SLT_IFB_Data.RptPkt));

        /*
         ** Create Software Bus message pipe.
         */
        status = CFE_SB_CreatePipe(&SLT_IFB_Data.CommandPipe, SLT_IFB_Data.PipeDepth, SLT_IFB_Data.PipeName);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SLT_IFB_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SLT_IFB App: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to Housekeeping request commands
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SLT_IFB_SEND_HK_MID), SLT_IFB_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SLT_IFB_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SLT_IFB App: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SLT_IFB_CMD_MID), SLT_IFB_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SLT_IFB_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SLT IFB App: Error Subscribing to Basic Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SLT_IFB_TRANSACTION_MID), SLT_IFB_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SLT_IFB_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SLT IFB App: Error Subscribing to Transaction Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SLT_IFB_GET_RPARAM_MID), SLT_IFB_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SLT_IFB_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SLT IFB App: Error Subscribing to Get Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SLT_IFB_SAVE_TABLE_MID), SLT_IFB_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SLT_IFB_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SLT IFB App: Error Subscribing to Save table Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Register Example Table(s)
        */
        status = CFE_TBL_Register(&SLT_IFB_Data.TblHandles[0], "ExampleTable", sizeof(SLT_IFB_ExampleTable_t),
                                  CFE_TBL_OPT_DEFAULT, SLT_IFB_TblValidationFunc);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SLT_IFB_TABLE_REG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Sample App: Error Registering Example Table, RC = 0x%08lX", (unsigned long)status);
        }
        else
        {
            status = CFE_TBL_Load(SLT_IFB_Data.TblHandles[0], CFE_TBL_SRC_FILE, SLT_IFB_TABLE_FILE);
        }

        CFE_Config_GetVersionString(VersionString, SLT_IFB_CFG_MAX_VERSION_STR_LEN, "SLT_IFB App", SLT_IFB_VERSION,
                                    SLT_IFB_BUILD_CODENAME, SLT_IFB_LAST_OFFICIAL);

        CFE_EVS_SendEvent(SLT_IFB_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "SLT_IFB Initialized.%s",
                          VersionString);
    }

    return status;
}
