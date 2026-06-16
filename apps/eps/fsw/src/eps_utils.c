/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * \file
 *   This file contains the source code for the Sample App utility functions
 */

/*
** Include Files:
*/
#include "eps_app.h"
#include "eps_eventids.h"
#include "eps_utils.h"
#include "cfe_srl_csp.h"
#include "eps_interface_cfg.h"
#include "eps_p80_drv.h"
#include "eps_bp8_drv.h"

#include <gs/param/internal/types.h>
#include <gs/param/rparam.h>
#include <gs/param/table.h>
#include <string.h>

void EPS_CopyCmdString(char *dst, size_t dst_size, const char *src, size_t src_size)
{
    size_t copy_size = src_size;

    if (copy_size >= dst_size)
    {
        copy_size = dst_size - 1;
    }

    memcpy(dst, src, copy_size);
    dst[copy_size] = '\0';
}

static double EPS_DeciDegCToDegC(int16_t temp_ddegc)
{
    return (double)temp_ddegc / 10.0;
}

static double EPS_MilliAmpToAmp(uint16_t current_ma)
{
    return (double)current_ma / 1000.0;
}

static double EPS_SocRatioToPercent(float soc)
{
    return (double)soc * 100.0;
}

static void EPS_FillBcnNodeInvalid(EPS_BcnTlm_Full_Payload_t *Bcn, uint8 NodeIndex)
{
    switch (NodeIndex)
    {
        case EPS_BCN_NODE_PMU_INDEX:
            memset(&Bcn->PMU, EPS_BCN_INVALID_FILL, sizeof(Bcn->PMU));
            break;

        case EPS_BCN_NODE_PDU_INDEX:
            memset(&Bcn->PDU, EPS_BCN_INVALID_FILL, sizeof(Bcn->PDU));
            break;

        case EPS_BCN_NODE_ACU1_INDEX:
            memset(&Bcn->ACU[0], EPS_BCN_INVALID_FILL, sizeof(Bcn->ACU[0]));
            break;

        case EPS_BCN_NODE_ACU2_INDEX:
            memset(&Bcn->ACU[1], EPS_BCN_INVALID_FILL, sizeof(Bcn->ACU[1]));
            break;

        case EPS_BCN_NODE_BP8_INDEX:
            memset(&Bcn->BP8, EPS_BCN_INVALID_FILL, sizeof(Bcn->BP8));
            break;

        default:
            break;
    }
}

static uint8 EPS_MarkBcnNodeFailure(EPS_BcnTlm_Full_Payload_t *Bcn, uint8 NodeIndex,
                                    gs_error_t Err, const char *DeviceName)
{
    uint8 NodeMask;

    if (NodeIndex >= EPS_BCN_NODE_COUNT)
    {
        return 0;
    }

    NodeMask = EPS_BCN_NODE_MASK(NodeIndex);

    EPS_FillBcnNodeInvalid(Bcn, NodeIndex);
    EPS_AppData.Counters.GetBcnErrCounter++;

    CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                      "EPS: BCN %s fetch failed, err=%d, filled zero",
                      DeviceName, (int)Err);

    return NodeMask;
}

void EPS_PrintP80PowerIfStatus(const char *title, const power_if_ch_status_t *status)
{
    OS_printf("%s\n", title);
    OS_printf("  ch_idx:     %u\n", status->ch_idx);
    OS_printf("  name:       %.8s\n", status->name);
    OS_printf("  mode:       %u\n", status->mode);
    OS_printf("  on_cnt:     %u\n", status->on_cnt);
    OS_printf("  off_cnt:    %u\n", status->off_cnt);
    OS_printf("  cur_lu_lim: %u\n", status->cur_lu_lim);
    OS_printf("  cur_lim:    %u\n", status->cur_lim);
    OS_printf("  voltage:    %u mV\n", status->voltage);
    OS_printf("  current:    %d mA\n", status->current);
    OS_printf("  latchup:    %u\n", status->latchup);
}

