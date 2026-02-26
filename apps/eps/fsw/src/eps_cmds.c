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
#include <gs/param/internal/types.h>
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
#include "eps_interface_cfg.h"

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

CFE_Status_t EPS_SendHkCmd(const EPS_SendHkCmd_t *Msg)
{
    /**
     * This command is supposed to be called by the scheduler and
     * should not increment the command counter.
     * Collects PMU and ACU1 housekeeping data and populates HkTlm.Payload.
     */
    gs_param_table_instance_t tinst = {0};
    EPS_HkTlm_Payload_t *hk = &EPS_AppData.HkTlm.Payload;

    /* --- PMU: battery, output, system data --- */
    gs_error_t err = p80_pmu_get_hk(&tinst, EPS_PMU_CSP_NODE, CSP_TIMEOUT(1));
    if (err == GS_OK && tinst.memory != NULL)
    {
        uint8_t *taddr = (uint8_t *)tinst.memory;

        hk->vbatt         = *(uint16_t *)(taddr + GS_P80_PMU_TELEMETRY_BATT_V);
        hk->counter_boot  = *(uint16_t *)(taddr + GS_P80_PMU_TELEMETRY_BOOTCOUNT);
        hk->wdt_gnd_time_left = *(uint32_t *)(taddr + GS_P80_PMU_TELEMETRY_GND_WDT_LEFT);
        hk->bootcause     = (uint8_t)(*(uint32_t *)(taddr + GS_P80_PMU_TELEMETRY_BOOTCAUSE));
        hk->battmode      = *(uint8_t  *)(taddr + GS_P80_PMU_TELEMETRY_BATT_MODE);
        hk->cursys        = (uint16_t)*(int16_t *)(taddr + GS_P80_PMU_TELEMETRY_VCC_I);
        hk->temp[0]       = *(int16_t  *)(taddr + GS_P80_PMU_TELEMETRY_TEMP(0));
        hk->temp[1]       = *(int16_t  *)(taddr + GS_P80_PMU_TELEMETRY_TEMP(1));

        /* PMU output channels (6 channels) */
        uint8_t out_en_bits = 0;
        for (int i = 0; i < 6; i++)
        {
            bool en = *(bool *)(taddr + GS_P80_PMU_TELEMETRY_OUT_EN(i));
            hk->curout[i]  = (uint16_t)*(int16_t *)(taddr + GS_P80_PMU_TELEMETRY_OUT_I(i));
            hk->latchup[i] = (uint8_t) *(uint16_t *)(taddr + GS_P80_PMU_TELEMETRY_LATCHUP(i));
            if (en) out_en_bits |= (uint8_t)(1u << i);
        }
        hk->output[0] = out_en_bits;
    }
    else
    {
        EPS_AppData.Counters.GetHkErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: SendHkCmd PMU get_hk failed, err=%d", err);
    }
    if (tinst.memory) free(tinst.memory);
    if (tinst.rows)   free((void *)tinst.rows);

    /* --- ACU1: solar input currents and temperature --- */
    memset(&tinst, 0, sizeof(tinst));
    err = p80_acu_get_hk(&tinst, EPS_ACU1_CSP_NODE, CSP_TIMEOUT(1));
    if (err == GS_OK && tinst.memory != NULL)
    {
        uint8_t *taddr = (uint8_t *)tinst.memory;
        uint16_t cursun = 0;
        for (int i = 0; i < 3; i++)
        {
            hk->curin[i] = (uint16_t)*(int16_t *)(taddr + GS_P80_ACU_TELEMETRY_INPUT_I(i));
            cursun += hk->curin[i];
        }
        hk->cursun  = cursun;
        hk->temp[2] = *(int16_t *)(taddr + GS_P80_ACU_TELEMETRY_TEMP(0));
    }
    else
    {
        EPS_AppData.Counters.GetHkErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: SendHkCmd ACU1 get_hk failed, err=%d", err);
    }
    if (tinst.memory) free(tinst.memory);
    if (tinst.rows)   free((void *)tinst.rows);

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(EPS_AppData.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(EPS_AppData.HkTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

CFE_Status_t EPS_SendBcnCmd(const EPS_SendBcnCmd_t *Msg)
{
    /**
     * This command is supposed to be called by the scheduler and
     * should not increment the command counter.
     * Collects beacon data from PMU (Dock), PDU, and ACU1 nodes.
     */
    gs_param_table_instance_t tinst = {0};
    EPS_BcnTlm_P80_Payload_t *bcn = &EPS_AppData.BcnTlm_P80.Payload;

    /* --- PMU (Dock) beacon data --- */
    gs_error_t err = p80_pmu_get_hk(&tinst, EPS_PMU_CSP_NODE, CSP_TIMEOUT(1));
    if (err == GS_OK && tinst.memory != NULL)
    {
        uint8_t *taddr = (uint8_t *)tinst.memory;
        uint16_t out_en_bits = 0;

        for (int i = 0; i < 6; i++)
        {
            bool en = *(bool *)(taddr + GS_P80_PMU_TELEMETRY_OUT_EN(i));
            bcn->Dock.c_out[i] = *(int16_t  *)(taddr + GS_P80_PMU_TELEMETRY_OUT_I(i));
            bcn->Dock.v_out[i] = *(uint16_t *)(taddr + GS_P80_PMU_TELEMETRY_OUT_V(i));
            if (en) out_en_bits |= (uint16_t)(1u << i);
        }
        /* channels 6-8 are zero-filled (PMU has 6 outputs) */

        bcn->Dock.out_en       = out_en_bits;
        bcn->Dock.bootcause    = *(uint32_t *)(taddr + GS_P80_PMU_TELEMETRY_BOOTCAUSE);
        bcn->Dock.bootcnt      = (uint32_t)*(uint16_t *)(taddr + GS_P80_PMU_TELEMETRY_BOOTCOUNT);
        bcn->Dock.batt_mode    = *(uint8_t  *)(taddr + GS_P80_PMU_TELEMETRY_BATT_MODE);
        bcn->Dock.vbat_v       = *(uint16_t *)(taddr + GS_P80_PMU_TELEMETRY_VBAT_V);
        bcn->Dock.vcc_c        = (uint16_t)*(int16_t *)(taddr + GS_P80_PMU_TELEMETRY_VCC_I);
        bcn->Dock.batt_v       = *(uint16_t *)(taddr + GS_P80_PMU_TELEMETRY_BATT_V);
        bcn->Dock.batt_temp[0] = *(int16_t  *)(taddr + GS_P80_PMU_TELEMETRY_TEMP(0));
        bcn->Dock.batt_temp[1] = *(int16_t  *)(taddr + GS_P80_PMU_TELEMETRY_TEMP(1));
        bcn->Dock.wdt_gnd_left = *(uint32_t *)(taddr + GS_P80_PMU_TELEMETRY_GND_WDT_LEFT);

        int16_t batt_i         = *(int16_t  *)(taddr + GS_P80_PMU_TELEMETRY_BATT_I);
        bcn->Dock.batt_chrg    = (batt_i > 0) ? batt_i : 0;
        bcn->Dock.batt_dischrg = (batt_i < 0) ? (int16_t)(-batt_i) : 0;
    }
    else
    {
        EPS_AppData.Counters.GetBcnErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: SendBcnCmd PMU get_hk failed, err=%d", err);
    }
    if (tinst.memory) free(tinst.memory);
    if (tinst.rows)   free((void *)tinst.rows);

    /* --- PDU beacon data (first 9 of 24 output channels) --- */
    memset(&tinst, 0, sizeof(tinst));
    err = p80_pdu_get_hk(&tinst, EPS_PDU_CSP_NODE, CSP_TIMEOUT(1));
    if (err == GS_OK && tinst.memory != NULL)
    {
        uint8_t *taddr = (uint8_t *)tinst.memory;
        uint16_t out_en_bits = 0;
        uint8_t  conv_en     = 0;

        for (int i = 0; i < 9; i++)
        {
            bool en = *(bool *)(taddr + GS_P80_PDU_TELEMETRY_OUT_EN(i));
            bcn->PDU.c_out[i] = *(int16_t  *)(taddr + GS_P80_PDU_TELEMETRY_OUT_I(i));
            bcn->PDU.v_out[i] = *(uint16_t *)(taddr + GS_P80_PDU_TELEMETRY_OUT_V(i));
            if (en) out_en_bits |= (uint16_t)(1u << i);
        }
        for (int i = 0; i < 4; i++)
        {
            bool en = *(bool *)(taddr + GS_P80_PDU_TELEMETRY_CONV_EN(i));
            if (en) conv_en |= (uint8_t)(1u << i);
        }
        bcn->PDU.out_en  = out_en_bits;
        bcn->PDU.conv_en = conv_en;
        bcn->PDU.vcc     = (int16_t)*(uint16_t *)(taddr + GS_P80_PDU_TELEMETRY_VCC_V);
    }
    else
    {
        EPS_AppData.Counters.GetBcnErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: SendBcnCmd PDU get_hk failed, err=%d", err);
    }
    if (tinst.memory) free(tinst.memory);
    if (tinst.rows)   free((void *)tinst.rows);

    /* --- ACU1 beacon data (6 solar input channels) --- */
    memset(&tinst, 0, sizeof(tinst));
    err = p80_acu_get_hk(&tinst, EPS_ACU1_CSP_NODE, CSP_TIMEOUT(1));
    if (err == GS_OK && tinst.memory != NULL)
    {
        uint8_t *taddr = (uint8_t *)tinst.memory;
        for (int i = 0; i < 6; i++)
        {
            bcn->ACU.c_in[i] = *(int16_t  *)(taddr + GS_P80_ACU_TELEMETRY_INPUT_I(i));
            bcn->ACU.v_in[i] = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_INPUT_V(i));
        }
    }
    else
    {
        EPS_AppData.Counters.GetBcnErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: SendBcnCmd ACU1 get_hk failed, err=%d", err);
    }
    if (tinst.memory) free(tinst.memory);
    if (tinst.rows)   free((void *)tinst.rows);

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(EPS_AppData.BcnTlm_P80.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(EPS_AppData.BcnTlm_P80.TelemetryHeader), true);

    return CFE_SUCCESS;
}

// CFE_Status_t EPS_ReportAppDataCmd(const EPS_ReportAppDataCmd_t *Msg)
// {


//     CFE_SB_TransmitMsg(CFE_MSG_PTR(EPS_AppData.Report.TelemetryHeader), true);
//     return CFE_SUCCESS;
// }

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


CFE_Status_t EPS_Power_If_Get_Cmd(const EPS_Power_If_Get_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    power_if_ch_status_t status = {0};

    strncpy(status.name, Msg->Payload.name, POWER_IF_NAME_LEN -1);
    status.name[POWER_IF_NAME_LEN -1] = '\0'; //the end of the string

    gs_error_t err = power_if_cmd(Msg->Payload.csp_node, GS_P80_PORT_CMDCONTROL, CSP_TIMEOUT(1), POWER_IF_GET, &status);
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Power Interface Get command failed, err=%d", err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("EPS: Power Interface Get command succeeded\n");
    OS_printf(" ch_idx = %u\n", status.ch_idx);
    OS_printf(" name = %s\n", status.name);
    OS_printf(" mode = %u\n", status.mode);
    OS_printf("  on_cnt:     %u\n", status.on_cnt); //delay
    OS_printf("  off_cnt:    %u\n", status.off_cnt);
    OS_printf("  cur_lu_lim: %u\n", status.cur_lu_lim);
    OS_printf("  cur_lim:    %u\n", status.cur_lim);
    OS_printf("  voltage:    %u mV\n", status.voltage);
    OS_printf("  current:    %d mA\n", status.current);
    OS_printf("  latchup:    %u\n", status.latchup);


    return CFE_SUCCESS;
}

CFE_Status_t EPS_Power_If_Set_Cmd(const EPS_Power_If_Set_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;
    
    power_if_ch_status_t status ={0};

    status.mode = Msg->Payload.mode;
    status.on_cnt = Msg->Payload.on_cnt;
    status.off_cnt = Msg->Payload.off_cnt;
    strncpy(status.name, Msg->Payload.name, POWER_IF_NAME_LEN -1);
    status.name[POWER_IF_NAME_LEN -1] = 0;

    gs_error_t err = power_if_cmd(Msg->Payload.csp_node, GS_P80_PORT_CMDCONTROL, CSP_TIMEOUT(1), POWER_IF_SET, &status);
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Power Interface Set command failed, err=%d", err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

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

    return CFE_SUCCESS;
}


CFE_Status_t EPS_Power_If_List_Cmd(const EPS_Power_If_List_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    power_if_cmd_list_response_t list ={0};


    gs_error_t err = power_if_cmd(Msg->Payload.csp_node, GS_P80_PORT_CMDCONTROL, CSP_TIMEOUT(1), POWER_IF_LIST, &list);
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Power Interface command failed, err=%d", err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

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

    return CFE_SUCCESS;
}




CFE_Status_t EPS_Get_Hk_Cmd(const EPS_Get_Hk_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;
    gs_param_table_instance_t tinst = {0};

    switch (Msg->Payload.csp_node)
    {
        case EPS_PMU_CSP_NODE:
        {
            gs_error_t err = p80_pmu_get_hk(&tinst, EPS_PMU_CSP_NODE, CSP_TIMEOUT(1));

            if (err != GS_OK)
            {
                EPS_AppData.Counters.ErrCounter++;
                CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                "EPS: Get PMU HK command failed, err=%d", err);
                if (tinst.memory) free(tinst.memory);
                if (tinst.rows) free((void*)tinst.rows);
                return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            }

            uint8_t *taddr = (uint8_t *)tinst.memory;

            if (taddr != NULL)
            {
                OS_printf("\n================[p80 PMU HK]===================\n");

                uint32_t uptime = *(uint32_t *)(taddr + GS_P80_PMU_TELEMETRY_UPTIME);
                uint32_t bootcause = *(uint32_t *)(taddr + GS_P80_PMU_TELEMETRY_BOOTCAUSE);
                uint16_t resetcause = *(uint16_t *)(taddr + GS_P80_PMU_TELEMETRY_RESETCAUSE);
                uint16_t bootcount = *(uint16_t *)(taddr + GS_P80_PMU_TELEMETRY_BOOTCOUNT);
                OS_printf("[SYSTEM] Uptime: %u s | BootCount: %u | Cause(Boot/Reset): %u / %u\n",
                        uptime, bootcount, bootcause, resetcause);

                OS_printf("[OUTPUT] IDX | EN | Volt(mV) | Curr(mA) | LatchUp | EMA(mA)\n");
                for (int i = 0; i < 6; i++) {
                    bool en = *(bool *)(taddr + GS_P80_PMU_TELEMETRY_OUT_EN(i));
                    uint16_t volt = *(uint16_t *)(taddr + GS_P80_PMU_TELEMETRY_OUT_V(i));
                    int16_t curr = *(int16_t *)(taddr + GS_P80_PMU_TELEMETRY_OUT_I(i));
                    uint16_t latchup = *(uint16_t *)(taddr + GS_P80_PMU_TELEMETRY_LATCHUP(i));
                    uint16_t ema = *(uint16_t *)(taddr + GS_P80_PMU_TELEMETRY_CUR_EMA(i));

                    OS_printf("         %3d | %2s | %8u | %8d | %7u | %7u\n",
                            i, en ? "ON" : "OFF", volt, curr, latchup, ema);
                }

                uint16_t batt_v    = *(uint16_t *)(taddr + GS_P80_PMU_TELEMETRY_BATT_V);
                int16_t  batt_i    = *(int16_t  *)(taddr + GS_P80_PMU_TELEMETRY_BATT_I);
                uint8_t  batt_mode = *(uint8_t  *)(taddr + GS_P80_PMU_TELEMETRY_BATT_MODE);
                uint16_t vbat_v    = *(uint16_t *)(taddr + GS_P80_PMU_TELEMETRY_VBAT_V);
                int16_t  vbat_i    = *(int16_t  *)(taddr + GS_P80_PMU_TELEMETRY_VBAT_I);
                uint16_t vcc_v     = *(uint16_t *)(taddr + GS_P80_PMU_TELEMETRY_VCC_V);
                int16_t  vcc_i     = *(int16_t  *)(taddr + GS_P80_PMU_TELEMETRY_VCC_I);
                bool     conv_5v   = *(bool     *)(taddr + GS_P80_PMU_TELEMETRY_CONV_5V_EN);
                int16_t  temp[2];
                temp[0]            = *(int16_t  *)(taddr + GS_P80_PMU_TELEMETRY_TEMP(0));
                temp[1]            = *(int16_t  *)(taddr + GS_P80_PMU_TELEMETRY_TEMP(1));

                OS_printf("[BATT]   Volt: %u mV | Curr: %d mA | Mode: %u\n", batt_v, batt_i, batt_mode);
                OS_printf("[PMU]    VBAT: %u mV (%d mA) | VCC: %u mV (%d mA)\n", vbat_v, vbat_i, vcc_v, vcc_i);
                OS_printf("[TEMP]   T0: %d | T1: %d (deci-degC) | 5V_Conv: %s\n", temp[0], temp[1], conv_5v ? "ON" : "OFF");

                OS_printf("[SUBMOD] Enable: ");
                for (int i = 0; i < 8; i++) OS_printf("%d ", *(bool *)(taddr + GS_P80_PMU_TELEMETRY_SM_EN(i)));
                OS_printf("\n");

                OS_printf("[BP_PACK] Enable: ");
                for (int i = 0; i < 4; i++) OS_printf("%d ", *(bool *)(taddr + GS_P80_PMU_TELEMETRY_BATT_EN(i)));
                OS_printf("\n");

                OS_printf("[DEVICE] Type: ");
                for (int i = 0; i < 8; i++) OS_printf("%u ", *(uint8_t *)(taddr + GS_P80_PMU_TELEMETRY_DEVICE_TYPE(i)));
                OS_printf("\n");

                OS_printf("[DEVICE] Status: ");
                for (int i = 0; i < 8; i++) OS_printf("%u ", *(uint8_t *)(taddr + GS_P80_PMU_TELEMETRY_DEVICE_STATUS(i)));
                OS_printf("\n");

                uint16_t gnd_cnt = *(uint16_t *)(taddr + GS_P80_PMU_TELEMETRY_GND_WDT_CNT);
                uint16_t bus_cnt = *(uint16_t *)(taddr + GS_P80_PMU_TELEMETRY_BUS_WDT_CNT);
                uint32_t gnd_lft = *(uint32_t *)(taddr + GS_P80_PMU_TELEMETRY_GND_WDT_LEFT);
                uint32_t bus_lft = *(uint32_t *)(taddr + GS_P80_PMU_TELEMETRY_BUS_WDT_LEFT);

                OS_printf("[WDT]    GND: %u (Left: %u s) | BUS: %u (Left: %u s)\n", gnd_cnt, gnd_lft, bus_cnt, bus_lft);

                OS_printf("[SM_WDT] Cnt : ");
                for (int i = 0; i < 8; i++) OS_printf("%u ", *(uint16_t *)(taddr + GS_P80_PMU_TELEMETRY_SM_WDT_CNT(i)));
                OS_printf("\n         Left: ");
                for (int i = 0; i < 8; i++) OS_printf("%u ", *(uint8_t  *)(taddr + GS_P80_PMU_TELEMETRY_SM_WDT_LEFT(i)));
                OS_printf("\n");

                OS_printf("[BP_WDT] Cnt : ");
                for (int i = 0; i < 4; i++) OS_printf("%u ", *(uint16_t *)(taddr + GS_P80_PMU_TELEMETRY_BATT_WDT_CNT(i)));
                OS_printf("\n         Left: ");
                for (int i = 0; i < 4; i++) OS_printf("%u ", *(uint8_t  *)(taddr + GS_P80_PMU_TELEMETRY_BATT_WDT_LEFT(i)));
                OS_printf("\n");

                bool dep_inhbt = *(bool *)(taddr + GS_P80_PMU_TELEMETRY_DEP_INHBT);
                OS_printf("[DEPLOY] Inhibit: %s\n", dep_inhbt ? "YES" : "NO");
                OS_printf("         Status: ");
                for (int i = 0; i < 12; i++) OS_printf("%d ", *(int32_t *)(taddr + GS_P80_PMU_TELEMETRY_AR6_STATUS(i)));
                OS_printf("\n         Burns : ");
                for (int i = 0; i < 12; i++) OS_printf("%d ", *(int8_t  *)(taddr + GS_P80_PMU_TELEMETRY_AR6_BURN_TRY(i)));
                OS_printf("\n");

                OS_printf("=======================================================\n");
            }

            if (tinst.memory) free(tinst.memory);
            if (tinst.rows) free((void*)tinst.rows);
            break;
        }

        case EPS_PDU_CSP_NODE:
        {
            gs_error_t err = p80_pdu_get_hk(&tinst, EPS_PDU_CSP_NODE, CSP_TIMEOUT(1));

            if (err != GS_OK)
            {
                EPS_AppData.Counters.ErrCounter++;
                CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                "EPS: Get PDU HK command failed, err=%d", err);
                if (tinst.memory) free(tinst.memory);
                if (tinst.rows) free((void*)tinst.rows);
                return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            }

            uint8_t *taddr = (uint8_t *)tinst.memory;

            if (taddr != NULL)
            {
                OS_printf("\n================[p80 PDU HK]===================\n");

                uint32_t uptime = *(uint32_t *)(taddr + GS_P80_PDU_TELEMETRY_UPTIME);
                uint32_t bootcause = *(uint32_t *)(taddr + GS_P80_PDU_TELEMETRY_BOOTCAUSE);
                uint16_t resetcause = *(uint16_t *)(taddr + GS_P80_PDU_TELEMETRY_RESETCAUSE);
                uint32_t bootcount = *(uint32_t *)(taddr + GS_P80_PDU_TELEMETRY_BOOTCOUNT);
                OS_printf("[SYSTEM] Uptime: %u s | BootCount: %u | Cause(Boot/Reset): %u / %u\n",
                        uptime, bootcount, bootcause, resetcause);

                OS_printf("[OUTPUT] IDX | EN | Volt(mV) | Curr(mA) | LatchUp | EMA(mA)\n");
                for (int i = 0; i < 24; i++) {
                    bool en = *(bool *)(taddr + GS_P80_PDU_TELEMETRY_OUT_EN(i));
                    uint16_t volt = *(uint16_t *)(taddr + GS_P80_PDU_TELEMETRY_OUT_V(i));
                    int16_t curr = *(int16_t *)(taddr + GS_P80_PDU_TELEMETRY_OUT_I(i));
                    uint16_t latchup = *(uint16_t *)(taddr + GS_P80_PDU_TELEMETRY_LATCHUP(i));
                    uint16_t ema = *(uint16_t *)(taddr + GS_P80_PDU_TELEMETRY_CUR_EMA(i));

                    OS_printf("         %3d | %2s | %8u | %8d | %7u | %7u\n",
                            i, en ? "ON" : "OFF", volt, curr, latchup, ema);
                }

                uint16_t vcc_v = *(uint16_t *)(taddr + GS_P80_PDU_TELEMETRY_VCC_V);
                uint16_t vcc_i = *(uint16_t *)(taddr + GS_P80_PDU_TELEMETRY_VCC_I);
                uint16_t vbat_v = *(uint16_t *)(taddr + GS_P80_PDU_TELEMETRY_VBAT_V);
                int16_t temp = *(int16_t *)(taddr + GS_P80_PDU_TELEMETRY_TEMP);
                uint8_t batt_mode = *(uint8_t *)(taddr + GS_P80_PDU_TELEMETRY_BATT_MODE);

                OS_printf("[PDU]    VCC: %u mV (%u mA) | VBAT: %u mV\n", vcc_v, vcc_i, vbat_v);
                OS_printf("[TEMP]   %d (deci-degC) | BattMode: %u\n", temp, batt_mode);

                OS_printf("[CONV]   IDX | EN | Volt(mV)\n");
                for (int i = 0; i < 4; i++) {
                    bool conv_en = *(bool *)(taddr + GS_P80_PDU_TELEMETRY_CONV_EN(i));
                    uint16_t conv_v = *(uint16_t *)(taddr + GS_P80_PDU_TELEMETRY_CONV_V(i));
                    OS_printf("         %3d | %2s | %8u\n", i, conv_en ? "ON" : "OFF", conv_v);
                }

                OS_printf("[DEVICE] Type  : ");
                for (int i = 0; i < 8; i++) OS_printf("%u ", *(uint8_t *)(taddr + GS_P80_PDU_TELEMETRY_DEVICE_TYPE(i)));
                OS_printf("\n");

                OS_printf("[DEVICE] Status: ");
                for (int i = 0; i < 8; i++) OS_printf("%u ", *(uint8_t *)(taddr + GS_P80_PDU_TELEMETRY_DEVICE_STATUS(i)));
                OS_printf("\n");

                uint32_t gnd_wdt_cnt = *(uint32_t *)(taddr + GS_P80_PDU_TELEMETRY_GND_WDT_CNT);
                uint32_t bus_wdt_cnt = *(uint32_t *)(taddr + GS_P80_PDU_TELEMETRY_BUS_WDT_CNT);
                uint32_t gnd_wdt_left = *(uint32_t *)(taddr + GS_P80_PDU_TELEMETRY_GND_WDT_LEFT);
                uint32_t bus_wdt_left = *(uint32_t *)(taddr + GS_P80_PDU_TELEMETRY_BUS_WDT_LEFT);

                OS_printf("[WDT]    GND: %u (Left: %u s) | BUS: %u (Left: %u s)\n",
                        gnd_wdt_cnt, gnd_wdt_left, bus_wdt_cnt, bus_wdt_left);

                OS_printf("[CSP_WDT] Cnt : ");
                for (int i = 0; i < 12; i++) OS_printf("%u ", *(uint32_t *)(taddr + GS_P80_PDU_TELEMETRY_CSP_WDT_CNT(i)));
                OS_printf("\n");

                OS_printf("          Left: ");
                for (int i = 0; i < 12; i++) OS_printf("%u ", *(uint8_t *)(taddr + GS_P80_PDU_TELEMETRY_CSP_WDT_LEFT(i)));
                OS_printf("\n");

                OS_printf("=======================================================\n");
            }

            if (tinst.memory) free(tinst.memory);
            if (tinst.rows) free((void*)tinst.rows);
            break;
        }

        case EPS_ACU1_CSP_NODE:
        {
            gs_error_t err = p80_acu_get_hk(&tinst, EPS_ACU1_CSP_NODE, CSP_TIMEOUT(1));

            if (err != GS_OK)
            {
                EPS_AppData.Counters.ErrCounter++;
                CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                "EPS: Get ACU1 HK command failed, err=%d", err);
                if (tinst.memory) free(tinst.memory);
                if (tinst.rows) free((void*)tinst.rows);
                return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            }

            uint8_t *taddr = (uint8_t *)tinst.memory;

            if (taddr != NULL)
            {
                OS_printf("\n================[p80 ACU1 HK]===================\n");

                uint32_t uptime = *(uint32_t *)(taddr + GS_P80_ACU_TELEMETRY_UPTIME);
                uint32_t bootcause = *(uint32_t *)(taddr + GS_P80_ACU_TELEMETRY_BOOTCAUSE);
                uint16_t resetcause = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_RESETCAUSE);
                uint32_t bootcount = *(uint32_t *)(taddr + GS_P80_ACU_TELEMETRY_BOOTCOUNT);
                OS_printf("[SYSTEM] Uptime: %u s | BootCount: %u | Cause(Boot/Reset): %u / %u\n",
                        uptime, bootcount, bootcause, resetcause);

                OS_printf("[INPUT]  IDX | EN | Volt(mV) | Curr(mA) | Power(mW) | PV_Set(mV) | DAC\n");
                for (int i = 0; i < 6; i++) {
                    bool en = *(bool *)(taddr + GS_P80_ACU_TELEMETRY_CHANNEL_EN(i));
                    uint16_t volt = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_INPUT_V(i));
                    int16_t curr = *(int16_t *)(taddr + GS_P80_ACU_TELEMETRY_INPUT_I(i));
                    uint16_t power = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_POWER(i));
                    uint16_t pv_set = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_PV_SETPOINT(i));
                    uint16_t dac = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_DAC_VAL(i));

                    OS_printf("         %3d | %2s | %8u | %8d | %9u | %10u | %5u\n",
                            i, en ? "ON" : "OFF", volt, curr, power, pv_set, dac);
                }

                uint16_t vcc_v = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_VCC_V);
                int16_t vcc_i = *(int16_t *)(taddr + GS_P80_ACU_TELEMETRY_VCC_I);
                uint16_t st5_v = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_ST5_V);
                int16_t st5_i = *(int16_t *)(taddr + GS_P80_ACU_TELEMETRY_ST5_I);
                uint16_t vbat_v = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_VBAT_V);

                OS_printf("[ACU1]    VCC: %u mV (%d mA) | ST5V: %u mV (%d mA) | VBAT: %u mV\n",
                        vcc_v, vcc_i, st5_v, st5_i, vbat_v);

                int16_t temp[3];
                for (int i = 0; i < 3; i++) {
                    temp[i] = *(int16_t *)(taddr + GS_P80_ACU_TELEMETRY_TEMP(i));
                }
                OS_printf("[TEMP]   T0: %d | T1: %d | T2: %d (deci-degC)\n", temp[0], temp[1], temp[2]);

                uint8_t mppt_mode = *(uint8_t *)(taddr + GS_P80_ACU_TELEMETRY_MPPT_MODE);
                uint16_t mppt_limit = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_MPPT_LIMIT);
                uint16_t mppt_time = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_MPPT_TIME);
                uint16_t mppt_period = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_MPPT_PERIOD);

                OS_printf("[MPPT]   Mode: %u | Limit: %u | Time: %u | Period: %u\n",
                        mppt_mode, mppt_limit, mppt_time, mppt_period);

                OS_printf("[DEVICE] Type  : ");
                for (int i = 0; i < 8; i++) OS_printf("%u ", *(uint8_t *)(taddr + GS_P80_ACU_TELEMETRY_DEVICE_TYPE(i)));
                OS_printf("\n");

                OS_printf("[DEVICE] Status: ");
                for (int i = 0; i < 8; i++) OS_printf("%u ", *(uint8_t *)(taddr + GS_P80_ACU_TELEMETRY_DEVICE_STATUS(i)));
                OS_printf("\n");

                uint32_t gnd_wdt_cnt = *(uint32_t *)(taddr + GS_P80_ACU_TELEMETRY_GND_WDT_CNT);
                uint32_t gnd_wdt_left = *(uint32_t *)(taddr + GS_P80_ACU_TELEMETRY_GND_WDT_LEFT);

                OS_printf("[WDT]    GND: %u (Left: %u s)\n", gnd_wdt_cnt, gnd_wdt_left);

                OS_printf("=======================================================\n");
            }

            if (tinst.memory) free(tinst.memory);
            if (tinst.rows) free((void*)tinst.rows);
            break;
        }

        case EPS_ACU2_CSP_NODE:
        {
            gs_error_t err = p80_acu_get_hk(&tinst, EPS_ACU2_CSP_NODE, CSP_TIMEOUT(1));

            if (err != GS_OK)
            {
                EPS_AppData.Counters.ErrCounter++;
                CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                "EPS: Get ACU2 HK command failed, err=%d", err);
                if (tinst.memory) free(tinst.memory);
                if (tinst.rows) free((void*)tinst.rows);
                return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            }

            uint8_t *taddr = (uint8_t *)tinst.memory;

            if (taddr != NULL)
            {
                OS_printf("\n================[p80 ACU2 HK]===================\n");

                uint32_t uptime = *(uint32_t *)(taddr + GS_P80_ACU_TELEMETRY_UPTIME);
                uint32_t bootcause = *(uint32_t *)(taddr + GS_P80_ACU_TELEMETRY_BOOTCAUSE);
                uint16_t resetcause = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_RESETCAUSE);
                uint32_t bootcount = *(uint32_t *)(taddr + GS_P80_ACU_TELEMETRY_BOOTCOUNT);
                OS_printf("[SYSTEM] Uptime: %u s | BootCount: %u | Cause(Boot/Reset): %u / %u\n",
                        uptime, bootcount, bootcause, resetcause);

                OS_printf("[INPUT]  IDX | EN | Volt(mV) | Curr(mA) | Power(mW) | PV_Set(mV) | DAC\n");
                for (int i = 0; i < 6; i++) {
                    bool en = *(bool *)(taddr + GS_P80_ACU_TELEMETRY_CHANNEL_EN(i));
                    uint16_t volt = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_INPUT_V(i));
                    int16_t curr = *(int16_t *)(taddr + GS_P80_ACU_TELEMETRY_INPUT_I(i));
                    uint16_t power = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_POWER(i));
                    uint16_t pv_set = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_PV_SETPOINT(i));
                    uint16_t dac = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_DAC_VAL(i));

                    OS_printf("         %3d | %2s | %8u | %8d | %9u | %10u | %5u\n",
                            i, en ? "ON" : "OFF", volt, curr, power, pv_set, dac);
                }

                uint16_t vcc_v = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_VCC_V);
                int16_t vcc_i = *(int16_t *)(taddr + GS_P80_ACU_TELEMETRY_VCC_I);
                uint16_t st5_v = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_ST5_V);
                int16_t st5_i = *(int16_t *)(taddr + GS_P80_ACU_TELEMETRY_ST5_I);
                uint16_t vbat_v = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_VBAT_V);

                OS_printf("[ACU2]    VCC: %u mV (%d mA) | ST5V: %u mV (%d mA) | VBAT: %u mV\n",
                        vcc_v, vcc_i, st5_v, st5_i, vbat_v);

                int16_t temp[3];
                for (int i = 0; i < 3; i++) {
                    temp[i] = *(int16_t *)(taddr + GS_P80_ACU_TELEMETRY_TEMP(i));
                }
                OS_printf("[TEMP]   T0: %d | T1: %d | T2: %d (deci-degC)\n", temp[0], temp[1], temp[2]);

                uint8_t mppt_mode = *(uint8_t *)(taddr + GS_P80_ACU_TELEMETRY_MPPT_MODE);
                uint16_t mppt_limit = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_MPPT_LIMIT);
                uint16_t mppt_time = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_MPPT_TIME);
                uint16_t mppt_period = *(uint16_t *)(taddr + GS_P80_ACU_TELEMETRY_MPPT_PERIOD);

                OS_printf("[MPPT]   Mode: %u | Limit: %u | Time: %u | Period: %u\n",
                        mppt_mode, mppt_limit, mppt_time, mppt_period);

                OS_printf("[DEVICE] Type  : ");
                for (int i = 0; i < 8; i++) OS_printf("%u ", *(uint8_t *)(taddr + GS_P80_ACU_TELEMETRY_DEVICE_TYPE(i)));
                OS_printf("\n");

                OS_printf("[DEVICE] Status: ");
                for (int i = 0; i < 8; i++) OS_printf("%u ", *(uint8_t *)(taddr + GS_P80_ACU_TELEMETRY_DEVICE_STATUS(i)));
                OS_printf("\n");

                uint32_t gnd_wdt_cnt = *(uint32_t *)(taddr + GS_P80_ACU_TELEMETRY_GND_WDT_CNT);
                uint32_t gnd_wdt_left = *(uint32_t *)(taddr + GS_P80_ACU_TELEMETRY_GND_WDT_LEFT);

                OS_printf("[WDT]    GND: %u (Left: %u s)\n", gnd_wdt_cnt, gnd_wdt_left);

                OS_printf("=======================================================\n");
            }

            if (tinst.memory) free(tinst.memory);
            if (tinst.rows) free((void*)tinst.rows);
            break;
        }
        default:
            EPS_AppData.Counters.ErrCounter++;
            CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                            "EPS: Get HK command failed (wrong node arg)");
            return CFE_STATUS_RANGE_ERROR;
            break;
    }

    

    

    return CFE_SUCCESS;
}

