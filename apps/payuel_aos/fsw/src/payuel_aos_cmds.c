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
 *   This file contains the source code for the Payuel Aos Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "payuel_aos.h"
#include "payuel_aos_cmds.h"
#include "payuel_aos_msgids.h"
#include "payuel_aos_eventids.h"
#include "payuel_aos_version.h"
#include "payuel_aos_tbl.h"
#include "payuel_aos_utils.h"
#include "payuel_aos_msg.h"
#include "payuel_aos_msgstruct.h"
#include <unistd.h>




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t PAYUEL_AOS_SendHkCmd(const PAYUEL_AOS_SendHkCmd_t *Msg)
{
    int i;

    /*
    ** Get command execution counters...
    */
    PAYUEL_AOS_Data.HkTlm.Payload.CommandErrorCounter = PAYUEL_AOS_Data.ErrCounter;
    PAYUEL_AOS_Data.HkTlm.Payload.CommandCounter      = PAYUEL_AOS_Data.CmdCounter;

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(PAYUEL_AOS_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(PAYUEL_AOS_Data.HkTlm.TelemetryHeader), true);

    /*
    ** Manage any pending table loads, validations, etc.
    */
    for (i = 0; i < PAYUEL_AOS_NUMBER_OF_TABLES; i++)
    {
        CFE_TBL_Manage(PAYUEL_AOS_Data.TblHandles[i]);
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUEL_AOS NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUEL_AOS_NoopCmd(const PAYUEL_AOS_NoopCmd_t *Msg)
{
    PAYUEL_AOS_Data.CmdCounter++;

    CFE_EVS_SendEvent(PAYUEL_AOS_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "PAYUEL_AOS: NOOP command %s",
                      PAYUEL_AOS_VERSION);
    

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t PAYUEL_AOS_ResetCountersCmd(const PAYUEL_AOS_ResetCountersCmd_t *Msg)
{
    PAYUEL_AOS_Data.CmdCounter = 0;
    PAYUEL_AOS_Data.ErrCounter = 0;

    CFE_EVS_SendEvent(PAYUEL_AOS_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "PAYUEL_AOS: RESET Counters command");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function Process Ground Station Command                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t PAYUEL_AOS_ResetCmd(const PAYUEL_AOS_ResetCmd_t *Msg)
{

    CFE_SRL_IO_Param_t param = {0,};

    uint8_t TxBuf[1] = {0x06};
    

    param.TxData = TxBuf;
    param.TxSize = sizeof(TxBuf);
    param.Addr = 0x48; //GND
    param.Timeout = 100;

    int32 Status = CFE_SRL_ApiWrite(i2c, &param);
    OS_printf("I2C Write Status: 0x%08X\n", Status);
    if (Status == CFE_SUCCESS) {
        OS_printf("PAYUEL_AOS_RESET Done!!");
    }

    return CFE_SUCCESS;
}

CFE_Status_t PAYUEL_AOS_Write_RegisterCmd(const PAYUEL_AOS_Write_RegisterCmd_t *Msg)
{
    CFE_SRL_IO_Param_t param = {0,};

    uint8_t TxBuf[3] = {0};
    TxBuf[0] = Msg->reg_addr;      
    TxBuf[1] = (uint8_t)(((uint16_t)Msg->data >> 8) & 0x00FFu);
    TxBuf[2] = (uint8_t)((uint16_t)Msg->data & 0x00FFu);
    OS_printf("tx[1]= 0x%02X\n",TxBuf[1]);
    OS_printf("tx[2]= 0x%02X\n",TxBuf[2]);

    param.TxData = TxBuf;
    param.TxSize = sizeof(TxBuf);
    param.Addr = 0x48;
    param.Timeout = 100;

    int32 Status = CFE_SRL_ApiWrite(i2c, &param);
    OS_printf("I2C Write Status: 0x%08X\n", Status);
    if (Status == CFE_SUCCESS) {
        OS_printf("PAYUEL_AOS_WRITE_REGISTER Done!!");
    }

    return CFE_SUCCESS;
}

CFE_Status_t PAYUEL_AOS_Read_RegisterCmd(const PAYUEL_AOS_Read_RegisterCmd_t *Msg)
{
    CFE_SRL_IO_Param_t param = {0,};

    uint8_t TxBuf[1] = {0};
    TxBuf[0] = Msg->reg_addr;
    uint8_t RxBuf[2] = {0};

    param.TxData = TxBuf;
    param.TxSize = sizeof(TxBuf);
    param.RxData = RxBuf;
    param.RxSize = sizeof(RxBuf);
    param.Addr = 0x48;
    param.Timeout = 100;

    int32 Status = CFE_SRL_ApiRead(i2c, &param);
    OS_printf("rx[0]= 0x%02X\n",RxBuf[0]);
    OS_printf("rx[1]= 0x%02X\n",RxBuf[1]);
    int16 raw = ((int16)RxBuf[0] << 8) | RxBuf[1];  //big-endian
    float Vout = raw * (6.144f / 32768.0f); //PGA=000b 일 때
  

    OS_printf("I2C Read Status: 0x%08X\n", Status);
    if (Status == CFE_SUCCESS){
        if (Msg->reg_addr == 0x00){
            float R = RREF*(VCC/Vout-1.0f);
            OS_printf("PAYUEL_AOS_READ_REGISTER Done!!\n"
                "Received MSB Data: 0x%02X\n"
                "Received LSB Data: 0x%02X\n"
                "Calculated Voltage: %.4f V\n"
                "Calculated Resistance: %.4f Ohm\n", RxBuf[0], RxBuf[1], Vout, R);
        }else{
            OS_printf("PAYUEL_AOS_READ_REGISTER Done!!\n Received MSB Data: 0x%02X\n Received LSB Data: 0x%02X\n", RxBuf[0], RxBuf[1]);
        } 
    }
    

    return CFE_SUCCESS;
}


// 각 채널별 참조 저항값
const float RREF_TABLE[4] = {100.0f, 100.64f, 98.45f, 100.54f};


CFE_Status_t PAYUEL_AOS_ReadAllChannelsCmd(const PAYUEL_AOS_ReadAllChannelsCmd_t *Msg)
{
    CFE_SRL_IO_Param_t param = {0,};
    float vout_results[4] = {0.0f};
    float rx_results[4] = {0.0f};

    int32 Status;
    int32 FileStatus;
    osal_id_t FileHandle;

    for (int ch = 0; ch < 4; ch++)
    {
        //Config 레지스터 설정 및 변환 시작
        // 기본값: OS=1, PGA=000(6.144V), MODE=1(Single), DR=100(128SPS), COMP=00011
        // MUX: (4 + ch) << 12  => A0=0x4000, A1=0x5000, A2=0x6000, A3=0x7000
        uint16_t configVal = 0x8183 | ((4 + ch) << 12);
        
        uint8_t TxBuf[3];
        TxBuf[0] = ADS_REG_CONF; //config : 0x01
        TxBuf[1] = (uint8_t)(configVal >> 8);
        TxBuf[2] = (uint8_t)(configVal & 0xFF);

        param.TxData = TxBuf;
        param.TxSize = 3;
        param.Addr = ADS_I2C_ADDR;
        param.Timeout = 100;

        Status = CFE_SRL_ApiWrite(i2c, &param);
        if (Status != CFE_SUCCESS) {
            OS_printf("Failed to write Config for Ch%d (Status: 0x%08X)\n", ch, Status);
            continue;
        }

        usleep(1000000); // read <-> write 사이에 1초 대기

        // Conversion 레지스터(0x00) 읽기
        uint8_t regAddr = ADS_REG_CONV;
        uint8_t RxBuf[2] = {0,};
        
        param.TxData = &regAddr;
        param.TxSize = 1;
        param.RxData = RxBuf;
        param.RxSize = 2;
        param.Timeout = 1000;

        Status = CFE_SRL_ApiRead(i2c, &param);
        if (Status == CFE_SUCCESS) {
            int16_t raw = (int16_t)((RxBuf[0] << 8) | RxBuf[1]);
            
            // 전압 계산
            vout_results[ch] = raw * (6.144f / 32768.0f); //PGA=000b(6.144V) 일 때;
            
            // 저항 계산
            if (vout_results[ch] > 0.01f) {
                rx_results[ch] = RREF_TABLE[ch] * (VCC / vout_results[ch] - 1.0f);
            } else {
                rx_results[ch] = -1.0f; // 측정 불가 표시
            }
        }
    }

    // 디렉토리 확인 및 생성
    const char *dirPath = "/cf/AOS";
    OS_mkdir(dirPath, 0); // 이미 존재하면 에러를 뱉지만 무시하고 진행 가능

    // 파일 열기 및 추가(Append) 로직
    const char *filePath = "/cf/AOS/rx_data";
    
    // 파일이 없으면 생성(CREATE), 있으면 그대로 열기(NONE)
    FileStatus = OS_OpenCreate(&FileHandle, filePath, OS_FILE_FLAG_CREATE, OS_READ_WRITE);
    
    if (FileStatus == OS_SUCCESS || FileStatus == OS_ERR_NAME_TAKEN) {
        // 이미 파일이 존재하는 경우(OS_ERR_NAME_TAKEN) 다시 열기
        if (FileStatus == OS_ERR_NAME_TAKEN) {
            OS_OpenCreate(&FileHandle, filePath, OS_FILE_FLAG_NONE, OS_READ_WRITE);
        }

        // 파일 포인터를 가장 끝으로 이동 
        OS_lseek(FileHandle, 0, OS_SEEK_END);

        // float 배열 16바이트를 쌩으로 기록
        int32 bytesWritten = OS_write(FileHandle, rx_results, sizeof(rx_results));
        
        if (bytesWritten == sizeof(rx_results)) {
            OS_printf("AOS: Appended 16 bytes to %s\n", filePath);
        }
        
        OS_close(FileHandle);

    } else {
        OS_printf("AOS: File Open Error! (0x%08X)\n", FileStatus);
    }

    // 파일 출력
    OS_printf("\n======= ADS1115 4-Channel Measurement Result =======\n");
    for (int i = 0; i < 4; i++) {
        OS_printf("Channel %d: Voltage = %.4f V, Resistance = %.4f Ohm\n", 
                  i, vout_results[i], rx_results[i]);
    }
    OS_printf("====================================================\n");

    return CFE_SUCCESS;
}