void EPS_PrintP80PowerIfList(const power_if_cmd_list_response_t *list)
{
    OS_printf("EPS Power_If_List Response:\n");
    OS_printf("  cmd:    %u\n", list->cmd);
    OS_printf("  status: %u\n", list->status);
    OS_printf("  count:  %u\n", list->count);
    OS_printf("  %-6s %-8s %s\n", "ch_idx", "mode", "name");
    for (uint8_t i = 0; i < list->count && i < (sizeof(list->list) / sizeof(list->list[0])); i++)
    {
        OS_printf("  %-6u %-8u %.8s\n",
                  list->list[i].ch_idx,
                  list->list[i].mode,
                  list->list[i].name);
    }
}

void EPS_PrintP80PmuHk(const EPS_P80_PMU_HkTlm_Payload_t *hk)
{
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
}

void EPS_PrintP80PduHk(const EPS_P80_PDU_HkTlm_Payload_t *hk)
{
    OS_printf("\n================[p80 PDU HK]===================\n");
    OS_printf("[SYSTEM] Uptime: %u s | BootCount: %u | Cause(Boot/Reset): %u / %u\n",
              hk->uptime, hk->bootcount, hk->bootcause, hk->resetcause);
    OS_printf("[PDU]    VCC: %u mV (%u mA) | VBAT: %u mV\n", hk->vcc_v, hk->vcc_i, hk->vbat_v);
    OS_printf("[OUTPUT] IDX | EN | Curr(mA)\n");
    for (int i = 0; i < 24; i++)
        OS_printf("         %3d | %2s | %8d\n", i, hk->out_en[i] ? "ON" : "OFF", hk->out_i[i]);
    OS_printf("[TEMP]   %d (deci-degC) | BattMode: %u\n", hk->temp, hk->batt_mode);
    OS_printf("[WDT]    GND: %u (Left: %u s) | BUS: %u (Left: %u s)\n",
              hk->gnd_wdt_cnt, hk->gnd_wdt_left, hk->bus_wdt_cnt, hk->bus_wdt_left);
    OS_printf("=======================================================\n");
}

void EPS_PrintP80AcuHk(uint8_t cspNode, const EPS_P80_ACU_HkTlm_Payload_t *hk)
{
    OS_printf("\n================[%s (node %u) HK]===================\n",
              EPS_GetCspNodeDeviceName(cspNode), cspNode);
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
}

void EPS_PrintBcnReport(const EPS_BcnTlm_Full_Payload_t *bcn)
{
    static const uint8_t PduBcnChannels[EPS_PDU_BCN_USED_CH_COUNT] =
        EPS_PDU_BCN_USED_CH_LIST;

    OS_printf("\n================[EPS BCN Report]===================\n");
    OS_printf("[PMU] BootCause: %u | ResetCause: %u | BootCount: %u\n",
              bcn->PMU.bootcause, bcn->PMU.resetcause, bcn->PMU.bootcount);
    OS_printf("[PMU] BattV: %u mV | BattI: %d mA | Mode: %u\n",
              bcn->PMU.batt_v, bcn->PMU.batt_i, bcn->PMU.batt_mode);
    OS_printf("[PMU] Temp: %d / %d (ddegC)\n", bcn->PMU.temp[0], bcn->PMU.temp[1]);
    OS_printf("[PMU] OutEn: ");
    for (int i = 0; i < 6; i++) OS_printf("%u ", bcn->PMU.out_en[i]);
    OS_printf("\n");
    OS_printf("[PMU] SmEnMask: 0x%02X (bit0..7=submodule enable 0..7, 1=enabled)\n",
              bcn->PMU.sm_en_mask);
    OS_printf("[PMU] WDT GND: %u (Left: %u s) | BUS: %u (Left: %u s)\n",
              bcn->PMU.gnd_wdt_cnt, bcn->PMU.gnd_wdt_left,
              bcn->PMU.bus_wdt_cnt, bcn->PMU.bus_wdt_left);
    OS_printf("[PDU] Used channel OutI(mA)/OutEn: ");
    for (size_t i = 0; i < EPS_PDU_BCN_USED_CH_COUNT; i++)
        OS_printf("ch%02u=%d/%u ", (unsigned int)PduBcnChannels[i],
                  (int)bcn->PDU.out_i[i], (unsigned int)bcn->PDU.out_en[i]);
    OS_printf("\n");
    static const uint8_t AcuNodes[EPS_BCN_ACU_COUNT] = {EPS_P80_ACU1_CSP_NODE, EPS_P80_ACU2_CSP_NODE};
    for (int acu = 0; acu < EPS_BCN_ACU_COUNT; acu++)
    {
        const char *device = EPS_GetCspNodeDeviceName(AcuNodes[acu]);
        OS_printf("[%s node %u] InputI(mA): ", device, (unsigned int)AcuNodes[acu]);
        for (int i = 0; i < 6; i++) OS_printf("%d ", bcn->ACU[acu].input_i[i]);
        OS_printf("\n");
        OS_printf("[%s node %u] InputV(mV): ", device, (unsigned int)AcuNodes[acu]);
        for (int i = 0; i < 6; i++) OS_printf("%u ", bcn->ACU[acu].input_v[i]);
        OS_printf("\n");
        OS_printf("[%s node %u] MPPT Mode: %u\n", device, (unsigned int)AcuNodes[acu], bcn->ACU[acu].mppt_mode);
    }
    OS_printf("[BP8] BootCount: %u | BootCause: %u | ResetCause: %u\n",
              bcn->BP8.bootcount, bcn->BP8.bootcause, bcn->BP8.resetcause);
    OS_printf("[BP8] SOC: %.1f %% | Vbat: %u mV | Current: %.3f A | HeaterI: %u mA\n",
              EPS_SocRatioToPercent(bcn->BP8.soc), bcn->BP8.vbat,
              (double)bcn->BP8.current, bcn->BP8.heater_i);
    OS_printf("[BP8] BatAvrTemp: %.1f degC\n", (double)bcn->BP8.bat_avr_temp);
    OS_printf("=======================================================\n");
}

