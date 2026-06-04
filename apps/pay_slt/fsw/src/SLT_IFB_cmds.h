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
 * @file
 *   This file contains the prototypes for the Sample App Ground Command-handling functions
 */

#ifndef SAMPLE_APP_CMDS_H
#define SAMPLE_APP_CMDS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "SLT_IFB_msg.h"
#include "SLT_IFB.h"

CFE_Status_t SLT_IFB_SendHkCmd(const SLT_IFB_SendHkCmd_t *Msg);
CFE_Status_t SLT_IFB_NoopCmd(const SLT_IFB_NoopCmd_t *Msg);
CFE_Status_t SLT_IFB_ResetCountersCmd(const SLT_IFB_ResetCountersCmd_t *Msg);

void SLT_IFB_CSP_CMP_Cmd(void);
void SLT_IFB_CSP_PING_Cmd(void);

void SLT_IFB_CSP_PS_Cmd(void);
void SLT_IFB_CSP_MEM_FREE_Cmd(void);
void SLT_IFB_CSP_REBOOT_Cmd(void);
void SLT_IFB_CSP_BUF_FREE_Cmd(void);
void SLT_IFB_CSP_UPTIME_Cmd(void);
void SLT_IFB_CSP_GNDWDT_Cmd(void);

void SLT_IFB_GET_BRD_UID_Cmd(char* Board_uid);
void SLT_IFB_GET_BRD_REV_Cmd(uint8* Board_revision);
void SLT_IFB_GET_CSP_ADDR_Cmd(uint8* MPU_CSP_address);
void SLT_IFB_GET_CAN_SPEED_Cmd(uint16* CAN_bus_speed);
void SLT_IFB_GET_I2C_ADDR_Cmd(uint8* I2C_address);
void SLT_IFB_GET_I2C_SPEED_Cmd(uint16* I2C_speed);
void SLT_IFB_GET_WDT_VAL_Cmd(uint32* WDT_value);
void SLT_IFB_GET_CSP_RTABLE_Cmd(char* MPU_CSP_routing_table);

void SLT_IFB_GET_SYS_STATUS_Cmd(int16* System_status);
void SLT_IFB_GET_SYS_UPTIME_Cmd(uint32 *System_uptime);
void SLT_IFB_GET_BOOT_CNT_Cmd(uint16* System_boot_count);
void SLT_IFB_GET_BOOT_CAUSE_Cmd(uint16 *Last_boot_cause);
void SLT_IFB_GET_REBOOT_CAUSE_Cmd(uint16 *Last_boot_cause);
void SLT_IFB_GET_WDT_LEFT_Cmd(uint32 *Time_left_WDT_cause_reboot);
void SLT_IFB_GET_BRD_TEMP_Cmd(uint16 *Board_temperature);
void SLT_IFB_GET_PWR_CURRENT_Cmd(uint16 *System_power_current);
void SLT_IFB_GET_IMU_DATA_Cmd(uint16* IMU_sensor_data);
void SLT_IFB_GET_NTC_DATA_Cmd(int16* NTC_sensor_data);

void SLT_IFB_SAVE_TABLE0_Cmd(void);
void SLT_IFB_SAVE_TABLE1_Cmd(void);
void SLT_IFB_SAVE_TABLE4_Cmd(void);
void SLT_IFB_SAVE_ALL_TABLE_Cmd(void);

#endif /* SAMPLE_APP_CMDS_H */