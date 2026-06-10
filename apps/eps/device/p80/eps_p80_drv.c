/**
 * @file
 *   EPS P80 (NanoPower) Device Driver Implementation
 */

#include "eps_p80_drv.h"

#include <gs/param/internal/types.h>
#include <gs/param/rparam.h>
#include <gs/param/table.h>
#include <stdlib.h>
#include <string.h>

#include "osapi.h"

#include <gs/p80/p80.h>
#include <gs/p80/power_if.h>
#include <gs/p80/param/board.h>
#include <gs/p80/param/param.h>

#include <gs/p80_pmu/param/calibration.h>
#include <gs/p80_pmu/param/configuration.h>
#include <gs/p80_pmu/param/param.h>
#include <gs/p80_pmu/param/pmu.h>
#include <gs/p80_pmu/param/telemetry.h>
#include <gs/p80_pmu/param/types.h>

#include <gs/p80_pdu/param/calibration.h>
#include <gs/p80_pdu/param/configuration.h>
#include <gs/p80_pdu/param/param.h>
#include <gs/p80_pdu/pdu.h>
#include <gs/p80_pdu/param/telemetry.h>

#include <gs/p80_acu/param/calibration.h>
#include <gs/p80_acu/param/configuration.h>
#include <gs/p80_acu/param/param.h>
#include <gs/p80_acu/param/p80acu.h>
#include <gs/p80_acu/param/telemetry.h>
#include <gs/p80_acu/param/types.h>

#include "common_types.h"

static uint8_t EPS_P80_DrvPackBoolArray8(const uint8_t values[8])
{
    uint8_t mask = 0;

    for (uint8_t index = 0; index < 8; index++)
    {
        if (values[index] != 0)
        {
            mask |= (uint8_t)(1u << index);
        }
    }

    return mask;
}

#define EPS_PARAM_GET_UINT32_FIELD(tinst, addr, dst) \
    do \
    { \
        uint32_t value = 0; \
        perr |= gs_param_get_uint32((tinst), (addr), &value, 0); \
        (dst) = value; \
    } while (0)

#define EPS_PARAM_GET_UINT16_FIELD(tinst, addr, dst) \
    do \
    { \
        uint16_t value = 0; \
        perr |= gs_param_get_uint16((tinst), (addr), &value, 0); \
        (dst) = value; \
    } while (0)

#define EPS_PARAM_GET_INT16_FIELD(tinst, addr, dst) \
    do \
    { \
        int16_t value = 0; \
        perr |= gs_param_get_int16((tinst), (addr), &value, 0); \
        (dst) = value; \
    } while (0)

#define EPS_PARAM_GET_UINT8_FIELD(tinst, addr, dst) \
    do \
    { \
        uint8_t value = 0; \
        perr |= gs_param_get_uint8((tinst), (addr), &value, 0); \
        (dst) = value; \
    } while (0)

#define EPS_P80_RPARAM_GET_UINT32(table_id, addr, dst) \
    do \
    { \
        uint32_t value = 0; \
        err = gs_rparam_get_uint32(csp_node, (table_id), (addr), \
                                   GS_RPARAM_MAGIC_CHECKSUM, timeout_ms, &value); \
        if (err != GS_OK) return err; \
        (dst) = value; \
    } while (0)

#define EPS_P80_RPARAM_GET_UINT16(table_id, addr, dst) \
    do \
    { \
        uint16_t value = 0; \
        err = gs_rparam_get_uint16(csp_node, (table_id), (addr), \
                                   GS_RPARAM_MAGIC_CHECKSUM, timeout_ms, &value); \
        if (err != GS_OK) return err; \
        (dst) = value; \
    } while (0)

#define EPS_P80_RPARAM_GET_INT16(table_id, addr, dst) \
    do \
    { \
        int16_t value = 0; \
        err = gs_rparam_get_int16(csp_node, (table_id), (addr), \
                                  GS_RPARAM_MAGIC_CHECKSUM, timeout_ms, &value); \
        if (err != GS_OK) return err; \
        (dst) = value; \
    } while (0)

