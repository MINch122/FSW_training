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

#define EPS_P80_POWER_IF_NAME_LEN 8

#define EPS_PACK    __attribute__((packed))



/**
 * P80 Command and Telemetry Message Definitions
*/

typedef struct {
    uint8_t csp_node; //PMU or PDU
    uint8_t mode;
    uint8_t on_cnt;
    uint8_t off_cnt;
    char    name[EPS_P80_POWER_IF_NAME_LEN];
}EPS_P80_Power_If_Set_Cmd_Payload_t;

typedef struct {
    uint8_t csp_node; //PMU or PDU
    char    name[EPS_P80_POWER_IF_NAME_LEN];
}EPS_P80_Power_If_Get_Cmd_Payload_t;

typedef struct {
    uint8_t csp_node; //PMU or PDU
}EPS_P80_Power_If_List_Cmd_Payload_t;

typedef struct{
    uint8_t csp_node; //PMU or PDU or ACU1 or ACU2
}EPS_P80_Gnd_Watchdog_Clear_Cmd_Payload_t;

typedef struct{
    uint8_t csp_node; //PMU or PDU or ACU1 or ACU2
}EPS_P80_Get_Hk_Cmd_Payload_t;

typedef struct{
    uint8_t csp_node;
    uint8_t table_id;
    uint16_t    addr;
    uint8_t     type;
    uint8_t     data[64];
    uint16_t    size;
}EPS_P80_Param_Set_Cmd_Payload_t;

typedef struct{
    uint8_t csp_node;
    uint8_t table_id;
    uint16_t    addr;
    uint8_t     type;
    uint8_t     data[64];
    uint16_t    size;
}EPS_P80_Param_Get_Cmd_Payload_t;

typedef struct{
    uint8_t csp_node;
    uint8_t table_id;
}EPS_P80_Get_Full_Table_Cmd_Payload_t;

typedef struct{
    uint8_t csp_node;
    uint8_t table_id;
}EPS_P80_Table_Save_Cmd_Payload_t;

/* Same fields as Save; typedef for clarity */
typedef EPS_P80_Table_Save_Cmd_Payload_t EPS_P80_Table_Load_Cmd_Payload_t;

/* Save all parameter tables on a node (no table_id needed) */
typedef struct{
    uint8_t csp_node;
}EPS_P80_Param_Save_Cmd_Payload_t;

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
 * P80 PMU Housekeeping telemetry payload.
 */
typedef struct {
    uint32 uptime;
    uint32 bootcause;
    uint16 resetcause;
    uint16 bootcount;
    uint16 batt_v;        /* mV */
    int16  batt_i;        /* mA */
    uint8  batt_mode;
    uint8  spare1;
    uint16 vbat_v;        /* mV */
    uint16 vcc_v;         /* mV */
    int16  temp[2];       /* ddegC */
    uint8  out_en[6];     /* bool per channel */
    int16  out_i[6];      /* mA per channel */
    uint8  sm_en[8];      /* submodule enables */
    uint16 gnd_wdt_cnt;
    uint16 bus_wdt_cnt;
    uint32 gnd_wdt_left;  /* seconds */
    uint32 bus_wdt_left;  /* seconds */
} EPS_P80_PMU_HkTlm_Payload_t;

/**
 * P80 PDU Housekeeping telemetry payload.
 */
typedef struct {
    uint32 uptime;
    uint32 bootcause;
    uint32 bootcount;
    uint16 resetcause;
    uint16 vcc_v;         /* mV */
    uint16 vcc_i;         /* mA */
    uint16 vbat_v;        /* mV */
    int16  temp;          /* ddegC */
    uint8  batt_mode;
    uint8  out_en[24];    /* bool per channel */
    uint8  spare1;
    int16  out_i[24];     /* mA per channel */
    uint32 gnd_wdt_cnt;
    uint32 bus_wdt_cnt;
    uint32 gnd_wdt_left;  /* seconds */
    uint32 bus_wdt_left;  /* seconds */
} EPS_P80_PDU_HkTlm_Payload_t;

/**
 * P80 ACU Housekeeping telemetry payload.
 */
