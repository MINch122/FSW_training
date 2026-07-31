/**
 * @file
 *   EPS P80 (NanoPower) Device Driver Implementation
 */

#include "eps_p80_drv.h"

#include <gs/param/internal/types.h>
#include <gs/param/internal/rparam.h>
#include <gs/param/rparam.h>
#include <gs/param/serialize.h>
#include <gs/param/table.h>
#include <gs/csp/csp.h>
#include <csp/csp_buffer.h>
#include <csp/csp_endian.h>
#include <csp/csp_iflist.h>
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

#define EPS_P80_CSP_CAN_IFACE_NAME "CSP CAN"
#define EPS_P80_RPARAM_LIST_MAX_COUNT 15u
#define EPS_P80_RPARAM_LIST_MAX_ELEMENT_SIZE sizeof(uint32_t)
#define EPS_P80_RPARAM_LIST_MAX_REPLY_PAYLOAD \
    ((sizeof(uint16_t) + EPS_P80_RPARAM_LIST_MAX_ELEMENT_SIZE) * EPS_P80_RPARAM_LIST_MAX_COUNT)

typedef struct
{
    OS_time_t start_time;
    bool      timing_valid;
    bool      can_stats_valid;
    int       free_buffers;
    uint32_t  can_tx;
    uint32_t  can_rx;
    uint32_t  can_tx_error;
    uint32_t  can_rx_error;
    uint32_t  can_drop;
    uint32_t  can_frame;
} EPS_P80_RparamDiag_t;

static EPS_P80_RparamDiag_t EPS_P80_DrvRparamDiagStart(void)
{
    EPS_P80_RparamDiag_t diag = {
        .start_time = OS_TIME_ZERO,
    };
    csp_iface_t *iface;

    diag.timing_valid = (OS_GetLocalTime(&diag.start_time) == OS_SUCCESS);
    diag.free_buffers = csp_buffer_remaining();

    iface = csp_iflist_get_by_name(EPS_P80_CSP_CAN_IFACE_NAME);
    if (iface != NULL)
    {
        diag.can_stats_valid = true;
        diag.can_tx          = iface->tx;
        diag.can_rx          = iface->rx;
        diag.can_tx_error    = iface->tx_error;
        diag.can_rx_error    = iface->rx_error;
        diag.can_drop        = iface->drop;
        diag.can_frame       = iface->frame;
    }

    return diag;
}

static void EPS_P80_DrvPrintRparamFailure(const EPS_P80_RparamDiag_t *start,
                                          uint8_t csp_node, uint8_t table_id,
                                          const char *operation, const char *field,
                                          uint16_t addr, gs_param_type_t type,
                                          size_t count, uint32_t timeout_ms,
                                          gs_error_t err)
{
    OS_time_t   end_time;
    int64_t     elapsed_ms = -1;
    int         free_buffers;
    csp_iface_t *iface;
    const char  *hint = "connect/buffer/send/read/reply-size";

    if (start->timing_valid && OS_GetLocalTime(&end_time) == OS_SUCCESS)
    {
        elapsed_ms = OS_TimeGetTotalMilliseconds(OS_TimeSubtract(end_time, start->start_time));
    }

    free_buffers = csp_buffer_remaining();
    iface = csp_iflist_get_by_name(EPS_P80_CSP_CAN_IFACE_NAME);

    if (iface != NULL && start->can_stats_valid)
    {
        if (iface->tx_error != start->can_tx_error)
        {
            hint = "CAN TX error";
        }
        else if (iface->rx_error != start->can_rx_error)
        {
            hint = "CAN RX error";
        }
        else if (iface->frame != start->can_frame)
        {
            hint = "CAN frame loss/order error";
        }
        else if (iface->drop != start->can_drop)
        {
            hint = "CSP queue drop";
        }
        else if (start->free_buffers == 0 || free_buffers == 0)
        {
            hint = "CSP buffer exhausted";
        }
        else if (elapsed_ms >= 0 && (uint64_t)elapsed_ms >= timeout_ms)
        {
            hint = "reply timeout/late reply";
        }

        OS_printf("[EPS][P80][RPARAM] FAIL node=%u table=%u op=%s field=%s "
                  "addr=%u type=%u count=%lu err=%d timeout=%lu ms elapsed=%ld ms hint=%s\n",
                  (unsigned int)csp_node, (unsigned int)table_id, operation, field,
                  (unsigned int)addr, (unsigned int)type, (unsigned long)count,
                  (int)err, (unsigned long)timeout_ms, (long)elapsed_ms, hint);
        OS_printf("[EPS][P80][RPARAM] CSP buffers=%d->%d CAN "
                  "tx=%lu(+%lu) rx=%lu(+%lu) tx_err=%lu(+%lu) rx_err=%lu(+%lu) "
                  "drop=%lu(+%lu) frame=%lu(+%lu)\n",
                  start->free_buffers, free_buffers,
                  (unsigned long)iface->tx, (unsigned long)(iface->tx - start->can_tx),
                  (unsigned long)iface->rx, (unsigned long)(iface->rx - start->can_rx),
                  (unsigned long)iface->tx_error,
                  (unsigned long)(iface->tx_error - start->can_tx_error),
                  (unsigned long)iface->rx_error,
                  (unsigned long)(iface->rx_error - start->can_rx_error),
                  (unsigned long)iface->drop, (unsigned long)(iface->drop - start->can_drop),
                  (unsigned long)iface->frame, (unsigned long)(iface->frame - start->can_frame));
    }
    else
    {
        if (start->free_buffers == 0 || free_buffers == 0)
        {
            hint = "CSP buffer exhausted";
        }
        else if (elapsed_ms >= 0 && (uint64_t)elapsed_ms >= timeout_ms)
        {
            hint = "reply timeout/late reply";
        }

        OS_printf("[EPS][P80][RPARAM] FAIL node=%u table=%u op=%s field=%s "
                  "addr=%u type=%u count=%lu err=%d timeout=%lu ms elapsed=%ld ms "
                  "hint=%s CSP_buffers=%d->%d CAN_stats=unavailable\n",
                  (unsigned int)csp_node, (unsigned int)table_id, operation, field,
                  (unsigned int)addr, (unsigned int)type, (unsigned long)count,
                  (int)err, (unsigned long)timeout_ms, (long)elapsed_ms, hint,
                  start->free_buffers, free_buffers);
    }
}

