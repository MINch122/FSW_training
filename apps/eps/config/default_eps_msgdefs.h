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
#include "rpt_interface_cfg.h"

#define EPS_P80_POWER_IF_NAME_LEN 8

#define EPS_PACK    __attribute__((packed))
#define EPS_BCN_ACU_COUNT 2
#define EPS_RPARAM_DATA_MAX_LEN 128
/* Matches libgscsp RPARAM store/slot fields: 25 chars plus NUL. */
#define EPS_RPARAM_STORE_NAME_LEN 26
#define EPS_RPARAM_STORE_SLOT_LEN 26

/*
 * EPS query command report envelope.
 *
 * These payloads are carried inside RPT_Report_t.ReturnValue for EPS commands
 * whose successful response data used to be printed only through OS_printf().
 */
#define EPS_QUERY_REPORT_HEADER_SIZE   12
#define EPS_QUERY_REPORT_DATA_MAX_LEN  (RPT_RET_VALUE_BUF_SIZE - EPS_QUERY_REPORT_HEADER_SIZE)

typedef enum
{
    EPS_QUERY_REPORT_P80_POWER_IF_STATUS = 1,
    EPS_QUERY_REPORT_P80_POWER_IF_LIST   = 2,
    EPS_QUERY_REPORT_P80_PMU_HK          = 3,
    EPS_QUERY_REPORT_P80_PDU_HK          = 4,
    EPS_QUERY_REPORT_P80_ACU_HK          = 5,
    EPS_QUERY_REPORT_BP8_HK              = 6,
    EPS_QUERY_REPORT_RPARAM_VALUE        = 7,
    EPS_QUERY_REPORT_RPARAM_TABLE_ROWS   = 8,
    EPS_QUERY_REPORT_RPARAM_TABLE_MEMORY = 9,
    EPS_QUERY_REPORT_CSP_PING_MS         = 10,
    EPS_QUERY_REPORT_CSP_PS_TEXT         = 11,
    EPS_QUERY_REPORT_CSP_MEMFREE         = 12,
    EPS_QUERY_REPORT_CSP_BUF_FREE        = 13,
    EPS_QUERY_REPORT_CSP_UPTIME          = 14
} EPS_Query_Report_DataId_t;

typedef struct EPS_PACK
{
    uint8  source;      /* CSP node for hardware query reports. */
    uint8  data_id;     /* EPS_Query_Report_DataId_t */
    uint8  arg0;        /* Command-specific: RPARAM table id when applicable. */
    uint8  arg1;        /* Command-specific: RPARAM type when applicable. */
    uint16 sequence;    /* Chunk sequence, starting at 0. */
    uint16 offset;      /* Byte offset into the full command result. */
    uint16 total_size;  /* Full result size in bytes. */
    uint16 chunk_size;  /* Valid bytes in data[]. */
    uint8  data[EPS_QUERY_REPORT_DATA_MAX_LEN];
} EPS_Query_Report_Payload_t;



/**
 * P80 Command and Telemetry Message Definitions
*/

/*
 * P80 Power_If_Set payload:
 * csp_node = P80 PMU(1) or P80 PDU(4).
 * mode     = GomSpace power_if SET mode.
 * on_cnt   = GomSpace power_if SET on counter in seconds.
 * off_cnt  = GomSpace power_if SET off counter in seconds.
 * name     = target power-if channel name or decimal channel string;
 *            CACTUS string input is fixed-length and NUL-padded to
 *            EPS_P80_POWER_IF_NAME_LEN.
 */
typedef struct EPS_PACK {
    uint8_t csp_node; //PMU or PDU
    uint8_t mode;
    uint16_t on_cnt;
    uint16_t off_cnt;
    char    name[EPS_P80_POWER_IF_NAME_LEN];
}EPS_P80_Power_If_Set_Cmd_Payload_t;

/*
 * P80 Power_If_Get payload:
 * csp_node = P80 PMU(1) or P80 PDU(4).
 * name     = power-if channel name or decimal channel string, up to
 *            EPS_P80_POWER_IF_NAME_LEN bytes.
 */
typedef struct EPS_PACK {
    uint8_t csp_node; //PMU or PDU
    char    name[EPS_P80_POWER_IF_NAME_LEN];
}EPS_P80_Power_If_Get_Cmd_Payload_t;

/*
 * P80 Power_If_List payload:
 * csp_node = P80 PMU(1) or P80 PDU(4).
 */
typedef struct EPS_PACK {
    uint8_t csp_node; //PMU or PDU
}EPS_P80_Power_If_List_Cmd_Payload_t;

