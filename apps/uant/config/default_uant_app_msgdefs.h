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
 *   Specification for the UANT_APP command and telemetry 
 *   message constant definitions.
 *
 *  For UANT_APP this is only the function/command code definitions
 */
#ifndef UANT_APP_MSGDEFS_H
#define UANT_APP_MSGDEFS_H

#include "common_types.h"
#include "uant_app_fcncodes.h"
#include "rpt_interface_cfg.h"

typedef struct {
    /* 보드 A (I2C 0x05) */
    uint8 ch0_status_A;      /* release_status.channel_0_status */
    uint8 ch1_status_A;      /* release_status.channel_1_status */
    uint8 backup_active_A;   /* backup_settings.backup_active   */
    // uint8 state_A;           /* backup_status.state             */

    /* 보드 B (I2C 0x06) */
    uint8 ch0_status_B;      /* release_status.channel_0_status */
    uint8 ch1_status_B;      /* release_status.channel_1_status */
    uint8 backup_active_B;   /* backup_settings.backup_active   */
    // uint8 state_B;           /* backup_status.state             */
    
} UANT_APP_BcnTlm_Payload_t;


typedef struct {
    int8_t IsDeploy; /* <\brief `1` for deploy, `0` for NOT deploy, negative for error */
} UANT_InternalTlm_Payload_t;

typedef struct {
    uint8_t address; /* <\brief should be `0x05` or `0x06` */
    uint8_t channel; /* <\brief should be `0` or `1` */
} UANT_RequestRelease_Payload_t;

#endif
