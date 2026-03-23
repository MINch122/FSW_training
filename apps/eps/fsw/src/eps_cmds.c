/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as "core Flight System: Bootes"
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
 *   This file contains the source code for the EPS App Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "eps_app.h"
#include "eps_cmds.h"
#include "eps_msgids.h"
#include "eps_eventids.h"
#include "eps_version.h"
#include "eps_msg.h"
#include "cfe_srl_csp.h"
#include "eps_interface_cfg.h"

#include "eps_p80_drv.h"
#include "eps_bp8_drv.h"

void EPS_SendReport(const void* cmd,
                    const void* data,
                    uint16 dataSize,
                    int32 retCode,
                    uint8 retType)
{
    CFE_SB_MsgId_t cmdMid;
    CFE_MSG_FcnCode_t cmdCode;

    CFE_MSG_GetMsgId(cmd, &cmdMid);
    CFE_MSG_GetFcnCode(cmd, &cmdCode);

    CFE_MSG_Init(CFE_MSG_PTR(EPS_AppData.Report.TelemetryHeader),
                 CFE_SB_ValueToMsgId(EPS_REPORT_MID),
                 sizeof(EPS_AppData.Report));
    EPS_AppData.Report.Payload.MsgID = (uint16_t)CFE_SB_MsgIdToValue(cmdMid);
    EPS_AppData.Report.Payload.CommandCode = cmdCode;
    EPS_AppData.Report.Payload.ReturnType = retType;
    EPS_AppData.Report.Payload.ReturnCode = retCode;
    EPS_AppData.Report.Payload.ReturnDataSize = dataSize;
    if (data && dataSize)
        memcpy(EPS_AppData.Report.Payload.ReturnValue,
               data,
               dataSize > RPT_RET_VALUE_BUF_SIZE
                        ? RPT_RET_VALUE_BUF_SIZE
                        : dataSize);
   CFE_SB_TransmitMsg(CFE_MSG_PTR(EPS_AppData.Report.TelemetryHeader), true);
}

// CFE_Status_t EPS_SendHkCmd(const EPS_SendHkCmd_t *Msg)
// {
//     /* TODO: re-implement using driver functions */
//     return CFE_SUCCESS;
// }

