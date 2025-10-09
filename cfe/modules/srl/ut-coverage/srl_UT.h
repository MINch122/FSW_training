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
 * Purpose:
 *    SRL unit test header
 *
 * References:
 *    1. cFE Application Developers Guide
 *    2. unit test standard 092503
 *    3. C Coding Standard 102904
 *
 * Notes:
 *    1. This is unit test code only, not for use in flight
 *
 */

#ifndef SRL_UT_H
#define SRL_UT_H

/**
 * Includes
 */
#include "cfe_srl_module_all.h"
#include "ut_support.h"

/*
** Structures
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint32                  Cmd32Param1;
    uint16                  Cmd16Param1;
    uint16                  Cmd16Param2;
    uint8                   Cmd8Param1;
    uint8                   Cmd8Param2;
    uint8                   Cmd8Param3;
    uint8                   Cmd8Param4;
} SRL_UT_Test_Cmd_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    uint32                    Tlm32Param1;
    uint16                    Tlm16Param1;
    uint16                    Tlm16Param2;
    uint8                     Tlm8Param1;
    uint8                     Tlm8Param2;
    uint8                     Tlm8Param3;
    uint8                     Tlm8Param4;
} SRL_UT_Test_Tlm_t;

#define SRL_UT_CMD_MID_VALUE_BASE 0x23
#define SRL_UT_TLM_MID_VALUE_BASE 0x27


void Test_SRL_TaskInit(void);
void Test_SRL_TaskMain(void);
void Test_SRL_TaskPipe(void);
void Test_SRL_EarlyInit(void);
void Test_SRL_Api(void);


#endif