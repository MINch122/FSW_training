/**
 * @file
 *   EPS BP8 (NanoPower Battery Pack) Device Driver Implementation
 */

#include "eps_bp8_drv.h"

#include <gs/param/internal/types.h>
#include <gs/param/internal/rparam.h>
#include <gs/param/rparam.h>
#include <gs/param/serialize.h>
#include <gs/param/table.h>
#include <gs/csp/csp.h>
#include <csp/csp_endian.h>
#include <stdlib.h>
#include <string.h>

#include "common_types.h"
#include "osapi.h"

#define EPS_BP8_RPARAM_LIST_MAX_COUNT 5u
#define EPS_BP8_RPARAM_LIST_MAX_ELEMENT_SIZE sizeof(float)
#define EPS_BP8_RPARAM_LIST_MAX_REPLY_PAYLOAD \
    ((sizeof(uint16_t) + EPS_BP8_RPARAM_LIST_MAX_ELEMENT_SIZE) * EPS_BP8_RPARAM_LIST_MAX_COUNT)

static gs_error_t EPS_BP8_DrvRparamGetList(uint8_t csp_node, const uint16_t *addresses,
                                           gs_param_type_t type, void *values,
                                           size_t value_element_size, size_t count,
                                           uint32_t timeout_ms)
{
    uint16_t transaction_words[(sizeof(gs_rparam_query_t) + EPS_BP8_RPARAM_LIST_MAX_REPLY_PAYLOAD +
                                sizeof(uint16_t) - 1u) /
                               sizeof(uint16_t)] = {0};
    gs_rparam_query_t *query = (gs_rparam_query_t *)transaction_words;
    const size_t query_payload_size = sizeof(query->payload.addr[0]) * count;
    const size_t query_size = RPARAM_QUERY_LENGTH(query, query_payload_size);
    const size_t reply_payload_element_size = sizeof(query->payload.addr[0]) + value_element_size;
    const size_t reply_payload_size = reply_payload_element_size * count;
    const size_t reply_size = RPARAM_QUERY_LENGTH(query, reply_payload_size);

    if (addresses == NULL || values == NULL || count == 0 ||
        count > EPS_BP8_RPARAM_LIST_MAX_COUNT || value_element_size == 0 ||
        value_element_size > EPS_BP8_RPARAM_LIST_MAX_ELEMENT_SIZE)
    {
        return GS_ERROR_ARG;
    }

    query->action   = RPARAM_GET;
    query->table_id = EPS_BP8_TABLE_TELEMETRY;
    query->checksum = csp_hton16(GS_RPARAM_MAGIC_CHECKSUM);
    query->seq      = 0;
    query->total    = 0;
    query->length   = csp_hton16(query_payload_size);

    for (size_t i = 0; i < count; i++)
    {
        query->payload.addr[i] = csp_hton16(addresses[i]);
    }

    if (csp_transaction2(CSP_PRIO_HIGH, csp_node, GS_CSP_PORT_RPARAM, timeout_ms,
                         query, query_size, query, reply_size, CSP_O_CRC32) <= 0)
    {
        return GS_ERROR_IO;
    }

    query->length = csp_ntoh16(query->length);
    if (query->action != RPARAM_REPLY ||
        query->table_id != EPS_BP8_TABLE_TELEMETRY ||
        query->length != reply_payload_size)
    {
        return GS_ERROR_DATA;
    }

    for (size_t i = 0; i < count; i++)
    {
        const size_t item_offset = i * reply_payload_element_size;
        uint16_t reply_addr;

        memcpy(&reply_addr, &query->payload.packed[item_offset], sizeof(reply_addr));
        reply_addr = csp_betoh16(reply_addr);
        if (reply_addr != addresses[i])
        {
            return GS_ERROR_DATA;
        }

        memcpy((uint8_t *)values + (i * value_element_size),
               &query->payload.packed[item_offset + sizeof(reply_addr)],
               value_element_size);
        gs_param_betoh(type, (uint8_t *)values + (i * value_element_size));
    }

    return GS_OK;
}

static gs_error_t EPS_BP8_DrvRparamGetListChecked(uint8_t csp_node, const char *field,
                                                  const uint16_t *addresses,
                                                  gs_param_type_t type, void *values,
                                                  size_t value_element_size, size_t count,
                                                  uint32_t timeout_ms)
{
    gs_error_t err;

    err = EPS_BP8_DrvRparamGetList(csp_node, addresses, type, values,
                                   value_element_size, count, timeout_ms);
    if (err != GS_OK)
    {
        OS_printf("[EPS][BP8][RPARAM] FAIL node=%u table=%u op=GET_LIST field=%s "
                  "addr=%u type=%u count=%lu err=%d timeout=%lu ms\n",
                  (unsigned int)csp_node, (unsigned int)EPS_BP8_TABLE_TELEMETRY, field,
                  (unsigned int)((addresses != NULL && count > 0) ? addresses[0] : 0),
                  (unsigned int)type, (unsigned long)count, (int)err,
                  (unsigned long)timeout_ms);
    }

    return err;
}

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

gs_error_t EPS_BP8_Drv_GetBcn(uint8_t csp_node, EPS_BP8_Drv_BcnTlm_t *bcn, uint32_t timeout_ms)
{
    EPS_BP8_Drv_BcnTlm_t next_bcn = {0};
    const uint16_t uint16_addresses[] = {
        EPS_BP8_TLM_BOOTCOUNT,
        EPS_BP8_TLM_BOOTCAUSE,
        EPS_BP8_TLM_RESETCAUSE,
        EPS_BP8_TLM_VBAT,
        EPS_BP8_TLM_HEATER_I,
    };
    const uint16_t float_addresses[] = {
        EPS_BP8_TLM_SOC,
        EPS_BP8_TLM_BAT_AVR_TEMP,
        EPS_BP8_TLM_I,
    };
    uint16_t uint16_values[5] = {0};
    float    float_values[3]  = {0.0F};
    gs_error_t err;

    if (bcn == NULL)
        return GS_ERROR_ARG;

    err = EPS_BP8_DrvRparamGetListChecked(csp_node, "BP8_BCN_UINT16",
                                          uint16_addresses, GS_PARAM_UINT16,
                                          uint16_values, sizeof(uint16_values[0]), 5,
                                          timeout_ms);
    if (err != GS_OK)
        return err;

    err = EPS_BP8_DrvRparamGetListChecked(csp_node, "BP8_BCN_FLOAT",
                                          float_addresses, GS_PARAM_FLOAT,
                                          float_values, sizeof(float_values[0]), 3,
                                          timeout_ms);
    if (err != GS_OK)
        return err;

    next_bcn.BootCount     = uint16_values[0];
    next_bcn.BootCause     = uint16_values[1];
    next_bcn.ResetCause    = uint16_values[2];
    next_bcn.Vbat          = uint16_values[3];
    next_bcn.HeaterCurrent = uint16_values[4];
    next_bcn.Soc           = float_values[0];
    next_bcn.BatAvrTemp    = float_values[1];
    next_bcn.Current       = float_values[2];

    *bcn = next_bcn;

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
