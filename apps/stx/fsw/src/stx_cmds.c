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
 *   This file contains the source code for the Sample App Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "stx.h"
#include "stx_cmds.h"
#include "stx_msgids.h"
#include "stx_eventids.h"
#include "stx_version.h"
#include "stx_tbl.h"
#include "stx_utils.h"
#include "stx_msg.h"
#include "esup.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "rpt_interface_cfg.h"


static stx_custom_dir_entry_t dir_entries[20];
static int32_t                file_handle = -1;
extern uint32_t open_file_size;
extern CFE_SRL_IO_Handle_t *Handle;

static inline void STX_rptsend(uint16_t cc, uint8_t type, int32_t status,  size_t len, const void *data)
{
    memset(&STX_Data.RptPkt, 0, sizeof(STX_Data.RptPkt));
    CFE_MSG_Init(CFE_MSG_PTR(STX_Data.RptPkt.TelemetryHeader), CFE_SB_ValueToMsgId(STX_APP_RPT_TLM_MID),
                 sizeof(STX_Data.RptPkt));

    STX_Data.RptPkt.Report.CommandCode = cc;
    STX_Data.RptPkt.Report.ReturnType  = type;
    STX_Data.RptPkt.Report.ReturnCode  = status;

    size_t datasize = len > RPT_RET_VALUE_BUF_SIZE ? RPT_RET_VALUE_BUF_SIZE : len;
    STX_Data.RptPkt.Report.ReturnDataSize = (uint16_t)datasize;
    if (len && data) memcpy(STX_Data.RptPkt.Report.ReturnValue, data, datasize);

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(STX_Data.RptPkt.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(STX_Data.RptPkt.TelemetryHeader), true);
}


void STX_SendHkCmd(void)
{

    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = CONFIG_TP_ALLPARAM;
    uint8_t rxbuf1[sizeof(ESUP_Packet_t) + sizeof(STX_GET_ALLPRAM_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*)rxbuf1;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(STX_GET_ALLPRAM_t));
    }

    memcpy(&(STX_Data.HkTlm.Payload.ALLPRAM), rxdata->DCP, sizeof(STX_GET_ALLPRAM_t));

    status  = ESUP_INSIG;
    command = CONFIG_CC_GET;
    type    = CONFIG_TP_MODULATORDTIFC;
    uint8_t rxbuf2[sizeof(ESUP_Packet_t) + sizeof(STX_GET_ModulationInterface_t)];
    rxdata = (ESUP_Packet_t*)rxbuf2;

    ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(STX_GET_ModulationInterface_t));
    }

    memcpy(&(STX_Data.HkTlm.Payload.Modulator), rxdata->DCP, sizeof(STX_GET_ModulationInterface_t));

    status  = ESUP_INSIG;
    command = CONFIG_CC_GET;
    type    = STATUS_TP_SIMPLE_REPORT;
    uint8_t rxbuf3[sizeof(ESUP_Packet_t) + sizeof(STX_GET_Report_t)];
    rxdata = (ESUP_Packet_t*)rxbuf3;

    ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(STX_GET_Report_t));
        
    }
    STX_GET_Report_t* databuf = (STX_GET_Report_t *)(rxdata->DCP);

    STX_Data.HkTlm.Payload.SystemState    = databuf->SystemState;
    STX_Data.HkTlm.Payload.StatusFlags    = databuf->StatusFlags;
    STX_Data.HkTlm.Payload.cputemperature = databuf->cputemperature;

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(STX_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(STX_Data.HkTlm.TelemetryHeader), true);
}

void STX_SendBCNCmd(void)
{

    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = CONFIG_TP_ALLPARAM;
    uint8_t rxbuf1[sizeof(ESUP_Packet_t) + sizeof(STX_GET_ALLPRAM_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*)rxbuf1;

    memset(&STX_Data.BCNTlm.Payload, 0, sizeof(STX_Data.BCNTlm.Payload));

    int32_t ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(STX_GET_ALLPRAM_t));
    }

    memcpy(&(STX_Data.BCNTlm.Payload.ALLPRAM), rxdata->DCP, sizeof(STX_GET_ALLPRAM_t));

    status  = ESUP_INSIG;
    command = CONFIG_CC_GET;
    type    = CONFIG_TP_MODULATORDTIFC;
    uint8_t rxbuf2[sizeof(ESUP_Packet_t) + sizeof(STX_GET_ModulationInterface_t)];
    rxdata = (ESUP_Packet_t*)rxbuf2;
    
    ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(STX_GET_ModulationInterface_t));
    }

    memcpy(&(STX_Data.BCNTlm.Payload.Modulator), rxdata->DCP, sizeof(STX_GET_ModulationInterface_t));

    status  = ESUP_INSIG;
    command = CONFIG_CC_GET;
    type    = STATUS_TP_SIMPLE_REPORT;
    uint8_t rxbuf3[sizeof(ESUP_Packet_t) + sizeof(STX_GET_Report_t)];
    rxdata = (ESUP_Packet_t*)rxbuf3;

    ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(STX_GET_Report_t));
    }

    STX_GET_Report_t* databuf = (STX_GET_Report_t *)(rxdata->DCP);

    STX_Data.BCNTlm.Payload.SystemState    = databuf->SystemState;
    STX_Data.BCNTlm.Payload.StatusFlags    = databuf->StatusFlags;
    STX_Data.BCNTlm.Payload.cputemperature = databuf->cputemperature;
    

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(STX_Data.BCNTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(STX_Data.BCNTlm.TelemetryHeader), true);

    OS_printf("[STX][BCN] sys_state=%u status_flags=0x%X cpu_temp=%d\n",
              (unsigned int)STX_Data.BCNTlm.Payload.SystemState,
              (unsigned int)STX_Data.BCNTlm.Payload.StatusFlags,
              (int)STX_Data.BCNTlm.Payload.cputemperature);
}

CFE_Status_t STX_NoopCmd(const STX_NoopCmd_t *Msg)
{
    CFE_EVS_SendEvent(STX_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "SAMPLE: NOOP command %s", STX_VERSION);
    uint16_t txdata[2] = {STX_Data.CmdCounter, STX_Data.ErrCounter};
    STX_rptsend(STX_NOOP_CC, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(txdata), &txdata);

    return CFE_SUCCESS;
}

CFE_Status_t STX_ResetCountersCmd(const STX_ResetCountersCmd_t *Msg)
{
    
    STX_Data.CmdCounter = 0;
    STX_Data.ErrCounter = 0;

    CFE_EVS_SendEvent(STX_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "SAMPLE: RESET command");
    uint16_t txdata[2] = {STX_Data.CmdCounter, STX_Data.ErrCounter};
    STX_rptsend(STX_RESET_COUNTERS_CC, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(txdata), &txdata);
    return CFE_SUCCESS;
}

CFE_Status_t STX_SetModuleIdCmd(const STX_SetModuleIdCmd_t *Msg)
{
    uint16_t module_id = Msg->Payload.ModuleId;
    int32_t  status    = ESUP_SetModuleId(module_id);

    if (status != DEVICE_SUCCESS)
    {
        STX_Data.ErrCounter++;
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : invalid module id 0x%04X (allowed: 0x1212, 0x1213)",
                          (unsigned int)module_id);
        STX_rptsend(STX_SET_MODULE_ID_CC, STX_rpt_type_CFE_ERROR, status, sizeof(module_id), &module_id);
        return status;
    }

    CFE_EVS_SendEvent(STX_VALUE_INF_EID, CFE_EVS_EventType_INFORMATION, "STX : module id set to 0x%04X",
                      (unsigned int)module_id);
    STX_rptsend(STX_SET_MODULE_ID_CC, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(module_id), &module_id);
    return CFE_SUCCESS;
}