void EPS_PrintBP8Hk(const EPS_BP8_HkTlm_Payload_t *hk)
{
    OS_printf("\n================[EPS BP8 HK]===================\n");
    OS_printf("[BP8] Uptime: %u s | BootCount: %u | BootCause: %u | ResetCause: %u\n",
              hk->Uptime, hk->BootCount, hk->BootCause, hk->ResetCause);
    OS_printf("[BP8] Vbat: %u mV | SOC: %.1f %% | Current: %.3f A\n",
              hk->Vbat, EPS_SocRatioToPercent(hk->Soc), (double)hk->Current);
    OS_printf("[BP8] InCurrent: %.3f A | OutCurrent: %.3f A | HeaterI: %u mA\n",
              EPS_MilliAmpToAmp(hk->InCurrent), EPS_MilliAmpToAmp(hk->OutCurrent),
              hk->HeaterCurrent);
    OS_printf("[BP8] IntTemp: %.1f | BatAvrTemp: %.1f degC\n",
              EPS_DeciDegCToDegC(hk->IntTemp), (double)hk->BatAvrTemp);
    OS_printf("[BP8] BatTemp: %.1f / %.1f / %.1f / %.1f degC\n",
              EPS_DeciDegCToDegC(hk->BatTemp[0]), EPS_DeciDegCToDegC(hk->BatTemp[1]),
              EPS_DeciDegCToDegC(hk->BatTemp[2]), EPS_DeciDegCToDegC(hk->BatTemp[3]));
    OS_printf("[BP8] OVoltCount: %u | BatFault: %u\n", hk->OVoltCount, hk->BatFault);
    OS_printf("=================================================\n");
}

void EPS_PrintHk(uint8_t cspNode)
{
    switch (cspNode)
    {
        case EPS_P80_PMU_CSP_NODE:
            EPS_PrintP80PmuHk(&EPS_AppData.PMU_HkTlm.Payload);
            break;

        case EPS_P80_PDU_CSP_NODE:
            EPS_PrintP80PduHk(&EPS_AppData.PDU_HkTlm.Payload);
            break;

        case EPS_P80_ACU1_CSP_NODE:
        case EPS_P80_ACU2_CSP_NODE:
            EPS_PrintP80AcuHk(cspNode, &EPS_AppData.ACU_HkTlm.Payload);
            break;

        case EPS_BP8_CSP_NODE:
        {
            const EPS_BP8_HkTlm_Payload_t *hk = &EPS_AppData.BP8_HkTlm.Payload;
            EPS_PrintBP8Hk(hk);
            CFE_EVS_SendEvent(EPS_BP8_HK_INF_EID, CFE_EVS_EventType_INFORMATION,
                              "EPS: BP8 Telemetry - Vbat=%u mV, SOC=%.1f%%, Fault=%u",
                              hk->Vbat, EPS_SocRatioToPercent(hk->Soc), hk->BatFault);
            break;
        }

        default:
            OS_printf("[EPS] HK print skipped unknown node=%u\n", cspNode);
            break;
    }
}

