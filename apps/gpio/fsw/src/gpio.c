/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Gpioace Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the gpioecific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * \file
 *   This file contains the source code for the Gpio.
 */

/*
** Include Files:
*/
#include "gpio.h"
#include "gpio_cmds.h"
#include "gpio_eventids.h"
#include "gpio_digpioatch.h"
#include "gpio_version.h"

/*
** global data
*/
GPIO_Data_t GPIO_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void GPIO_Main(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(GPIO_PERF_ID);

    /*
    ** Perform application-gpioecific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = GPIO_Init();
    if (status != CFE_SUCCESS)
    {
        GPIO_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** Gpio Runloop
    */
    while (CFE_ES_RunLoop(&GPIO_Data.RunStatus) == true)
    {

        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(GPIO_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, GPIO_Data.CommandPipe, CFE_SB_PEND_FOREVER);
        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(GPIO_PERF_ID);


        if (status == CFE_SUCCESS)
        {
            GPIO_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(GPIO_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "GPIO: SB Pipe Read Error, App Will Exit");

            GPIO_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(GPIO_PERF_ID);

    CFE_ES_ExitApp(GPIO_Data.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t GPIO_Init(void)
{
    CFE_Status_t status;
    char         VersionString[GPIO_CFG_MAX_VERSION_STR_LEN];

    /* Zero out the global data structure */
    memset(&GPIO_Data, 0, sizeof(GPIO_Data));

    GPIO_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app configuration data
    */
    GPIO_Data.PipeDepth = GPIO_PIPE_DEPTH;

    strncpy(GPIO_Data.PipeName, "GPIO_CMD_PIPE", sizeof(GPIO_Data.PipeName));
    GPIO_Data.PipeName[sizeof(GPIO_Data.PipeName) - 1] = 0;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Gpio: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
    }
    else
    {
        /*
         ** Initialize housekeeping packet (clear user data area).
         */
        CFE_MSG_Init(CFE_MSG_PTR(GPIO_Data.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(GPIO_HK_TLM_MID),
                     sizeof(GPIO_Data.HkTlm));
        CFE_MSG_Init(CFE_MSG_PTR(GPIO_Data.BcnTlm.TelemetryHeader), CFE_SB_ValueToMsgId(GPIO_BCN_TLM_MID),
                     sizeof(GPIO_Data.BcnTlm));
        GPIO_InitOutputDefaults();

        /*
         ** Create Software Bus message pipe.
         */
        status = CFE_SB_CreatePipe(&GPIO_Data.CommandPipe, GPIO_Data.PipeDepth, GPIO_Data.PipeName);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(GPIO_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Gpio: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to Housekeeping request commands
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(GPIO_SEND_HK_MID), GPIO_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(GPIO_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Gpio: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(GPIO_SEND_BCN_MID), GPIO_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(GPIO_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Gpio: Error Subscribing to BCN request, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(GPIO_CMD_MID), GPIO_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(GPIO_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Gpio: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {

        CFE_Config_GetVersionString(VersionString, GPIO_CFG_MAX_VERSION_STR_LEN, "Gpio", GPIO_VERSION,
                                    GPIO_BUILD_CODENAME, GPIO_LAST_OFFICIAL);

        CFE_EVS_SendEvent(GPIO_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "Gpio Initialized.%s",
                          VersionString);
    }

    return status;
}
