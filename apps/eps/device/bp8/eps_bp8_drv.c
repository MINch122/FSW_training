/**
 * @file
 *   EPS BP8 (NanoPower Battery Pack) Device Driver Implementation
 */

#include "eps_bp8_drv.h"

#include <gs/param/internal/types.h>
#include <gs/param/rparam.h>
#include <gs/param/table.h>
#include <stdlib.h>
#include <string.h>

#include "common_types.h"

#define EPS_BP8_RPARAM_GET_UINT32(addr, dst) \
    do \
    { \
        uint32_t value = 0; \
        err = gs_rparam_get_uint32(csp_node, EPS_BP8_TABLE_TELEMETRY, (addr), \
                                   GS_RPARAM_MAGIC_CHECKSUM, timeout_ms, &value); \
        if (err != GS_OK) return err; \
        (dst) = value; \
    } while (0)

#define EPS_BP8_RPARAM_GET_UINT16(addr, dst) \
    do \
    { \
        uint16_t value = 0; \
        err = gs_rparam_get_uint16(csp_node, EPS_BP8_TABLE_TELEMETRY, (addr), \
                                   GS_RPARAM_MAGIC_CHECKSUM, timeout_ms, &value); \
        if (err != GS_OK) return err; \
        (dst) = value; \
    } while (0)

#define EPS_BP8_RPARAM_GET_INT16(addr, dst) \
    do \
    { \
        int16_t value = 0; \
        err = gs_rparam_get_int16(csp_node, EPS_BP8_TABLE_TELEMETRY, (addr), \
                                  GS_RPARAM_MAGIC_CHECKSUM, timeout_ms, &value); \
        if (err != GS_OK) return err; \
        (dst) = value; \
    } while (0)

#define EPS_BP8_RPARAM_GET_FLOAT(addr, dst) \
    do \
    { \
        float value = 0.0F; \
        err = gs_rparam_get_float(csp_node, EPS_BP8_TABLE_TELEMETRY, (addr), \
                                  GS_RPARAM_MAGIC_CHECKSUM, timeout_ms, &value); \
        if (err != GS_OK) return err; \
        (dst) = value; \
    } while (0)

#define EPS_BP8_RPARAM_GET_ARRAY(addr, type, dst, count) \
    do \
    { \
        err = gs_rparam_get_array(csp_node, EPS_BP8_TABLE_TELEMETRY, (addr), (type), \
                                  GS_RPARAM_MAGIC_CHECKSUM, timeout_ms, \
                                  (dst), sizeof((dst)[0]), (count)); \
        if (err != GS_OK) return err; \
    } while (0)

