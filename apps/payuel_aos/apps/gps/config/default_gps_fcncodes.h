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
 *   Specification for the GPS command function codes
 *
 * @note
 *   This file should be strictly limited to the command/function code (CC)
 *   macro definitions.  Other definitions such as enums, typedefs, or other
 *   macros should be placed in the msgdefs.h or msg.h files.
 */
#ifndef DEFAULT_GPS_FCNCODES_H
#define DEFAULT_GPS_FCNCODES_H

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/*
** GPS App commands.
*/
#define GPS_NOOP_CC                         0
#define GPS_RESET_COUNTERS_CC               1
#define GPS_GET_COUNTERS_CC                 2
#define GPS_GET_APPDATA_CC                  3
#define GPS_DRIVER_REPORT_HK_CC             4
#define GPS_DRIVER_CLEAR_HK_CC              5

/*
** OEM receiver commands.
*/
#define GPS_OEM_CMD_LOG_CC                  12
#define GPS_OEM_CMD_LOG_ONCE_CC             13
#define GPS_OEM_CMD_LOG_ONTIME_CC           14
#define GPS_OEM_CMD_LOG_ONCHANGED_CC        15
#define GPS_OEM_CMD_LOG_ONNEW_CC            16
#define GPS_OEM_CMD_UNLOG_CC                17
#define GPS_OEM_CMD_UNLOGALL_CC             18
#define GPS_OEM_CMD_ELEVATION_CUTOFF_CC     19
#define GPS_OEM_CMD_INTERFACE_MODE_CC       20
#define GPS_OEM_CMD_SERIAL_CONFIG_CC        21
#define GPS_OEM_CMD_PUBLISH_CC              22

/*
** OEM log handler commands.
*/
#define GPS_OEM_LOG_GET_HANDLER_HK_CC       30
#define GPS_OEM_LOG_GET_MSG_STAT_CC         31
#define GPS_OEM_LOG_SET_HANDLER_STATUS_CC   32
#define GPS_OEM_LOG_GET_HANDLER_STATUS_CC   33
#define GPS_OEM_LOG_HANDLER_ACTIVATE_CC     34
#define GPS_OEM_LOG_HANDLER_DEACTIVATE_CC   35
#define GPS_OEM_LOG_HANDLER_GO_DORMANT_CC   36
#define GPS_OEM_LOG_HANDLER_WAKEUP_CC       37
#define GPS_OEM_LOG_HANDLER_ACTIVATEALL_CC  38
#define GPS_OEM_LOG_HANDLER_DEACTIVATEALL_CC    39

#define GPS_OEM_LOG_HANDLER_REGISTER_CC     50
#define GPS_OEM_LOG_HANDLER_UNREGISTER_CC   51
#define GPS_OEM_LOG_ADD_CALLBACK_CC         52
#define GPS_OEM_LOG_CLEAR_CALLBACK_CC       53
#define GPS_OEM_LOG_HANDLER_SET_BROKEN_CC   54
#define GPS_OEM_LOG_GET_MESSAGE_LENGTH_CC   55
#define GPS_OEM_LOG_GET_HANDLER_NAME_CC     56
#define GPS_OEM_LOG_RESET_HANDLER_COUNTERS_CC   57
#define GPS_OEM_LOG_DUMP_RECENT_MESSAGE_CC  58
#define GPS_OEM_LOG_IGNORE_CHECKSUM_CC      59
#define GPS_OEM_LOG_DONOT_IGNORE_CHECKSUM_CC    60

#define GPS_OEM_LOG_LOCK_HANDLERS_CC        100
#define GPS_OEM_LOG_UNLOCK_HANDLERS_CC      101

#endif
