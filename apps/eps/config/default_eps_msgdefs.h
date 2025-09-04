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
 *   Specification for the EPS command and telemetry
 *   message constant definitions.
 */
#ifndef EPS_MSGDEFS_H
#define EPS_MSGDEFS_H

#include "common_types.h"
#include "eps_fcncodes.h"

typedef struct {
    uint8_t  channel;
    uint8_t  value;
    uint16_t delay;
} EPS_P31U_SetOutputSingle_Payload_t;

typedef struct {
    uint8_t  mask;
} EPS_P31U_SetOutputs_Payload_t;

typedef struct {
    uint8_t  id;
    uint16_t size;
} EPS_P31U_GetHk_Payload_t;
 
typedef struct {
    int16_t  voltage[3];
} EPS_P31U_SetPvVolt_Payload_t;

typedef struct {
    uint8_t  mode;
} EPS_P31U_SetPvAuto_Payload_t;

typedef struct {
    uint8_t  cmd;
    uint8_t  heater;
    uint8_t  mode;
} EPS_P31U_SetHeater_Payload_t;

typedef struct {
    uint8_t cmd;
} EPS_P31U_Config_Payload_t;

typedef struct {
    uint8_t  ppt_mode;
    uint8_t  battheater_mode;
    int8_t   battheater_low;
    int8_t   battheater_high;
    uint8_t  output_normal_value[8];
    uint8_t  output_safe_value[8];
    uint16_t output_initial_on_delay[8];
    uint16_t output_initial_off_delay[8];
    uint16_t vboost[3];
} EPS_P31U_SetConfig_Payload_t;

typedef struct {
    uint8_t cmd;
} EPS_P31U_Config2_Payload_t;

typedef struct {
    uint16_t batt_maxvoltage;
    uint16_t batt_safevoltage;
    uint16_t batt_criticalvoltage;
    uint16_t batt_normalvoltage;
    uint32_t reserved1[2];
    uint8_t  reserved2[4];
} EPS_P31U_SetConfig2_Payload_t;

typedef struct {
    uint8_t  version;
    uint8_t  cmd;
    uint8_t  length;
    uint8_t  flags;
    uint16_t cur_lim[8];
    uint8_t  cur_ema_gain;
    uint8_t  cspwdt_channel[2];
    uint8_t  cspwdt_address[2];
} EPS_P31U_SetConfig3_Payload_t;

typedef struct {
    uint8_t  port;
    uint8_t  reserved;
    uint8_t  txSize;
    uint8_t  rxSize;
    uint8_t  tx[128];
} EPS_P31U_Transaction_Payload_t;

/*************************************************************************/
/*
** Type definition (EPS housekeeping)
*/

/**
 * @brief EPS housekeeping packet payload.
 */
typedef struct {
    uint16 vbatt;
    uint8  output[8];
    uint16 curout[6];
    uint8  latchup[6];
    uint16 curin[3];
    uint16 cursun;
    uint16 cursys;
    uint16 counter_boot;
    uint32 wdt_gnd_time_left;
    int16  temp[3];  /* TEMP1, BP4a, BP4b */
    uint8  bootcause;
    uint8  battmode;
} EPS_HkTlm_Payload_t;

/**
 * @brief EPS beacon packet payload.
 *        Packed attribute is mandatory due to the uint32 member.
 *        (2-byte tail padding with natural alignment)
 */
typedef struct __attribute__((packed)) {
    uint16 vbatt;
    uint8  output[8];
    uint16 curout[6];
    uint16 curin[3];
    uint16 cursys;
    uint16 counter_boot;
    uint32 wdt_gnd_time_left;
    uint8  bootcause;
    uint8  battmode;

    /**
     * Indicate battery heater control mode (`Auto = 1` or `Manual = 0`)
     */
    uint8  battheater_mode;

    /**
     * Battery Temperature
     * [0] : BP4 temperature
     * [1] : Onboard temperature
     */
    int16  bp4_temp[2];
} EPS_BcnTlm_Payload_t;

#endif
