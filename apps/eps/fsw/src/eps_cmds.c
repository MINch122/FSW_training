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
#include <gs/param/table.h>
#include <stdbool.h>
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

// CFE_Status_t EPS_SendHkCmd(const EPS_SendHkCmd_t *Msg)
// {
//     /**
//      * This command is supposed to be called by the scheduler and
//      * should not increment the command counter.
//      * Collects PMU and ACU1 housekeeping data and populates HkTlm.Payload.
//      */
//     gs_param_table_instance_t tinst = {0};
//     EPS_HkTlm_Payload_t *hk = &EPS_AppData.HkTlm.Payload;
//     gs_error_t perr;

//     /* --- PMU: battery, output, system data --- */
//     gs_error_t err = p80_pmu_get_hk(&tinst, EPS_PMU_CSP_NODE, CSP_TIMEOUT(1));
//     if (err == GS_OK && tinst.memory != NULL)
//     {
//         uint32_t bootcause_raw = 0;
//         int16_t  vcc_i = 0;

//         perr  = GS_OK;
//         perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_BATT_V,        &hk->vbatt,              0);
//         perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_BOOTCOUNT,     &hk->counter_boot,       0);
//         perr |= gs_param_get_uint32(&tinst, GS_P80_PMU_TELEMETRY_GND_WDT_LEFT,  &hk->wdt_gnd_time_left,  0);
//         perr |= gs_param_get_uint32(&tinst, GS_P80_PMU_TELEMETRY_BOOTCAUSE,     &bootcause_raw,          0);
//         hk->bootcause = (uint8_t)bootcause_raw;
//         perr |= gs_param_get_uint8 (&tinst, GS_P80_PMU_TELEMETRY_BATT_MODE,     &hk->battmode,           0);
//         perr |= gs_param_get_int16 (&tinst, GS_P80_PMU_TELEMETRY_VCC_I,         &vcc_i,                  0);
//         hk->cursys = (uint16_t)vcc_i;
//         perr |= gs_param_get_int16 (&tinst, GS_P80_PMU_TELEMETRY_TEMP(0),       &hk->temp[0],            0);
//         perr |= gs_param_get_int16 (&tinst, GS_P80_PMU_TELEMETRY_TEMP(1),       &hk->temp[1],            0);

//         /* PMU output channels (6 channels) */
//         uint8_t out_en_bits = 0;
//         for (int i = 0; i < 6; i++)
//         {
//             bool en = false;
//             int16_t out_i = 0;
//             uint16_t latchup_raw = 0;
//             perr |= gs_param_get_bool  (&tinst, GS_P80_PMU_TELEMETRY_OUT_EN(i),  &en,          0);
//             perr |= gs_param_get_int16 (&tinst, GS_P80_PMU_TELEMETRY_OUT_I(i),   &out_i,       0);
//             perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_LATCHUP(i), &latchup_raw, 0);
//             hk->curout[i]  = (uint16_t)out_i;
//             hk->latchup[i] = (uint8_t)latchup_raw;
//             if (en) out_en_bits |= (uint8_t)(1u << i);
//         }
//         hk->output[0] = out_en_bits;

//         if (perr != GS_OK)
//         {
//             CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
//                               "EPS: SendHkCmd PMU param parse error, err=%d", perr);
//         }
//     }
//     else
//     {
//         EPS_AppData.Counters.GetHkErrCounter++;
//         CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
//                           "EPS: SendHkCmd PMU get_hk failed, err=%d", err);
//     }
//     if (tinst.memory) free(tinst.memory);
//     if (tinst.rows)   free((void *)tinst.rows);

//     /* --- ACU1: solar input currents and temperature --- */
//     memset(&tinst, 0, sizeof(tinst));
//     err = p80_acu_get_hk(&tinst, EPS_ACU1_CSP_NODE, CSP_TIMEOUT(1));
//     if (err == GS_OK && tinst.memory != NULL)
//     {
//         perr = GS_OK;
//         uint16_t cursun = 0;
//         for (int i = 0; i < 3; i++)
//         {
//             int16_t input_i = 0;
//             perr |= gs_param_get_int16(&tinst, GS_P80_ACU_TELEMETRY_INPUT_I(i), &input_i, 0);
//             hk->curin[i] = (uint16_t)input_i;
//             cursun += hk->curin[i];
//         }
//         hk->cursun = cursun;
//         perr |= gs_param_get_int16(&tinst, GS_P80_ACU_TELEMETRY_TEMP(0), &hk->temp[2], 0);

//         if (perr != GS_OK)
//         {
//             CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
//                               "EPS: SendHkCmd ACU1 param parse error, err=%d", perr);
//         }
//     }
//     else
//     {
//         EPS_AppData.Counters.GetHkErrCounter++;
//         CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
//                           "EPS: SendHkCmd ACU1 get_hk failed, err=%d", err);
//     }
//     if (tinst.memory) free(tinst.memory);
//     if (tinst.rows)   free((void *)tinst.rows);

//     CFE_SB_TimeStampMsg(CFE_MSG_PTR(EPS_AppData.HkTlm.TelemetryHeader));
//     CFE_SB_TransmitMsg(CFE_MSG_PTR(EPS_AppData.HkTlm.TelemetryHeader), true);

