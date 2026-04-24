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
 *   This file contains the source code for the PAYUEL LGPM Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "PAYUEL_LGPM_app.h"
#include "PAYUEL_LGPM_cmds.h"
#include "PAYUEL_LGPM_msgids.h"
#include "PAYUEL_LGPM_eventids.h"
#include "PAYUEL_LGPM_version.h"
#include "PAYUEL_LGPM_tbl.h"
#include "PAYUEL_LGPM_utils.h"
#include "PAYUEL_LGPM_msg.h"

/* The PAYUEL_LGPM_lib module provides the PAYUEL_LGPM_Function() prototype */
// #include "PAYUEL_LGPM_lib.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t PAYUEL_LGPM_SendHkCmd(const PAYUEL_LGPM_SendHkCmd_t *Msg)
{
    int i;

    /*
    ** Get command execution counters...
    */
    PAYUEL_LGPM_Data.HkTlm.Payload.CommandErrorCounter = PAYUEL_LGPM_Data.ErrCounter;
    PAYUEL_LGPM_Data.HkTlm.Payload.CommandCounter      = PAYUEL_LGPM_Data.CmdCounter;

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(PAYUEL_LGPM_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(PAYUEL_LGPM_Data.HkTlm.TelemetryHeader), true);

    /*
    ** Manage any pending table loads, validations, etc.
    */
    for (i = 0; i < PAYUEL_LGPM_NUMBER_OF_TABLES; i++)
    {
        CFE_TBL_Manage(PAYUEL_LGPM_Data.TblHandles[i]);
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUEL_LGPM NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t PAYUEL_LGPM_NoopCmd(const PAYUEL_LGPM_NoopCmd_t *Msg)
{
    PAYUEL_LGPM_Data.CmdCounter++;

    CFE_EVS_SendEvent(PAYUEL_LGPM_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "PAYUEL_LGPM : NOOP command %s",
                      PAYUEL_LGPM_VERSION);
    

    return CFE_SUCCESS;
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t PAYUEL_LGPM_ResetCountersCmd(const PAYUEL_LGPM_ResetCountersCmd_t *Msg)
{
    PAYUEL_LGPM_Data.CmdCounter = 0;
    PAYUEL_LGPM_Data.ErrCounter = 0;

    CFE_SRL_IO_Handle_t *RS422 = CFE_SRL_ApiGetHandle(CFE_SRL_RS422_HANDLE_INDEXER);
    CFE_SRL_IO_Param_t param = {0,};

    uint8_t TxBuf[12] = {0};
    uint8_t RxBuf[12] = {0};
    memcpy(TxBuf, "ILOVEMOZART", 12);
    param.TxData = TxBuf;
    param.TxSize = 12;
    param.RxData = RxBuf;
    param.RxSize = sizeof(RxBuf);
    param.Interval = 5000; // empirical value - 2000 has an error in stm32 slave

    int32 Status = CFE_SRL_ApiRead(RS422, &param);
    OS_printf("SPI Transaction Status: 0x%08X\n", Status);
    if (Status == CFE_SUCCESS) {
        OS_printf("Rx Data: %s\n", RxBuf);
    }

    CFE_EVS_SendEvent(PAYUEL_LGPM_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "PAYUEL_LGPM: RESET command");

    return CFE_SUCCESS;
}


CFE_Status_t PAYUEL_LGPM_MCU_ALIVE_CHECK_Cmd(const PAYUEL_LGPM_MCU_ALIVE_CHECK_Cmd_t *Msg) 
{
    PAYUEL_LGPM_Data.CmdCounter++;

    uint32 UTCtime = Msg->Ground_UTC_Time;

    // TX 데이터 준비
    PAYUEL_LGPM_OBC2Payload_MCU_ALIVE_CHECK_Payload_t txdata;
    memset(&txdata, 0, sizeof(txdata));
    
    // ICD에 맞게 정보할당
    txdata.header.cmd_code = PWR_cmd_code;
    txdata.header.sync = 0x45;
    txdata.header.length = 5;
    txdata.MCU_ALIVE_CHECK = 0x01;
    txdata.SYNC_Timestamp = UTCtime;

    uint16 calc_length = sizeof(txdata) - sizeof(txdata.CRC16) - sizeof(txdata.header);
    txdata.CRC16 = Usart6_CalculateCRC16((uint8_t *)&txdata.MCU_ALIVE_CHECK, calc_length);

    // RX 데이터 및 파라미터 초기화
    PAYUEL_LGPM_Payload2OBC_MCU_ALIVE_CHECK_Payload_t rxdata;
    memset(&rxdata, 0, sizeof(rxdata));

    CFE_SRL_IO_Param_t Param;
    memset(&Param, 0, sizeof(Param));

    // 명령 송신(TX) + 헤더 수신(RX 3바이트)
    Param.TxData   = (uint8_t *)&txdata;           
    Param.TxSize   = sizeof(txdata); 
    Param.RxData   = (uint8_t *)&rxdata.header;
    Param.RxSize   = sizeof(rxdata.header);
    Param.Timeout  = 150;
    Param.Interval = 100;

    int32 status = CFE_SRL_ApiRead( PAYUEL_LGPM_Data.Handle, &Param );

    if (status == CFE_SUCCESS) 
    {
        uint8_t dynamic_length = rxdata.header.length; 
        uint16_t remaining_size = dynamic_length + 2;

        uint8_t raw_buffer[300];
        memset(raw_buffer, 0, sizeof(raw_buffer));

        // TX 없이 남은 데이터만 추가 수신
        // (명령이 중복 송신되지 않도록 TxSize를 0으로 설정)
        Param.TxData = NULL;
        Param.TxSize = 0;
        Param.RxData = raw_buffer;
        Param.RxSize = remaining_size;
        Param.Timeout  = 150;
        Param.Interval = 100;
        
        status = CFE_SRL_ApiRead(PAYUEL_LGPM_Data.Handle, &Param);

        if (status == CFE_SUCCESS) 
        {
            // CMD INFO + Timesatamp 메모리 복사 
            rxdata.MCU_ALIVE_CHECK = raw_buffer[0];
            memcpy(&rxdata.Execution_Timestamp, &raw_buffer[1], 4);
            
            // dynamic length : CMD INFO(1) + Timesatamp(4) + Reply_Message
            // msg_len : Reply_Message
            uint8_t msg_len = dynamic_length - 5;

            if (msg_len <= sizeof(rxdata.Reply_Message)) 
            {
                memcpy(rxdata.Reply_Message, &raw_buffer[5], msg_len);
                
            }
            
            memcpy(&rxdata.CRC16, &raw_buffer[dynamic_length], 2);
            
            if (rxdata.MCU_ALIVE_CHECK != 0x01) 
            {
                CFE_EVS_SendEvent(PAYUEL_LGPM_MCU_CHECK_ALIVE_ERR_EID, CFE_EVS_EventType_ERROR, 
                                "Alive Check Failed: Wrong payload (0x%02X)", rxdata.MCU_ALIVE_CHECK);
            }
            else 
            {
                OS_printf("======================================\n");
                OS_printf(" 페이로드 응답 수신 성공!\n");
                OS_printf("  - Sync: 0x%02X\n", rxdata.header.sync);
                OS_printf("  - Alive Check: 0x%02X\n", rxdata.MCU_ALIVE_CHECK);
                OS_printf("  - Timestamp: %u\n", (unsigned int)rxdata.Execution_Timestamp);
                OS_printf("  - Reply Msg: %.*s\n", (int)msg_len, rxdata.Reply_Message);
                OS_printf("======================================\n");
                
                // Send_To_Report_App(&rxdata);
            }
        }
        else 
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_MCU_CHECK_ALIVE_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "UART Body(CMD_INFO + Time_stamp + Reply_Message) Read Failed! Status: 0x%08X", (unsigned int)status);
        }
    }
    else 
    {
        CFE_EVS_SendEvent(PAYUEL_LGPM_MCU_CHECK_ALIVE_ERR_EID, CFE_EVS_EventType_ERROR, 
                          "UART Header(Sync + length) Read Failed or Timeout! Status: 0x%08X", (unsigned int)status);
    }
    
    return CFE_SUCCESS; 
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUEL_LGPM 3V3 PWR ON command                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_LGPM_3V3_PWR_ON_Cmd(const PAYUEL_LGPM_3V3_PWR_ON_Cmd_t *Msg) 
{
    PAYUEL_LGPM_Data.CmdCounter++;

    // 송신 구조체 선언 및 초기화     
    PAYUEL_LGPM_OBC2Payload_3V3_PWR_ON_Payload_t  txdata;
    memset(&txdata, 0, sizeof(txdata));
    
    // ICD에 맞게 정보 할당 (Payload 크기는 PWR_ON_3V3 1바이트이므로 length=1)
    txdata.header.cmd_code = PWR_cmd_code;
    txdata.header.sync = 0x45;
    txdata.header.length = 1; 
    txdata.PWR_ON_3V3 = 0x10; 

    uint16 calc_length = sizeof(txdata) - sizeof(txdata.CRC16) - sizeof(txdata.header);
    txdata.CRC16 = Usart6_CalculateCRC16((uint8_t *)&txdata.PWR_ON_3V3, calc_length);

    // 수신 구조체 선언 및 초기화
    PAYUEL_LGPM_Payload2OBC_3V3_PWR_ON_Payload_t  rxdata;
    memset(&rxdata, 0, sizeof(rxdata)); 

    // UART 통신을 위한 파라미터 구조체 초기화
    CFE_SRL_IO_Param_t Param;
    memset(&Param, 0, sizeof(Param));

    // 명령 송신(TX) + 헤더 수신(RX 3바이트)
    Param.TxData   = (uint8_t *)&txdata;           
    Param.TxSize   = sizeof(txdata);     
    Param.RxData   = (uint8_t *)&rxdata.header;
    Param.RxSize   = sizeof(rxdata.header);    
    Param.Timeout  = 250;   
    Param.Interval = 100;

    int32 status = CFE_SRL_ApiRead( PAYUEL_LGPM_Data.Handle, &Param );

    if (status == CFE_SUCCESS) 
    {
        uint8_t dynamic_length = rxdata.header.length; 
        uint16_t remaining_size = dynamic_length + 2;

        uint8_t raw_buffer[300];
        memset(raw_buffer, 0, sizeof(raw_buffer));

        // TX 없이 남은 데이터만 추가 수신
        // (명령이 중복 송신되지 않도록 TxSize를 0으로 설정)
        Param.TxData = NULL;
        Param.TxSize = 0;
        Param.RxData = raw_buffer;
        Param.RxSize = remaining_size;
        Param.Timeout  = 150;
        Param.Interval = 100; 
        
        status = CFE_SRL_ApiRead(PAYUEL_LGPM_Data.Handle, &Param);

        if (status == CFE_SUCCESS) 
        {
            // CMD INFO + Timesatamp 메모리 복사 
            rxdata.PWR_ON_3V3 = raw_buffer[0];
            memcpy(&rxdata.Execution_Timestamp, &raw_buffer[1], 4);
            
            // dynamic length : CMD INFO(1) + Timesatamp(4) + Reply_Message
            // msg_len : Reply_Message
            uint8_t msg_len = dynamic_length - 5;

            if (msg_len <= sizeof(rxdata.Reply_Message)) 
            {
                memcpy(rxdata.Reply_Message, &raw_buffer[5], msg_len);
                
            }
            
            memcpy(&rxdata.CRC16, &raw_buffer[dynamic_length], 2);
            
            OS_printf("======================================\n");
            OS_printf(" 페이로드 응답 수신 성공 (3V3 PWR ON)\n");
            OS_printf("  - Sync: 0x%02X\n", rxdata.header.sync);
            OS_printf("  - 3V3 Status: 0x%02X\n", rxdata.PWR_ON_3V3);
            OS_printf("  - Timestamp: %u\n", (unsigned int)rxdata.Execution_Timestamp);
            OS_printf("  - Reply Msg: %.*s\n", (int)msg_len, rxdata.Reply_Message);
            OS_printf("======================================\n");
        }
        else 
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "UART Body(CMD_INFO + Time_stamp + Reply_Message) Read Failed! Status: 0x%08X", (unsigned int)status);
        }
    }
    else 
    {
        CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                          "UART Header(Sync + length) Read Failed or Timeout! Status: 0x%08X", (unsigned int)status);
    }

    return CFE_SUCCESS; 
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUEL_LGPM 3V3 PWR OFF command                                            */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_LGPM_3V3_PWR_OFF_Cmd(const PAYUEL_LGPM_3V3_PWR_OFF_Cmd_t *Msg) 
{
    PAYUEL_LGPM_Data.CmdCounter++;

    // TX 데이터 준비
    PAYUEL_LGPM_OBC2Payload_3V3_PWR_OFF_Payload_t txdata;
    memset(&txdata, 0, sizeof(txdata));
    
    txdata.header.cmd_code = PWR_cmd_code;
    txdata.header.sync = 0x45;
    txdata.header.length = 1; 
    txdata.PWR_OFF_3V3 = 0x11; 

    uint16 calc_length = sizeof(txdata) - sizeof(txdata.CRC16) - sizeof(txdata.header);
    txdata.CRC16 = Usart6_CalculateCRC16((uint8_t *)&txdata.PWR_OFF_3V3, calc_length);

    // RX 데이터 및 파라미터 초기화
    PAYUEL_LGPM_Payload2OBC_3V3_PWR_OFF_Payload_t rxdata;
    memset(&rxdata, 0, sizeof(rxdata));

    CFE_SRL_IO_Param_t Param;
    memset(&Param, 0, sizeof(Param));

    // 명령 송신(TX) + 헤더 수신(RX 3바이트)
    Param.TxData   = (uint8_t *)&txdata;           
    Param.TxSize   = sizeof(txdata); 
    Param.RxData   = (uint8_t *)&rxdata.header;
    Param.RxSize   = sizeof(rxdata.header);
    Param.Timeout  = 200;
    Param.Interval = 100;

    int32 status = CFE_SRL_ApiRead( PAYUEL_LGPM_Data.Handle, &Param );

    if (status == CFE_SUCCESS) 
    {
        uint8_t dynamic_length = rxdata.header.length; 
        uint16_t remaining_size = dynamic_length + 2;

        uint8_t raw_buffer[300];
        memset(raw_buffer, 0, sizeof(raw_buffer));

        // TX 없이 남은 데이터만 추가 수신
        // (명령이 중복 송신되지 않도록 TxSize를 0으로 설정)
        Param.TxData = NULL;
        Param.TxSize = 0;
        Param.RxData = raw_buffer;
        Param.RxSize = remaining_size;
        Param.Timeout  = 150;
        Param.Interval = 100;  
        
        status = CFE_SRL_ApiRead(PAYUEL_LGPM_Data.Handle, &Param);

        if (status == CFE_SUCCESS) 
        {
            // CMD INFO + Timesatamp 메모리 복사 
            rxdata.PWR_OFF_3V3 = raw_buffer[0];
            memcpy(&rxdata.Execution_Timestamp, &raw_buffer[1], 4);
            
            // dynamic length : CMD INFO(1) + Timesatamp(4) + Reply_Message
            // msg_len : Reply_Message
            uint8_t msg_len = dynamic_length - 5;

            if (msg_len <= sizeof(rxdata.Reply_Message)) 
            {
                memcpy(rxdata.Reply_Message, &raw_buffer[5], msg_len);
                
            }
            
            memcpy(&rxdata.CRC16, &raw_buffer[dynamic_length], 2);
            
            OS_printf("======================================\n");
            OS_printf(" 페이로드 응답 수신 성공 (3V3 PWR OFF)\n");
            OS_printf("  - Sync: 0x%02X\n", rxdata.header.sync);
            OS_printf("  - Status: 0x%02X\n", rxdata.PWR_OFF_3V3);
            OS_printf("  - Timestamp: %u\n", (unsigned int)rxdata.Execution_Timestamp);
            OS_printf("  - Reply Msg: %.*s\n", (int)msg_len, rxdata.Reply_Message);
            OS_printf("======================================\n");
        }
        else 
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "UART Body(CMD_INFO + Time_stamp + Reply_Message) Read Failed! Status: 0x%08X", (unsigned int)status);
        }
    }
    else 
    {
        CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                          "UART Header(Sync + length) Read Failed or Timeout! Status: 0x%08X", (unsigned int)status);
    }
    
    return CFE_SUCCESS; 
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* */
/* PAYUEL_LGPM MAIN BOOST SW ON command                                       */
/* */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_LGPM_MAIN_BOOST_SW_ON_Cmd(const PAYUEL_LGPM_MAIN_BOOST_SW_ON_Cmd_t *Msg) 
{
    PAYUEL_LGPM_Data.CmdCounter++;

    // TX 데이터 준비
    PAYUEL_LGPM_OBC2Payload_MAIN_BOOST_SW_ON_Payload_t txdata;
    memset(&txdata, 0, sizeof(txdata));
    
    txdata.header.cmd_code = PWR_cmd_code;
    txdata.header.sync = 0x45;
    txdata.header.length = 1; 
    txdata.MAIN_BOOST_SW_ON = 0x12; 

    uint16 calc_length = sizeof(txdata) - sizeof(txdata.CRC16) - sizeof(txdata.header);
    txdata.CRC16 = Usart6_CalculateCRC16((uint8_t *)&txdata.MAIN_BOOST_SW_ON, calc_length);

    // RX 데이터 및 파라미터 초기화
    PAYUEL_LGPM_Payload2OBC_MAIN_BOOST_SW_ON_Payload_t rxdata;
    memset(&rxdata, 0, sizeof(rxdata));

    CFE_SRL_IO_Param_t Param;
    memset(&Param, 0, sizeof(Param));

    // 명령 송신(TX) + 헤더 수신(RX 3바이트)
    Param.TxData   = (uint8_t *)&txdata;           
    Param.TxSize   = sizeof(txdata); 
    Param.RxData   = (uint8_t *)&rxdata.header;
    Param.RxSize   = sizeof(rxdata.header);
    Param.Timeout  = 2500;  
    Param.Interval = 500;  

    int32 status = CFE_SRL_ApiRead( PAYUEL_LGPM_Data.Handle, &Param );

    if (status == CFE_SUCCESS) 
    {
        uint8_t dynamic_length = rxdata.header.length; 
        uint16_t remaining_size = dynamic_length + 2;

        uint8_t raw_buffer[300];
        memset(raw_buffer, 0, sizeof(raw_buffer));

        // TX 없이 남은 데이터만 추가 수신
        // (명령이 중복 송신되지 않도록 TxSize를 0으로 설정)
        Param.TxData = NULL;
        Param.TxSize = 0;
        Param.RxData = raw_buffer;
        Param.RxSize = remaining_size; 
        Param.Timeout  = 150;
        Param.Interval = 100; 
        
        status = CFE_SRL_ApiRead(PAYUEL_LGPM_Data.Handle, &Param);

        if (status == CFE_SUCCESS) 
        {
            // CMD INFO + Timesatamp 메모리 복사 
            rxdata.MAIN_BOOST_SW_ON = raw_buffer[0];
            memcpy(&rxdata.Execution_Timestamp, &raw_buffer[1], 4);
            
            // dynamic length : CMD INFO(1) + Timesatamp(4) + Reply_Message
            // msg_len : Reply_Message
            uint8_t msg_len = dynamic_length - 5;

            if (msg_len <= sizeof(rxdata.Reply_Message)) 
            {
                memcpy(rxdata.Reply_Message, &raw_buffer[5], msg_len);
                
            }
            
            memcpy(&rxdata.CRC16, &raw_buffer[dynamic_length], 2);
            
            OS_printf("======================================\n");
            OS_printf(" 페이로드 응답 수신 성공 (MAIN BOOST SW ON)\n");
            OS_printf("  - Sync: 0x%02X\n", rxdata.header.sync);
            OS_printf("  - Status: 0x%02X\n", rxdata.MAIN_BOOST_SW_ON);
            OS_printf("  - Timestamp: %u\n", (unsigned int)rxdata.Execution_Timestamp);
            OS_printf("  - Reply Msg: %.*s\n", (int)msg_len, rxdata.Reply_Message);
            OS_printf("======================================\n");
        }
        else 
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "UART Body(CMD_INFO + Time_stamp + Reply_Message) Read Failed! Status: 0x%08X", (unsigned int)status);
        }
    }
    else 
    {
        CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                          "UART Header(Sync + length) Read Failed or Timeout! Status: 0x%08X", (unsigned int)status);
    }
    
    return CFE_SUCCESS; 
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* */
/* PAYUEL_LGPM MAIN BOOST SW OFF command                                      */
/* */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_LGPM_MAIN_BOOST_SW_OFF_Cmd(const PAYUEL_LGPM_MAIN_BOOST_SW_OFF_Cmd_t *Msg) 
{
    PAYUEL_LGPM_Data.CmdCounter++;

    // TX 데이터 준비
    PAYUEL_LGPM_OBC2Payload_MAIN_BOOST_SW_OFF_Payload_t txdata;
    memset(&txdata, 0, sizeof(txdata));
    
    txdata.header.cmd_code = PWR_cmd_code;
    txdata.header.sync = 0x45;
    txdata.header.length = 1; 
    txdata.MAIN_BOOST_SW_OFF = 0x13; 

    uint16 calc_length = sizeof(txdata) - sizeof(txdata.CRC16) - sizeof(txdata.header);
    txdata.CRC16 = Usart6_CalculateCRC16((uint8_t *)&txdata.MAIN_BOOST_SW_OFF, calc_length);

    // RX 데이터 및 파라미터 초기화
    PAYUEL_LGPM_Payload2OBC_MAIN_BOOST_SW_OFF_Payload_t rxdata;
    memset(&rxdata, 0, sizeof(rxdata));

    CFE_SRL_IO_Param_t Param;
    memset(&Param, 0, sizeof(Param));

    // 명령 송신(TX) + 헤더 수신(RX 3바이트)
    Param.TxData   = (uint8_t *)&txdata;           
    Param.TxSize   = sizeof(txdata); 
    Param.RxData   = (uint8_t *)&rxdata.header;
    Param.RxSize   = sizeof(rxdata.header);
    Param.Timeout  = 280;
    Param.Interval = 100;


    int32 status = CFE_SRL_ApiRead( PAYUEL_LGPM_Data.Handle, &Param );

    if (status == CFE_SUCCESS) 
    {
        uint8_t dynamic_length = rxdata.header.length; 
        uint16_t remaining_size = dynamic_length + 2;

        uint8_t raw_buffer[300];
        memset(raw_buffer, 0, sizeof(raw_buffer));

        // TX 없이 남은 데이터만 추가 수신
        // (명령이 중복 송신되지 않도록 TxSize를 0으로 설정)
        Param.TxData = NULL;
        Param.TxSize = 0;
        Param.RxData = raw_buffer;
        Param.RxSize = remaining_size;
        Param.Timeout  = 150;
        Param.Interval = 100;  
        

        status = CFE_SRL_ApiRead(PAYUEL_LGPM_Data.Handle, &Param);

        if (status == CFE_SUCCESS) 
        {
            // CMD INFO + Timesatamp 메모리 복사 
            rxdata.MAIN_BOOST_SW_OFF = raw_buffer[0];
            memcpy(&rxdata.Execution_Timestamp, &raw_buffer[1], 4);
            
            // dynamic length : CMD INFO(1) + Timesatamp(4) + Reply_Message
            // msg_len : Reply_Message
            uint8_t msg_len = dynamic_length - 5;

            if (msg_len <= sizeof(rxdata.Reply_Message)) 
            {
                memcpy(rxdata.Reply_Message, &raw_buffer[5], msg_len);
                
            }
            
            memcpy(&rxdata.CRC16, &raw_buffer[dynamic_length], 2);
            
            OS_printf("======================================\n");
            OS_printf(" 페이로드 응답 수신 성공 (MAIN BOOST SW OFF)\n");
            OS_printf("  - Sync: 0x%02X\n", rxdata.header.sync);
            OS_printf("  - Status: 0x%02X\n", rxdata.MAIN_BOOST_SW_OFF);
            OS_printf("  - Timestamp: %u\n", (unsigned int)rxdata.Execution_Timestamp);
            OS_printf("  - Reply Msg: %.*s\n", (int)msg_len, rxdata.Reply_Message);
            OS_printf("======================================\n");
        }
        else 
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "UART Body(CMD_INFO + Time_stamp + Reply_Message) Read Failed! Status: 0x%08X", (unsigned int)status);
        }
    }
    else 
    {
        CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                          "UART Header(Sync + length) Read Failed or Timeout! Status: 0x%08X", (unsigned int)status);
    }
    
    return CFE_SUCCESS; 
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* */
/* PAYUEL_LGPM SUB BOOST SW ON command                                        */
/* */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_LGPM_SUB_BOOST_SW_ON_Cmd(const PAYUEL_LGPM_SUB_BOOST_SW_ON_Cmd_t *Msg) 
{
    PAYUEL_LGPM_Data.CmdCounter++;

    // TX 데이터 준비
    PAYUEL_LGPM_OBC2Payload_SUB_BOOST_SW_ON_Payload_t txdata;
    memset(&txdata, 0, sizeof(txdata));
    
    txdata.header.cmd_code = PWR_cmd_code;
    txdata.header.sync = 0x45;
    txdata.header.length = 1; 
    txdata.SUB_BOOST_SW_ON = 0x14; 

    uint16 calc_length = sizeof(txdata) - sizeof(txdata.CRC16) - sizeof(txdata.header);
    txdata.CRC16 = Usart6_CalculateCRC16((uint8_t *)&txdata.SUB_BOOST_SW_ON, calc_length);

    // RX 데이터 및 파라미터 초기화
    PAYUEL_LGPM_Payload2OBC_SUB_BOOST_SW_ON_Payload_t rxdata;
    memset(&rxdata, 0, sizeof(rxdata));

    CFE_SRL_IO_Param_t Param;
    memset(&Param, 0, sizeof(Param));

    // 명령 송신(TX) + 헤더 수신(RX 3바이트)
    Param.TxData   = (uint8_t *)&txdata;           
    Param.TxSize   = sizeof(txdata); 
    Param.RxData   = (uint8_t *)&rxdata.header;
    Param.RxSize   = sizeof(rxdata.header);
    Param.Timeout  = 280;
    Param.Interval = 100;

    int32 status = CFE_SRL_ApiRead( PAYUEL_LGPM_Data.Handle, &Param );

    if (status == CFE_SUCCESS) 
    {
        uint8_t dynamic_length = rxdata.header.length; 
        uint16_t remaining_size = dynamic_length + 2;

        uint8_t raw_buffer[300];
        memset(raw_buffer, 0, sizeof(raw_buffer));

        // TX 없이 남은 데이터만 추가 수신
        // (명령이 중복 송신되지 않도록 TxSize를 0으로 설정)
        Param.TxData = NULL;
        Param.TxSize = 0;
        Param.RxData = raw_buffer;
        Param.RxSize = remaining_size; 
        Param.Timeout  = 150;
        Param.Interval = 100; 

        status = CFE_SRL_ApiRead(PAYUEL_LGPM_Data.Handle, &Param);

        if (status == CFE_SUCCESS) 
        {
            // CMD INFO + Timesatamp 메모리 복사 
            rxdata.SUB_BOOST_SW_ON = raw_buffer[0];
            memcpy(&rxdata.Execution_Timestamp, &raw_buffer[1], 4);
            
            // dynamic length : CMD INFO(1) + Timesatamp(4) + Reply_Message
            // msg_len : Reply_Message
            uint8_t msg_len = dynamic_length - 5;

            if (msg_len <= sizeof(rxdata.Reply_Message)) 
            {
                memcpy(rxdata.Reply_Message, &raw_buffer[5], msg_len);
                
            }
            
            memcpy(&rxdata.CRC16, &raw_buffer[dynamic_length], 2);
            
            OS_printf("======================================\n");
            OS_printf(" 페이로드 응답 수신 성공 (SUB BOOST SW ON)\n");
            OS_printf("  - Sync: 0x%02X\n", rxdata.header.sync);
            OS_printf("  - Status: 0x%02X\n", rxdata.SUB_BOOST_SW_ON);
            OS_printf("  - Timestamp: %u\n", (unsigned int)rxdata.Execution_Timestamp);
            OS_printf("  - Reply Msg: %.*s\n", (int)msg_len, rxdata.Reply_Message);
            OS_printf("======================================\n");
        }
        else 
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "UART Body(CMD_INFO + Time_stamp + Reply_Message) Read Failed! Status: 0x%08X", (unsigned int)status);
        }
    }
    else 
    {
        CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                          "UART Header(Sync + length) Read Failed or Timeout! Status: 0x%08X", (unsigned int)status);
    }
    
    return CFE_SUCCESS; 
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* */
/* PAYUEL_LGPM SUB BOOST SW OFF command                                       */
/* */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_LGPM_SUB_BOOST_SW_OFF_Cmd(const PAYUEL_LGPM_SUB_BOOST_SW_OFF_Cmd_t *Msg) 
{
    PAYUEL_LGPM_Data.CmdCounter++;

    // TX 데이터 준비
    PAYUEL_LGPM_OBC2Payload_SUB_BOOST_SW_OFF_Payload_t txdata;
    memset(&txdata, 0, sizeof(txdata));
    
    txdata.header.cmd_code = PWR_cmd_code;
    txdata.header.sync = 0x45;
    txdata.header.length = 1; 
    txdata.SUB_BOOST_SW_OFF = 0x15; 

    uint16 calc_length = sizeof(txdata) - sizeof(txdata.CRC16) - sizeof(txdata.header);
    txdata.CRC16 = Usart6_CalculateCRC16((uint8_t *)&txdata.SUB_BOOST_SW_OFF, calc_length);

    // RX 데이터 및 파라미터 초기화
    PAYUEL_LGPM_Payload2OBC_SUB_BOOST_SW_OFF_Payload_t rxdata;
    memset(&rxdata, 0, sizeof(rxdata));

    CFE_SRL_IO_Param_t Param;
    memset(&Param, 0, sizeof(Param));

    // 명령 송신(TX) + 헤더 수신(RX 3바이트)
    Param.TxData   = (uint8_t *)&txdata;           
    Param.TxSize   = sizeof(txdata); 
    Param.RxData   = (uint8_t *)&rxdata.header;
    Param.RxSize   = sizeof(rxdata.header);
    Param.Timeout  = 280;
    Param.Interval = 100;

    int32 status = CFE_SRL_ApiRead( PAYUEL_LGPM_Data.Handle, &Param );

    if (status == CFE_SUCCESS) 
    {
        uint8_t dynamic_length = rxdata.header.length; 
        uint16_t remaining_size = dynamic_length + 2;

        uint8_t raw_buffer[300];
        memset(raw_buffer, 0, sizeof(raw_buffer));

        // TX 없이 남은 데이터만 추가 수신
        // (명령이 중복 송신되지 않도록 TxSize를 0으로 설정)
        Param.TxData = NULL;
        Param.TxSize = 0;
        Param.RxData = raw_buffer;
        Param.RxSize = remaining_size;
        Param.Timeout  = 150;
        Param.Interval = 100;  
        
        status = CFE_SRL_ApiRead(PAYUEL_LGPM_Data.Handle, &Param);

        if (status == CFE_SUCCESS) 
        {
            // CMD INFO + Timesatamp 메모리 복사 
            rxdata.SUB_BOOST_SW_OFF = raw_buffer[0];
            memcpy(&rxdata.Execution_Timestamp, &raw_buffer[1], 4);
            
            // dynamic length : CMD INFO(1) + Timesatamp(4) + Reply_Message
            // msg_len : Reply_Message
            uint8_t msg_len = dynamic_length - 5;

            if (msg_len <= sizeof(rxdata.Reply_Message)) 
            {
                memcpy(rxdata.Reply_Message, &raw_buffer[5], msg_len);
                
            }
            
            memcpy(&rxdata.CRC16, &raw_buffer[dynamic_length], 2);
            
            OS_printf("======================================\n");
            OS_printf(" 페이로드 응답 수신 성공 (SUB BOOST SW OFF)\n");
            OS_printf("  - Sync: 0x%02X\n", rxdata.header.sync);
            OS_printf("  - Status: 0x%02X\n", rxdata.SUB_BOOST_SW_OFF);
            OS_printf("  - Timestamp: %u\n", (unsigned int)rxdata.Execution_Timestamp);
            OS_printf("  - Reply Msg: %.*s\n", (int)msg_len, rxdata.Reply_Message);
            OS_printf("======================================\n");
        }
        else 
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "UART Body(CMD_INFO + Time_stamp + Reply_Message) Read Failed! Status: 0x%08X", (unsigned int)status);
        }
    }
    else 
    {
        CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                          "UART Header(Sync + length) Read Failed or Timeout! Status: 0x%08X", (unsigned int)status);
    }
    
    return CFE_SUCCESS; 
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* */
/* PAYUEL_LGPM V28 MAIN ON command                                            */
/* */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_LGPM_V28_MAIN_ON_Cmd(const PAYUEL_LGPM_V28_MAIN_ON_Cmd_t *Msg) 
{
    PAYUEL_LGPM_Data.CmdCounter++;

    // TX 데이터 준비
    PAYUEL_LGPM_OBC2Payload_V28_MAIN_ON_Payload_t txdata;
    memset(&txdata, 0, sizeof(txdata));
    
    txdata.header.cmd_code = PWR_cmd_code;
    txdata.header.sync = 0x45;
    txdata.header.length = 1; 
    txdata.V28_MAIN_ON = 0x16; 

    uint16 calc_length = sizeof(txdata) - sizeof(txdata.CRC16) - sizeof(txdata.header);
    txdata.CRC16 = Usart6_CalculateCRC16((uint8_t *)&txdata.V28_MAIN_ON, calc_length);

    // RX 데이터 및 파라미터 초기화
    PAYUEL_LGPM_Payload2OBC_V28_MAIN_ON_Payload_t rxdata;
    memset(&rxdata, 0, sizeof(rxdata));

    CFE_SRL_IO_Param_t Param;
    memset(&Param, 0, sizeof(Param));

    // 명령 송신(TX) + 헤더 수신(RX 3바이트)
    Param.TxData   = (uint8_t *)&txdata;           
    Param.TxSize   = sizeof(txdata); 
    Param.RxData   = (uint8_t *)&rxdata.header;
    Param.RxSize   = sizeof(rxdata.header);
    Param.Timeout  = 2500; 
    Param.Interval = 100;

    int32 status = CFE_SRL_ApiRead( PAYUEL_LGPM_Data.Handle, &Param );

    if (status == CFE_SUCCESS) 
    {
        uint8_t dynamic_length = rxdata.header.length; 
        uint16_t remaining_size = dynamic_length + 2;

        uint8_t raw_buffer[300];
        memset(raw_buffer, 0, sizeof(raw_buffer));

        // TX 없이 남은 데이터만 추가 수신
        // (명령이 중복 송신되지 않도록 TxSize를 0으로 설정)
        Param.TxData = NULL;
        Param.TxSize = 0;
        Param.RxData = raw_buffer;
        Param.RxSize = remaining_size; 
        Param.Timeout  = 150;
        Param.Interval = 100; 
        
        status = CFE_SRL_ApiRead(PAYUEL_LGPM_Data.Handle, &Param);

        if (status == CFE_SUCCESS) 
        {
            // CMD INFO + Timesatamp 메모리 복사 
            rxdata.V28_MAIN_ON = raw_buffer[0];
            memcpy(&rxdata.Execution_Timestamp, &raw_buffer[1], 4);
            
            // dynamic length : CMD INFO(1) + Timesatamp(4) + Reply_Message
            // msg_len : Reply_Message
            uint8_t msg_len = dynamic_length - 5;

            if (msg_len <= sizeof(rxdata.Reply_Message)) 
            {
                memcpy(rxdata.Reply_Message, &raw_buffer[5], msg_len);
                
            }
            
            memcpy(&rxdata.CRC16, &raw_buffer[dynamic_length], 2);
            
            OS_printf("======================================\n");
            OS_printf(" 페이로드 응답 수신 성공 (V28 MAIN ON)\n");
            OS_printf("  - Sync: 0x%02X\n", rxdata.header.sync);
            OS_printf("  - Status: 0x%02X\n", rxdata.V28_MAIN_ON);
            OS_printf("  - Timestamp: %u\n", (unsigned int)rxdata.Execution_Timestamp);
            OS_printf("  - Reply Msg: %.*s\n", (int)msg_len, rxdata.Reply_Message); // %.14s 제한 해제
            OS_printf("======================================\n");
        }
        else 
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "UART Body(CMD_INFO + Time_stamp + Reply_Message) Read Failed! Status: 0x%08X", (unsigned int)status);
        }
    }
    else 
    {
        CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                          "UART Header(Sync + length) Read Failed or Timeout! Status: 0x%08X", (unsigned int)status);
    }
    
    return CFE_SUCCESS; 
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* */
/* PAYUEL_LGPM V28 MAIN OFF command                                           */
/* */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_LGPM_V28_MAIN_OFF_Cmd(const PAYUEL_LGPM_V28_MAIN_OFF_Cmd_t *Msg) 
{
    PAYUEL_LGPM_Data.CmdCounter++;

    // TX 데이터 준비
    PAYUEL_LGPM_OBC2Payload_V28_MAIN_OFF_Payload_t txdata;
    memset(&txdata, 0, sizeof(txdata));
    
    txdata.header.cmd_code = PWR_cmd_code;
    txdata.header.sync = 0x45;
    txdata.header.length = 1; 
    txdata.V28_MAIN_OFF = 0x17; 

    uint16 calc_length = sizeof(txdata) - sizeof(txdata.CRC16) - sizeof(txdata.header);
    txdata.CRC16 = Usart6_CalculateCRC16((uint8_t *)&txdata.V28_MAIN_OFF, calc_length);

    // RX 데이터 및 파라미터 초기화
    PAYUEL_LGPM_Payload2OBC_V28_MAIN_OFF_Payload_t rxdata;
    memset(&rxdata, 0, sizeof(rxdata));

    CFE_SRL_IO_Param_t Param;
    memset(&Param, 0, sizeof(Param));

    // 명령 송신(TX) + 헤더 수신(RX 3바이트)
    Param.TxData   = (uint8_t *)&txdata;           
    Param.TxSize   = sizeof(txdata); 
    Param.RxData   = (uint8_t *)&rxdata.header;
    Param.RxSize   = sizeof(rxdata.header);
    Param.Timeout  = 300;
    Param.Interval = 100;

    int32 status = CFE_SRL_ApiRead( PAYUEL_LGPM_Data.Handle, &Param );

    if (status == CFE_SUCCESS) 
    {
        uint8_t dynamic_length = rxdata.header.length; 
        uint16_t remaining_size = dynamic_length + 2;

        uint8_t raw_buffer[300];
        memset(raw_buffer, 0, sizeof(raw_buffer));

        // TX 없이 남은 데이터만 추가 수신
        // (명령이 중복 송신되지 않도록 TxSize를 0으로 설정)
        Param.TxData = NULL;
        Param.TxSize = 0;
        Param.RxData = raw_buffer;
        Param.RxSize = remaining_size;
        Param.Timeout  = 150;
        Param.Interval = 100;  
        
        status = CFE_SRL_ApiRead(PAYUEL_LGPM_Data.Handle, &Param);

        if (status == CFE_SUCCESS) 
        {
            // CMD INFO + Timesatamp 메모리 복사 
            rxdata.V28_MAIN_OFF = raw_buffer[0];
            memcpy(&rxdata.Execution_Timestamp, &raw_buffer[1], 4);
            
            // dynamic length : CMD INFO(1) + Timesatamp(4) + Reply_Message
            // msg_len : Reply_Message
            uint8_t msg_len = dynamic_length - 5;

            if (msg_len <= sizeof(rxdata.Reply_Message)) 
            {
                memcpy(rxdata.Reply_Message, &raw_buffer[5], msg_len);
                
            }
            
            memcpy(&rxdata.CRC16, &raw_buffer[dynamic_length], 2);
            
            OS_printf("======================================\n");
            OS_printf(" 페이로드 응답 수신 성공 (V28 MAIN OFF)\n");
            OS_printf("  - Sync: 0x%02X\n", rxdata.header.sync);
            OS_printf("  - Status: 0x%02X\n", rxdata.V28_MAIN_OFF);
            OS_printf("  - Timestamp: %u\n", (unsigned int)rxdata.Execution_Timestamp);
            OS_printf("  - Reply Msg: %.*s\n", (int)msg_len, rxdata.Reply_Message);
            OS_printf("======================================\n");
        }
        else 
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "UART Body(CMD_INFO + Time_stamp + Reply_Message) Read Failed! Status: 0x%08X", (unsigned int)status);
        }
    }
    else 
    {
        CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                          "UART Header(Sync + length) Read Failed or Timeout! Status: 0x%08X", (unsigned int)status);
    }
    
    return CFE_SUCCESS; 
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* */
/* PAYUEL_LGPM V28 SUB ON command                                             */
/* */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_LGPM_V28_SUB_ON_Cmd(const PAYUEL_LGPM_V28_SUB_ON_Cmd_t *Msg) 
{
    PAYUEL_LGPM_Data.CmdCounter++;

    // TX 데이터 준비
    PAYUEL_LGPM_OBC2Payload_V28_SUB_ON_Payload_t txdata;
    memset(&txdata, 0, sizeof(txdata));
    
    txdata.header.cmd_code = PWR_cmd_code;
    txdata.header.sync = 0x45;
    txdata.header.length = 1; 
    txdata.V28_SUB_ON = 0x18; 

    uint16 calc_length = sizeof(txdata) - sizeof(txdata.CRC16) - sizeof(txdata.header);
    txdata.CRC16 = Usart6_CalculateCRC16((uint8_t *)&txdata.V28_SUB_ON, calc_length);

    // RX 데이터 및 파라미터 초기화
    PAYUEL_LGPM_Payload2OBC_V28_SUB_ON_Payload_t rxdata;
    memset(&rxdata, 0, sizeof(rxdata));

    CFE_SRL_IO_Param_t Param;
    memset(&Param, 0, sizeof(Param));

    // 명령 송신(TX) + 헤더 수신(RX 3바이트)
    Param.TxData   = (uint8_t *)&txdata;           
    Param.TxSize   = sizeof(txdata); 
    Param.RxData   = (uint8_t *)&rxdata.header;
    Param.RxSize   = sizeof(rxdata.header);
    Param.Timeout  = 270;
    Param.Interval = 100;

    int32 status = CFE_SRL_ApiRead( PAYUEL_LGPM_Data.Handle, &Param );

    if (status == CFE_SUCCESS) 
    {
        uint8_t dynamic_length = rxdata.header.length; 
        uint16_t remaining_size = dynamic_length + 2;

        uint8_t raw_buffer[300];
        memset(raw_buffer, 0, sizeof(raw_buffer));

        // TX 없이 남은 데이터만 추가 수신
        // (명령이 중복 송신되지 않도록 TxSize를 0으로 설정)
        Param.TxData = NULL;
        Param.TxSize = 0;
        Param.RxData = raw_buffer;
        Param.RxSize = remaining_size; 
        Param.Timeout  = 150;
        Param.Interval = 100; 
        
        status = CFE_SRL_ApiRead(PAYUEL_LGPM_Data.Handle, &Param);

        if (status == CFE_SUCCESS) 
        {
            // CMD INFO + Timesatamp 메모리 복사 
            rxdata.V28_SUB_ON = raw_buffer[0];
            memcpy(&rxdata.Execution_Timestamp, &raw_buffer[1], 4);
            
            // dynamic length : CMD INFO(1) + Timesatamp(4) + Reply_Message
            // msg_len : Reply_Message
            uint8_t msg_len = dynamic_length - 5;

            if (msg_len <= sizeof(rxdata.Reply_Message)) 
            {
                memcpy(rxdata.Reply_Message, &raw_buffer[5], msg_len);
                
            }
            
            memcpy(&rxdata.CRC16, &raw_buffer[dynamic_length], 2);
            
            OS_printf("======================================\n");
            OS_printf(" 페이로드 응답 수신 성공 (V28 SUB ON)\n");
            OS_printf("  - Sync: 0x%02X\n", rxdata.header.sync);
            OS_printf("  - Status: 0x%02X\n", rxdata.V28_SUB_ON);
            OS_printf("  - Timestamp: %u\n", (unsigned int)rxdata.Execution_Timestamp);
            OS_printf("  - Reply Msg: %.*s\n", (int)msg_len, rxdata.Reply_Message);
            OS_printf("======================================\n");
        }
        else 
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "UART Body(CMD_INFO + Time_stamp + Reply_Message) Read Failed! Status: 0x%08X", (unsigned int)status);
        }
    }
    else 
    {
        CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                          "UART Header(Sync + length) Read Failed or Timeout! Status: 0x%08X", (unsigned int)status);
    }
    
    return CFE_SUCCESS; 
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* */
/* PAYUEL_LGPM V28 SUB OFF command                                            */
/* */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_LGPM_V28_SUB_OFF_Cmd(const PAYUEL_LGPM_V28_SUB_OFF_Cmd_t *Msg) 
{
    PAYUEL_LGPM_Data.CmdCounter++;

    // TX 데이터 준비
    PAYUEL_LGPM_OBC2Payload_V28_SUB_OFF_Payload_t txdata;
    memset(&txdata, 0, sizeof(txdata));
    
    txdata.header.cmd_code = PWR_cmd_code;
    txdata.header.sync = 0x45;
    txdata.header.length = 1; 
    txdata.V28_SUB_OFF = 0x19; 

    uint16 calc_length = sizeof(txdata) - sizeof(txdata.CRC16) - sizeof(txdata.header);
    txdata.CRC16 = Usart6_CalculateCRC16((uint8_t *)&txdata.V28_SUB_OFF, calc_length);

    // RX 데이터 및 파라미터 초기화
    PAYUEL_LGPM_Payload2OBC_V28_SUB_OFF_Payload_t rxdata;
    memset(&rxdata, 0, sizeof(rxdata));

    CFE_SRL_IO_Param_t Param;
    memset(&Param, 0, sizeof(Param));

    // 명령 송신(TX) + 헤더 수신(RX 3바이트)
    Param.TxData   = (uint8_t *)&txdata;           
    Param.TxSize   = sizeof(txdata); 
    Param.RxData   = (uint8_t *)&rxdata.header;
    Param.RxSize   = sizeof(rxdata.header);
    Param.Timeout  = 280;
    Param.Interval = 100;

    int32 status = CFE_SRL_ApiRead( PAYUEL_LGPM_Data.Handle, &Param );

    if (status == CFE_SUCCESS) 
    {
        uint8_t dynamic_length = rxdata.header.length; 
        uint16_t remaining_size = dynamic_length + 2;

        uint8_t raw_buffer[300];
        memset(raw_buffer, 0, sizeof(raw_buffer));

        // TX 없이 남은 데이터만 추가 수신
        // (명령이 중복 송신되지 않도록 TxSize를 0으로 설정)
        Param.TxData = NULL;
        Param.TxSize = 0;
        Param.RxData = raw_buffer;
        Param.RxSize = remaining_size; 
        Param.Timeout  = 150;
        Param.Interval = 100; 
        
        status = CFE_SRL_ApiRead(PAYUEL_LGPM_Data.Handle, &Param);

        if (status == CFE_SUCCESS) 
        {
            // CMD INFO + Timesatamp 메모리 복사 
            rxdata.V28_SUB_OFF = raw_buffer[0];
            memcpy(&rxdata.Execution_Timestamp, &raw_buffer[1], 4);
            
            // dynamic length : CMD INFO(1) + Timesatamp(4) + Reply_Message
            // msg_len : Reply_Message
            uint8_t msg_len = dynamic_length - 5;

            if (msg_len <= sizeof(rxdata.Reply_Message)) 
            {
                memcpy(rxdata.Reply_Message, &raw_buffer[5], msg_len);
                
            }
            
            memcpy(&rxdata.CRC16, &raw_buffer[dynamic_length], 2);
            
            OS_printf("======================================\n");
            OS_printf(" 페이로드 응답 수신 성공 (V28 SUB OFF)\n");
            OS_printf("  - Sync: 0x%02X\n", rxdata.header.sync);
            OS_printf("  - Status: 0x%02X\n", rxdata.V28_SUB_OFF);
            OS_printf("  - Timestamp: %u\n", (unsigned int)rxdata.Execution_Timestamp);
            OS_printf("  - Reply Msg: %.*s\n", (int)msg_len, rxdata.Reply_Message);
            OS_printf("======================================\n");
        }
        else 
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "UART Body(CMD_INFO + Time_stamp + Reply_Message) Read Failed! Status: 0x%08X", (unsigned int)status);
        }
    }
    else 
    {
        CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                          "UART Header(Sync + length) Read Failed or Timeout! Status: 0x%08X", (unsigned int)status);
    }
    
    return CFE_SUCCESS; 
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* */
/* PAYUEL_LGPM V12 MAIN ON command                                            */
/* */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_LGPM_V12_MAIN_ON_Cmd(const PAYUEL_LGPM_V12_MAIN_ON_Cmd_t *Msg) 
{
    PAYUEL_LGPM_Data.CmdCounter++;

    // TX 데이터 준비
    PAYUEL_LGPM_OBC2Payload_V12_MAIN_ON_Payload_t txdata;
    memset(&txdata, 0, sizeof(txdata));
    
    txdata.header.cmd_code = PWR_cmd_code;
    txdata.header.sync = 0x45;
    txdata.header.length = 1; 
    txdata.V12_MAIN_ON = 0x1A; 

    uint16 calc_length = sizeof(txdata) - sizeof(txdata.CRC16) - sizeof(txdata.header);
    txdata.CRC16 = Usart6_CalculateCRC16((uint8_t *)&txdata.V12_MAIN_ON, calc_length);

    // RX 데이터 및 파라미터 초기화
    PAYUEL_LGPM_Payload2OBC_V12_MAIN_ON_Payload_t rxdata;
    memset(&rxdata, 0, sizeof(rxdata));

    CFE_SRL_IO_Param_t Param;
    memset(&Param, 0, sizeof(Param));

    // 명령 송신(TX) + 헤더 수신(RX 3바이트)
    Param.TxData   = (uint8_t *)&txdata;           
    Param.TxSize   = sizeof(txdata); 
    Param.RxData   = (uint8_t *)&rxdata.header;
    Param.RxSize   = sizeof(rxdata.header);
    Param.Timeout  = 280;
    Param.Interval = 100;

    int32 status = CFE_SRL_ApiRead( PAYUEL_LGPM_Data.Handle, &Param );

    if (status == CFE_SUCCESS) 
    {
        uint8_t dynamic_length = rxdata.header.length; 
        uint16_t remaining_size = dynamic_length + 2;

        uint8_t raw_buffer[300];
        memset(raw_buffer, 0, sizeof(raw_buffer));

        // TX 없이 남은 데이터만 추가 수신
        // (명령이 중복 송신되지 않도록 TxSize를 0으로 설정)
        Param.TxData = NULL;
        Param.TxSize = 0;
        Param.RxData = raw_buffer;
        Param.RxSize = remaining_size; 
        Param.Timeout  = 150;
        Param.Interval = 100; 
        
        status = CFE_SRL_ApiRead(PAYUEL_LGPM_Data.Handle, &Param);

        if (status == CFE_SUCCESS) 
        {
            // CMD INFO + Timesatamp 메모리 복사 
            rxdata.V12_MAIN_ON = raw_buffer[0];
            memcpy(&rxdata.Execution_Timestamp, &raw_buffer[1], 4);
            
            // dynamic length : CMD INFO(1) + Timesatamp(4) + Reply_Message
            // msg_len : Reply_Message
            uint8_t msg_len = dynamic_length - 5;

            if (msg_len <= sizeof(rxdata.Reply_Message)) 
            {
                memcpy(rxdata.Reply_Message, &raw_buffer[5], msg_len);
                
            }
            
            memcpy(&rxdata.CRC16, &raw_buffer[dynamic_length], 2);
            
            OS_printf("======================================\n");
            OS_printf(" 페이로드 응답 수신 성공 (V12 MAIN ON)\n");
            OS_printf("  - Sync: 0x%02X\n", rxdata.header.sync);
            OS_printf("  - Status: 0x%02X\n", rxdata.V12_MAIN_ON);
            OS_printf("  - Timestamp: %u\n", (unsigned int)rxdata.Execution_Timestamp);
            OS_printf("  - Reply Msg: %.*s\n", (int)msg_len, rxdata.Reply_Message);
            OS_printf("======================================\n");
        }
        else 
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "UART Body(CMD_INFO + Time_stamp + Reply_Message) Read Failed! Status: 0x%08X", (unsigned int)status);
        }
    }
    else 
    {
        CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                          "UART Header(Sync + length) Read Failed or Timeout! Status: 0x%08X", (unsigned int)status);
    }
    
    return CFE_SUCCESS; 
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* */
/* PAYUEL_LGPM V12 MAIN OFF command                                           */
/* */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_LGPM_V12_MAIN_OFF_Cmd(const PAYUEL_LGPM_V12_MAIN_OFF_Cmd_t *Msg) 
{
    PAYUEL_LGPM_Data.CmdCounter++;

    // TX 데이터 준비
    PAYUEL_LGPM_OBC2Payload_V12_MAIN_OFF_Payload_t txdata;
    memset(&txdata, 0, sizeof(txdata));
    
    txdata.header.cmd_code = PWR_cmd_code;
    txdata.header.sync = 0x45;
    txdata.header.length = 1; 
    txdata.V12_MAIN_OFF = 0x1B; 

    uint16 calc_length = sizeof(txdata) - sizeof(txdata.CRC16) - sizeof(txdata.header);
    txdata.CRC16 = Usart6_CalculateCRC16((uint8_t *)&txdata.V12_MAIN_OFF, calc_length);

    // RX 데이터 및 파라미터 초기화
    PAYUEL_LGPM_Payload2OBC_V12_MAIN_OFF_Payload_t rxdata;
    memset(&rxdata, 0, sizeof(rxdata));

    CFE_SRL_IO_Param_t Param;
    memset(&Param, 0, sizeof(Param));

    // 명령 송신(TX) + 헤더 수신(RX 3바이트)
    Param.TxData   = (uint8_t *)&txdata;           
    Param.TxSize   = sizeof(txdata); 
    Param.RxData   = (uint8_t *)&rxdata.header;
    Param.RxSize   = sizeof(rxdata.header);
    Param.Timeout  = 270;
    Param.Interval = 100;

    int32 status = CFE_SRL_ApiRead( PAYUEL_LGPM_Data.Handle, &Param );

    if (status == CFE_SUCCESS) 
    {
        uint8_t dynamic_length = rxdata.header.length; 
        uint16_t remaining_size = dynamic_length + 2;

        uint8_t raw_buffer[300];
        memset(raw_buffer, 0, sizeof(raw_buffer));

        // TX 없이 남은 데이터만 추가 수신
        // (명령이 중복 송신되지 않도록 TxSize를 0으로 설정)
        Param.TxData = NULL;
        Param.TxSize = 0;
        Param.RxData = raw_buffer;
        Param.RxSize = remaining_size;
        Param.Timeout  = 150;
        Param.Interval = 100;  
        
        status = CFE_SRL_ApiRead(PAYUEL_LGPM_Data.Handle, &Param);

        if (status == CFE_SUCCESS) 
        {
            // CMD INFO + Timesatamp 메모리 복사 
            rxdata.V12_MAIN_OFF = raw_buffer[0];
            memcpy(&rxdata.Execution_Timestamp, &raw_buffer[1], 4);
            
            // dynamic length : CMD INFO(1) + Timesatamp(4) + Reply_Message
            // msg_len : Reply_Message
            uint8_t msg_len = dynamic_length - 5;

            if (msg_len <= sizeof(rxdata.Reply_Message)) 
            {
                memcpy(rxdata.Reply_Message, &raw_buffer[5], msg_len);
                
            }
            
            memcpy(&rxdata.CRC16, &raw_buffer[dynamic_length], 2);
            
            OS_printf("======================================\n");
            OS_printf(" 페이로드 응답 수신 성공 (V12 MAIN OFF)\n");
            OS_printf("  - Sync: 0x%02X\n", rxdata.header.sync);
            OS_printf("  - Status: 0x%02X\n", rxdata.V12_MAIN_OFF);
            OS_printf("  - Timestamp: %u\n", (unsigned int)rxdata.Execution_Timestamp);
            OS_printf("  - Reply Msg: %.*s\n", (int)msg_len, rxdata.Reply_Message);
            OS_printf("======================================\n");
        }
        else 
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "UART Body(CMD_INFO + Time_stamp + Reply_Message) Read Failed! Status: 0x%08X", (unsigned int)status);
        }
    }
    else 
    {
        CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                          "UART Header(Sync + length) Read Failed or Timeout! Status: 0x%08X", (unsigned int)status);
    }
    
    return CFE_SUCCESS; 
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUEL_LGPM PWR SENSE INFO command                                         */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_LGPM_PWR_SENSE_INFO_Cmd(const PAYUEL_LGPM_PWR_SENSE_INFO_Cmd_t *Msg) 
{
    PAYUEL_LGPM_Data.CmdCounter++;

    // TX 데이터 준비
    PAYUEL_LGPM_OBC2Payload_PWR_SENSE_INFO_Payload_t txdata;
    memset(&txdata, 0, sizeof(txdata));
    
    txdata.header.cmd_code = PWR_cmd_code;
    txdata.header.sync = 0x45;
    txdata.header.length = 1; 
    txdata.PWR_SENSE_INFO = 0x1C; 

    uint16 calc_length = sizeof(txdata) - sizeof(txdata.CRC16) - sizeof(txdata.header);
    txdata.CRC16 = Usart6_CalculateCRC16((uint8_t *)&txdata.PWR_SENSE_INFO, calc_length);

    // RX 데이터 및 파라미터 초기화
    PAYUEL_LGPM_Payload2OBC_PWR_SENSE_INFO_Payload_t rxdata;
    memset(&rxdata, 0, sizeof(rxdata));

    CFE_SRL_IO_Param_t Param;
    memset(&Param, 0, sizeof(Param));

    // 명령 송신(TX) + 헤더 수신(RX 3바이트)
    Param.TxData   = (uint8_t *)&txdata;           
    Param.TxSize   = sizeof(txdata); 
    Param.RxData   = (uint8_t *)&rxdata.header;
    Param.RxSize   = sizeof(rxdata.header);
    Param.Timeout  = 1500;
    Param.Interval = 100;

    int32 status = CFE_SRL_ApiRead( PAYUEL_LGPM_Data.Handle, &Param );

    if (status == CFE_SUCCESS) 
    {
        uint8_t dynamic_length = rxdata.header.length; 
        uint16_t remaining_size = dynamic_length + 2;

        uint8_t raw_buffer[300];
        memset(raw_buffer, 0, sizeof(raw_buffer));

        // TX 없이 남은 데이터만 추가 수신
        // (명령이 중복 송신되지 않도록 TxSize를 0으로 설정)
        Param.TxData = NULL;
        Param.TxSize = 0;
        Param.RxData = raw_buffer;
        Param.RxSize = remaining_size;
        Param.Timeout  = 500;
        Param.Interval = 0; 
        
        status = CFE_SRL_ApiRead(PAYUEL_LGPM_Data.Handle, &Param);

        if (status == CFE_SUCCESS) 
        {
            // CMD INFO + Timesatamp 메모리 복사 
            rxdata.PWR_SENSE_INFO = raw_buffer[0];
            memcpy(&rxdata.Execution_Timestamp, &raw_buffer[1], 4);
            
            // dynamic length : CMD INFO(1) + Timesatamp(4) + Reply_Message
            // msg_len : Reply_Message
            uint8_t msg_len = dynamic_length - 5;

            if (msg_len <= sizeof(rxdata.Reply_Message)) 
            {
                memcpy(rxdata.Reply_Message, &raw_buffer[5], msg_len);
                
            }
            
            memcpy(&rxdata.CRC16, &raw_buffer[dynamic_length], 2);
            
            OS_printf("======================================\n");
            OS_printf(" 페이로드 응답 수신 성공\n");
            OS_printf("  - Sync: 0x%02X\n", rxdata.header.sync);
            OS_printf("  - Status: 0x%02X\n", rxdata.PWR_SENSE_INFO);
            OS_printf("  - Timestamp: %u\n", (unsigned int)rxdata.Execution_Timestamp);
            OS_printf("  - Reply Msg: \n");

            int chunk_size = 64;
            for (int i = 0; i < msg_len; i += chunk_size) 
            {
                int print_len = (msg_len - i > chunk_size) ? chunk_size : (msg_len - i);
                OS_printf("    %.*s\n", print_len, &rxdata.Reply_Message[i]);
            }

            // OS_printf("  - [DEBUG] REPLY_MSG Length: %d\n", (int)sizeof(rxdata.Reply_Message) );
            OS_printf("======================================\n");

        }
        else 
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "UART Body(CMD_INFO + Time_stamp + Reply_Message) Read Failed! Status: 0x%08X", (unsigned int)status);
        }
    }
    else 
    {
        CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                          "UART Header(Sync + length) Read Failed or Timeout! Status: 0x%08X", (unsigned int)status);
        OS_printf("  - cmd_code      : 0x%02X\n", rxdata.header.cmd_code);
        OS_printf("  - sync        : 0x%02X\n", rxdata.header.sync);
        OS_printf("  - length      : 0x%02X\n", rxdata.header.length);
    }   
    
    return CFE_SUCCESS; 
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* */
/* PAYUEL_LGPM PWR SEQ ON command                                             */
/* */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_LGPM_PWR_SEQ_ON_Cmd(const PAYUEL_LGPM_PWR_SEQ_ON_Cmd_t *Msg) 
{
    PAYUEL_LGPM_Data.CmdCounter++;

    PAYUEL_LGPM_OBC2Payload_PWR_SEQ_ON_Payload_t txdata;
    memset(&txdata, 0, sizeof(txdata));
    
    txdata.header.cmd_code = PWR_cmd_code;
    txdata.header.sync = 0x45;
    txdata.header.length = 1; 
    txdata.PWR_SEQ_ON = 0x31; 

    uint16 calc_length = sizeof(txdata) - sizeof(txdata.CRC16) - sizeof(txdata.header);
    txdata.CRC16 = Usart6_CalculateCRC16((uint8_t *)&txdata.PWR_SEQ_ON, calc_length);

    CFE_SRL_IO_Param_t Param;
    memset(&Param, 0, sizeof(Param));

    Param.TxData   = (uint8_t *)&txdata;           
    Param.TxSize   = sizeof(txdata); 
    Param.Timeout  = 5000;  // 첫 응답까지 대기시간
    Param.Interval = 100;

    int max_responses = 5;
    int seq_complete = 0;  

    // 여러 개의 응답 패킷을 순차적으로 읽기 위한 루프
    for (int i = 0; i < max_responses; i++)
    {
        // 매 루프마다 RX 데이터 버퍼 초기화
        PAYUEL_LGPM_Payload2OBC_PWR_SEQ_ON_Payload_t rxdata;
        memset(&rxdata, 0, sizeof(rxdata));

        // 헤더 수신 (3바이트) 대기
        Param.RxData   = (uint8_t *)&rxdata.header;
        Param.RxSize   = sizeof(rxdata.header);

        int32 status = CFE_SRL_ApiRead( PAYUEL_LGPM_Data.Handle, &Param );

        if (status == CFE_SUCCESS) 
        {
            uint8_t dynamic_length = rxdata.header.length; 
            uint16_t remaining_size = dynamic_length + 2; // 바디 + CRC

            uint8_t raw_buffer[300];
            memset(raw_buffer, 0, sizeof(raw_buffer));

            // 바디 수신 (이때는 TX가 나가지 않도록 설정)
            Param.TxData = NULL;
            Param.TxSize = 0;
            Param.RxData = raw_buffer;
            Param.RxSize = remaining_size;
            Param.Timeout  = 500;
            Param.Interval = 0;  
            
            status = CFE_SRL_ApiRead(PAYUEL_LGPM_Data.Handle, &Param);

            if (status == CFE_SUCCESS) 
            {
                // CMD INFO + Timesatamp 메모리 복사 
                rxdata.PWR_SEQ_ON = raw_buffer[0];
                memcpy(&rxdata.Execution_Timestamp, &raw_buffer[1], 4);
                
                uint8_t msg_len = dynamic_length - 5;

                if (msg_len <= sizeof(rxdata.Reply_Message)) 
                {
                    memcpy(rxdata.Reply_Message, &raw_buffer[5], msg_len);
                }
                
                memcpy(&rxdata.CRC16, &raw_buffer[dynamic_length], 2);
                
                OS_printf("======================================\n");
                OS_printf(" 응답 패킷 [%d] 수신 성공 (PWR SEQ ON)\n", i + 1);
                OS_printf("  - Sync: 0x%02X\n", rxdata.header.sync);
                OS_printf("  - Status: 0x%02X\n", rxdata.PWR_SEQ_ON);
                OS_printf("  - Timestamp: %u\n", (unsigned int)rxdata.Execution_Timestamp);
                OS_printf("  - Reply Msg: %.*s\n", (int)msg_len, rxdata.Reply_Message);
                OS_printf("======================================\n");

                // 종료 조건 검사: 마지막 메시지가 도착했는지 확인
                // 수신된 데이터에 널 종료(NULL-termination)가 없을 수 있으므로 strncmp 사용 (길이 19)
                if (strncmp((char*)rxdata.Reply_Message, "PWR_SEQ_ON_COMPLETE", 19) == 0)
                {
                    seq_complete = 1;
                    break; // 모든 시퀀스 응답을 받았으므로 루프를 안전하게 탈출
                }
            }
            else 
            {
                CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                                "UART Body Read Failed! Status: 0x%08X", (unsigned int)status);
                break; // 바디 읽기 실패 시 루프 탈출
            }
        }
        else 
        {
            if (i > 0) {
                // 이미 첫 번째 패킷을 받았는데 다음 헤더를 못 읽은 경우 (시퀀스 중간 끊김)
                break; 
            } else {
                // 아예 처음부터 응답이 없는 경우
                CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                                  "UART Header Read Failed or Timeout! Status: 0x%08X", (unsigned int)status);
                break;
            }
        }

      // 두 번째 루프부터는 다음 패킷 헤더 수신을 위해 파라미터 재설정
        Param.TxData = NULL;
        Param.TxSize = 0;
        Param.Timeout = 5000;  
        Param.Interval = 100;  
    }

    // 최종 상태 확인용 출력
    if (seq_complete) {
        OS_printf(" -> PWR SEQ ON 시퀀스 전체 완료 확인!\n");
    } else {
        OS_printf(" 시퀀스 COMPLETE 메시지를 받지 못했습니다.\n");
    }
    
    return CFE_SUCCESS; 
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* */
/* PAYUEL_LGPM PWR SEQ OFF command                                            */
/* */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_LGPM_PWR_SEQ_OFF_Cmd(const PAYUEL_LGPM_PWR_SEQ_OFF_Cmd_t *Msg) 
{
    PAYUEL_LGPM_Data.CmdCounter++;

    // TX 데이터
    PAYUEL_LGPM_OBC2Payload_PWR_SEQ_OFF_Payload_t txdata;
    memset(&txdata, 0, sizeof(txdata));
    txdata.header.cmd_code = PWR_cmd_code;
    txdata.header.sync = 0x45;
    txdata.header.length = 1; 
    txdata.PWR_SEQ_OFF = 0x32; 

    uint16 calc_length = sizeof(txdata) - sizeof(txdata.CRC16) - sizeof(txdata.header);
    txdata.CRC16 = Usart6_CalculateCRC16((uint8_t *)&txdata.PWR_SEQ_OFF, calc_length);

    // 초기화
    CFE_SRL_IO_Param_t Param;
    memset(&Param, 0, sizeof(Param));

    Param.TxData   = (uint8_t *)&txdata;           
    Param.TxSize   = sizeof(txdata); 
    Param.Timeout  = 1200; 
    Param.Interval = 100;

    int max_responses = 5; // 무한 루프 방지용 (최대 5개 패킷 대기)
    int seq_complete = 0;  // 완료 플래그

    for (int i = 0; i < max_responses; i++)
    {
        PAYUEL_LGPM_Payload2OBC_PWR_SEQ_OFF_Payload_t rxdata;
        memset(&rxdata, 0, sizeof(rxdata));
        
        Param.RxData = (uint8_t *)&rxdata.header;
        Param.RxSize = sizeof(rxdata.header);

        int32 status = CFE_SRL_ApiRead(PAYUEL_LGPM_Data.Handle, &Param);

        if (status == CFE_SUCCESS) 
        {
            uint8_t dynamic_length = rxdata.header.length; 
            uint16_t remaining_size = dynamic_length + 2; // 바디 + CRC

            uint8_t raw_buffer[300];
            memset(raw_buffer, 0, sizeof(raw_buffer));

            Param.TxData = NULL;
            Param.TxSize = 0;
            Param.RxData = raw_buffer;
            Param.RxSize = remaining_size;
            Param.Timeout  = 500;
            Param.Interval = 0;  
            
            status = CFE_SRL_ApiRead(PAYUEL_LGPM_Data.Handle, &Param);

            if (status == CFE_SUCCESS) 
            {
                rxdata.PWR_SEQ_OFF = raw_buffer[0];
                memcpy(&rxdata.Execution_Timestamp, &raw_buffer[1], 4);
                
                uint8_t msg_len = dynamic_length - 5;
                if (msg_len <= sizeof(rxdata.Reply_Message)) 
                {
                    memcpy(rxdata.Reply_Message, &raw_buffer[5], msg_len);
                }
                
                OS_printf("======================================\n");
                OS_printf(" 응답 패킷 [%d] 수신 성공\n", i+1);
                OS_printf("  - Reply Msg: %.*s\n", (int)msg_len, rxdata.Reply_Message);
                OS_printf("======================================\n");

                // 종료 조건 검사: 마지막 메시지가 도착했는지 확인
                if (strncmp((char*)rxdata.Reply_Message, "PWR_SEQ_OFF_COMPLETE", 20) == 0)
                {
                    seq_complete = 1;
                    break; // 모든 시퀀스 응답을 받았으므로 루프 탈출
                }
            }
            else 
            {
                OS_printf("Body Read Error!\n");
                break;
            }
        }
        else 
        {
            // 헤더를 못 읽음 (더 이상 올 패킷이 없거나 타임아웃)
            if (i > 0) {
                // 이미 패킷을 하나라도 받았다면 단순히 더 올게 없는 것일 수 있음
                break; 
            } else {
                CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, "UART Header Read Failed!");
                break;
            }
        }

        Param.TxData = NULL;
        Param.TxSize = 0;
        Param.Timeout = 1500; // 다음 패킷이 올 때까지의 대기 시간 (페이로드 시퀀스 딜레이 고려)
    }

    if (seq_complete) {
        OS_printf(" -> PWR SEQ OFF 시퀀스 전체 완료 확인!\n");
    } else {
        OS_printf(" 시퀀스 COMPLETE 메시지를 받지 못했습니다.\n");
    }

    return CFE_SUCCESS; 
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* */
/* PAYUEL_LGPM RWA CONTROL command                                            */
/* */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_LGPM_RWA_CONTROL_Cmd(const PAYUEL_LGPM_RWA_CONTROL_Cmd_t *Msg) 
{
    PAYUEL_LGPM_Data.CmdCounter++;

    PAYUEL_LGPM_OBC2Payload_RWA_CONTROL_Payload_t txdata;
    memset(&txdata, 0, sizeof(txdata));
    
    txdata.header.cmd_code = RWA_cmd_code; 
    txdata.header.sync = 0x45;    
    txdata.header.length = 9;     
    txdata.RWA_CONTROL = 0x20; 

    txdata.TargetSpeed_RPM = Msg->payload.TargetSpeed_RPM;
    txdata.TargetAcc_RPM   = Msg->payload.TargetAcc_RPM;
    txdata.Operating_Time  = Msg->payload.Operating_Time;
    txdata.sub_index       = Msg->payload.sub_index;

    // check sum

    uint8_t checksum_val = 0;
    // uint8_t *payload_ptr = (uint8_t *)&txdata.RWA_CONTROL;
    uint8_t *payload_ptr = (uint8_t *)&txdata.TargetSpeed_RPM;
    for (int i = 0; i < 7; i++) 
    {
        checksum_val ^= payload_ptr[i];
    }
    txdata.Checksum = checksum_val;
    
    uint16 calc_length = sizeof(txdata) - sizeof(txdata.CRC16) - sizeof(txdata.header);
    txdata.CRC16 = Usart6_CalculateCRC16((uint8_t *)&txdata.RWA_CONTROL, calc_length);

    OS_printf("\n");


    PAYUEL_LGPM_Payload2OBC_RWA_CONTROL_Payload_t rxdata;
    memset(&rxdata, 0, sizeof(rxdata)); 

    CFE_SRL_IO_Param_t Param;
    memset(&Param, 0, sizeof(Param));

    Param.TxData   = (uint8_t *)&txdata;           
    Param.TxSize   = sizeof(txdata); 
    Param.RxData   = (uint8_t *)&rxdata.header;
    Param.RxSize   = sizeof(rxdata.header);
    Param.Timeout  = 1400;  
    Param.Interval = 100;

    int32 status = CFE_SRL_ApiRead( PAYUEL_LGPM_Data.Handle, &Param );

    if (status == CFE_SUCCESS) 
    {
        uint8_t dynamic_length = rxdata.header.length; 
        uint16_t remaining_size = dynamic_length + 2;

        uint8_t raw_buffer[300];
        memset(raw_buffer, 0, sizeof(raw_buffer));

        Param.TxData = NULL;
        Param.TxSize = 0;
        Param.RxData = raw_buffer;
        Param.RxSize = remaining_size;
        Param.Timeout  = 150;
        Param.Interval = 100;  
        
        status = CFE_SRL_ApiRead(PAYUEL_LGPM_Data.Handle, &Param);

        if (status == CFE_SUCCESS) 
        {
            rxdata.RWA_CONTROL = raw_buffer[0];
            memcpy(&rxdata.Execution_Timestamp, &raw_buffer[1], 4);
            
            uint8_t msg_len = dynamic_length - 5;
            if (msg_len <= sizeof(rxdata.Reply_Message)) 
            {
                memcpy(rxdata.Reply_Message, &raw_buffer[5], msg_len);
                
            }
            
            memcpy(&rxdata.CRC16, &raw_buffer[dynamic_length], 2);
            
            OS_printf("======================================\n");
            OS_printf(" 페이로드 응답 수신 성공 (RWA CONTROL)\n");
            OS_printf("  - Sync: 0x%02X\n", rxdata.header.sync);
            OS_printf("  - Status: 0x%02X\n", rxdata.RWA_CONTROL);
            OS_printf("  - Timestamp: %u\n", (unsigned int)rxdata.Execution_Timestamp);
            OS_printf("  - Reply Msg: %.*s\n", (int)msg_len, rxdata.Reply_Message);
            OS_printf("======================================\n");
        }
        else 
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "UART Body Read Failed! Status: 0x%08X", (unsigned int)status);
        }
    }
    else 
    {
        CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                          "UART Header Read Failed or Timeout! Status: 0x%08X", (unsigned int)status);
    }
    return CFE_SUCCESS; 
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* */
/* PAYUEL_LGPM RWA PWR ON command                                             */
/* */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_LGPM_RWA_PWR_ON_Cmd(const PAYUEL_LGPM_RWA_PWR_ON_Cmd_t *Msg) 
{
    PAYUEL_LGPM_Data.CmdCounter++;

    PAYUEL_LGPM_OBC2Payload_RWA_PWR_ON_Payload_t txdata;
    memset(&txdata, 0, sizeof(txdata));
    
    txdata.header.cmd_code = RWA_cmd_code;
    txdata.header.sync = 0x45;
    txdata.header.length = 1; 
    txdata.RWA_PWR_ON = 0x21; 


    uint16 calc_length = sizeof(txdata) - sizeof(txdata.CRC16) - sizeof(txdata.header);
    txdata.CRC16 = Usart6_CalculateCRC16((uint8_t *)&txdata.RWA_PWR_ON, calc_length);

    PAYUEL_LGPM_Payload2OBC_RWA_PWR_ON_Payload_t rxdata;
    memset(&rxdata, 0, sizeof(rxdata)); 

    CFE_SRL_IO_Param_t Param;
    memset(&Param, 0, sizeof(Param));

    Param.TxData   = (uint8_t *)&txdata;           
    Param.TxSize   = sizeof(txdata); 

    //Debug 
        OS_printf("RWA_PWR_ON TX size=%u\n", (unsigned int)sizeof(txdata));
    for (int i = 0; i < sizeof(txdata); i++) {
        OS_printf("tx[%d] = 0x%02X\n", i, ((uint8 *)&txdata)[i]);
    }

    Param.RxData   = (uint8_t *)&rxdata.header;
    Param.RxSize   = sizeof(rxdata.header);
    Param.Timeout  = 5000;  
    Param.Interval = 100;

    int32 status = CFE_SRL_ApiRead( PAYUEL_LGPM_Data.Handle, &Param );

    if (status == CFE_SUCCESS) 
    {
        uint8_t dynamic_length = rxdata.header.length; 
        uint16_t remaining_size = dynamic_length + 2;

        uint8_t raw_buffer[300];
        memset(raw_buffer, 0, sizeof(raw_buffer));

        Param.TxData = NULL;
        Param.TxSize = 0;
        Param.RxData = raw_buffer;
        Param.RxSize = remaining_size;
        Param.Timeout  = 150;
        Param.Interval = 100;  
        
        status = CFE_SRL_ApiRead(PAYUEL_LGPM_Data.Handle, &Param);

        if (status == CFE_SUCCESS) 
        {
            rxdata.RWA_PWR_ON = raw_buffer[0];
            memcpy(&rxdata.Execution_Timestamp, &raw_buffer[1], 4);
            
            uint8_t msg_len = dynamic_length - 5;
            if (msg_len <= sizeof(rxdata.Reply_Message)) 
            {
                memcpy(rxdata.Reply_Message, &raw_buffer[5], msg_len);
                
            }
            
            memcpy(&rxdata.CRC16, &raw_buffer[dynamic_length], 2);
            
            OS_printf("======================================\n");
            OS_printf(" 페이로드 응답 수신 성공 (RWA PWR ON)\n");
            OS_printf("  - Sync: 0x%02X\n", rxdata.header.sync);
            OS_printf("  - Status: 0x%02X\n", rxdata.RWA_PWR_ON);
            OS_printf("  - Timestamp: %u\n", (unsigned int)rxdata.Execution_Timestamp);
            OS_printf("  - Reply Msg: %.*s\n", (int)msg_len, rxdata.Reply_Message);
            OS_printf("======================================\n");
        }
        else 
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "UART Body Read Failed! Status: 0x%08X", (unsigned int)status);
        }
    }
    else 
    {
        CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                          "UART Header Read Failed or Timeout! Status: 0x%08X", (unsigned int)status);
    }
    return CFE_SUCCESS; 
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* */
/* PAYUEL_LGPM RWA PWR OFF command                                            */
/* */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_LGPM_RWA_PWR_OFF_Cmd(const PAYUEL_LGPM_RWA_PWR_OFF_Cmd_t *Msg) 
{
    PAYUEL_LGPM_Data.CmdCounter++;

    PAYUEL_LGPM_OBC2Payload_RWA_PWR_OFF_Payload_t txdata;
    memset(&txdata, 0, sizeof(txdata));
    
    txdata.header.cmd_code = RWA_cmd_code;
    txdata.header.sync = 0x45;
    txdata.header.length = 1; 
    txdata.RWA_PWR_OFF = 0x22; 

    uint16 calc_length = sizeof(txdata) - sizeof(txdata.CRC16) - sizeof(txdata.header);
    txdata.CRC16 = Usart6_CalculateCRC16((uint8_t *)&txdata.RWA_PWR_OFF, calc_length);

    PAYUEL_LGPM_Payload2OBC_RWA_PWR_OFF_Payload_t rxdata;
    memset(&rxdata, 0, sizeof(rxdata)); 

    CFE_SRL_IO_Param_t Param;
    memset(&Param, 0, sizeof(Param));

    Param.TxData   = (uint8_t *)&txdata;           
    Param.TxSize   = sizeof(txdata); 
    Param.RxData   = (uint8_t *)&rxdata.header;
    Param.RxSize   = sizeof(rxdata.header);
    Param.Timeout  = 300;  
    Param.Interval = 100;

    int32 status = CFE_SRL_ApiRead( PAYUEL_LGPM_Data.Handle, &Param );

    if (status == CFE_SUCCESS) 
    {
        uint8_t dynamic_length = rxdata.header.length; 
        uint16_t remaining_size = dynamic_length + 2;

        uint8_t raw_buffer[300];
        memset(raw_buffer, 0, sizeof(raw_buffer));

        Param.TxData = NULL;
        Param.TxSize = 0;
        Param.RxData = raw_buffer;
        Param.RxSize = remaining_size;
        Param.Timeout  = 150;
        Param.Interval = 100;  
        
        status = CFE_SRL_ApiRead(PAYUEL_LGPM_Data.Handle, &Param);

        if (status == CFE_SUCCESS) 
        {
            rxdata.RWA_PWR_OFF = raw_buffer[0];
            memcpy(&rxdata.Execution_Timestamp, &raw_buffer[1], 4);
            
            uint8_t msg_len = dynamic_length - 5;
            if (msg_len <= sizeof(rxdata.Reply_Message)) 
            {
                memcpy(rxdata.Reply_Message, &raw_buffer[5], msg_len);
                
            }
            
            memcpy(&rxdata.CRC16, &raw_buffer[dynamic_length], 2);
            
            OS_printf("======================================\n");
            OS_printf(" 페이로드 응답 수신 성공 (RWA PWR OFF)\n");
            OS_printf("  - Sync: 0x%02X\n", rxdata.header.sync);
            OS_printf("  - Status: 0x%02X\n", rxdata.RWA_PWR_OFF);
            OS_printf("  - Timestamp: %u\n", (unsigned int)rxdata.Execution_Timestamp);
            OS_printf("  - Reply Msg: %.*s\n", (int)msg_len, rxdata.Reply_Message);
            OS_printf("======================================\n");
        }
        else 
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "UART Body Read Failed! Status: 0x%08X", (unsigned int)status);
        }
    }
    else 
    {
        CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                          "UART Header Read Failed or Timeout! Status: 0x%08X", (unsigned int)status);
    }
    return CFE_SUCCESS; 
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* */
/* PAYUEL_LGPM RWA SENSE INFO command                                         */
/* */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_LGPM_RWA_SENSE_INFO_Cmd(const PAYUEL_LGPM_RWA_SENSE_INFO_Cmd_t *Msg) 
{
    PAYUEL_LGPM_Data.CmdCounter++;

    PAYUEL_LGPM_OBC2Payload_RWA_SENSE_INFO_Payload_t txdata;
    memset(&txdata, 0, sizeof(txdata));
    
    txdata.header.cmd_code = RWA_cmd_code;
    txdata.header.sync = 0x45;
    txdata.header.length = 1; 
    txdata.RWA_SENSE_INFO = 0x23; 

    uint16 calc_length = sizeof(txdata) - sizeof(txdata.CRC16) - sizeof(txdata.header);
    txdata.CRC16 = Usart6_CalculateCRC16((uint8_t *)&txdata.RWA_SENSE_INFO, calc_length);

    PAYUEL_LGPM_Payload2OBC_RWA_SENSE_INFO_Payload_t rxdata;
    memset(&rxdata, 0, sizeof(rxdata)); 

    CFE_SRL_IO_Param_t Param;
    memset(&Param, 0, sizeof(Param));

    Param.TxData   = (uint8_t *)&txdata;           
    Param.TxSize   = sizeof(txdata); 
    Param.RxData   = (uint8_t *)&rxdata.header;
    Param.RxSize   = sizeof(rxdata.header);
    Param.Timeout  = 1400;  
    Param.Interval = 100;

    int32 status = CFE_SRL_ApiRead( PAYUEL_LGPM_Data.Handle, &Param );

    if (status == CFE_SUCCESS) 
    {
        uint8_t dynamic_length = rxdata.header.length; 
        uint16_t remaining_size = dynamic_length + 2;

        uint8_t raw_buffer[300];
        memset(raw_buffer, 0, sizeof(raw_buffer));

        Param.TxData = NULL;
        Param.TxSize = 0;
        Param.RxData = raw_buffer;
        Param.RxSize = remaining_size; 
        Param.Timeout  = 150;
        Param.Interval = 100; 
        
        status = CFE_SRL_ApiRead(PAYUEL_LGPM_Data.Handle, &Param);

        if (status == CFE_SUCCESS) 
        {
            rxdata.RWA_SENSE_INFO = raw_buffer[0];
            memcpy(&rxdata.Execution_Timestamp, &raw_buffer[1], 4);
            
            uint8_t msg_len = dynamic_length - 5;
            if (msg_len <= sizeof(rxdata.Reply_Message)) 
            {
                memcpy(rxdata.Reply_Message, &raw_buffer[5], msg_len);
                
            }
            
            memcpy(&rxdata.CRC16, &raw_buffer[dynamic_length], 2);
            
            OS_printf("======================================\n");
            OS_printf(" 페이로드 응답 수신 성공 (RWA SENSE INFO)\n");
            OS_printf("  - Sync: 0x%02X\n", rxdata.header.sync);
            OS_printf("  - Status: 0x%02X\n", rxdata.RWA_SENSE_INFO);
            OS_printf("  - Timestamp: %u\n", (unsigned int)rxdata.Execution_Timestamp);
            OS_printf("  - Reply Msg: %.*s\n", (int)msg_len, rxdata.Reply_Message);
            OS_printf("================[DeBug Line below]======================\n");
            OS_printf("  - dynamic length : %u\n", (unsigned int)dynamic_length);
            OS_printf("  - Size of dynamic length : %u\n", (unsigned int)sizeof(dynamic_length));  
            OS_printf("======================================\n");     
        }
        else 
        {
            CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                            "UART Body Read Failed! Status: 0x%08X", (unsigned int)status);
        }
    }
    else 
    {
        CFE_EVS_SendEvent(PAYUEL_LGPM_CC_ERR_EID, CFE_EVS_EventType_ERROR, 
                          "UART Header Read Failed or Timeout! Status: 0x%08X", (unsigned int)status);
    }
    return CFE_SUCCESS; 
}

