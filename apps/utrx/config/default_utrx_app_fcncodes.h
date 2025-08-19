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
 *   Specification for the UTRX_APP command function codes
 *
 * @note
 *   This file should be strictly limited to the command/function code (CC)
 *   macro definitions.  Other definitions such as enums, typedefs, or other
 *   macros should be placed in the msgdefs.h or msg.h files.
 */
#ifndef UTRX_APP_FCNCODES_H
#define UTRX_APP_FCNCODES_H

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/*
** Utrx App command codes
*/

#define UTRX_APP_NOOP_CC                  0  //set
#define UTRX_APP_RESET_COUNTERS_CC        1  //set
#define UTRX_APP_RESET_APP_CMD_COUNTERS_CC    2  //set
#define UTRX_APP_RESET_DEVICE_CMD_COUNTERS_CC 3  //set
#define UTRX_GNDWDT_CLEAR_CC              6 //set
#define UTRX_REBOOT_CC                    7 //set
#define UTRX_RXCONF_SET_BAUD_CC           8 //set
#define UTRX_TXCONF_SET_BAUD_CC           9 //set
#define UTRX_SET_DEFAULT_BAUD_CC          10 //set
#define UTRX_RPARAM_SAVE_1_CC             11 //set
#define UTRX_RPARAM_SAVE_5_CC             12 //set
#define UTRX_RPARAM_SAVE_ALL_CC           13 //set
#define UTRX_RXCONF_SET_FREQ_CC           14 //set
#define UTRX_TXCONF_SET_FREQ_CC           15 //set
#define UTRX_CHECK_STATE_PING_CC          32 //set


#define UTRX_RXCONF_GET_BAUD_CC           16 //get
#define UTRX_RXCONF_GET_GUARD_CC          17 //get
#define UTRX_TXCONF_GET_BAUD_CC           18 //get
#define UTRX_TLM_GET_TEMP_BRD_CC          19 //get
#define UTRX_TLM_GET_LAST_RSSI_CC         20 //get
#define UTRX_TLM_GET_LAST_RFERR_CC        21 //get
#define UTRX_TLM_GET_ACTIVE_CONF_CC       22 //get
#define UTRX_TLM_GET_BOOT_COUNT_CC        23 //get
#define UTRX_TLM_GET_BOOT_CAUSE_CC        24 //get
#define UTRX_TLM_GET_LAST_CONTACT_CC      25 //get
#define UTRX_TLM_GET_TOT_TX_BYTES_CC      26 //get
#define UTRX_TLM_GET_TOT_RX_BYTES_CC      27 //get
#define UTRX_GET_STATUS_CONFIGURATION_CC  28 //get // 사용 보류중 //
#define UTRX_RXCONF_GET_FREQ_CC           29 //get
#define UTRX_TXCONF_GET_FREQ_CC           30 //get

#endif
