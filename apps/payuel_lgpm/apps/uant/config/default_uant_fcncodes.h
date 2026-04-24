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
 *   Specification for the UANT_APP command function codes
 *
 * @note
 *   This file should be strictly limited to the command/function code (CC)
 *   macro definitions.  Other definitions such as enums, typedefs, or other
 *   macros should be placed in the msgdefs.h or msg.h files.
 */
#ifndef UANT_APP_FCNCODES_H
#define UANT_APP_FCNCODES_H

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/*
** ISIS UANT command codes
*/
#define UANT_NOOP_CC                          1
#define UANT_RESET_COUNTERS_CC                2
#define UANT_RESET_CC                         3
#define UANT_GET_DEPLOYMENT_STATUS_CC         4
#define UANT_ARM_ANTENNA_SYSTEMS_CC           5
#define UANT_DISARM_CC                        6
#define UANT_AUTOMATED_DEPLOYMENT_CC          7
#define UANT_CANCEL_DEPLOYMENT_ACTIVATION_CC  8
#define UANT_DEPLOY_ANT1_CC                   9
#define UANT_DEPLOY_ANT2_CC                   10
#define UANT_DEPLOY_ANT3_CC                   11
#define UANT_DEPLOY_ANT4_CC                   12
#define UANT_DEPLOY_ANT1_OVERRIDE_CC          13
#define UANT_DEPLOY_ANT2_OVERRIDE_CC          14
#define UANT_DEPLOY_ANT3_OVERRIDE_CC          15
#define UANT_DEPLOY_ANT4_OVERRIDE_CC          16
#define UANT_REPORT_ANT_ACTIVATION_CNT_CC     17
#define UANT_REPORT_ANT_ACTIVATION_TIME_CC    18
#define UANT_MEASURE_SYSTEM_TEMPERATURE_CC    19

#endif