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
 * \file
 *   This file contains the source code for the Ttc.
 */

/*
** Include Files:
*/
#include "ttc.h"
#include "ttc_cmds.h"
#include "ttc_utils.h"
#include "ttc_eventids.h"
#include "ttc_dispatch.h"
#include "ttc_tbl.h"
#include "ttc_version.h"
#include "ttc_timeline.h"

/*
** global data
*/
TTC_AppData_t TTC_AppData;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void TTC_Main(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(TTC_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = TTC_Init();
    if (status != CFE_SUCCESS)
    {
        TTC_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** Ttc Runloop
    */
    while (CFE_ES_RunLoop(&TTC_AppData.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(TTC_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, TTC_AppData.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(TTC_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            TTC_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(TTC_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "TTC: SB Pipe Read Error, App Will Exit");

            TTC_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(TTC_PERF_ID);

    CFE_ES_ExitApp(TTC_AppData.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t TTC_Init(void)
{
    CFE_Status_t status;
    char         VersionString[TTC_CFG_MAX_VERSION_STR_LEN];

    /* Zero out the global data structure */
    memset(&TTC_AppData, 0, sizeof(TTC_AppData));

    TTC_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Ttc: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
    }
    else
    {
        /*
         ** Initialize housekeeping packet (clear user data area).
         */
        CFE_MSG_Init(CFE_MSG_PTR(TTC_AppData.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(TTC_HK_TLM_MID),
                     sizeof(TTC_AppData.HkTlm));

        /*
         ** Create Software Bus message pipe.
         */
        status = CFE_SB_CreatePipe(&TTC_AppData.CommandPipe, TTC_PLATFORM_PIPE_DEPTH,
                                   TTC_PLATFORM_PIPE_NAME);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(TTC_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Ttc: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to Housekeeping request commands
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(TTC_SEND_HK_MID), TTC_AppData.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(TTC_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Ttc: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(TTC_CMD_MID), TTC_AppData.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(TTC_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Ttc: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to 1Hz wakeup packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(TTC_ONEHZ_WAKEUP_MID), TTC_AppData.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(TTC_SUB_WAKEUP_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Ttc: Error Subscribing to Wakeup, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Register Example Table(s)
        */
        // status = CFE_TBL_Register(&TTC_AppData.TblHandles[0], "ExampleTable", sizeof(TTC_ExampleTable_t),
        //                           CFE_TBL_OPT_DEFAULT, TTC_TblValidationFunc);
        // if (status != CFE_SUCCESS)
        // {
        //     CFE_EVS_SendEvent(TTC_TABLE_REG_ERR_EID, CFE_EVS_EventType_ERROR,
        //                       "Ttc: Error Registering Example Table, RC = 0x%08lX", (unsigned long)status);
        // }
        // else
        // {
        //     status = CFE_TBL_Load(TTC_AppData.TblHandles[0], CFE_TBL_SRC_FILE, TTC_PLATFORM_TABLE_FILE);
        // }

        CFE_Config_GetVersionString(VersionString, TTC_CFG_MAX_VERSION_STR_LEN, "Ttc", TTC_VERSION,
                                    TTC_BUILD_CODENAME, TTC_LAST_OFFICIAL);

        CFE_EVS_SendEvent(TTC_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "Ttc Initialized.%s",
                          VersionString);
    }

    TTC_TimelineInitialize();

    return status;
}