CFE_Status_t EPS_Gnd_Watchdog_Clear_Cmd(const EPS_Gnd_Watchdog_Clear_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    switch (Msg->Payload.csp_node)
    {
        case EPS_PMU_CSP_NODE:
        {
            gs_error_t err = p80_pmu_gndwdt_clear(EPS_PMU_CSP_NODE, CSP_TIMEOUT(1));
            if (err != GS_OK)
            {
                EPS_AppData.Counters.ErrCounter++;
                CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "EPS: PMU GND WDT clear failed");
                return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            }
            break;
        }

        case EPS_PDU_CSP_NODE:
        {
            gs_error_t err = p80_pdu_gndwdt_clear(EPS_PDU_CSP_NODE, CSP_TIMEOUT(1));
            if (err != GS_OK)
            {
                EPS_AppData.Counters.ErrCounter++;
                CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "EPS: PDU GND WDT clear failed");
                return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            }
            break;
        }

        case EPS_ACU1_CSP_NODE:
        {
            gs_error_t err = p80_acu_gndwdt_clear(EPS_ACU1_CSP_NODE, CSP_TIMEOUT(1));
            if (err != GS_OK)
            {
                EPS_AppData.Counters.ErrCounter++;
                CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "EPS: ACU1 GND WDT clear failed");
                return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            }
            break;
        }

        case EPS_ACU2_CSP_NODE:
        {
            gs_error_t err = p80_acu_gndwdt_clear(EPS_ACU2_CSP_NODE, CSP_TIMEOUT(1));
            if (err != GS_OK)
            {
                EPS_AppData.Counters.ErrCounter++;
                CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "EPS: ACU2 GND WDT clear failed");
                return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            }
            break;
        }

        default:
            EPS_AppData.Counters.ErrCounter++;
            CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                            "EPS: Get Gnd Watchdog command failed (wrong node arg)");
            return CFE_STATUS_RANGE_ERROR;
    }

    
    return CFE_SUCCESS;
}


