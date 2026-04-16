/**
 * @file
 *   EPS P80 (NanoPower) Device Driver Implementation
 */

#include "eps_p80_drv.h"

#include <gs/param/internal/types.h>
#include <gs/param/rparam.h>
#include <gs/param/table.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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
                                   uint8_t mode, uint8_t on_cnt, uint8_t off_cnt,
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
    gs_param_table_instance_t tinst = {0};

    gs_error_t err = p80_pmu_get_hk(&tinst, csp_node, timeout_ms);
    if (err != GS_OK)
    {
        gs_param_table_free(&tinst);
        return err;
    }

    gs_error_t perr = GS_OK;

    EPS_PARAM_GET_UINT32_FIELD(&tinst, GS_P80_PMU_TELEMETRY_UPTIME,       hk->uptime);
    EPS_PARAM_GET_UINT32_FIELD(&tinst, GS_P80_PMU_TELEMETRY_BOOTCAUSE,    hk->bootcause);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, GS_P80_PMU_TELEMETRY_RESETCAUSE,   hk->resetcause);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, GS_P80_PMU_TELEMETRY_BOOTCOUNT,    hk->bootcount);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, GS_P80_PMU_TELEMETRY_BATT_V,       hk->batt_v);
    EPS_PARAM_GET_INT16_FIELD (&tinst, GS_P80_PMU_TELEMETRY_BATT_I,       hk->batt_i);
    EPS_PARAM_GET_UINT8_FIELD (&tinst, GS_P80_PMU_TELEMETRY_BATT_MODE,    hk->batt_mode);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, GS_P80_PMU_TELEMETRY_VBAT_V,       hk->vbat_v);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, GS_P80_PMU_TELEMETRY_VCC_V,        hk->vcc_v);
    EPS_PARAM_GET_INT16_FIELD (&tinst, GS_P80_PMU_TELEMETRY_TEMP(0),      hk->temp[0]);
    EPS_PARAM_GET_INT16_FIELD (&tinst, GS_P80_PMU_TELEMETRY_TEMP(1),      hk->temp[1]);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, GS_P80_PMU_TELEMETRY_GND_WDT_CNT,  hk->gnd_wdt_cnt);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, GS_P80_PMU_TELEMETRY_BUS_WDT_CNT,  hk->bus_wdt_cnt);
    EPS_PARAM_GET_UINT32_FIELD(&tinst, GS_P80_PMU_TELEMETRY_GND_WDT_LEFT, hk->gnd_wdt_left);
    EPS_PARAM_GET_UINT32_FIELD(&tinst, GS_P80_PMU_TELEMETRY_BUS_WDT_LEFT, hk->bus_wdt_left);

    for (int i = 0; i < 6; i++)
    {
        bool en = false;
        perr |= gs_param_get_bool (&tinst, GS_P80_PMU_TELEMETRY_OUT_EN(i), &en, 0);
        hk->out_en[i] = (uint8_t)en;
        EPS_PARAM_GET_INT16_FIELD(&tinst, GS_P80_PMU_TELEMETRY_OUT_I(i), hk->out_i[i]);
    }

    for (int i = 0; i < 8; i++)
    {
        bool en = false;
        perr |= gs_param_get_bool(&tinst, GS_P80_PMU_TELEMETRY_SM_EN(i), &en, 0);
        hk->sm_en[i] = (uint8_t)en;
    }

    if (perr != GS_OK)
    {
        gs_param_table_free(&tinst);
        return perr;
    }

    gs_param_table_free(&tinst);

    return GS_OK;
}

/* ========================================================================== */
/*  PDU Housekeeping                                                          */
/* ========================================================================== */

