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
#define EPS_NOOP_CC                 0
#define EPS_RESET_COUNTERS_CC       1
#define EPS_GET_COUNTERS_CC         2
#define EPS_GET_APPDATA_CC          3

/*
** P31u device & channel control
*/
#define EPS_P31U_SET_OUT_SINGLE_CC  10
#define EPS_P31U_SET_OUTPUTS_CC     11
#define EPS_P31U_RESET_WDT_CC       20
#define EPS_P31U_RESET_COUNTERS_CC  21
#define EPS_P31U_HARD_RESET_CC      22

/*
** P31u housekeeping requests
*/
#define EPS_P31U_GETHK_ALL_CC       30
#define EPS_P31U_GETHK_OUT_CC       31
#define EPS_P31U_GETHK_VI_CC        32
#define EPS_P31U_GETHK_WDT_CC       33
#define EPS_P31U_GETHK_BASIC_CC     34
#define EPS_P31U_GETHK_OLD_CC       35
#define EPS_P31U_GETHK_CC           36

/*
** P31u config commands
*/
#define EPS_P31U_SET_PV_VOLT_CC     40
#define EPS_P31U_SET_PV_AUTO_CC     41
#define EPS_P31U_SET_HEATER_CC      42

#define EPS_P31U_GET_CONFIG_CC      50
#define EPS_P31U_SET_CONFIG_CC      51
#define EPS_P31U_CONFIG_CC          52
#define EPS_P31U_GET_CONFIG2_CC     53
#define EPS_P31U_SET_CONFIG2_CC     54
#define EPS_P31U_CONFIG2_CC         55
#define EPS_P31U_SET_CONFIG3_CC     56

/*
** Generic transaction (plumbing)
*/
#define EPS_P31U_TRANSACTION_CC     99

#endif
