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
 *   This file contains the source code for the Stx App.
 */

/*
** Include Files:
*/
#include "stx.h"
#include "stx_cmds.h"
#include "stx_utils.h"
#include "stx_eventids.h"
#include "stx_dispatch.h"
#include "stx_tbl.h"
#include "stx_version.h"

/*
** global data
*/
STX_Data_t STX_Data;
CFE_SRL_IO_Handle_t *Handle =NULL;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void STX_Main(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(STX_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = STX_Init();
    if (status != CFE_SUCCESS)
    {
        STX_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** Stx App Runloop
    */
    while (CFE_ES_RunLoop(&STX_Data.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(STX_PERF_ID);

        /* Pend on receipt of command packet 가짜 data 송신 */ 
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, STX_Data.CommandPipe, CFE_SB_PEND_FOREVER);
        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(STX_PERF_ID);

        if (status == CFE_SUCCESS)
        {   
            STX_TaskPipe(SBBufPtr);        
        }
        else
        {
            CFE_EVS_SendEvent(STX_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "STX APP: SB Pipe Read Error, App Will Exit");

            STX_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(STX_PERF_ID);

    CFE_ES_ExitApp(STX_Data.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t STX_Init(void)
{
    CFE_Status_t status;

    Handle = CFE_SRL_ApiGetHandle(CFE_SRL_RS485_HANDLE_INDEXER);
    char         VersionString[STX_CFG_MAX_VERSION_STR_LEN];

    /* Zero out the global data structure */
    memset(&STX_Data, 0, sizeof(STX_Data));

    STX_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app configuration data
    */
    STX_Data.PipeDepth = STX_PIPE_DEPTH;

    strncpy(STX_Data.PipeName, "STX_CMD_PIPE", sizeof(STX_Data.PipeName));
    STX_Data.PipeName[sizeof(STX_Data.PipeName) - 1] = 0;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Stx App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
    }
    else
    {
        /*
         ** Initialize housekeeping packet (clear user data area).
         */
        /*CFE_MSG_Init(CFE_MSG_PTR(STX_Data.SetTlm.TelemetryHeader), CFE_SB_ValueToMsgId(STX_SET_TLM_MID),
                     sizeof(STX_Data.SetTlm));

        CFE_MSG_Init(CFE_MSG_PTR(STX_Data.GetTlm.TelemetryHeader), CFE_SB_ValueToMsgId(STX_GET_TLM_MID),
                     sizeof(STX_Data.GetTlm));*/

        CFE_MSG_Init(CFE_MSG_PTR(STX_Data.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(STX_HK_TLM_MID),
                     sizeof(STX_Data.HkTlm));

        CFE_MSG_Init(CFE_MSG_PTR(STX_Data.BCNTlm.TelemetryHeader), CFE_SB_ValueToMsgId(STX_BCN_TLM_MID),
                     sizeof(STX_Data.BCNTlm));
        
        CFE_MSG_Init(CFE_MSG_PTR(STX_Data.RptPkt.TelemetryHeader), CFE_SB_ValueToMsgId(STX_APP_RPT_TLM_MID),
                     sizeof(STX_Data.RptPkt));

        /*
         ** Create Software Bus message pipe.
         */
        status = CFE_SB_CreatePipe(&STX_Data.CommandPipe, STX_Data.PipeDepth, STX_Data.PipeName);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(STX_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Stx App: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to Housekeeping request commands
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(STX_SEND_HK_MID), STX_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(STX_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Stx App: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to Housekeeping request commands
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(STX_SEND_BCN_MID), STX_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(STX_SUB_BCN_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Stx App: Error Subscribing to BCN request, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(STX_CMD_MID), STX_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(STX_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Stx App: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(STX_CMD_MID), STX_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(STX_SUB_ACK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Stx App: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

        if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(STX_CMD_MID), STX_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(STX_SUB_TLM_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Stx App: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {

        CFE_Config_GetVersionString(VersionString, STX_CFG_MAX_VERSION_STR_LEN, "Stx App", STX_VERSION,
                                    STX_BUILD_CODENAME, STX_LAST_OFFICIAL);

        CFE_EVS_SendEvent(STX_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "Stx App Initialized.%s",
                          VersionString);
    }

    STX_Set_SYMBOLRAtE_t init_symrate;
    STX_Set_CENTERFREQ_t init_centfreq;
    STX_Set_MODCOD_t init_modcod;
    STX_Set_ROLLOFF_t init_rolloff;

    init_symrate.Payload.data = 0x04;
    init_centfreq.Payload.data = 2403.5;
    init_modcod.Payload.data = 1;
    init_rolloff.Payload.data = 2;

    STX_SET_SYMBOLRATECmd(&init_symrate);
    STX_Set_CENTERFREQCmd(&init_centfreq);
    STX_Set_MODCODCmd(&init_modcod);
    STX_Set_ROLLOFFCmd(&init_rolloff);

    return status;
}