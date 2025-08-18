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
 *   This file contains the source code for the SANT App.
 */



/*===== SANT APP – include order =====*/


#include "sant_app_msgstruct.h"
#include "sant_app_msg.h"
#include "sant_app_cmds.h"
#include "sant_app.h"
#include "sant_app_utils.h"
#include "sant_app_eventids.h"
#include "sant_app_dispatch.h"
#include "cfe_msg.h"


#include <gs/util/linux/drivers/i2c/i2c.h>
#include <gs/util/linux/drivers/i2c/i2c_native.h>



/*
** global data
*/
SANT_APP_Data_t SANT_APP_Data;
//CFE_ES_MAIN(UAT_AppMain);
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void SANT_APP_Main(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(SANT_APP_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = SANT_APP_Init();
    if (status != CFE_SUCCESS)
    {
        SANT_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** SANT App Runloop
    */
    while (CFE_ES_RunLoop(&SANT_APP_Data.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(SANT_APP_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, SANT_APP_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(SANT_APP_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            SANT_APP_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(SANT_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SANT APP: SB Pipe Read Error, App Will Exit");

            SANT_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(SANT_APP_PERF_ID);

    CFE_ES_ExitApp(SANT_APP_Data.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t SANT_APP_Init(void)
{
    CFE_Status_t status;
     /* Zero out the global data structure */
    memset(&SANT_APP_Data, 0, sizeof(SANT_APP_Data));
    
   

    SANT_APP_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;


    /*
    ** Initialize app configuration data
    */
    SANT_APP_Data.PipeDepth = SANT_APP_PIPE_DEPTH;

    strncpy(SANT_APP_Data.PipeName, "SANT_APP_CMD_PIPE", sizeof(SANT_APP_Data.PipeName));
    SANT_APP_Data.PipeName[sizeof(SANT_APP_Data.PipeName) - 1] = 0;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("SANT App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
        CFE_EVS_SendEvent(SANT_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                            "SANT App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
                            
    }
    else
    {
        /*
         ** Initialize housekeeping packet (clear user data area). // 헤더 초기화하는 코드
         자기가 생성하는 메세지에만 하면 됨
         */
        CFE_MSG_Init(CFE_MSG_PTR(SANT_APP_Data.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(SANT_APP_HK_TLM_MID),
                     sizeof(SANT_APP_Data.HkTlm));
        // OP_TLM_MID는 dispatch 에서 초기화

        /*
         ** Create Software Bus message pipe.
         */
        status = CFE_SB_CreatePipe(&SANT_APP_Data.CommandPipe, SANT_APP_Data.PipeDepth, SANT_APP_Data.PipeName);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SANT_APP_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SANT App: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    }

    int init_err = gs_linux_i2c_init(0, "/dev/i2c-0");
    if (init_err != GS_OK)
    {
        
        CFE_EVS_SendEvent(SANT_APP_I2C_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Sensor read failed: driver error code = 0x%X", init_err);

        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    } else {
        status = CFE_SUCCESS;
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to Housekeeping request commands
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SANT_APP_SEND_HK_MID), SANT_APP_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SANT_APP_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SANT App: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SANT_APP_CMD_MID), SANT_APP_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SANT_APP_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SANT App: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

        if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SANT_APP_SEND_OP_MID), SANT_APP_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(SANT_APP_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SANT App: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    
    return status;
}