#define EPS_P80_RPARAM_GET_UINT8(table_id, addr, dst) \
    do \
    { \
        uint8_t value = 0; \
        err = gs_rparam_get_uint8(csp_node, (table_id), (addr), \
                                  GS_RPARAM_MAGIC_CHECKSUM, timeout_ms, &value); \
        if (err != GS_OK) return err; \
        (dst) = value; \
    } while (0)

#define EPS_P80_RPARAM_GET_BOOL(table_id, addr, dst) \
    do \
    { \
        uint8_t value = 0; \
        err = gs_rparam_get(csp_node, (table_id), (addr), GS_PARAM_BOOL, \
                            GS_RPARAM_MAGIC_CHECKSUM, timeout_ms, &value, sizeof(value)); \
        if (err != GS_OK) return err; \
        (dst) = (uint8_t)(value != 0); \
    } while (0)

#define EPS_P80_RPARAM_GET_ARRAY(table_id, addr, type, dst, count) \
    do \
    { \
        err = gs_rparam_get_array(csp_node, (table_id), (addr), (type), \
                                  GS_RPARAM_MAGIC_CHECKSUM, timeout_ms, \
                                  (dst), sizeof((dst)[0]), (count)); \
        if (err != GS_OK) return err; \
    } while (0)

/* ========================================================================== */
/*  Power Interface Commands                                                  */
/* ========================================================================== */

gs_error_t EPS_P80_Drv_PowerIfGet(uint8_t csp_node, const char *name,
                                   power_if_ch_status_t *status, uint32_t timeout_ms)
{
    if (status == NULL)
        return GS_ERROR_ARG;

    memset(status, 0, sizeof(*status));
    strncpy(status->name, name, POWER_IF_NAME_LEN - 1);
    status->name[POWER_IF_NAME_LEN - 1] = '\0';

    return power_if_cmd(csp_node, GS_P80_PORT_CMDCONTROL, timeout_ms, POWER_IF_GET, status);
}

gs_error_t EPS_P80_Drv_PowerIfSet(uint8_t csp_node, const char *name,
                                   uint8_t mode, uint16_t on_cnt, uint16_t off_cnt,
                                   power_if_ch_status_t *status, uint32_t timeout_ms)
{
    if (status == NULL)
        return GS_ERROR_ARG;

    memset(status, 0, sizeof(*status));
    status->mode = mode;
    status->on_cnt = on_cnt;
    status->off_cnt = off_cnt;
    strncpy(status->name, name, POWER_IF_NAME_LEN - 1);
    status->name[POWER_IF_NAME_LEN - 1] = 0;

    return power_if_cmd(csp_node, GS_P80_PORT_CMDCONTROL, timeout_ms, POWER_IF_SET, status);
}

gs_error_t EPS_P80_Drv_PowerIfList(uint8_t csp_node, power_if_cmd_list_response_t *list,
                                    uint32_t timeout_ms)
{
    if (list == NULL)
        return GS_ERROR_ARG;

    memset(list, 0, sizeof(*list));

    return power_if_cmd(csp_node, GS_P80_PORT_CMDCONTROL, timeout_ms, POWER_IF_LIST, list);
}

/* ========================================================================== */
/*  PMU Housekeeping                                                          */
/* ========================================================================== */

