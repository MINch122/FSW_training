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
#ifndef DEFAULT_EPS_MSGDEFS_H
#define DEFAULT_EPS_MSGDEFS_H

#include "common_types.h"
#include "default_eps_fcncodes.h"
#include <gs/p80/power_if.h>

#define EPS_PACK    __attribute__((packed))



/**
 * P80 Command and Telemetry Message Definitions
*/

typedef struct {
    uint8_t csp_node; //PMU or PDU
    uint8_t mode;
    uint8_t on_cnt;
    uint8_t off_cnt;
    char    name[POWER_IF_NAME_LEN];
}EPS_Power_If_Set_Cmd_Payload_t;

typedef struct {
    uint8_t csp_node; //PMU or PDU
    char    name[POWER_IF_NAME_LEN];
}EPS_Power_If_Get_Cmd_Payload_t;

typedef struct {
    uint8_t csp_node; //PMU or PDU
}EPS_Power_If_List_Cmd_Payload_t;

typedef struct{
    uint8_t csp_node; //PMU or PDU or ACU1 or ACU2
}EPS_Gnd_Watchdog_Clear_Cmd_Payload_t;

typedef struct{
    uint8_t csp_node; //PMU or PDU or ACU1 or ACU2
}EPS_Get_Hk_Cmd_Payload_t;

typedef struct{
    uint8_t csp_node;
    uint8_t table_id;
    uint16_t    addr;
    uint8_t     type;
    uint8_t     data[64];
    uint16_t    size;
}EPS_Param_Set_Cmd_Payload_t;

typedef struct{
    uint8_t csp_node;
    uint8_t table_id;
    uint16_t    addr;
    uint8_t     type;
    uint8_t     data[64];
    uint16_t    size;
}EPS_Param_Get_Cmd_Payload_t;

typedef struct{
    uint8_t csp_node;
    uint8_t table_id;
}EPS_Get_Full_Table_Cmd_Payload_t;

typedef struct{
    uint8_t csp_node;
    uint8_t table_id;
}EPS_Table_Save_Cmd_Payload_t;

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
} EPS_BcnTlm_Payload_t;

typedef struct __attribute__((packed)) {
    int16  c_out[9];
    uint16 v_out[9];
    uint16 out_en;
    uint32 bootcause;
    uint32 bootcnt;
    uint8  batt_mode;
    uint8  heater_on;
    uint16 vbat_v;
    uint16 vcc_c;
    uint16 batt_v;
    int16  batt_temp[2];
    uint32 wdt_gnd_left;
    int16  batt_chrg;
    int16  batt_dischrg;
} EPS_BcnTlm_P80_Dock_Payload_t;

typedef struct __attribute__((packed)) {
    int16  c_out[9];
    uint16 v_out[9];
    int16  vcc;
    uint8  conv_en;
    uint16 out_en;
} EPS_BcnTlm_P80_PDU_Payload_t;

typedef struct __attribute__((packed)) {
    int16  c_in[6];
    uint16 v_in[6];
} EPS_BcnTlm_P80_ACU_Payload_t;

typedef struct __attribute__((packed)) {
    EPS_BcnTlm_P80_Dock_Payload_t  Dock;
    EPS_BcnTlm_P80_PDU_Payload_t   PDU;
    EPS_BcnTlm_P80_ACU_Payload_t   ACU;
} EPS_BcnTlm_P80_Payload_t;

#endif