CFE_Status_t EPS_SendBcnCmd(const EPS_SendBcnCmd_t *Msg)
{
    EPS_BcnTlm_Full_Payload_t *bcn = &EPS_AppData.BcnTlm.Payload;
    gs_error_t err;

    /* --- PMU Beacon (node 1) --- */
    EPS_P80_Drv_PMU_BcnTlm_t pmu_bcn = {0};
    err = EPS_P80_Drv_PMU_GetBcn(EPS_P80_PMU_CSP_NODE, &pmu_bcn, CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.GetBcnErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: BCN PMU fetch failed, err=%d", err);
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
        memcpy(bcn->PMU.sm_en, pmu_bcn.sm_en, sizeof(bcn->PMU.sm_en));
        bcn->PMU.gnd_wdt_cnt  = pmu_bcn.gnd_wdt_cnt;
        bcn->PMU.bus_wdt_cnt  = pmu_bcn.bus_wdt_cnt;
        bcn->PMU.gnd_wdt_left = pmu_bcn.gnd_wdt_left;
        bcn->PMU.bus_wdt_left = pmu_bcn.bus_wdt_left;
    }

    /* --- PDU Beacon (node 10) --- */
    EPS_P80_Drv_PDU_BcnTlm_t pdu_bcn = {0};
    err = EPS_P80_Drv_PDU_GetBcn(EPS_P80_PDU_CSP_NODE, &pdu_bcn, CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.GetBcnErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: BCN PDU fetch failed, err=%d", err);
    }
    else
    {
        memcpy(bcn->PDU.out_en, pdu_bcn.out_en, sizeof(bcn->PDU.out_en));
    }

    /* --- ACU Beacon (node 2) --- */
    EPS_P80_Drv_ACU_BcnTlm_t acu_bcn = {0};
    err = EPS_P80_Drv_ACU_GetBcn(EPS_P80_ACU1_CSP_NODE, &acu_bcn, CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.GetBcnErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: BCN ACU fetch failed, err=%d", err);
    }
    else
    {
        memcpy(bcn->ACU.input_i, acu_bcn.input_i, sizeof(bcn->ACU.input_i));
        memcpy(bcn->ACU.input_v, acu_bcn.input_v, sizeof(bcn->ACU.input_v));
        bcn->ACU.mppt_mode = acu_bcn.mppt_mode;
    }

    /* --- BP8 Beacon (node 7) --- */
    EPS_BP8_Drv_HkTlm_t bp8_hk = {0};
    err = EPS_BP8_Drv_GetHk(EPS_BP8_CSP_NODE, &bp8_hk, CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.GetBcnErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: BCN BP8 fetch failed, err=%d", err);
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

    /* Timestamp and transmit beacon on SB */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(EPS_AppData.BcnTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(EPS_AppData.BcnTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

// CFE_Status_t EPS_ReportAppDataCmd(const EPS_ReportAppDataCmd_t *Msg)
// {
//     CFE_SB_TransmitMsg(CFE_MSG_PTR(EPS_AppData.Report.TelemetryHeader), true);
//     return CFE_SUCCESS;
// }

CFE_Status_t EPS_ReportBcnCmd(const EPS_ReportBcnCmd_t *Msg)
{
    EPS_BcnTlm_Full_Payload_t *bcn = &EPS_AppData.BcnTlm.Payload;

    OS_printf("\n================[EPS BCN Report]===================\n");
    OS_printf("[PMU] BootCause: %u | ResetCause: %u | BootCount: %u\n",
              bcn->PMU.bootcause, bcn->PMU.resetcause, bcn->PMU.bootcount);
    OS_printf("[PMU] BattV: %u mV | BattI: %d mA | Mode: %u\n",
              bcn->PMU.batt_v, bcn->PMU.batt_i, bcn->PMU.batt_mode);
    OS_printf("[PMU] Temp: %d / %d (ddegC)\n", bcn->PMU.temp[0], bcn->PMU.temp[1]);
    OS_printf("[PMU] OutEn: ");
    for (int i = 0; i < 6; i++) OS_printf("%u ", bcn->PMU.out_en[i]);
    OS_printf("\n");
    OS_printf("[PMU] SmEn: ");
    for (int i = 0; i < 8; i++) OS_printf("%u ", bcn->PMU.sm_en[i]);
    OS_printf("\n");
    OS_printf("[PMU] WDT GND: %u (Left: %u s) | BUS: %u (Left: %u s)\n",
              bcn->PMU.gnd_wdt_cnt, bcn->PMU.gnd_wdt_left,
              bcn->PMU.bus_wdt_cnt, bcn->PMU.bus_wdt_left);
    OS_printf("[PDU] OutEn: ");
    for (int i = 0; i < 24; i++) OS_printf("%u ", bcn->PDU.out_en[i]);
    OS_printf("\n");
    OS_printf("[ACU] InputI(mA): ");
    for (int i = 0; i < 6; i++) OS_printf("%d ", bcn->ACU.input_i[i]);
    OS_printf("\n");
    OS_printf("[ACU] InputV(mV): ");
    for (int i = 0; i < 6; i++) OS_printf("%u ", bcn->ACU.input_v[i]);
    OS_printf("\n");
    OS_printf("[ACU] MPPT Mode: %u\n", bcn->ACU.mppt_mode);
    OS_printf("[BP8] BootCount: %u | BootCause: %u | ResetCause: %u\n",
              bcn->BP8.bootcount, bcn->BP8.bootcause, bcn->BP8.resetcause);
    OS_printf("[BP8] SOC: %.2f | Vbat: %u mV | Current: %.3f A | HeaterI: %u mA\n",
              (double)bcn->BP8.soc, bcn->BP8.vbat,
              (double)bcn->BP8.current, bcn->BP8.heater_i);
    OS_printf("[BP8] BatAvrTemp: %.1f degC\n", (double)bcn->BP8.bat_avr_temp);
    OS_printf("=======================================================\n");

    EPS_SendReport(Msg, bcn, sizeof(*bcn), CFE_SUCCESS, RPT_RETTYPE_SUCCESS);

    CFE_EVS_SendEvent(EPS_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "EPS: BCN report sent (%u bytes)", (unsigned)sizeof(*bcn));

    return CFE_SUCCESS;
}

CFE_Status_t EPS_NoopCmd(const EPS_NoopCmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    CFE_EVS_SendEvent(EPS_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "EPS: NOOP command %s",
                      EPS_VERSION);

    return CFE_SUCCESS;
}

CFE_Status_t EPS_ResetCountersCmd(const EPS_ResetCountersCmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter = 0;
    EPS_AppData.Counters.ErrCounter = 0;
    EPS_AppData.Counters.GetHkErrCounter = 0;
    EPS_AppData.Counters.GetBcnErrCounter = 0;

    CFE_EVS_SendEvent(EPS_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "EPS: RESET command");

    return CFE_SUCCESS;
}


/* ========================================================================== */
/*  P80 Power Interface Commands                                              */
/* ========================================================================== */

CFE_Status_t EPS_P80_Power_If_Get_Cmd(const EPS_P80_Power_If_Get_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    gs_error_t err = EPS_P80_Drv_PowerIfGet(Msg->Payload.csp_node, Msg->Payload.name, CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Power Interface Get command failed, err=%d", err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}

CFE_Status_t EPS_P80_Power_If_Set_Cmd(const EPS_P80_Power_If_Set_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    gs_error_t err = EPS_P80_Drv_PowerIfSet(Msg->Payload.csp_node, Msg->Payload.name,
                                             Msg->Payload.mode, Msg->Payload.on_cnt,
                                             Msg->Payload.off_cnt, CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Power Interface Set command failed, err=%d", err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}


CFE_Status_t EPS_P80_Power_If_List_Cmd(const EPS_P80_Power_If_List_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    gs_error_t err = EPS_P80_Drv_PowerIfList(Msg->Payload.csp_node, CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Power Interface command failed, err=%d", err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}


/* ========================================================================== */
/*  P80 Housekeeping Command                                                  */
/* ========================================================================== */

CFE_Status_t EPS_P80_Get_Hk_Cmd(const EPS_P80_Get_Hk_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;
    gs_error_t err;

    switch (Msg->Payload.csp_node)
    {
        case EPS_P80_PMU_CSP_NODE:
        {
            EPS_P80_Drv_PMU_HkTlm_t drv_hk = {0};
            err = EPS_P80_Drv_PMU_GetHk(EPS_P80_PMU_CSP_NODE, &drv_hk, CSP_TIMEOUT(1));
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
            break;
        }

        case EPS_P80_PDU_CSP_NODE:
        {
            EPS_P80_Drv_PDU_HkTlm_t drv_hk = {0};
            err = EPS_P80_Drv_PDU_GetHk(EPS_P80_PDU_CSP_NODE, &drv_hk, CSP_TIMEOUT(1));
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
            break;
        }

        case EPS_P80_ACU1_CSP_NODE:
        case EPS_P80_ACU2_CSP_NODE:
        {
            EPS_P80_Drv_ACU_HkTlm_t drv_hk = {0};
            err = EPS_P80_Drv_ACU_GetHk(Msg->Payload.csp_node, &drv_hk, CSP_TIMEOUT(1));
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
            break;
        }

        default:
            EPS_AppData.Counters.ErrCounter++;
            CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "EPS: Get HK command failed (wrong node arg)");
            return CFE_STATUS_RANGE_ERROR;
    }

    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Get HK command failed on node %u, err=%d",
                          Msg->Payload.csp_node, err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}


/* ========================================================================== */
/*  P80 Ground Watchdog Clear                                                 */
/* ========================================================================== */

CFE_Status_t EPS_P80_Gnd_Watchdog_Clear_Cmd(const EPS_P80_Gnd_Watchdog_Clear_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;
    gs_error_t err;

    switch (Msg->Payload.csp_node)
    {
        case EPS_P80_PMU_CSP_NODE:
            err = EPS_P80_Drv_PMU_GndWdtClear(EPS_P80_PMU_CSP_NODE, CSP_TIMEOUT(1));
            break;

        case EPS_P80_PDU_CSP_NODE:
            err = EPS_P80_Drv_PDU_GndWdtClear(EPS_P80_PDU_CSP_NODE, CSP_TIMEOUT(1));
            break;

        case EPS_P80_ACU1_CSP_NODE:
            err = EPS_P80_Drv_ACU_GndWdtClear(EPS_P80_ACU1_CSP_NODE, CSP_TIMEOUT(1));
            break;

        case EPS_P80_ACU2_CSP_NODE:
            err = EPS_P80_Drv_ACU_GndWdtClear(EPS_P80_ACU2_CSP_NODE, CSP_TIMEOUT(1));
            break;

        default:
            EPS_AppData.Counters.ErrCounter++;
            CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "EPS: Get Gnd Watchdog command failed (wrong node arg)");
            return CFE_STATUS_RANGE_ERROR;
    }

    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: GND WDT clear failed on node %u, err=%d",
                          Msg->Payload.csp_node, err);
        OS_printf("[EPS] GND WDT Clear FAILED on node %u, err=%d\n",
                  Msg->Payload.csp_node, err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("[EPS] GND WDT Clear OK on node %u\n", Msg->Payload.csp_node);

    return CFE_SUCCESS;
}


/* ========================================================================== */
/*  P80 Remote Parameter Commands                                             */
/* ========================================================================== */

CFE_Status_t EPS_P80_Param_Set_Cmd(const EPS_P80_Param_Set_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    gs_error_t err = EPS_P80_Drv_ParamSet(Msg->Payload.csp_node,
                                           Msg->Payload.table_id,
                                           Msg->Payload.addr,
                                           Msg->Payload.type,
                                           Msg->Payload.data,
                                           Msg->Payload.size,
                                           CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: RParam Set command failed, err=%d", err);
        OS_printf("[EPS] RParam Set FAILED node=%u table=%u addr=%u type=%u size=%u err=%d\n",
                  Msg->Payload.csp_node, Msg->Payload.table_id,
                  Msg->Payload.addr, Msg->Payload.type, Msg->Payload.size, err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("[EPS] RParam Set OK node=%u table=%u addr=%u type=%u size=%u\n",
              Msg->Payload.csp_node, Msg->Payload.table_id,
              Msg->Payload.addr, Msg->Payload.type, Msg->Payload.size);

    return CFE_SUCCESS;
}

CFE_Status_t EPS_P80_Param_Get_Cmd(const EPS_P80_Param_Get_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    gs_error_t err = EPS_P80_Drv_ParamGet(Msg->Payload.csp_node,
                                           Msg->Payload.table_id,
                                           Msg->Payload.addr,
                                           Msg->Payload.type,
                                           (uint8_t *)Msg->Payload.data,
                                           Msg->Payload.size,
                                           CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: RParam Get command failed, err=%d", err);
        OS_printf("[EPS] RParam Get FAILED node=%u table=%u addr=%u type=%u size=%u err=%d\n",
                  Msg->Payload.csp_node, Msg->Payload.table_id,
                  Msg->Payload.addr, Msg->Payload.type, Msg->Payload.size, err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("[EPS] RParam Get OK node=%u table=%u addr=%u type=%u size=%u data=",
              Msg->Payload.csp_node, Msg->Payload.table_id,
              Msg->Payload.addr, Msg->Payload.type, Msg->Payload.size);
    for (uint16_t i = 0; i < Msg->Payload.size && i < 32; i++)
        OS_printf("%02X ", Msg->Payload.data[i]);
    OS_printf("\n");

    return CFE_SUCCESS;
}

CFE_Status_t EPS_P80_Get_Full_Table_Cmd(const EPS_P80_Get_Full_Table_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    gs_error_t err = EPS_P80_Drv_GetFullTable(Msg->Payload.csp_node,
                                               Msg->Payload.table_id,
                                               CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: RParam Get Full Table command failed, err=%d", err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}


/* ========================================================================== */
/*  P80 Table Save/Load Commands                                              */
/* ========================================================================== */

CFE_Status_t EPS_P80_Table_Save_Cmd(const EPS_P80_Table_Save_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    gs_error_t err = EPS_P80_Drv_TableSave(Msg->Payload.csp_node,
                                            Msg->Payload.table_id,
                                            CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Table Save command failed, err=%d", err);
        OS_printf("[EPS] Table Save FAILED node=%u table=%u err=%d\n",
                  Msg->Payload.csp_node, Msg->Payload.table_id, err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("[EPS] Table Save OK node=%u table=%u\n",
              Msg->Payload.csp_node, Msg->Payload.table_id);

    return CFE_SUCCESS;
}

CFE_Status_t EPS_P80_Table_Load_Cmd(const EPS_P80_Table_Load_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    gs_error_t err = EPS_P80_Drv_TableLoad(Msg->Payload.csp_node,
                                            Msg->Payload.table_id,
                                            CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Table Load command failed, err=%d", err);
        OS_printf("[EPS] Table Load FAILED node=%u table=%u err=%d\n",
                  Msg->Payload.csp_node, Msg->Payload.table_id, err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("[EPS] Table Load OK node=%u table=%u\n",
              Msg->Payload.csp_node, Msg->Payload.table_id);

    return CFE_SUCCESS;
}

CFE_Status_t EPS_P80_Param_Save_Cmd(const EPS_P80_Param_Save_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    gs_error_t err = EPS_P80_Drv_ParamSaveAll(Msg->Payload.csp_node, CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Param Save (all tables) command failed, err=%d", err);
        OS_printf("[EPS] Param SaveAll FAILED node=%u err=%d\n",
                  Msg->Payload.csp_node, err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("[EPS] Param SaveAll OK node=%u\n", Msg->Payload.csp_node);

    CFE_EVS_SendEvent(EPS_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "EPS: Param Save (all tables) succeeded on node %u", Msg->Payload.csp_node);

    return CFE_SUCCESS;
}


/* ========================================================================== */
/*  BP8 Battery Pack Command Handlers                                         */
/* ========================================================================== */

CFE_Status_t EPS_BP8_GetHkCmd(const EPS_BP8_GetHkCmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    EPS_BP8_Drv_HkTlm_t drv_hk = {0};
    gs_error_t err = EPS_BP8_Drv_GetHk(EPS_BP8_CSP_NODE, &drv_hk, CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: BP8 GetHk command failed, err=%d", err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    /* Copy driver HK data to app telemetry payload */
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

    OS_printf("\n================[EPS BP8 HK]===================\n");
    OS_printf("[BP8] Uptime: %u s | BootCount: %u | BootCause: %u | ResetCause: %u\n",
              hk->Uptime, hk->BootCount, hk->BootCause, hk->ResetCause);
    OS_printf("[BP8] Vbat: %u mV | SOC: %.2f | Current: %.3f A\n",
              hk->Vbat, (double)hk->Soc, (double)hk->Current);
    OS_printf("[BP8] InCurrent: %.3f A | OutCurrent: %.3f A | HeaterI: %u mA\n",
              (double)hk->InCurrent, (double)hk->OutCurrent, hk->HeaterCurrent);
    OS_printf("[BP8] IntTemp: %.1f | BatAvrTemp: %.1f degC\n",
              (double)hk->IntTemp, (double)hk->BatAvrTemp);
    OS_printf("[BP8] BatTemp: %.1f / %.1f / %.1f / %.1f degC\n",
              (double)hk->BatTemp[0], (double)hk->BatTemp[1],
              (double)hk->BatTemp[2], (double)hk->BatTemp[3]);
    OS_printf("[BP8] OVoltCount: %u | BatFault: %u\n", hk->OVoltCount, hk->BatFault);
    OS_printf("=================================================\n");

    CFE_EVS_SendEvent(EPS_BP8_HK_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "EPS: BP8 Telemetry - Vbat=%u mV, SOC=%.2f, Fault=%u",
                      hk->Vbat, (double)hk->Soc, hk->BatFault);

    return CFE_SUCCESS;
}

CFE_Status_t EPS_BP8_SetHeaterCmd(const EPS_BP8_SetHeaterCmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    uint16_t duration = Msg->Payload.Duration;
    gs_error_t err = EPS_BP8_Drv_SetHeater(EPS_BP8_CSP_NODE, duration, CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: BP8 set heater command failed, err=%d", err);
        OS_printf("[EPS] BP8 SetHeater FAILED duration=%u err=%d\n", duration, err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("[EPS] BP8 SetHeater OK duration=%u s\n", duration);

    CFE_EVS_SendEvent(EPS_BP8_HEATER_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "EPS: BP8 manual heater set to %u seconds", duration);

    return CFE_SUCCESS;
}

CFE_Status_t EPS_BP8_ResetFaultCmd(const EPS_BP8_ResetFaultCmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    gs_error_t err = EPS_BP8_Drv_ResetFault(EPS_BP8_CSP_NODE, CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: BP8 reset fault command failed, err=%d", err);
        OS_printf("[EPS] BP8 ResetFault FAILED err=%d\n", err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("[EPS] BP8 ResetFault OK\n");

    CFE_EVS_SendEvent(EPS_BP8_FAULT_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "EPS: BP8 battery fault reset command sent");

    return CFE_SUCCESS;
}