gs_error_t EPS_P80_Drv_PMU_GetHk(uint8_t csp_node, EPS_P80_Drv_PMU_HkTlm_t *hk, uint32_t timeout_ms)
{
    const uint8_t table_id = GS_P80_PMU_TELEMETRY_TABLE_MEM_ID;
    EPS_P80_Drv_PMU_HkTlm_t next_hk = {0};
    uint8_t out_en[6] = {0};
    int16_t out_i[6] = {0};
    uint8_t sm_en[8] = {0};
    int16_t temp[2] = {0};
    gs_error_t err;

    if (hk == NULL)
        return GS_ERROR_ARG;

    EPS_P80_RPARAM_GET_UINT32(table_id, GS_P80_PMU_TELEMETRY_UPTIME,       next_hk.uptime);
    EPS_P80_RPARAM_GET_UINT32(table_id, GS_P80_PMU_TELEMETRY_BOOTCAUSE,    next_hk.bootcause);
    EPS_P80_RPARAM_GET_UINT16(table_id, GS_P80_PMU_TELEMETRY_RESETCAUSE,   next_hk.resetcause);
    EPS_P80_RPARAM_GET_UINT16(table_id, GS_P80_PMU_TELEMETRY_BOOTCOUNT,    next_hk.bootcount);
    EPS_P80_RPARAM_GET_UINT16(table_id, GS_P80_PMU_TELEMETRY_BATT_V,       next_hk.batt_v);
    EPS_P80_RPARAM_GET_INT16 (table_id, GS_P80_PMU_TELEMETRY_BATT_I,       next_hk.batt_i);
    EPS_P80_RPARAM_GET_UINT8 (table_id, GS_P80_PMU_TELEMETRY_BATT_MODE,    next_hk.batt_mode);
    EPS_P80_RPARAM_GET_UINT16(table_id, GS_P80_PMU_TELEMETRY_VBAT_V,       next_hk.vbat_v);
    EPS_P80_RPARAM_GET_UINT16(table_id, GS_P80_PMU_TELEMETRY_VCC_V,        next_hk.vcc_v);
    EPS_P80_RPARAM_GET_UINT16(table_id, GS_P80_PMU_TELEMETRY_GND_WDT_CNT,  next_hk.gnd_wdt_cnt);
    EPS_P80_RPARAM_GET_UINT16(table_id, GS_P80_PMU_TELEMETRY_BUS_WDT_CNT,  next_hk.bus_wdt_cnt);
    EPS_P80_RPARAM_GET_UINT32(table_id, GS_P80_PMU_TELEMETRY_GND_WDT_LEFT, next_hk.gnd_wdt_left);
    EPS_P80_RPARAM_GET_UINT32(table_id, GS_P80_PMU_TELEMETRY_BUS_WDT_LEFT, next_hk.bus_wdt_left);

    EPS_P80_RPARAM_GET_ARRAY(table_id, GS_P80_PMU_TELEMETRY_TEMP(0),   GS_PARAM_INT16, temp,   2);
    EPS_P80_RPARAM_GET_ARRAY(table_id, GS_P80_PMU_TELEMETRY_OUT_EN(0), GS_PARAM_BOOL,  out_en, 6);
    EPS_P80_RPARAM_GET_ARRAY(table_id, GS_P80_PMU_TELEMETRY_OUT_I(0),  GS_PARAM_INT16, out_i,  6);
    EPS_P80_RPARAM_GET_ARRAY(table_id, GS_P80_PMU_TELEMETRY_SM_EN(0),  GS_PARAM_BOOL,  sm_en,  8);
    memcpy(next_hk.temp, temp, sizeof(next_hk.temp));
    memcpy(next_hk.out_en, out_en, sizeof(next_hk.out_en));
    memcpy(next_hk.out_i, out_i, sizeof(next_hk.out_i));
    memcpy(next_hk.sm_en, sm_en, sizeof(next_hk.sm_en));

    *hk = next_hk;

    return GS_OK;
}

/* ========================================================================== */
/*  PDU Housekeeping                                                          */
/* ========================================================================== */

