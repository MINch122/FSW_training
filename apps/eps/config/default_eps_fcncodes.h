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
    EPS_REPORT_APPDATA_CC    = 2,

    /* Power Interface Commands */
    EPS_POWER_IF_GET_CC      = 10,
    EPS_POWER_IF_SET_CC      = 11,
    EPS_POWER_IF_LIST_CC     = 12,

    /* Housekeeping Commands */
    EPS_GET_HK_CC            = 20,

    /* Watchdog Commands */
    EPS_GND_WDT_CLEAR_CC     = 30,

    /* Remote Parameter Commands */
    EPS_PARAM_GET_CC         = 40,
    EPS_PARAM_SET_CC         = 41,
    EPS_GET_FULL_TABLE_CC    = 42,

    /* Table Save/Load Commands */
    EPS_PARAM_SAVE_CC        = 50,
    EPS_TABLE_SAVE_CC        = 51,
    EPS_TABLE_LOAD_CC        = 52,

} EPS_CommandCode_t;



#endif
