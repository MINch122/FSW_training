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
 *   This file contains the source code for the UELYSYS Payload Roma-SP Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "payuel_roma.h"
#include "payuel_roma_cmds.h"
#include "payuel_roma_msgids.h"
#include "payuel_roma_eventids.h"
#include "payuel_roma_version.h"
#include "payuel_roma_tbl.h"
#include "payuel_roma_utils.h"
#include "payuel_roma_msg.h"
#include "s5lab.h"

#include "csp/csp_endian.h"


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t PAYUEL_ROMA_SendHkCmd(const PAYUEL_ROMA_SendHkCmd_t *Msg)
{
    int i;

    /*
    ** Get command execution counters...
    */
    PAYUEL_ROMA_Data.HkTlm.Payload.CommandErrorCounter = PAYUEL_ROMA_Data.ErrCounter;
    PAYUEL_ROMA_Data.HkTlm.Payload.CommandCounter      = PAYUEL_ROMA_Data.CmdCounter;

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(PAYUEL_ROMA_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(PAYUEL_ROMA_Data.HkTlm.TelemetryHeader), true);

    /*
    ** Manage any pending table loads, validations, etc.
    */
    for (i = 0; i < PAYUEL_ROMA_NUMBER_OF_TABLES; i++)
    {
        CFE_TBL_Manage(PAYUEL_ROMA_Data.TblHandles[i]);
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* NOOP commands                                                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_ROMA_NoopCmd(const PAYUEL_ROMA_NoopCmd_t *Msg)
{
    PAYUEL_ROMA_Data.CmdCounter++;

    CFE_EVS_SendEvent(PAYUEL_ROMA_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "NOOP command %s",
                      PAYUEL_ROMA_VERSION);
    
    CFE_SRL_IO_Handle_t *i2c = CFE_SRL_ApiGetHandle(CFE_SRL_I2C1_HANDLE_INDEXER);
    CFE_SRL_IO_Param_t param = {0,};

    uint8 TxBuf[12] = {0};
    uint8 RxBuf[12] = {0};
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

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_NOOP_CC,
                             (Status == CFE_SUCCESS) ? RxBuf : NULL,
                             (Status == CFE_SUCCESS) ? sizeof(RxBuf) : 0);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t PAYUEL_ROMA_ResetCountersCmd(const PAYUEL_ROMA_ResetCountersCmd_t *Msg)
{
    PAYUEL_ROMA_Data.CmdCounter = 0;
    PAYUEL_ROMA_Data.ErrCounter = 0;

    CFE_SRL_IO_Handle_t *spi = CFE_SRL_ApiGetHandle(CFE_SRL_SPIO_HANDLE_INDEXER);
    CFE_SRL_IO_Param_t param = {0,};

    uint8 TxBuf[12] = {0};
    uint8 RxBuf[12] = {0};
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

    CFE_EVS_SendEvent(PAYUEL_ROMA_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "RESET command");

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_RESET_COUNTERS_CC,
                             (Status == CFE_SUCCESS) ? RxBuf : NULL,
                             (Status == CFE_SUCCESS) ? sizeof(RxBuf) : 0);

    return CFE_SUCCESS;
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Custom Command (Ground Test)                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Communication Test:                                                        */
/*  This is a simple ping between the OBC and the Payload, to establish that  */
/*  the CAN communication works correctly. If it fails, the route should      */
/*  be verified.                                                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_ROMA_CommTestCmd(const PAYUEL_ROMA_CommTestCmd_t *Msg)
{
    /* ping */
    uint8 TxBuf[1] = {0x00};
    uint8 RxBuf[256] = {0,};
    int32 Status;

    Status = CFE_SRL_ApiTransactionCSP(CSP_NODE_ROMA, 1, TxBuf, sizeof(TxBuf), RxBuf, -1);

    if (Status >= 0)
    {
        OS_printf("[ROMA-SP] Communication Test Success. Received %d bytes\n", (int)Status);

        if (Status > 0)
        {
            OS_printf("[ROMA-SP] Response:");
            for (int i = 0; i < Status; i++) OS_printf("%02X ", RxBuf[i]);
            OS_printf("\n");
        }
    }
    else
    {
        OS_printf("[ROMA-SP] Comm. Test Failed. Status: 0x%08X\n", (unsigned int)Status);
    }

    PAYUEL_ROMA_HandleReport((Status >= 0) ? CFE_SUCCESS : CFE_STATUS_EXTERNAL_RESOURCE_FAIL,
                             PAYUEL_ROMA_COMM_TEST_CC,
                             (Status > 0) ? RxBuf : NULL,
                             (Status > 0) ? (uint16)Status : 0);

    return CFE_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Clock Synchronization Test:                                                */
/*  This test synchronize the OBC's time to the Payload's                     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_ROMA_ClockSyncCmd(const PAYUEL_ROMA_ClockSyncCmd_t *Msg)
{
    CFE_TIME_SysTime_t CurrentTime;
    uint32 Seconds;
    uint32 MicroSeconds;

    CurrentTime = CFE_TIME_GetTime();
    Seconds = CurrentTime.Seconds;
    MicroSeconds = CFE_TIME_Sub2MicroSecs(CurrentTime.Subseconds);

    uint32_t NanoSeconds = MicroSeconds * 1000;

    // Big Endian
    uint8 TxBuf[9] = {0};
    TxBuf[0] = 0x06; // Command ID: Clock Sync

    // Seconds (4 bytes)
    TxBuf[1] = (Seconds >> 24) & 0xFF;
    TxBuf[2] = (Seconds >> 16) & 0xFF;
    TxBuf[3] = (Seconds >> 8) & 0xFF;
    TxBuf[4] = Seconds & 0xFF;

    // Nanoseconds (4 bytes)
    TxBuf[5] = (NanoSeconds >> 24) & 0xFF;
    TxBuf[6] = (NanoSeconds >> 16) & 0xFF;
    TxBuf[7] = (NanoSeconds >> 8) & 0xFF;
    TxBuf[8] = NanoSeconds & 0xFF;

    // debug print
    OS_printf("[ROMA-SP] Tx Data: ");
    for (int i = 0; i < sizeof(TxBuf); i++) OS_printf("%02X ", TxBuf[i]);
    OS_printf("\n");

    uint8 RxBuf[9] = {0};

    int32 Status = CFE_SRL_ApiTransactionCSP(CSP_NODE_ROMA, 0, TxBuf, sizeof(TxBuf), RxBuf, sizeof(RxBuf));

    if (Status >= 0)
    {
        OS_printf("%s: status(recieved len) %d\n", __func__, Status);
        
        if (Status > 0)
        {
            OS_printf("[ROMA-SP] Rx Data: ");
            for (int i = 0; i < Status; i++) OS_printf("%02X ", RxBuf[i]);
            OS_printf("\n");
        }
    }
    else
    {
        OS_printf("[ROMA-SP] Clock Synchronization Test Failed. Status: 0x%08X\n", (unsigned int)Status);
    }

    PAYUEL_ROMA_HandleReport((Status >= 0) ? CFE_SUCCESS : CFE_STATUS_EXTERNAL_RESOURCE_FAIL,
                             PAYUEL_ROMA_CLOCK_SYNC_CC,
                             (Status > 0) ? RxBuf : NULL,
                             (Status > 0) ? (uint16)Status : 0);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Log Test:                                                                  */
/*  This test retrieves the last log entry from the Payload. The string       */
/*  should report the Boot Count (BC) and the Reset Cause (RC).               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_ROMA_LogTestCmd(const PAYUEL_ROMA_LogTestCmd_t *Msg)
{
    uint8 TxBuf[1] = {0x03};
    uint8 RxBuf[256] = {0};
    int32 Status;

    Status = CFE_SRL_ApiTransactionCSP(CSP_NODE_ROMA, 16, TxBuf, sizeof(TxBuf), RxBuf, -1);

    if (Status > 0)
    {
        OS_printf("[ROMA-SP] Log retrieved successfully. (Len: %d)\n", (int)Status);

        OS_printf("[ROMA-SP] Raw Data: ");
        for (int i = 0; i < Status; i++) OS_printf("%02X ", RxBuf[i]);
        OS_printf("\n");
        

        // manual의 데이터 형식 참조 : [Header(4)] + [Time(4)] + [Meta(4)] + [String]
        if (Status > 12)
        {
            int loglen = (int)(Status - 12);
            OS_printf("[ROMA-SP] Log test: %.*s\n", loglen, &RxBuf[12]);
        }
    }
    else
    {
        OS_printf("[ROMA-SP] Log Retrieval Failed. Error: 0x%08X\n", (unsigned int)Status);
    }

    PAYUEL_ROMA_HandleReport((Status > 0) ? CFE_SUCCESS : CFE_STATUS_EXTERNAL_RESOURCE_FAIL,
                             PAYUEL_ROMA_LOG_TEST_CC,
                             (Status > 0) ? RxBuf : NULL,
                             (Status > 0) ? (uint16)Status : 0);

    return CFE_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Payload Transmission Test:                                                 */
/*  This command will configure the Payload's radio and transmit a packet     */
/*  shortly after. The radio spectrum at 870 MHz will show a packet being     */
/*  transmitted.                                                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Tx DATA example:                                                           */
/*  01 1D 01 03 | 00 FA 01 20 .. 00 FA .. | 4A 41 49 4A 40 | 00 90 00 .. 00   */
/*  command hdr | RF config (12 bytes)    | callsign(JAIJ@)| data payload     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_TransTestCmd(const PAYUEL_ROMA_TransTestCmd_t *Msg)
{

    uint8 TxBuf[] = {
        0x01, 0x1D, 0x01, 0x03, 0x00, 0xFA, 0x01, 0x20, 0x07, 0x09, 
        0x00, 0xFA, 0x01, 0x20, 0x03, 0x05, 0x4A, 0x41, 0x49, 0x4A, 
        0x40, 0x00, 0x90, 0x00, 0xFA, 0x01, 0x5A, 0x01, 0x00, 0x11, 
        0x00, 0x00, 0x00, 0x00
    };
    
    uint8 RxBuf[16] = {0};
    int32  Status = CFE_SRL_ApiTransactionCSP(CSP_NODE_ROMA, 8, TxBuf, sizeof(TxBuf), RxBuf, -1);

    if (Status >= 1)
    {
        OS_printf("[ROMA-SP] Transmission Test Success. Response: 0x%02X\n", RxBuf[0]);
        
        if (RxBuf[0] == 0x01) {
            OS_printf("[ROMA-SP] Radio Configured and Transmitting at 870MHz\n");
            OS_printf("[ROMA-SP] Please check if the signal has been caught in that bandwidth\n");
        }
    }
    else
    {
        OS_printf("[ROMA-SP] Trans Test Failed. Error: 0x%08X\n", (unsigned int)Status);
    }

    PAYUEL_ROMA_HandleReport((Status >= 1) ? CFE_SUCCESS : CFE_STATUS_EXTERNAL_RESOURCE_FAIL,
                             PAYUEL_ROMA_TRANS_TEST_CC,
                             (Status > 0) ? RxBuf : NULL,
                             (Status > 0) ? (uint16)Status : 0);

    return CFE_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Custom Command (ICD, Telecommands Table)                                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* LOG (Port 16)                                                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* GET_SPECIFIC_LINE:                                                         */
/*   Gets a specific line in the log                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_GetSpecificLineCmd(const PAYUEL_ROMA_GetSpecificLineCmd_t *Msg)
{
    uint8 replyBuf[256] = {0};
    s5lab_rep_get_line_t* reply = (s5lab_rep_get_line_t*)replyBuf;
    uint16 rep_size = 0;

    if (Msg->Payload.type != 0)
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "GET_SPECIFIC_LINE Tx err: type got %u", (unsigned int)Msg->Payload.type);
        return CFE_SUCCESS;
    }

    int32 Status = s5lab_get_specific_line(Msg->Payload.line_no, reply, sizeof(replyBuf), &rep_size);
    
    if (Status == S5LAB_OK)
    {
        reply->line_no = csp_ntoh16(reply->line_no);
        reply->time_s  = csp_ntoh32(reply->time_s);
        reply->time_ms = csp_ntoh16(reply->time_ms);
        reply->incr    = csp_ntoh16(reply->incr);

        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                "Rx Log [Line:%u Info:%u Time: %u.%03u s incr: %u]: %s", (unsigned int)reply->line_no, (unsigned int)reply->info,
                (unsigned int)reply->time_s, (unsigned int)reply->time_ms, (unsigned int)reply->incr, reply->line);
        
        // debug print
        OS_printf("[ROMA-SP] GET_SPECIFIC_LINE:\nLine:%u, info:%u, time: %u.%03u s, incr:%u\nMsg: %s\n", 
            (unsigned int)reply->line_no, (unsigned int)reply->info, (unsigned int)reply->time_s,
            (unsigned int)reply->time_ms, (unsigned int)reply->incr, reply->line);
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                            "GET_SPECIFIC_LINE err: %d", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_GET_SPECIFIC_LINE_CC, reply, rep_size);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* GET_MULTIPLE_LINE:                                                         */
/*   Get multiple lines from line start to line stop                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

int log_handler(uint16 idx, void* reply_data, uint16 reply_len, void* user_data)
{
    s5lab_rep_get_line_t* logline = (s5lab_rep_get_line_t*)reply_data;

    char safeline[128] = {0}; // 방어벽

    int32 str_len = reply_len - sizeof(s5lab_rep_get_line_t);

    if (str_len > 0)
    {
        if (str_len >= sizeof(safeline))
        {
            str_len = sizeof(safeline) - 1;
        }
        memcpy(safeline, logline->line, str_len);
    }
    safeline[str_len] = '\0';

    CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "Rx multiple Log [Index:%d]: %s", idx, safeline);
    
    // debug print
    OS_printf("[ROMA-SP] GET_MULTIPLE_LINES:\nIndex:%d, Msg: %s\n", idx, safeline);

    return 0;
}

CFE_Status_t PAYUEL_ROMA_GetMultipleLinesCmd(const PAYUEL_ROMA_GetMultipleLinesCmd_t *Msg)
{

    if (Msg->Payload.type != 1) // type이 1이 맞는지 확인
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "GET_MULTIPLE_LINES Tx err: type got %u", (unsigned int)Msg->Payload.type);
        return CFE_SUCCESS;
    }

    int32 Status = s5lab_get_multiple_lines(Msg->Payload.line_start, Msg->Payload.line_stop, log_handler, NULL);
    
    if (Status == S5LAB_OK)
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "Get_MULTIPLE_LINES totally succeeded. status=%d", Status);
        // debug print
        OS_printf("[ROMA-SP] GET_MULTUPLE_LINES: succeeded\n");
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                            "GET_MULTIPLE_LINES err: %d", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_GET_MULTIPLE_LINES_CC, NULL, 0);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* GET_LATEST_LINE:                                                           */
/*   Get the latest line                                                      */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_GetLatestLineCmd(const PAYUEL_ROMA_GetLatestLineCmd_t *Msg)
{
    uint8 replyBuf[256] = {0};
    s5lab_rep_get_line_t* reply = (s5lab_rep_get_line_t*)replyBuf;
    uint16 rep_size = 0;    

    if (Msg->Payload.type != 3) // type이 3이 맞는지 확인
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "GET_LATEST_LINE Tx err: type got %u", (unsigned int)Msg->Payload.type);
        return CFE_SUCCESS;
    }

    int32 Status = s5lab_get_latest_line(reply, sizeof(replyBuf), &rep_size);
    
    if (Status == S5LAB_OK)
    {
        reply->line_no = csp_ntoh16(reply->line_no);
        reply->time_s  = csp_ntoh32(reply->time_s);
        reply->time_ms = csp_ntoh16(reply->time_ms);
        reply->incr    = csp_ntoh16(reply->incr);

        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                "Rx Log [Line:%u Info:%u Time: %u.%03u s incr: %u]: %s", (unsigned int)reply->line_no, (unsigned int)reply->info,
                (unsigned int)reply->time_s,(unsigned int)reply->time_ms, (unsigned int)reply->incr, reply->line);

        // debug print
        OS_printf("[ROMA-SP] GET_LATEST_LINE\nLine:%u, info:%u, time: %u.%03u s, incr:%u\nMsg: %s\n", 
            (unsigned int)reply->line_no, (unsigned int)reply->info, (unsigned int)reply->time_s,
            (unsigned int)reply->time_ms, (unsigned int)reply->incr, reply->line);
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "GET_LATEST_LINE err: %d", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_GET_LATEST_LINE_CC, reply, rep_size);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* GET_LATEST_N_LINES:                                                        */
/*   Gets the latest N lines                                                  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_GetLatestNLinesCmd(const PAYUEL_ROMA_GetLatestNLinesCmd_t *Msg)
{

    if (Msg->Payload.type != 4) // type이 4가 맞는지 확인
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "GET_LATEST_N_LINES Tx err: type got %u", (unsigned int)Msg->Payload.type);
        return CFE_SUCCESS;
    }

    int32 Status = s5lab_get_latest_n_lines(Msg->Payload.n_lines, log_handler, NULL);
    
    if (Status == S5LAB_OK)
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                            "GET_LATEST_N_LINES totally succeeded. status = %d", Status);

        // debug print
        OS_printf("[ROMA-SP] GET_LATEST_N_LINES: succeeded\n");
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                            "GET_LATEST_N_LINES err: %d", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_GET_LATEST_N_LINES_CC, NULL, 0);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* CLEAR_ALL_LINES:                                                           */
/*   Clear all log lines                                                      */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_ClearAllLinesCmd(const PAYUEL_ROMA_ClearAllLinesCmd_t *Msg)
{
    if ((Msg->Payload.type != 5) || (Msg->Payload.code != 0xB00B))// type이 5가 맞는지, code가 0xB00B가 맞는지 확인
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "CLEAR_ALL_LINES Tx err: type got %u, code got 0x%04X", (unsigned int)Msg->Payload.type, (unsigned int)Msg->Payload.code);
        return CFE_SUCCESS;
    }

    int32 Status = s5lab_clear_all_lines();

    if (Status == S5LAB_OK)
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION, "CLEAR_ALL_LINES: Success");

        // debug print
        OS_printf("[ROMA-SP] CLEAR_ALL_LINES: Success\n");
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "CLEAR_ALL_LINES: CSP transaction failed: 0x%d", Status);

        // debug print
        OS_printf("[ROMA-SP] CLEAR_ALL_LINES: Failed (Status: %d)\n", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_CLEAR_ALL_LINES_CC, NULL, 0);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SCHEDULE (Port 17)                                                         */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* GET_SINGLE_ENTRY:                                                          */