gs_error_t EPS_P80_Drv_PDU_GetHk(uint8_t csp_node, EPS_P80_Drv_PDU_HkTlm_t *hk, uint32_t timeout_ms)
{
    const uint8_t table_id = GS_P80_PDU_TELEMETRY_TABLE_MEM_ID;
    EPS_P80_Drv_PDU_HkTlm_t next_hk = {0};
    uint8_t out_en[24] = {0};
    int16_t out_i[24] = {0};
    gs_error_t err;

    if (hk == NULL)
        return GS_ERROR_ARG;

    EPS_P80_RPARAM_GET_UINT32(table_id, GS_P80_PDU_TELEMETRY_UPTIME,       next_hk.uptime);
    EPS_P80_RPARAM_GET_UINT32(table_id, GS_P80_PDU_TELEMETRY_BOOTCAUSE,    next_hk.bootcause);
    EPS_P80_RPARAM_GET_UINT16(table_id, GS_P80_PDU_TELEMETRY_RESETCAUSE,   next_hk.resetcause);
    EPS_P80_RPARAM_GET_UINT32(table_id, GS_P80_PDU_TELEMETRY_BOOTCOUNT,    next_hk.bootcount);
    EPS_P80_RPARAM_GET_UINT16(table_id, GS_P80_PDU_TELEMETRY_VCC_V,        next_hk.vcc_v);
    EPS_P80_RPARAM_GET_UINT16(table_id, GS_P80_PDU_TELEMETRY_VCC_I,        next_hk.vcc_i);
    EPS_P80_RPARAM_GET_UINT16(table_id, GS_P80_PDU_TELEMETRY_VBAT_V,       next_hk.vbat_v);
    EPS_P80_RPARAM_GET_INT16 (table_id, GS_P80_PDU_TELEMETRY_TEMP,         next_hk.temp);
    EPS_P80_RPARAM_GET_UINT8 (table_id, GS_P80_PDU_TELEMETRY_BATT_MODE,    next_hk.batt_mode);
    EPS_P80_RPARAM_GET_UINT32(table_id, GS_P80_PDU_TELEMETRY_GND_WDT_CNT,  next_hk.gnd_wdt_cnt);
    EPS_P80_RPARAM_GET_UINT32(table_id, GS_P80_PDU_TELEMETRY_BUS_WDT_CNT,  next_hk.bus_wdt_cnt);
    EPS_P80_RPARAM_GET_UINT32(table_id, GS_P80_PDU_TELEMETRY_GND_WDT_LEFT, next_hk.gnd_wdt_left);
    EPS_P80_RPARAM_GET_UINT32(table_id, GS_P80_PDU_TELEMETRY_BUS_WDT_LEFT, next_hk.bus_wdt_left);

    EPS_P80_RPARAM_GET_ARRAY(table_id, GS_P80_PDU_TELEMETRY_OUT_EN(0), GS_PARAM_BOOL,  out_en, 24);
    EPS_P80_RPARAM_GET_ARRAY(table_id, GS_P80_PDU_TELEMETRY_OUT_I(0),  GS_PARAM_INT16, out_i,  24);
    memcpy(next_hk.out_en, out_en, sizeof(next_hk.out_en));
    memcpy(next_hk.out_i, out_i, sizeof(next_hk.out_i));

    *hk = next_hk;

    return GS_OK;
}

/* ========================================================================== */
/*  ACU Housekeeping (shared for ACU1 and ACU2)                               */
/* ========================================================================== */

