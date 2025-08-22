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
#define ADCS_GPIO_ENALBE_LOW_CC 6
#define ADCS_GPIO_BOOT_HIGH_CC 8
#define ADCS_GPIO_BOOT_LOW_CC 9
#define ADCS_EXIT_BOOTLOADER_CC 10

/* < ADCS Module Command Code (TC, TM) > */
#define ADCS_SET_RESET_CC                   11
#define ADCS_SET_CURRENT_UNIX_TIME_CC       12
#define ADCS_SET_CONTROL_ESTIMATION_MODE_CC 13
#define ADCS_SET_REFERENCE_LLH_TARGET_CC    14
#define ADCS_SET_ORBIT_MODE_CC              15
#define ADCS_SET_REFERENCE_RPY_VALUES_CC    16
#define ADCS_SET_SAT_ORBIT_PARAMS_CONFIG_CC 17

#define ADCS_GET_CURRENT_UNIX_TIME_CC       20
#define ADCS_GET_CONTROL_ESTIMATION_MODE_CC 21
#define ADCS_GET_REFERENCE_LLH_TARGET_CC    22
#define ADCS_GET_ORBIT_MODE_CC              23
#define ADCS_GET_RAW_CUBESENSE_SUN_CC       24
#define ADCS_GET_POWER_STATE_CC             25
#define ADCS_GET_SAT_ORBIT_PARAM_CONFIG_CC  26
#define ADCS_GET_RAW_CSS_SENSOR_CC          27
#define ADCS_GET_RAW_GYR_SENSOR_CC          28
#define ADCS_GET_CALIBRATED_GYR_SENSOR_CC   29

#endif