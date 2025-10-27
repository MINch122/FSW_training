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
#include "eps_msg.h"
#include "eps_eventids.h"

#include "p31u.h"

static void print_hk_all(const void* all)
{
    const p31u_hk_t* hk = all;
    printf("vboost[mV]:   %4d, %4d, %4d\n",
            hk->vboost[0], hk->vboost[1], hk->vboost[2]);
    printf("vbatt[mV]:    %4d\n", hk->vbatt);
    printf("curin[mA]:    %4d, %4d, %4d\n",
            hk->curin[0], hk->curin[1], hk->curin[2]);
    printf("cursun[mA]:   %4d\n", hk->cursun);
    printf("cursys[mA]:   %4d\n", hk->cursys);
    printf("reserved1:    %4d\n", hk->reserved1);
    printf("curout[mA]:   %4d, %4d, %4d, %4d, %4d, %4d\n",
            hk->curout[0], hk->curout[1], hk->curout[2],
            hk->curout[3], hk->curout[4], hk->curout[5]);
    printf("output:       %2d, %2d, %2d, %2d, %2d, %2d, %2d, %2d\n",
            hk->output[0], hk->output[1], hk->output[2], hk->output[3],
            hk->output[4], hk->output[5], hk->output[6], hk->output[7]);
    printf("out_on_delta[s]: %2d, %2d, %2d, %2d, %2d, %2d, %2d, %2d\n", 
            hk->output_on_delta[0], hk->output_on_delta[1],
            hk->output_on_delta[2], hk->output_on_delta[3],
            hk->output_on_delta[4], hk->output_on_delta[5],
            hk->output_on_delta[6], hk->output_on_delta[7]);
    printf("out_off_delta[s]:  %2d, %2d, %2d, %2d, %2d, %2d, %2d, %2d\n",
            hk->output_off_delta[0], hk->output_off_delta[1],
            hk->output_off_delta[2], hk->output_off_delta[3],
            hk->output_off_delta[4], hk->output_off_delta[5],
            hk->output_off_delta[6], hk->output_off_delta[7]);
    printf("latchup:      %4d, %4d, %4d, %4d, %4d, %4d\n",
            hk->latchup[0], hk->latchup[1], hk->latchup[2],
            hk->latchup[3], hk->latchup[4], hk->latchup[5]);
    printf("wdt_i2c_time_left[s]: %4u\n", hk->wdt_i2c_time_left);
    printf("wdt_gnd_time_left[s]: %4u\n", hk->wdt_gnd_time_left);
    printf("wdt_csp_pings_left:   %4d, %4d\n",
            hk->wdt_csp_pings_left[0], hk->wdt_csp_pings_left[1]);
    printf("counter_wdt_i2c:      %4u\n", hk->counter_wdt_i2c);
    printf("counter_wdt_gnd:      %4u\n", hk->counter_wdt_gnd);
    printf("counter_wdt_csp:      %4u, %4u\n",
            hk->counter_wdt_csp[0], hk->counter_wdt_csp[1]);
    printf("counter_boot:         %4u\n", hk->counter_boot);
    printf("temp[degC]:   %2d, %2d, %2d, %2d, %2d, %2d\n",
            hk->temp[0], hk->temp[1], hk->temp[2],
            hk->temp[3],hk->temp[4], hk->temp[5]);
    printf("bootcause:    %4d\n", hk->bootcause);
    printf("battmode:     %4d\n", hk->battmode);
    printf("pptmode:      %4d\n", hk->pptmode);
}


static void EPS_SendReport(const void* cmd,
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
                 CFE_SB_ValueToMsgId(EPS_REPORT_MID), // todo: define eps report mid.
                 sizeof(EPS_AppData.Report));
    EPS_AppData.Report.Payload.MsgID = CFE_SB_MsgIdToValue(cmdMid);
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
                        
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(EPS_AppData.Report.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(EPS_AppData.Report.TelemetryHeader), true);
}


void EPS_P31U_PingCmd(const EPS_P31U_PingCmd_t *Msg) {

    EPS_AppData.Counters.CmdCounter++;

    uint8_t tx = 0x55;
    uint8_t rx = 0x00;
    int ret = p31u_ping(&tx, &rx);

    if (ret == P31U_OK && rx == 0x55) {
        OS_printf("Ping Success.\n");
    }
    else if (ret == P31U_OK && rx != 0x55) {
        OS_printf("Ping Failed.\n");
    }
}


