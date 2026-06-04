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
#define SLT_IFB_NOOP_CC                 0                   
#define SLT_IFB_RESET_COUNTERS_CC       1     

//IFB transaction
#define SLT_IFB_CSP_CMP_CC              2 
#define SLT_IFB_CSP_PING_CC             3 
#define SLT_IFB_CSP_PS_CC               4 
#define SLT_IFB_CSP_MEM_FREE_CC         5 
#define SLT_IFB_CSP_REBOOT_CC           6 
#define SLT_IFB_CSP_BUF_FREE_CC         7 
#define SLT_IFB_CSP_UPTIME_CC           8 

//IFB WatchDog Reset transaction
#define SLT_IFB_CSP_GNDWDT_CC           9 

//IFB Table 0 : Board Parameters Get
#define SLT_IFB_GET_BRD_UID_CC          10
#define SLT_IFB_GET_BRD_REV_CC          11
#define SLT_IFB_GET_CSP_ADDR_CC         12
#define SLT_IFB_GET_CAN_SPEED_CC        13
#define SLT_IFB_GET_I2C_ADDR_CC         14
#define SLT_IFB_GET_I2C_SPEED_CC        15
#define SLT_IFB_GET_WDT_VAL_CC          16
#define SLT_IFB_GET_CSP_RTABLE_CC       17

//IFB Table 4 : Telemetry Get
#define SLT_IFB_GET_SYS_STATUS_CC       18
#define SLT_IFB_GET_SYS_UPTIME_CC       19
#define SLT_IFB_GET_BOOT_CNT_CC         20
#define SLT_IFB_GET_BOOT_CAUSE_CC       21
#define SLT_IFB_GET_REBOOT_CAUSE_CC     22
#define SLT_IFB_GET_WDT_LEFT_CC         23
#define SLT_IFB_GET_BRD_TEMP_CC         24
#define SLT_IFB_GET_PWR_CURRENT_CC      25
#define SLT_IFB_GET_IMU_DATA_CC         26
#define SLT_IFB_GET_NTC_DATA_CC         27

//Table Save
#define SLT_IFB_SAVE_TABLE0_CC          28
#define SLT_IFB_SAVE_TABLE1_CC          29
#define SLT_IFB_SAVE_TABLE4_CC          30
#define SLT_IFB_SAVE_ALL_TABLE_CC       31


#endif 