/*   Gets one specific schedule entry                                         */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_GetSingleEntryCmd(const PAYUEL_ROMA_GetSingleEntryCmd_t *Msg)
{
    uint8 replyBuf[256] = {0};
    s5lab_rep_get_single_entry_t* reply = (s5lab_rep_get_single_entry_t*)replyBuf;
    uint16 size = 0;

    if (Msg->Payload.type != 0) // type이 0이 맞는지 확인
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "GET_SINGLE_ENTRY Tx err: type got %u", (unsigned int)Msg->Payload.type);
        return CFE_SUCCESS;
    }

    int32 Status = s5lab_get_single_entry(Msg->Payload.entry_index, reply, sizeof(replyBuf),&size);
    
    if (Status == S5LAB_OK)
    {
        
        if (reply->sign != 'S')
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "GET_SINGLE_ENTRY: Invalid sync (0x%02X)", (unsigned int)reply->sign);

            return CFE_SUCCESS;
        }

        if (size == 2) // 'S'와 index만 온 경우 == 빈 SCH
        {
            // debug print
            OS_printf("[ROMA-SP] Schedule Entry [%u] is empty\n", (unsigned int)reply->entry_index);
        }
        else if (size >= 15) // 헤더 15 byte 이상의 데이터가 온 경우
        {
            int32 cmd_len = size - 15;

            uint32 time_s = (uint32)(reply->time_ms / 1000);
            uint32 time_ms = (uint32)(reply->time_ms % 1000);

            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                                "Rx Schedule[%u]: Time: %u.%03u s, Rep %u ms, Max %u, cmd_len=%d", (unsigned int)reply->entry_index,
                                (unsigned int)time_s,(unsigned int)time_ms,(unsigned int)reply->repeat_every_ms,
                                (unsigned int)reply->repeat_max, (int)cmd_len);
            
            // debug print
            OS_printf("[ROMA-SP] GET_SINGLE_ENTRY:\nEntry Index:%u command length: %d bytes\n", (unsigned int)reply->entry_index, (int)cmd_len);
        }
        
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                            "GET_SINGLE_ENTRY err: %d", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_GET_SINGLE_ENTRY_CC, reply, size);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* GET_MULTIPLE_ENTRIES:                                                      */
/*   Gets a range of entries [start, end)                                     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

int32 sch_handler(uint16 idx, void* reply_data, uint16 reply_len, void* user_data)
{
    s5lab_rep_get_single_entry_t* reply = (s5lab_rep_get_single_entry_t*)reply_data;

    if (reply->sign != 'S')
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                "SCH Handler: Invalid sync (0x%02X) at idx %d", (unsigned int)reply->sign, idx);

        return -1; // error 발생 시 전체 transaction 중단
    }

    if (reply_len == 2) // 빈 schedule entry
    {
        // debug print
        OS_printf("[ROMA-SP] Schedule Entry [%u] is empty\n", (unsigned int)reply->entry_index);
    }
    else if (reply_len >= 15) // 헤더 15 byte 이상의 데이터가 온 경우
    {
        int32 cmd_len = reply_len - 15;

        uint32 time_s = (uint32)(reply->time_ms / 1000);
        uint32 time_ms = (uint32)(reply->time_ms % 1000);

        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                            "Rx Schedule[%u]: Time: %u.%03u s, Rep %u ms, Max %u, cmd_len=%d", (unsigned int)reply->entry_index,
                            (unsigned int)time_s,(unsigned int)time_ms,(unsigned int)reply->repeat_every_ms,
                            (unsigned int)reply->repeat_max, (int)cmd_len);
        
        // debug print
        OS_printf("[ROMA-SP] GET_MULTIPLE_ENTRIES:\nEntry Index:%u command length: %d bytes\n", (unsigned int)reply->entry_index, (int)cmd_len);
    }

    PAYUEL_ROMA_HandleReport(CFE_SUCCESS, PAYUEL_ROMA_GET_MULTIPLE_ENTRIES_CC, reply, reply_len);

    return 0;
}

CFE_Status_t PAYUEL_ROMA_GetMultipleEntriesCmd(const PAYUEL_ROMA_GetMultipleEntriesCmd_t *Msg)
{

    if (Msg->Payload.type != 1) // type이 1이 맞는지 확인
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "GET_MULTIPLE_ENTRIES Tx err: type got %u", (unsigned int)Msg->Payload.type);
        return CFE_SUCCESS;
    }

    int32 Status = s5lab_get_multiple_entries(Msg->Payload.start_index, Msg->Payload.end_index, sch_handler, NULL);

    if (Status == S5LAB_OK)
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                         "GET_MULTILE_ENTRIES totally succeeded.");
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                         "GET_MULTIPLE_ENTRIES err: %d", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_GET_MULTIPLE_ENTRIES_CC, NULL, 0);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* ADD_ENTRY:                                                                 */
/*   Adds a schedule entry                                                    */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_AddEntryCmd(const PAYUEL_ROMA_AddEntryCmd_t *Msg)
{
    uint8 error_code = 0; // tranceiver가 보내는 error code

    if (Msg->Payload.type != 3) // type이 3이 맞는지 확인
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "ADD_ENTRY Tx err: type got %u", (unsigned int)Msg->Payload.type);
        return CFE_SUCCESS;
    }

    // 지상국에서 보낸 Msg의 전체 길이
    size_t TotalMsgSize = 0;
    CFE_MSG_GetSize((CFE_MSG_Message_t *)Msg, &TotalMsgSize);

    // command의 순수 길이 = 전체 길이 - 헤더 등의 고정 길이
    size_t cmd_offset = offsetof(PAYUEL_ROMA_AddEntryCmd_t, Payload.command);
    int32 cmd_len = TotalMsgSize - cmd_offset;

    // overflow 방지
    if (cmd_len < 0)
    {
        cmd_len = 0;
    }
    else if (cmd_len > sizeof(Msg->Payload.command))
    {
        cmd_len = sizeof(Msg->Payload.command);
    }

    int32 Status = s5lab_add_entry(Msg->Payload.time_ms, Msg->Payload.repeat_every_ms, Msg->Payload.repeat_max,
                                  Msg->Payload.command, (uint16)cmd_len, &error_code);

    if (Status == S5LAB_OK)
    {
        if (error_code == 0) // 0x00 이 성공이라고 예상
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                            "ADD_ENTRY success. (Code: 0x%02X)", (unsigned int)error_code);

            // debug print
            OS_printf("[ROMA-SP] ADD_ENTRY:Success (Rx error code = 0x%02X)\n", (unsigned int)error_code);
        }
        else // 통신은 성공했지만 add entry에 실패한 경우
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                            "ADD_ENTRY: Failed (Rx error code = 0x%02X)", (unsigned int)error_code);
        }

        // debug print
        OS_printf("[ROMA-SP] ADD_ENTRY:\nRx error code=0x%02X\n", (unsigned int)error_code);
    }
    else // 통신 자체에 실패
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADD_ENTRY: CSP transaction failed: %d", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_ADD_ENTRY_CC, &error_code, sizeof(error_code));

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* REMOVE_ENTRY:                                                              */
/*   Removes one entry without shifting                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_RemoveEntryCmd(const PAYUEL_ROMA_RemoveEntryCmd_t *Msg)
{
    uint8 result = 0;

    if (Msg->Payload.type != 4) // type이 4가 맞는지 확인
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "REMOVE_ENTRY Tx err: type got %u", (unsigned int)Msg->Payload.type);
        return CFE_SUCCESS;
    }

    int32 Status = s5lab_remove_entry(Msg->Payload.entry_index, &result);

    if (Status == S5LAB_OK)
    {

        if (result == 1) // 1 = removed
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                              "REMOVE_ENTRY (idx: %u): Success", (unsigned int)Msg->Payload.entry_index);
            // debug print
            OS_printf("[ROMA-SP] REMOVE_ENTRY (idx: %u): Success\n", (unsigned int)Msg->Payload.entry_index);
        }
        else if (result == 0) // 0 = failed
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "REMOVE_ENTRY (idx: %u): Failed", (unsigned int)Msg->Payload.entry_index);
            // debug print
            OS_printf("[ROMA-SP] REMOVE_ENTRY (idx: %u): Failed\n", (unsigned int)Msg->Payload.entry_index);
        }
        else
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "REMOVE_ENTRY: Failed command itself (result: %u)", (unsigned int)result);
            // debug print
            OS_printf("[ROMA-SP] REMOVE_ENTRY (idx: %u): Failed command itself. result = %u\n", (unsigned int)Msg->Payload.entry_index, (unsigned int)result);
        }
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "REMOVE_ENTRY: CSP transaction failed: %d", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_REMOVE_ENTRY_CC, &result, sizeof(result));

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* GET_USED_SLOTS:                                                            */
/*   Returns number of active entries                                         */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_GetUsedSlotsCmd(const PAYUEL_ROMA_GetUsedSlotsCmd_t *Msg)
{
    uint8 slots = 0;

    if (Msg->Payload.type != 5) // type이 5가 맞는지 확인
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "GET_USED_SLOTS Tx err: type got %u", (unsigned int)Msg->Payload.type);
        return CFE_SUCCESS;
    }

    int32 Status = s5lab_get_used_slots(&slots);

    if (Status == S5LAB_OK)
    {

        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "GET_USED_SLOTS success: %u slots in use", (unsigned int)slots);
        
        // debug print
        OS_printf("[ROMA-SP] GET_USED_SLOTS: %u\n", (unsigned int)slots);
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "GET_USED_SLOTS: CSP transaction failed: %d", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_GET_USED_SLOTS_CC, &slots, sizeof(slots));

    return CFE_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* ROUTING (Port 18)                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SET_ROUTE_DEFAULT:                                                         */
