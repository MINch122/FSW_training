/**
 * @file
 *   EPS BP8 (NanoPower Battery Pack) Device Driver Interface
 *
 * Provides hardware-level functions for communicating with GomSpace
 * NanoPower BP8 battery pack via CSP/rparam.
 * This driver is independent of cFE command/telemetry structures.
 */
#ifndef EPS_BP8_DRV_H
#define EPS_BP8_DRV_H

#include <stdint.h>
#include <gs/param/table.h>
#include <gs/param/types.h>
#include <gs/util/error.h>

#ifndef EPS_BP8_DRV_PACK
#define EPS_BP8_DRV_PACK __attribute__((packed))
#endif

/**
 * BP8 Table IDs
 */
#define EPS_BP8_TABLE_BOARD          0
#define EPS_BP8_TABLE_CONFIGURATION  1
#define EPS_BP8_TABLE_CALIBRATION    2
#define EPS_BP8_TABLE_CONTROL        3
#define EPS_BP8_TABLE_TELEMETRY      4

/**
 * BP8 Telemetry Table Parameter Addresses
 */
#define EPS_BP8_TLM_UPTIME       0x00
#define EPS_BP8_TLM_BOOTCOUNT    0x04
#define EPS_BP8_TLM_BOOTCAUSE    0x06
#define EPS_BP8_TLM_RESETCAUSE   0x08
#define EPS_BP8_TLM_SOC          0x0C
#define EPS_BP8_TLM_INT_TEMP     0x10
#define EPS_BP8_TLM_BAT_AVR_TEMP 0x14
#define EPS_BP8_TLM_BAT_1_TEMP   0x18
#define EPS_BP8_TLM_BAT_2_TEMP   0x1A
#define EPS_BP8_TLM_BAT_3_TEMP   0x1C
#define EPS_BP8_TLM_BAT_4_TEMP   0x1E
#define EPS_BP8_TLM_VBAT         0x20
#define EPS_BP8_TLM_I            0x24
#define EPS_BP8_TLM_IN_I         0x28
#define EPS_BP8_TLM_HEATER_I     0x2A
#define EPS_BP8_TLM_OUT_I        0x2C
#define EPS_BP8_TLM_O_VOLT_COUNT 0x2E
#define EPS_BP8_TLM_BAT_FAULT    0x30

/**
 * BP8 Control Table Parameter Addresses
 */
#define EPS_BP8_CTRL_SOC_RESET    0x00
#define EPS_BP8_CTRL_FAULT_RESET  0x01
#define EPS_BP8_CTRL_FUSE_BURN    0x02
#define EPS_BP8_CTRL_HEAT_MANUAL  0x12

/**
 * BP8 Housekeeping Telemetry Structure (driver-level, no cFE dependency)
 */
typedef struct EPS_BP8_DRV_PACK {
    uint32_t Uptime;
    uint16_t BootCount;
    uint16_t BootCause;
    uint16_t ResetCause;
    uint16_t Vbat;          /* mV */
    float    Soc;           /* 0.0-1.0 */
    float    Current;       /* A */
    uint16_t InCurrent;     /* mA */
    uint16_t OutCurrent;    /* mA */
    uint16_t HeaterCurrent; /* mA */
    int16_t  IntTemp;       /* ddegC */
    float    BatAvrTemp;    /* degC */
    int16_t  BatTemp[4];    /* ddegC */
    uint16_t OVoltCount;
    uint8_t  BatFault;
} EPS_BP8_Drv_HkTlm_t;

/**
 * BP8 beacon telemetry structure (only fields used by the combined beacon).
 */
typedef struct EPS_BP8_DRV_PACK {
    uint16_t BootCount;
    uint16_t BootCause;
    uint16_t ResetCause;
    float    Soc;
    float    BatAvrTemp;
    uint16_t Vbat;
    float    Current;
    uint16_t HeaterCurrent;
} EPS_BP8_Drv_BcnTlm_t;

/**
 * Get BP8 housekeeping telemetry data.
 * @param csp_node   CSP node address of the BP8
 * @param hk         Output: parsed telemetry data
 * @param timeout_ms CSP timeout in milliseconds
 * @return GS_OK on success, error code on failure
 */
gs_error_t EPS_BP8_Drv_GetHk(uint8_t csp_node, EPS_BP8_Drv_HkTlm_t *hk, uint32_t timeout_ms);

/**
 * Get only the BP8 fields used by the combined beacon.
 */
gs_error_t EPS_BP8_Drv_GetBcn(uint8_t csp_node, EPS_BP8_Drv_BcnTlm_t *bcn, uint32_t timeout_ms);

/**
 * Remote Parameter Commands
 */
gs_error_t EPS_BP8_Drv_ParamSet(uint8_t csp_node, uint8_t table_id,
                                 uint16_t addr, uint8_t type,
                                 const uint8_t *data, uint16_t size,
                                 uint32_t timeout_ms);
gs_error_t EPS_BP8_Drv_ParamGet(uint8_t csp_node, uint8_t table_id,
                                 uint16_t addr, uint8_t type,
                                 uint8_t *data, uint16_t size,
                                 uint32_t timeout_ms);
gs_error_t EPS_BP8_Drv_GetFullTable(uint8_t csp_node, uint8_t table_id,
                                     gs_param_table_instance_t *tinst, uint32_t timeout_ms);

/**
 * Table Save/Load Commands
 */
gs_error_t EPS_BP8_Drv_TableSave(uint8_t csp_node, uint8_t table_id, uint32_t timeout_ms);
gs_error_t EPS_BP8_Drv_TableLoad(uint8_t csp_node, uint8_t table_id, uint32_t timeout_ms);
gs_error_t EPS_BP8_Drv_ParamSaveAll(uint8_t csp_node, uint32_t timeout_ms);

#endif /* EPS_BP8_DRV_H */
