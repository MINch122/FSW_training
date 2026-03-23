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
#include "osapi.h"

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

    perr |= gs_param_get_uint32(&tinst, EPS_BP8_TLM_UPTIME,       &hk->Uptime,        0);
    perr |= gs_param_get_uint16(&tinst, EPS_BP8_TLM_BOOTCOUNT,    &hk->BootCount,     0);
    perr |= gs_param_get_uint16(&tinst, EPS_BP8_TLM_BOOTCAUSE,    &hk->BootCause,     0);
    perr |= gs_param_get_uint16(&tinst, EPS_BP8_TLM_RESETCAUSE,   &hk->ResetCause,    0);
    perr |= gs_param_get_uint16(&tinst, EPS_BP8_TLM_VBAT,         &hk->Vbat,          0);
    perr |= gs_param_get_float (&tinst, EPS_BP8_TLM_SOC,          &hk->Soc,           0);
    perr |= gs_param_get_float (&tinst, EPS_BP8_TLM_I,            &hk->Current,       0);
    perr |= gs_param_get_uint16(&tinst, EPS_BP8_TLM_IN_I,         &hk->InCurrent,     0);
    perr |= gs_param_get_uint16(&tinst, EPS_BP8_TLM_OUT_I,        &hk->OutCurrent,    0);
    perr |= gs_param_get_uint16(&tinst, EPS_BP8_TLM_HEATER_I,     &hk->HeaterCurrent, 0);
    perr |= gs_param_get_int16 (&tinst, EPS_BP8_TLM_INT_TEMP,     &hk->IntTemp,       0);
    perr |= gs_param_get_float (&tinst, EPS_BP8_TLM_BAT_AVR_TEMP, &hk->BatAvrTemp,    0);
    perr |= gs_param_get_int16 (&tinst, EPS_BP8_TLM_BAT_1_TEMP,   &hk->BatTemp[0],    0);
    perr |= gs_param_get_int16 (&tinst, EPS_BP8_TLM_BAT_2_TEMP,   &hk->BatTemp[1],    0);
    perr |= gs_param_get_int16 (&tinst, EPS_BP8_TLM_BAT_3_TEMP,   &hk->BatTemp[2],    0);
    perr |= gs_param_get_int16 (&tinst, EPS_BP8_TLM_BAT_4_TEMP,   &hk->BatTemp[3],    0);
    perr |= gs_param_get_uint16(&tinst, EPS_BP8_TLM_O_VOLT_COUNT,  &hk->OVoltCount,    0);
    perr |= gs_param_get_bool  (&tinst, EPS_BP8_TLM_BAT_FAULT,     &bat_fault,         0);
    hk->BatFault = (uint8_t)bat_fault;

    if (perr != GS_OK)
    {
        if (tinst.memory) free(tinst.memory);
        if (tinst.rows) free((void *)tinst.rows);
        return perr;
    }

    OS_printf("\n================[NanoPower BP8 Telemetry]===================\n");
    OS_printf("[SYSTEM]  Uptime: %u s | BootCount: %u | Cause(Boot/Reset): %u / %u\n",
              hk->Uptime, hk->BootCount, hk->BootCause, hk->ResetCause);
    OS_printf("[BATTERY] Vbat: %u mV | SOC: %.2f | Current: %.3f A\n",
              hk->Vbat, (double)hk->Soc, (double)hk->Current);
    OS_printf("[CURRENT] In: %u mA | Out: %u mA | Heater: %u mA\n",
              hk->InCurrent, hk->OutCurrent, hk->HeaterCurrent);
    OS_printf("[TEMP]    MCU: %d ddegC | Avg: %.1f degC\n",
              hk->IntTemp, (double)hk->BatAvrTemp);
    OS_printf("[TEMP]    Bat1: %d | Bat2: %d | Bat3: %d | Bat4: %d (ddegC)\n",
              hk->BatTemp[0], hk->BatTemp[1], hk->BatTemp[2], hk->BatTemp[3]);
    OS_printf("[STATUS]  OVoltCount: %u | BatFault: %s\n",
              hk->OVoltCount, hk->BatFault ? "YES" : "NO");
    OS_printf("=============================================================\n");

    if (tinst.memory) free(tinst.memory);
    if (tinst.rows) free((void *)tinst.rows);

    return GS_OK;
}

gs_error_t EPS_BP8_Drv_SetHeater(uint8_t csp_node, uint16_t duration, uint32_t timeout_ms)
{
    return gs_rparam_set_uint16(csp_node,
                                 EPS_BP8_TABLE_CONTROL,
                                 EPS_BP8_CTRL_HEAT_MANUAL,
                                 GS_RPARAM_MAGIC_CHECKSUM,
                                 timeout_ms,
                                 duration);
}

gs_error_t EPS_BP8_Drv_ResetFault(uint8_t csp_node, uint32_t timeout_ms)
{
    uint8_t value = 1;
    return gs_rparam_set_uint8(csp_node,
                                EPS_BP8_TABLE_CONTROL,
                                EPS_BP8_CTRL_FAULT_RESET,
                                GS_RPARAM_MAGIC_CHECKSUM,
                                timeout_ms,
                                value);
}