/*   Apply the default routing table (0/0 CAN). Does not reset the table.     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_SetRouteDefaultCmd(const PAYUEL_ROMA_SetRouteDefaultCmd_t *Msg)
{
    uint8 result = 0;

    if (Msg->Payload.type != 0) // type이 0이 맞는지 확인
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "SET_ROUTE_DEFAULT Tx err: type got %u", (unsigned int)Msg->Payload.type);
        return CFE_SUCCESS;
    }

    int32 Status = s5lab_set_route_default(&result);

    if (Status == S5LAB_OK)
    {
        if (result == 1) // 1 = success
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,"SET_ROUTE_DEFAULT: Success");
            // debug print
            OS_printf("[ROMA-SP] SET_ROUTE_DEFAULT: Success\n");
        }
        else if (result == 0) // 0 = failed
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,"SET_ROUTE_DEFAULT: Failed");
            // debug print
            OS_printf("[ROMA-SP] SET_ROUTE_DEFAULT: Failed\n");
        }
        else
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                            "SET_ROUTE_DEFAULT: Failed command itself. result: %u", (unsigned int)result);
            // debug print
            OS_printf("[ROMA-SP] SET_ROUTE_DEFAULT: Failed command itself. result: %u\n", (unsigned int)result);
        }
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SET_ROUTE_DEFAULT: CSP transaction failed: %d", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_SET_ROUTE_DEFAULT_CC, &result, sizeof(result));

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* RESET_ROUTE:                                                               */
/*   Resets the table to 1/5 CAN                                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_ResetRouteCmd(const PAYUEL_ROMA_ResetRouteCmd_t *Msg)
{
    uint8 result = 0;

    if (Msg->Payload.type != 1) // type이 1이 맞는지 확인
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "RESET_ROUTE Tx err: type got %u", (unsigned int)Msg->Payload.type);
        return CFE_SUCCESS;
    }

    int32 Status = s5lab_reset_route(&result);

    if (Status == S5LAB_OK)
    {
        if (result == 1) // 1 = success
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,"RESET_ROUTE: Success");
            // debug print
            OS_printf("[ROMA-SP] RESET_ROUTE: Success\n");
        }
        else if (result == 0) // 0 = failed
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,"RESET_ROUTE: Failed");
            // debug print
            OS_printf("[ROMA-SP] RESET_ROUTE: Failed\n");
        }
        else
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                "RESET_ROUTE: Failed command itself. result: %u", (unsigned int)result);
            // debug print
            OS_printf("[ROMA-SP] RESET_ROUTE: Failed command itself. result: %u\n", (unsigned int)result);
        }
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "RESET_ROUTE: CSP transaction failed: 0x%08X", (unsigned int)Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_RESET_ROUTE_CC, &result, sizeof(result));

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* LOAD_ROUTE:                                                                */
/*   Loads the route from FRAM                                                */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_LoadRouteCmd(const PAYUEL_ROMA_LoadRouteCmd_t *Msg)
{
    uint8 result = 0;

    if (Msg->Payload.type != 2) // type이 2가 맞는지 확인
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "LOAD_ROUTE Tx err: type got %u", (unsigned int)Msg->Payload.type);
        return CFE_SUCCESS;
    }

    int32 Status = s5lab_load_route(&result);

    if (Status == S5LAB_OK)
    {
        if (result == 1) // 1 = success
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,"LOAD_ROUTE: Success");
            // debug print
            OS_printf("[ROMA-SP] LOAD_ROUTE: Success\n");
        }
        else if (result == 0) // 0 = failed
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,"LOAD_ROUTE: Failed");
            // debug print
            OS_printf("[ROMA-SP] LOAD_ROUTE: Failed\n");
        }
        else
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                            "LOAD_ROUTE: Failed command itself. result: %u", (unsigned int)result);

            // debug print
            OS_printf("[ROMA-SP] LOAD_ROUTE: Failed command itself. result: %u\n", (unsigned int)result);
        }
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LOAD_ROUTE: CSP transaction failed: %d", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_LOAD_ROUTE_CC, &result, sizeof(result));

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SAVE_ROUTE:                                                                */
/*   Save the route to FRAM                                                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_SaveRouteCmd(const PAYUEL_ROMA_SaveRouteCmd_t *Msg)
{
    char route[128] = {0};

    if (Msg->Payload.type != 3) // type이 3이 맞는지 확인
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "SAVE_ROUTE Tx err: type got %u", (unsigned int)Msg->Payload.type);
        return CFE_SUCCESS;
    }

    int32 Status = s5lab_save_route(route, sizeof(route));

    if (Status == S5LAB_OK)
    {
        // 혹시라도 데이터가 꽉 찬 경우
        route[sizeof(route) - 1] = '\0';

        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "SAVE_ROUTE success. Saved in FRAM: %s", route);
        // debug print
        OS_printf("[ROMA-SP] SAVE_ROUTE FRAM saved: %s\n", route);
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SAVE_ROUTE: CSP transaction failed: %d", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_SAVE_ROUTE_CC, route, sizeof(route));

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SEND_ROUTE:                                                                */
/*   Sends the FRAM route via CSP                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_SendRouteCmd(const PAYUEL_ROMA_SendRouteCmd_t *Msg)
{
    uint8 result = 0;

    if (Msg->Payload.type != 4) // type이 4가 맞는지 확인
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "SEND_ROUTE Tx err: type got %u", (unsigned int)Msg->Payload.type);
        return CFE_SUCCESS;
    }

    int32 Status = s5lab_send_route(&result);

    if (Status == S5LAB_OK)
    {
        if (result == 1) // 1 = success
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,"SEND_ROUTE: Success");
            // debug print
            OS_printf("[ROMA-SP] SEND_ROUTE: Success\n");
        }
        else if (result == 0) // 0 = failed
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,"SEND_ROUTE: Failed");
            // debug print
            OS_printf("[ROMA-SP] SEND_ROUTE: Failed\n");
        }
        else
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                            "SEND_ROUTE: Failed command itself. result: %u", (unsigned int)result);
            // debug print
            OS_printf("[ROMA-SP] SEND_ROUTE: Failed command itself. result: %u\n", (unsigned int)result);
        }
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SEND_ROUTE: CSP transaction failed: %d", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_SEND_ROUTE_CC, &result, sizeof(result));

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SET_ROUTE:                                                                 */
/*   Sends the FRAM route from CSP                                            */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_SetRouteCmd(const PAYUEL_ROMA_SetRouteCmd_t *Msg)
{
    uint8 result = 0;

    if (Msg->Payload.type != 5) // type이 5가 맞는지 확인
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "SET_ROUTE Tx err: type got %u", (unsigned int)Msg->Payload.type);
        return CFE_SUCCESS;
    }

    int32 Status = s5lab_set_route((const char*)Msg->Payload.route, &result);

    if (Status == S5LAB_OK)
    {
        if (result == 1) // 1 = success
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,"SET_ROUTE: Success");
            // debug print
            OS_printf("[ROMA-SP] SET_ROUTE: Success\n");
        }
        else if (result == 0) // 0 = failed
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,"SET_ROUTE: Failed");
            // debug print
            OS_printf("[ROMA-SP] SET_ROUTE: Failed\n");
        }
        else
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "SET_ROUTE: Failed command itself. result: %u", (unsigned int)result);
            // debug print
            OS_printf("[ROMA-SP] SET_ROUTE: Failed command itself. result: %u\n", (unsigned int)result);
        }
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SET_ROUTE: CSP transaction failed: %d", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_SET_ROUTE_CC, &result, sizeof(result));

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PARAMETERS (Port 19)                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAR_GET:                                                                   */
/*   Reads a parameter value                                                  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_ParGetCmd(const PAYUEL_ROMA_ParGetCmd_t *Msg)
{
    int32 value = 0;

    if (Msg->Payload.type != 0) // type이 0이 맞는지 확인
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "PAR_GET Tx err: type got %u", (unsigned int)Msg->Payload.type);
        return CFE_SUCCESS;
    }

    int32 Status = s5lab_par_get(Msg->Payload.table, Msg->Payload.param, &value);

    if (Status == S5LAB_OK)
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                        "PAR_GET success: Table %u, Param %u = %d", (unsigned int)Msg->Payload.table, (unsigned int)Msg->Payload.param, (int)value);
        // debug print
        OS_printf("[ROMA-SP] PAR_GET: Table %u, Param %u = %d\n", (unsigned int)Msg->Payload.table, (unsigned int)Msg->Payload.param, (int)value);
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SET_ROUTE: CSP transaction failed: %d", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_PAR_GET_CC, &value, sizeof(value));

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAR_SET:                                                                   */
/*   Sets a parameter value                                                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_ParSetCmd(const PAYUEL_ROMA_ParSetCmd_t *Msg)
{
    uint8 result = 0;

    if (Msg->Payload.type != 1) // type이 1이 맞는지 확인
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "PAR_SET Tx err: type got %u", (unsigned int)Msg->Payload.type);
        return CFE_SUCCESS;
    }

    int32 Status = s5lab_par_set(Msg->Payload.table, Msg->Payload.param, Msg->Payload.value, &result);

    if (Status == S5LAB_OK)
    {
        if (result == 1) // 1 = success
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,"PAR_SET: Success");
            // debug print
            OS_printf("[ROMA-SP] PAR_SET: Success\n");
        }
        else if (result == 0) // 0 = failed
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,"PAR_SET: Failed");
            // debug print
            OS_printf("[ROMA-SP] PAR_SET: Failed\n");
        }
        else
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "PAR_SET: Failed command itself. result: %u", (unsigned int)result);
            // debug print
            OS_printf("[ROMA-SP] PAR_SET: Failed command itself. result: %u\n", (unsigned int)result);
        }
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAR_SET: CSP transaction failed: %d", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_PAR_SET_CC, &result, sizeof(result));

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAR_DEFAULTS:                                                              */
/*   Restores all parameters in the table to defaults                         */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_ParDefaultsCmd(const PAYUEL_ROMA_ParDefaultsCmd_t *Msg)
{
    uint8 result = 0;

    if (Msg->Payload.type != 3) // type이 3이 맞는지 확인
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "PAR_DEFAULTS Tx err: type got %u", (unsigned int)Msg->Payload.type);
        return CFE_SUCCESS;
    }

    int32 Status = s5lab_par_defaults(Msg->Payload.table, &result);

    if (Status == S5LAB_OK)
    {
        if (result == 1) // 1 = success
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,"PAR_DEFAULTS: Success");
            // debug print
            OS_printf("[ROMA-SP] PAR_DEFAULTS: Success\n");
        }
        else if (result == 0) // 0 = failed
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,"PAR_DEFAULTS: Failed");
            // debug print
            OS_printf("[ROMA-SP] PAR_DEFAULTS: Failed\n");
        }
        else
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "PAR_DEFAULTS: Failed command itself. result: %u", (unsigned int)result);
            // debug print
            OS_printf("[ROMA-SP] PAR_DEFAULTS: Failed command itself. result: %u\n", (unsigned int)result);
        }
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAR_DEFAULTS: CSP transaction failed: %d", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_PAR_DEFAULTS_CC, &result, sizeof(result));

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAR_SAVE:                                                                  */
/*   Saves current table parameters to nonvolatile storage                    */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_ParSaveCmd(const PAYUEL_ROMA_ParSaveCmd_t *Msg)
{
    uint8 result = 0;

    if (Msg->Payload.type != 4) // type이 4이 맞는지 확인
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "PAR_SAVE Tx err: type got %u", (unsigned int)Msg->Payload.type);
        return CFE_SUCCESS;
    }

    int32 Status = s5lab_par_save(Msg->Payload.table, &result);

    if (Status == S5LAB_OK)
    {
        if (result == 1) // 1 = success
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,"PAR_SAVE: Success");
            // debug print
            OS_printf("[ROMA-SP] PAR_SAVE: Success\n");
        }
        else if (result == 0) // 0 = failed
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,"PAR_SAVE: Failed");
            // debug print
            OS_printf("[ROMA-SP] PAR_SAVE: Failed\n");
        }
        else
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "PAR_SAVE: Failed command itself. result: %u", (unsigned int)result);
            // debug print
            OS_printf("[ROMA-SP] PAR_SAVE: Failed command itself. result: %u\n", (unsigned int)result);
        }
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAR_SAVE: CSP transaction failed: %d", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_PAR_SAVE_CC, &result, sizeof(result));

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAR_RESTORE:                                                               */
/*   Restores table parameters from saved nonvolatile copy                    */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_ParRestoreCmd(const PAYUEL_ROMA_ParRestoreCmd_t *Msg)
{
    uint8 result = 0;

    if (Msg->Payload.type != 5) // type이 5이 맞는지 확인
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "PAR_RESTORE Tx err: type got %u", (unsigned int)Msg->Payload.type);
        return CFE_SUCCESS;
    }

    int32 Status = s5lab_par_restore(Msg->Payload.table, &result);

    if (Status == S5LAB_OK)
    {
        if (result == 1) // 1 = success
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,"PAR_RESTORE: Success");
            // debug print
            OS_printf("[ROMA-SP] PAR_RESTORE: Success\n");
        }
        else if (result == 0) // 0 = failed
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,"PAR_RESTORE: Failed");
            // debug print
            OS_printf("[ROMA-SP] PAR_RESTORE: Failed\n");
        }
        else
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "PAR_RESTORE: Failed command itself. result: %u", (unsigned int)result);
            // debug print
            OS_printf("[ROMA-SP] PAR_RESTORE: Failed command itself. result: %u\n", (unsigned int)result);
        }
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAR_RESTORE: CSP transaction failed: %d", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_PAR_RESTORE_CC, &result, sizeof(result));

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAR_LOAD:                                                                  */
/*   Loads built-in parameter set for the table                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_ParLoadCmd(const PAYUEL_ROMA_ParLoadCmd_t *Msg)
{
    uint8 result = 0;

    if (Msg->Payload.type != 6) // type이 6이 맞는지 확인
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "PAR_LOAD Tx err: type got %u", (unsigned int)Msg->Payload.type);
        return CFE_SUCCESS;
    }

    int32 Status = s5lab_par_load(Msg->Payload.table, &result);

    if (Status == S5LAB_OK)
    {
        if (result == 1) // 1 = success
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,"PAR_LOAD: Success");
            // debug print
            OS_printf("[ROMA-SP] PAR_LOAD: Success\n");
        }
        else if (result == 0) // 0 = failed
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,"PAR_LOAD: Failed");
            // debug print
            OS_printf("[ROMA-SP] PAR_LOAD: Failed\n");
        }
        else
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "PAR_LOAD: Failed command itself. result: %u", (unsigned int)result);
            // debug print
            OS_printf("[ROMA-SP] PAR_LOAD: Failed command itself. result: %u\n", (unsigned int)result);
        }
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAR_LOAD: CSP transaction failed: %d", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_PAR_LOAD_CC, &result, sizeof(result));

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAR_SET_OOB:                                                               */
/*   Enables or disables out-ouf-bounds changes for the table                 */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_ParSetOobCmd(const PAYUEL_ROMA_ParSetOobCmd_t *Msg)
{
    uint8 result = 0;

    if (Msg->Payload.type != 7) // type이 7이 맞는지 확인
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "PAR_SET_OOB Tx err: type got %u", (unsigned int)Msg->Payload.type);
        return CFE_SUCCESS;
    }

    int32 Status = s5lab_par_set_oob(Msg->Payload.table, Msg->Payload.enable, &result);

    if (Status == S5LAB_OK)
    {
        if (result == 1) // 1 = success
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,"PAR_SET_OOB: Success");
            // debug print
            OS_printf("[ROMA-SP] PAR_SET_OOB: Success\n");
        }
        else if (result == 0) // 0 = failed
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,"PAR_SET_OOB: Failed");
            // debug print
            OS_printf("[ROMA-SP] PAR_SET_OOB: Failed\n");
        }
        else
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "PAR_SET_OOB: Failed command itself. result: %u", (unsigned int)result);
            // debug print
            OS_printf("[ROMA-SP] PAR_SET_OOB: Failed command itself. result: %u\n", (unsigned int)result);
        }
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAR_SET_OOB: CSP transaction failed: %d", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_PAR_SET_OOB_CC, &result, sizeof(result));

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* REMOTE TERMINAL (PORT 20)                                                  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SEND_COMMAND:                                                              */
/*   emulatets typing a command into the terminal, to issue commands          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_SendCommandCmd(const PAYUEL_ROMA_SendCommandCmd_t *Msg)
{
    uint8 result = 0;

    // 방어 버퍼
    char safe_cmd[128] = {0};

    strncpy(safe_cmd, (const char*)Msg->Payload.cmd, sizeof(safe_cmd) - 1);

    safe_cmd[sizeof(safe_cmd) - 1] = '\0';

    int32 Status = s5lab_send_command(safe_cmd, &result);

    OS_printf("[ROMA-SP] Sended Command: %s\n", Msg->Payload.cmd);
    OS_printf("[ROMA-SP] Raw Data: ");
        for (int i = 0; i < strlen(safe_cmd) + 1; i++) OS_printf("%02X ", (uint8)safe_cmd[i]);
    OS_printf("\n");

    if (Status == S5LAB_OK)
    {
        if (result == 1) // 1 = success
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,"SEND_COMMAND: Success");
            // debug print
            OS_printf("[ROMA-SP] SEND_COMMAND: Success\n");
        }
        else if (result == 0) // 0 = failed
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,"SEND_COMMAND: Failed");
            // debug print
            OS_printf("[ROMA-SP] SEND_COMMAND: Failed\n");
        }
        else
        {
            CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "SEND_COMMAND: Failed command itself. result: %u", (unsigned int)result);
            // debug print
            OS_printf("[ROMA-SP] SEND_COMMAND: Failed command itself. result: %u\n", (unsigned int)result);
        }
    }
    else
    {
        CFE_EVS_SendEvent(PAYUEL_ROMA_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SEND_COMMAND: CSP transaction failed: %d", Status);
    }

    PAYUEL_ROMA_HandleReport(Status, PAYUEL_ROMA_SEND_COMMAND_CC, &result, sizeof(result));

    return CFE_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYLOAD OPERATIONS (PORT 8)                                                */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SEND_MSG:                                                                  */