void EPS_P31U_SetOutputSingleCmd(const EPS_P31U_SetOutputSingleCmd_t *Msg)
{
    int ret;

    EPS_AppData.Counters.CmdCounter++;
    
    ret = p31u_set_output_single(Msg->Payload.channel,
                                 Msg->Payload.value,
                                 Msg->Payload.delay);
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;

    EPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void EPS_P31U_SetOutputSingleInternalCmd(const EPS_P31U_SetOutputSingleCmd_t *Msg)
{
    int ret;

    EPS_AppData.Counters.CmdCounter++;
    
    ret = p31u_set_output_single(Msg->Payload.channel,
                                 Msg->Payload.value,
                                 Msg->Payload.delay);
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;

}

void EPS_P31U_SetOutputsCmd(const EPS_P31U_SetOutputsCmd_t *Msg)
{
    int ret;

    EPS_AppData.Counters.CmdCounter++;

    ret = p31u_set_outputs(Msg->Payload.mask);
    EPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void EPS_P31U_ResetWdtCmd(const EPS_P31U_ResetWdtCmd_t *Msg)
{
    int ret;
    
    EPS_AppData.Counters.CmdCounter++;
    
    ret = p31u_reset_wdt();
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;
        
    EPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void EPS_P31U_ResetCountersCmd(const EPS_P31U_ResetCountersCmd_t* Msg)
{
    int ret;
    
    EPS_AppData.Counters.CmdCounter++;
    
    ret = p31u_reset_counters();
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;

    EPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void EPS_P31U_HardResetCmd(const EPS_P31U_HardResetCmd_t *Msg)
{
    int ret;
    
    EPS_AppData.Counters.CmdCounter++;
    
    ret = p31u_hard_reset();
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;

    EPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void EPS_P31U_GetHkAllCmd(const EPS_P31U_GetHkAllCmd_t *Msg)
{
    p31u_hk_t hk;
    int ret;
    
    EPS_AppData.Counters.CmdCounter++;
    
    ret = p31u_gethk_all(&hk);
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;
    print_hk_all(&hk);

    EPS_SendReport(Msg, &hk, sizeof(hk), ret, RPT_RETTYPE_HW);
}

void EPS_P31U_GetHkOutCmd(const EPS_P31U_GetHkOutCmd_t *Msg)
{
    p31u_hk_out_t hk;
    int ret;
    
    EPS_AppData.Counters.CmdCounter++;

    ret = p31u_gethk_out(&hk);
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;

    EPS_SendReport(Msg, &hk, sizeof(hk), ret, RPT_RETTYPE_HW);
}

void EPS_P31U_GetHkOutInternalCmd(const EPS_P31U_GetHkOutCmd_t *Msg)
{
    p31u_hk_out_t hk;
    int ret;
    
    EPS_AppData.Counters.CmdCounter++;
    OS_printf("EPS Out recved\n");
    ret = p31u_gethk_out(&hk);
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;

    /* Send EPS vbatt */
    EPS_Output_Tlm_t *out = (EPS_Output_Tlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(EPS_Output_Tlm_t));
    if (!out) return;
    if (CFE_MSG_Init(CFE_MSG_PTR(out->TelemetryHeader), CFE_SB_ValueToMsgId(EPS_OUT_TLM_MID), sizeof(EPS_Output_Tlm_t)) != CFE_SUCCESS) return;

    memcpy(&out->Output, &hk.output, sizeof(out->Output));

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(out->TelemetryHeader));
    if (CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)out, true) != CFE_SUCCESS)
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)out);
}

void EPS_P31U_GetHkViCmd(const EPS_P31U_GetHkViCmd_t *Msg)
{
    p31u_hk_vi_t hk;
    int ret;
    
    EPS_AppData.Counters.CmdCounter++;

    ret = p31u_gethk_vi(&hk);
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;

    EPS_SendReport(Msg, &hk, sizeof(hk), ret, RPT_RETTYPE_HW);
    
    return;
}

void EPS_P31U_GetHkViInternalCmd(const EPS_P31U_GetHkViCmd_t *Msg)
{
    p31u_hk_vi_t hk;
    int ret;
    
    EPS_AppData.Counters.CmdCounter++;
    OS_printf("%s:EPS Vi recved.\n", __func__);
    ret = p31u_gethk_vi(&hk);
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;

    /* Send EPS vbatt */
    EPS_Vi_Tlm_t *vi = (EPS_Vi_Tlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(EPS_Vi_Tlm_t));
    if (!vi) return;
    if (CFE_MSG_Init(CFE_MSG_PTR(vi->TelemetryHeader), CFE_SB_ValueToMsgId(EPS_VI_TLM_MID), sizeof(EPS_Vi_Tlm_t)) != CFE_SUCCESS) return;

    vi->Vbatt = hk.vbatt;
    vi->CurIn[0] = hk.curin[0];
    vi->CurIn[1] = hk.curin[1];
    OS_printf("%s:EPS Vbatt2: %u\n", __func__, vi->Vbatt);

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(vi->TelemetryHeader));
    if (CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)vi, true) != CFE_SUCCESS)
        CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)vi);
    
    return;
}

