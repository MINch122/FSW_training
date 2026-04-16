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
#ifndef ADCS2_FCNCODES_H
#define ADCS2_FCNCODES_H

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/*
** Adcs App command codes
*/
#define ADCS2_NOOP_CC		  	0
#define ADCS2_RESET_COUNTERS_CC 1

/* < ADCS Module Command Code (TC, TM) > */
/* Telecommand */
#define ADCS2_SET_RESET_CC						11 // 1
#define ADCS2_SET_CURRENT_UNIX_TIME_CC			12 // 2
#define ADCS2_SET_PERSIST_CONFIG_CC				14 // 7
#define ADCS2_SET_CONTROL_ESTIMATION_MODE_CC	15 // 42
#define ADCS2_SET_POWER_STATE_CC				23 // 56
#define ADCS2_SET_MOUNTING_CONFIG_CC			31 // 65


/* Telemetry */
#define ADCS2_GET_CURRENT_UNIX_TIME_CC			52 // 133
#define ADCS2_GET_CONTROL_ESTIMATION_MODE_CC	55 // 150
#define	ADCS2_GET_RAW_MAG_SENSOR_CC				61 // 180
#define ADCS2_GET_POWER_STATE_CC				63 // 183
#define ADCS2_GET_MOUNTING_CONFIG_CC			71 // 193
#define ADCS2_GET_RAW_GYR_SENSOR_CC				80 // 204


/* Commissioning Sequence */
#define ADCS2_COMM_01_CC						101
#define ADCS2_COMM_02_CC						102
#define ADCS2_COMM_03_CC						103
#define ADCS2_COMM_04_CC						104
#define ADCS2_COMM_05_CC						105
#define ADCS2_COMM_06_CC						106
#define ADCS2_COMM_07_CC						107
#define ADCS2_COMM_08_CC						108
#define ADCS2_COMM_09_CC						109
#define ADCS2_COMM_10_CC						110


#endif