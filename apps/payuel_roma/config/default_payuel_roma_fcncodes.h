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
 *   Specification for the PAYUEL_ROMA command function codes
 *
 * @note
 *   This file should be strictly limited to the command/function code (CC)
 *   macro definitions.  Other definitions such as enums, typedefs, or other
 *   macros should be placed in the msgdefs.h or msg.h files.
 */
#ifndef PAYUEL_ROMA_FCNCODES_H
#define PAYUEL_ROMA_FCNCODES_H

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/*
** Uelysys Payload (ROMA) command codes
*/
#define PAYUEL_ROMA_NOOP_CC           0
#define PAYUEL_ROMA_RESET_COUNTERS_CC 1
#define PAYUEL_ROMA_COMM_TEST_CC      2
#define PAYUEL_ROMA_CLOCK_SYNC_CC     3
#define PAYUEL_ROMA_LOG_TEST_CC       4
#define PAYUEL_ROMA_TRANS_TEST_CC     5
#define PAYUEL_ROMA_GET_SPECIFIC_LINE_CC     6
#define PAYUEL_ROMA_GET_MULTIPLE_LINES_CC    7
#define PAYUEL_ROMA_GET_LATEST_LINE_CC       8
#define PAYUEL_ROMA_GET_LATEST_N_LINES_CC    9
#define PAYUEL_ROMA_CLEAR_ALL_LINES_CC      10
#define PAYUEL_ROMA_GET_SINGLE_ENTRY_CC     11
#define PAYUEL_ROMA_GET_MULTIPLE_ENTRIES_CC 12
#define PAYUEL_ROMA_ADD_ENTRY_CC            13
#define PAYUEL_ROMA_REMOVE_ENTRY_CC         14
#define PAYUEL_ROMA_GET_USED_SLOTS_CC       15
#define PAYUEL_ROMA_SET_ROUTE_DEFAULT_CC    16
#define PAYUEL_ROMA_RESET_ROUTE_CC          17
#define PAYUEL_ROMA_LOAD_ROUTE_CC           18
#define PAYUEL_ROMA_SAVE_ROUTE_CC           19
#define PAYUEL_ROMA_SEND_ROUTE_CC           20
#define PAYUEL_ROMA_SET_ROUTE_CC            21
#define PAYUEL_ROMA_PAR_GET_CC              22
#define PAYUEL_ROMA_PAR_SET_CC              23
#define PAYUEL_ROMA_PAR_DEFAULTS_CC         24
#define PAYUEL_ROMA_PAR_SAVE_CC             25
#define PAYUEL_ROMA_PAR_RESTORE_CC          26
#define PAYUEL_ROMA_PAR_LOAD_CC             27
#define PAYUEL_ROMA_PAR_SET_OOB_CC          28
#define PAYUEL_ROMA_SEND_COMMAND_CC         29
#define PAYUEL_ROMA_SEND_MSG_CC             30
#define PAYUEL_ROMA_SYNC_RX_CC              31
#define PAYUEL_ROMA_SYNC_TX_CC              32
#define PAYUEL_ROMA_PAY_INIT_CC             33


#endif
