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
 *   This file contains the source code for the LTRX App.
 */

/*
** Include Files:
*/
#include "ltrx_app.h"
#include "ltrx_cmds.h"
#include "ltrx_utils.h"
#include "ltrx_eventids.h"
#include "ltrx_dispatch.h"
#include "ltrx_child.h"
#include "hk_msgids.h"
#include "cfe.h"

#include <csp/csp.h>
#include <string.h>

#define LTRX_CSP_SOCKET_OPTIONS_NONE  0
#define LTRX_CSP_STATUS_OK            0
#define LTRX_CSP_STATUS_USED         -4
#define LTRX_CSP_RX_READ_MAX         (LTRX_BEACON_HEADER_SIZE + sizeof(LTRX_MessagePartHeader_t) + LTRX_MAX_MESSAGE_PART_SIZE)

static void *s_LtrxListenSocket = NULL;

CFE_Status_t LTRX_CFE_ListenInit(void)
{
    int status;

    if (s_LtrxListenSocket != NULL)
    {
        (void)csp_close(s_LtrxListenSocket);
        s_LtrxListenSocket = NULL;
    }

    s_LtrxListenSocket = csp_socket(LTRX_CSP_SOCKET_OPTIONS_NONE);
    if (s_LtrxListenSocket == NULL)
    {
        CFE_ES_WriteToSysLog("%s: CFE_SRL_ApiSocketCSP failed\n", __func__);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    status = csp_bind(s_LtrxListenSocket, LTRX_CSP_RX_PORT);
        if (status != LTRX_CSP_STATUS_OK && status != LTRX_CSP_STATUS_USED)
    {
        CFE_ES_WriteToSysLog("%s: CFE_SRL_ApiBindCSP failed at port %u rc=%d\n",
                             __func__, (unsigned)LTRX_CSP_RX_PORT, status);
        (void)csp_close(s_LtrxListenSocket);
        s_LtrxListenSocket = NULL;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    status = csp_listen(s_LtrxListenSocket, 4);
    if (status != LTRX_CSP_STATUS_OK)
    {
        CFE_ES_WriteToSysLog("%s: CFE_SRL_ApiListenCSP failed rc=%d\n",
                             __func__, status);
        (void)csp_close(s_LtrxListenSocket);
        s_LtrxListenSocket = NULL;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    CFE_ES_WriteToSysLog("%s: LTRX listen ready on CSP port %u\n",
                         __func__, (unsigned)LTRX_CSP_RX_PORT);
    return CFE_SUCCESS;
}

/* Transport wrapper: moved from device */
static int32_t LTRX_TransportWrapper(const void *tx_buf,
                                     uint16_t    tx_len,
                                     void       *rx_buf,
                                     int32_t     rx_size,
                                     uint16_t    timeout_ms)
{
    int32 rc;

    if (tx_len > 0)
    {
        (void)timeout_ms;
        rc = CFE_SRL_ApiTransactionCSP(CSP_NODE_LTRX, LTRX_CSP_TX_PORT,
                                       (void *)tx_buf, (int)tx_len,
                                       NULL, 0);

        if (rc == 0)
        {
            CFE_EVS_SendEvent(LTRX_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                              "LTRX: TX error node=%u port=%u tx_len=%u rc=%ld",
                              (unsigned)CSP_NODE_LTRX, (unsigned)LTRX_CSP_TX_PORT,
                              (unsigned)tx_len, (long)rc);
            return -1;
        }

        return 0;
    }

    if (rx_buf == NULL || rx_size == 0 || s_LtrxListenSocket == NULL)
    {
        CFE_EVS_SendEvent(LTRX_RX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LTRX: RX setup error rx_buf=%p rx_size=%ld listen=%p",
                          rx_buf, (long)rx_size, s_LtrxListenSocket);
        return -1;
    }

    csp_conn_t *conn = csp_accept(s_LtrxListenSocket, timeout_ms);
    csp_packet_t *packet = NULL;
    int read_size = (rx_size < 0) ? LTRX_CSP_RX_READ_MAX : rx_size;

    if (conn == NULL)
    {
        return -1;
    }

    packet = csp_read(conn, timeout_ms);
    if (packet == NULL)
    {
        rc = -1;
    }
    else if (packet->length <= 0 || packet->length > read_size)
    {
        csp_buffer_free(packet);
        rc = -1;
    }
    else
    {
        memcpy(rx_buf, packet->data, packet->length);
        rc = packet->length;
        csp_buffer_free(packet);
    }

    if (rc <= 0)
    {
        CFE_EVS_SendEvent(LTRX_RX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LTRX: RX read error port=%u timeout=%u rc=%ld",
                          (unsigned)LTRX_CSP_RX_PORT, (unsigned)timeout_ms, (long)rc);
        (void)csp_close(conn);
        return -1;
    }

    if (csp_conn_dport(conn) != LTRX_CSP_RX_PORT)
    {
        CFE_EVS_SendEvent(LTRX_RX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LTRX: RX port mismatch expected=%u actual=%d len=%ld",
                          (unsigned)LTRX_CSP_RX_PORT, csp_conn_dport(conn), (long)rc);
        (void)csp_close(conn);
        return -1;
    }

    (void)csp_close(conn);

    return rc;
}

/* global data */
LTRX_AppData_t LTRX_AppData;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void LTRX_AppMain(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr; 

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(LTRX_PERF_ID);

    /*
    ** Perform application-specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = LTRX_AppInit();
    if (status != CFE_SUCCESS)
    {
        LTRX_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** LTRX App Runloop
    */
    while (CFE_ES_RunLoop(&LTRX_AppData.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(LTRX_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, LTRX_AppData.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(LTRX_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            LTRX_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(LTRX_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "LTRX APP: SB Pipe Read Error, App Will Exit");

            LTRX_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /* Shut child task after loop */
    LTRX_ChildRequestShutdown();

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(LTRX_PERF_ID);

    CFE_ES_ExitApp(LTRX_AppData.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t LTRX_AppInit(void)
{
    CFE_Status_t status;

    /* Zero out the global data structure */
    memset(&LTRX_AppData, 0, sizeof(LTRX_AppData));

    LTRX_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app configuration data
    */
    LTRX_AppData.PipeDepth = LTRX_PIPE_DEPTH;

    strncpy(LTRX_AppData.PipeName, "LTRX_CMD_PIPE", sizeof(LTRX_AppData.PipeName));
    LTRX_AppData.PipeName[sizeof(LTRX_AppData.PipeName) - 1] = 0;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("LTRX App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
    }
    else
    {
        /*
         ** Create Software Bus message pipe.
         */
        status = CFE_SB_CreatePipe(&LTRX_AppData.CommandPipe, LTRX_AppData.PipeDepth, LTRX_AppData.PipeName);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(LTRX_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "LTRX App: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to Housekeeping request commands
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(LTRX_SEND_HK_MID), LTRX_AppData.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(LTRX_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "LTRX App: Error Subscribing to HK request, RC = 0x%08lX", (unsigned long)status);
        }
    }
    
    if (status == CFE_SUCCESS)
    {  
         /*
        ** Subscribe to Bcn request commands
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(LTRX_SEND_BCN_MID), LTRX_AppData.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(LTRX_SUB_BCN_ERR_EID, CFE_EVS_EventType_ERROR,
                            "LTRX App: Error Subscribing to BCN request, RC = 0x%08lX", (unsigned long)status);
        }
    }
  
    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to ground command packets
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(LTRX_CMD_MID), LTRX_AppData.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(LTRX_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "LTRX App: Error Subscribing to Commands, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        /*
        ** Subscribe to HK Combined Packet 1 (Bus Beacon)
        ** cache latest snapshot for downlink
        */
        status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(HK_COMBINED_PKT1_MID), LTRX_AppData.CommandPipe);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(LTRX_SUB_BUS_BCN_ERR_EID, CFE_EVS_EventType_ERROR,
                              "LTRX App: Error Subscribing to Bus Beacon, RC = 0x%08lX", (unsigned long)status);
        }
    }

    if (status == CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(LTRX_INIT_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "LTRX App Successfully Initialized.");

        LTRX_RegisterTransport(LTRX_TransportWrapper);

        status = LTRX_CFE_ListenInit();
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(LTRX_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                              "LTRX App: Failed to initialize CSP listen socket, RC=0x%08lX",
                              (unsigned long)status);
            return status;
        }

        /* Create Child task */
        status = LTRX_ChildCreate();
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(LTRX_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                            "LTRX App: Failed to create child task, RC=0x%08lX",
                            (unsigned long)status);
        }
    }
    return status;
}