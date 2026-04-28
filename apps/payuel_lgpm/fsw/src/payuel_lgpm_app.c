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
 *   This file contains the source code for the PAYUEL_LGPM.
 */

/*
** Include Files:
*/
#include "payuel_lgpm_app.h"
#include "payuel_lgpm_cmds.h"
#include "payuel_lgpm_utils.h"
#include "payuel_lgpm_eventids.h"
#include "payuel_lgpm_dispatch.h"
#include "payuel_lgpm_tbl.h"
#include "payuel_lgpm_version.h"

/*
** global data
*/
PAYUEL_LGPM_Data_t PAYUEL_LGPM_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void PAYUEL_LGPM_AppMain(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(PAYUEL_LGPM_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = PAYUEL_LGPM_AppInit();
    if (status != CFE_SUCCESS)
    {
        PAYUEL_LGPM_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** PAYUEL_LGPM Runloop
    */
    while (CFE_ES_RunLoop(&PAYUEL_LGPM_Data.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(PAYUEL_LGPM_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, PAYUEL_LGPM_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(PAYUEL_LGPM_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            OS_printf("LGPM: Packet received");
            PAYUEL_LGPM_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_LGPM: SB Pipe Read Error, App Will Exit");

            PAYUEL_LGPM_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(PAYUEL_LGPM_PERF_ID);

    CFE_ES_ExitApp(PAYUEL_LGPM_Data.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_LGPM_AppInit(void)
{
    CFE_Status_t status;
    char         VersionString[PAYUEL_LGPM_CFG_MAX_VERSION_STR_LEN];

    /* Zero out the global data structure */
    memset(&PAYUEL_LGPM_Data, 0, sizeof(PAYUEL_LGPM_Data));

    PAYUEL_LGPM_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app configuration data
    */
    PAYUEL_LGPM_Data.PipeDepth = PAYUEL_LGPM_PIPE_DEPTH;

    strncpy(PAYUEL_LGPM_Data.PipeName, "PAYUEL_LGPM_CMD_PIPE", sizeof(PAYUEL_LGPM_Data.PipeName));
    PAYUEL_LGPM_Data.PipeName[sizeof(PAYUEL_LGPM_Data.PipeName) - 1] = 0;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("PAYUEL_LGPM: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
    }
    else
    {
        /*
         ** Initialize housekeeping packet (clear user data area).
         */
        CFE_MSG_Init(CFE_MSG_PTR(PAYUEL_LGPM_Data.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(PAYUEL_LGPM_HK_TLM_MID),
                     sizeof(PAYUEL_LGPM_Data.HkTlm));

        /*
         ** Create Software Bus message pipe.
         */
        status = CFE_SB_CreatePipe(&PAYUEL_LGPM_Data.CommandPipe, PAYUEL_LGPM_Data.PipeDepth, PAYUEL_LGPM_Data.PipeName);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_LGPM: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to Housekeeping request commands
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAYUEL_LGPM_SEND_HK_MID), PAYUEL_LGPM_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_LGPM: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAYUEL_LGPM_CMD_MID), PAYUEL_LGPM_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_LGPM: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Register Example Table(s)
        */
        status = CFE_TBL_Register(&PAYUEL_LGPM_Data.TblHandles[0], "ExampleTable", sizeof(PAYUEL_LGPM_ExampleTable_t),
                                  CFE_TBL_OPT_DEFAULT, PAYUEL_LGPM_TblValidationFunc);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_TABLE_REG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_LGPM: Error Registering Example Table, RC = 0x%08lX", (unsigned long)status);
        }
        else
        {
            status = CFE_TBL_Load(PAYUEL_LGPM_Data.TblHandles[0], CFE_TBL_SRC_FILE, PAYUEL_LGPM_TABLE_FILE);
        }

        CFE_Config_GetVersionString(VersionString, PAYUEL_LGPM_CFG_MAX_VERSION_STR_LEN, "PAYUEL_LGPM", PAYUEL_LGPM_VERSION,
                                    PAYUEL_LGPM_BUILD_CODENAME, PAYUEL_LGPM_LAST_OFFICIAL);

        CFE_EVS_SendEvent(PAYUEL_LGPM_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "PAYUEL_LGPM Initialized.%s",
                          VersionString);
    }

    /**
     * Get RS422 Handle
     */
    PAYUEL_LGPM_Data.Handle = CFE_SRL_ApiGetHandle(CFE_SRL_RS422_HANDLE_INDEXER);

    if (status == CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAYUEL_LGPM_INIT_INF_EID,CFE_EVS_EventType_INFORMATION,
                            "PAYUEL_LGPM app Successfully Initialized.");
    }
    
    return status;
}