gs_error_t EPS_P80_Drv_ACU_GetHk(uint8_t csp_node, EPS_P80_Drv_ACU_HkTlm_t *hk, uint32_t timeout_ms)
{
    const uint8_t table_id = GS_P80_ACU_TELEMETRY_TABLE_MEM_ID;
    EPS_P80_Drv_ACU_HkTlm_t next_hk = {0};
    int16_t input_i[6] = {0};
    uint16_t input_v[6] = {0};
    int16_t temp[3] = {0};
    gs_error_t err;

    if (hk == NULL)
        return GS_ERROR_ARG;

    EPS_P80_RPARAM_GET_UINT32(table_id, GS_P80_ACU_TELEMETRY_UPTIME,       next_hk.uptime);
    EPS_P80_RPARAM_GET_UINT32(table_id, GS_P80_ACU_TELEMETRY_BOOTCAUSE,    next_hk.bootcause);
    EPS_P80_RPARAM_GET_UINT16(table_id, GS_P80_ACU_TELEMETRY_RESETCAUSE,   next_hk.resetcause);
    EPS_P80_RPARAM_GET_UINT32(table_id, GS_P80_ACU_TELEMETRY_BOOTCOUNT,    next_hk.bootcount);
    EPS_P80_RPARAM_GET_UINT16(table_id, GS_P80_ACU_TELEMETRY_VCC_V,        next_hk.vcc_v);
    EPS_P80_RPARAM_GET_UINT16(table_id, GS_P80_ACU_TELEMETRY_VBAT_V,       next_hk.vbat_v);
    EPS_P80_RPARAM_GET_UINT8 (table_id, GS_P80_ACU_TELEMETRY_MPPT_MODE,    next_hk.mppt_mode);
    EPS_P80_RPARAM_GET_UINT32(table_id, GS_P80_ACU_TELEMETRY_GND_WDT_CNT,  next_hk.gnd_wdt_cnt);
    EPS_P80_RPARAM_GET_UINT32(table_id, GS_P80_ACU_TELEMETRY_GND_WDT_LEFT, next_hk.gnd_wdt_left);

    EPS_P80_RPARAM_GET_ARRAY(table_id, GS_P80_ACU_TELEMETRY_INPUT_I(0), GS_PARAM_INT16,  input_i, 6);
    EPS_P80_RPARAM_GET_ARRAY(table_id, GS_P80_ACU_TELEMETRY_INPUT_V(0), GS_PARAM_UINT16, input_v, 6);
    EPS_P80_RPARAM_GET_ARRAY(table_id, GS_P80_ACU_TELEMETRY_TEMP(0),    GS_PARAM_INT16,  temp,    3);
    memcpy(next_hk.input_i, input_i, sizeof(next_hk.input_i));
    memcpy(next_hk.input_v, input_v, sizeof(next_hk.input_v));
    memcpy(next_hk.temp, temp, sizeof(next_hk.temp));

    *hk = next_hk;

    return GS_OK;
}

/* ========================================================================== */
/*  Ground Watchdog Clear                                                     */
/* ========================================================================== */

gs_error_t EPS_P80_Drv_PMU_GndWdtClear(uint8_t csp_node, uint32_t timeout_ms)
{
    return p80_pmu_gndwdt_clear(csp_node, timeout_ms);
}

gs_error_t EPS_P80_Drv_PDU_GndWdtClear(uint8_t csp_node, uint32_t timeout_ms)
{
    return p80_pdu_gndwdt_clear(csp_node, timeout_ms);
}

gs_error_t EPS_P80_Drv_ACU_GndWdtClear(uint8_t csp_node, uint32_t timeout_ms)
{
    return p80_acu_gndwdt_clear(csp_node, timeout_ms);
}

/* ========================================================================== */
/*  Remote Parameter Commands                                                 */
/* ========================================================================== */

gs_error_t EPS_P80_Drv_ParamSet(uint8_t csp_node, uint8_t table_id,
                                 uint16_t addr, uint8_t type,
                                 const uint8_t *data, uint16_t size,
                                 uint32_t timeout_ms)
{
    return gs_rparam_set(csp_node, table_id, addr, type,
                         GS_RPARAM_MAGIC_CHECKSUM, timeout_ms,
                         data, size);
}

gs_error_t EPS_P80_Drv_ParamGet(uint8_t csp_node, uint8_t table_id,
                                 uint16_t addr, uint8_t type,
                                 uint8_t *data, uint16_t size,
                                 uint32_t timeout_ms)
{
    return gs_rparam_get(csp_node, table_id, addr, type,
                         GS_RPARAM_MAGIC_CHECKSUM, timeout_ms,
                         (void *)data, size);
}