// /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
// /*                                                                            */
// /*  Purpose:                                                                  */
// /*         This function Process Ground Station Command                       */
// /*                                                                            */
// /* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
// CFE_Status_t PAYUEL_LGPM_ProcessCmd(const PAYUEL_LGPM_ProcessCmd_t *Msg)
// {
//     CFE_SRL_IO_Handle_t *uart = CFE_SRL_ApiGetHandle(CFE_SRL_RS422_HANDLE_INDEXER);
//     CFE_SRL_IO_Param_t param = {0,};

//     uint8_t TxBuf[12] = {0};
//     uint8_t RxBuf[12] = {0};
//     memcpy(TxBuf, "ILOVEMOZART", 12);
//     param.TxData = TxBuf;
//     param.TxSize = sizeof(TxBuf);
//     param.RxData = RxBuf;
//     param.RxSize = sizeof(RxBuf);
//     param.Timeout = 500;

//     int32 Status = CFE_SRL_ApiRead(uart, &param);
//     OS_printf("UART Transaction Status: 0x%08X\n", Status);
//     if (Status == CFE_SUCCESS) {
//         OS_printf("Rx Data: %s\n", RxBuf);
//     }

//     return CFE_SUCCESS;
// }

// /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
// /*                                                                            */
// /* A simple example command that displays a passed-in value                   */
// /*                                                                            */
// /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
// CFE_Status_t PAYUEL_LGPM_DisplayParamCmd(const PAYUEL_LGPM_DisplayParamCmd_t *Msg)
// {
//     uint8_t TxBuf[12] = {0,};
//     uint8_t RxBuf[32] = {0,};
//     memcpy(TxBuf, "ILOVEMOZART", sizeof(TxBuf));
//     int32 Status = CFE_SRL_ApiTransactionCSP(CSP_NODE_TEST, 0, TxBuf, sizeof(TxBuf), RxBuf, sizeof(RxBuf));
//     OS_printf("CSP Transaction Status: 0x%08X\n", Status);
//     if (Status > 0) {
//         OS_printf("Rx Data: %s\n", RxBuf);
//     }

//     return CFE_SUCCESS;
// }

// CFE_Status_t PAYUEL_LGPM_NativeCANCmd(const PAYUEL_LGPM_NativeCANCmd_t *Msg)
// {
//     CFE_SRL_IO_Handle_t *uart = CFE_SRL_ApiGetHandle(CFE_SRL_CAN0_HANDLE_INDEXER);
//     CFE_SRL_IO_Param_t param = {0,};

//     uint8_t TxBuf[12] = {0};
//     uint8_t RxBuf[12] = {0};
//     memcpy(TxBuf, "ILOVEMOZART", 12);
//     param.TxData = TxBuf;
//     param.TxSize = sizeof(TxBuf);
//     param.RxData = RxBuf;
//     param.RxSize = sizeof(RxBuf);
//     param.Timeout = 5000;

//     int32 Status = CFE_SRL_ApiRead(uart, &param);
//     OS_printf("CAN Transaction Status: 0x%08X\n", Status);
//     if (Status == CFE_SUCCESS) {
//         OS_printf("Rx Data: %s\n", RxBuf);
//     }

//     return CFE_SUCCESS;
//}