gs_error_t EPS_P80_Drv_PDU_GetHk(uint8_t csp_node, EPS_P80_Drv_PDU_HkTlm_t *hk, uint32_t timeout_ms)
{
    gs_param_table_instance_t tinst = {0};

    gs_error_t err = p80_pdu_get_hk(&tinst, csp_node, timeout_ms);
    if (err != GS_OK)
    {
        gs_param_table_free(&tinst);
        return err;
    }

    gs_error_t perr = GS_OK;

    EPS_PARAM_GET_UINT32_FIELD(&tinst, GS_P80_PDU_TELEMETRY_UPTIME,       hk->uptime);
    EPS_PARAM_GET_UINT32_FIELD(&tinst, GS_P80_PDU_TELEMETRY_BOOTCAUSE,    hk->bootcause);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, GS_P80_PDU_TELEMETRY_RESETCAUSE,   hk->resetcause);
    EPS_PARAM_GET_UINT32_FIELD(&tinst, GS_P80_PDU_TELEMETRY_BOOTCOUNT,    hk->bootcount);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, GS_P80_PDU_TELEMETRY_VCC_V,        hk->vcc_v);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, GS_P80_PDU_TELEMETRY_VCC_I,        hk->vcc_i);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, GS_P80_PDU_TELEMETRY_VBAT_V,       hk->vbat_v);
    EPS_PARAM_GET_INT16_FIELD (&tinst, GS_P80_PDU_TELEMETRY_TEMP,         hk->temp);
    EPS_PARAM_GET_UINT8_FIELD (&tinst, GS_P80_PDU_TELEMETRY_BATT_MODE,    hk->batt_mode);
    EPS_PARAM_GET_UINT32_FIELD(&tinst, GS_P80_PDU_TELEMETRY_GND_WDT_CNT,  hk->gnd_wdt_cnt);
    EPS_PARAM_GET_UINT32_FIELD(&tinst, GS_P80_PDU_TELEMETRY_BUS_WDT_CNT,  hk->bus_wdt_cnt);
    EPS_PARAM_GET_UINT32_FIELD(&tinst, GS_P80_PDU_TELEMETRY_GND_WDT_LEFT, hk->gnd_wdt_left);
    EPS_PARAM_GET_UINT32_FIELD(&tinst, GS_P80_PDU_TELEMETRY_BUS_WDT_LEFT, hk->bus_wdt_left);

    for (int i = 0; i < 24; i++)
    {
        bool en = false;
        perr |= gs_param_get_bool (&tinst, GS_P80_PDU_TELEMETRY_OUT_EN(i), &en, 0);
        hk->out_en[i] = (uint8_t)en;
        EPS_PARAM_GET_INT16_FIELD(&tinst, GS_P80_PDU_TELEMETRY_OUT_I(i), hk->out_i[i]);
    }

    if (perr != GS_OK)
    {
        gs_param_table_free(&tinst);
        return perr;
    }

    gs_param_table_free(&tinst);

    return GS_OK;
}

/* ========================================================================== */
/*  ACU Housekeeping (shared for ACU1 and ACU2)                               */
/* ========================================================================== */

