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
 *   This file contains the source code for the UELYSYS Payload Roma-SP.
 */

/*
** Include Files:
*/
#include "payuel_roma.h"
#include "payuel_roma_cmds.h"
#include "payuel_roma_utils.h"
#include "payuel_roma_eventids.h"
#include "payuel_roma_dispatch.h"
#include "payuel_roma_tbl.h"
#include "payuel_roma_version.h"

/*
** global data
*/
PAYUEL_ROMA_Data_t PAYUEL_ROMA_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void PAYUEL_ROMA_Main(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;


    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(PAYUEL_ROMA_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = PAYUEL_ROMA_Init();


    if (status != CFE_SUCCESS)
    {
        PAYUEL_ROMA_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** UELYSYS Payload Roma-SP Runloop
    */
    while (CFE_ES_RunLoop(&PAYUEL_ROMA_Data.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(PAYUEL_ROMA_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, PAYUEL_ROMA_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(PAYUEL_ROMA_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            PAYUEL_ROMA_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Roma-SP: SB Pipe Read Error, App Will Exit");

            PAYUEL_ROMA_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(PAYUEL_ROMA_PERF_ID);

    CFE_ES_ExitApp(PAYUEL_ROMA_Data.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_ROMA_Init(void)
{
    CFE_Status_t status;
    char         VersionString[PAYUEL_ROMA_CFG_MAX_VERSION_STR_LEN];


    /* Zero out the global data structure */
    memset(&PAYUEL_ROMA_Data, 0, sizeof(PAYUEL_ROMA_Data));

    PAYUEL_ROMA_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app configuration data
    */
    PAYUEL_ROMA_Data.PipeDepth = PAYUEL_ROMA_PIPE_DEPTH;

    strncpy(PAYUEL_ROMA_Data.PipeName, "PAYUEL_ROMA_CMD_PIPE", sizeof(PAYUEL_ROMA_Data.PipeName));
    PAYUEL_ROMA_Data.PipeName[sizeof(PAYUEL_ROMA_Data.PipeName) - 1] = 0;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Roma-SP: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
    }
    else
    {
        /*
         ** Initialize housekeeping packet (clear user data area).
         */
        CFE_MSG_Init(CFE_MSG_PTR(PAYUEL_ROMA_Data.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(PAYUEL_ROMA_HK_TLM_MID),
                     sizeof(PAYUEL_ROMA_Data.HkTlm));

        CFE_MSG_Init(CFE_MSG_PTR(PAYUEL_ROMA_Data.bcn.TelemetryHeader), CFE_SB_ValueToMsgId(PAYUEL_ROMA_BCN_TLM_MID),
                     sizeof(PAYUEL_ROMA_Data.bcn));

        CFE_MSG_Init(CFE_MSG_PTR(PAYUEL_ROMA_Data.rpt.TelemetryHeader), CFE_SB_ValueToMsgId(PAYUEL_ROMA_REPORT_TLM_MID),
                     sizeof(PAYUEL_ROMA_Data.rpt));

        /*
         ** Create Software Bus message pipe.
         */
        status = CFE_SB_CreatePipe(&PAYUEL_ROMA_Data.CommandPipe, PAYUEL_ROMA_Data.PipeDepth, PAYUEL_ROMA_Data.PipeName);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Roma-SP: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to Housekeeping request commands
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAYUEL_ROMA_SEND_HK_MID), PAYUEL_ROMA_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Roma-SP: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAYUEL_ROMA_CMD_MID), PAYUEL_ROMA_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Roma-SP: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAYUEL_ROMA_SEND_BCN_MID), PAYUEL_ROMA_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Roma-SP: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }


    if (status == CFE_SUCCESS)
    {
        /*
        ** Register Example Table(s)
        */
        status = CFE_TBL_Register(&PAYUEL_ROMA_Data.TblHandles[0], "ExampleTable", sizeof(PAYUEL_ROMA_ExampleTable_t),
                                  CFE_TBL_OPT_DEFAULT, PAYUEL_ROMA_TblValidationFunc);
        
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_TABLE_REG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Roma-SP: Error Registering Example Table, RC = 0x%08lX", (unsigned long)status);
        }
        else
        {
            status = CFE_TBL_Load(PAYUEL_ROMA_Data.TblHandles[0], CFE_TBL_SRC_FILE, PAYUEL_ROMA_TABLE_FILE);
        }

        CFE_Config_GetVersionString(VersionString, PAYUEL_ROMA_CFG_MAX_VERSION_STR_LEN, "UELYSYS PAYLOAD ROMA-SP", PAYUEL_ROMA_VERSION,
                                    PAYUEL_ROMA_BUILD_CODENAME, PAYUEL_ROMA_LAST_OFFICIAL);

        CFE_EVS_SendEvent(PAYUEL_ROMA_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "Roma-SP Initialized. %s",
                          VersionString);
    }

    return status;
}