void EPS_P31U_GetHkWdtCmd(const EPS_P31U_GetHkWdtCmd_t *Msg)
{
    p31u_hk_wdt_t hk;
    int ret;
    
    EPS_AppData.Counters.CmdCounter++;
    
    ret = p31u_gethk_wdt(&hk);
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;

    EPS_SendReport(Msg, &hk, sizeof(hk), ret, RPT_RETTYPE_HW);
}

void EPS_P31U_GetHkBasicCmd(const EPS_P31U_GetHkBasicCmd_t *Msg)
{
    p31u_hk_basic_t hk;
    int ret;
    
    EPS_AppData.Counters.CmdCounter++;
    
    ret = p31u_gethk_basic(&hk);
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;

    EPS_SendReport(Msg, &hk, sizeof(hk), ret, RPT_RETTYPE_HW);
}

void EPS_P31U_GetHkOldCmd(const EPS_P31U_GetHkOldCmd_t *Msg)
{
    // todo: implement p31u_gethk_old.
    return;

    // p31u_hkparam_t hk; 
    // int ret;
    
    // EPS_AppData.Counters.CmdCounter++;
    
    // ret = p31u_gethk_old(&hk);
    // if (ret != P31U_OK)
    //     EPS_AppData.Counters.ErrCounter++;

    // EPS_SendReport(Msg, &hk, sizeof(hk), ret, RPT_RETTYPE_HW);
}

void EPS_P31U_GetHkCmd(const EPS_P31U_GetHkCmd_t *Msg)
{
    int ret;
    union {
        p31u_hk_t all;
        p31u_hk_out_t out;
        p31u_hk_vi_t vi;
        p31u_hk_wdt_t wdt;
        p31u_hk_basic_t basic;
    } hk;
    
    EPS_AppData.Counters.CmdCounter++;
    
    ret = p31u_gethk(&hk, Msg->Payload.id, Msg->Payload.size);
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;

    uint16 trimSize = Msg->Payload.size > sizeof(hk)
                      ? sizeof(hk)
                      : Msg->Payload.size;
    EPS_SendReport(Msg, &hk, trimSize, ret, RPT_RETTYPE_HW);
}

