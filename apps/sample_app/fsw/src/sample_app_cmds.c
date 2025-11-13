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
 *   This file contains the source code for the Sample App Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "sample_app.h"
#include "sample_app_cmds.h"
#include "sample_app_msgids.h"
#include "sample_app_eventids.h"
#include "sample_app_version.h"
#include "sample_app_tbl.h"
#include "sample_app_utils.h"
#include "sample_app_msg.h"

/* The sample_lib module provides the SAMPLE_Function() prototype */
#include "sample_lib.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t SAMPLE_APP_SendHkCmd(const SAMPLE_APP_SendHkCmd_t *Msg)
{
    int i;

    /*
    ** Get command execution counters...
    */
    SAMPLE_APP_Data.HkTlm.Payload.CommandErrorCounter = SAMPLE_APP_Data.ErrCounter;
    SAMPLE_APP_Data.HkTlm.Payload.CommandCounter      = SAMPLE_APP_Data.CmdCounter;

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(SAMPLE_APP_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(SAMPLE_APP_Data.HkTlm.TelemetryHeader), true);

    /*
    ** Manage any pending table loads, validations, etc.
    */
    for (i = 0; i < SAMPLE_APP_NUMBER_OF_TABLES; i++)
    {
        CFE_TBL_Manage(SAMPLE_APP_Data.TblHandles[i]);
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SAMPLE NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t SAMPLE_APP_NoopCmd(const SAMPLE_APP_NoopCmd_t *Msg)
{
    SAMPLE_APP_Data.CmdCounter++;

    CFE_EVS_SendEvent(SAMPLE_APP_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "SAMPLE: NOOP command %s",
                      SAMPLE_APP_VERSION);
    
    CFE_SRL_IO_Handle_t *i2c = CFE_SRL_ApiGetHandle(CFE_SRL_I2C1_HANDLE_INDEXER);
    CFE_SRL_IO_Param_t param = {0,};

    uint8_t TxBuf[12] = {0};
    uint8_t RxBuf[12] = {0};
    param.Addr = 23; // stm32
    memcpy(TxBuf, "ILOVEMOZART", 12);
    param.TxData = TxBuf;
    param.TxSize = 12;
    param.RxData = RxBuf;
    param.RxSize = sizeof(RxBuf);
    param.Timeout = 50;

    int32 Status = CFE_SRL_ApiRead(i2c, &param);
    OS_printf("I2C Transaction Status: 0x%08X\n", Status);
    if (Status == CFE_SUCCESS) {
        OS_printf("Rx Data: %s\n", RxBuf);
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t SAMPLE_APP_ResetCountersCmd(const SAMPLE_APP_ResetCountersCmd_t *Msg)
{
    SAMPLE_APP_Data.CmdCounter = 0;
    SAMPLE_APP_Data.ErrCounter = 0;

    CFE_SRL_IO_Handle_t *spi = CFE_SRL_ApiGetHandle(CFE_SRL_SPIO_HANDLE_INDEXER);
    CFE_SRL_IO_Param_t param = {0,};

    uint8_t TxBuf[12] = {0};
    uint8_t RxBuf[12] = {0};
    memcpy(TxBuf, "ILOVEMOZART", 12);
    param.TxData = TxBuf;
    param.TxSize = 12;
    param.RxData = RxBuf;
    param.RxSize = sizeof(RxBuf);
    param.Interval = 5000; // empirical value - 2000 has an error in stm32 slave

    int32 Status = CFE_SRL_ApiRead(spi, &param);
    OS_printf("SPI Transaction Status: 0x%08X\n", Status);
    if (Status == CFE_SUCCESS) {
        OS_printf("Rx Data: %s\n", RxBuf);
    }

    CFE_EVS_SendEvent(SAMPLE_APP_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "SAMPLE: RESET command");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function Process Ground Station Command                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t SAMPLE_APP_ProcessCmd(const SAMPLE_APP_ProcessCmd_t *Msg)
{
    CFE_SRL_IO_Handle_t *uart = CFE_SRL_ApiGetHandle(CFE_SRL_UART_HANDLE_INDEXER);
    CFE_SRL_IO_Param_t param = {0,};

    uint8_t TxBuf[12] = {0};
    uint8_t RxBuf[12] = {0};
    memcpy(TxBuf, "ILOVEMOZART", 12);
    param.TxData = TxBuf;
    param.TxSize = sizeof(TxBuf);
    param.RxData = RxBuf;
    param.RxSize = sizeof(RxBuf);
    param.Timeout = 500;

    int32 Status = CFE_SRL_ApiRead(uart, &param);
    OS_printf("UART Transaction Status: 0x%08X\n", Status);
    if (Status == CFE_SUCCESS) {
        OS_printf("Rx Data: %s\n", RxBuf);
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* A simple example command that displays a passed-in value                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t SAMPLE_APP_DisplayParamCmd(const SAMPLE_APP_DisplayParamCmd_t *Msg)
{
    uint8_t TxBuf[12] = {0,};
    uint8_t RxBuf[32] = {0,};
    memcpy(TxBuf, "ILOVEMOZART", sizeof(TxBuf));
    int32 Status = CFE_SRL_ApiTransactionCSP(CSP_NODE_SOBC, 0, TxBuf, sizeof(TxBuf), RxBuf, sizeof(RxBuf));
    OS_printf("CSP Transaction Status: 0x%08X\n", Status);
    if (Status > 0) {
        OS_printf("Rx Data: %s\n", RxBuf);
    }

    return CFE_SUCCESS;
}

CFE_Status_t SAMPLE_APP_NativeCANCmd(const SAMPLE_APP_NativeCANCmd_t *Msg)
{
    CFE_SRL_IO_Handle_t *uart = CFE_SRL_ApiGetHandle(CFE_SRL_CAN0_HANDLE_INDEXER);
    CFE_SRL_IO_Param_t param = {0,};

    uint8_t TxBuf[12] = {0};
    uint8_t RxBuf[12] = {0};
    memcpy(TxBuf, "ILOVEMOZART", 12);
    param.TxData = TxBuf;
    param.TxSize = sizeof(TxBuf);
    param.RxData = RxBuf;
    param.RxSize = sizeof(RxBuf);
    param.Timeout = 5000;

    int32 Status = CFE_SRL_ApiRead(uart, &param);
    OS_printf("CAN Transaction Status: 0x%08X\n", Status);
    if (Status == CFE_SUCCESS) {
        OS_printf("Rx Data: %s\n", RxBuf);
    }

    return CFE_SUCCESS;
}