gs_error_t EPS_ReadHk(uint8_t cspNode, uint32_t timeoutMs)
{
    switch (cspNode)
    {
        case EPS_P80_PMU_CSP_NODE:
        {
            EPS_P80_Drv_PMU_HkTlm_t drv_hk = {0};
            gs_error_t err = EPS_P80_Drv_PMU_GetHk(cspNode, &drv_hk, timeoutMs);
            if (err == GS_OK)
            {
                EPS_P80_PMU_HkTlm_Payload_t *hk = &EPS_AppData.PMU_HkTlm.Payload;
                hk->uptime       = drv_hk.uptime;
                hk->bootcause    = drv_hk.bootcause;
                hk->resetcause   = drv_hk.resetcause;
                hk->bootcount    = drv_hk.bootcount;
                hk->batt_v       = drv_hk.batt_v;
                hk->batt_i       = drv_hk.batt_i;
                hk->batt_mode    = drv_hk.batt_mode;
                hk->vbat_v       = drv_hk.vbat_v;
                hk->vcc_v        = drv_hk.vcc_v;
                hk->temp[0]      = drv_hk.temp[0];
                hk->temp[1]      = drv_hk.temp[1];
                memcpy(hk->out_en, drv_hk.out_en, sizeof(hk->out_en));
                memcpy(hk->out_i,  drv_hk.out_i,  sizeof(hk->out_i));
                memcpy(hk->sm_en,  drv_hk.sm_en,  sizeof(hk->sm_en));
                hk->gnd_wdt_cnt  = drv_hk.gnd_wdt_cnt;
                hk->bus_wdt_cnt  = drv_hk.bus_wdt_cnt;
                hk->gnd_wdt_left = drv_hk.gnd_wdt_left;
                hk->bus_wdt_left = drv_hk.bus_wdt_left;
            }
            return err;
        }

        case EPS_P80_PDU_CSP_NODE:
        {
            EPS_P80_Drv_PDU_HkTlm_t drv_hk = {0};
            gs_error_t err = EPS_P80_Drv_PDU_GetHk(cspNode, &drv_hk, timeoutMs);
            if (err == GS_OK)
            {
                EPS_P80_PDU_HkTlm_Payload_t *hk = &EPS_AppData.PDU_HkTlm.Payload;
                hk->uptime       = drv_hk.uptime;
                hk->bootcause    = drv_hk.bootcause;
                hk->bootcount    = drv_hk.bootcount;
                hk->resetcause   = drv_hk.resetcause;
                hk->vcc_v        = drv_hk.vcc_v;
                hk->vcc_i        = drv_hk.vcc_i;
                hk->vbat_v       = drv_hk.vbat_v;
                hk->temp         = drv_hk.temp;
                hk->batt_mode    = drv_hk.batt_mode;
                memcpy(hk->out_en, drv_hk.out_en, sizeof(hk->out_en));
                memcpy(hk->out_i,  drv_hk.out_i,  sizeof(hk->out_i));
                hk->gnd_wdt_cnt  = drv_hk.gnd_wdt_cnt;
                hk->bus_wdt_cnt  = drv_hk.bus_wdt_cnt;
                hk->gnd_wdt_left = drv_hk.gnd_wdt_left;
                hk->bus_wdt_left = drv_hk.bus_wdt_left;
            }
            return err;
        }

        case EPS_P80_ACU1_CSP_NODE:
        case EPS_P80_ACU2_CSP_NODE:
        {
            EPS_P80_Drv_ACU_HkTlm_t drv_hk = {0};
            gs_error_t err = EPS_P80_Drv_ACU_GetHk(cspNode, &drv_hk, timeoutMs);
            if (err == GS_OK)
            {
                EPS_P80_ACU_HkTlm_Payload_t *hk = &EPS_AppData.ACU_HkTlm.Payload;
                hk->uptime       = drv_hk.uptime;
                hk->bootcause    = drv_hk.bootcause;
                hk->bootcount    = drv_hk.bootcount;
                hk->resetcause   = drv_hk.resetcause;
                memcpy(hk->input_i, drv_hk.input_i, sizeof(hk->input_i));
                memcpy(hk->input_v, drv_hk.input_v, sizeof(hk->input_v));
                hk->vcc_v        = drv_hk.vcc_v;
                hk->vbat_v       = drv_hk.vbat_v;
                hk->temp[0]      = drv_hk.temp[0];
                hk->temp[1]      = drv_hk.temp[1];
                hk->temp[2]      = drv_hk.temp[2];
                hk->mppt_mode    = drv_hk.mppt_mode;
                hk->gnd_wdt_cnt  = drv_hk.gnd_wdt_cnt;
                hk->gnd_wdt_left = drv_hk.gnd_wdt_left;
            }
            return err;
        }

        case EPS_BP8_CSP_NODE:
        {
            EPS_BP8_Drv_HkTlm_t drv_hk = {0};
            gs_error_t err = EPS_BP8_Drv_GetHk(cspNode, &drv_hk, timeoutMs);
            if (err == GS_OK)
            {
                EPS_BP8_HkTlm_Payload_t *hk = &EPS_AppData.BP8_HkTlm.Payload;
                hk->Uptime        = drv_hk.Uptime;
                hk->BootCount     = drv_hk.BootCount;
                hk->BootCause     = drv_hk.BootCause;
                hk->ResetCause    = drv_hk.ResetCause;
                hk->Vbat          = drv_hk.Vbat;
                hk->Soc           = drv_hk.Soc;
                hk->Current       = drv_hk.Current;
                hk->InCurrent     = drv_hk.InCurrent;
                hk->OutCurrent    = drv_hk.OutCurrent;
                hk->HeaterCurrent = drv_hk.HeaterCurrent;
                hk->IntTemp       = drv_hk.IntTemp;
                hk->BatAvrTemp    = drv_hk.BatAvrTemp;
                hk->BatTemp[0]    = drv_hk.BatTemp[0];
                hk->BatTemp[1]    = drv_hk.BatTemp[1];
                hk->BatTemp[2]    = drv_hk.BatTemp[2];
                hk->BatTemp[3]    = drv_hk.BatTemp[3];
                hk->OVoltCount    = drv_hk.OVoltCount;
                hk->BatFault      = drv_hk.BatFault;
            }
            return err;
        }

        default:
            return GS_ERROR_ARG;
    }
}

