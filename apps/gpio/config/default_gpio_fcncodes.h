/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Gpioace Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the gpioecific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * @file
 *   Gpioecification for the GPIO command function codes
 *
 * @note
 *   This file should be strictly limited to the command/function code (CC)
 *   macro definitions.  Other definitions such as enums, typedefs, or other
 *   macros should be placed in the msgdefs.h or msg.h files.
 */
#ifndef GPIO_FCNCODES_H
#define GPIO_FCNCODES_H

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/*
** Sample App command codes
*/
#define GPIO_NOOP_CC           0
#define GPIO_RESET_COUNTERS_CC 1
#define GPIO_PROCESS_CC        2
#define GPIO_DIGPIOLAY_PARAM_CC  3
#define GPIO_LTRX_EN_ON_CC     4
#define GPIO_LTRX_EN_OFF_CC    5
#define GPIO_DEP1_EN_ON_CC     6
#define GPIO_DEP1_EN_OFF_CC    7
#define GPIO_DEP2_EN_ON_CC     8
#define GPIO_DEP2_EN_OFF_CC    9
#define GPIO_SP_IN_READ_5S_CC  10
#define GPIO_STX_EN_ON_CC      11
#define GPIO_STX_EN_OFF_CC     12
#define GPIO_ADCS_EN_ON_CC     13
#define GPIO_ADCS_EN_OFF_CC    14
#define GPIO_ADCS_BOOT_ON_CC   15
#define GPIO_ADCS_BOOT_OFF_CC  16
#define GPIO_DEP1_DEP2_EN_90S_CC 17


#endif
