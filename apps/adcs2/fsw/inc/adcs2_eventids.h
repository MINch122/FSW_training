/************************************************************************
**
**      GSC-18128-1, "Core Flight Executive Version 6.6"
**
**      Copyright (c) 2006-2019 United States Government as represented by
**      the Administrator of the National Aeronautics and Space Administration.
**      All Rights Reserved.
**
**      Licensed under the Apache License, Version 2.0 (the "License");
**      you may not use this file except in compliance with the License.
**      You may obtain a copy of the License at
**
**        http://www.apache.org/licenses/LICENSE-2.0
**
**      Unless required by applicable law or agreed to in writing, software
**      distributed under the License is distributed on an "AS IS" BASIS,
**      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**      See the License for the specific language governing permissions and
**      limitations under the License.
**
** File:
**    adcs_app_events.h 
**
** Purpose: 
**  Define ADCS App Events IDs
**
** Notes:
**
**
*************************************************************************/
#ifndef _ADCS2_EVENTS_H_
#define _ADCS2_EVENTS_H_


#define ADCS2_RESERVED_EID      0
#define ADCS2_INIT_INF_EID      1
#define ADCS2_CC_ERR_EID        2
#define ADCS2_NOOP_INF_EID      3
#define ADCS2_RESET_INF_EID     4
#define ADCS2_MID_ERR_EID       5
#define ADCS2_CMD_LEN_ERR_EID   6
#define ADCS2_PIPE_ERR_EID      7
#define ADCS2_VALUE_INF_EID     8
#define ADCS2_CR_PIPE_ERR_EID   9
#define ADCS2_SUB_HK_ERR_EID    10
#define ADCS2_SUB_CMD_ERR_EID   11
#define ADCS2_TABLE_REG_ERR_EID 12

#define ADCS2_STARTUP_INF_EID       14
#define ADCS2_COMMAND_ERR_EID       15
#define ADCS2_COMMANDNOP_INF_EID    16
#define ADCS2_COMMANDRST_INF_EID    17
#define ADCS2_CMD_ERR_EID            18
#define ADCS2_INVALID_MSGID_ERR_EID  19
#define ADCS2_LEN_ERR_EID            20
#define ADCS2_I2C_INIT_ERR_EID       22
#define ADCS2_BEACON_DATA_REQUEST_ERR_EID    23
#define ADCS2_HOUSEKEEPING_DATA_REQUEST_ERR_EID 24
#define ADCS2_ATTITUDE_ORBIT_DATA_REQUEST_ERR_EID 25
#define ADCS2_ENABLE_OBC_ENABLE_PIN_ERR_EID 26
#define ADCS2_DISABLE_OBC_ENABLE_PIN_ERR_EID 27
#define ADCS2_ENABLE_OBC_BOOT_PIN_ERR_EID 28
#define ADCS2_DISABLE_OBC_BOOT_PIN_ERR_EID 29

#define ADCS2_BOOTUP_CHECK_ERR_EID          114

#endif /* _adcs_app_events_h_ */