gs_error_t EPS_RParamSet(uint8_t cspNode, uint8_t tableId, uint16_t addr, uint8_t type,
                         const uint8_t *data, uint16_t size, uint32_t timeoutMs)
{
    if (cspNode == EPS_BP8_CSP_NODE)
    {
        return EPS_BP8_Drv_ParamSet(cspNode, tableId, addr, type, data, size, timeoutMs);
    }

    return EPS_P80_Drv_ParamSet(cspNode, tableId, addr, type, data, size, timeoutMs);
}

gs_error_t EPS_RParamGet(uint8_t cspNode, uint8_t tableId, uint16_t addr, uint8_t type,
                         uint8_t *data, uint16_t size, uint32_t timeoutMs)
{
    if (cspNode == EPS_BP8_CSP_NODE)
    {
        return EPS_BP8_Drv_ParamGet(cspNode, tableId, addr, type, data, size, timeoutMs);
    }

    return EPS_P80_Drv_ParamGet(cspNode, tableId, addr, type, data, size, timeoutMs);
}

gs_error_t EPS_RParamFetchFullTable(uint8_t cspNode, uint8_t tableId,
                                    gs_param_table_instance_t *tinst, uint32_t timeoutMs)
{
    if (cspNode == EPS_BP8_CSP_NODE)
    {
        return EPS_BP8_Drv_GetFullTable(cspNode, tableId, tinst, timeoutMs);
    }

    return EPS_P80_Drv_GetFullTable(cspNode, tableId, tinst, timeoutMs);
}

gs_error_t EPS_RParamTableSave(uint8_t cspNode, uint8_t tableId, uint32_t timeoutMs)
{
    if (cspNode == EPS_BP8_CSP_NODE)
    {
        return EPS_BP8_Drv_TableSave(cspNode, tableId, timeoutMs);
    }

    return EPS_P80_Drv_TableSave(cspNode, tableId, timeoutMs);
}