gs_error_t EPS_P80_Drv_GetFullTable(uint8_t csp_node, uint8_t table_id,
                                     gs_param_table_instance_t *tinst, uint32_t timeout_ms)
{
    uint16_t checksum;

    if (tinst == NULL)
        return GS_ERROR_ARG;

    memset(tinst, 0, sizeof(*tinst));

    gs_error_t result = gs_rparam_download_table_spec(tinst, NULL, csp_node, table_id, timeout_ms, &checksum);
    if (result)
    {
        gs_param_table_free(tinst);
        return result;
    }

    gs_error_t err = gs_rparam_get_full_table(tinst, csp_node, table_id, checksum, timeout_ms);
    if (err != GS_OK)
    {
        gs_param_table_free(tinst);
        return err;
    }

    return GS_OK;
}

/* ========================================================================== */
/*  Table Save/Load                                                           */
/* ========================================================================== */

gs_error_t EPS_P80_Drv_TableSave(uint8_t csp_node, uint8_t table_id, uint32_t timeout_ms)
{
    return gs_rparam_save(csp_node, timeout_ms, table_id, 0);
}

gs_error_t EPS_P80_Drv_TableLoad(uint8_t csp_node, uint8_t table_id, uint32_t timeout_ms)
{
    return gs_rparam_load(csp_node, timeout_ms, 0, table_id);
}

gs_error_t EPS_P80_Drv_ParamSaveAll(uint8_t csp_node, uint32_t timeout_ms)
{
    return gs_rparam_save(csp_node, timeout_ms, 0xFF, 0);
}

/* ========================================================================== */
/*  Beacon Data Fetch                                                         */
/* ========================================================================== */

gs_error_t EPS_P80_Drv_PMU_GetBcn(uint8_t csp_node, EPS_P80_Drv_PMU_BcnTlm_t *bcn, uint32_t timeout_ms)
{
    const uint8_t table_id = GS_P80_PMU_TELEMETRY_TABLE_MEM_ID;
    EPS_P80_Drv_PMU_BcnTlm_t next_bcn = {0};
    uint8_t out_en[6] = {0};
    int16_t temp[2] = {0};
    uint8_t sm_en[8] = {0};
    gs_error_t err;

    if (bcn == NULL)
        return GS_ERROR_ARG;

    EPS_P80_RPARAM_GET_UINT32(table_id, GS_P80_PMU_TELEMETRY_BOOTCAUSE,  next_bcn.bootcause);
    EPS_P80_RPARAM_GET_UINT16(table_id, GS_P80_PMU_TELEMETRY_RESETCAUSE, next_bcn.resetcause);
    EPS_P80_RPARAM_GET_UINT16(table_id, GS_P80_PMU_TELEMETRY_BOOTCOUNT,  next_bcn.bootcount);
    EPS_P80_RPARAM_GET_UINT8 (table_id, GS_P80_PMU_TELEMETRY_BATT_MODE, next_bcn.batt_mode);
    EPS_P80_RPARAM_GET_INT16 (table_id, GS_P80_PMU_TELEMETRY_BATT_I,    next_bcn.batt_i);
    EPS_P80_RPARAM_GET_UINT16(table_id, GS_P80_PMU_TELEMETRY_BATT_V,    next_bcn.batt_v);
    EPS_P80_RPARAM_GET_UINT16(table_id, GS_P80_PMU_TELEMETRY_GND_WDT_CNT,  next_bcn.gnd_wdt_cnt);
    EPS_P80_RPARAM_GET_UINT16(table_id, GS_P80_PMU_TELEMETRY_BUS_WDT_CNT,  next_bcn.bus_wdt_cnt);
    EPS_P80_RPARAM_GET_UINT32(table_id, GS_P80_PMU_TELEMETRY_GND_WDT_LEFT, next_bcn.gnd_wdt_left);
    EPS_P80_RPARAM_GET_UINT32(table_id, GS_P80_PMU_TELEMETRY_BUS_WDT_LEFT, next_bcn.bus_wdt_left);
    EPS_P80_RPARAM_GET_ARRAY(table_id, GS_P80_PMU_TELEMETRY_OUT_EN(0), GS_PARAM_BOOL,  out_en, 6);
    EPS_P80_RPARAM_GET_ARRAY(table_id, GS_P80_PMU_TELEMETRY_TEMP(0),   GS_PARAM_INT16, temp,   2);
    EPS_P80_RPARAM_GET_ARRAY(table_id, GS_P80_PMU_TELEMETRY_SM_EN(0),  GS_PARAM_BOOL,  sm_en,  8);
    memcpy(next_bcn.out_en, out_en, sizeof(next_bcn.out_en));
    memcpy(next_bcn.temp, temp, sizeof(next_bcn.temp));
    next_bcn.sm_en_mask = EPS_P80_DrvPackBoolArray8(sm_en);

    *bcn = next_bcn;

    return GS_OK;
}

