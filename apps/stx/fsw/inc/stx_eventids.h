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


typedef enum{
STK_rpt_tpye_REULT,
STK_rpt_tpye_CFE_ERROR,
STK_rpt_tpye_ACK_ERROR
}STK_rpt_type_t;


#define DEVICE_SUCCESS 0


#ifndef STX_EVENTS_H
#define STX_EVENTS_H

#define STX_RESERVED_EID      0
#define STX_INIT_INF_EID      1
#define STX_CC_ERR_EID        2
#define STX_NOOP_INF_EID      3
#define STX_RESET_INF_EID     4
#define STX_MID_ERR_EID       5
#define STX_CMD_LEN_ERR_EID   6
#define STX_PIPE_ERR_EID      7
#define STX_VALUE_INF_EID     8
#define STX_CR_PIPE_ERR_EID   9
#define STX_SUB_HK_ERR_EID    10
#define STX_SUB_CMD_ERR_EID   11
#define STX_TABLE_REG_ERR_EID 12

#define STX_SUB_TLM_ERR_EID    13
#define STX_SUB_ACK_ERR_EID    14

// STX Status 오류
#define STX_COMMAND_STATUS_ERR_EID  15
#define STX_SUB_BCN_ERR_EID 16

/* SET (starting at 20) */
#define STX_SET_SYMBOLRATE_ERR_EID                       20
#define STX_SET_TRANSMITPW_ERR_EID                       21
#define STX_SET_CENTERFREQ_ERR_EID                       22
#define STX_SET_MODCOD_ERR_EID                           23
#define STX_SET_ROLLOFF_ERR_EID                          24
#define STX_SET_PILOTSIG_ERR_EID                         25
#define STX_SET_FECFRAMESZ_ERR_EID                       26
#define STX_SET_PRETX_DELAY_ERR_EID                      27
#define STX_SET_ALL_PRAMETERS_ERR_EID                    28
#define STX_SET_RS485BAUD_ERR_EID                        29
#define STX_SET_MODULATOR_DATA_INTERFACE_ERR_EID         30

/* GET */
#define STX_GET_SYMBOL_RATE_ERR_EID                      31
#define STX_GET_TX_POWER_ERR_EID                         32
#define STX_GET_CENTER_FREQ_ERR_EID                      33
#define STX_GET_MODCOD_ERR_EID                           34
#define STX_GET_ROLL_OFF_ERR_EID                         35
#define STX_GET_PILOT_SIGNAL_ERR_EID                     36
#define STX_GET_FEC_FRAME_SIZE_ERR_EID                   37
#define STX_GET_PRETX_DELAY_ERR_EID                      38
#define STX_GET_ALL_PRAMETERS_ERR_EID                    39
#define STX_GET_REPORT_ERR_EID                           40
#define STX_GET_MODULATOR_DATA_INTERFACE_ERR_EID         41

/* FILESYS */
#define STX_FILESYS_DIR_ERR_EID                          42
#define STX_FILESYS_DIRNEXT_ERR_EID                      43
#define STX_FILESYS_DELFILE_ERR_EID                      44
#define STX_FILESYS_DELALLFILE_ERR_EID                   45
#define STX_FILESYS_CREATEFILE_ERR_EID                   46
#define STX_FILESYS_WRITEFILE_ERR_EID                    47
#define STX_FILESYS_OPENFILE_ERR_EID                     48
#define STX_FILESYS_READFILE_ERR_EID                     49
#define STX_FILESYS_SENDFILE_ERR_EID                     50

/* SYSCONF */
#define STX_SYSCONF_TRANSMITMODE_ERR_EID                 51
#define STX_SYSCONF_IDLEMODE_ERR_EID                     52
#define STX_SYSCONF_UPDATEFW_ERR_EID                     53
#define STX_SYSCONF_SAFESHUTDOWN_ERR_EID                 54


#define STX_rpt_type_RESULT                              56
#define STX_rpt_type_CFE_ERROR                           57
#define STX_rpt_type_ACK_ERROR                           58
#define STX_rpt_type_FINAL_ACK_ERROR                     59

#endif /* STX_EVENTS_H */
