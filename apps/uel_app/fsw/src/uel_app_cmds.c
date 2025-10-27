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
 *   This file contains the source code for the UEL Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "uel_app.h"
#include "uel_app_cmds.h"
#include "uel_app_msgids.h"
#include "uel_app_eventids.h"
#include "uel_app_version.h"
#include "uel_app_tbl.h"
#include "uel_app_utils.h"
#include "uel_app_msg.h"
#include <stdbool.h>
#include <stdio.h>
#include <fcntl.h>   // open, O_CREAT, O_WRONLY, O_TRUNC
#include <unistd.h>  // write, close

/* The uel_lib module provides the UEL_Function() prototype */
//#include "uel_lib.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */


CFE_Status_t UEL_APP_SendBcnCmd(const UEL_APP_SendBcnCmd_t *Msg)
{
    CFE_Status_t status;
    uint8_t TxData[1] = {UEL_APP_ID_SendBcn};
    uint8_t RxData[25] = {0};

    status = CFE_SRL_ApiTransactionCSP(
        UEL_APP_UEL_CAM_NODE,
        UEL_APP_PORT,
        TxData,
        sizeof(TxData),
        RxData,
        sizeof(RxData)
    );

    if (status > 0){
        
        uint8_t *p = &RxData[1];
        int idx = 0;

        UEL_APP_Data.bcn.Payload.PI_boot_State = p[idx++];
        UEL_APP_Data.bcn.Payload.PI_boot_Count = (uint16_t)(p[idx] | (p[idx+1] << 8)); idx += 2;
        UEL_APP_Data.bcn.Payload.CAM_detect_State = p[idx++];

        UEL_APP_Data.bcn.Payload.IMU_ax = (int16_t)(p[idx] | (p[idx+1] << 8)); idx += 2;
        UEL_APP_Data.bcn.Payload.IMU_ay = (int16_t)(p[idx] | (p[idx+1] << 8)); idx += 2;
        UEL_APP_Data.bcn.Payload.IMU_az = (int16_t)(p[idx] | (p[idx+1] << 8)); idx += 2;
        UEL_APP_Data.bcn.Payload.IMU_temp = (int16_t)(p[idx] | (p[idx+1] << 8)); idx += 2;

        UEL_APP_Data.bcn.Payload.ESC_Ia_mA = (int16_t)(p[idx] | (p[idx+1] << 8)); idx += 2;
        UEL_APP_Data.bcn.Payload.ESC_Ib_mA = (int16_t)(p[idx] | (p[idx+1] << 8)); idx += 2;
        UEL_APP_Data.bcn.Payload.ESC_Ic_mA = (int16_t)(p[idx] | (p[idx+1] << 8)); idx += 2;
        UEL_APP_Data.bcn.Payload.ESC_I_rms_true_mA = (int16_t)(p[idx] | (p[idx+1] << 8)); idx += 2;
        UEL_APP_Data.bcn.Payload.ESC_temp_C = (int16_t)(p[idx] | (p[idx+1] << 8)); idx += 2;

        UEL_APP_Data.bcn.Payload.CAM_Shutter_Count = (uint16_t)(p[idx] | (p[idx+1] << 8)); idx += 2;

        CFE_SB_TimeStampMsg(CFE_MSG_PTR(UEL_APP_Data.bcn.TelemetryHeader));
        CFE_SB_TransmitMsg(CFE_MSG_PTR(UEL_APP_Data.bcn.TelemetryHeader), true);

        OS_printf("Beacon sent succesfully");

        OS_printf("Beacon: PI_State=%d, PI_Count=%d, CAM_State=%d, Shutter=%d\n",
                UEL_APP_Data.bcn.Payload.PI_boot_State,
                UEL_APP_Data.bcn.Payload.PI_boot_Count,
                UEL_APP_Data.bcn.Payload.CAM_detect_State,
                UEL_APP_Data.bcn.Payload.CAM_Shutter_Count);
        OS_printf("IMU: ax=%d, ay=%d, az=%d, temp=%d\n",
                UEL_APP_Data.bcn.Payload.IMU_ax,
                UEL_APP_Data.bcn.Payload.IMU_ay,
                UEL_APP_Data.bcn.Payload.IMU_az,
                UEL_APP_Data.bcn.Payload.IMU_temp);
        OS_printf("ESC: Ia=%d, Ib=%d, Ic=%d, I_rms=%d, temp=%d\n",
                UEL_APP_Data.bcn.Payload.ESC_Ia_mA,
                UEL_APP_Data.bcn.Payload.ESC_Ib_mA,
                UEL_APP_Data.bcn.Payload.ESC_Ic_mA,
                UEL_APP_Data.bcn.Payload.ESC_I_rms_true_mA,
                UEL_APP_Data.bcn.Payload.ESC_temp_C);

        

        UEL_APP_Data.CmdCounter++;
    }else{
        CFE_EVS_SendEvent(UEL_APP_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UEL_APP: Failed to send beacon command, status=0x%08X", status);
        UEL_APP_Data.ErrCounter++;
        return status;
    }


    
    return CFE_SUCCESS;
}



CFE_Status_t UEL_APP_NoopCmd(const UEL_APP_NoopCmd_t *Msg)
{
    UEL_APP_Data.CmdCounter++;
    

    CFE_EVS_SendEvent(UEL_APP_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "UEL: NOOP command %s",
                      UEL_APP_VERSION);

    return CFE_SUCCESS;
}

CFE_Status_t UEL_APP_ResetCountersCmd(const UEL_APP_ResetCountersCmd_t *Msg)
{
    UEL_APP_Data.CmdCounter = 0;
    UEL_APP_Data.ErrCounter = 0;

    CFE_EVS_SendEvent(UEL_APP_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "UEL: RESET command");

    return CFE_SUCCESS;
}


CFE_Status_t UEL_APP_GetSensData(const UEL_APP_GetSensDataCmd_t *Msg)
{
    UEL_APP_Data.CmdCounter++;
    uint8_t TxData[1] = {UEL_APP_ID_GetSensData};  
    uint8_t RxData[41] = {0};    
    
    int32 status = CFE_SRL_ApiTransactionCSP(
        UEL_APP_UEL_CAM_NODE,
        UEL_APP_PORT,
        TxData,
        sizeof(TxData),
        RxData,
        sizeof(RxData)
    );
    
    if (status > 0 ) {
        if (RxData[0] == UEL_APP_ID_GetSensData) {  
            uint8_t *p = &RxData[1];
            int idx = 0;
            
            
            int16_t IMU_cnt = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t IMU_diag = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t IMU_gx = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t IMU_gy = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t IMU_gz = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t IMU_ax = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t IMU_ay = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t IMU_az = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t IMU_temp = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t IMU_chk_ok = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            
            
            int16_t ESC_Ia_mA = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t ESC_Ib_mA = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t ESC_Ic_mA = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t ESC_I_rms_true_mA = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t ESC_I_std_abs_mA = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t ESC_temp_C = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t ESC_vbus_mV = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t ESC_timestamp_ms = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t ESC_esc_seq = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t ESC_crc16 = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;


            OS_printf("[Sensor] IMU: gx=%d, gy=%d, gz=%d, ax=%d, ay=%d, az=%d, temp=%d\n",
                     IMU_gx, IMU_gy, IMU_gz, IMU_ax, IMU_ay, IMU_az, IMU_temp);
            OS_printf("[Sensor] ESC: Ia=%d, Ib=%d, Ic=%d, Vbus=%d, Temp=%d\n",
                     ESC_Ia_mA, ESC_Ib_mA, ESC_Ic_mA, ESC_vbus_mV, ESC_temp_C);

           
            OS_printf("[Sensor][IMU extra] cnt=%d, diag=%d, chk_ok=%d, temp=%d\n",
                    IMU_cnt, IMU_diag, IMU_chk_ok, IMU_temp);

            OS_printf("[Sensor][ESC extra] I_rms_true=%d, I_std_abs=%d, ts_ms=%d, esc_seq=%d, crc16=0x%04X\n",
                    ESC_I_rms_true_mA, ESC_I_std_abs_mA, ESC_timestamp_ms, ESC_esc_seq, (uint16_t)ESC_crc16);
        }
    }else{
        CFE_EVS_SendEvent(UEL_APP_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UEL_APP: Failed to GetSensData command, status=0x%08X", status);
        UEL_APP_Data.ErrCounter++;
    }
    
    
    RPT_Report_t report = (RPT_Report_t){0};
    report.MsgID        = UEL_APP_CMD_MID;
    report.CommandCode  = UEL_APP_GET_SENS_DATA_CC;
    report.ReturnType   = (status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
    report.ReturnCode   = status;
    report.ReturnDataSize = 40;
    memcpy(report.ReturnValue, &RxData[1], report.ReturnDataSize);
    UEL_APP_Data.rpt.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UEL_APP_Data.rpt.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(UEL_APP_Data.rpt.TelemetryHeader), true);
   

    return status;
}


CFE_Status_t UEL_APP_SetCamPowerOnCmd(const UEL_APP_SetCamPowerCmd_t *Msg)
{
    UEL_APP_Data.CmdCounter++;

    uint8_t TxData[1] = {UEL_APP_ID_SetCamPower};
    uint8_t RxData[2];

    int32 status = CFE_SRL_ApiTransactionCSP(
        UEL_APP_UEL_OBC_NODE,  
        26,
        TxData,
        sizeof(TxData),
        RxData,
        sizeof(RxData)
    );


    if (status >0 ){
        if (RxData[0] == UEL_APP_ID_SetCamPower){
        OS_printf("Cam Power Ack: %u,%u\n", RxData[0], RxData[1]);
        }
    }else{
        CFE_EVS_SendEvent(UEL_APP_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UEL_APP: Failed to SetCamPowerOn command, status=0x%08X", status);
        UEL_APP_Data.ErrCounter++;
    }

    RPT_Report_t report = (RPT_Report_t){0};
    report.MsgID = UEL_APP_CMD_MID;
    report.CommandCode = UEL_APP_SET_CAM_POWER_ON_CC;
    report.ReturnType = (status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
    report.ReturnCode = status;
    report.ReturnDataSize = (status == CFE_SUCCESS && RxData[0] == UEL_APP_ID_SetCamPower) ? 1 : 0;
    if (report.ReturnDataSize == 1) memcpy(report.ReturnValue, &RxData[1], 1);
    UEL_APP_Data.rpt.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UEL_APP_Data.rpt.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(UEL_APP_Data.rpt.TelemetryHeader), true);

    return status;
}

CFE_Status_t UEL_APP_SetCamPowerOffCmd(const UEL_APP_SetCamPowerCmd_t *Msg)
{
    UEL_APP_Data.CmdCounter++;

    uint8_t TxData[1] = {UEL_APP_ID_SetCamPower};
    uint8_t RxData[2];

    int32 status = CFE_SRL_ApiTransactionCSP(
        UEL_APP_UEL_OBC_NODE,  
        27,
        TxData,
        sizeof(TxData),
        RxData,
        sizeof(RxData)
    );


    if (status >0 ){
        if (RxData[0] == UEL_APP_ID_SetCamPower){
        OS_printf("Cam Power Ack: %u,%u\n", RxData[0], RxData[1]);
        }
    }else{
        CFE_EVS_SendEvent(UEL_APP_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UEL_APP: Failed to SetCamPowerOff command, status=0x%08X", status);
        UEL_APP_Data.ErrCounter++;
    }

    RPT_Report_t report = (RPT_Report_t){0};
    report.MsgID = UEL_APP_CMD_MID;
    report.CommandCode = UEL_APP_SET_CAM_POWER_OFF_CC;
    report.ReturnType = (status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
    report.ReturnCode = status;
    report.ReturnDataSize = (status == CFE_SUCCESS && RxData[0] == UEL_APP_ID_SetCamPower) ? 1 : 0;
    if (report.ReturnDataSize == 1) memcpy(report.ReturnValue, &RxData[1], 1);
    UEL_APP_Data.rpt.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UEL_APP_Data.rpt.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(UEL_APP_Data.rpt.TelemetryHeader), true);

    return status;
}





CFE_Status_t UEL_APP_SetMotorMode(const UEL_APP_SetMotorMode_t *Msg) 
{
    UEL_APP_Data.CmdCounter++;
    uint8_t TxData[3] = {0x23, Msg->Mode, Msg->Seconds};
    uint8_t RxData[41] = {0};  // 총 전송 바이트: 41
    
    int32 status = CFE_SRL_ApiTransactionCSP(
        UEL_APP_UEL_CAM_NODE,
        UEL_APP_PORT,
        TxData,
        sizeof(TxData),
        RxData,
        sizeof(RxData)
    );
    
    if (status > 0) {
        if (RxData[0] == 0x23) {  // Cmd 확인
            uint8_t *p = &RxData[1];
            int idx = 0;
            
            // IMU 데이터 (각 int16, 2byte)
            int16_t IMU_cnt = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t IMU_diag = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t IMU_gx = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t IMU_gy = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t IMU_gz = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t IMU_ax = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t IMU_ay = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t IMU_az = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t IMU_temp = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t IMU_chk_ok = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            
            // ESC 데이터 (각 int16, 2byte)
            int16_t ESC_Ia_mA = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t ESC_Ib_mA = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t ESC_Ic_mA = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t ESC_I_rms_true_mA = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t ESC_I_std_abs_mA = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t ESC_temp_C = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t ESC_vbus_mV = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t ESC_timestamp_ms = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t ESC_esc_seq = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            int16_t ESC_crc16 = (int16_t)((p[idx] << 8) | p[idx+1]); idx += 2;
            
            OS_printf("[Motor][IMU] cnt=%d, diag=%d, chk_ok=%d\n",
          IMU_cnt, IMU_diag, IMU_chk_ok);

            OS_printf("[Motor][IMU] gx=%d, gy=%d, gz=%d, ax=%d, ay=%d, az=%d, temp=%d\n",
                    IMU_gx, IMU_gy, IMU_gz, IMU_ax, IMU_ay, IMU_az, IMU_temp);

            OS_printf("[Motor][ESC] Ia=%d, Ib=%d, Ic=%d, I_rms_true=%d, I_std_abs=%d, Vbus=%d, Temp=%d\n",
                    ESC_Ia_mA, ESC_Ib_mA, ESC_Ic_mA, ESC_I_rms_true_mA, ESC_I_std_abs_mA,
                    ESC_vbus_mV, ESC_temp_C);

            OS_printf("[Motor][ESC] ts_ms=%d, esc_seq=%d, crc16=0x%04X\n",
                    ESC_timestamp_ms, ESC_esc_seq, (uint16_t)ESC_crc16);
        }
    }else{
        CFE_EVS_SendEvent(UEL_APP_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UEL_APP: Failed to SetMotorMode command, status=0x%08X", status);
        UEL_APP_Data.ErrCounter++;
    }

    RPT_Report_t report = (RPT_Report_t){0};
    report.MsgID = UEL_APP_CMD_MID;
    report.CommandCode = UEL_APP_SET_MOTOR_MODE_CC;
    report.ReturnType = (status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
    report.ReturnCode = status;
    report.ReturnDataSize = 40; 
    memcpy(report.ReturnValue, &RxData[1], 40);
    UEL_APP_Data.rpt.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UEL_APP_Data.rpt.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(UEL_APP_Data.rpt.TelemetryHeader), true);

    return status;
}


CFE_Status_t UEL_APP_SetCamShotCmd(const UEL_APP_SetCamShotCmd_t *Msg)
{
    UEL_APP_Data.CmdCounter++;

    uint8_t TxData[2];
    TxData[0] = UEL_APP_ID_SetCamShotCmd;   
    TxData[1] = Msg->ImageSlot;
    
    uint8_t RxData[2];
   

    int32 status = CFE_SRL_ApiTransactionCSP(
        UEL_APP_UEL_CAM_NODE,
        UEL_APP_PORT,
        TxData,
        sizeof(TxData),
        RxData,
        sizeof(RxData)
    );

    if (status > 0){
        if (RxData[0] == UEL_APP_ID_SetCamShotCmd){
        OS_printf("Set Cam Shot Cmd Success\n");
        }
    }else{
        CFE_EVS_SendEvent(UEL_APP_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UEL_APP: Failed to SetCamShot command, status=0x%08X", status);
        UEL_APP_Data.ErrCounter++;
    }

    RPT_Report_t report = (RPT_Report_t){0};
    report.MsgID = UEL_APP_CMD_MID;
    report.CommandCode = UEL_APP_SET_CAM_SHOT_CC;
    report.ReturnType = (status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
    report.ReturnCode = status;
    report.ReturnDataSize = 1;
    memcpy(report.ReturnValue,&RxData[1],1);
    UEL_APP_Data.rpt.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UEL_APP_Data.rpt.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(UEL_APP_Data.rpt.TelemetryHeader), true);

    return status;
}


CFE_Status_t UEL_APP_SetTerminalCmd(const UEL_APP_SetTerminalCmd_t *Msg)
{
    UEL_APP_Data.CmdCounter++;
    uint8_t TxData[128] = {0};
    TxData[0] = UEL_APP_ID_SetTerminalCmd;  
    uint16_t cmdLength = Msg->CMDLength;
    memcpy(&TxData[1], &cmdLength, sizeof(uint16_t));
    
    // CMD[Length] 배열
    memcpy(&TxData[3], Msg->CMD, cmdLength);
    
    // 전송 데이터 총 길이: 1 + 2 + cmdLength
    size_t txLength = 3 + cmdLength;
    
    // 응답 데이터 버퍼 (Cmd 1byte + Response Length 2byte + Response[253-Length] 가변)
    uint8_t RxData[256] = {0};
    
    int32 status = CFE_SRL_ApiTransactionCSP(
        UEL_APP_UEL_CAM_NODE,
        UEL_APP_PORT,
        TxData,
        txLength,  // 실제 전송 길이
        RxData,
        sizeof(RxData)
    );
    
    if (status >0) {
  
        if (RxData[0] == UEL_APP_ID_SetTerminalCmd) {
            
            uint16_t responseLength;
            memcpy(&responseLength, &RxData[1], sizeof(uint16_t));
            OS_printf("Set Terminal Cmd Success\n");
            OS_printf("Response: %.*s\n", responseLength, &RxData[3]);
        }
    }else{
        CFE_EVS_SendEvent(UEL_APP_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UEL_APP: Failed to SetTerminal command, status=0x%08X", status);
        UEL_APP_Data.ErrCounter++;
    }

    RPT_Report_t report = (RPT_Report_t){0};
    report.MsgID = UEL_APP_CMD_MID;
    report.CommandCode = UEL_APP_SET_TERMINAL_CC;
    report.ReturnType = (status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
    report.ReturnCode = status;
    uint16_t resp_len = 0;
    memcpy(&resp_len, &RxData[1], sizeof(uint16_t));
    uint16_t total = (uint16_t)(2 + resp_len);      /* length(2) + data */
    if (total > sizeof(report.ReturnValue)) total = sizeof(report.ReturnValue);
    report.ReturnDataSize = total;
    memcpy(report.ReturnValue, &RxData[1], total);  /* [len(2)|data...] */
    UEL_APP_Data.rpt.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UEL_APP_Data.rpt.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(UEL_APP_Data.rpt.TelemetryHeader), true);
    
    return status;
}

CFE_Status_t UEL_APP_GetCamShotStatus(const UEL_APP_GetCamShootStatusCmd_t *Msg)
{
    UEL_APP_Data.CmdCounter++;

    
    uint8_t TxData[3] = { UEL_APP_ID_GetCamShotStat, Msg->ImageSlot, Msg->ImageNumber };
    
    uint8_t RxData[6] = {0};

    int32 status = CFE_SRL_ApiTransactionCSP(
        UEL_APP_UEL_CAM_NODE,
        UEL_APP_PORT,
        TxData,
        sizeof(TxData),
        RxData,
        sizeof(RxData)
    );

    if (status > 0){
        if (RxData[0] == UEL_APP_ID_GetCamShotStat){
            uint8_t  img_slot   = RxData[1];
            uint8_t  img_num    = RxData[2];
            uint16_t total_chunk = (uint16_t)RxData[3] | ((uint16_t)RxData[4] << 8); /* Little Endian */
            uint8_t  last_chunk_size = RxData[5];

            OS_printf("Img Slot:%u, Img Num:%u, Total Chunk:%u, Last Chunk Size:%u\n",
                      img_slot, img_num, total_chunk, last_chunk_size);
        }
    }else{
        CFE_EVS_SendEvent(UEL_APP_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UEL_APP: Failed to GetCamShotStatus command, status=0x%08X", status);
        UEL_APP_Data.ErrCounter++;
    }

    
    {
        RPT_Report_t report = (RPT_Report_t){0};
        report.MsgID        = UEL_APP_CMD_MID;
        report.CommandCode  = UEL_APP_GET_CAM_SHOT_STATUS_CC;
        report.ReturnType   = (status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
        report.ReturnCode   = status;
        report.ReturnDataSize = 5;
        memcpy(report.ReturnValue, &RxData[1], 5); 
        UEL_APP_Data.rpt.Payload = report;
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(UEL_APP_Data.rpt.TelemetryHeader));
        CFE_SB_TransmitMsg(CFE_MSG_PTR(UEL_APP_Data.rpt.TelemetryHeader), true);
    }

    return status;
}


CFE_Status_t UEL_APP_GetCamImageCmd(const UEL_APP_GetCamImageCmd_t *Msg)
{
    UEL_APP_Data.CmdCounter++;

    uint8_t TxData[5];
    TxData[0] = UEL_APP_ID_GetCamImage;  
    TxData[1] = Msg->ImgSlot;
    TxData[2] = Msg->ImgNumber;
    TxData[3] = (uint8_t)(Msg->ChunkNumber & 0xFF);        // Little Endian LSB
    TxData[4] = (uint8_t)((Msg->ChunkNumber >> 8) & 0xFF); // Little Endian MSB

    uint8_t RxData[256] = {0};

    int32 status = CFE_SRL_ApiTransactionCSP(
        UEL_APP_UEL_CAM_NODE,
        UEL_APP_PORT,
        TxData,
        sizeof(TxData),
        RxData,
        sizeof(RxData)
    );

    if (status > 0)
    {
        if (RxData[0] == UEL_APP_ID_GetCamImage)
        {
            uint8_t  imgSlot     = RxData[1];
            uint8_t  imgNumber   = RxData[2];
            uint16_t chunkNumber = (uint16_t)RxData[4] | ((uint16_t)RxData[3] << 8);

            OS_printf("Image Downloading: Img Slot:%u, Img No:%u, Chunk number:%u\n",
                      imgSlot, imgNumber, chunkNumber);

            
            uint8_t *p = &RxData[5];
            uint16_t dataSize = 251;

            char filename[64];
            snprintf(filename, sizeof(filename),
                     "./cf/sdcard/img_%u_%u.bin", imgSlot, chunkNumber);

            
            int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
            if (fd < 0)
            {
                CFE_EVS_SendEvent(UEL_APP_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "UEL_APP: Failed to open file %s", filename);
                return CFE_STATUS_FILE_READ_ERROR;
            }

            
            ssize_t written = write(fd, p, dataSize);
            if (written != dataSize)
            {
                CFE_EVS_SendEvent(UEL_APP_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "UEL_APP: File write incomplete (%zd/%u)", written, dataSize);
            }

            
            close(fd);

            OS_printf("Image Downloaded!! (%s)\n", filename);
        }
    }else{
        CFE_EVS_SendEvent(UEL_APP_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UEL_APP: Failed to GetCamImag command, status=0x%08X", status);
        UEL_APP_Data.ErrCounter++;
    }

    RPT_Report_t report = (RPT_Report_t){0};
    report.MsgID = UEL_APP_CMD_MID;
    report.CommandCode = UEL_APP_GET_CAM_IMAGE_CC;
    report.ReturnType = (status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
    report.ReturnCode = status;
    report.ReturnDataSize = 0; 
    UEL_APP_Data.rpt.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UEL_APP_Data.rpt.TelemetryHeader));
    (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UEL_APP_Data.rpt.TelemetryHeader), true);

    return status;
}


CFE_Status_t UEL_APP_DownloadImgCmd(const UEL_APP_DownloadAllCmd_t *Msg) {

    uint8_t TxData[3] = { UEL_APP_ID_GetCamShotStat, Msg->ImageSlot, Msg->ImageNumber };
    
    uint8_t RxData[6] = {0};

    /* First, request the image information */
    int32 status = CFE_SRL_ApiTransactionCSP(
        UEL_APP_UEL_CAM_NODE,
        UEL_APP_PORT,
        TxData,
        sizeof(TxData),
        RxData,
        sizeof(RxData)
    );
    if (status == 0) {
        CFE_EVS_SendEvent(UEL_APP_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UEL_APP: Failed to GetCamShotStatus command, status=0x%08X", status);
        UEL_APP_Data.ErrCounter++;
        status = CFE_SRL_TRANSACTION_ERR;
        goto report;
    }
    else if (status != sizeof(RxData)) { // Transaction Fail
        CFE_EVS_SendEvent(UEL_APP_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UEL_APP: Failed to GetCamShotStatus command, status=0x%08X", status);
        UEL_APP_Data.ErrCounter++;
        status = CFE_SRL_TRANSACTION_ERR;
        goto report;
    }
    else status = CFE_SUCCESS;

    /* Configure the download task args */
    OS_MutSemTake(UEL_APP_Data.MutexId);
    UEL_APP_Data.ImgSlot = RxData[1];
    UEL_APP_Data.ImgNumber = RxData[2];
    UEL_APP_Data.TotChunk = (uint16_t)RxData[3] | ((uint16_t)RxData[4] << 8); /* Little Endian */
    UEL_APP_Data.LastChunkSize = RxData[5];
    UEL_APP_Data.StartChunk = Msg->StartChunNumber;
    UEL_APP_Data.EndChunk = Msg->EndChunNumber;
    OS_MutSemGive(UEL_APP_Data.MutexId);

    /* Create Download child */
    CFE_ES_CreateChildTask(&UEL_APP_Data.TaskId, UEL_APP_CHILD_NAME, UEL_APP_DownloadTask,
                            CFE_ES_TASK_STACK_ALLOCATE, UEL_APP_CHILD_STACK_SIZE(2), UEL_APP_CHILD_PRIORITY, 0);

report:
    RPT_Report_t report = (RPT_Report_t){0};
    report.MsgID = UEL_APP_CMD_MID;
    report.CommandCode = UEL_APP_DOWNLOAD_IMG_CC;
    report.ReturnType = (status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
    report.ReturnCode = status;
    report.ReturnDataSize = 0; 
    UEL_APP_Data.rpt.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UEL_APP_Data.rpt.TelemetryHeader));
    (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UEL_APP_Data.rpt.TelemetryHeader), true);

    return CFE_SUCCESS;

}











