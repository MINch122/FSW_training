/**
 * @file
 *   EPS P80 (NanoPower) Device Driver Interface
 *
 * Provides hardware-level functions for communicating with GomSpace P80
 * EPS subsystem nodes (PMU, PDU, ACU) via CSP/rparam.
 * This driver is independent of cFE command/telemetry structures.
 */
#ifndef EPS_P80_DRV_H
#define EPS_P80_DRV_H

#include <stdint.h>
#include <gs/util/error.h>

/**
 * PMU Housekeeping data structure (driver-level, no cFE dependency)
 */
typedef struct {
    uint32_t uptime;
    uint32_t bootcause;
    uint16_t resetcause;
    uint16_t bootcount;
    uint16_t batt_v;        /* mV */
    int16_t  batt_i;        /* mA */
    uint8_t  batt_mode;
    uint16_t vbat_v;        /* mV */
    uint16_t vcc_v;         /* mV */
    int16_t  temp[2];       /* ddegC */
    uint8_t  out_en[6];     /* bool per channel */
    int16_t  out_i[6];      /* mA per channel */
    uint8_t  sm_en[8];      /* submodule enables */
    uint16_t gnd_wdt_cnt;
    uint16_t bus_wdt_cnt;
    uint32_t gnd_wdt_left;  /* seconds */
    uint32_t bus_wdt_left;  /* seconds */
} EPS_P80_Drv_PMU_HkTlm_t;

/**
 * PDU Housekeeping data structure (driver-level, no cFE dependency)
 */
typedef struct {
    uint32_t uptime;
    uint32_t bootcause;
    uint32_t bootcount;
    uint16_t resetcause;
    uint16_t vcc_v;         /* mV */
    uint16_t vcc_i;         /* mA */
    uint16_t vbat_v;        /* mV */
    int16_t  temp;          /* ddegC */
    uint8_t  batt_mode;
    uint8_t  out_en[24];    /* bool per channel */
    int16_t  out_i[24];     /* mA per channel */
    uint32_t gnd_wdt_cnt;
    uint32_t bus_wdt_cnt;
    uint32_t gnd_wdt_left;  /* seconds */
    uint32_t bus_wdt_left;  /* seconds */
} EPS_P80_Drv_PDU_HkTlm_t;

/**
 * ACU Housekeeping data structure (driver-level, no cFE dependency)
 */
typedef struct {
    uint32_t uptime;
    uint32_t bootcause;
    uint32_t bootcount;
    uint16_t resetcause;
    int16_t  input_i[6];    /* mA per channel */
    uint16_t input_v[6];    /* mV per channel */
    uint16_t vcc_v;         /* mV */
    uint16_t vbat_v;        /* mV */
    int16_t  temp[3];       /* ddegC */
    uint8_t  mppt_mode;
    uint32_t gnd_wdt_cnt;
    uint32_t gnd_wdt_left;  /* seconds */
} EPS_P80_Drv_ACU_HkTlm_t;

/**
 * Beacon data structures (driver-level, no cFE dependency)
 */
typedef struct {
    uint32_t bootcause;
    uint16_t resetcause;
    uint16_t bootcount;
    uint8_t  out_en[6];
    int16_t  temp[2];
    uint8_t  batt_mode;
    int16_t  batt_i;
    uint16_t batt_v;
    uint8_t  sm_en[8];
    uint16_t gnd_wdt_cnt;
    uint16_t bus_wdt_cnt;
    uint32_t gnd_wdt_left;
    uint32_t bus_wdt_left;
} EPS_P80_Drv_PMU_BcnTlm_t;

typedef struct {
    uint8_t out_en[24];
} EPS_P80_Drv_PDU_BcnTlm_t;

typedef struct {
    int16_t  input_i[6];
    uint16_t input_v[6];
    uint8_t  mppt_mode;
} EPS_P80_Drv_ACU_BcnTlm_t;

/**
 * Power Interface Commands
 */
gs_error_t EPS_P80_Drv_PowerIfGet(uint8_t csp_node, const char *name, uint32_t timeout_ms);
gs_error_t EPS_P80_Drv_PowerIfSet(uint8_t csp_node, const char *name,
                                   uint8_t mode, uint8_t on_cnt, uint8_t off_cnt,
                                   uint32_t timeout_ms);
gs_error_t EPS_P80_Drv_PowerIfList(uint8_t csp_node, uint32_t timeout_ms);

/**
 * Housekeeping - per node type
 * @param hk  Output: parsed housekeeping data (NULL to skip storage)
 */
gs_error_t EPS_P80_Drv_PMU_GetHk(uint8_t csp_node, EPS_P80_Drv_PMU_HkTlm_t *hk, uint32_t timeout_ms);
gs_error_t EPS_P80_Drv_PDU_GetHk(uint8_t csp_node, EPS_P80_Drv_PDU_HkTlm_t *hk, uint32_t timeout_ms);
gs_error_t EPS_P80_Drv_ACU_GetHk(uint8_t csp_node, EPS_P80_Drv_ACU_HkTlm_t *hk, uint32_t timeout_ms);

/**
 * Beacon data fetch - per node type
 */
gs_error_t EPS_P80_Drv_PMU_GetBcn(uint8_t csp_node, EPS_P80_Drv_PMU_BcnTlm_t *bcn, uint32_t timeout_ms);
gs_error_t EPS_P80_Drv_PDU_GetBcn(uint8_t csp_node, EPS_P80_Drv_PDU_BcnTlm_t *bcn, uint32_t timeout_ms);
gs_error_t EPS_P80_Drv_ACU_GetBcn(uint8_t csp_node, EPS_P80_Drv_ACU_BcnTlm_t *bcn, uint32_t timeout_ms);

/**
 * Ground Watchdog Clear - per node type
 */
gs_error_t EPS_P80_Drv_PMU_GndWdtClear(uint8_t csp_node, uint32_t timeout_ms);
gs_error_t EPS_P80_Drv_PDU_GndWdtClear(uint8_t csp_node, uint32_t timeout_ms);
gs_error_t EPS_P80_Drv_ACU_GndWdtClear(uint8_t csp_node, uint32_t timeout_ms);

/**
 * Remote Parameter Commands
 */
gs_error_t EPS_P80_Drv_ParamSet(uint8_t csp_node, uint8_t table_id,
                                 uint16_t addr, uint8_t type,
                                 const uint8_t *data, uint16_t size,
                                 uint32_t timeout_ms);
gs_error_t EPS_P80_Drv_ParamGet(uint8_t csp_node, uint8_t table_id,
                                 uint16_t addr, uint8_t type,
                                 uint8_t *data, uint16_t size,
                                 uint32_t timeout_ms);
gs_error_t EPS_P80_Drv_GetFullTable(uint8_t csp_node, uint8_t table_id, uint32_t timeout_ms);

/**
 * Table Save/Load Commands
 */
gs_error_t EPS_P80_Drv_TableSave(uint8_t csp_node, uint8_t table_id, uint32_t timeout_ms);
gs_error_t EPS_P80_Drv_TableLoad(uint8_t csp_node, uint8_t table_id, uint32_t timeout_ms);
gs_error_t EPS_P80_Drv_ParamSaveAll(uint8_t csp_node, uint32_t timeout_ms);

#endif /* EPS_P80_DRV_H */