gs_error_t EPS_RParamTableLoad(uint8_t cspNode, uint8_t tableId, uint32_t timeoutMs)
{
    if (cspNode == EPS_BP8_CSP_NODE)
    {
        return EPS_BP8_Drv_TableLoad(cspNode, tableId, timeoutMs);
    }

    return EPS_P80_Drv_TableLoad(cspNode, tableId, timeoutMs);
}

gs_error_t EPS_RParamSaveAll(uint8_t cspNode, uint32_t timeoutMs)
{
    if (cspNode == EPS_BP8_CSP_NODE)
    {
        return EPS_BP8_Drv_ParamSaveAll(cspNode, timeoutMs);
    }

    return EPS_P80_Drv_ParamSaveAll(cspNode, timeoutMs);
}

gs_error_t EPS_RParamSaveToStore(uint8_t cspNode, uint8_t tableId,
                                 const char *store, const char *slot,
                                 uint32_t timeoutMs)
{
    return gs_rparam_save_to_store(cspNode, timeoutMs, tableId, store, slot);
}

gs_error_t EPS_RParamLoadFromStore(uint8_t cspNode, uint8_t tableId,
                                   const char *store, const char *slot,
                                   uint32_t timeoutMs)
{
    return gs_rparam_load_from_store(cspNode, timeoutMs, tableId, store, slot);
}

const char *EPS_GetCspNodeDeviceName(uint8 cspNode)
{
    switch (cspNode)
    {
        case EPS_P80_PMU_CSP_NODE:
            return "P80 PMU";
        case EPS_P80_PDU_CSP_NODE:
            return "P80 PDU";
        case EPS_P80_ACU1_CSP_NODE:
            return "P80 ACU1";
        case EPS_P80_ACU2_CSP_NODE:
            return "P80 ACU2";
        case EPS_BP8_CSP_NODE:
            return "BP8";
        default:
            return "Unknown";
    }
}

const char *EPS_GetRParamTableName(uint8 cspNode, uint8 tableId)
{
    if (cspNode == EPS_BP8_CSP_NODE)
    {
        switch (tableId)
        {
            case EPS_BP8_TABLE_BOARD:
                return "board";
            case EPS_BP8_TABLE_CONFIGURATION:
                return "configuration";
            case EPS_BP8_TABLE_CALIBRATION:
                return "calibration";
            case EPS_BP8_TABLE_CONTROL:
                return "control";
            case EPS_BP8_TABLE_TELEMETRY:
                return "telemetry";
            default:
                return "unknown";
        }
    }

    switch (tableId)
    {
        case 0:
            return "board";
        case 1:
            return "configuration";
        case 2:
            return "calibration";
        case 4:
            return "telemetry";
        default:
            return "unknown";
    }
}

static void EPS_PrintParamTableRow(const gs_param_table_instance_t *tinst, const gs_param_table_row_t *row)
{
    char buf[128] = {0};
    unsigned int written = 0;
    uint16_t addr = GS_PARAM_ADDR(row);
    size_t elem_size = (size_t)GS_PARAM_SIZE(row);
    size_t elem_count = (size_t)GS_PARAM_ARRAY_SIZE(row);
    size_t value_size = elem_size * elem_count;

    if ((size_t)addr >= tinst->memory_size || value_size > ((size_t)tinst->memory_size - (size_t)addr))
    {
        OS_printf("  [%3u] %-14.14s <addr out of range>\n", addr, row->name);
        return;
    }

    OS_printf("  [%3u] %-14.14s", addr, row->name);
    for (size_t i = 0; i < elem_count; i++)
    {
        const void *value = (const uint8_t *)tinst->memory + addr + (elem_size * i);
        gs_error_t err = gs_param_to_string(row, value, (i == 0), buf, sizeof(buf), 0, &written);
        if (err != GS_OK)
        {
            OS_printf(" <decode err[%u]=%d>\n", (unsigned int)i, err);
            return;
        }

        OS_printf(" %s", buf);
    }

    OS_printf("\n");
}

