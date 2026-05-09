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

#ifndef SAMPLE_APP_CMDS_H
#define SAMPLE_APP_CMDS_H

#include "cfe_error.h"
#include "pay_slt_msg.h"
#include "pay_slt.h"

CFE_Status_t SLT_IFB_SendHkCmd(const SLT_IFB_SendHkCmd_t *Msg);
CFE_Status_t PAY_SLT_SendBeaconCmd(const SLT_IFB_SendBcnCmd_t *Msg);
CFE_Status_t SLT_IFB_NoopCmd(const SLT_IFB_NoopCmd_t *Msg);
CFE_Status_t SLT_IFB_ResetCountersCmd(const SLT_IFB_ResetCountersCmd_t *Msg);

CFE_Status_t PAY_SLT_IFB_CSP_CMP_Cmd(const PAY_SLT_IFB_CSP_CMP_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_CSP_PING_Cmd(const PAY_SLT_IFB_CSP_PING_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_CSP_PS_Cmd(const PAY_SLT_IFB_CSP_PS_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_CSP_MEM_FREE_Cmd(const PAY_SLT_IFB_CSP_MEM_FREE_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_CSP_REBOOT_Cmd(const PAY_SLT_IFB_CSP_REBOOT_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_CSP_BUF_FREE_Cmd(const PAY_SLT_IFB_CSP_BUF_FREE_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_CSP_UPTIME_Cmd(const PAY_SLT_IFB_CSP_UPTIME_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_CSP_GNDWDT_Cmd(const PAY_SLT_IFB_CSP_GNDWDT_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_GET_BRD_UID_Cmd(const PAY_SLT_IFB_GET_BRD_UID_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_GET_BRD_REV_Cmd(const PAY_SLT_IFB_GET_BRD_REV_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_GET_CSP_ADDR_Cmd(const PAY_SLT_IFB_GET_CSP_ADDR_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_GET_CAN_SPEED_Cmd(const PAY_SLT_IFB_GET_CAN_SPEED_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_GET_I2C_ADDR_Cmd(const PAY_SLT_IFB_GET_I2C_ADDR_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_GET_I2C_SPEED_Cmd(const PAY_SLT_IFB_GET_I2C_SPEED_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_GET_WDT_VAL_Cmd(const PAY_SLT_IFB_GET_WDT_VAL_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_GET_CSP_RTABLE_Cmd(const PAY_SLT_IFB_GET_CSP_RTABLE_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_GET_SYS_STATUS_Cmd(const PAY_SLT_IFB_GET_SYS_STATUS_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_GET_SYS_UPTIME_Cmd(const PAY_SLT_IFB_GET_SYS_UPTIME_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_GET_BOOT_CNT_Cmd(const PAY_SLT_IFB_GET_BOOT_CNT_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_GET_BOOT_CAUSE_Cmd(const PAY_SLT_IFB_GET_BOOT_CAUSE_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_GET_REBOOT_CAUSE_Cmd(const PAY_SLT_IFB_GET_REBOOT_CAUSE_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_GET_WDT_LEFT_Cmd(const PAY_SLT_IFB_GET_WDT_LEFT_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_GET_BRD_TEMP_Cmd(const PAY_SLT_IFB_GET_BRD_TEMP_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_GET_PWR_CURRENT_Cmd(const PAY_SLT_IFB_GET_PWR_CURRENT_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_GET_IMU_DATA_Cmd(const PAY_SLT_IFB_GET_IMU_DATA_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_GET_NTC_DATA_Cmd(const PAY_SLT_IFB_GET_NTC_DATA_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_SAVE_TABLE0_Cmd(const PAY_SLT_IFB_SAVE_TABLE0_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_SAVE_TABLE1_Cmd(const PAY_SLT_IFB_SAVE_TABLE1_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_SAVE_TABLE4_Cmd(const PAY_SLT_IFB_SAVE_TABLE4_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_SAVE_ALL_TABLE_Cmd(const PAY_SLT_IFB_SAVE_ALL_TABLE_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_SET_CSP_ADDR_Cmd(const PAY_SLT_IFB_SET_CSP_ADDR_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_SET_CAN_SPEED_Cmd(const PAY_SLT_IFB_SET_CAN_SPEED_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_SET_I2C_ADDR_Cmd(const PAY_SLT_IFB_SET_I2C_ADDR_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_SET_I2C_SPEED_Cmd(const PAY_SLT_IFB_SET_I2C_SPEED_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_SET_WDT_VAL_Cmd(const PAY_SLT_IFB_SET_WDT_VAL_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_SET_CSP_RTABLE_Cmd(const PAY_SLT_IFB_SET_CSP_RTABLE_Cmd_t *Msg);

CFE_Status_t PAY_SLT_EXP_CSP_CMP_Cmd(const PAY_SLT_EXP_CSP_CMP_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_CSP_PING_Cmd(const PAY_SLT_EXP_CSP_PING_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_CSP_PS_Cmd(const PAY_SLT_EXP_CSP_PS_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_CSP_MEM_FREE_Cmd(const PAY_SLT_EXP_CSP_MEM_FREE_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_CSP_REBOOT_Cmd(const PAY_SLT_EXP_CSP_REBOOT_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_CSP_BUF_FREE_Cmd(const PAY_SLT_EXP_CSP_BUF_FREE_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_CSP_UPTIME_Cmd(const PAY_SLT_EXP_CSP_UPTIME_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_CSP_GNDWDT_Cmd(const PAY_SLT_EXP_CSP_GNDWDT_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_GET_BRD_UID_Cmd(const PAY_SLT_EXP_GET_BRD_UID_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_GET_BRD_REV_Cmd(const PAY_SLT_EXP_GET_BRD_REV_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_GET_CSP_ADDR_Cmd(const PAY_SLT_EXP_GET_CSP_ADDR_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_GET_CAN_SPEED_Cmd(const PAY_SLT_EXP_GET_CAN_SPEED_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_GET_I2C_ADDR_Cmd(const PAY_SLT_EXP_GET_I2C_ADDR_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_GET_I2C_SPEED_Cmd(const PAY_SLT_EXP_GET_I2C_SPEED_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_GET_WDT_VAL_Cmd(const PAY_SLT_EXP_GET_WDT_VAL_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_GET_CSP_RTABLE_Cmd(const PAY_SLT_EXP_GET_CSP_RTABLE_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_GET_SYS_STATUS_Cmd(const PAY_SLT_EXP_GET_SYS_STATUS_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_GET_SYS_UPTIME_Cmd(const PAY_SLT_EXP_GET_SYS_UPTIME_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_GET_BOOT_CNT_Cmd(const PAY_SLT_EXP_GET_BOOT_CNT_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_GET_BOOT_CAUSE_Cmd(const PAY_SLT_EXP_GET_BOOT_CAUSE_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_GET_REBOOT_CAUSE_Cmd(const PAY_SLT_EXP_GET_REBOOT_CAUSE_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_GET_WDT_LEFT_Cmd(const PAY_SLT_EXP_GET_WDT_LEFT_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_GET_BRD_TEMP_Cmd(const PAY_SLT_EXP_GET_BRD_TEMP_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_GET_PWR_CURRENT_Cmd(const PAY_SLT_EXP_GET_PWR_CURRENT_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_GET_IMU_DATA_Cmd(const PAY_SLT_EXP_GET_IMU_DATA_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_GET_NTC_DATA_Cmd(const PAY_SLT_EXP_GET_NTC_DATA_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_SAVE_TABLE0_Cmd(const PAY_SLT_EXP_SAVE_TABLE0_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_SAVE_TABLE1_Cmd(const PAY_SLT_EXP_SAVE_TABLE1_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_SAVE_TABLE4_Cmd(const PAY_SLT_EXP_SAVE_TABLE4_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_SAVE_ALL_TABLE_Cmd(const PAY_SLT_EXP_SAVE_ALL_TABLE_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_SET_CSP_ADDR_Cmd(const PAY_SLT_EXP_SET_CSP_ADDR_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_SET_CAN_SPEED_Cmd(const PAY_SLT_EXP_SET_CAN_SPEED_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_SET_I2C_ADDR_Cmd(const PAY_SLT_EXP_SET_I2C_ADDR_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_SET_I2C_SPEED_Cmd(const PAY_SLT_EXP_SET_I2C_SPEED_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_SET_WDT_VAL_Cmd(const PAY_SLT_EXP_SET_WDT_VAL_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_SET_CSP_RTABLE_Cmd(const PAY_SLT_EXP_SET_CSP_RTABLE_Cmd_t *Msg);
CFE_Status_t PAY_SLT_EXP_I2C_READ_CHUNK_Cmd(const PAY_SLT_EXP_I2C_READ_CHUNK_Cmd_t *Msg);

#endif