//     return CFE_SUCCESS;
// }

// CFE_Status_t EPS_SendBcnCmd(const EPS_SendBcnCmd_t *Msg)
// {
//     /**
//      * This command is supposed to be called by the scheduler and
//      * should not increment the command counter.
//      * Collects beacon data from PMU (Dock), PDU, and ACU1 nodes.
//      */
//     gs_param_table_instance_t tinst = {0};
//     EPS_BcnTlm_P80_Payload_t *bcn = &EPS_AppData.BcnTlm_P80.Payload;
//     gs_error_t perr;

//     /* --- PMU (Dock) beacon data --- */
//     gs_error_t err = p80_pmu_get_hk(&tinst, EPS_PMU_CSP_NODE, CSP_TIMEOUT(1));
//     if (err == GS_OK && tinst.memory != NULL)
//     {
//         perr = GS_OK;
//         uint16_t out_en_bits = 0;

//         for (int i = 0; i < 6; i++)
//         {
//             bool en = false;
//             int16_t  c_out_tmp = 0;
//             uint16_t v_out_tmp = 0;
//             perr |= gs_param_get_bool  (&tinst, GS_P80_PMU_TELEMETRY_OUT_EN(i), &en,        0);
//             perr |= gs_param_get_int16 (&tinst, GS_P80_PMU_TELEMETRY_OUT_I(i),  &c_out_tmp, 0);
//             perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_OUT_V(i),  &v_out_tmp, 0);
//             bcn->Dock.c_out[i] = c_out_tmp;
//             bcn->Dock.v_out[i] = v_out_tmp;
//             if (en) out_en_bits |= (uint16_t)(1u << i);
//         }
//         /* channels 6-8 are zero-filled (PMU has 6 outputs) */

//         bcn->Dock.out_en = out_en_bits;

//         uint32_t bootcause_tmp = 0;
//         uint16_t bootcount_raw = 0;
//         uint8_t  batt_mode_tmp = 0;
//         uint16_t vbat_v_tmp = 0;
//         int16_t  vcc_i = 0;
//         uint16_t batt_v_tmp = 0;
//         int16_t  batt_temp0 = 0, batt_temp1 = 0;
//         uint32_t wdt_gnd_left_tmp = 0;
//         int16_t  batt_i = 0;

//         perr |= gs_param_get_uint32(&tinst, GS_P80_PMU_TELEMETRY_BOOTCAUSE,    &bootcause_tmp,    0);
//         perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_BOOTCOUNT,    &bootcount_raw,    0);
//         perr |= gs_param_get_uint8 (&tinst, GS_P80_PMU_TELEMETRY_BATT_MODE,    &batt_mode_tmp,    0);
//         perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_VBAT_V,       &vbat_v_tmp,       0);
//         perr |= gs_param_get_int16 (&tinst, GS_P80_PMU_TELEMETRY_VCC_I,        &vcc_i,            0);
//         perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_BATT_V,       &batt_v_tmp,       0);
//         perr |= gs_param_get_int16 (&tinst, GS_P80_PMU_TELEMETRY_TEMP(0),      &batt_temp0,       0);
//         perr |= gs_param_get_int16 (&tinst, GS_P80_PMU_TELEMETRY_TEMP(1),      &batt_temp1,       0);
//         perr |= gs_param_get_uint32(&tinst, GS_P80_PMU_TELEMETRY_GND_WDT_LEFT, &wdt_gnd_left_tmp, 0);
//         perr |= gs_param_get_int16 (&tinst, GS_P80_PMU_TELEMETRY_BATT_I,       &batt_i,           0);

//         bcn->Dock.bootcause    = bootcause_tmp;
//         bcn->Dock.bootcnt      = (uint32_t)bootcount_raw;
//         bcn->Dock.batt_mode    = batt_mode_tmp;
//         bcn->Dock.vbat_v       = vbat_v_tmp;
//         bcn->Dock.vcc_c        = (uint16_t)vcc_i;
//         bcn->Dock.batt_v       = batt_v_tmp;
//         bcn->Dock.batt_temp[0] = batt_temp0;
//         bcn->Dock.batt_temp[1] = batt_temp1;
//         bcn->Dock.wdt_gnd_left = wdt_gnd_left_tmp;
//         bcn->Dock.batt_chrg    = (batt_i > 0) ? batt_i : 0;
//         bcn->Dock.batt_dischrg = (batt_i < 0) ? (int16_t)(-batt_i) : 0;

//         if (perr != GS_OK)
//         {
//             CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
//                               "EPS: SendBcnCmd PMU param parse error, err=%d", perr);
//         }
//     }
//     else
//     {
//         EPS_AppData.Counters.GetBcnErrCounter++;
//         CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
//                           "EPS: SendBcnCmd PMU get_hk failed, err=%d", err);
//     }
//     if (tinst.memory) free(tinst.memory);
//     if (tinst.rows)   free((void *)tinst.rows);

//     /* --- PDU beacon data (first 9 of 24 output channels) --- */
//     memset(&tinst, 0, sizeof(tinst));
//     err = p80_pdu_get_hk(&tinst, EPS_PDU_CSP_NODE, CSP_TIMEOUT(1));
//     if (err == GS_OK && tinst.memory != NULL)
//     {
//         perr = GS_OK;
//         uint16_t out_en_bits = 0;
//         uint8_t  conv_en     = 0;