void EPS_PrintParamTable(const char *title, uint8 cspNode, uint8 tableId,
                         const char *tableName, const gs_param_table_instance_t *tinst)
{
    const char *deviceName = EPS_GetCspNodeDeviceName(cspNode);
    const char *resolvedTableName = tableName ? tableName : EPS_GetRParamTableName(cspNode, tableId);

    OS_printf("\n================[%s]===================\n", title ? title : "EPS Param Table");
    OS_printf("  Device: %s | Node: %u | Table: %u (%s)\n",
              deviceName, cspNode, tableId, resolvedTableName);

    if (tinst == NULL)
    {
        OS_printf("  <no table instance>\n");
        OS_printf("=======================================================\n");
        return;
    }

    OS_printf("  Remote Name: %s | Rows: %u | Size: %u bytes\n",
              tinst->name ? tinst->name : "N/A", tinst->row_count, tinst->memory_size);

    if (tinst->rows == NULL || tinst->memory == NULL)
    {
        OS_printf("  <table rows or memory unavailable>\n");
        OS_printf("=======================================================\n");
        return;
    }

    for (unsigned int i = 0; i < tinst->row_count; i++)
        EPS_PrintParamTableRow(tinst, &tinst->rows[i]);

    OS_printf("=======================================================\n");
}

void EPS_PrintRParamFullTable(uint8_t cspNode, uint8_t tableId,
                              const gs_param_table_instance_t *tinst)
{
    EPS_PrintParamTable("EPS RParam Full Table", cspNode, tableId,
                        EPS_GetRParamTableName(cspNode, tableId), tinst);
}

void EPS_SendReport(const void *cmd, const void *data, uint16 dataSize, int32 retCode, uint8 retType)
{
    CFE_SB_MsgId_t    cmdMid;
    CFE_MSG_FcnCode_t cmdCode;
    uint16            copySize = 0;

    CFE_MSG_GetMsgId(cmd, &cmdMid);
    CFE_MSG_GetFcnCode(cmd, &cmdCode);

    if (data != NULL && dataSize > 0)
    {
        copySize = (dataSize > RPT_RET_VALUE_BUF_SIZE) ? RPT_RET_VALUE_BUF_SIZE : dataSize;
    }

    CFE_MSG_Init(CFE_MSG_PTR(EPS_AppData.Report.TelemetryHeader), CFE_SB_ValueToMsgId(EPS_REPORT_MID),
                 sizeof(EPS_AppData.Report));
    EPS_AppData.Report.Payload.MsgID          = (uint16_t)CFE_SB_MsgIdToValue(cmdMid);
    EPS_AppData.Report.Payload.CommandCode    = cmdCode;
    EPS_AppData.Report.Payload.ReturnType     = retType;
    EPS_AppData.Report.Payload.ReturnCode     = retCode;
    EPS_AppData.Report.Payload.ReturnDataSize = copySize;
    memset(EPS_AppData.Report.Payload.ReturnValue, 0, sizeof(EPS_AppData.Report.Payload.ReturnValue));
    if (copySize > 0)
    {
        memcpy(EPS_AppData.Report.Payload.ReturnValue, data, copySize);
    }
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(EPS_AppData.Report.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(EPS_AppData.Report.TelemetryHeader), true);
}