gs_error_t EPS_BP8_Drv_GetHk(uint8_t csp_node, EPS_BP8_Drv_HkTlm_t *hk, uint32_t timeout_ms)
{
    EPS_BP8_Drv_HkTlm_t next_hk = {0};
    int16_t bat_temp[4] = {0};
    uint8_t bat_fault = 0;
    gs_error_t err;

    if (hk == NULL)
        return GS_ERROR_ARG;

    EPS_BP8_RPARAM_GET_UINT32(EPS_BP8_TLM_UPTIME,       next_hk.Uptime);
    EPS_BP8_RPARAM_GET_UINT16(EPS_BP8_TLM_BOOTCOUNT,    next_hk.BootCount);
    EPS_BP8_RPARAM_GET_UINT16(EPS_BP8_TLM_BOOTCAUSE,    next_hk.BootCause);
    EPS_BP8_RPARAM_GET_UINT16(EPS_BP8_TLM_RESETCAUSE,   next_hk.ResetCause);
    EPS_BP8_RPARAM_GET_UINT16(EPS_BP8_TLM_VBAT,         next_hk.Vbat);
    EPS_BP8_RPARAM_GET_FLOAT (EPS_BP8_TLM_SOC,          next_hk.Soc);
    EPS_BP8_RPARAM_GET_FLOAT (EPS_BP8_TLM_I,            next_hk.Current);
    EPS_BP8_RPARAM_GET_UINT16(EPS_BP8_TLM_IN_I,         next_hk.InCurrent);
    EPS_BP8_RPARAM_GET_UINT16(EPS_BP8_TLM_OUT_I,        next_hk.OutCurrent);
    EPS_BP8_RPARAM_GET_UINT16(EPS_BP8_TLM_HEATER_I,     next_hk.HeaterCurrent);
    EPS_BP8_RPARAM_GET_INT16 (EPS_BP8_TLM_INT_TEMP,     next_hk.IntTemp);
    EPS_BP8_RPARAM_GET_FLOAT (EPS_BP8_TLM_BAT_AVR_TEMP, next_hk.BatAvrTemp);
    EPS_BP8_RPARAM_GET_ARRAY (EPS_BP8_TLM_BAT_1_TEMP,   GS_PARAM_INT16, bat_temp, 4);
    memcpy(next_hk.BatTemp, bat_temp, sizeof(next_hk.BatTemp));
    EPS_BP8_RPARAM_GET_UINT16(EPS_BP8_TLM_O_VOLT_COUNT, next_hk.OVoltCount);

    err = gs_rparam_get(csp_node, EPS_BP8_TABLE_TELEMETRY, EPS_BP8_TLM_BAT_FAULT,
                        GS_PARAM_BOOL, GS_RPARAM_MAGIC_CHECKSUM, timeout_ms,
                        &bat_fault, sizeof(bat_fault));
    if (err != GS_OK)
        return err;
    next_hk.BatFault = (uint8_t)(bat_fault != 0);

    *hk = next_hk;

    return GS_OK;
}

/* ========================================================================== */
/*  Remote Parameter Commands                                                 */
/* ========================================================================== */

gs_error_t EPS_BP8_Drv_ParamSet(uint8_t csp_node, uint8_t table_id,
                                 uint16_t addr, uint8_t type,
                                 const uint8_t *data, uint16_t size,
                                 uint32_t timeout_ms)
{
    return gs_rparam_set(csp_node, table_id, addr, type,
                         GS_RPARAM_MAGIC_CHECKSUM, timeout_ms,
                         data, size);
}

gs_error_t EPS_BP8_Drv_ParamGet(uint8_t csp_node, uint8_t table_id,
                                 uint16_t addr, uint8_t type,
                                 uint8_t *data, uint16_t size,
                                 uint32_t timeout_ms)
{
    return gs_rparam_get(csp_node, table_id, addr, type,
                         GS_RPARAM_MAGIC_CHECKSUM, timeout_ms,
                         (void *)data, size);
}

gs_error_t EPS_BP8_Drv_GetFullTable(uint8_t csp_node, uint8_t table_id,
                                     gs_param_table_instance_t *tinst, uint32_t timeout_ms)
{
    uint16_t checksum;

    if (tinst == NULL)
        return GS_ERROR_ARG;

    memset(tinst, 0, sizeof(*tinst));

    gs_error_t result = gs_rparam_download_table_spec(tinst, NULL, csp_node, table_id, timeout_ms, &checksum);
    if (result != GS_OK)
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

gs_error_t EPS_BP8_Drv_TableSave(uint8_t csp_node, uint8_t table_id, uint32_t timeout_ms)
{
    return gs_rparam_save(csp_node, timeout_ms, table_id, 0);
}

gs_error_t EPS_BP8_Drv_TableLoad(uint8_t csp_node, uint8_t table_id, uint32_t timeout_ms)
{
    return gs_rparam_load(csp_node, timeout_ms, 0, table_id);
}

gs_error_t EPS_BP8_Drv_ParamSaveAll(uint8_t csp_node, uint32_t timeout_ms)
{
    return gs_rparam_save(csp_node, timeout_ms, 0xFF, 0);
}