/*
 * P80 Ground Watchdog Clear payload:
 * csp_node = P80 PMU(1), P80 ACU1(2), P80 ACU2(3), or P80 PDU(4).
 * BP8(7) is not valid for this command.
 */
typedef struct EPS_PACK {
    uint8_t csp_node; //PMU or PDU or ACU1 or ACU2
}EPS_P80_Gnd_Watchdog_Clear_Cmd_Payload_t;

/*
 * Get HK payload:
 * csp_node = P80 PMU(1), P80 ACU1(2), P80 ACU2(3), P80 PDU(4), or BP8(7).
 */
typedef struct EPS_PACK {
    uint8_t csp_node; //PMU or PDU or ACU1 or ACU2 or BP8
}EPS_Get_HK_Cmd_Payload_t;

/*
 * RParam Set payload:
 * csp_node = P80 PMU(1), P80 ACU1(2), P80 ACU2(3), P80 PDU(4), or BP8(7).
 * table_id = board(0), configuration(1), calibration(2), telemetry(4);
 *            BP8 also supports control(3).
 * addr     = parameter offset in that table.
 * type     = gs_param_type_t from gs/param/types.h:
 *            UINT8=0, UINT16=1, UINT32=2, UINT64=3,
 *            INT8=4, INT16=5, INT32=6, INT64=7,
 *            DOUBLE=12, FLOAT=13, STRING=14, DATA=15, BOOL=16.
 * data     = raw bytes. In CACTUS bytes[128], enter hex such as "01" or
 *            "01 00"; CACTUS pads shorter input with 00.
 * size     = valid byte count; should match the selected type/array/string
 *            length. size > 128 is rejected.
 */
typedef struct EPS_PACK {
    uint8_t csp_node;
    uint8_t table_id;
    uint16_t    addr;
    uint8_t     type; 
    uint8_t     data[EPS_RPARAM_DATA_MAX_LEN];
    uint16_t    size;
}EPS_RParam_Set_Cmd_Payload_t;

/*
 * RParam Get payload:
 * csp_node = same mapping as RParam Set.
 * table_id = same mapping as RParam Set.
 * addr     = same mapping as RParam Set.
 * type     = same mapping as RParam Set.
 * size     = number of bytes to read, max EPS_RPARAM_DATA_MAX_LEN(128).
 */
typedef struct EPS_PACK {
    uint8_t csp_node;
    uint8_t table_id;
    uint16_t    addr;
    uint8_t     type;
    uint16_t    size;
}EPS_RParam_Get_Cmd_Payload_t;

/*
 * RParam Get report payload:
 * data = bytes returned by the remote RPARAM get transaction.
 */
typedef struct EPS_PACK {
    uint8_t csp_node;
    uint8_t table_id;
    uint16_t    addr;
    uint8_t     type;
    uint8_t     data[EPS_RPARAM_DATA_MAX_LEN];
    uint16_t    size;
}EPS_RParam_Get_Report_Payload_t;

/*
 * RParam Get Full Table payload:
 * csp_node = target EPS node.
 * table_id = remote parameter table id using the same table mapping as
 *            RParam Set.
 */
typedef struct EPS_PACK {
    uint8_t csp_node;
    uint8_t table_id;
}EPS_RParam_Get_Full_Table_Cmd_Payload_t;

/*
 * RParam Table Save payload:
 * csp_node = same mapping as RParam Set.
 * table_id = same mapping as RParam Set.
 * action   = saves one remote table to its primary/default store.
 */
typedef struct EPS_PACK {
    uint8_t csp_node;
    uint8_t table_id;
}EPS_RParam_Table_Save_Cmd_Payload_t;

/* Same fields as Save; typedef for clarity */
typedef EPS_RParam_Table_Save_Cmd_Payload_t EPS_RParam_Table_Load_Cmd_Payload_t;

/*
 * RParam Save/Load Store payload:
 * csp_node = same mapping as RParam Set.
 * table_id = same mapping as RParam Set.
 * store    = required param-4 store name, for example persistent,
 *            protected, or flash.
 * slot     = optional slot name; leave empty to request the default slot.
 */
typedef struct EPS_PACK {
    uint8_t csp_node;
    uint8_t table_id;
    char    store[EPS_RPARAM_STORE_NAME_LEN];
    char    slot[EPS_RPARAM_STORE_SLOT_LEN];
}EPS_RParam_Store_Cmd_Payload_t;