gs_error_t EPS_P80_Drv_PDU_GetBcn(uint8_t csp_node, EPS_P80_Drv_PDU_BcnTlm_t *bcn, uint32_t timeout_ms)
{
    const uint8_t table_id = GS_P80_PDU_TELEMETRY_TABLE_MEM_ID;
    static const uint8_t PduBcnChannels[EPS_P80_DRV_PDU_BCN_USED_CH_COUNT] =
        EPS_P80_DRV_PDU_BCN_USED_CH_LIST;
    EPS_P80_Drv_PDU_BcnTlm_t next_bcn = {0};
    uint8_t out_en[GS_P80_PDU_TELEMETRY_OUT_EN_ARRAY_SIZE] = {0};
    int16_t out_i[GS_P80_PDU_TELEMETRY_OUT_I_ARRAY_SIZE] = {0};
    gs_error_t err;

    if (bcn == NULL)
        return GS_ERROR_ARG;

    EPS_P80_RPARAM_GET_ARRAY(table_id, GS_P80_PDU_TELEMETRY_OUT_EN(0), GS_PARAM_BOOL, out_en,
                             GS_P80_PDU_TELEMETRY_OUT_EN_ARRAY_SIZE);
    EPS_P80_RPARAM_GET_ARRAY(table_id, GS_P80_PDU_TELEMETRY_OUT_I(0), GS_PARAM_INT16, out_i,
                             GS_P80_PDU_TELEMETRY_OUT_I_ARRAY_SIZE);

    for (uint8_t i = 0; i < EPS_P80_DRV_PDU_BCN_USED_CH_COUNT; i++)
    {
        uint8_t channel = PduBcnChannels[i];

        next_bcn.out_en[i] = out_en[channel];
        next_bcn.out_i[i] = out_i[channel];
    }

    *bcn = next_bcn;

    return GS_OK;
}

gs_error_t EPS_P80_Drv_ACU_GetBcn(uint8_t csp_node, EPS_P80_Drv_ACU_BcnTlm_t *bcn, uint32_t timeout_ms)
{
    const uint8_t table_id = GS_P80_ACU_TELEMETRY_TABLE_MEM_ID;
    EPS_P80_Drv_ACU_BcnTlm_t next_bcn = {0};
    int16_t input_i[6] = {0};
    uint16_t input_v[6] = {0};
    gs_error_t err;

    if (bcn == NULL)
        return GS_ERROR_ARG;

    EPS_P80_RPARAM_GET_UINT8(table_id, GS_P80_ACU_TELEMETRY_MPPT_MODE, next_bcn.mppt_mode);
    EPS_P80_RPARAM_GET_ARRAY(table_id, GS_P80_ACU_TELEMETRY_INPUT_I(0), GS_PARAM_INT16,  input_i, 6);
    EPS_P80_RPARAM_GET_ARRAY(table_id, GS_P80_ACU_TELEMETRY_INPUT_V(0), GS_PARAM_UINT16, input_v, 6);
    memcpy(next_bcn.input_i, input_i, sizeof(next_bcn.input_i));
    memcpy(next_bcn.input_v, input_v, sizeof(next_bcn.input_v));

    *bcn = next_bcn;

    return GS_OK;
}
