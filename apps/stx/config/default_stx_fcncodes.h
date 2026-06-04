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
 *   Specification for the STX command function codes
 *
 * @note
 *   This file should be strictly limited to the command/function code (CC)
 *   macro definitions.  Other definitions such as enums, typedefs, or other
 *   macros should be placed in the msgdefs.h or msg.h files.
 */
#ifndef STX_FCNCODES_H
#define STX_FCNCODES_H

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/*
** Sample App command codes
*/
#define STX_NOOP_CC           0
#define STX_RESET_COUNTERS_CC 1
#define STX_PARAM_INIT_CC     2
#define STX_MODULE_ID_INIT_CC 3

/**set**/
#define STX_SET_SYMBOLRATE      10
#define STX_SET_TRANSMITPW      11
#define STX_SET_CENTERFREQ      12
#define STX_SET_MODCOD          13
#define STX_SET_ROLLOFF         14
#define STX_SET_PILOTSIG        15
#define STX_SET_FECFRAMESZ      16
#define STX_SET_PRETX_DELAY               17
#define STX_SET_ALL_PRAMETERS            18
#define STX_SET_RS485BAUD                19
#define STX_SET_MODULATOR_DATA_INTERFACE 20

/**get**/
#define STX_GET_SYMBOL_RATE     30
#define STX_GET_TX_POWER        31
#define STX_GET_CENTER_FREQ     32
#define STX_GET_MODCOD          33
#define STX_GET_ROLL_OFF        34
#define STX_GET_PILOT_SIGNAL    35
#define STX_GET_FEC_FRAME_SIZE  36
#define STX_GET_PRETX_DELAY     37
#define STX_GET_ALL_PRAMETERS           38
#define STX_GET_REPORT                  39
#define STX_GET_MODULATOR_DATA_INTERFACE 40

#define STX_FILESYS_CC_DIR               50
#define STX_FILESYS_CC_DIRNEXT           51
#define STX_FILESYS_CC_DELFILE           52
#define STX_FILESYS_CC_DELALLFILE        53
#define STX_FILESYS_CC_CREATEFILE        54
#define STX_FILESYS_CC_WRITEFILE         55
#define STX_FILESYS_CC_OPENFILE          56
#define STX_FILESYS_CC_READFILE          57
#define STX_FILESYS_CC_SENDFILE          58
#define STX_FILESYS_CC_SENDFILERTI       59

#define STX_SYSCONF_CC_TRANSMITMODE      62
#define STX_SYSCONF_CC_IDLEMODE          63
#define STX_SYSCONF_CC_UPDATEFW          64
#define STX_SYSCONF_CC_SAFESHUTDOWN      65

#define STX_GETRES_CC_GETRES             66

#define STX_HK_CC             67
#define STX_BCN_CC             68


#endif
