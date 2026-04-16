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
 *   Specification for the EPS command function codes
 *
 * @note
 *   This file should be strictly limited to the command/function code (CC)
 *   macro definitions.  Other definitions such as enums, typedefs, or other
 *   macros should be placed in the msgdefs.h or msg.h files.
 */
#ifndef DEFAULT_EPS_FCNCODES_H
#define DEFAULT_EPS_FCNCODES_H

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/*
** EPS App command codes
*/
/**
 * EPS App Command Codes
 */
typedef enum
{
    /* Basic Commands */
    EPS_NOOP_CC              = 0,
    EPS_RESET_COUNTERS_CC    = 1,

    /* Utility helpers are not dispatched command codes. */
    /* EPS_REPORT_APPDATA_CC    = 2, */ /* EPS_ReportAppDataCmd lives in eps_utils.c */

    /* Power Interface Commands */
    EPS_P80_POWER_IF_GET_CC      = 10,
    EPS_P80_POWER_IF_SET_CC      = 11,
    EPS_P80_POWER_IF_LIST_CC     = 12,

    /* Housekeeping Commands */
    EPS_GET_HK_CC                = 20,
    EPS_GET_HK_ALL_CC            = 21,

    /* Watchdog Commands */
    EPS_P80_GND_WDT_CLEAR_CC     = 30,
    EPS_P80_GND_WDT_CLEAR_ALL_CC = 31,

    /* Remote Parameter Commands */
    EPS_RPARAM_GET_CC            = 40,
    EPS_RPARAM_SET_CC            = 41,
    EPS_RPARAM_GET_FULL_TABLE_CC = 42,

    /* Table Save/Load Commands */
    EPS_RPARAM_SAVE_ALL_CC       = 50,
    EPS_RPARAM_TABLE_SAVE_CC     = 51,
    EPS_RPARAM_TABLE_LOAD_CC     = 52,
    EPS_RPARAM_SAVE_TO_STORE_CC  = 53,
    EPS_RPARAM_LOAD_FROM_STORE_CC = 54,

    /* Beacon Report Command */
    EPS_REPORT_BCN_CC            = 70,

    /* CSP Standard Service Commands */
    EPS_CSP_PING_CC              = 80,
    EPS_CSP_REBOOT_CC            = 81,
    EPS_CSP_PS_CC                = 82,
    EPS_CSP_MEMFREE_CC           = 83,
    EPS_CSP_BUF_FREE_CC          = 84,
    EPS_CSP_UPTIME_CC            = 85,

} EPS_CommandCode_t;



#endif
