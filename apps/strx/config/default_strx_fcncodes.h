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
 *   Specification for the STRX command function codes
 *
 * @note
 *   This file should be strictly limited to the command/function code (CC)
 *   macro definitions.  Other definitions such as enums, typedefs, or other
 *   macros should be placed in the msgdefs.h or msg.h files.
 */
#ifndef STRX_FCNCODES_H
#define STRX_FCNCODES_H

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/*
** Strx App command codes
*/

#define STRX_NOOP_CC                  0  //set
#define STRX_RESET_COUNTERS_CC        1  //set
#define STRX_RESET_APP_CMD_COUNTERS_CC    2  //set
#define STRX_RESET_DEVICE_CMD_COUNTERS_CC 3  //set
#define STRX_GNDWDT_CLEAR_CC              6 //set
#define STRX_REBOOT_CC                    7 //set
#define STRX_RXCONF_SET_BAUD_CC           8 //set
#define STRX_TXCONF_SET_BAUD_CC           9 //set
#define STRX_SET_DEFAULT_BAUD_CC          10 //set
#define STRX_RPARAM_SAVE_1_CC             11 //set
#define STRX_RPARAM_SAVE_5_CC             12 //set
#define STRX_RPARAM_SAVE_ALL_CC           13 //set
#define STRX_RXCONF_SET_FREQ_CC           14 //set
#define STRX_TXCONF_SET_FREQ_CC           15 //set

#define STRX_RXCONF_GET_BAUD_CC           16 //get
#define STRX_RXCONF_GET_GUARD_CC          17 //get
#define STRX_TXCONF_GET_BAUD_CC           18 //get
#define STRX_TLM_GET_TEMP_BRD_CC          19 //get
#define STRX_TLM_GET_LAST_RSSI_CC         20 //get
#define STRX_TLM_GET_LAST_RFERR_CC        21 //get
// #define STRX_TLM_GET_ACTIVE_CONF_CC       22 //get
#define STRX_TLM_GET_BOOT_COUNT_CC        23 //get
#define STRX_TLM_GET_BOOT_CAUSE_CC        24 //get
#define STRX_TLM_GET_LAST_CONTACT_CC      25 //get
#define STRX_TLM_GET_TOT_TX_BYTES_CC      26 //get
#define STRX_TLM_GET_TOT_RX_BYTES_CC      27 //get
// #define STRX_GET_STATUS_CONFIGURATION_CC  28 //get
#define STRX_RXCONF_GET_FREQ_CC           29 //get
#define STRX_TXCONF_GET_FREQ_CC           30 //get
#define STRX_TLM_RXMODE_CC                32 //get
#define STRX_TLM_GET_GNDWDT_CNT_CC        33 //get
#define STRX_TLM_GET_GNDWDT_LEFT_CC       34 //get
#define STRX_TLM_GET_KISS_USART_CC        35 //get
#define STRX_TLM_SET_KISS_USART_CC        36 //get
#define STRX_TLM_GET_GOSH_USART_CC        37 //get
#define STRX_TLM_SET_GOSH_USART_CC        38 //get

#define STRX_RPARAM_SAVE_0_CC             39 //set
#define STRX_RPARAM_SAVE_4_CC             40 //set

#define STRX_CHECK_STATE_PING_CC          41 //set

#endif
