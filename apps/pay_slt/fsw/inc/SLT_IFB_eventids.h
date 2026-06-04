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
 *
 * Define Sample App Events IDs
 */

#ifndef SLT_IFB_EVENTIDS_H
#define SLT_IFB_EVENTIDS_H

#define SLT_IFB_RESERVED_EID 1
#define SLT_IFB_INIT_INF_EID 2
#define SLT_IFB_CC_ERR_EID 3
#define SLT_IFB_NOOP_INF_EID 4
#define SLT_IFB_RESET_INF_EID 5
#define SLT_IFB_MID_ERR_EID 6
#define SLT_IFB_CMD_LEN_ERR_EID 7
#define SLT_IFB_PIPE_ERR_EID 8
#define SLT_IFB_VALUE_INF_EID 9
#define SLT_IFB_CR_PIPE_ERR_EID 10
#define SLT_IFB_SUB_HK_ERR_EID 11

#define SLT_IFB_CSP_CMP_ERR_EID 12
#define SLT_IFB_CSP_PING_ERR_EID 13
#define SLT_IFB_CSP_PS_ERR_EID 14
#define SLT_IFB_CSP_MEM_FREE_ERR_EID 15
#define SLT_IFB_CSP_REBOOT_ERR_EID 16
#define SLT_IFB_CSP_BUF_FREE_ERR_EID 17
#define SLT_IFB_CSP_UPTIME_ERR_EID 18
#define SLT_IFB_GNDWDT_ERR_EID 19

#define SLT_IFB_GET_BRD_UID_ERR_EID 20
#define SLT_IFB_GET_BRD_REV_ERR_EID 21
#define SLT_IFB_GET_CSP_ADDR_ERR_EID 22
#define SLT_IFB_GET_CAN_SPEED_ERR_EID 23
#define SLT_IFB_GET_I2C_ADDR_ERR_EID 24
#define SLT_IFB_GET_I2C_SPEED_ERR_EID 25
#define SLT_IFB_GET_WDT_VAL_ERR_EID 26
#define SLT_IFB_GET_CSP_RTABLE_ERR_EID 27

#define SLT_IFB_GET_SYS_STATUS_ERR_EID 28 
#define SLT_IFB_GET_SYS_UPTIME_ERR_EID 29
#define SLT_IFB_GET_BOOT_CNT_ERR_EID 30
#define SLT_IFB_GET_BOOT_CAUSE_ERR_EID 31
#define SLT_IFB_GET_REBOOT_CAUSE_ERR_EID 32
#define SLT_IFB_GET_WDT_LEFT_ERR_EID 33
#define SLT_IFB_GET_BRD_TEMP_ERR_EID 34
#define SLT_IFB_GET_PWR_CURRENT_ERR_EID 35
#define SLT_IFB_GET_IMU_DATA_ERR_EID 36
#define SLT_IFB_GET_NTC_DATA_ERR_EID 37

#define SLT_IFB_SAVE_TABLE0_ERR_EID 38
#define SLT_IFB_SAVE_TABLE1_ERR_EID 39
#define SLT_IFB_SAVE_TABLE4_ERR_EID 40
#define SLT_IFB_SAVE_ALL_TABLE_ERR_EID 41

#define SLT_IFB_CMD_SUCCESS_EID 42
#define SLT_IFB_SUB_CMD_ERR_EID   43
#define SLT_IFB_TABLE_REG_ERR_EID 44

#endif