//         for (int i = 0; i < 9; i++)
//         {
//             bool en = false;
//             int16_t  c_out_tmp = 0;
//             uint16_t v_out_tmp = 0;
//             perr |= gs_param_get_bool  (&tinst, GS_P80_PDU_TELEMETRY_OUT_EN(i), &en,        0);
//             perr |= gs_param_get_int16 (&tinst, GS_P80_PDU_TELEMETRY_OUT_I(i),  &c_out_tmp, 0);
//             perr |= gs_param_get_uint16(&tinst, GS_P80_PDU_TELEMETRY_OUT_V(i),  &v_out_tmp, 0);
//             bcn->PDU.c_out[i] = c_out_tmp;
//             bcn->PDU.v_out[i] = v_out_tmp;
//             if (en) out_en_bits |= (uint16_t)(1u << i);
//         }
//         for (int i = 0; i < 4; i++)
//         {
//             bool en = false;
//             perr |= gs_param_get_bool(&tinst, GS_P80_PDU_TELEMETRY_CONV_EN(i), &en, 0);
//             if (en) conv_en |= (uint8_t)(1u << i);
//         }
//         bcn->PDU.out_en  = out_en_bits;
//         bcn->PDU.conv_en = conv_en;

//         uint16_t vcc_v = 0;
//         perr |= gs_param_get_uint16(&tinst, GS_P80_PDU_TELEMETRY_VCC_V, &vcc_v, 0);
//         bcn->PDU.vcc = (int16_t)vcc_v;

//         if (perr != GS_OK)
//         {
//             CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
//                               "EPS: SendBcnCmd PDU param parse error, err=%d", perr);
//         }
//     }
//     else
//     {
//         EPS_AppData.Counters.GetBcnErrCounter++;
//         CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
//                           "EPS: SendBcnCmd PDU get_hk failed, err=%d", err);
//     }
//     if (tinst.memory) free(tinst.memory);
//     if (tinst.rows)   free((void *)tinst.rows);

//     /* --- ACU1 beacon data (6 solar input channels) --- */
//     memset(&tinst, 0, sizeof(tinst));
//     err = p80_acu_get_hk(&tinst, EPS_ACU1_CSP_NODE, CSP_TIMEOUT(1));
//     if (err == GS_OK && tinst.memory != NULL)
//     {
//         perr = GS_OK;
//         for (int i = 0; i < 6; i++)
//         {
//             int16_t  c_in_tmp = 0;
//             uint16_t v_in_tmp = 0;
//             perr |= gs_param_get_int16 (&tinst, GS_P80_ACU_TELEMETRY_INPUT_I(i), &c_in_tmp, 0);
//             perr |= gs_param_get_uint16(&tinst, GS_P80_ACU_TELEMETRY_INPUT_V(i), &v_in_tmp, 0);
//             bcn->ACU.c_in[i] = c_in_tmp;
//             bcn->ACU.v_in[i] = v_in_tmp;
//         }

//         if (perr != GS_OK)
//         {
//             CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
//                               "EPS: SendBcnCmd ACU1 param parse error, err=%d", perr);
//         }
//     }
//     else
//     {
//         EPS_AppData.Counters.GetBcnErrCounter++;
//         CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
//                           "EPS: SendBcnCmd ACU1 get_hk failed, err=%d", err);
//     }
//     if (tinst.memory) free(tinst.memory);
//     if (tinst.rows)   free((void *)tinst.rows);

//     CFE_SB_TimeStampMsg(CFE_MSG_PTR(EPS_AppData.BcnTlm_P80.TelemetryHeader));
//     CFE_SB_TransmitMsg(CFE_MSG_PTR(EPS_AppData.BcnTlm_P80.TelemetryHeader), true);