CFE_Status_t EPS_Param_Set_Cmd(const EPS_Param_Set_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    gs_error_t err = gs_rparam_set(Msg->Payload.csp_node,
                                   Msg->Payload.table_id,
                                   Msg->Payload.addr,
                                   Msg->Payload.type,
                                   GS_RPARAM_MAGIC_CHECKSUM,
                                   CSP_TIMEOUT(1),
                                   Msg->Payload.data,
                                   Msg->Payload.size);

    if(err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: RParam Set command failed, err=%d", err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}

CFE_Status_t EPS_Param_Get_Cmd(const EPS_Param_Get_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    gs_error_t err = gs_rparam_get(Msg->Payload.csp_node,
                                   Msg->Payload.table_id,
                                   Msg->Payload.addr,
                                   Msg->Payload.type,
                                   GS_RPARAM_MAGIC_CHECKSUM,
                                   CSP_TIMEOUT(1),
                                   (void *)Msg->Payload.data,
                                   Msg->Payload.size);

    if(err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: RParam Get command failed, err=%d", err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}

CFE_Status_t EPS_Get_Full_Table_Cmd(const EPS_Get_Full_Table_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    uint16_t checksum;
    gs_param_table_instance_t tinst = {0};

    gs_error_t result = gs_rparam_download_table_spec(&tinst, NULL, Msg->Payload.csp_node, Msg->Payload.table_id, CSP_TIMEOUT(1), &checksum);

    if (result){
        return result;
    }

    gs_error_t err =  gs_rparam_get_full_table(&tinst, Msg->Payload.csp_node, Msg->Payload.table_id, checksum, CSP_TIMEOUT(1));

    if(err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: RParam Get Full Table command failed, err=%d", err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}





CFE_Status_t EPS_Table_Save_Cmd(const EPS_Table_Save_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    gs_error_t err = gs_rparam_save(Msg->Payload.csp_node, CSP_TIMEOUT(1), Msg->Payload.table_id, 0);

    if(err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Table Save command failed, err=%d", err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}

CFE_Status_t EPS_Table_Load_Cmd(const EPS_Table_Load_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    gs_error_t err = gs_rparam_load(Msg->Payload.csp_node, CSP_TIMEOUT(1), 0, Msg->Payload.table_id);

    if(err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Table Load command failed, err=%d", err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}

CFE_Status_t EPS_Param_Save_Cmd(const EPS_Param_Save_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    /* Save all parameter tables (table_id = 0xFF means all tables) */
    gs_error_t err = gs_rparam_save(Msg->Payload.csp_node, CSP_TIMEOUT(1), 0xFF, 0);

    if(err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Param Save (all tables) command failed, err=%d", err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    CFE_EVS_SendEvent(EPS_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "EPS: Param Save (all tables) succeeded on node %u", Msg->Payload.csp_node);

    return CFE_SUCCESS;
}