static gs_error_t EPS_P80_DrvRparamGetList(uint8_t csp_node, uint8_t table_id,
                                           const uint16_t *addresses,
                                           gs_param_type_t type, void *values,
                                           size_t value_element_size, size_t count,
                                           uint32_t timeout_ms)
{
    uint16_t transaction_words[(sizeof(gs_rparam_query_t) + EPS_P80_RPARAM_LIST_MAX_REPLY_PAYLOAD +
                                sizeof(uint16_t) - 1u) /
                               sizeof(uint16_t)] = {0};
    gs_rparam_query_t *query = (gs_rparam_query_t *)transaction_words;
    const size_t query_payload_size = sizeof(query->payload.addr[0]) * count;
    const size_t query_size = RPARAM_QUERY_LENGTH(query, query_payload_size);
    const size_t reply_payload_element_size = sizeof(query->payload.addr[0]) + value_element_size;
    const size_t reply_payload_size = reply_payload_element_size * count;
    const size_t reply_size = RPARAM_QUERY_LENGTH(query, reply_payload_size);

    if (addresses == NULL || values == NULL || count == 0 ||
        count > EPS_P80_RPARAM_LIST_MAX_COUNT || value_element_size == 0 ||
        value_element_size > EPS_P80_RPARAM_LIST_MAX_ELEMENT_SIZE)
    {
        return GS_ERROR_ARG;
    }

    query->action   = RPARAM_GET;
    query->table_id = table_id;
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
    if (query->action != RPARAM_REPLY || query->table_id != table_id ||
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

static gs_error_t EPS_P80_DrvRparamGetListChecked(uint8_t csp_node, uint8_t table_id,
                                                  const char *field, const uint16_t *addresses,
                                                  gs_param_type_t type, void *values,
                                                  size_t value_element_size, size_t count,
                                                  uint32_t timeout_ms)
{
    EPS_P80_RparamDiag_t diag;
    gs_error_t           err;

    diag = EPS_P80_DrvRparamDiagStart();
    err  = EPS_P80_DrvRparamGetList(csp_node, table_id, addresses, type, values,
                                    value_element_size, count, timeout_ms);
    if (err != GS_OK)
    {
        EPS_P80_DrvPrintRparamFailure(&diag, csp_node, table_id, "GET_LIST", field,
                                      (addresses != NULL && count > 0) ? addresses[0] : 0,
                                      type, count, timeout_ms, err);
    }

    return err;
}

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

#define EPS_P80_RPARAM_GET_UINT32(table_id, addr, dst) \
    do \
    { \
        EPS_P80_RparamDiag_t diag = EPS_P80_DrvRparamDiagStart(); \
        uint32_t value = 0; \
        err = gs_rparam_get_uint32(csp_node, (table_id), (addr), \
                                   GS_RPARAM_MAGIC_CHECKSUM, timeout_ms, &value); \
        if (err != GS_OK) \
        { \
            EPS_P80_DrvPrintRparamFailure(&diag, csp_node, (table_id), "GET", #addr, \
                                          (addr), GS_PARAM_UINT32, 1, timeout_ms, err); \
            return err; \
        } \
        (dst) = value; \
    } while (0)

#define EPS_P80_RPARAM_GET_UINT16(table_id, addr, dst) \
    do \
    { \
        EPS_P80_RparamDiag_t diag = EPS_P80_DrvRparamDiagStart(); \
        uint16_t value = 0; \
        err = gs_rparam_get_uint16(csp_node, (table_id), (addr), \
                                   GS_RPARAM_MAGIC_CHECKSUM, timeout_ms, &value); \
        if (err != GS_OK) \
        { \
            EPS_P80_DrvPrintRparamFailure(&diag, csp_node, (table_id), "GET", #addr, \
                                          (addr), GS_PARAM_UINT16, 1, timeout_ms, err); \
            return err; \
        } \
        (dst) = value; \
    } while (0)

#define EPS_P80_RPARAM_GET_INT16(table_id, addr, dst) \
    do \
    { \
        EPS_P80_RparamDiag_t diag = EPS_P80_DrvRparamDiagStart(); \
        int16_t value = 0; \
        err = gs_rparam_get_int16(csp_node, (table_id), (addr), \
                                  GS_RPARAM_MAGIC_CHECKSUM, timeout_ms, &value); \
        if (err != GS_OK) \
        { \
            EPS_P80_DrvPrintRparamFailure(&diag, csp_node, (table_id), "GET", #addr, \
                                          (addr), GS_PARAM_INT16, 1, timeout_ms, err); \
            return err; \
        } \
        (dst) = value; \
    } while (0)

#define EPS_P80_RPARAM_GET_UINT8(table_id, addr, dst) \
    do \
    { \
        EPS_P80_RparamDiag_t diag = EPS_P80_DrvRparamDiagStart(); \
        uint8_t value = 0; \
        err = gs_rparam_get_uint8(csp_node, (table_id), (addr), \
                                  GS_RPARAM_MAGIC_CHECKSUM, timeout_ms, &value); \
        if (err != GS_OK) \
        { \
            EPS_P80_DrvPrintRparamFailure(&diag, csp_node, (table_id), "GET", #addr, \
                                          (addr), GS_PARAM_UINT8, 1, timeout_ms, err); \
            return err; \
        } \
        (dst) = value; \
    } while (0)

#define EPS_P80_RPARAM_GET_BOOL(table_id, addr, dst) \
    do \
    { \
        EPS_P80_RparamDiag_t diag = EPS_P80_DrvRparamDiagStart(); \
        uint8_t value = 0; \
        err = gs_rparam_get(csp_node, (table_id), (addr), GS_PARAM_BOOL, \
                            GS_RPARAM_MAGIC_CHECKSUM, timeout_ms, &value, sizeof(value)); \
        if (err != GS_OK) \
        { \
            EPS_P80_DrvPrintRparamFailure(&diag, csp_node, (table_id), "GET", #addr, \
                                          (addr), GS_PARAM_BOOL, 1, timeout_ms, err); \
            return err; \
        } \
        (dst) = (uint8_t)(value != 0); \
    } while (0)

#define EPS_P80_RPARAM_GET_ARRAY(table_id, addr, type, dst, count) \
    do \
    { \
        EPS_P80_RparamDiag_t diag = EPS_P80_DrvRparamDiagStart(); \
        err = gs_rparam_get_array(csp_node, (table_id), (addr), (type), \
                                  GS_RPARAM_MAGIC_CHECKSUM, timeout_ms, \
                                  (dst), sizeof((dst)[0]), (count)); \
        if (err != GS_OK) \
        { \
            EPS_P80_DrvPrintRparamFailure(&diag, csp_node, (table_id), "GET_ARRAY", #addr, \
                                          (addr), (type), (count), timeout_ms, err); \
            return err; \
        } \
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
    const uint16_t uint32_addresses[] = {
        GS_P80_PMU_TELEMETRY_BOOTCAUSE,
        GS_P80_PMU_TELEMETRY_GND_WDT_LEFT,
        GS_P80_PMU_TELEMETRY_BUS_WDT_LEFT,
    };
    const uint16_t uint16_addresses[] = {
        GS_P80_PMU_TELEMETRY_RESETCAUSE,
        GS_P80_PMU_TELEMETRY_BOOTCOUNT,
        GS_P80_PMU_TELEMETRY_BATT_V,
        GS_P80_PMU_TELEMETRY_BUS_WDT_CNT,
    };
    const uint16_t int16_addresses[] = {
        GS_P80_PMU_TELEMETRY_BATT_I,
        GS_P80_PMU_TELEMETRY_TEMP(0),
        GS_P80_PMU_TELEMETRY_TEMP(1),
    };
    uint16_t byte_addresses[15];
    uint32_t uint32_values[3] = {0};
    uint16_t uint16_values[4] = {0};
    int16_t  int16_values[3]  = {0};
    uint8_t  byte_values[15]  = {0};
    gs_error_t err;

    if (bcn == NULL)
        return GS_ERROR_ARG;

    byte_addresses[0] = GS_P80_PMU_TELEMETRY_BATT_MODE;
    for (uint8_t i = 0; i < 6; i++)
    {
        byte_addresses[1 + i] = GS_P80_PMU_TELEMETRY_OUT_EN(i);
    }
    for (uint8_t i = 0; i < 8; i++)
    {
        byte_addresses[7 + i] = GS_P80_PMU_TELEMETRY_SM_EN(i);
    }

    err = EPS_P80_DrvRparamGetListChecked(csp_node, table_id, "PMU_BCN_UINT32",
                                          uint32_addresses, GS_PARAM_UINT32,
                                          uint32_values, sizeof(uint32_values[0]), 3, timeout_ms);
    if (err != GS_OK)
        return err;

    err = EPS_P80_DrvRparamGetListChecked(csp_node, table_id, "PMU_BCN_UINT16",
                                          uint16_addresses, GS_PARAM_UINT16,
                                          uint16_values, sizeof(uint16_values[0]), 4, timeout_ms);
    if (err != GS_OK)
        return err;

    err = EPS_P80_DrvRparamGetListChecked(csp_node, table_id, "PMU_BCN_INT16",
                                          int16_addresses, GS_PARAM_INT16,
                                          int16_values, sizeof(int16_values[0]), 3, timeout_ms);
    if (err != GS_OK)
        return err;

    /*
     * batt_mode is UINT8 and out_en/sm_en are BOOL, but all are one-byte
     * parameters. GET_LIST carries addresses only, so they can share one
     * transaction; UINT8 conversion is a no-op for every item in this group.
     */
    err = EPS_P80_DrvRparamGetListChecked(csp_node, table_id, "PMU_BCN_BYTES",
                                          byte_addresses, GS_PARAM_UINT8,
                                          byte_values, sizeof(byte_values[0]), 15, timeout_ms);
    if (err != GS_OK)
        return err;

    next_bcn.bootcause    = uint32_values[0];
    next_bcn.gnd_wdt_left = uint32_values[1];
    next_bcn.bus_wdt_left = uint32_values[2];
    next_bcn.resetcause   = uint16_values[0];
    next_bcn.bootcount    = uint16_values[1];
    next_bcn.batt_v       = uint16_values[2];
    next_bcn.bus_wdt_cnt  = uint16_values[3];
    next_bcn.batt_i       = int16_values[0];
    next_bcn.temp[0]      = int16_values[1];
    next_bcn.temp[1]      = int16_values[2];
    next_bcn.batt_mode    = byte_values[0];
    memcpy(next_bcn.out_en, &byte_values[1], sizeof(next_bcn.out_en));
    next_bcn.sm_en_mask = EPS_P80_DrvPackBoolArray8(&byte_values[7]);

    *bcn = next_bcn;

    return GS_OK;
}

gs_error_t EPS_P80_Drv_PDU_GetBcn(uint8_t csp_node, EPS_P80_Drv_PDU_BcnTlm_t *bcn, uint32_t timeout_ms)
{
    const uint8_t table_id = GS_P80_PDU_TELEMETRY_TABLE_MEM_ID;
    static const uint8_t PduBcnChannels[EPS_P80_DRV_PDU_BCN_USED_CH_COUNT] =
        EPS_P80_DRV_PDU_BCN_USED_CH_LIST;
    EPS_P80_Drv_PDU_BcnTlm_t next_bcn = {0};
    uint16_t out_en_addr[EPS_P80_DRV_PDU_BCN_USED_CH_COUNT];
    uint16_t out_i_addr[EPS_P80_DRV_PDU_BCN_USED_CH_COUNT];
    uint8_t out_en[EPS_P80_DRV_PDU_BCN_USED_CH_COUNT] = {0};
    int16_t out_i[EPS_P80_DRV_PDU_BCN_USED_CH_COUNT] = {0};
    const uint16_t gnd_wdt_cnt_addr[] = {
        GS_P80_PDU_TELEMETRY_GND_WDT_CNT,
    };
    uint32_t gnd_wdt_cnt[1] = {0};
    gs_error_t err;

    if (bcn == NULL)
        return GS_ERROR_ARG;

    for (uint8_t i = 0; i < EPS_P80_DRV_PDU_BCN_USED_CH_COUNT; i++)
    {
        uint8_t channel = PduBcnChannels[i];

        out_en_addr[i] = GS_P80_PDU_TELEMETRY_OUT_EN(channel);
        out_i_addr[i]  = GS_P80_PDU_TELEMETRY_OUT_I(channel);
    }

    err = EPS_P80_DrvRparamGetListChecked(csp_node, table_id, "PDU_BCN_GND_WDT_CNT",
                                          gnd_wdt_cnt_addr, GS_PARAM_UINT32,
                                          gnd_wdt_cnt, sizeof(gnd_wdt_cnt[0]), 1, timeout_ms);
    if (err != GS_OK)
        return err;

    err = EPS_P80_DrvRparamGetListChecked(csp_node, table_id, "PDU_BCN_OUT_EN",
                                          out_en_addr, GS_PARAM_BOOL,
                                          out_en, sizeof(out_en[0]),
                                          EPS_P80_DRV_PDU_BCN_USED_CH_COUNT, timeout_ms);
    if (err != GS_OK)
        return err;

    err = EPS_P80_DrvRparamGetListChecked(csp_node, table_id, "PDU_BCN_OUT_I",
                                          out_i_addr, GS_PARAM_INT16,
                                          out_i, sizeof(out_i[0]),
                                          EPS_P80_DRV_PDU_BCN_USED_CH_COUNT, timeout_ms);
    if (err != GS_OK)
        return err;

    next_bcn.gnd_wdt_cnt = gnd_wdt_cnt[0];
    for (uint8_t i = 0; i < EPS_P80_DRV_PDU_BCN_USED_CH_COUNT; i++)
    {
        next_bcn.out_en[i] = out_en[i];
        next_bcn.out_i[i]  = out_i[i];
    }

    *bcn = next_bcn;

    return GS_OK;
}

gs_error_t EPS_P80_Drv_ACU_GetBcn(uint8_t csp_node, EPS_P80_Drv_ACU_BcnTlm_t *bcn, uint32_t timeout_ms)
{
    const uint8_t table_id = GS_P80_ACU_TELEMETRY_TABLE_MEM_ID;
    EPS_P80_Drv_ACU_BcnTlm_t next_bcn = {0};
    const uint16_t mppt_mode_addr[] = {
        GS_P80_ACU_TELEMETRY_MPPT_MODE,
    };
    uint16_t input_i_addr[6];
    uint16_t input_v_addr[6];
    uint8_t mppt_mode[1] = {0};
    int16_t input_i[6] = {0};
    uint16_t input_v[6] = {0};
    gs_error_t err;

    if (bcn == NULL)
        return GS_ERROR_ARG;

    for (uint8_t i = 0; i < 6; i++)
    {
        input_i_addr[i] = GS_P80_ACU_TELEMETRY_INPUT_I(i);
        input_v_addr[i] = GS_P80_ACU_TELEMETRY_INPUT_V(i);
    }

    err = EPS_P80_DrvRparamGetListChecked(csp_node, table_id, "ACU_BCN_MPPT_MODE",
                                          mppt_mode_addr, GS_PARAM_UINT8,
                                          mppt_mode, sizeof(mppt_mode[0]), 1, timeout_ms);
    if (err != GS_OK)
        return err;

    err = EPS_P80_DrvRparamGetListChecked(csp_node, table_id, "ACU_BCN_INPUT_I",
                                          input_i_addr, GS_PARAM_INT16,
                                          input_i, sizeof(input_i[0]), 6, timeout_ms);
    if (err != GS_OK)
        return err;

    err = EPS_P80_DrvRparamGetListChecked(csp_node, table_id, "ACU_BCN_INPUT_V",
                                          input_v_addr, GS_PARAM_UINT16,
                                          input_v, sizeof(input_v[0]), 6, timeout_ms);
    if (err != GS_OK)
        return err;

    next_bcn.mppt_mode = mppt_mode[0];
    memcpy(next_bcn.input_i, input_i, sizeof(next_bcn.input_i));
    memcpy(next_bcn.input_v, input_v, sizeof(next_bcn.input_v));

    *bcn = next_bcn;

    return GS_OK;
}