typedef EPS_RParam_Store_Cmd_Payload_t EPS_RParam_Save_To_Store_Cmd_Payload_t;
typedef EPS_RParam_Store_Cmd_Payload_t EPS_RParam_Load_From_Store_Cmd_Payload_t;

/* Save all parameter tables on a node (no table_id needed) */
/*
 * RParam Save All payload:
 * csp_node = target EPS node.
 * action   = persists all parameter tables on that node.
 */
typedef struct EPS_PACK {
    uint8_t csp_node;
}EPS_RParam_Save_All_Cmd_Payload_t;

/*
 * CSP PS payload:
 * csp_node = target CSP address.
 * action   = requests the remote task/process list and prints it.
 */
typedef struct EPS_PACK {
    uint8_t csp_node;
}EPS_CSP_PS_Cmd_Payload_t;

/*
 * CSP MemFree payload:
 * csp_node = target CSP address.
 * action   = queries free memory on the remote node.
 */
typedef struct EPS_PACK {
    uint8_t csp_node;
}EPS_CSP_MemFree_Cmd_Payload_t;

/*
 * CSP BufFree payload:
 * csp_node = target CSP address.
 * action   = queries free CSP buffer count on the remote node.
 */
typedef struct EPS_PACK {
    uint8_t csp_node;
}EPS_CSP_BufFree_Cmd_Payload_t;

/*
 * CSP Uptime payload:
 * csp_node = target CSP address.
 * action   = queries uptime in seconds on the remote node.
 */
typedef struct EPS_PACK {
    uint8_t csp_node;
}EPS_CSP_Uptime_Cmd_Payload_t;

/*
 * CSP Ping payload:
 * csp_node = target CSP address.
 * size     = ping payload bytes; 0 means EPS default 1 byte, max is 128.
 * opts     = CSP ping option bitmask.
 */
typedef struct EPS_PACK {
    uint8_t  csp_node;
    uint16_t size;
    uint8_t  opts;
}EPS_CSP_Ping_Cmd_Payload_t;

/*
 * CSP Reboot payload:
 * csp_node = target CSP address.
 * action   = requests a remote device reboot; use carefully because this does
 *            not wait for an application-level response.
 */
typedef struct EPS_PACK {
    uint8_t csp_node;
}EPS_CSP_Reboot_Cmd_Payload_t;

/*************************************************************************/
/*
** Type definition (EPS housekeeping)
*/

/**
 * @brief EPS housekeeping packet payload.
 */
typedef struct EPS_PACK {
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
typedef struct EPS_PACK {
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
typedef struct EPS_PACK {
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
typedef struct EPS_PACK {
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
typedef struct EPS_PACK {
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
typedef struct EPS_PACK {
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
typedef struct EPS_PACK {
    uint8  out_en[24];         /* PDU addr 0x70, bool × 24 */
} EPS_BcnTlm_PDU_Payload_t;

/* ACU Beacon - 25 bytes */
typedef struct EPS_PACK {
    int16  input_i[6];         /* ACU addr 0x10, int16 × 6 */
    uint16 input_v[6];         /* ACU addr 0x1C, uint16 × 6 */
    uint8  mppt_mode;          /* ACU addr 0x38 */
} EPS_BcnTlm_ACU_Payload_t;

/* BP8 Beacon - 22 bytes */
typedef struct EPS_PACK {
    uint16 bootcount;          /* BP8 addr 0x04 */
    uint16 bootcause;          /* BP8 addr 0x06 */
    uint16 resetcause;         /* BP8 addr 0x08 */
    float  soc;                /* BP8 addr 0x0C */
    float  bat_avr_temp;       /* BP8 addr 0x14 */
    uint16 vbat;               /* BP8 addr 0x20 */
    float  current;            /* BP8 addr 0x24 */
    uint16 heater_i;           /* BP8 addr 0x2A */
} EPS_BcnTlm_BP8_Payload_t;

/* Full EPS Beacon Payload: PMU(43) + PDU(24) + ACU(25) * 2 + BP8(22) = 139 bytes */
typedef struct EPS_PACK {
    EPS_BcnTlm_PMU_Payload_t  PMU;
    EPS_BcnTlm_PDU_Payload_t  PDU;
    EPS_BcnTlm_ACU_Payload_t  ACU[EPS_BCN_ACU_COUNT];
    EPS_BcnTlm_BP8_Payload_t  BP8;
} EPS_BcnTlm_Full_Payload_t;

/**
 * BP8 Battery Pack Command and Telemetry Definitions
 */
typedef struct EPS_PACK {
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
