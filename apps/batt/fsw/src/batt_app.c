/**
 * \file
 *   This file contains the source code for the BATT (NanoPower BP8) App.
 */

/*
** Include Files:
*/
#include "batt_app.h"
#include "batt_cmds.h"
#include "batt_eventids.h"
#include "batt_dispatch.h"
#include "batt_version.h"

/*
** global data
*/
BATT_Data_t BATT_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void BATT_Main(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(BATT_PERF_ID);

    /*
    ** Perform application-specific initialization
    */
    status = BATT_Init();
    if (status != CFE_SUCCESS)
    {
        BATT_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** BATT App Runloop
    */
    while (CFE_ES_RunLoop(&BATT_Data.RunStatus) == true)
    {
        CFE_ES_PerfLogExit(BATT_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, BATT_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        CFE_ES_PerfLogEntry(BATT_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            BATT_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(BATT_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "BATT: SB Pipe Read Error, App Will Exit");

            BATT_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    CFE_ES_PerfLogExit(BATT_PERF_ID);

    CFE_ES_ExitApp(BATT_Data.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t BATT_Init(void)
{
    CFE_Status_t status;
    char         VersionString[BATT_CFG_MAX_VERSION_STR_LEN];

    /* Zero out the global data structure */
    memset(&BATT_Data, 0, sizeof(BATT_Data));

    BATT_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app configuration data
    */
    BATT_Data.PipeDepth = BATT_PIPE_DEPTH;

    strncpy(BATT_Data.PipeName, "BATT_CMD_PIPE", sizeof(BATT_Data.PipeName));
    BATT_Data.PipeName[sizeof(BATT_Data.PipeName) - 1] = 0;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("BATT App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
    }
    else
    {
        /*
         ** Initialize housekeeping packet (clear user data area).
         */
        CFE_MSG_Init(CFE_MSG_PTR(BATT_Data.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(BATT_HK_TLM_MID),
                     sizeof(BATT_Data.HkTlm));

        /*
         ** Create Software Bus message pipe.
         */
        status = CFE_SB_CreatePipe(&BATT_Data.CommandPipe, BATT_Data.PipeDepth, BATT_Data.PipeName);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(BATT_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "BATT: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to Housekeeping request commands
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(BATT_SEND_HK_MID), BATT_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(BATT_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "BATT: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(BATT_CMD_MID), BATT_Data.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(BATT_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "BATT: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        CFE_Config_GetVersionString(VersionString, BATT_CFG_MAX_VERSION_STR_LEN, "BATT App", BATT_VERSION,
                                    BATT_BUILD_CODENAME, BATT_LAST_OFFICIAL);

        CFE_EVS_SendEvent(BATT_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "BATT App Initialized.%s",
                          VersionString);
    }

    return status;
}
