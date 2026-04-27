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
 *   Specification for the PAYUEL_LGPM_APP command function codes
 *
 * @note
 *   This file should be strictly limited to the command/function code (CC)
 *   macro definitions.  Other definitions such as enums, typedefs, or other
 *   macros should be placed in the msgdefs.h or msg.h files.
 */


#ifndef PAYUEL_LGPM_FCNCODES_H
#define PAYUEL_LGPM_FCNCODES_H


/************************************************************************
 * Macro Definitions
 ************************************************************************/

/*
** PAYUEL_LGPM command codes
*/

#define PAYUEL_LGPM_NOOP_CC                         0  
#define PAYUEL_LGPM_RESET_COUNTERS_CC               1  
#define PAYUEL_LGPM_MCU_ALIVE_CHECK_CC              2
#define PAYUEL_LGPM_3V3_PWR_ON_CC                   3
#define PAYUEL_LGPM_3V3_PWR_OFF_CC                  4
#define PAYUEL_LGPM_MAIN_BOOST_SW_ON_CC             5
#define PAYUEL_LGPM_MAIN_BOOST_SW_OFF_CC            6
#define PAYUEL_LGPM_SUB_BOOST_SW_ON_CC              7
#define PAYUEL_LGPM_SUB_BOOST_SW_OFF_CC             8
#define PAYUEL_LGPM_V28_MAIN_ON_CC                  9
#define PAYUEL_LGPM_V28_MAIN_OFF_CC                 10
#define PAYUEL_LGPM_V28_SUB_ON_CC                   11
#define PAYUEL_LGPM_V28_SUB_OFF_CC                  12
#define PAYUEL_LGPM_V12_MAIN_ON_CC                  13
#define PAYUEL_LGPM_V12_MAIN_OFF_CC                 14
#define PAYUEL_LGPM_PWR_SENSE_INFO_CC               15
#define PAYUEL_LGPM_PWR_SEQ_ON_CC                   16
#define PAYUEL_LGPM_PWR_SEQ_OFF_CC                  17
#define PAYUEL_LGPM_RWA_CONTROL_CC                  18
#define PAYUEL_LGPM_RWA_PWR_ON_CC                   19
#define PAYUEL_LGPM_RWA_PWR_OFF_CC                  20
#define PAYUEL_LGPM_RWA_SENSE_INFO_CC               21

#endif /* PAYUEL_LGPM_FCNCODES_H */