/*   Transmits a packet with the TX configuration. There is no need for       */
/*   null termination                                                         */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_SendMsgCmd(const PAYUEL_ROMA_SendMsgCmd_t *Msg)
{
    uint8 TxBuf[256] = {0};
    
    uint8 msg_len = (uint8)strlen(Msg->Payload.msg);

    uint16 total_size = 4 + msg_len; // 헤더 4 byte: type(1) cmd(1) sub(1) len(1)

    TxBuf[0] = 1;  // type
    TxBuf[1] = 32; // cmd
    TxBuf[2] = 3;   // rm 
    TxBuf[3] = msg_len; // len
    memcpy(&TxBuf[4], Msg->Payload.msg, msg_len);


    int32 Status = CFE_SRL_ApiTransactionCSP(CSP_NODE_ROMA, 8, TxBuf, total_size, NULL, 0);

    if (Status >= 0)
    {
        OS_printf("[ROMA-SP] Send Message Command Success.\n");
        OS_printf("[ROMA-SP] Sended Message: %s", Msg->Payload.msg);

    }
    else
    {
        OS_printf("[ROMA-SP] Send Message Command Failed. Status: 0x%08X\n", (unsigned int)Status);
        OS_printf("[ROMA-SP] Sended Message: %s", Msg->Payload.msg);
    }

    PAYUEL_ROMA_HandleReport((Status >= 0) ? CFE_SUCCESS : CFE_STATUS_EXTERNAL_RESOURCE_FAIL,
                             PAYUEL_ROMA_SEND_MSG_CC, NULL, 0);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SYNC_RX:                                                                   */
/*   Sync the RX parameters from the parameters table                         */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_SyncRxCmd(const PAYUEL_ROMA_SyncRxCmd_t *Msg)
{
    uint8 TxBuf[3] = {0};

    TxBuf[0] = 1;  // type
    TxBuf[1] = 32; // cmd
    TxBuf[2] = 5;   // sub

    int32 Status = CFE_SRL_ApiTransactionCSP(CSP_NODE_ROMA, 8, TxBuf, sizeof(TxBuf), NULL, 0);

    if (Status >= 0)
    {
        OS_printf("[ROMA-SP] Sync RX Command Success.\n");
    }
    else
    {
        OS_printf("[ROMA-SP] Sync RX Command Failed. Status: 0x%08X\n", (unsigned int)Status);
    }

    PAYUEL_ROMA_HandleReport((Status >= 0) ? CFE_SUCCESS : CFE_STATUS_EXTERNAL_RESOURCE_FAIL,
                             PAYUEL_ROMA_SYNC_RX_CC, NULL, 0);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SYNC_TX:                                                                   */
/*   Sync the TX parameters from the parameters table - The radio will be     */
/*   left in the TX configuration, so that reception will happen also         */
/*   with the TX settings                                                     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_SyncTxCmd(const PAYUEL_ROMA_SyncTxCmd_t *Msg)
{
    uint8 TxBuf[3] = {0};

    TxBuf[0] = 1;  // type
    TxBuf[1] = 32; // cmd
    TxBuf[2] = 6;   // sub

    int32 Status = CFE_SRL_ApiTransactionCSP(CSP_NODE_ROMA, 8, TxBuf, sizeof(TxBuf), NULL, 0);

    if (Status >= 0)
    {
        OS_printf("[ROMA-SP] Sync TX Command Success.\n");
    }
    else
    {
        OS_printf("[ROMA-SP] Sync TX Command Failed. Status: 0x%08X\n", (unsigned int)Status);
    }

    PAYUEL_ROMA_HandleReport((Status >= 0) ? CFE_SUCCESS : CFE_STATUS_EXTERNAL_RESOURCE_FAIL,
                             PAYUEL_ROMA_SYNC_TX_CC, NULL, 0);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAY_INIT:                                                                  */
/*   Initializes the module (same as 'pay lora')                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_ROMA_PayInitCmd(const PAYUEL_ROMA_PayInitCmd_t *Msg)
{
    uint8 TxBuf[3] = {0};

    TxBuf[0] = 1;  // type
    TxBuf[1] = 32; // cmd
    TxBuf[2] = 7;   // sub

    int32 Status = CFE_SRL_ApiTransactionCSP(CSP_NODE_ROMA, 8, TxBuf, sizeof(TxBuf), NULL, 0);

    if (Status >= 0)
    {
        OS_printf("[ROMA-SP] Payload Initialization Command Success.\n");
    }
    else
    {
        OS_printf("[ROMA-SP] Payload Initialization Command Failed. Status: 0x%08X\n", (unsigned int)Status);
    }

    PAYUEL_ROMA_HandleReport((Status >= 0) ? CFE_SUCCESS : CFE_STATUS_EXTERNAL_RESOURCE_FAIL,
                             PAYUEL_ROMA_PAY_INIT_CC, NULL, 0);

    return CFE_SUCCESS;
}
