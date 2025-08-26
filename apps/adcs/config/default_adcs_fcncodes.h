/************************************************************************
 * NASA Docket No. GSC-18719-1 and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * @file
 *   Specification for the ADCS command function codes
 *
 * @note
 *   This file should be strictly limited to the command/function code (CC)
 *   macro definitions.  Other definitions such as enums typedefs or other
 *   macros should be placed in the msgdefs.h or msg.h files.
 */
#ifndef ADCS_FCNCODES_H
#define ADCS_FCNCODES_H

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/*
** Adcs App command codes
*/
#define ADCS_NOOP_CC           0
#define ADCS_RESET_COUNTERS_CC 1

#define ADCS_RESET_APP_CMD_COUNTERS_CC          2
#define ADCS_RESET_DEVICE_CMD_COUNTERS_CC       3
#define ADCS_SET_COMMUNICATION_MODE_AS_CAN_CC   4
#define ADCS_GPIO_ENABLE_HIGH_CC 5
#define ADCS_GPIO_ENABLE_LOW_CC 6
#define ADCS_GPIO_BOOT_HIGH_CC 8
#define ADCS_GPIO_BOOT_LOW_CC 9
#define ADCS_EXIT_BOOTLOADER_CC 10

/* < ADCS Module Command Code (TC, TM) > */
/* Telecommand */
#define ADCS_SET_RESET_CC                       11 // 1
#define ADCS_SET_CURRENT_UNIX_TIME_CC           12 // 2
#define ADCS_SET_CONTROL_ESTIMATION_MODE_CC     13 // 42
#define ADCS_SET_REFERENCE_LLH_TARGET_CC        14 // 48
#define ADCS_SET_ORBIT_MODE_CC                  15 // 51
#define ADCS_SET_REFERENCE_RPY_VALUES_CC        16 // 54
#define ADCS_SET_SAT_ORBIT_PARAMS_CONFIG_CC     17 // 68

#define ADCS_SET_PERSIST_CONFIG_CC              18 // 7
#define ADCS_SET_POWER_STATE_CC                 19 // 56
#define ADCS_SET_RUN_MODE_CC                    20 // 57
#define ADCS_SET_SATELLITE_CONFIG_CC            21 // 61
#define ADCS_SET_CONTROLLER_CONFIG_CC           22 // 62
#define ADCS_SET_DEFAULT_MODE_CONFIG_CC         23 // 64
#define ADCS_SET_MOUNTING_CONFIG_CC             24 // 65
#define ADCS_SET_UNSOLICIT_EVENT_MSG_SETUP_CC   25 // 116

/* Telemetry */
#define ADCS_GET_CURRENT_UNIX_TIME_CC           30 // 133
#define ADCS_GET_CONTROL_ESTIMATION_MODE_CC     31 // 150
#define ADCS_GET_REFERENCE_LLH_TARGET_CC        32 // 157
#define ADCS_GET_ORBIT_MODE_CC                  33 // 162
#define ADCS_GET_RAW_CUBESENSE_SUN_CC           34 // 170
#define ADCS_GET_POWER_STATE_CC                 35 // 183
#define ADCS_GET_SAT_ORBIT_PARAM_CONFIG_CC      36 // 196
#define ADCS_GET_RAW_CSS_SENSOR_CC              37 // 203
#define ADCS_GET_RAW_GYR_SENSOR_CC              38 // 204
#define ADCS_GET_CALIBRATED_GYR_SENSOR_CC       39 // 207

#define ADCS_GET_PERSIST_CONFIG_DIAGNOSTIC_CC   40 // 134
#define ADCS_GET_COMMUNICATION_STATUS_CC        41 // 135
#define ADCS_GET_RUN_MODE_CC                    42 // 184
#define ADCS_GET_SATELLITE_CONFIG_CC            43 // 189
#define ADCS_GET_CONTROLLER_CONFIG_CC           44 // 190
#define ADCS_GET_DEFAULT_MODE_CONFIG_CC         45 // 192
#define ADCS_GET_MOUNTING_CONFIG_CC             46 // 193
#define ADCS_GET_OPERATIONAL_STATE              47 // 200
#define ADCS_GET_UNSOLICIT_EVENT_MSG_SETUP_CC   48 // 233

#endif