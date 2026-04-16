/**
 * @file
 *   EPS BP8 (NanoPower Battery Pack) Device Driver Implementation
 */

#include "eps_bp8_drv.h"

#include <gs/param/internal/types.h>
#include <gs/param/rparam.h>
#include <gs/param/table.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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

#define EPS_PARAM_GET_FLOAT_FIELD(tinst, addr, dst) \
    do \
    { \
        float value = 0.0F; \
        perr |= gs_param_get_float((tinst), (addr), &value, 0); \
        (dst) = value; \
    } while (0)

gs_error_t EPS_BP8_Drv_GetHk(uint8_t csp_node, EPS_BP8_Drv_HkTlm_t *hk, uint32_t timeout_ms)
{
    gs_param_table_instance_t tinst = {0};
    uint16_t checksum;

    gs_error_t err = gs_rparam_download_table_spec(&tinst, NULL,
                                                     csp_node,
                                                     EPS_BP8_TABLE_TELEMETRY,
                                                     timeout_ms,
                                                     &checksum);
    if (err != GS_OK)
    {
        if (tinst.memory) free(tinst.memory);
        if (tinst.rows) free((void *)tinst.rows);
        return err;
    }

    err = gs_rparam_get_full_table(&tinst,
                                    csp_node,
                                    EPS_BP8_TABLE_TELEMETRY,
                                    checksum,
                                    timeout_ms);
    if (err != GS_OK)
    {
        if (tinst.memory) free(tinst.memory);
        if (tinst.rows) free((void *)tinst.rows);
        return err;
    }

    bool bat_fault = false;
    gs_error_t perr = GS_OK;

    EPS_PARAM_GET_UINT32_FIELD(&tinst, EPS_BP8_TLM_UPTIME,       hk->Uptime);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, EPS_BP8_TLM_BOOTCOUNT,    hk->BootCount);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, EPS_BP8_TLM_BOOTCAUSE,    hk->BootCause);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, EPS_BP8_TLM_RESETCAUSE,   hk->ResetCause);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, EPS_BP8_TLM_VBAT,         hk->Vbat);
    EPS_PARAM_GET_FLOAT_FIELD (&tinst, EPS_BP8_TLM_SOC,          hk->Soc);
    EPS_PARAM_GET_FLOAT_FIELD (&tinst, EPS_BP8_TLM_I,            hk->Current);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, EPS_BP8_TLM_IN_I,         hk->InCurrent);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, EPS_BP8_TLM_OUT_I,        hk->OutCurrent);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, EPS_BP8_TLM_HEATER_I,     hk->HeaterCurrent);
    EPS_PARAM_GET_INT16_FIELD (&tinst, EPS_BP8_TLM_INT_TEMP,     hk->IntTemp);
    EPS_PARAM_GET_FLOAT_FIELD (&tinst, EPS_BP8_TLM_BAT_AVR_TEMP, hk->BatAvrTemp);
    EPS_PARAM_GET_INT16_FIELD (&tinst, EPS_BP8_TLM_BAT_1_TEMP,   hk->BatTemp[0]);
    EPS_PARAM_GET_INT16_FIELD (&tinst, EPS_BP8_TLM_BAT_2_TEMP,   hk->BatTemp[1]);
    EPS_PARAM_GET_INT16_FIELD (&tinst, EPS_BP8_TLM_BAT_3_TEMP,   hk->BatTemp[2]);
    EPS_PARAM_GET_INT16_FIELD (&tinst, EPS_BP8_TLM_BAT_4_TEMP,   hk->BatTemp[3]);
    EPS_PARAM_GET_UINT16_FIELD(&tinst, EPS_BP8_TLM_O_VOLT_COUNT, hk->OVoltCount);
    perr |= gs_param_get_bool  (&tinst, EPS_BP8_TLM_BAT_FAULT,     &bat_fault,         0);
    hk->BatFault = (uint8_t)bat_fault;

    if (perr != GS_OK)
    {
        if (tinst.memory) free(tinst.memory);
        if (tinst.rows) free((void *)tinst.rows);
        return perr;
    }

    if (tinst.memory) free(tinst.memory);
    if (tinst.rows) free((void *)tinst.rows);

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