//     return CFE_SUCCESS;
// }

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

            gs_error_t perr = GS_OK;
            uint32_t uptime = 0, bootcause = 0;
            uint16_t resetcause = 0, bootcount = 0;
            uint16_t batt_v = 0, vbat_v = 0, vcc_v = 0;
            int16_t  batt_i = 0, vbat_i = 0, vcc_i = 0;
            uint8_t  batt_mode = 0;
            bool     conv_5v = false;
            int16_t  temp[2] = {0};
            uint16_t gnd_cnt = 0, bus_cnt = 0;
            uint32_t gnd_lft = 0, bus_lft = 0;
            bool     dep_inhbt = false;

            perr |= gs_param_get_uint32(&tinst, GS_P80_PMU_TELEMETRY_UPTIME,      &uptime,     0);
            perr |= gs_param_get_uint32(&tinst, GS_P80_PMU_TELEMETRY_BOOTCAUSE,   &bootcause,  0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_RESETCAUSE,  &resetcause, 0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_BOOTCOUNT,   &bootcount,  0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_BATT_V,      &batt_v,     0);
            perr |= gs_param_get_int16 (&tinst, GS_P80_PMU_TELEMETRY_BATT_I,      &batt_i,     0);
            perr |= gs_param_get_uint8 (&tinst, GS_P80_PMU_TELEMETRY_BATT_MODE,   &batt_mode,  0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_VBAT_V,      &vbat_v,     0);
            perr |= gs_param_get_int16 (&tinst, GS_P80_PMU_TELEMETRY_VBAT_I,      &vbat_i,     0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_VCC_V,       &vcc_v,      0);
            perr |= gs_param_get_int16 (&tinst, GS_P80_PMU_TELEMETRY_VCC_I,       &vcc_i,      0);
            perr |= gs_param_get_bool  (&tinst, GS_P80_PMU_TELEMETRY_CONV_5V_EN,  &conv_5v,    0);
            perr |= gs_param_get_int16 (&tinst, GS_P80_PMU_TELEMETRY_TEMP(0),     &temp[0],    0);
            perr |= gs_param_get_int16 (&tinst, GS_P80_PMU_TELEMETRY_TEMP(1),     &temp[1],    0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_GND_WDT_CNT,  &gnd_cnt,   0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_PMU_TELEMETRY_BUS_WDT_CNT,  &bus_cnt,   0);
            perr |= gs_param_get_uint32(&tinst, GS_P80_PMU_TELEMETRY_GND_WDT_LEFT, &gnd_lft,   0);
            perr |= gs_param_get_uint32(&tinst, GS_P80_PMU_TELEMETRY_BUS_WDT_LEFT, &bus_lft,   0);
            perr |= gs_param_get_bool  (&tinst, GS_P80_PMU_TELEMETRY_DEP_INHBT,    &dep_inhbt, 0);

            if (perr != GS_OK)
            {
                EPS_AppData.Counters.ErrCounter++;
                CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                "EPS: Failed to parse PMU telemetry params, err=%d", perr);
                if (tinst.memory) free(tinst.memory);
                if (tinst.rows) free((void*)tinst.rows);
                return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            }

            OS_printf("\n================[p80 PMU HK]===================\n");

            OS_printf("[SYSTEM] Uptime: %u s | BootCount: %u | Cause(Boot/Reset): %u / %u\n",
                    uptime, bootcount, bootcause, resetcause);

            OS_printf("[OUTPUT] IDX | EN | Volt(mV) | Curr(mA) | LatchUp | EMA(mA)\n");
            for (int i = 0; i < 6; i++) {
                bool     en      = gs_param_get_bool_nc  (&tinst, GS_P80_PMU_TELEMETRY_OUT_EN(i),  0);
                uint16_t volt    = gs_param_get_uint16_nc(&tinst, GS_P80_PMU_TELEMETRY_OUT_V(i),   0);
                int16_t  curr    = gs_param_get_int16_nc (&tinst, GS_P80_PMU_TELEMETRY_OUT_I(i),   0);
                uint16_t latchup = gs_param_get_uint16_nc(&tinst, GS_P80_PMU_TELEMETRY_LATCHUP(i), 0);
                uint16_t ema     = gs_param_get_uint16_nc(&tinst, GS_P80_PMU_TELEMETRY_CUR_EMA(i), 0);
                OS_printf("         %3d | %2s | %8u | %8d | %7u | %7u\n",
                        i, en ? "ON" : "OFF", volt, curr, latchup, ema);
            }

            OS_printf("[BATT]   Volt: %u mV | Curr: %d mA | Mode: %u\n", batt_v, batt_i, batt_mode);
            OS_printf("[PMU]    VBAT: %u mV (%d mA) | VCC: %u mV (%d mA)\n", vbat_v, vbat_i, vcc_v, vcc_i);
            OS_printf("[TEMP]   T0: %d | T1: %d (deci-degC) | 5V_Conv: %s\n", temp[0], temp[1], conv_5v ? "ON" : "OFF");

            OS_printf("[SUBMOD] Enable: ");
            for (int i = 0; i < 8; i++) OS_printf("%d ", gs_param_get_bool_nc(&tinst, GS_P80_PMU_TELEMETRY_SM_EN(i), 0));
            OS_printf("\n");

            OS_printf("[BP_PACK] Enable: ");
            for (int i = 0; i < 4; i++) OS_printf("%d ", gs_param_get_bool_nc(&tinst, GS_P80_PMU_TELEMETRY_BATT_EN(i), 0));
            OS_printf("\n");

            OS_printf("[DEVICE] Type: ");
            for (int i = 0; i < 8; i++) OS_printf("%u ", gs_param_get_uint8_nc(&tinst, GS_P80_PMU_TELEMETRY_DEVICE_TYPE(i), 0));
            OS_printf("\n");

            OS_printf("[DEVICE] Status: ");
            for (int i = 0; i < 8; i++) OS_printf("%u ", gs_param_get_uint8_nc(&tinst, GS_P80_PMU_TELEMETRY_DEVICE_STATUS(i), 0));
            OS_printf("\n");

            OS_printf("[WDT]    GND: %u (Left: %u s) | BUS: %u (Left: %u s)\n", gnd_cnt, gnd_lft, bus_cnt, bus_lft);

            OS_printf("[SM_WDT] Cnt : ");
            for (int i = 0; i < 8; i++) OS_printf("%u ", gs_param_get_uint16_nc(&tinst, GS_P80_PMU_TELEMETRY_SM_WDT_CNT(i), 0));
            OS_printf("\n         Left: ");
            for (int i = 0; i < 8; i++) OS_printf("%u ", gs_param_get_uint8_nc(&tinst, GS_P80_PMU_TELEMETRY_SM_WDT_LEFT(i), 0));
            OS_printf("\n");

            OS_printf("[BP_WDT] Cnt : ");
            for (int i = 0; i < 4; i++) OS_printf("%u ", gs_param_get_uint16_nc(&tinst, GS_P80_PMU_TELEMETRY_BATT_WDT_CNT(i), 0));
            OS_printf("\n         Left: ");
            for (int i = 0; i < 4; i++) OS_printf("%u ", gs_param_get_uint8_nc(&tinst, GS_P80_PMU_TELEMETRY_BATT_WDT_LEFT(i), 0));
            OS_printf("\n");

            OS_printf("[DEPLOY] Inhibit: %s\n", dep_inhbt ? "YES" : "NO");
            OS_printf("         Status: ");
            for (int i = 0; i < 12; i++) OS_printf("%d ", gs_param_get_int32_nc(&tinst, GS_P80_PMU_TELEMETRY_AR6_STATUS(i), 0));
            OS_printf("\n         Burns : ");
            for (int i = 0; i < 12; i++) OS_printf("%d ", gs_param_get_int8_nc(&tinst, GS_P80_PMU_TELEMETRY_AR6_BURN_TRY(i), 0));
            OS_printf("\n");

            OS_printf("=======================================================\n");

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

            gs_error_t perr = GS_OK;
            uint32_t uptime = 0, bootcause = 0, bootcount = 0;
            uint16_t resetcause = 0;
            uint16_t vcc_v = 0, vcc_i = 0, vbat_v = 0;
            int16_t  temp = 0;
            uint8_t  batt_mode = 0;
            uint32_t gnd_wdt_cnt = 0, bus_wdt_cnt = 0;
            uint32_t gnd_wdt_left = 0, bus_wdt_left = 0;

            perr |= gs_param_get_uint32(&tinst, GS_P80_PDU_TELEMETRY_UPTIME,       &uptime,       0);
            perr |= gs_param_get_uint32(&tinst, GS_P80_PDU_TELEMETRY_BOOTCAUSE,    &bootcause,    0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_PDU_TELEMETRY_RESETCAUSE,   &resetcause,   0);
            perr |= gs_param_get_uint32(&tinst, GS_P80_PDU_TELEMETRY_BOOTCOUNT,    &bootcount,    0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_PDU_TELEMETRY_VCC_V,        &vcc_v,        0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_PDU_TELEMETRY_VCC_I,        &vcc_i,        0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_PDU_TELEMETRY_VBAT_V,       &vbat_v,       0);
            perr |= gs_param_get_int16 (&tinst, GS_P80_PDU_TELEMETRY_TEMP,         &temp,         0);
            perr |= gs_param_get_uint8 (&tinst, GS_P80_PDU_TELEMETRY_BATT_MODE,    &batt_mode,    0);
            perr |= gs_param_get_uint32(&tinst, GS_P80_PDU_TELEMETRY_GND_WDT_CNT,  &gnd_wdt_cnt,  0);
            perr |= gs_param_get_uint32(&tinst, GS_P80_PDU_TELEMETRY_BUS_WDT_CNT,  &bus_wdt_cnt,  0);
            perr |= gs_param_get_uint32(&tinst, GS_P80_PDU_TELEMETRY_GND_WDT_LEFT, &gnd_wdt_left, 0);
            perr |= gs_param_get_uint32(&tinst, GS_P80_PDU_TELEMETRY_BUS_WDT_LEFT, &bus_wdt_left, 0);

            if (perr != GS_OK)
            {
                EPS_AppData.Counters.ErrCounter++;
                CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                "EPS: Failed to parse PDU telemetry params, err=%d", perr);
                if (tinst.memory) free(tinst.memory);
                if (tinst.rows) free((void*)tinst.rows);
                return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            }

            OS_printf("\n================[p80 PDU HK]===================\n");

            OS_printf("[SYSTEM] Uptime: %u s | BootCount: %u | Cause(Boot/Reset): %u / %u\n",
                    uptime, bootcount, bootcause, resetcause);

            OS_printf("[OUTPUT] IDX | EN | Volt(mV) | Curr(mA) | LatchUp | EMA(mA)\n");
            for (int i = 0; i < 24; i++) {
                bool     en      = gs_param_get_bool_nc  (&tinst, GS_P80_PDU_TELEMETRY_OUT_EN(i),  0);
                uint16_t volt    = gs_param_get_uint16_nc(&tinst, GS_P80_PDU_TELEMETRY_OUT_V(i),   0);
                int16_t  curr    = gs_param_get_int16_nc (&tinst, GS_P80_PDU_TELEMETRY_OUT_I(i),   0);
                uint16_t latchup = gs_param_get_uint16_nc(&tinst, GS_P80_PDU_TELEMETRY_LATCHUP(i), 0);
                uint16_t ema     = gs_param_get_uint16_nc(&tinst, GS_P80_PDU_TELEMETRY_CUR_EMA(i), 0);
                OS_printf("         %3d | %2s | %8u | %8d | %7u | %7u\n",
                        i, en ? "ON" : "OFF", volt, curr, latchup, ema);
            }

            OS_printf("[PDU]    VCC: %u mV (%u mA) | VBAT: %u mV\n", vcc_v, vcc_i, vbat_v);
            OS_printf("[TEMP]   %d (deci-degC) | BattMode: %u\n", temp, batt_mode);

            OS_printf("[CONV]   IDX | EN | Volt(mV)\n");
            for (int i = 0; i < 4; i++) {
                bool     conv_en = gs_param_get_bool_nc  (&tinst, GS_P80_PDU_TELEMETRY_CONV_EN(i), 0);
                uint16_t conv_v  = gs_param_get_uint16_nc(&tinst, GS_P80_PDU_TELEMETRY_CONV_V(i),  0);
                OS_printf("         %3d | %2s | %8u\n", i, conv_en ? "ON" : "OFF", conv_v);
            }

            OS_printf("[DEVICE] Type  : ");
            for (int i = 0; i < 8; i++) OS_printf("%u ", gs_param_get_uint8_nc(&tinst, GS_P80_PDU_TELEMETRY_DEVICE_TYPE(i), 0));
            OS_printf("\n");

            OS_printf("[DEVICE] Status: ");
            for (int i = 0; i < 8; i++) OS_printf("%u ", gs_param_get_uint8_nc(&tinst, GS_P80_PDU_TELEMETRY_DEVICE_STATUS(i), 0));
            OS_printf("\n");

            OS_printf("[WDT]    GND: %u (Left: %u s) | BUS: %u (Left: %u s)\n",
                    gnd_wdt_cnt, gnd_wdt_left, bus_wdt_cnt, bus_wdt_left);

            OS_printf("[CSP_WDT] Cnt : ");
            for (int i = 0; i < 12; i++) OS_printf("%u ", gs_param_get_uint32_nc(&tinst, GS_P80_PDU_TELEMETRY_CSP_WDT_CNT(i), 0));
            OS_printf("\n");

            OS_printf("          Left: ");
            for (int i = 0; i < 12; i++) OS_printf("%u ", gs_param_get_uint8_nc(&tinst, GS_P80_PDU_TELEMETRY_CSP_WDT_LEFT(i), 0));
            OS_printf("\n");

            OS_printf("=======================================================\n");

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

            gs_error_t perr = GS_OK;
            uint32_t uptime = 0, bootcause = 0, bootcount = 0;
            uint16_t resetcause = 0;
            uint16_t vcc_v = 0, st5_v = 0, vbat_v = 0;
            int16_t  vcc_i = 0, st5_i = 0;
            int16_t  temp[3] = {0};
            uint8_t  mppt_mode = 0;
            uint16_t mppt_limit = 0, mppt_time = 0, mppt_period = 0;
            uint32_t gnd_wdt_cnt = 0, gnd_wdt_left = 0;

            perr |= gs_param_get_uint32(&tinst, GS_P80_ACU_TELEMETRY_UPTIME,      &uptime,      0);
            perr |= gs_param_get_uint32(&tinst, GS_P80_ACU_TELEMETRY_BOOTCAUSE,   &bootcause,   0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_ACU_TELEMETRY_RESETCAUSE,  &resetcause,  0);
            perr |= gs_param_get_uint32(&tinst, GS_P80_ACU_TELEMETRY_BOOTCOUNT,   &bootcount,   0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_ACU_TELEMETRY_VCC_V,       &vcc_v,       0);
            perr |= gs_param_get_int16 (&tinst, GS_P80_ACU_TELEMETRY_VCC_I,       &vcc_i,       0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_ACU_TELEMETRY_ST5_V,       &st5_v,       0);
            perr |= gs_param_get_int16 (&tinst, GS_P80_ACU_TELEMETRY_ST5_I,       &st5_i,       0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_ACU_TELEMETRY_VBAT_V,      &vbat_v,      0);
            perr |= gs_param_get_int16 (&tinst, GS_P80_ACU_TELEMETRY_TEMP(0),     &temp[0],     0);
            perr |= gs_param_get_int16 (&tinst, GS_P80_ACU_TELEMETRY_TEMP(1),     &temp[1],     0);
            perr |= gs_param_get_int16 (&tinst, GS_P80_ACU_TELEMETRY_TEMP(2),     &temp[2],     0);
            perr |= gs_param_get_uint8 (&tinst, GS_P80_ACU_TELEMETRY_MPPT_MODE,   &mppt_mode,   0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_ACU_TELEMETRY_MPPT_LIMIT,  &mppt_limit,  0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_ACU_TELEMETRY_MPPT_TIME,   &mppt_time,   0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_ACU_TELEMETRY_MPPT_PERIOD, &mppt_period, 0);
            perr |= gs_param_get_uint32(&tinst, GS_P80_ACU_TELEMETRY_GND_WDT_CNT,  &gnd_wdt_cnt,  0);
            perr |= gs_param_get_uint32(&tinst, GS_P80_ACU_TELEMETRY_GND_WDT_LEFT, &gnd_wdt_left, 0);

            if (perr != GS_OK)
            {
                EPS_AppData.Counters.ErrCounter++;
                CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                "EPS: Failed to parse ACU1 telemetry params, err=%d", perr);
                if (tinst.memory) free(tinst.memory);
                if (tinst.rows) free((void*)tinst.rows);
                return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            }

            OS_printf("\n================[p80 ACU1 HK]===================\n");

            OS_printf("[SYSTEM] Uptime: %u s | BootCount: %u | Cause(Boot/Reset): %u / %u\n",
                    uptime, bootcount, bootcause, resetcause);

            OS_printf("[INPUT]  IDX | EN | Volt(mV) | Curr(mA) | Power(mW) | PV_Set(mV) | DAC\n");
            for (int i = 0; i < 6; i++) {
                bool     en     = gs_param_get_bool_nc  (&tinst, GS_P80_ACU_TELEMETRY_CHANNEL_EN(i),  0);
                uint16_t volt   = gs_param_get_uint16_nc(&tinst, GS_P80_ACU_TELEMETRY_INPUT_V(i),     0);
                int16_t  curr   = gs_param_get_int16_nc (&tinst, GS_P80_ACU_TELEMETRY_INPUT_I(i),     0);
                uint16_t power  = gs_param_get_uint16_nc(&tinst, GS_P80_ACU_TELEMETRY_POWER(i),       0);
                uint16_t pv_set = gs_param_get_uint16_nc(&tinst, GS_P80_ACU_TELEMETRY_PV_SETPOINT(i), 0);
                uint16_t dac    = gs_param_get_uint16_nc(&tinst, GS_P80_ACU_TELEMETRY_DAC_VAL(i),     0);
                OS_printf("         %3d | %2s | %8u | %8d | %9u | %10u | %5u\n",
                        i, en ? "ON" : "OFF", volt, curr, power, pv_set, dac);
            }

            OS_printf("[ACU1]    VCC: %u mV (%d mA) | ST5V: %u mV (%d mA) | VBAT: %u mV\n",
                    vcc_v, vcc_i, st5_v, st5_i, vbat_v);
            OS_printf("[TEMP]   T0: %d | T1: %d | T2: %d (deci-degC)\n", temp[0], temp[1], temp[2]);
            OS_printf("[MPPT]   Mode: %u | Limit: %u | Time: %u | Period: %u\n",
                    mppt_mode, mppt_limit, mppt_time, mppt_period);

            OS_printf("[DEVICE] Type  : ");
            for (int i = 0; i < 8; i++) OS_printf("%u ", gs_param_get_uint8_nc(&tinst, GS_P80_ACU_TELEMETRY_DEVICE_TYPE(i), 0));
            OS_printf("\n");

            OS_printf("[DEVICE] Status: ");
            for (int i = 0; i < 8; i++) OS_printf("%u ", gs_param_get_uint8_nc(&tinst, GS_P80_ACU_TELEMETRY_DEVICE_STATUS(i), 0));
            OS_printf("\n");

            OS_printf("[WDT]    GND: %u (Left: %u s)\n", gnd_wdt_cnt, gnd_wdt_left);

            OS_printf("=======================================================\n");

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

            gs_error_t perr = GS_OK;
            uint32_t uptime = 0, bootcause = 0, bootcount = 0;
            uint16_t resetcause = 0;
            uint16_t vcc_v = 0, st5_v = 0, vbat_v = 0;
            int16_t  vcc_i = 0, st5_i = 0;
            int16_t  temp[3] = {0};
            uint8_t  mppt_mode = 0;
            uint16_t mppt_limit = 0, mppt_time = 0, mppt_period = 0;
            uint32_t gnd_wdt_cnt = 0, gnd_wdt_left = 0;

            perr |= gs_param_get_uint32(&tinst, GS_P80_ACU_TELEMETRY_UPTIME,      &uptime,      0);
            perr |= gs_param_get_uint32(&tinst, GS_P80_ACU_TELEMETRY_BOOTCAUSE,   &bootcause,   0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_ACU_TELEMETRY_RESETCAUSE,  &resetcause,  0);
            perr |= gs_param_get_uint32(&tinst, GS_P80_ACU_TELEMETRY_BOOTCOUNT,   &bootcount,   0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_ACU_TELEMETRY_VCC_V,       &vcc_v,       0);
            perr |= gs_param_get_int16 (&tinst, GS_P80_ACU_TELEMETRY_VCC_I,       &vcc_i,       0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_ACU_TELEMETRY_ST5_V,       &st5_v,       0);
            perr |= gs_param_get_int16 (&tinst, GS_P80_ACU_TELEMETRY_ST5_I,       &st5_i,       0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_ACU_TELEMETRY_VBAT_V,      &vbat_v,      0);
            perr |= gs_param_get_int16 (&tinst, GS_P80_ACU_TELEMETRY_TEMP(0),     &temp[0],     0);
            perr |= gs_param_get_int16 (&tinst, GS_P80_ACU_TELEMETRY_TEMP(1),     &temp[1],     0);
            perr |= gs_param_get_int16 (&tinst, GS_P80_ACU_TELEMETRY_TEMP(2),     &temp[2],     0);
            perr |= gs_param_get_uint8 (&tinst, GS_P80_ACU_TELEMETRY_MPPT_MODE,   &mppt_mode,   0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_ACU_TELEMETRY_MPPT_LIMIT,  &mppt_limit,  0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_ACU_TELEMETRY_MPPT_TIME,   &mppt_time,   0);
            perr |= gs_param_get_uint16(&tinst, GS_P80_ACU_TELEMETRY_MPPT_PERIOD, &mppt_period, 0);
            perr |= gs_param_get_uint32(&tinst, GS_P80_ACU_TELEMETRY_GND_WDT_CNT,  &gnd_wdt_cnt,  0);
            perr |= gs_param_get_uint32(&tinst, GS_P80_ACU_TELEMETRY_GND_WDT_LEFT, &gnd_wdt_left, 0);

            if (perr != GS_OK)
            {
                EPS_AppData.Counters.ErrCounter++;
                CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                "EPS: Failed to parse ACU2 telemetry params, err=%d", perr);
                if (tinst.memory) free(tinst.memory);
                if (tinst.rows) free((void*)tinst.rows);
                return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            }

            OS_printf("\n================[p80 ACU2 HK]===================\n");

            OS_printf("[SYSTEM] Uptime: %u s | BootCount: %u | Cause(Boot/Reset): %u / %u\n",
                    uptime, bootcount, bootcause, resetcause);

            OS_printf("[INPUT]  IDX | EN | Volt(mV) | Curr(mA) | Power(mW) | PV_Set(mV) | DAC\n");
            for (int i = 0; i < 6; i++) {
                bool     en     = gs_param_get_bool_nc  (&tinst, GS_P80_ACU_TELEMETRY_CHANNEL_EN(i),  0);
                uint16_t volt   = gs_param_get_uint16_nc(&tinst, GS_P80_ACU_TELEMETRY_INPUT_V(i),     0);
                int16_t  curr   = gs_param_get_int16_nc (&tinst, GS_P80_ACU_TELEMETRY_INPUT_I(i),     0);
                uint16_t power  = gs_param_get_uint16_nc(&tinst, GS_P80_ACU_TELEMETRY_POWER(i),       0);
                uint16_t pv_set = gs_param_get_uint16_nc(&tinst, GS_P80_ACU_TELEMETRY_PV_SETPOINT(i), 0);
                uint16_t dac    = gs_param_get_uint16_nc(&tinst, GS_P80_ACU_TELEMETRY_DAC_VAL(i),     0);
                OS_printf("         %3d | %2s | %8u | %8d | %9u | %10u | %5u\n",
                        i, en ? "ON" : "OFF", volt, curr, power, pv_set, dac);
            }

            OS_printf("[ACU2]    VCC: %u mV (%d mA) | ST5V: %u mV (%d mA) | VBAT: %u mV\n",
                    vcc_v, vcc_i, st5_v, st5_i, vbat_v);
            OS_printf("[TEMP]   T0: %d | T1: %d | T2: %d (deci-degC)\n", temp[0], temp[1], temp[2]);
            OS_printf("[MPPT]   Mode: %u | Limit: %u | Time: %u | Period: %u\n",
                    mppt_mode, mppt_limit, mppt_time, mppt_period);

            OS_printf("[DEVICE] Type  : ");
            for (int i = 0; i < 8; i++) OS_printf("%u ", gs_param_get_uint8_nc(&tinst, GS_P80_ACU_TELEMETRY_DEVICE_TYPE(i), 0));
            OS_printf("\n");

            OS_printf("[DEVICE] Status: ");
            for (int i = 0; i < 8; i++) OS_printf("%u ", gs_param_get_uint8_nc(&tinst, GS_P80_ACU_TELEMETRY_DEVICE_STATUS(i), 0));
            OS_printf("\n");

            OS_printf("[WDT]    GND: %u (Left: %u s)\n", gnd_wdt_cnt, gnd_wdt_left);

            OS_printf("=======================================================\n");

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

    //send report

    return CFE_SUCCESS;
}

