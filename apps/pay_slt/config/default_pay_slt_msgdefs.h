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
 *   Specification for the SAMPLE_APP command and telemetry
 *   message constant definitions.
 *
 *  For SAMPLE_APP this is only the function/command code definitions
 */
#ifndef SLT_IFB_MSGDEFS_H
#define SLT_IFB_MSGDEFS_H

#include "common_types.h"
#include "pay_slt_fcncodes.h"

#define PAY_SLT_BRD_UID_SIZE      16
#define PAY_SLT_RTABLE_STR_SIZE   96

typedef struct __attribute__((__packed__))
{
    uint8 CommandErrorCounter;
    uint8 CommandCounter;
    int16 sys_status;
    uint32 sys_uptime;
    uint16 boot_cnt;
    uint16 boot_cause;
    uint16 reboot_cause;
    uint32 wdt_left;
    int16 brd_temp;
    uint16 pwr_current;
    uint16 imu_data[6];
    int16 ntc_data[4];
    uint8 spare[2];
}SLT_IFB_HkTlm_Payload_t;

typedef struct __attribute__((__packed__))
{
    uint8 CommandErrorCounter;
    uint8 CommandCounter;
    int16 sys_status;
    uint16 boot_cnt;
    int16 brd_temp;
    uint16 pwr_current;
} SLT_IFB_BcnTlm_Payload_t;

#endif