gs_error_t EPS_P80_Drv_ACU_GetHk(uint8_t csp_node, EPS_P80_Drv_ACU_HkTlm_t *hk, uint32_t timeout_ms)
{
    gs_param_table_instance_t tinst = {0};

    gs_error_t err = p80_acu_get_hk(&tinst, csp_node, timeout_ms);
    if (err != GS_OK)
    {
        gs_param_table_free(&tinst);
        return err;
    }

    gs_error_t perr = GS_OK;

    EPS_PARAM_GET_UINT32_FIELD(&tinst, GS_P80_ACU_TELEMETRY_UPTIME,       hk->uptime);
    EPS_PARAM_GET_UINT32_FIELD(&tinst, GS_P80_ACU_TELEMETRY_BOOTCAUSE,    hk->bootcause);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, GS_P80_ACU_TELEMETRY_RESETCAUSE,   hk->resetcause);
    EPS_PARAM_GET_UINT32_FIELD(&tinst, GS_P80_ACU_TELEMETRY_BOOTCOUNT,    hk->bootcount);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, GS_P80_ACU_TELEMETRY_VCC_V,        hk->vcc_v);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, GS_P80_ACU_TELEMETRY_VBAT_V,       hk->vbat_v);
    EPS_PARAM_GET_INT16_FIELD (&tinst, GS_P80_ACU_TELEMETRY_TEMP(0),      hk->temp[0]);
    EPS_PARAM_GET_INT16_FIELD (&tinst, GS_P80_ACU_TELEMETRY_TEMP(1),      hk->temp[1]);
    EPS_PARAM_GET_INT16_FIELD (&tinst, GS_P80_ACU_TELEMETRY_TEMP(2),      hk->temp[2]);
    EPS_PARAM_GET_UINT8_FIELD (&tinst, GS_P80_ACU_TELEMETRY_MPPT_MODE,    hk->mppt_mode);
    EPS_PARAM_GET_UINT32_FIELD(&tinst, GS_P80_ACU_TELEMETRY_GND_WDT_CNT,  hk->gnd_wdt_cnt);
    EPS_PARAM_GET_UINT32_FIELD(&tinst, GS_P80_ACU_TELEMETRY_GND_WDT_LEFT, hk->gnd_wdt_left);

    for (int i = 0; i < 6; i++)
    {
        EPS_PARAM_GET_INT16_FIELD (&tinst, GS_P80_ACU_TELEMETRY_INPUT_I(i), hk->input_i[i]);
        EPS_PARAM_GET_UINT16_FIELD(&tinst, GS_P80_ACU_TELEMETRY_INPUT_V(i), hk->input_v[i]);
    }

    if (perr != GS_OK)
    {
        gs_param_table_free(&tinst);
        return perr;
    }

    gs_param_table_free(&tinst);

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
    gs_param_table_instance_t tinst = {0};

    gs_error_t err = p80_pmu_get_hk(&tinst, csp_node, timeout_ms);
    if (err != GS_OK)
    {
        gs_param_table_free(&tinst);
        return err;
    }

    gs_error_t perr = GS_OK;

    EPS_PARAM_GET_UINT32_FIELD(&tinst, GS_P80_PMU_TELEMETRY_BOOTCAUSE, bcn->bootcause);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, GS_P80_PMU_TELEMETRY_RESETCAUSE, bcn->resetcause);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, GS_P80_PMU_TELEMETRY_BOOTCOUNT,  bcn->bootcount);

    for (int i = 0; i < 6; i++)
    {
        bool en = false;
        perr |= gs_param_get_bool(&tinst, GS_P80_PMU_TELEMETRY_OUT_EN(i), &en, 0);
        bcn->out_en[i] = (uint8_t)en;
    }

    EPS_PARAM_GET_INT16_FIELD(&tinst, GS_P80_PMU_TELEMETRY_TEMP(0), bcn->temp[0]);
    EPS_PARAM_GET_INT16_FIELD(&tinst, GS_P80_PMU_TELEMETRY_TEMP(1), bcn->temp[1]);

    EPS_PARAM_GET_UINT8_FIELD (&tinst, GS_P80_PMU_TELEMETRY_BATT_MODE, bcn->batt_mode);
    EPS_PARAM_GET_INT16_FIELD (&tinst, GS_P80_PMU_TELEMETRY_BATT_I,    bcn->batt_i);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, GS_P80_PMU_TELEMETRY_BATT_V,    bcn->batt_v);

    for (int i = 0; i < 8; i++)
    {
        bool en = false;
        perr |= gs_param_get_bool(&tinst, GS_P80_PMU_TELEMETRY_SM_EN(i), &en, 0);
        bcn->sm_en[i] = (uint8_t)en;
    }

    EPS_PARAM_GET_UINT16_FIELD(&tinst, GS_P80_PMU_TELEMETRY_GND_WDT_CNT,  bcn->gnd_wdt_cnt);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, GS_P80_PMU_TELEMETRY_BUS_WDT_CNT,  bcn->bus_wdt_cnt);
    EPS_PARAM_GET_UINT32_FIELD(&tinst, GS_P80_PMU_TELEMETRY_GND_WDT_LEFT, bcn->gnd_wdt_left);
    EPS_PARAM_GET_UINT32_FIELD(&tinst, GS_P80_PMU_TELEMETRY_BUS_WDT_LEFT, bcn->bus_wdt_left);

    gs_param_table_free(&tinst);

    return perr;
}

gs_error_t EPS_P80_Drv_PDU_GetBcn(uint8_t csp_node, EPS_P80_Drv_PDU_BcnTlm_t *bcn, uint32_t timeout_ms)
{
    gs_param_table_instance_t tinst = {0};

    gs_error_t err = p80_pdu_get_hk(&tinst, csp_node, timeout_ms);
    if (err != GS_OK)
    {
        gs_param_table_free(&tinst);
        return err;
    }

    gs_error_t perr = GS_OK;

    for (int i = 0; i < 24; i++)
    {
        bool en = false;
        perr |= gs_param_get_bool(&tinst, GS_P80_PDU_TELEMETRY_OUT_EN(i), &en, 0);
        bcn->out_en[i] = (uint8_t)en;
    }

    gs_param_table_free(&tinst);

    return perr;
}

gs_error_t EPS_P80_Drv_ACU_GetBcn(uint8_t csp_node, EPS_P80_Drv_ACU_BcnTlm_t *bcn, uint32_t timeout_ms)
{
    gs_param_table_instance_t tinst = {0};

    gs_error_t err = p80_acu_get_hk(&tinst, csp_node, timeout_ms);
    if (err != GS_OK)
    {
        gs_param_table_free(&tinst);
        return err;
    }

    gs_error_t perr = GS_OK;

    for (int i = 0; i < 6; i++)
    {
        EPS_PARAM_GET_INT16_FIELD (&tinst, GS_P80_ACU_TELEMETRY_INPUT_I(i), bcn->input_i[i]);
        EPS_PARAM_GET_UINT16_FIELD(&tinst, GS_P80_ACU_TELEMETRY_INPUT_V(i), bcn->input_v[i]);
    }

    EPS_PARAM_GET_UINT8_FIELD(&tinst, GS_P80_ACU_TELEMETRY_MPPT_MODE, bcn->mppt_mode);

    gs_param_table_free(&tinst);

    return perr;
}
