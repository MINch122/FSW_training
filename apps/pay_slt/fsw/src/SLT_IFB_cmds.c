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
#include "SLT_IFB_app.h"
#include "SLT_IFB_cmds.h"
#include "SLT_IFB_msgids.h"
#include "SLT_IFB_eventids.h"
#include "SLT_IFB_version.h"
#include "SLT_IFB_tbl.h"
#include "SLT_IFB_utils.h"

static inline void SLT_IFB_rptsend(uint16 MsgID, uint8 CommandCode, uint8 ReturnType, int32 ReturnCode, int16 ReturnDatasize, uint8* ReturnValue)
{
    memset(&SLT_IFB_Data.RptPkt, 0, sizeof(SLT_IFB_Data.RptPkt));
    CFE_MSG_Init(CFE_MSG_PTR(SLT_IFB_Data.RptPkt.TelemetryHeader), CFE_SB_ValueToMsgId(SLT_IFB_RPT_TLM_MID),
                 sizeof(SLT_IFB_Data.RptPkt));

    SLT_IFB_Data.RptPkt.Report.MsgID = MsgID;
    SLT_IFB_Data.RptPkt.Report.CommandCode = CommandCode;
    SLT_IFB_Data.RptPkt.Report.ReturnType = ReturnType;
    SLT_IFB_Data.RptPkt.Report.ReturnCode = ReturnCode;
    SLT_IFB_Data.RptPkt.Report.ReturnDataSize = ReturnDatasize;

    size_t datasize = ReturnDatasize > RPT_RET_VALUE_BUF_SIZE ? RPT_RET_VALUE_BUF_SIZE : ReturnDatasize;
    SLT_IFB_Data.RptPkt.Report.ReturnDataSize = (int16)datasize;
    if (datasize && ReturnValue) memcpy(SLT_IFB_Data.RptPkt.Report.ReturnValue, ReturnValue, datasize);

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(SLT_IFB_Data.RptPkt.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(SLT_IFB_Data.RptPkt.TelemetryHeader), true);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t SLT_IFB_SendHkCmd(const SLT_IFB_SendHkCmd_t *Msg)
{
    int i;

    /*
    ** Get command execution counters...
    */
    SLT_IFB_Data.HkTlm.Payload.CommandErrorCounter = SLT_IFB_Data.ErrCounter;
    SLT_IFB_Data.HkTlm.Payload.CommandCounter      = SLT_IFB_Data.CmdCounter;
    SLT_IFB_Data.HkTlm.Payload.sys_status          = SLT_IFB_Data.sys_status;
    SLT_IFB_Data.HkTlm.Payload.sys_uptime          = SLT_IFB_Data.sys_uptime;
    SLT_IFB_Data.HkTlm.Payload.boot_cnt            = SLT_IFB_Data.boot_cnt;
    SLT_IFB_Data.HkTlm.Payload.boot_cause          = SLT_IFB_Data.boot_cause;
    SLT_IFB_Data.HkTlm.Payload.reboot_cause        = SLT_IFB_Data.reboot_cause;
    SLT_IFB_Data.HkTlm.Payload.wdt_left            = SLT_IFB_Data.wdt_left;
    SLT_IFB_Data.HkTlm.Payload.brd_temp            = SLT_IFB_Data.brd_temp;
    SLT_IFB_Data.HkTlm.Payload.pwr_current         = SLT_IFB_Data.pwr_current;
    memcpy(SLT_IFB_Data.HkTlm.Payload.imu_data, SLT_IFB_Data.imu_data, sizeof(SLT_IFB_Data.imu_data));
    memcpy(SLT_IFB_Data.HkTlm.Payload.ntc_data, SLT_IFB_Data.ntc_data, sizeof(SLT_IFB_Data.ntc_data));

    /*
    ** Send housekeeping telemetry packet...
    */
    SLT_IFB_rptsend(SLT_IFB_CMD_MID, 0, RPT_RETTYPE_SUCCESS, CFE_SUCCESS, sizeof(SLT_IFB_Data.HkTlm.Payload), (uint8 *)&SLT_IFB_Data.HkTlm.Payload);

    /*
    ** Manage any pending table loads, validations, etc.
    */
    for (i = 0; i < SLT_IFB_NUMBER_OF_TABLES; i++)
    {
        CFE_TBL_Manage(SLT_IFB_Data.TblHandles[i]);
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SAMPLE NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t SLT_IFB_NoopCmd(const SLT_IFB_NoopCmd_t *Msg)
{
    SLT_IFB_Data.CmdCounter++;

    CFE_EVS_SendEvent(SLT_IFB_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "SLT_IFB : NOOP command %s", SLT_IFB_VERSION);
    SLT_IFB_rptsend(SLT_IFB_CMD_MID, SLT_IFB_NOOP_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, 0, NULL);

    return CFE_SUCCESS;
}

CFE_Status_t SLT_IFB_ResetCountersCmd(const SLT_IFB_ResetCountersCmd_t *Msg)
{
    SLT_IFB_Data.CmdCounter = 0;
    SLT_IFB_Data.AppErrCounter = 0;
    SLT_IFB_Data.DeviceErrCounter = 0;

    CFE_EVS_SendEvent(SLT_IFB_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "UTRX: RESET command");
    SLT_IFB_rptsend(SLT_IFB_CMD_MID, SLT_IFB_RESET_COUNTERS_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, 0, NULL);

    return CFE_SUCCESS;
}

/************************************************************************************************************************************/
void SLT_IFB_CSP_CMP_Cmd(void)
{
    int32 status;
    status = SLT_IFB_CSP_CMP();

    if (status > 0){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "CSP_CMP command success");
        SLT_IFB_rptsend(SLT_IFB_TRANSACTION_MID, SLT_IFB_CSP_CMP_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, 0, NULL);

    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_CSP_CMP_ERR_EID, CFE_EVS_EventType_ERROR, "CSP_CMP command fail");
        SLT_IFB_rptsend(SLT_IFB_TRANSACTION_MID, SLT_IFB_CSP_CMP_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_CSP_PING_Cmd(void)
{
    int32 status;
    status = SLT_IFB_CSP_PING();

    if (status > 0){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "CSP_PING command success");
        SLT_IFB_rptsend(SLT_IFB_TRANSACTION_MID, SLT_IFB_CSP_PING_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, 0, NULL);

    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_CSP_PING_ERR_EID, CFE_EVS_EventType_ERROR, "CSP_PING command fail");
        SLT_IFB_rptsend(SLT_IFB_TRANSACTION_MID, SLT_IFB_CSP_PING_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_CSP_PS_Cmd(void)
{
    int32 status;
    status = SLT_IFB_CSP_PS();

    if (status > 0){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "CSP_PS command success");
        SLT_IFB_rptsend(SLT_IFB_TRANSACTION_MID,SLT_IFB_CSP_PS_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, 0, NULL);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_CSP_PS_ERR_EID, CFE_EVS_EventType_ERROR, "CSP_PS command fail");
        SLT_IFB_rptsend(SLT_IFB_TRANSACTION_MID, SLT_IFB_CSP_PS_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_CSP_MEM_FREE_Cmd(void)
{
    int32 status;
    status = SLT_IFB_CSP_MEM_FREE();

    if (status > 0){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "CSP_MEM_FREE command success");
        SLT_IFB_rptsend(SLT_IFB_TRANSACTION_MID, SLT_IFB_CSP_MEM_FREE_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, 0, NULL);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_CSP_MEM_FREE_ERR_EID, CFE_EVS_EventType_ERROR, "CSP_MEM_FREE command fail");
        SLT_IFB_rptsend(SLT_IFB_TRANSACTION_MID, SLT_IFB_CSP_MEM_FREE_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_CSP_REBOOT_Cmd(void)
{
    int32 status;
    status = SLT_IFB_CSP_REBOOT();

    if (status > 0){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "CSP_REBOOT command success");
        SLT_IFB_rptsend(SLT_IFB_TRANSACTION_MID, SLT_IFB_CSP_REBOOT_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, 0, NULL);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_CSP_REBOOT_ERR_EID, CFE_EVS_EventType_ERROR, "CSP_REBOOT command fail");
        SLT_IFB_rptsend(SLT_IFB_TRANSACTION_MID, SLT_IFB_CSP_REBOOT_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_CSP_BUF_FREE_Cmd(void)
{
    int32 status;
    status = SLT_IFB_CSP_BUF_FREE();

    if (status > 0){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "CSP_BUF_FREE command success");
        SLT_IFB_rptsend(SLT_IFB_TRANSACTION_MID, SLT_IFB_CSP_BUF_FREE_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, 0, NULL);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_CSP_BUF_FREE_ERR_EID, CFE_EVS_EventType_ERROR, "CSP_BUF_FREE command fail");
        SLT_IFB_rptsend(SLT_IFB_TRANSACTION_MID, SLT_IFB_CSP_BUF_FREE_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_CSP_UPTIME_Cmd(void)
{
    int32 status;
    status = SLT_IFB_CSP_UPTIME();

    if (status > 0){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "CSP_UPTIME command success");
        SLT_IFB_rptsend(SLT_IFB_TRANSACTION_MID, SLT_IFB_CSP_UPTIME_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, 0, NULL);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_CSP_UPTIME_ERR_EID, CFE_EVS_EventType_ERROR, "CSP_UPTIME command fail");
        SLT_IFB_rptsend(SLT_IFB_TRANSACTION_MID, SLT_IFB_CSP_UPTIME_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_CSP_GNDWDT_Cmd(void)
{
    int32 status;
    status = SLT_IFB_CSP_GNDWDT();

    if (status > 0){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "CSP_GNDWDT command success");
        SLT_IFB_rptsend(SLT_IFB_TRANSACTION_MID, SLT_IFB_CSP_GNDWDT_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, 0, NULL);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_GNDWDT_ERR_EID, CFE_EVS_EventType_ERROR, "CSP_GNDWDT command fail");
        SLT_IFB_rptsend(SLT_IFB_TRANSACTION_MID, SLT_IFB_CSP_GNDWDT_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}
/***************************************************************************************************************************************************/
void SLT_IFB_GET_BRD_UID_Cmd(char* Board_uid)
{
    int32 status;
    status = SLT_IFB_GET_BRD_UID(Board_uid);

    if (status == CFE_SUCCESS){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "GET_BRD_UID command success");
        OS_printf("result : %s \n", Board_uid);
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_BRD_UID_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, sizeof(char) * 19, (uint8 *)Board_uid);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_GET_BRD_UID_ERR_EID, CFE_EVS_EventType_ERROR, "GET_BRD_UID command fail");
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_BRD_UID_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_GET_BRD_REV_Cmd(uint8* Board_revision)
{
    int32 status;
    status = SLT_IFB_GET_BRD_REV(Board_revision);
    
    if (status == CFE_SUCCESS){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "GET_BRD_REV command success");
        OS_printf("result : ");
        while(Board_revision){
            OS_printf("%u ", *Board_revision);
            Board_revision++;
        }
        OS_printf("\n");
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_BRD_REV_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, sizeof(uint8) * 3, (uint8 *)Board_revision);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_GET_BRD_REV_ERR_EID, CFE_EVS_EventType_ERROR, "GET_BRD_REV command fail");
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_BRD_REV_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_GET_CSP_ADDR_Cmd(uint8* MPU_CSP_address)
{
    int32 status;
    status = SLT_IFB_GET_CSP_ADDR(MPU_CSP_address);

    if (status == CFE_SUCCESS){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "GET_CSP_ADDR command success");
        OS_printf("result : %u \n", *MPU_CSP_address);
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_CSP_ADDR_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, sizeof(uint8), (uint8 *)MPU_CSP_address);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_GET_CSP_ADDR_ERR_EID, CFE_EVS_EventType_ERROR, "GET_CSP_ADDR command fail");
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_CSP_ADDR_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_GET_CAN_SPEED_Cmd(uint16* CAN_bus_speed)
{
    int32 status;
    status = SLT_IFB_GET_CAN_SPEED(CAN_bus_speed);

    if (status == CFE_SUCCESS){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "GET_CAN_SPEED command success");
        OS_printf("result : %u \n", *CAN_bus_speed);
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_CAN_SPEED_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, sizeof(uint16), (uint8 *)CAN_bus_speed);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_GET_CAN_SPEED_ERR_EID, CFE_EVS_EventType_ERROR, "GET_CAN_SPEED command fail");
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_CAN_SPEED_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_GET_I2C_ADDR_Cmd(uint8* I2C_address)
{
    int32 status;
    status = SLT_IFB_GET_I2C_ADDR(I2C_address);

    if (status == CFE_SUCCESS){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "GET_I2C_ADDR command success");
        OS_printf("result : 0x%02hx \n", *I2C_address);
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_I2C_ADDR_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, sizeof(uint8), (uint8 *)I2C_address);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_GET_I2C_ADDR_ERR_EID, CFE_EVS_EventType_ERROR, "GET_I2C_ADDR command error");
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_I2C_ADDR_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_GET_I2C_SPEED_Cmd(uint16* I2C_speed)
{
    int32 status;
    status = SLT_IFB_GET_I2C_SPEED(I2C_speed);

    if (status == CFE_SUCCESS){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "GET_I2C_SPEED command success");
        OS_printf("result : %u \n", *I2C_speed);
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_I2C_SPEED_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, sizeof(uint16), (uint8 *)I2C_speed);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_GET_I2C_SPEED_ERR_EID, CFE_EVS_EventType_ERROR, "GET_I2C_SPEED command fail");
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_I2C_SPEED_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_GET_WDT_VAL_Cmd(uint32* WDT_value)
{
    int32 status;
    status = SLT_IFB_GET_WDT_VAL(WDT_value);

    if (status == CFE_SUCCESS){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "GET_WDT_VAL command success");
        OS_printf("result : %u \n", *WDT_value);
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_WDT_VAL_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, sizeof(uint32), (uint8 *)WDT_value);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_GET_WDT_VAL_ERR_EID, CFE_EVS_EventType_ERROR, "GET_WDT_VAL command fail");
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_WDT_VAL_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_GET_CSP_RTABLE_Cmd(char* MPU_CSP_routing_table)
{
    int32 status;
    status = SLT_IFB_GET_CSP_RTABLE(MPU_CSP_routing_table);

    if (status == CFE_SUCCESS){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "GET_CSP_RTABLE command success");
        OS_printf("result : %s \n", MPU_CSP_routing_table);
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_CSP_RTABLE_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, sizeof(char) * 96, (uint8 *)MPU_CSP_routing_table);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_GET_CSP_RTABLE_ERR_EID, CFE_EVS_EventType_ERROR, "GET_CSP_RTABLE command fail");
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_CSP_RTABLE_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_GET_SYS_STATUS_Cmd(int16* System_status)
{
    int32 status;
    status = SLT_IFB_GET_SYS_STATUS(System_status);

    if (status == CFE_SUCCESS){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "GET_SYS_STATUS command success");
        SLT_IFB_Data.sys_status = *System_status;
        OS_printf("result : %d \n", *System_status);
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_SYS_STATUS_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, sizeof(int16), (uint8 *)System_status);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_GET_SYS_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "GET_SYS_STATUS command fail");
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_SYS_STATUS_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_GET_SYS_UPTIME_Cmd(uint32* System_uptime)
{
    int32 status;
    status = SLT_IFB_GET_SYS_UPTIME(System_uptime);

    if (status == CFE_SUCCESS){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "GET_SYS_UPTIME command success");
        SLT_IFB_Data.sys_uptime = *System_uptime;
        OS_printf("result : %u \n", *System_uptime);
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_SYS_UPTIME_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, sizeof(uint32), (uint8 *)System_uptime);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_GET_SYS_UPTIME_ERR_EID, CFE_EVS_EventType_ERROR, "GET_SYS_UPTIME command fail");
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_SYS_UPTIME_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_GET_BOOT_CNT_Cmd(uint16* System_boot_count)
{
    int32 status;
    status = SLT_IFB_GET_BOOT_CNT(System_boot_count);

    if (status == CFE_SUCCESS){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "GET_SYS_BOOT_CNT command success");
        SLT_IFB_Data.boot_cnt = *System_boot_count;
        OS_printf("result : %u \n", *System_boot_count);
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_BOOT_CNT_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, sizeof(uint16), (uint8 *)System_boot_count);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_GET_BOOT_CNT_ERR_EID, CFE_EVS_EventType_ERROR, "GET_SYS_BOOT_CNT command fail");
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_BOOT_CNT_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_GET_BOOT_CAUSE_Cmd(uint16* Last_boot_cause)
{
    int32 status;
    status = SLT_IFB_GET_BOOT_CAUSE(Last_boot_cause);

    if (status == CFE_SUCCESS){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "GET_BOOT_CAUSE command success");
        SLT_IFB_Data.boot_cause = *Last_boot_cause;
        OS_printf("result : %u \n", *Last_boot_cause);
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_BOOT_CAUSE_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, sizeof(uint16), (uint8 *)Last_boot_cause);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_GET_BOOT_CAUSE_ERR_EID, CFE_EVS_EventType_ERROR, "GET_BOOT_CAUSE command fail");
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_BOOT_CAUSE_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_GET_REBOOT_CAUSE_Cmd(uint16* Last_reboot_cause)
{
    int32 status;
    status = SLT_IFB_GET_REBOOT_CAUSE(Last_reboot_cause);

    if (status == CFE_SUCCESS){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "GET_REBOOT_CAUSE command success");
        SLT_IFB_Data.reboot_cause = *Last_reboot_cause;
        OS_printf("result : %u \n", *Last_reboot_cause);
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_REBOOT_CAUSE_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, sizeof(uint16), (uint8 *)Last_reboot_cause);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_GET_REBOOT_CAUSE_ERR_EID, CFE_EVS_EventType_ERROR, "GET_REBOOT_CAUSE command fail");
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_REBOOT_CAUSE_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_GET_WDT_LEFT_Cmd(uint32* Time_left_WDT_cause_reboot)
{
    int32 status;
    status = SLT_IFB_GET_WDT_LEFT(Time_left_WDT_cause_reboot);

    if (status == CFE_SUCCESS){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "GET_WDT_LEFT command success");
        SLT_IFB_Data.wdt_left = *Time_left_WDT_cause_reboot;
        OS_printf("result : %u \n", *Time_left_WDT_cause_reboot);
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_WDT_LEFT_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, sizeof(uint32), (uint8 *)Time_left_WDT_cause_reboot);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_GET_WDT_LEFT_ERR_EID, CFE_EVS_EventType_ERROR, "GET_WDT_LEFT command fail");
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_WDT_LEFT_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_GET_BRD_TEMP_Cmd(uint16* Board_temperature)
{
    int32 status;
    status = SLT_IFB_GET_BRD_TEMP(Board_temperature);

    if (status == CFE_SUCCESS){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "GET_BRD_TEMP command success");
        SLT_IFB_Data.brd_temp = *Board_temperature;
        OS_printf("result : %d \n", *Board_temperature);
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_BRD_TEMP_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, sizeof(int16), (uint8 *)Board_temperature);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_GET_BRD_TEMP_ERR_EID, CFE_EVS_EventType_ERROR, "GET_BRD_TEMP command fail");
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_BRD_TEMP_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_GET_PWR_CURRENT_Cmd(uint16* System_power_current)
{
    int32 status;
    status = SLT_IFB_GET_PWR_CURRENT(System_power_current);

    if (status == CFE_SUCCESS){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "GET_PWR_CURRENT command success");
        SLT_IFB_Data.pwr_current = *System_power_current;
        OS_printf("result : %u \n", *System_power_current);
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_PWR_CURRENT_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, sizeof(uint16), (uint8 *)System_power_current);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_GET_PWR_CURRENT_ERR_EID, CFE_EVS_EventType_ERROR, "GET_PWR_CURRENT command fail");
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_PWR_CURRENT_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_GET_IMU_DATA_Cmd(uint16* IMU_sensor_data)
{
    int32 status;
    status = SLT_IFB_GET_IMU_DATA(IMU_sensor_data);

    if (status == CFE_SUCCESS){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "GET_IMU_DATA command success");
        memcpy(SLT_IFB_Data.imu_data, IMU_sensor_data, sizeof(SLT_IFB_Data.imu_data));
        OS_printf("result : ");
        while(IMU_sensor_data){
            OS_printf("%u ", *IMU_sensor_data);
            IMU_sensor_data++;
        }
        OS_printf("\n");
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_IMU_DATA_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, sizeof(int16) * 6, (uint8 *)IMU_sensor_data);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_GET_IMU_DATA_ERR_EID, CFE_EVS_EventType_ERROR, "GET_IMU_DATA command fail");
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_IMU_DATA_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_GET_NTC_DATA_Cmd(int16* NTC_sensor_data)
{
    int32 status;
    status = SLT_IFB_GET_NTC_DATA(NTC_sensor_data);

    if (status == CFE_SUCCESS){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "GET_NTC_DATA command success");
        memcpy(SLT_IFB_Data.ntc_data, NTC_sensor_data, sizeof(SLT_IFB_Data.ntc_data));
        OS_printf("result : ");
        while(NTC_sensor_data){
            OS_printf("%d ", *NTC_sensor_data);
            NTC_sensor_data++;
        }
        OS_printf("\n");
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_NTC_DATA_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, sizeof(int16) * 4, (uint8 *)NTC_sensor_data);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_GET_NTC_DATA_ERR_EID, CFE_EVS_EventType_ERROR, "GET_NTC_DATA command fail");
        SLT_IFB_rptsend(SLT_IFB_GET_RPARAM_MID, SLT_IFB_GET_NTC_DATA_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

/***********************************************************************************************************************************************************/

void SLT_IFB_SAVE_TABLE0_Cmd(void)
{
    int32 status;
    status = SLT_IFB_SAVE_TABLE0();

    if (status == CFE_SUCCESS){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "SAVE_TABLE0 command success");
        SLT_IFB_rptsend(SLT_IFB_SAVE_TABLE_MID, SLT_IFB_SAVE_TABLE0_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, 0, NULL);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_SAVE_TABLE0_ERR_EID, CFE_EVS_EventType_ERROR, "SAVE_TABLE0 command fail");
        SLT_IFB_rptsend(SLT_IFB_SAVE_TABLE_MID, SLT_IFB_SAVE_TABLE0_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_SAVE_TABLE1_Cmd(void)
{
    int32 status;
    status = SLT_IFB_SAVE_TABLE1();

    if (status == CFE_SUCCESS){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "SAVE_TABLE1 command success");
        SLT_IFB_rptsend(SLT_IFB_SAVE_TABLE_MID, SLT_IFB_SAVE_TABLE1_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, 0, NULL);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_SAVE_TABLE1_ERR_EID, CFE_EVS_EventType_ERROR, "SAVE_TABLE1 command fail");
        SLT_IFB_rptsend(SLT_IFB_SAVE_TABLE_MID, SLT_IFB_SAVE_TABLE1_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_SAVE_TABLE4_Cmd(void)
{
    int32 status;
    status = SLT_IFB_SAVE_TABLE4();

    if (status == CFE_SUCCESS){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "SAVE_TABLE4 command success");
        SLT_IFB_rptsend(SLT_IFB_SAVE_TABLE_MID, SLT_IFB_SAVE_TABLE4_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, 0, NULL);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_SAVE_TABLE4_ERR_EID, CFE_EVS_EventType_ERROR, "SAVE_TABLE4 command fail");
        SLT_IFB_rptsend(SLT_IFB_SAVE_TABLE_MID, SLT_IFB_SAVE_TABLE4_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

void SLT_IFB_SAVE_ALL_TABLE_Cmd(void)
{
    int32 status;
    status = SLT_IFB_SAVE_TABLE0();

    if (status == CFE_SUCCESS){
        CFE_EVS_SendEvent(SLT_IFB_CMD_SUCCESS_EID, CFE_EVS_EventType_INFORMATION, "SAVE_ALL_TABLE command success");
        SLT_IFB_rptsend(SLT_IFB_SAVE_TABLE_MID, SLT_IFB_SAVE_ALL_TABLE_CC, RPT_RETTYPE_SUCCESS, SLT_IFB_CMD_SUCCESS_EID, 0, NULL);
    }
    else{
        CFE_EVS_SendEvent(SLT_IFB_SAVE_ALL_TABLE_ERR_EID, CFE_EVS_EventType_ERROR, "SAVE_ALL_TABLE command fail");
        OS_printf("error in Table %d", status-1);
        SLT_IFB_rptsend(SLT_IFB_SAVE_TABLE_MID, SLT_IFB_SAVE_ALL_TABLE_CC, RPT_RETTYPE_APP, status, 0, NULL);
    }
}