CFE_Status_t EPS_UpdateBcnTlmFromHw(void)
{
    EPS_BcnTlm_Full_Payload_t next_bcn = {0};
    EPS_BcnTlm_Full_Payload_t *bcn = &next_bcn;
    gs_error_t err;
    uint8_t fail_count = 0;
    uint8 invalid_mask = 0;

    /* --- PMU Beacon (node 1) --- */
    EPS_P80_Drv_PMU_BcnTlm_t pmu_bcn = {0};
    err = EPS_P80_Drv_PMU_GetBcn(EPS_P80_PMU_CSP_NODE, &pmu_bcn, CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        fail_count++;
        invalid_mask |= EPS_MarkBcnNodeFailure(bcn, EPS_BCN_NODE_PMU_INDEX, err, "PMU");
    }
    else
    {
        bcn->PMU.bootcause    = pmu_bcn.bootcause;
        bcn->PMU.resetcause   = pmu_bcn.resetcause;
        bcn->PMU.bootcount    = pmu_bcn.bootcount;
        memcpy(bcn->PMU.out_en, pmu_bcn.out_en, sizeof(bcn->PMU.out_en));
        bcn->PMU.temp[0]      = pmu_bcn.temp[0];
        bcn->PMU.temp[1]      = pmu_bcn.temp[1];
        bcn->PMU.batt_mode    = pmu_bcn.batt_mode;
        bcn->PMU.batt_i       = pmu_bcn.batt_i;
        bcn->PMU.batt_v       = pmu_bcn.batt_v;
        bcn->PMU.sm_en_mask   = pmu_bcn.sm_en_mask;
        bcn->PMU.gnd_wdt_cnt  = pmu_bcn.gnd_wdt_cnt;
        bcn->PMU.bus_wdt_cnt  = pmu_bcn.bus_wdt_cnt;
        bcn->PMU.gnd_wdt_left = pmu_bcn.gnd_wdt_left;
        bcn->PMU.bus_wdt_left = pmu_bcn.bus_wdt_left;
    }

    /* --- PDU Beacon --- */
    EPS_P80_Drv_PDU_BcnTlm_t pdu_bcn = {0};
    err = EPS_P80_Drv_PDU_GetBcn(EPS_P80_PDU_CSP_NODE, &pdu_bcn, CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        fail_count++;
        invalid_mask |= EPS_MarkBcnNodeFailure(bcn, EPS_BCN_NODE_PDU_INDEX, err, "PDU");
    }
    else
    {
        for (size_t i = 0; i < EPS_PDU_BCN_USED_CH_COUNT; i++)
        {
            bcn->PDU.out_i[i] = pdu_bcn.out_i[i];
            bcn->PDU.out_en[i] = pdu_bcn.out_en[i];
        }
    }

    /* --- ACU Beacon --- */
    static const uint8_t AcuNodes[EPS_BCN_ACU_COUNT] = {EPS_P80_ACU1_CSP_NODE, EPS_P80_ACU2_CSP_NODE};
    for (int acu = 0; acu < EPS_BCN_ACU_COUNT; acu++)
    {
        EPS_P80_Drv_ACU_BcnTlm_t acu_bcn = {0};
        uint8 node_index = EPS_BCN_NODE_ACU1_INDEX + acu;

        err = EPS_P80_Drv_ACU_GetBcn(AcuNodes[acu], &acu_bcn, CSP_TIMEOUT(1));
        if (err != GS_OK)
        {
            fail_count++;
            invalid_mask |= EPS_MarkBcnNodeFailure(bcn, node_index, err, EPS_GetCspNodeDeviceName(AcuNodes[acu]));
        }
        else
        {
            memcpy(bcn->ACU[acu].input_i, acu_bcn.input_i, sizeof(bcn->ACU[acu].input_i));
            memcpy(bcn->ACU[acu].input_v, acu_bcn.input_v, sizeof(bcn->ACU[acu].input_v));
            bcn->ACU[acu].mppt_mode = acu_bcn.mppt_mode;
        }
    }

    /* --- BP8 Beacon (node 7) --- */
    EPS_BP8_Drv_HkTlm_t bp8_hk = {0};
    err = EPS_BP8_Drv_GetHk(EPS_BP8_CSP_NODE, &bp8_hk, CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        fail_count++;
        invalid_mask |= EPS_MarkBcnNodeFailure(bcn, EPS_BCN_NODE_BP8_INDEX, err, "BP8");
    }
    else
    {
        bcn->BP8.bootcount    = bp8_hk.BootCount;
        bcn->BP8.bootcause    = bp8_hk.BootCause;
        bcn->BP8.resetcause   = bp8_hk.ResetCause;
        bcn->BP8.soc          = bp8_hk.Soc;
        bcn->BP8.bat_avr_temp = bp8_hk.BatAvrTemp;
        bcn->BP8.vbat         = bp8_hk.Vbat;
        bcn->BP8.current      = bp8_hk.Current;
        bcn->BP8.heater_i     = bp8_hk.HeaterCurrent;
    }

    if (fail_count > 0)
    {
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: BCN partial update, failed=%u zeroed=0x%02X",
                          (unsigned int)fail_count,
                          invalid_mask);
    }

    EPS_AppData.BcnTlm.Payload = next_bcn;
    return CFE_SUCCESS;
}