/* SET COMMNAD */
void STX_SET_SYMBOLRATECmd(const STX_Set_SYMBOLRAtE_t *cmd)
{
    

    void    *txdata   = (void *)&cmd->Payload.data;
    uint16_t txlength = sizeof(cmd->Payload.data);

    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_SET;
    uint16_t type    = CONFIG_TP_SYMBOLRATE;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(uint8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*)rxbuf;

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, rxdata, 0);
    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(uint8_t));
            
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_SET_SYMBOLRATE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_SET_SYMBOLRATE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_SET_SYMBOLRATE, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SET_SYMBOLRATE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_SYMBOLRATE,
                          (unsigned)cmd->Payload.data, ret_status);
        STX_rptsend(STX_SET_SYMBOLRATE, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_SYMBOLRATE,
                          (unsigned)cmd->Payload.data, rxdata->header.com_stt);
        STX_rptsend(STX_SET_SYMBOLRATE, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }
}

void STX_Set_TRANSMITPWCmd(const STX_Set_TRANSMITPW_t *Msg)
{
    const STX_Set_TRANSMITPW_t *cmd      = (const STX_Set_TRANSMITPW_t *)Msg;
    void                       *txdata   = (void *)&cmd->Payload.data;
    uint16_t                    txlength = sizeof(cmd->Payload.data);

    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_SET;
    uint16_t type    = CONFIG_TP_TRANSMITPW;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(uint8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*)rxbuf;

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, rxdata, 0);
    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(uint8_t));
        
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_SET_TRANSMITPW, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_SET_TRANSMITPW, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_SET_TRANSMITPW, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SET_TRANSMITPW_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_TRANSMITPW,
                          (unsigned)cmd->Payload.data, ret_status);
        STX_rptsend(STX_SET_TRANSMITPW, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_TRANSMITPW,
                          (unsigned)cmd->Payload.data, rxdata->header.com_stt);
        STX_rptsend(STX_SET_TRANSMITPW, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_Set_CENTERFREQCmd(const STX_Set_CENTERFREQ_t *Msg)
{
    
    const STX_Set_CENTERFREQ_t *cmd      = (const STX_Set_CENTERFREQ_t *)Msg;
    void                       *txdata   = (void *)&cmd->Payload.data;
    uint16_t                    txlength = sizeof(cmd->Payload.data);

    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_SET;
    uint16_t type    = CONFIG_TP_CENTERFREQ;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(uint8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*)rxbuf;

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, rxdata, 0);
    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(uint8_t));
        
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_SET_CENTERFREQ, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_SET_CENTERFREQ, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_SET_CENTERFREQ, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SET_CENTERFREQ_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_CENTERFREQ,
                          (unsigned)cmd->Payload.data, ret_status);
        STX_rptsend(STX_SET_CENTERFREQ, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_CENTERFREQ,
                          (unsigned)cmd->Payload.data, rxdata->header.com_stt);
        STX_rptsend(STX_SET_CENTERFREQ, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_Set_MODCODCmd(const STX_Set_MODCOD_t *Msg)
{
    
    const STX_Set_MODCOD_t *cmd      = (const STX_Set_MODCOD_t *)Msg;
    void                   *txdata   = (void *)&cmd->Payload.data;
    uint16_t                txlength = sizeof(cmd->Payload.data);

    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_SET;
    uint16_t type    = CONFIG_TP_MODCOD;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(uint8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(uint8_t));
        
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_SET_MODCOD, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_SET_MODCOD, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_SET_MODCOD, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SET_MODCOD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_MODCOD, (unsigned)cmd->Payload.data,
                          ret_status);
        STX_rptsend(STX_SET_MODCOD, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_MODCOD, (unsigned)cmd->Payload.data,
                          rxdata->header.com_stt);
        STX_rptsend(STX_SET_MODCOD, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_Set_ROLLOFFCmd(const STX_Set_ROLLOFF_t *Msg)
{
    
    const STX_Set_ROLLOFF_t *cmd      = (const STX_Set_ROLLOFF_t *)Msg;
    void                    *txdata   = (void *)&cmd->Payload.data;
    uint16_t                 txlength = sizeof(cmd->Payload.data);

    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_SET;
    uint16_t type    = CONFIG_TP_ROLLOFF;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(uint8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, rxdata, 0);
    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(uint8_t));
        
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_SET_ROLLOFF, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_SET_ROLLOFF, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_SET_ROLLOFF, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SET_ROLLOFF_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_ROLLOFF, (unsigned)cmd->Payload.data,
                          ret_status);
        STX_rptsend(STX_SET_ROLLOFF, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_ROLLOFF, (unsigned)cmd->Payload.data,
                          rxdata->header.com_stt);
        STX_rptsend(STX_SET_ROLLOFF, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_Set_PILOTSIGCmd(const STX_Set_PILOTSIG_t *Msg)
{
    
    const STX_Set_PILOTSIG_t *cmd      = (const STX_Set_PILOTSIG_t *)Msg;
    void                     *txdata   = (void *)&cmd->Payload.data;
    uint16_t                  txlength = sizeof(cmd->Payload.data);

    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_SET;
    uint16_t type    = CONFIG_TP_PILOTSIG;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(uint8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, rxdata, 0);
    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(uint8_t));
            
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_SET_PILOTSIG, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_SET_PILOTSIG, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_SET_PILOTSIG, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SET_PILOTSIG_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_PILOTSIG, (unsigned)cmd->Payload.data,
                          ret_status);
        STX_rptsend(STX_SET_PILOTSIG, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_PILOTSIG, (unsigned)cmd->Payload.data,
                          rxdata->header.com_stt);
        STX_rptsend(STX_SET_PILOTSIG, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_Set_FECFRAMECmd(const STX_Set_FECFRAME_t *Msg)
{
    
    const STX_Set_FECFRAME_t *cmd      = (const STX_Set_FECFRAME_t *)Msg;
    void                     *txdata   = (void *)&cmd->Payload.data;
    uint16_t                  txlength = sizeof(cmd->Payload.data);

    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_SET;
    uint16_t type    = CONFIG_TP_FECFRAMESZ;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(uint8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(uint8_t));
        
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_SET_FECFRAMESZ, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_SET_FECFRAMESZ, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_SET_FECFRAMESZ, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SET_FECFRAMESZ_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_FECFRAMESZ,
                          (unsigned)cmd->Payload.data, ret_status);
        STX_rptsend(STX_SET_FECFRAMESZ, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_FECFRAMESZ,
                          (unsigned)cmd->Payload.data, rxdata->header.com_stt);
        STX_rptsend(STX_SET_FECFRAMESZ, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_Set_PRETX_DELAYCmd(const STX_Set_PRETX_DELAY_t *Msg)
{
    
    const STX_Set_PRETX_DELAY_t *cmd      = (const STX_Set_PRETX_DELAY_t *)Msg;
    void                        *txdata   = (void *)&cmd->Payload.data;
    uint16_t                     txlength = sizeof(cmd->Payload.data);

    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_SET;
    uint16_t type    = CONFIG_TP_PRETXSTUFFDEL;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(uint8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(uint8_t));
        
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_SET_PRETX_DELAY, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_SET_PRETX_DELAY, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_SET_PRETX_DELAY, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SET_PRETX_DELAY_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_PRETX_DELAY,
                          (unsigned)cmd->Payload.data, ret_status);
        STX_rptsend(STX_SET_PRETX_DELAY, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_PRETX_DELAY,
                          (unsigned)cmd->Payload.data, rxdata->header.com_stt);
        STX_rptsend(STX_SET_PRETX_DELAY, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_Set_ALLPRAMCmd(const STX_Set_ALLPRAM_t *Msg)
{
    
    const STX_Set_ALLPRAM_t *cmd      = (const STX_Set_ALLPRAM_t *)Msg;
    void                    *txdata   = (void *)&cmd->Payload;
    uint16_t                 txlength = sizeof(cmd->Payload);

    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_SET;
    uint16_t type    = CONFIG_TP_ALLPARAM;

    uint8_t rxbuf[sizeof(ESUP_Packet_t)+sizeof(uint8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*)rxbuf;

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(uint8_t));
        
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_SET_ALL_PRAMETERS, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                break;
            }  
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_SET_ALL_PRAMETERS, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_SET_ALL_PRAMETERS, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SET_ALL_PRAMETERS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_SET_ALL_PRAMETERS, ret_status);
        STX_rptsend(STX_SET_ALL_PRAMETERS, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_SET_ALL_PRAMETERS, rxdata->header.com_stt);
        STX_rptsend(STX_SET_ALL_PRAMETERS, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_Set_RS485Cmd(const STX_Set_RS485_t *Msg)
{
    
    const STX_Set_RS485_t *cmd      = (const STX_Set_RS485_t *)Msg;
    void                  *txdata   = (void *)&cmd->Payload.data;
    uint16_t               txlength = sizeof(cmd->Payload.data);

    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_SET;
    uint16_t type    = CONFIG_TP_RS485BAUD;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(uint8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(uint8_t));
        
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_SET_RS485BAUD, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_SET_RS485BAUD, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_SET_RS485BAUD, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }   
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SET_RS485BAUD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_RS485BAUD,
                          (unsigned)cmd->Payload.data, ret_status);
        STX_rptsend(STX_SET_RS485BAUD, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_RS485BAUD,
                          (unsigned)cmd->Payload.data, rxdata->header.com_stt);
        STX_rptsend(STX_SET_RS485BAUD, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    } 
}

void STX_Set_MODULATION_INTERFACECmd(const STX_Set_MODULATION_INTERFACE_t *Msg)
{
    
    const STX_Set_MODULATION_INTERFACE_t *cmd      = (const STX_Set_MODULATION_INTERFACE_t *)Msg;
    void                                 *txdata   = (void *)&cmd->Payload;
    uint16_t                              txlength = sizeof(cmd->Payload);

    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_SET;
    uint16_t type    = CONFIG_TP_MODULATORDTIFC;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(uint8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(uint8_t));
        
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_SET_MODULATOR_DATA_INTERFACE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_SET_MODULATOR_DATA_INTERFACE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_SET_MODULATOR_DATA_INTERFACE, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SET_MODULATOR_DATA_INTERFACE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed Status=%" PRId32, STX_SET_MODULATOR_DATA_INTERFACE, ret_status);
        STX_rptsend(STX_SET_MODULATOR_DATA_INTERFACE, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_SET_MODULATOR_DATA_INTERFACE, rxdata->header.com_stt);
        STX_rptsend(STX_SET_MODULATOR_DATA_INTERFACE, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

/*  FILE COMMAND */
static size_t STX_strnlen(const char *s, size_t maxlen)
{
    const char *p = (const char *)memchr(s, '\0', maxlen);
    return p ? (size_t)(p - s) : maxlen;
}

void STX_DIRCmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = FILESYS_CC_DIR;
    uint16_t type    = FILESYS_TP_NA;

    int dir_entry_count = 0;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(STX_DIR_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);
    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        STX_DIR_t *data;
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, rxdata, sizeof(STX_DIR_t));
            data = (STX_DIR_t *)(rxdata->DCP);
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0)))
            {

                const uint8_t *p   = data->listfile;
                const uint8_t *end = p + sizeof(data->listfile); // 버퍼의 끝

                for (uint16_t i = 0; i < data->file_cnt; i++)
                {
                    // 파일 이름 길이 확인
                    size_t name_len = STX_strnlen((const char *)p, end - p); // end-p: 버퍼 끝까지 남은 최대 길이
                    if (p + name_len + 1 > end)
                    { /* 오류 처리 */
                        break;
                    }

                    // 파일명 저장
                    strncpy(dir_entries[dir_entry_count].path, (const char *)p,
                            sizeof(dir_entries[dir_entry_count].path) - 1);
                    dir_entries[dir_entry_count].path[sizeof(dir_entries[dir_entry_count].path) - 1] =
                        '\0'; // 안전한 NULL 종단 방식 사용
                    p += name_len + 1;

                    // 파일 길이 읽기 전 경계 확인
                    if (p + sizeof(uint32_t) > end)
                    { /* 오류 처리 */
                        break;
                    }

                    // 파일 길이 저장 (LE)
                    OS_printf("%X %X %X %X\n", p[0], p[1], p[2], p[3]);
                    uint32_t sizeTemp;
                    memcpy(&sizeTemp, p, sizeof(uint32_t));
                    OS_printf("Size Temp: %u\n", sizeTemp);
                    dir_entries[dir_entry_count].size = sizeTemp;
                    p += 4;
                    dir_entries[dir_entry_count].index = dir_entry_count;
                    dir_entry_count++;
                    if (dir_entry_count >= 20)
                    {
                        break;
                    }
                }

                // 저장된 배열 출력
                for (uint16_t i = 0; i < dir_entry_count; i++)
                {
                    OS_printf("[%u] %s (%u bytes)\n", (uint16_t)(i + 1), dir_entries[i].path, (uint32_t)dir_entries[i].size);
                }
                STX_rptsend(STX_FILESYS_CC_DIR, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_FILESYS_CC_DIR, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else
            {
                CFE_EVS_SendEvent(STX_FILESYS_DIR_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                                STX_FILESYS_CC_DIR, ret_status);
                STX_rptsend(STX_FILESYS_CC_DIR, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_FILESYS_DIR_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_FILESYS_CC_DIR, ret_status);
        STX_rptsend(STX_FILESYS_CC_DIR, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_FILESYS_CC_DIR, rxdata->header.com_stt);
        STX_rptsend(STX_FILESYS_CC_DIR, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_DIRNEXTCmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = FILESYS_CC_DIRNEXT;
    uint16_t type    = FILESYS_TP_NA;

    int dir_entry_count = 0;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(STX_DIR_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);
    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        STX_DIR_t *data;
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, rxdata, sizeof(STX_DIR_t));
            data = (STX_DIR_t *)(rxdata->DCP);
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0)))
            {

                const uint8_t *p   = data->listfile;
                const uint8_t *end = p + sizeof(data->listfile); // 버퍼의 끝

                for (uint16_t i = 0; i < data->file_cnt; i++)
                {

                    // 파일 이름 길이 확인
                    size_t name_len = STX_strnlen((const char *)p, end - p); // end-p: 버퍼 끝까지 남은 최대 길이
                    if (p + name_len + 1 > end)
                    { /* 오류 처리 */
                        break;
                    }

                    // 파일명 저장
                    strncpy(dir_entries[dir_entry_count].path, (const char *)p,
                            sizeof(dir_entries[dir_entry_count].path) - 1);
                    dir_entries[dir_entry_count].path[sizeof(dir_entries[dir_entry_count].path) - 1] =
                        '\0'; // 안전한 NULL 종단 방식 사용
                    p += name_len + 1;

                    // 파일 길이 읽기 전 경계 확인
                    if (p + sizeof(uint32_t) > end)
                    { /* 오류 처리 */
                        break;
                    }

                    // 파일 길이 저장 (LE)
                    OS_printf("%X %X %X %X\n", p[0], p[1], p[2], p[3]);
                    uint32_t sizeTemp;
                    memcpy(&sizeTemp, p, sizeof(uint32_t));
                    OS_printf("Size Temp: %u\n", sizeTemp);
                    dir_entries[dir_entry_count].size = sizeTemp;
                    p += 4;
                    dir_entries[dir_entry_count].index = dir_entry_count;
                    dir_entry_count++;
                    if (dir_entry_count >= 20)
                    {
                        break;
                    }
                }

                // 저장된 배열 출력
                for (uint16_t i = 0; i < dir_entry_count; i++)
                {
                    OS_printf("[%u] %s (%u bytes)\n", (uint16_t)(i + 1), dir_entries[i].path, (uint32_t)dir_entries[i].size);
                }
                STX_rptsend(STX_FILESYS_CC_DIRNEXT, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_FILESYS_CC_DIRNEXT, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else
            {
                CFE_EVS_SendEvent(STX_FILESYS_DIR_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                                STX_FILESYS_CC_DIR, ret_status);
                STX_rptsend(STX_FILESYS_CC_DIRNEXT, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_FILESYS_DIR_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_FILESYS_CC_DIR, ret_status);
        STX_rptsend(STX_FILESYS_CC_DIRNEXT, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_FILESYS_CC_DIR, rxdata->header.com_stt);
        STX_rptsend(STX_FILESYS_CC_DIRNEXT, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_DELFILECmd(const STX_DELFILE_t *Msg)
{
    
    const STX_DELFILE_t *cmd = (const STX_DELFILE_t *)Msg;

    uint8_t  txbuf[31];
    uint16_t txlength = 0;

    memcpy(txbuf, cmd->Payload.filename_max, (size_t)cmd->Payload.filename_len);
    txbuf[cmd->Payload.filename_len] = '\0';

    void *txdata = (void *)txbuf;
    txlength     = (uint16_t)(cmd->Payload.filename_len + 1);

    int16_t  status  = ESUP_INSIG;
    uint16_t command = FILESYS_CC_DELFILE;
    uint16_t type    = FILESYS_TP_NA;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(uint8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, rxdata, 0);
    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, rxdata, sizeof(uint8_t));
        
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_FILESYS_CC_DELFILE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_FILESYS_CC_DELFILE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_FILESYS_CC_DELFILE, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_FILESYS_DELFILE_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_FILESYS_CC_DELFILE, ret_status);
        STX_rptsend(STX_FILESYS_CC_DELFILE, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_FILESYS_CC_DELFILE, rxdata->header.com_stt);
        STX_rptsend(STX_FILESYS_CC_DELFILE, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_DELALLFILECmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = FILESYS_CC_DELALLFILE;
    uint16_t type    = FILESYS_TP_NA;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(uint8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, rxdata, sizeof(uint8_t));
        
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_FILESYS_CC_DELALLFILE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_FILESYS_CC_DELALLFILE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_FILESYS_CC_DELALLFILE, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_FILESYS_DELALLFILE_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_FILESYS_CC_DELALLFILE, ret_status);
        STX_rptsend(STX_FILESYS_CC_DELALLFILE, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_FILESYS_CC_DELALLFILE, rxdata->header.com_stt);
        STX_rptsend(STX_FILESYS_CC_DELALLFILE, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_CREATEFILECmd(const STX_CREATEFILE_t *Msg)
{
    
    const STX_CREATEFILE_t *cmd = (const STX_CREATEFILE_t *)Msg;

    uint8_t  txbuf[35];
    uint16_t txlength = 0;

    memcpy(txbuf, cmd->Payload.filename_max, (size_t)cmd->Payload.filename_len);
    txbuf[cmd->Payload.filename_len] = '\0';
    memcpy(txbuf + cmd->Payload.filename_len + 1, &cmd->Payload.file_size, sizeof(cmd->Payload.file_size));

    void *txdata = (void *)txbuf;
    txlength     = (uint16_t)(cmd->Payload.filename_len + 1 + sizeof(cmd->Payload.file_size));

    int16_t  status  = ESUP_INSIG;
    uint16_t command = FILESYS_CC_CREATEFILE;
    uint16_t type    = FILESYS_TP_NA;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(STX_FILE_CREATE_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        STX_FILE_CREATE_t *data;
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, rxdata, sizeof(STX_FILE_CREATE_t));
        
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                data = (STX_FILE_CREATE_t*)(rxdata->DCP);
                file_handle = data->file_handle;
                STX_rptsend(STX_FILESYS_CC_CREATEFILE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_printf("Creating file: %s, size: %u, txlen: %u\n", cmd->Payload.filename_max, cmd->Payload.file_size,
                    txlength);
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_SET_CENTERFREQ, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_FILESYS_CC_CREATEFILE, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_FILESYS_CREATEFILE_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_FILESYS_CC_CREATEFILE, ret_status);
        STX_rptsend(STX_FILESYS_CC_CREATEFILE, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_FILESYS_CC_CREATEFILE, rxdata->header.com_stt);
        STX_rptsend(STX_FILESYS_CC_CREATEFILE, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }
    OS_printf("**********file handle : 0x%08X ************ \n", file_handle);
}

#define WRITE_OPEN_ERROR -1001
#define WRITE_READ_ERROR -1002

/* 실험 필요*/
void STX_WRITEFILECmd(const STX_WRITEFILE_t *Msg)
{
    
    const STX_WRITEFILE_t *cmd = (const STX_WRITEFILE_t *)Msg;

    FILE    *fp;
    size_t   remaining;
    uint32_t packet_number = cmd->Payload.offset;

    OS_printf("STX_WRITEFILECmd called: filename=%s, size=%u, offset=%u, interpacket_delay=%u\n", cmd->Payload.filename,
             cmd->Payload.size, cmd->Payload.offset, cmd->Payload.interpacket_delay);

    struct __attribute__((__packed__))
    {
        int      ret;
        uint32_t packet_number;
        uint8_t  cstatus;
        uint8_t  err_count;
        uint8_t  data;
    } write_status = {0, 0, 0, 0};

    int16_t                      status  = ESUP_INSIG;
    uint16_t                     command = FILESYS_CC_WRITEFILE;
    uint16_t                     type    = FILESYS_TP_NA;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(uint8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;
    STX_ESUP_WRITEFILE_Payload_t writePacket;
    void                        *txdata = (void *)&writePacket;

    fp = fopen(cmd->Payload.filename, "rb");
    if (fp == NULL)
    {
        write_status.ret           = WRITE_OPEN_ERROR;
        write_status.packet_number = packet_number;
        OS_printf("file open failed\n");
        return;
    }
    if (cmd->Payload.size == 0 && cmd->Payload.offset == 0)
    {
        fseek(fp, 0, SEEK_END);
        remaining = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        OS_printf("file size : %d \n", (int)remaining);
    }
    
    else if (cmd->Payload.size == 0 && cmd->Payload.offset != 0)
    {
        fseek(fp, 0, SEEK_END);
        remaining = ftell(fp) - cmd->Payload.offset*ESUP_MAX_WRITE_LENGTH;
        fseek(fp, cmd->Payload.offset * ESUP_MAX_WRITE_LENGTH, SEEK_SET);
    }

    else if (cmd->Payload.size != 0 && cmd->Payload.offset == 0)
    {
        remaining = cmd->Payload.size;
    }

    else //cmd->Payload.size != 0 && cmd->Payload.offset != 0
    {
        remaining = cmd->Payload.size - (cmd->Payload.offset * ESUP_MAX_WRITE_LENGTH);
        fseek(fp, cmd->Payload.offset * ESUP_MAX_WRITE_LENGTH, SEEK_SET);
    }

    writePacket.file_handle = file_handle;
    OS_printf("write file ready: size=%zu, offset=%u, handle=%u\n", remaining, cmd->Payload.offset, file_handle);

    int count = 0;
    uint8_t err_count = 0;

    while (remaining > 0)
    {
        size_t bytes_to_read = (remaining > ESUP_MAX_WRITE_LENGTH) ? ESUP_MAX_WRITE_LENGTH : remaining;
        OS_printf("remain : %zu / max_length : %d / bytes_to_read : %zu \n", remaining, ESUP_MAX_WRITE_LENGTH, bytes_to_read);
        size_t bytes_read    = fread(writePacket.packet_data, 1, bytes_to_read, fp);
        
        if (bytes_read != bytes_to_read)
        {
            fclose(fp);
            write_status.ret           = WRITE_READ_ERROR;
            write_status.packet_number = packet_number;
            return;
        }

        writePacket.data_length   = (uint16_t)bytes_read;
        writePacket.packet_number = packet_number;

        uint16_t txlength   = (uint16_t)(offsetof(STX_ESUP_WRITEFILE_Payload_t, packet_data) + writePacket.data_length);
        int32_t  ret_status = ESUP(status, command, type, txdata, txlength, rxdata, 0);

        if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
        {
            if (cmd->Payload.interpacket_delay)
            usleep(cmd->Payload.interpacket_delay * 1000);

            ret_status = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, rxdata, sizeof(uint8_t));
            OS_printf("---------------------------------------------------\n");
            if (ret_status == STX_ESUP_READ_ERR || ret_status == STX_ESUP_DECODER_ERR)
            {
                remaining -= bytes_read;
                count += bytes_read;
                packet_number++;

                OS_printf("ret status error : %#x\n", ret_status);
                OS_printf("remain file size : %d\n", (int)remaining-ESUP_MAX_WRITE_LENGTH);
                OS_printf("Write file size : %d\n", (int)count + ESUP_MAX_WRITE_LENGTH);
                OS_printf("wirte packet num : %d\n", (int)(packet_number));

                if (err_count > 2){
                    write_status.ret = ret_status;
                    write_status.packet_number = packet_number;
                    write_status.cstatus = rxdata->header.com_stt;
                    write_status.err_count = err_count;
                    memcpy(&write_status.data, rxdata->DCP, sizeof(uint8_t));
                    STX_rptsend(STX_FILESYS_CC_WRITEFILE, STX_rpt_type_CFE_ERROR, ret_status, sizeof(write_status), &write_status);
                    fclose(fp);
                    return;
                }
               
                err_count += 1;

                write_status.ret = ret_status;
                write_status.packet_number = packet_number;
                write_status.cstatus = rxdata->header.com_stt;
                write_status.err_count = err_count;
                memcpy(&write_status.data, rxdata->DCP, sizeof(uint8_t));
                STX_rptsend(STX_FILESYS_CC_WRITEFILE, STX_rpt_type_CFE_ERROR, ret_status, sizeof(write_status), &write_status);
                OS_TaskDelay(10);
                
                OS_printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!error count!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                
                continue;
            }
            
            else if (ret_status != CFE_SUCCESS || (ret_status == CFE_SUCCESS && rxdata->DCP[0] != 0))
            {
                OS_printf("ret status error : %#x\n", ret_status);
                OS_printf("remain file size : %d\n", (int)remaining);
                OS_printf("wirte packet num : %d\n", (int)packet_number);

                if (err_count > 2){
                    write_status.ret = ret_status;
                    write_status.packet_number = packet_number;
                    write_status.cstatus = rxdata->header.com_stt;
                    write_status.err_count = err_count;
                    memcpy(&write_status.data, rxdata->DCP, sizeof(uint8_t));
                    STX_rptsend(STX_FILESYS_CC_WRITEFILE, STX_rpt_type_CFE_ERROR, ret_status, sizeof(write_status), &write_status);
                    fclose(fp);
                    return;
                }
                
                OS_printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!error!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                
                err_count += 1;
                
                write_status.ret = ret_status;
                write_status.packet_number = packet_number;
                write_status.cstatus = rxdata->header.com_stt;
                write_status.err_count = err_count;
                memcpy(&write_status.data, rxdata->DCP, sizeof(uint8_t));
                STX_rptsend(STX_FILESYS_CC_WRITEFILE, STX_rpt_type_CFE_ERROR, ret_status, sizeof(write_status), &write_status);
                OS_TaskDelay(10);
                
                continue;
            }

            else{
                remaining -= bytes_read;
                count += bytes_read;
                packet_number++;

                OS_printf("remain file size : %d \n", (int)remaining);
                OS_printf("Write file size : %d\n", (int)count);
                OS_printf("write packet num : %d\n", (int)(packet_number));

                ESUP_ACK_CMD(ESUP_ACK, command, FILESYS_TP_NA);

                err_count = 0;

                write_status.ret = ret_status;
                write_status.packet_number = packet_number;
                write_status.cstatus = rxdata->header.com_stt;
                write_status.err_count = err_count;
                memcpy(&write_status.data, rxdata->DCP, sizeof(uint8_t));
                STX_rptsend(STX_FILESYS_CC_WRITEFILE, STX_rpt_type_RESULT, ret_status, sizeof(write_status), &write_status);

                if (cmd->Payload.interpacket_delay)
                    usleep(cmd->Payload.interpacket_delay * 1000);
            }
        }

        else if (ret_status != CFE_SUCCESS)
        {
            OS_printf("ret status error : %#x\n", ret_status);
            OS_printf("remain file size : %d\n", (int)remaining);
            OS_printf("wirte packet num : %d\n", (int)packet_number);

            if (err_count > 2){
                write_status.ret           = ret_status;
                write_status.packet_number = packet_number;
                write_status.cstatus = rxdata->header.com_stt;
                write_status.err_count = err_count;
                memcpy(&write_status.data, rxdata->DCP, sizeof(uint8_t));
                STX_rptsend(STX_FILESYS_CC_WRITEFILE, STX_rpt_type_CFE_ERROR, ret_status, sizeof(write_status), &write_status);
                fclose(fp);
                return;
            }
            
            OS_printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!error!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            
            err_count += 1;
            
            write_status.ret = ret_status;
            write_status.packet_number = packet_number;
            write_status.cstatus = rxdata->header.com_stt;
            write_status.err_count = err_count;
            memcpy(&write_status.data, rxdata->DCP, sizeof(uint8_t));
            STX_rptsend(STX_FILESYS_CC_WRITEFILE, STX_rpt_type_CFE_ERROR, ret_status, sizeof(write_status), &write_status);
            OS_TaskDelay(10);
            
            continue;
        }

        else if (rxdata->header.com_stt != ESUP_ACK)
        {
            OS_printf("cstatus error : %#x", rxdata->header.com_stt);
            OS_printf("remain file size : %d\n", (int)remaining);
            OS_printf("wirte packet num : %d\n", (int)packet_number);

            if (err_count > 2){
                write_status.ret           = ret_status;
                write_status.packet_number = packet_number;
                write_status.cstatus       = rxdata->header.com_stt;
                write_status.err_count = err_count;
                memcpy(&write_status.data, rxdata->DCP, sizeof(uint8_t));
                STX_rptsend(STX_FILESYS_CC_WRITEFILE, STX_rpt_type_ACK_ERROR, ret_status, sizeof(write_status), &write_status);
                fclose(fp);
                return;
            }
            
            OS_printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!error!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            
            err_count += 1;
            
            write_status.ret = ret_status;
            write_status.packet_number = packet_number;
            write_status.cstatus = rxdata->header.com_stt;
            write_status.err_count = err_count;
            memcpy(&write_status.data, rxdata->DCP, sizeof(uint8_t));
            STX_rptsend(STX_FILESYS_CC_WRITEFILE, STX_rpt_type_CFE_ERROR, ret_status, sizeof(write_status), &write_status);
            OS_TaskDelay(10);
            
            continue;
        }

        else{
            if (err_count > 2){

                write_status.ret = ret_status;
                write_status.packet_number = packet_number;
                write_status.cstatus = rxdata->header.com_stt;
                write_status.err_count = err_count;
                memcpy(&write_status.data, rxdata->DCP, sizeof(uint8_t));
                STX_rptsend(STX_FILESYS_CC_WRITEFILE, STX_rpt_type_CFE_ERROR, ret_status, sizeof(write_status), &write_status);

                return;
            }
            
            OS_printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!error!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            
            err_count += 1;

            write_status.ret = ret_status;
            write_status.packet_number = packet_number;
            write_status.cstatus = rxdata->header.com_stt;
            write_status.err_count = err_count;
            memcpy(&write_status.data, rxdata->DCP, sizeof(uint8_t));
            STX_rptsend(STX_FILESYS_CC_WRITEFILE, STX_rpt_type_CFE_ERROR, ret_status, sizeof(write_status), &write_status);
            OS_TaskDelay(10);

            continue;
        }
        OS_printf("?????????????????????????????????error count: %u ??????????????????????????????????????????????\n", err_count);
    }

    fclose(fp);
}

void STX_OPENFILECmd(const STX_OPENFILE_t *Msg)
{

    const STX_OPENFILE_t *cmd = (const STX_OPENFILE_t *)Msg;

    uint8_t  txbuf[31];
    uint16_t txlength = 0;

    memcpy(txbuf, cmd->Payload.filename_max, (size_t)cmd->Payload.filename_len);

    txbuf[cmd->Payload.filename_len] = '\0';

    void *txdata = (void *)txbuf;
    txlength     = (uint16_t)(cmd->Payload.filename_len + 1);

    int16_t  status  = ESUP_INSIG;
    uint16_t command = FILESYS_CC_OPENFILE;
    uint16_t type    = FILESYS_TP_NA;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(STX_FILE_OPEN_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, rxdata, 0);
    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        STX_FILE_OPEN_t* data;
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status  = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, rxdata, sizeof(STX_FILE_OPEN_t));

            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0)))
            {   
                data = (STX_FILE_OPEN_t *)(rxdata->DCP);
                file_handle = data->file_handle;
                OS_printf("Status : 0x%02x , Handle : 0x%08x , File Length : 0x%08x \n", data->commad_status, data->file_handle, data->file_length);
                STX_rptsend(STX_FILESYS_CC_OPENFILE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_FILESYS_CC_OPENFILE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_FILESYS_CC_OPENFILE, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        STX_rptsend(STX_FILESYS_CC_OPENFILE, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        STX_rptsend(STX_FILESYS_CC_OPENFILE, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }
}

void STX_READFILECmd(const STX_READFILE_t *Msg)
{
    void    *txdata   = (void *)&file_handle;
    uint16_t txlength = sizeof(file_handle);

    int16_t  status  = ESUP_INSIG;
    uint16_t command = FILESYS_CC_READFILE;
    uint16_t type    = FILESYS_TP_NA;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(STX_FILE_READ_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    uint16_t STX_timeout = 30;
    long t1 = latch_ms();
    
    int32_t ret_status;
    
    while(open_file_size && (latch_ms() - t1 < STX_timeout)){
        
        ret_status = ESUP(status, command, type, txdata, txlength, rxdata, 0);
        
        if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
        {
            ret_status = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, rxdata, sizeof(STX_FILE_READ_t));

            STX_FILE_READ_t* data = (STX_FILE_READ_t *)(rxdata->DCP);
            
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0)))
            {
                OS_printf("read byte : %d \n", (int)data->Packet_length);
            
                open_file_size -= (uint32_t)data->Packet_length;

                OS_printf("remain byte : %d \n", open_file_size);

                STX_rptsend(STX_FILESYS_CC_READFILE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), &rxdata);

                uint8_t file_retdata[data->Packet_length];
                memcpy(file_retdata, data->file_data, data->Packet_length);

                OS_printf("file_retdata: %s\n", file_retdata);
                
                t1 = latch_ms();
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_FILESYS_CC_READFILE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else
            {
                CFE_EVS_SendEvent(STX_FILESYS_READFILE_ERR_EID, CFE_EVS_EventType_ERROR,
                                "STX : CC=%u failed Status=%" PRId32, STX_FILESYS_CC_READFILE, ret_status);
                STX_rptsend(STX_FILESYS_CC_READFILE, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
        else if (ret_status != CFE_SUCCESS)
        {
            OS_printf("ReadFile ERR \n");
            CFE_EVS_SendEvent(STX_FILESYS_READFILE_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                            STX_FILESYS_CC_READFILE, ret_status);
            STX_rptsend(STX_FILESYS_CC_READFILE, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
        }
        else if (rxdata->header.com_stt != ESUP_ACK)
        {
            OS_printf("Command status ERR\n");
            CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                            STX_FILESYS_CC_READFILE, rxdata->header.com_stt);
            STX_rptsend(STX_FILESYS_CC_READFILE, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
        }
    }
   
}

void STX_SENDFILE_WITH_ERROR_Cmd(const STX_SENDFILE_t * Msg)
{
    const STX_SENDFILE_t *cmd = (const STX_SENDFILE_t *)Msg;
    uint8_t txbuf[31];
    uint16_t txlength = 0;
    
    memcpy(txbuf, cmd->Payload.filename_max, (size_t)cmd->Payload.filename_len);
    
    txbuf[cmd->Payload.filename_len] = '\0';
    
    void *txdata = (void *)txbuf;
    txlength = (uint16_t)(cmd->Payload.filename_len + 1);
    
    int16_t status = ESUP_INSIG;
    uint16_t command = FILESYS_CC_SENDFILE;
    uint16_t type = FILESYS_TP_SENDFILERTI;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(uint8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, rxdata, 0);
    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        uint8_t err_count = 0;

        uint16_t request_data = 0x0050;

        while (err_count < 4){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, (void *)&request_data, sizeof(request_data), rxdata, sizeof(uint8_t));

            if (ret_status == DEVICE_SUCCESS && (rxdata->header.com_stt == 0x0007)){
                err_count = 0;
                OS_printf("File still sending.... \n");
                STX_rptsend(STX_FILESYS_CC_SENDFILERTI, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                sleep(5);
            }

            else if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                OS_printf("File sending finish! \n");
                ESUP_ACK_CMD(ESUP_ACK, command, FILESYS_TP_SENDFILE);
                STX_rptsend(STX_FILESYS_CC_SENDFILERTI, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                return;
            }

            else if (ret_status == DEVICE_SUCCESS){
                err_count += 1;
                sleep(5);
                STX_rptsend(STX_FILESYS_CC_SENDFILERTI, STX_rpt_type_RESULT, err_count, sizeof(*rxdata), rxdata);
            }

            else{
                err_count += 1;
                STX_rptsend(STX_FILESYS_CC_SENDFILERTI, STX_rpt_type_RESULT, ret_status, 0, NULL);
                sleep(5);  
            }
        }

        OS_printf("fail to sending file");
        STX_rptsend(STX_FILESYS_CC_SENDFILERTI, STX_rpt_type_FINAL_ACK_ERROR, err_count, 0, NULL);
    }

    else if (ret_status != CFE_SUCCESS)
    {
        STX_rptsend(STX_FILESYS_CC_SENDFILERTI, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }

    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        STX_rptsend(STX_FILESYS_CC_SENDFILERTI, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }
    
}

void STX_SENDFILECmd(const STX_SENDFILE_t *Msg)
{
    
    const STX_SENDFILE_t *cmd = (const STX_SENDFILE_t *)Msg;
    uint8_t               txbuf[31];
    uint16_t              txlength = 0;

    memcpy(txbuf, cmd->Payload.filename_max, (size_t)cmd->Payload.filename_len);

    txbuf[cmd->Payload.filename_len] = '\0';

    void *txdata = (void *)txbuf;
    txlength     = (uint16_t)(cmd->Payload.filename_len + 1);

    int16_t  status  = ESUP_INSIG;
    uint16_t command = FILESYS_CC_SENDFILE;
    uint16_t type    = FILESYS_TP_SENDFILE;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(uint8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, rxdata, 0);
    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        uint8_t err_count = 0;

        uint16_t request_data = 0x0050;

        while (err_count < 4){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, (void *)&request_data, sizeof(request_data), rxdata, sizeof(uint8_t));

            if (ret_status == DEVICE_SUCCESS && (rxdata->header.com_stt == 0x0007)){
                err_count = 0;
                OS_printf("File still sending.... \n");
                STX_rptsend(FILESYS_CC_SENDFILE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                sleep(5);
            }

            else if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                OS_printf("File sending finish! \n");
                ESUP_ACK_CMD(ESUP_ACK, command, FILESYS_TP_SENDFILE);
                STX_rptsend(FILESYS_CC_SENDFILE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                return;
            }

            else if (ret_status == DEVICE_SUCCESS){
                err_count += 1;
                sleep(5);
                STX_rptsend(FILESYS_CC_SENDFILE, STX_rpt_type_RESULT, err_count, sizeof(*rxdata), rxdata);
            }

            else{
                err_count += 1;
                STX_rptsend(FILESYS_CC_SENDFILE, STX_rpt_type_RESULT, ret_status, 0, NULL);
                sleep(5);  
            }
        }

        OS_printf("fail to sending file");
        STX_rptsend(FILESYS_CC_SENDFILE, STX_rpt_type_FINAL_ACK_ERROR, err_count, 0, NULL);
    }

    else if (ret_status != CFE_SUCCESS)
    {
        STX_rptsend(STX_FILESYS_CC_SENDFILE, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }

    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        STX_rptsend(STX_FILESYS_CC_SENDFILE, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }
}

/*  mode command    */

void STX_SYSCONF_CC_TRANSMITMODECmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = SYSCONF_CC_TRANSMITMODE;
    uint16_t type    = SYSCONF_TP_NA;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(uint8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        sleep(15);
        uint8_t err_count = 0;
        while (err_count < 4){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, rxdata, sizeof(uint8_t));

            if (ret_status == DEVICE_SUCCESS && (rxdata->header.com_stt == 0x0007)){
                err_count = 0;
                OS_printf("Stilling mode changing... \n");
                STX_rptsend(STX_SYSCONF_CC_TRANSMITMODE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                sleep(5);
            }

            else if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                OS_printf("Turn to Tx mode Success! \n");
                ESUP_ACK_CMD(ESUP_ACK, command, SYSCONF_TP_NA);
                STX_rptsend(STX_SYSCONF_CC_TRANSMITMODE,STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                return;
            }

            else if (ret_status == DEVICE_SUCCESS){
                err_count += 1;
                STX_rptsend(STX_SYSCONF_CC_TRANSMITMODE, STX_rpt_type_RESULT,err_count, sizeof(*rxdata), rxdata);
                sleep(5);
            }

            else{
            err_count += 1;
            STX_rptsend(STX_SYSCONF_CC_TRANSMITMODE, STX_rpt_type_RESULT, ret_status, 0, NULL);
            sleep(5);
            }
        }
        
        OS_printf("fail to turn Tx mode");
        STX_rptsend(STX_SYSCONF_CC_TRANSMITMODE, STX_rpt_type_FINAL_ACK_ERROR, err_count, 0, NULL);
    }

    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SYSCONF_TRANSMITMODE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed Status=%" PRId32, STX_SYSCONF_CC_TRANSMITMODE, ret_status);
        STX_rptsend(STX_SYSCONF_CC_TRANSMITMODE, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_SYSCONF_CC_TRANSMITMODE, rxdata->header.com_stt);
        STX_rptsend(STX_SYSCONF_CC_TRANSMITMODE, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_SYSCONF_CC_IDLEMODECmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = SYSCONF_CC_IDLEMODE;
    uint16_t type    = SYSCONF_TP_NA;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(uint8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);
    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, rxdata, sizeof(uint8_t));
        
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_SYSCONF_CC_IDLEMODE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_SYSCONF_CC_IDLEMODE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_SYSCONF_CC_IDLEMODE, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SYSCONF_IDLEMODE_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_SYSCONF_CC_IDLEMODE, ret_status);
        STX_rptsend(STX_SYSCONF_CC_IDLEMODE, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_SYSCONF_CC_IDLEMODE, rxdata->header.com_stt);
        STX_rptsend(STX_SYSCONF_CC_IDLEMODE, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    } 
}

void STX_SYSCONF_CC_SAFESHUTDOWNCmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = SYSCONF_CC_SAFESHUTDOWN;
    uint16_t type    = SYSCONF_TP_NA;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(uint8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);
    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, rxdata, sizeof(uint8_t));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_SYSCONF_CC_SAFESHUTDOWN, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
        else
            STX_rptsend(STX_SYSCONF_CC_SAFESHUTDOWN, STX_rpt_type_RESULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SYSCONF_SAFESHUTDOWN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed Status=%" PRId32, STX_SYSCONF_CC_SAFESHUTDOWN, ret_status);
        STX_rptsend(STX_SYSCONF_CC_SAFESHUTDOWN, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_SYSCONF_CC_SAFESHUTDOWN, rxdata->header.com_stt);
        STX_rptsend(STX_SYSCONF_CC_SAFESHUTDOWN, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

/* Get command */

void STX_GET_SYMBOL_RATECmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = CONFIG_TP_SYMBOLRATE;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(STX_GET_U8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(STX_GET_U8_t));
        
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_GET_SYMBOL_RATE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                STX_GET_U8_t* data = (STX_GET_U8_t*)(rxdata->DCP);
                OS_printf("Command Status : %d \n", (int)(data->commad_status));
                OS_printf("Symbol Rate : %d \n", (int)(data->rxdata_val));
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_GET_SYMBOL_RATE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_GET_SYMBOL_RATE, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_GET_SYMBOL_RATE_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_SYMBOL_RATE, ret_status);
        STX_rptsend(STX_GET_SYMBOL_RATE, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_SYMBOL_RATE, rxdata->header.com_stt);
        STX_rptsend(STX_GET_SYMBOL_RATE, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }
}

void STX_GET_TX_POWERCmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = CONFIG_TP_TRANSMITPW;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(STX_GET_U8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(STX_GET_U8_t));
        
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_GET_TX_POWER, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                STX_GET_U8_t* data = (STX_GET_U8_t*)(rxdata->DCP);
                OS_printf("command Status : %d \n", (int)(data->commad_status));
                OS_printf("Tx Power : %d \n", (int)(data->rxdata_val));
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_GET_TX_POWER, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_GET_TX_POWER, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_GET_TX_POWER_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_TX_POWER, ret_status);
        STX_rptsend(STX_GET_TX_POWER, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_TX_POWER, rxdata->header.com_stt);
        STX_rptsend(STX_GET_TX_POWER, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_GET_CENTER_FREQCmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = CONFIG_TP_CENTERFREQ;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(STX_GET_FLOAT_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*)rxbuf;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(STX_GET_FLOAT_t));
        
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_GET_CENTER_FREQ, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                STX_GET_FLOAT_t* data = (STX_GET_FLOAT_t*)(rxdata->DCP);
                OS_printf("Command Status : %d \n", (int)(data->commad_status));
                OS_printf("Center Freq : %.3f \n", data->rxdata_val);
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_GET_CENTER_FREQ, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_GET_CENTER_FREQ, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_GET_CENTER_FREQ_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_CENTER_FREQ, ret_status);
        STX_rptsend(STX_GET_CENTER_FREQ, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_CENTER_FREQ, rxdata->header.com_stt);
        STX_rptsend(STX_GET_CENTER_FREQ, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_GET_MODCODCmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = CONFIG_TP_MODCOD;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(STX_GET_U8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(STX_GET_U8_t));
            
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_GET_MODCOD, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                STX_GET_U8_t* data = (STX_GET_U8_t*)(rxdata->DCP);
                OS_printf("Command Status : %d", (int)(data->commad_status));
                OS_printf("Mod Code (1/4 QPSK -> 1) : %d \n", (int)(data->rxdata_val));
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_GET_MODCOD, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_GET_MODCOD, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_GET_MODCOD_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_MODCOD, ret_status);
        STX_rptsend(STX_GET_MODCOD, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_MODCOD, rxdata->header.com_stt);
        STX_rptsend(STX_GET_MODCOD, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_GET_ROLL_OFFCmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = CONFIG_TP_ROLLOFF;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(STX_GET_U8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(STX_GET_U8_t));
        
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_GET_ROLL_OFF, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                STX_GET_U8_t* data = (STX_GET_U8_t*)(rxdata->DCP);
                OS_printf("Command Status : %d \n", (int)(data->commad_status));
                OS_printf("Roll Off (0.35 -> 0 / 0.25 -> 1 / 0.2 -> 2) : %d \n", (int)(data->rxdata_val));
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_SET_CENTERFREQ, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_GET_ROLL_OFF, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_GET_ROLL_OFF_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_ROLL_OFF, ret_status);
        STX_rptsend(STX_GET_ROLL_OFF, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_ROLL_OFF, rxdata->header.com_stt);
        STX_rptsend(STX_GET_ROLL_OFF, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_GET_PILOT_SIGNALCmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = CONFIG_TP_PILOTSIG;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(STX_GET_U8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(STX_GET_U8_t));
        
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_GET_PILOT_SIGNAL, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                STX_GET_U8_t* data = (STX_GET_U8_t*)(rxdata->DCP);
                OS_printf("Command Status : %d \n", (int)(data->commad_status));
                OS_printf("Pilot Signal(On -> 1 / Off -> 0)) : %d \n", (int)(data->rxdata_val));
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_GET_PILOT_SIGNAL, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_GET_PILOT_SIGNAL, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_GET_PILOT_SIGNAL_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_PILOT_SIGNAL, ret_status);
        STX_rptsend(STX_GET_PILOT_SIGNAL, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_PILOT_SIGNAL, rxdata->header.com_stt);
        STX_rptsend(STX_GET_PILOT_SIGNAL, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_GET_FEC_FRAME_SIZECmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = CONFIG_TP_FECFRAMESZ;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(STX_GET_U8_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(STX_GET_U8_t));
        
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_GET_FEC_FRAME_SIZE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                STX_GET_U8_t* data = (STX_GET_U8_t*)(rxdata->DCP);
                OS_printf("Command Status : %d \n", (int)(data->commad_status));
                OS_printf("FEC Frame(Short -> 1 / Normal -> 0)) : %d \n", (int)(data->rxdata_val));
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_GET_FEC_FRAME_SIZE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_GET_FEC_FRAME_SIZE, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_GET_FEC_FRAME_SIZE_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_FEC_FRAME_SIZE, ret_status);
        STX_rptsend(STX_GET_FEC_FRAME_SIZE, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_FEC_FRAME_SIZE, rxdata->header.com_stt);
        STX_rptsend(STX_GET_FEC_FRAME_SIZE, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_GET_PRETX_DELAYCmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = CONFIG_TP_PRETXSTUFFDEL;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(STX_GET_U16_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(STX_GET_U16_t));
        
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_GET_PRETX_DELAY, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                STX_GET_U16_t* data = (STX_GET_U16_t*)(rxdata->DCP);
                OS_printf("Command Status : %d \n", (int)(data->commad_status));
                OS_printf("PRE_TX_Delay : %d \n", (int)(data->rxdata_val));
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_GET_PRETX_DELAY, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_GET_PRETX_DELAY, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_GET_PRETX_DELAY_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_PRETX_DELAY, ret_status);
        STX_rptsend(STX_GET_PRETX_DELAY, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_PRETX_DELAY, rxdata->header.com_stt);
        STX_rptsend(STX_GET_PRETX_DELAY, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_GET_ALL_PRAMETERSCmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = CONFIG_TP_ALLPARAM;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(STX_GET_ALLPRAM_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(STX_GET_ALLPRAM_t));
            
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                STX_rptsend(STX_GET_ALL_PRAMETERS, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                STX_GET_ALLPRAM_t* data = (STX_GET_ALLPRAM_t*)(rxdata->DCP);
                OS_printf("Command Status : %d \n", (int)(data->command_status));
                OS_printf("Symbol Rate : %d \n", (int)(data->symbol_rate));
                OS_printf("Tx Power : %d \n", (int)(data->transmit_power));
                OS_printf("Mod Code (1/4 QPSK -> 1) : %d \n", (int)(data->modcod));
                OS_printf("Roll Off (0.35 -> 0 / 0.25 -> 1 / 0.2 -> 2) : %d \n", (int)(data->roll_off));
                OS_printf("Pilot Signal(On -> 1 / Off -> 0)) : %d \n", (int)(data->pilot_signal));
                OS_printf("FEC Frame(Short -> 1 / Normal -> 0)) : %d \n", (int)(data->fec_frame_size));
                OS_printf("PRE_TX_Delay : %d \n", (int)(data->pretransmission_delay));
                OS_printf("Center Freq : %.3f \n", data->center_frequency);
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_GET_ALL_PRAMETERS, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_GET_ALL_PRAMETERS, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_GET_ALL_PRAMETERS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_ALL_PRAMETERS, ret_status);
        STX_rptsend(STX_GET_ALL_PRAMETERS, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_ALL_PRAMETERS, rxdata->header.com_stt);
        STX_rptsend(STX_GET_ALL_PRAMETERS, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_GET_REPORTCmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = STATUS_TP_SIMPLE_REPORT;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(STX_GET_Report_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(STX_GET_Report_t));
        
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                uint8_t data1;
                float data2;
                uint32_t data3;
                memcpy(&data1, &(rxdata->DCP[1]), sizeof(data1));
                OS_printf("System State (1 -> After Reset / 2 -> Idle / 3 -> Tx Mode / 4 -> Going to Shutdown) : %d \n", (int)data1);
                memcpy(&data1, &(rxdata->DCP[2]), sizeof(data1));
                OS_printf("Status Flags (bit 2 : SDR Not Initialized / bit 4 : System Over Temp Stop) : 0X%02x \n", data1);
                memcpy(&data2, &(rxdata->DCP[5]), sizeof(data2));
                OS_printf("CPU Temp : %f \n", data2);
                memcpy(&data3, &(rxdata->DCP[9]), sizeof(data3));
                OS_printf("Firmware Version : %d \n", data3);

                STX_rptsend(STX_GET_REPORT, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_GET_REPORT, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_GET_REPORT, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_GET_REPORT_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_REPORT, ret_status);
        STX_rptsend(STX_GET_REPORT, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_REPORT, rxdata->header.com_stt);
        STX_rptsend(STX_GET_REPORT, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_GET_MODULATOR_DATA_INTERFACECmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = CONFIG_TP_MODULATORDTIFC;

    uint8_t rxbuf[sizeof(ESUP_Packet_t) + sizeof(STX_GET_ModulationInterface_t)];
    ESUP_Packet_t* rxdata = (ESUP_Packet_t*) rxbuf;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, rxdata, 0);

    if (rxdata->header.com_stt == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        long t1 = latch_ms();
        while(latch_ms() - t1 < 10 * 1000){
            ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), rxdata, sizeof(STX_GET_ModulationInterface_t));
        
            if (ret_status == DEVICE_SUCCESS && ((rxdata->header.length >= 1) && (rxdata->DCP[0] == 0))){
                uint8_t data;
                memcpy(&data, &(rxdata->DCP[1]), sizeof(data));
                OS_printf("Modulator Data Interface (1 -> SD Card / 2 -> LVDS) : %d \n", (int)data);
                memcpy(&data, &(rxdata->DCP[2]), sizeof(data));
                OS_printf("LVDS input/output type ( 0-> PC104 connector / 2 -> LVDS connector) : %d \n", (int)data);
                STX_rptsend(STX_GET_MODULATOR_DATA_INTERFACE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                break;
            }
            else if ((rxdata->header.length >= 1) && (rxdata->DCP[0] != 0)){
                STX_rptsend(STX_GET_MODULATOR_DATA_INTERFACE, STX_rpt_type_RESULT, DEVICE_SUCCESS, sizeof(*rxdata), rxdata);
                OS_TaskDelay(1 * 1000);
            }
            else{
                STX_rptsend(STX_GET_MODULATOR_DATA_INTERFACE, STX_rpt_type_RESULT, ret_status, 0, NULL);
                OS_TaskDelay(1 * 1000);
            }
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_GET_MODULATOR_DATA_INTERFACE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed Status=%" PRId32, STX_GET_MODULATOR_DATA_INTERFACE, ret_status);
        STX_rptsend(STX_GET_MODULATOR_DATA_INTERFACE, STX_rpt_type_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (rxdata->header.com_stt != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_MODULATOR_DATA_INTERFACE, rxdata->header.com_stt);
        STX_rptsend(STX_GET_MODULATOR_DATA_INTERFACE, STX_rpt_type_ACK_ERROR, rxdata->header.com_stt, 0, NULL);
    }

   
}

void STX_Param_init(void)
{
    /* SRL(RS485) 핸들이 부팅 시점에 준비되지 않아 NULL로 캐시되었을 수 있으므로,
       명령 처리 시점(SRL 확실히 기동 후)에 한 번 더 취득해 유효 핸들을 보장한다. */
    if (Handle == NULL)
    {
        Handle = CFE_SRL_ApiGetHandle(CFE_SRL_RS485_HANDLE_INDEXER);
    }

    STX_Set_SYMBOLRAtE_t init_symrate;
    STX_Set_CENTERFREQ_t init_centfreq;
    STX_Set_MODCOD_t init_modcod;
    STX_Set_ROLLOFF_t init_rolloff;
    STX_Set_TRANSMITPW_t init_txpwr;

    init_symrate.Payload.data = 0x02;
    init_centfreq.Payload.data = 2403.5;
    init_modcod.Payload.data = 1;
    init_rolloff.Payload.data = 2;
    init_txpwr.Payload.data = 33;

    STX_SET_SYMBOLRATECmd(&init_symrate);
    STX_Set_CENTERFREQCmd(&init_centfreq);
    STX_Set_MODCODCmd(&init_modcod);
    STX_Set_ROLLOFFCmd(&init_rolloff);
    STX_Set_TRANSMITPWCmd(&init_txpwr);
}