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
#include "osapi.h"

/* ========================================================================== */
/*  Power Interface Commands                                                  */
/* ========================================================================== */

gs_error_t EPS_P80_Drv_PowerIfGet(uint8_t csp_node, const char *name, uint32_t timeout_ms)
{
    power_if_ch_status_t status = {0};

    strncpy(status.name, name, POWER_IF_NAME_LEN - 1);
    status.name[POWER_IF_NAME_LEN - 1] = '\0';

    gs_error_t err = power_if_cmd(csp_node, GS_P80_PORT_CMDCONTROL, timeout_ms, POWER_IF_GET, &status);
    if (err != GS_OK)
        return err;

    OS_printf("EPS: Power Interface Get command succeeded\n");
    OS_printf(" ch_idx = %u\n", status.ch_idx);
    OS_printf(" name = %s\n", status.name);
    OS_printf(" mode = %u\n", status.mode);
    OS_printf("  on_cnt:     %u\n", status.on_cnt);
    OS_printf("  off_cnt:    %u\n", status.off_cnt);
    OS_printf("  cur_lu_lim: %u\n", status.cur_lu_lim);
    OS_printf("  cur_lim:    %u\n", status.cur_lim);
    OS_printf("  voltage:    %u mV\n", status.voltage);
    OS_printf("  current:    %d mA\n", status.current);
    OS_printf("  latchup:    %u\n", status.latchup);

    return GS_OK;
}

gs_error_t EPS_P80_Drv_PowerIfSet(uint8_t csp_node, const char *name,
                                   uint8_t mode, uint8_t on_cnt, uint8_t off_cnt,
                                   uint32_t timeout_ms)
{
    power_if_ch_status_t status = {0};

    status.mode = mode;
    status.on_cnt = on_cnt;
    status.off_cnt = off_cnt;
    strncpy(status.name, name, POWER_IF_NAME_LEN - 1);
    status.name[POWER_IF_NAME_LEN - 1] = 0;

    gs_error_t err = power_if_cmd(csp_node, GS_P80_PORT_CMDCONTROL, timeout_ms, POWER_IF_SET, &status);
    if (err != GS_OK)
        return err;

    OS_printf("EPS Power_If_Set Response Succeeded\n");
    OS_printf("  ch_idx:     %u\n", status.ch_idx);
    OS_printf("  name:       %s\n", status.name);
    OS_printf("  mode:       %u\n", status.mode);
    OS_printf("  on_cnt:     %u\n", status.on_cnt);
    OS_printf("  off_cnt:    %u\n", status.off_cnt);
    OS_printf("  cur_lu_lim: %u\n", status.cur_lu_lim);
    OS_printf("  cur_lim:    %u\n", status.cur_lim);
    OS_printf("  voltage:    %u mV\n", status.voltage);
    OS_printf("  current:    %d mA\n", status.current);
    OS_printf("  latchup:    %u\n", status.latchup);

    return GS_OK;
}

