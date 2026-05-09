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
 *   Specification for the UTRX command function codes
 *
 * @note
 *   This file should be strictly limited to the command/function code (CC)
 *   macro definitions.  Other definitions such as enums, typedefs, or other
 *   macros should be placed in the msgdefs.h or msg.h files.
 */
#ifndef SLT_IFB_FCNCODES_H
#define SLT_IFB_FCNCODES_H

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/*
** SLT_IFB App command codes
*/
#define PAY_SLT_NOOP_CC                 0
#define PAY_SLT_RESET_COUNTERS_CC       1
#define PAY_SLT_REPORT_BCN_CC           2

#define PAY_SLT_IFB_CSP_CMP_CC          10
#define PAY_SLT_IFB_CSP_PING_CC         11
#define PAY_SLT_IFB_CSP_PS_CC           12
#define PAY_SLT_IFB_CSP_MEM_FREE_CC     13
#define PAY_SLT_IFB_CSP_REBOOT_CC       14
#define PAY_SLT_IFB_CSP_BUF_FREE_CC     15
#define PAY_SLT_IFB_CSP_UPTIME_CC       16
#define PAY_SLT_IFB_CSP_GNDWDT_CC       17
#define PAY_SLT_IFB_GET_BRD_UID_CC      18
#define PAY_SLT_IFB_GET_BRD_REV_CC      19
#define PAY_SLT_IFB_GET_CSP_ADDR_CC     20
#define PAY_SLT_IFB_GET_CAN_SPEED_CC    21
#define PAY_SLT_IFB_GET_I2C_ADDR_CC     22
#define PAY_SLT_IFB_GET_I2C_SPEED_CC    23
#define PAY_SLT_IFB_GET_WDT_VAL_CC      24
#define PAY_SLT_IFB_GET_CSP_RTABLE_CC   25
#define PAY_SLT_IFB_GET_SYS_STATUS_CC   26
#define PAY_SLT_IFB_GET_SYS_UPTIME_CC   27
#define PAY_SLT_IFB_GET_BOOT_CNT_CC     28
#define PAY_SLT_IFB_GET_BOOT_CAUSE_CC   29
#define PAY_SLT_IFB_GET_REBOOT_CAUSE_CC 30
#define PAY_SLT_IFB_GET_WDT_LEFT_CC     31
#define PAY_SLT_IFB_GET_BRD_TEMP_CC     32
#define PAY_SLT_IFB_GET_PWR_CURRENT_CC  33
#define PAY_SLT_IFB_GET_IMU_DATA_CC     34
#define PAY_SLT_IFB_GET_NTC_DATA_CC     35
#define PAY_SLT_IFB_SAVE_TABLE0_CC      36
#define PAY_SLT_IFB_SAVE_TABLE1_CC      37
#define PAY_SLT_IFB_SAVE_TABLE4_CC      38
#define PAY_SLT_IFB_SAVE_ALL_TABLE_CC   39
#define PAY_SLT_IFB_SET_CSP_ADDR_CC     40
#define PAY_SLT_IFB_SET_CAN_SPEED_CC    41
#define PAY_SLT_IFB_SET_I2C_ADDR_CC     42
#define PAY_SLT_IFB_SET_I2C_SPEED_CC    43
#define PAY_SLT_IFB_SET_WDT_VAL_CC      44
#define PAY_SLT_IFB_SET_CSP_RTABLE_CC   45

#define PAY_SLT_EXP_CSP_CMP_CC          50
#define PAY_SLT_EXP_CSP_PING_CC         51
#define PAY_SLT_EXP_CSP_PS_CC           52
#define PAY_SLT_EXP_CSP_MEM_FREE_CC     53
#define PAY_SLT_EXP_CSP_REBOOT_CC       54
#define PAY_SLT_EXP_CSP_BUF_FREE_CC     55
#define PAY_SLT_EXP_CSP_UPTIME_CC       56
#define PAY_SLT_EXP_CSP_GNDWDT_CC       57
#define PAY_SLT_EXP_GET_BRD_UID_CC      58
#define PAY_SLT_EXP_GET_BRD_REV_CC      59
#define PAY_SLT_EXP_GET_CSP_ADDR_CC     60
#define PAY_SLT_EXP_GET_CAN_SPEED_CC    61
#define PAY_SLT_EXP_GET_I2C_ADDR_CC     62
#define PAY_SLT_EXP_GET_I2C_SPEED_CC    63
#define PAY_SLT_EXP_GET_WDT_VAL_CC      64
#define PAY_SLT_EXP_GET_CSP_RTABLE_CC   65
#define PAY_SLT_EXP_GET_SYS_STATUS_CC   66
#define PAY_SLT_EXP_GET_SYS_UPTIME_CC   67
#define PAY_SLT_EXP_GET_BOOT_CNT_CC     68
#define PAY_SLT_EXP_GET_BOOT_CAUSE_CC   69
#define PAY_SLT_EXP_GET_REBOOT_CAUSE_CC 70
#define PAY_SLT_EXP_GET_WDT_LEFT_CC     71
#define PAY_SLT_EXP_GET_BRD_TEMP_CC     72
#define PAY_SLT_EXP_GET_PWR_CURRENT_CC  73
#define PAY_SLT_EXP_GET_IMU_DATA_CC     74
#define PAY_SLT_EXP_GET_NTC_DATA_CC     75
#define PAY_SLT_EXP_SAVE_TABLE0_CC      76
#define PAY_SLT_EXP_SAVE_TABLE1_CC      77
#define PAY_SLT_EXP_SAVE_TABLE4_CC      78
#define PAY_SLT_EXP_SAVE_ALL_TABLE_CC   79
#define PAY_SLT_EXP_SET_CSP_ADDR_CC     80
#define PAY_SLT_EXP_SET_CAN_SPEED_CC    81
#define PAY_SLT_EXP_SET_I2C_ADDR_CC     82
#define PAY_SLT_EXP_SET_I2C_SPEED_CC    83
#define PAY_SLT_EXP_SET_WDT_VAL_CC      84
#define PAY_SLT_EXP_SET_CSP_RTABLE_CC   85
#define PAY_SLT_EXP_I2C_READ_CHUNK_CC   86

#define SLT_IFB_NOOP_CC                 PAY_SLT_NOOP_CC
#define SLT_IFB_RESET_COUNTERS_CC       PAY_SLT_RESET_COUNTERS_CC


#endif 