void EPS_P31U_SetPvVoltCmd(const EPS_P31U_SetPvVoltCmd_t *Msg)
{
    int ret;
    
    EPS_AppData.Counters.CmdCounter++;
    
    ret = p31u_set_pv_volt(Msg->Payload.voltage[0],
                           Msg->Payload.voltage[1],
                           Msg->Payload.voltage[2]);
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;

    EPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void EPS_P31U_SetPvAutoCmd(const EPS_P31U_SetPvAutoCmd_t *Msg)
{
    int ret;
    
    EPS_AppData.Counters.CmdCounter++;
    
    ret = p31u_set_pv_auto(Msg->Payload.mode);
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;

    EPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void EPS_P31U_SetHeaterCmd(const EPS_P31U_SetHeaterCmd_t *Msg)
{
    p31u_reply_set_heater rep;
    int ret;
    
    EPS_AppData.Counters.CmdCounter++;
    
    ret = p31u_set_heater(Msg->Payload.cmd,
                          Msg->Payload.heater,
                          Msg->Payload.mode,
                          &rep);
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;
    
    EPS_SendReport(Msg, &rep, sizeof(rep), ret, RPT_RETTYPE_HW);
}

void EPS_P31U_ConfigCmd(const EPS_P31U_ConfigCmd_t *Msg)
{
    int ret;
    
    EPS_AppData.Counters.CmdCounter++;
    
    ret = p31u_config_cmd(Msg->Payload.cmd);
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;

    EPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void EPS_P31U_GetConfigCmd(const EPS_P31U_GetConfigCmd_t *Msg)
{
    p31u_config_t conf;
    int ret;
    
    EPS_AppData.Counters.CmdCounter++;
    
    ret = p31u_get_config(&conf);
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;

    EPS_SendReport(Msg, &conf, sizeof(conf), ret, RPT_RETTYPE_HW);
}

void EPS_P31U_SetConfigCmd(const EPS_P31U_SetConfigCmd_t *Msg)
{
    int ret;
    p31u_config_t conf;
    conf.ppt_mode = Msg->Payload.ppt_mode;
    conf.battheater_mode = Msg->Payload.battheater_mode;
    conf.battheater_low = Msg->Payload.battheater_low;
    conf.battheater_high = Msg->Payload.battheater_high;
    for (int i = 0; i < 8; ++i) {
        conf.output_normal_value[i] = Msg->Payload.output_normal_value[i];
        conf.output_safe_value[i] = Msg->Payload.output_safe_value[i];
        conf.output_initial_on_delay[i] = Msg->Payload.output_initial_on_delay[i];
        conf.output_initial_off_delay[i] = Msg->Payload.output_initial_off_delay[i];
    }
    conf.vboost[0] = Msg->Payload.vboost[0];
    conf.vboost[1] = Msg->Payload.vboost[1];
    conf.vboost[2] = Msg->Payload.vboost[2];

    ret = p31u_set_config(&conf);
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;

    EPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void EPS_P31U_Config2Cmd(const EPS_P31U_Config2Cmd_t *Msg)
{
    int ret;
    
    EPS_AppData.Counters.CmdCounter++;
    
    ret = p31u_config2_cmd(Msg->Payload.cmd);
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;

    EPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void EPS_P31U_GetConfig2Cmd(const EPS_P31U_GetConfig2Cmd_t *Msg)
{
    p31u_config2_t conf;
    int ret;
    
    EPS_AppData.Counters.CmdCounter++;
    
    ret = p31u_get_config2(&conf);
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;

    EPS_SendReport(Msg, &conf, sizeof(conf), ret, RPT_RETTYPE_HW);
}

void EPS_P31U_SetConfig2Cmd(const EPS_P31U_SetConfig2Cmd_t *Msg)
{
    int ret;
    p31u_config2_t conf2;
    conf2.batt_maxvoltage = Msg->Payload.batt_maxvoltage;
    conf2.batt_safevoltage = Msg->Payload.batt_safevoltage;
    conf2.batt_criticalvoltage = Msg->Payload.batt_criticalvoltage;
    conf2.batt_normalvoltage = Msg->Payload.batt_normalvoltage;
    conf2.reserved1[0] = Msg->Payload.reserved1[0];
    conf2.reserved1[1] = Msg->Payload.reserved1[1];
    conf2.reserved2[0] = Msg->Payload.reserved2[0];
    conf2.reserved2[1] = Msg->Payload.reserved2[1];
    conf2.reserved2[2] = Msg->Payload.reserved2[2];
    conf2.reserved2[3] = Msg->Payload.reserved2[3];

    ret = p31u_set_config2(&conf2);
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;

    EPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void EPS_P31U_SetConfig3Cmd(const EPS_P31U_SetConfig3Cmd_t *Msg)
{
    int ret;
    p31u_config3_t conf3;
    conf3.version = Msg->Payload.version;
    conf3.cmd = Msg->Payload.cmd;
    conf3.length = Msg->Payload.length;
    conf3.flags = Msg->Payload.flags;
    for (int i = 0; i < 8; ++i) {
        conf3.cur_lim[i] = Msg->Payload.cur_lim[i];
    }
    conf3.cur_ema_gain = Msg->Payload.cur_ema_gain;
    conf3.cspwdt_channel[0] = Msg->Payload.cspwdt_channel[0];
    conf3.cspwdt_channel[1] = Msg->Payload.cspwdt_channel[1];
    conf3.cspwdt_address[0] = Msg->Payload.cspwdt_address[0];
    conf3.cspwdt_address[1] = Msg->Payload.cspwdt_address[1];

    ret = p31u_set_config3(&conf3);
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;

    EPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void EPS_P31U_TransactionCmd(const EPS_P31U_TransactionCmd_t* Msg)
{
    int ret;
    uint16 rxSize;
    static uint8_t rx[RPT_RET_VALUE_BUF_SIZE];

    EPS_AppData.Counters.CmdCounter++;

    if (Msg->Payload.txSize > sizeof(Msg->Payload.tx)) {
        /**
         * txSize violates maximum tx data length.
         */
        EPS_AppData.Counters.ErrCounter++;
        EPS_SendReport(Msg,
                       &Msg->Payload.txSize,
                       sizeof(Msg->Payload.txSize),
                       -1,
                       RPT_RETTYPE_HW);
        return;
    }

    rxSize = Msg->Payload.rxSize > RPT_RET_VALUE_BUF_SIZE
             ? RPT_RET_VALUE_BUF_SIZE
             : Msg->Payload.rxSize;

    ret = p31u_transaction(Msg->Payload.port,
                           Msg->Payload.tx,
                           Msg->Payload.txSize,
                           rx,
                           rxSize);
    if (ret != P31U_OK)
        EPS_AppData.Counters.ErrCounter++;

    EPS_SendReport(Msg, rx, rxSize, ret, RPT_RETTYPE_HW);
}