gs_error_t EPS_P80_Drv_PowerIfList(uint8_t csp_node, uint32_t timeout_ms)
{
    power_if_cmd_list_response_t list = {0};

    gs_error_t err = power_if_cmd(csp_node, GS_P80_PORT_CMDCONTROL, timeout_ms, POWER_IF_LIST, &list);
    if (err != GS_OK)
        return err;

    OS_printf("EPS Power_If_List Response:\n");
    OS_printf("  cmd:    %u\n", list.cmd);
    OS_printf("  status: %u\n", list.status);
    OS_printf("  count:  %u\n", list.count);
    OS_printf("  %-6s %-8s %s\n", "ch_idx", "mode", "name");
    for (uint8_t i = 0; i < list.count && i < 24; i++)
    {
        OS_printf("  %-6u %-8u %s\n",
                  list.list[i].ch_idx,
                  list.list[i].mode,
                  list.list[i].name);
    }

    return GS_OK;
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

    perr |= gs_param_get_uint32(&tinst, GS_P80_PMU_TELEMETRY_UPTIME,      &hk->uptime,       0);
    perr |= gs_param_get_uint32(&tinst, GS_P80_PMU_TELEMETRY_BOOTCAUSE,   &hk->bootcause,    0);
    perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_RESETCAUSE,  &hk->resetcause,   0);
    perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_BOOTCOUNT,   &hk->bootcount,    0);
    perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_BATT_V,      &hk->batt_v,       0);
    perr |= gs_param_get_int16 (&tinst, GS_P80_PMU_TELEMETRY_BATT_I,      &hk->batt_i,       0);
    perr |= gs_param_get_uint8 (&tinst, GS_P80_PMU_TELEMETRY_BATT_MODE,   &hk->batt_mode,    0);
    perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_VBAT_V,      &hk->vbat_v,       0);
    perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_VCC_V,       &hk->vcc_v,        0);
    perr |= gs_param_get_int16 (&tinst, GS_P80_PMU_TELEMETRY_TEMP(0),     &hk->temp[0],      0);
    perr |= gs_param_get_int16 (&tinst, GS_P80_PMU_TELEMETRY_TEMP(1),     &hk->temp[1],      0);
    perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_GND_WDT_CNT, &hk->gnd_wdt_cnt,  0);
    perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_BUS_WDT_CNT, &hk->bus_wdt_cnt,  0);
    perr |= gs_param_get_uint32(&tinst, GS_P80_PMU_TELEMETRY_GND_WDT_LEFT,&hk->gnd_wdt_left, 0);
    perr |= gs_param_get_uint32(&tinst, GS_P80_PMU_TELEMETRY_BUS_WDT_LEFT,&hk->bus_wdt_left, 0);

    for (int i = 0; i < 6; i++)
    {
        bool en = false;
        perr |= gs_param_get_bool (&tinst, GS_P80_PMU_TELEMETRY_OUT_EN(i), &en, 0);
        hk->out_en[i] = (uint8_t)en;
        perr |= gs_param_get_int16(&tinst, GS_P80_PMU_TELEMETRY_OUT_I(i),  &hk->out_i[i], 0);
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

    OS_printf("\n================[p80 PMU HK]===================\n");
    OS_printf("[SYSTEM] Uptime: %u s | BootCount: %u | Cause(Boot/Reset): %u / %u\n",
              hk->uptime, hk->bootcount, hk->bootcause, hk->resetcause);
    OS_printf("[OUTPUT] IDX | EN | Curr(mA)\n");
    for (int i = 0; i < 6; i++)
        OS_printf("         %3d | %2s | %8d\n", i, hk->out_en[i] ? "ON" : "OFF", hk->out_i[i]);
    OS_printf("[BATT]   Volt: %u mV | Curr: %d mA | Mode: %u\n", hk->batt_v, hk->batt_i, hk->batt_mode);
    OS_printf("[PMU]    VBAT: %u mV | VCC: %u mV\n", hk->vbat_v, hk->vcc_v);
    OS_printf("[TEMP]   T0: %d | T1: %d (deci-degC)\n", hk->temp[0], hk->temp[1]);
    OS_printf("[SUBMOD] Enable: ");
    for (int i = 0; i < 8; i++) OS_printf("%d ", hk->sm_en[i]);
    OS_printf("\n");
    OS_printf("[WDT]    GND: %u (Left: %u s) | BUS: %u (Left: %u s)\n",
              hk->gnd_wdt_cnt, hk->gnd_wdt_left, hk->bus_wdt_cnt, hk->bus_wdt_left);
    OS_printf("=======================================================\n");

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

    perr |= gs_param_get_uint32(&tinst, GS_P80_PDU_TELEMETRY_UPTIME,       &hk->uptime,       0);
    perr |= gs_param_get_uint32(&tinst, GS_P80_PDU_TELEMETRY_BOOTCAUSE,    &hk->bootcause,    0);
    perr |= gs_param_get_uint16(&tinst, GS_P80_PDU_TELEMETRY_RESETCAUSE,   &hk->resetcause,   0);
    perr |= gs_param_get_uint32(&tinst, GS_P80_PDU_TELEMETRY_BOOTCOUNT,    &hk->bootcount,    0);
    perr |= gs_param_get_uint16(&tinst, GS_P80_PDU_TELEMETRY_VCC_V,        &hk->vcc_v,        0);
    perr |= gs_param_get_uint16(&tinst, GS_P80_PDU_TELEMETRY_VCC_I,        &hk->vcc_i,        0);
    perr |= gs_param_get_uint16(&tinst, GS_P80_PDU_TELEMETRY_VBAT_V,       &hk->vbat_v,       0);
    perr |= gs_param_get_int16 (&tinst, GS_P80_PDU_TELEMETRY_TEMP,         &hk->temp,         0);
    perr |= gs_param_get_uint8 (&tinst, GS_P80_PDU_TELEMETRY_BATT_MODE,    &hk->batt_mode,    0);
    perr |= gs_param_get_uint32(&tinst, GS_P80_PDU_TELEMETRY_GND_WDT_CNT,  &hk->gnd_wdt_cnt,  0);
    perr |= gs_param_get_uint32(&tinst, GS_P80_PDU_TELEMETRY_BUS_WDT_CNT,  &hk->bus_wdt_cnt,  0);
    perr |= gs_param_get_uint32(&tinst, GS_P80_PDU_TELEMETRY_GND_WDT_LEFT, &hk->gnd_wdt_left, 0);
    perr |= gs_param_get_uint32(&tinst, GS_P80_PDU_TELEMETRY_BUS_WDT_LEFT, &hk->bus_wdt_left, 0);

    for (int i = 0; i < 24; i++)
    {
        bool en = false;
        perr |= gs_param_get_bool (&tinst, GS_P80_PDU_TELEMETRY_OUT_EN(i), &en, 0);
        hk->out_en[i] = (uint8_t)en;
        perr |= gs_param_get_int16(&tinst, GS_P80_PDU_TELEMETRY_OUT_I(i),  &hk->out_i[i], 0);
    }

    if (perr != GS_OK)
    {
        gs_param_table_free(&tinst);
        return perr;
    }

    OS_printf("\n================[p80 PDU HK]===================\n");
    OS_printf("[SYSTEM] Uptime: %u s | BootCount: %u | Cause(Boot/Reset): %u / %u\n",
              hk->uptime, hk->bootcount, hk->bootcause, hk->resetcause);
    OS_printf("[PDU]    VCC: %u mV (%u mA) | VBAT: %u mV\n", hk->vcc_v, hk->vcc_i, hk->vbat_v);
    OS_printf("[TEMP]   %d (deci-degC) | BattMode: %u\n", hk->temp, hk->batt_mode);
    OS_printf("[WDT]    GND: %u (Left: %u s) | BUS: %u (Left: %u s)\n",
              hk->gnd_wdt_cnt, hk->gnd_wdt_left, hk->bus_wdt_cnt, hk->bus_wdt_left);
    OS_printf("=======================================================\n");

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

    perr |= gs_param_get_uint32(&tinst, GS_P80_ACU_TELEMETRY_UPTIME,      &hk->uptime,      0);
    perr |= gs_param_get_uint32(&tinst, GS_P80_ACU_TELEMETRY_BOOTCAUSE,   &hk->bootcause,   0);
    perr |= gs_param_get_uint16(&tinst, GS_P80_ACU_TELEMETRY_RESETCAUSE,  &hk->resetcause,  0);
    perr |= gs_param_get_uint32(&tinst, GS_P80_ACU_TELEMETRY_BOOTCOUNT,   &hk->bootcount,   0);
    perr |= gs_param_get_uint16(&tinst, GS_P80_ACU_TELEMETRY_VCC_V,       &hk->vcc_v,       0);
    perr |= gs_param_get_uint16(&tinst, GS_P80_ACU_TELEMETRY_VBAT_V,      &hk->vbat_v,      0);
    perr |= gs_param_get_int16 (&tinst, GS_P80_ACU_TELEMETRY_TEMP(0),     &hk->temp[0],     0);
    perr |= gs_param_get_int16 (&tinst, GS_P80_ACU_TELEMETRY_TEMP(1),     &hk->temp[1],     0);
    perr |= gs_param_get_int16 (&tinst, GS_P80_ACU_TELEMETRY_TEMP(2),     &hk->temp[2],     0);
    perr |= gs_param_get_uint8 (&tinst, GS_P80_ACU_TELEMETRY_MPPT_MODE,   &hk->mppt_mode,   0);
    perr |= gs_param_get_uint32(&tinst, GS_P80_ACU_TELEMETRY_GND_WDT_CNT, &hk->gnd_wdt_cnt, 0);
    perr |= gs_param_get_uint32(&tinst, GS_P80_ACU_TELEMETRY_GND_WDT_LEFT,&hk->gnd_wdt_left,0);

    for (int i = 0; i < 6; i++)
    {
        perr |= gs_param_get_int16 (&tinst, GS_P80_ACU_TELEMETRY_INPUT_I(i), &hk->input_i[i], 0);
        perr |= gs_param_get_uint16(&tinst, GS_P80_ACU_TELEMETRY_INPUT_V(i), &hk->input_v[i], 0);
    }

    if (perr != GS_OK)
    {
        gs_param_table_free(&tinst);
        return perr;
    }

    OS_printf("\n================[p80 ACU (node %u) HK]===================\n", csp_node);
    OS_printf("[SYSTEM] Uptime: %u s | BootCount: %u | Cause(Boot/Reset): %u / %u\n",
              hk->uptime, hk->bootcount, hk->bootcause, hk->resetcause);
    OS_printf("[INPUT]  IDX | Volt(mV) | Curr(mA)\n");
    for (int i = 0; i < 6; i++)
        OS_printf("         %3d | %8u | %8d\n", i, hk->input_v[i], hk->input_i[i]);
    OS_printf("[ACU]    VCC: %u mV | VBAT: %u mV\n", hk->vcc_v, hk->vbat_v);
    OS_printf("[TEMP]   T0: %d | T1: %d | T2: %d (deci-degC)\n", hk->temp[0], hk->temp[1], hk->temp[2]);
    OS_printf("[MPPT]   Mode: %u\n", hk->mppt_mode);
    OS_printf("[WDT]    GND: %u (Left: %u s)\n", hk->gnd_wdt_cnt, hk->gnd_wdt_left);
    OS_printf("=======================================================\n");

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

gs_error_t EPS_P80_Drv_GetFullTable(uint8_t csp_node, uint8_t table_id, uint32_t timeout_ms)
{
    uint16_t checksum;
    gs_param_table_instance_t tinst = {0};

    gs_error_t result = gs_rparam_download_table_spec(&tinst, NULL, csp_node, table_id, timeout_ms, &checksum);
    if (result)
    {
        gs_param_table_free(&tinst);
        return result;
    }

    gs_error_t err = gs_rparam_get_full_table(&tinst, csp_node, table_id, checksum, timeout_ms);
    if (err != GS_OK)
    {
        gs_param_table_free(&tinst);
        return err;
    }

    OS_printf("\n================[EPS Full Table (Node %u, Table %u)]===================\n",
              csp_node, table_id);
    OS_printf("  Name: %s | Rows: %u | Size: %u bytes\n",
              tinst.name ? tinst.name : "N/A", tinst.row_count, tinst.memory_size);

    for (unsigned int i = 0; i < tinst.row_count; i++)
    {
        const gs_param_table_row_t *row = &tinst.rows[i];
        char buf[128] = {0};
        unsigned int written = 0;
        const void *value = (const uint8_t *)tinst.memory + row->addr;

        gs_param_to_string(row, value, true, buf, sizeof(buf), 0, &written);
        OS_printf("  [%3u] %-14s %s\n", row->addr, row->name, buf);
    }

    OS_printf("=======================================================\n");

    gs_param_table_free(&tinst);

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

    perr |= gs_param_get_uint32(&tinst, GS_P80_PMU_TELEMETRY_BOOTCAUSE,    &bcn->bootcause,   0);
    perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_RESETCAUSE,   &bcn->resetcause,  0);
    perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_BOOTCOUNT,    &bcn->bootcount,   0);

    for (int i = 0; i < 6; i++)
    {
        bool en = false;
        perr |= gs_param_get_bool(&tinst, GS_P80_PMU_TELEMETRY_OUT_EN(i), &en, 0);
        bcn->out_en[i] = (uint8_t)en;
    }

    perr |= gs_param_get_int16(&tinst, GS_P80_PMU_TELEMETRY_TEMP(0), &bcn->temp[0], 0);
    perr |= gs_param_get_int16(&tinst, GS_P80_PMU_TELEMETRY_TEMP(1), &bcn->temp[1], 0);

    perr |= gs_param_get_uint8(&tinst, GS_P80_PMU_TELEMETRY_BATT_MODE, &bcn->batt_mode, 0);
    perr |= gs_param_get_int16(&tinst, GS_P80_PMU_TELEMETRY_BATT_I,    &bcn->batt_i,    0);
    perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_BATT_V,   &bcn->batt_v,    0);

    for (int i = 0; i < 8; i++)
    {
        bool en = false;
        perr |= gs_param_get_bool(&tinst, GS_P80_PMU_TELEMETRY_SM_EN(i), &en, 0);
        bcn->sm_en[i] = (uint8_t)en;
    }

    perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_GND_WDT_CNT,  &bcn->gnd_wdt_cnt,  0);
    perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_BUS_WDT_CNT,  &bcn->bus_wdt_cnt,  0);
    perr |= gs_param_get_uint32(&tinst, GS_P80_PMU_TELEMETRY_GND_WDT_LEFT, &bcn->gnd_wdt_left, 0);
    perr |= gs_param_get_uint32(&tinst, GS_P80_PMU_TELEMETRY_BUS_WDT_LEFT, &bcn->bus_wdt_left, 0);

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
        perr |= gs_param_get_int16 (&tinst, GS_P80_ACU_TELEMETRY_INPUT_I(i), &bcn->input_i[i], 0);
        perr |= gs_param_get_uint16(&tinst, GS_P80_ACU_TELEMETRY_INPUT_V(i), &bcn->input_v[i], 0);
    }

    perr |= gs_param_get_uint8(&tinst, GS_P80_ACU_TELEMETRY_MPPT_MODE, &bcn->mppt_mode, 0);

    gs_param_table_free(&tinst);

    return perr;
}