CFE_Status_t EPS_Get_Full_Table_Cmd(const EPS_Get_Full_Table_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    uint16_t checksum;
    gs_param_table_instance_t tinst = {0};

    gs_error_t result = gs_rparam_download_table_spec(&tinst, NULL, Msg->Payload.csp_node, Msg->Payload.table_id, CSP_TIMEOUT(1), &checksum);

    if (result){
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: RParam Download Table Spec failed, err=%d", result);
        if (tinst.memory) free(tinst.memory);
        if (tinst.rows) free((void*)tinst.rows);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    gs_error_t err =  gs_rparam_get_full_table(&tinst, Msg->Payload.csp_node, Msg->Payload.table_id, checksum, CSP_TIMEOUT(1));

    if(err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: RParam Get Full Table command failed, err=%d", err);
        if (tinst.memory) free(tinst.memory);
        if (tinst.rows) free((void*)tinst.rows);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("\n================[EPS Full Table (Node %u, Table %u)]===================\n",
              Msg->Payload.csp_node, Msg->Payload.table_id);
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

    //EPS_SendReport(Msg, tinst.memory, tinst.memory_size, err, RPT_RETTYPE_SUCCESS);

    if (tinst.memory) free(tinst.memory);
    if (tinst.rows) free((void*)tinst.rows);

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