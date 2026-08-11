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
 *   Gpioecification for the GPIO command and telemetry
 *   message constant definitions.
 *
 *  For GPIO this is only the function/command code definitions
 */
#ifndef GPIO_MSGDEFS_H
#define GPIO_MSGDEFS_H

#include "common_types.h"
#include "gpio_fcncodes.h"

typedef struct GPIO_DigpiolayParam_Payload
{
    uint32 ValU32;                            /**< 32 bit unsigned integer value */
    int16  ValI16;                            /**< 16 bit signed integer value */
    char   ValStr[GPIO_STRING_VAL_LEN]; /**< An example string */
} GPIO_DigpiolayParam_Payload_t;

/*************************************************************************/
/*
** Type definition (Sample App housekeeping)
*/

#define GPIO_DEP_BURN_CHANNEL_LTRX 0u
#define GPIO_DEP_BURN_CHANNEL_DEP1 1u
#define GPIO_DEP_BURN_CHANNEL_DEP2 2u

typedef struct GPIO_DepBurn_Payload
{
    uint8  Channel;
    uint8  Reserved[3];
    uint32 BurnTimeSeconds;
} GPIO_DepBurn_Payload_t;

typedef struct GPIO_HkTlm_Payload
{
    uint8 CommandErrorCounter;
    uint8 CommandCounter;
    uint8 GpioState[6];
    uint8 isDeployed;
} GPIO_HkTlm_Payload_t;

typedef struct GPIO_BcnTlm_Payload
{
    uint8 GpioState;
    uint8 Padding;
    uint8 isDeployed;
} __attribute__((packed)) GPIO_BcnTlm_Payload_t;

typedef struct GPIO_SpInReadReport_Payload
{
    uint8 isDeployed;
} GPIO_SpInReadReport_Payload_t;

#endif