typedef struct {
    uint32 uptime;
    uint32 bootcause;
    uint32 bootcount;
    uint16 resetcause;
    int16  input_i[6];    /* mA per channel */
    uint16 input_v[6];    /* mV per channel */
    uint16 vcc_v;         /* mV */
    uint16 vbat_v;        /* mV */
    int16  temp[3];       /* ddegC */
    uint8  mppt_mode;
    uint8  spare1;
    uint32 gnd_wdt_cnt;
    uint32 gnd_wdt_left;  /* seconds */
} EPS_P80_ACU_HkTlm_Payload_t;

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

/**
 * Beacon telemetry payload structures.
 * These match the satellite beacon packet specification.
 * All structs are packed to ensure wire-format compatibility.
 */

/* PMU Beacon - 43 bytes */
typedef struct __attribute__((packed)) {
    uint32 bootcause;          /* PMU addr 0x04 */
    uint16 resetcause;         /* PMU addr 0x08 */
    uint16 bootcount;          /* PMU addr 0x0A */
    uint8  out_en[6];          /* PMU addr 0x24, bool × 6 */
    int16  temp[2];            /* PMU addr 0x2A, int16 × 2 */
    uint8  batt_mode;          /* PMU addr 0x2E */
    int16  batt_i;             /* PMU addr 0x50 */
    uint16 batt_v;             /* PMU addr 0x52 */
    uint8  sm_en[8];           /* PMU addr 0x54, bool × 8 */
    uint16 gnd_wdt_cnt;        /* PMU addr 0x72 */
    uint16 bus_wdt_cnt;        /* PMU addr 0x74 */
    uint32 gnd_wdt_left;       /* PMU addr 0x90 */
    uint32 bus_wdt_left;       /* PMU addr 0x94 */
} EPS_BcnTlm_PMU_Payload_t;

/* PDU Beacon - 24 bytes */
typedef struct __attribute__((packed)) {
    uint8  out_en[24];         /* PDU addr 0x70, bool × 24 */
} EPS_BcnTlm_PDU_Payload_t;

/* ACU Beacon - 25 bytes */
typedef struct __attribute__((packed)) {
    int16  input_i[6];         /* ACU addr 0x10, int16 × 6 */
    uint16 input_v[6];         /* ACU addr 0x1C, uint16 × 6 */
    uint8  mppt_mode;          /* ACU addr 0x38 */
} EPS_BcnTlm_ACU_Payload_t;

/* BP8 Beacon - 22 bytes */
typedef struct __attribute__((packed)) {
    uint16 bootcount;          /* BP8 addr 0x04 */
    uint16 bootcause;          /* BP8 addr 0x06 */
    uint16 resetcause;         /* BP8 addr 0x08 */
    float  soc;                /* BP8 addr 0x0C */
    float  bat_avr_temp;       /* BP8 addr 0x14 */
    uint16 vbat;               /* BP8 addr 0x20 */
    float  current;            /* BP8 addr 0x24 */
    uint16 heater_i;           /* BP8 addr 0x2A */
} EPS_BcnTlm_BP8_Payload_t;

/* Full EPS Beacon Payload: PMU(43) + PDU(24) + ACU(25) + BP8(22) = 114 bytes */
typedef struct __attribute__((packed)) {
    EPS_BcnTlm_PMU_Payload_t  PMU;
    EPS_BcnTlm_PDU_Payload_t  PDU;
    EPS_BcnTlm_ACU_Payload_t  ACU;
    EPS_BcnTlm_BP8_Payload_t  BP8;
} EPS_BcnTlm_Full_Payload_t;

/**
 * BP8 Battery Pack Command and Telemetry Definitions
 */
typedef struct {
    uint16 Duration;  /* Heater duration in seconds (1-600, 0=stop) */
    uint16 spare;
} EPS_BP8_SetHeater_Payload_t;

typedef struct {
    uint32 Uptime;
    uint16 BootCount;
    uint16 BootCause;
    uint16 ResetCause;
    uint16 Vbat;          /* mV */
    float  Soc;           /* 0.0-1.0 */
    float  Current;       /* A */
    uint16 InCurrent;     /* mA */
    uint16 OutCurrent;    /* mA */
    uint16 HeaterCurrent; /* mA */
    int16  IntTemp;       /* ddegC */
    float  BatAvrTemp;    /* degC */
    int16  BatTemp[4];    /* ddegC */
    uint16 OVoltCount;
    uint8  BatFault;
    uint8  spare2;
} EPS_BP8_HkTlm_Payload_t;

#endif
