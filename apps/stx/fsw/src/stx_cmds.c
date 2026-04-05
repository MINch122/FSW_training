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
#include <fcntl.h>
#include <unistd.h>
#include "rpt_interface_cfg.h"


static stx_custom_dir_entry_t dir_entries[20];
static int32_t                file_handle = -1;
extern uint32_t open_file_size;

static inline void STX_rptsend(uint8_t cc, uint8_t type, int32_t status,  size_t len, const void *data)
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

    uint16_t          cstatus = 0;
    STX_GET_ALLPRAM_t rxdata1;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata1, sizeof(rxdata1));
    }

    status  = ESUP_INSIG;
    command = CONFIG_CC_GET;
    type    = CONFIG_TP_MODULATORDTIFC;
    cstatus = 0;
    STX_GET_ModulationInterface_t rxdata2;

    ret_status = ESUP(status, command, type, NULL, 0, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata2, sizeof(rxdata2));
    }

    status  = ESUP_INSIG;
    command = CONFIG_CC_GET;
    type    = STATUS_TP_SIMPLE_REPORT;
    cstatus = 0;
    STX_GET_Report_t rxdata3;

    ret_status = ESUP(status, command, type, NULL, 0, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata3, sizeof(rxdata3));
        
    }

    STX_Data.HkTlm.Payload.ALLPRAM        = rxdata1;
    STX_Data.HkTlm.Payload.Modulator      = rxdata2;
    STX_Data.HkTlm.Payload.SystemState    = rxdata3.SystemState;
    STX_Data.HkTlm.Payload.StatusFlags    = rxdata3.StatusFlags;
    STX_Data.HkTlm.Payload.cputemperature = rxdata3.cputemperature;

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
    uint16_t type    = STATUS_TP_SIMPLE_REPORT;
    uint16_t cstatus = 0;
    STX_GET_Report_t rxdata3;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata3, sizeof(rxdata3));
        
    }

    STX_Data.BCNTlm.Payload.SystemState    = rxdata3.SystemState;
    STX_Data.BCNTlm.Payload.StatusFlags    = rxdata3.StatusFlags;

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(STX_Data.BCNTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(STX_Data.BCNTlm.TelemetryHeader), true);
}

CFE_Status_t STX_NoopCmd(const STX_NoopCmd_t *Msg)
{
    CFE_EVS_SendEvent(STX_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "SAMPLE: NOOP command %s", STX_VERSION);
    uint16_t txdata[2] = {STX_Data.CmdCounter, STX_Data.ErrCounter};
    STX_rptsend(STX_NOOP_CC, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(txdata), &txdata);

    return CFE_SUCCESS;
}

CFE_Status_t STX_ResetCountersCmd(const STX_ResetCountersCmd_t *Msg)
{
    
    STX_Data.CmdCounter = 0;
    STX_Data.ErrCounter = 0;

    CFE_EVS_SendEvent(STX_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "SAMPLE: RESET command");
    uint16_t txdata[2] = {STX_Data.CmdCounter, STX_Data.ErrCounter};
    STX_rptsend(STX_RESET_COUNTERS_CC, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(txdata), &txdata);
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

    uint16_t cstatus = 0; // command status 받을 예정
    uint8_t  rxdata  = 0; // data 받을 예정

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, &cstatus, 0);
    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_SET_SYMBOLRATE, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_SET_SYMBOLRATE, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SET_SYMBOLRATE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_SYMBOLRATE,
                          (unsigned)cmd->Payload.data, ret_status);
        STX_rptsend(STX_SET_SYMBOLRATE, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_SYMBOLRATE,
                          (unsigned)cmd->Payload.data, cstatus);
        STX_rptsend(STX_SET_SYMBOLRATE, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
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

    uint16_t cstatus = 0; // command status 받을 예정
    uint8_t  rxdata;      // data 받을 예정

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, &cstatus, 0);
    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_SET_TRANSMITPW, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_SET_TRANSMITPW, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SET_TRANSMITPW_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_TRANSMITPW,
                          (unsigned)cmd->Payload.data, ret_status);
        STX_rptsend(STX_SET_TRANSMITPW, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_TRANSMITPW,
                          (unsigned)cmd->Payload.data, cstatus);
        STX_rptsend(STX_SET_TRANSMITPW, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
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

    uint16_t cstatus = 0; // command status 받을 예정
    uint8_t  rxdata;      // data 받을 예정

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, &cstatus, 0);
    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_SET_CENTERFREQ, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_SET_CENTERFREQ, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SET_CENTERFREQ_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_CENTERFREQ,
                          (unsigned)cmd->Payload.data, ret_status);
        STX_rptsend(STX_SET_CENTERFREQ, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_CENTERFREQ,
                          (unsigned)cmd->Payload.data, cstatus);
        STX_rptsend(STX_SET_CENTERFREQ, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
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

    uint16_t cstatus = 0; // command status 받을 예정
    uint8_t  rxdata;      // data 받을 예정

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_SET_MODCOD, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_SET_MODCOD, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SET_MODCOD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_MODCOD, (unsigned)cmd->Payload.data,
                          ret_status);
        STX_rptsend(STX_SET_MODCOD, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_MODCOD, (unsigned)cmd->Payload.data,
                          cstatus);
        STX_rptsend(STX_SET_MODCOD, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
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

    uint16_t cstatus = 0; // command status 받을 예정
    uint8_t  rxdata;      // data 받을 예정

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, &cstatus, 0);
    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_SET_ROLLOFF, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_SET_ROLLOFF, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SET_ROLLOFF_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_ROLLOFF, (unsigned)cmd->Payload.data,
                          ret_status);
        STX_rptsend(STX_SET_ROLLOFF, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_ROLLOFF, (unsigned)cmd->Payload.data,
                          cstatus);
        STX_rptsend(STX_SET_ROLLOFF, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
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

    uint16_t cstatus = 0; // command status 받을 예정
    uint8_t  rxdata;      // data 받을 예정

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, &cstatus, 0);
    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_SET_PILOTSIG, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_SET_PILOTSIG, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SET_PILOTSIG_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_PILOTSIG, (unsigned)cmd->Payload.data,
                          ret_status);
        STX_rptsend(STX_SET_PILOTSIG, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_PILOTSIG, (unsigned)cmd->Payload.data,
                          cstatus);
        STX_rptsend(STX_SET_PILOTSIG, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
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

    uint16_t cstatus = 0; // command status 받을 예정
    uint8_t  rxdata;      // data 받을 예정

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_SET_FECFRAMESZ, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_SET_FECFRAMESZ, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SET_FECFRAMESZ_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_FECFRAMESZ,
                          (unsigned)cmd->Payload.data, ret_status);
        STX_rptsend(STX_SET_FECFRAMESZ, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_FECFRAMESZ,
                          (unsigned)cmd->Payload.data, cstatus);
        STX_rptsend(STX_SET_FECFRAMESZ, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
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

    uint16_t cstatus = 0; // command status 받을 예정
    uint8_t  rxdata;      // data 받을 예정

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_SET_PRETX_DELAY, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_SET_PRETX_DELAY, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SET_PRETX_DELAY_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_PRETX_DELAY,
                          (unsigned)cmd->Payload.data, ret_status);
        STX_rptsend(STX_SET_PRETX_DELAY, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_PRETX_DELAY,
                          (unsigned)cmd->Payload.data, cstatus);
        STX_rptsend(STX_SET_PRETX_DELAY, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
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

    uint16_t cstatus = 0; // command status 받을 예정
    uint8_t  rxdata;      // data 받을 예정

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_SET_ALL_PRAMETERS, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_SET_ALL_PRAMETERS, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SET_ALL_PRAMETERS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_SET_ALL_PRAMETERS, ret_status);
        STX_rptsend(STX_SET_ALL_PRAMETERS, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_SET_ALL_PRAMETERS, cstatus);
        STX_rptsend(STX_SET_ALL_PRAMETERS, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
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

    uint16_t cstatus = 0; // command status 받을 예정
    uint8_t  rxdata;      // data 받을 예정

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_SET_RS485BAUD, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_SET_RS485BAUD, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SET_RS485BAUD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_RS485BAUD,
                          (unsigned)cmd->Payload.data, ret_status);
        STX_rptsend(STX_SET_RS485BAUD, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed (arg=%u), Status=%" PRId32, STX_SET_RS485BAUD,
                          (unsigned)cmd->Payload.data, cstatus);
        STX_rptsend(STX_SET_RS485BAUD, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
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

    uint16_t cstatus = 0; // command status 받을 예정
    uint8_t  rxdata;      // data 받을 예정

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_SET_MODULATOR_DATA_INTERFACE, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_SET_MODULATOR_DATA_INTERFACE, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SET_MODULATOR_DATA_INTERFACE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed Status=%" PRId32, STX_SET_MODULATOR_DATA_INTERFACE, ret_status);
        STX_rptsend(STX_SET_MODULATOR_DATA_INTERFACE, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_SET_MODULATOR_DATA_INTERFACE, cstatus);
        STX_rptsend(STX_SET_MODULATOR_DATA_INTERFACE, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
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

    uint16_t  cstatus = 0; // command status 받을 예정
    STX_DIR_t rxdata  = {
        0,
    }; // data 받을 예정

    int32_t ret_status = ESUP(status, command, type, NULL, 0, &cstatus, 0);
    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {

        ret_status = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, &rxdata, sizeof(rxdata));
        if (ret_status == DEVICE_SUCCESS && rxdata.commad_status == 0)
        {

            const uint8_t *p   = rxdata.listfile;
            const uint8_t *end = p + sizeof(rxdata.listfile); // 버퍼의 끝

            for (uint16_t i = 0; i < rxdata.file_cnt; i++)
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
                OS_printf("[%u] %s (%u bytes)\n", (uint16_t)(i + 1), dir_entries[i].path, (uint16_t)dir_entries[i].size);
            }
            STX_rptsend(STX_FILESYS_CC_DIR, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        }
        else
        {
            CFE_EVS_SendEvent(STX_FILESYS_DIR_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                              STX_FILESYS_CC_DIR, ret_status);
            STX_rptsend(STX_FILESYS_CC_DIR, STK_rpt_tpye_REULT, ret_status, 0, NULL);
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_FILESYS_DIR_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_FILESYS_CC_DIR, ret_status);
        STX_rptsend(STX_FILESYS_CC_DIR, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_FILESYS_CC_DIR, cstatus);
        STX_rptsend(STX_FILESYS_CC_DIR, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
    }

   
}

void STX_DIRNEXTCmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = FILESYS_CC_DIRNEXT;
    uint16_t type    = FILESYS_TP_NA;

    int dir_entry_count = 0;

    uint16_t  cstatus = 0; // command status 받을 예정
    STX_DIR_t rxdata  = {
        0,
    }; // data 받을 예정

    int32_t ret_status = ESUP(status, command, type, NULL, 0, &cstatus, 0);
    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {

        ret_status = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, &rxdata, sizeof(rxdata));
        if (ret_status == DEVICE_SUCCESS && rxdata.commad_status == 0)
        {

            const uint8_t *p   = rxdata.listfile;
            const uint8_t *end = p + sizeof(rxdata.listfile); // 버퍼의 끝

            for (uint16_t i = 0; i < rxdata.file_cnt; i++)
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
                OS_printf("[%u] %s (%u bytes)\n", (uint16_t)(i + 1), dir_entries[i].path, (uint16_t)dir_entries[i].size);
            }
            STX_rptsend(STX_FILESYS_CC_DIRNEXT, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        }
        else
        {
            CFE_EVS_SendEvent(STX_FILESYS_DIR_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                              STX_FILESYS_CC_DIR, ret_status);
            STX_rptsend(STX_FILESYS_CC_DIRNEXT, STK_rpt_tpye_REULT, ret_status, 0, NULL);
        }
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_FILESYS_DIR_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_FILESYS_CC_DIR, ret_status);
        STX_rptsend(STX_FILESYS_CC_DIRNEXT, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_FILESYS_CC_DIR, cstatus);
        STX_rptsend(STX_FILESYS_CC_DIRNEXT, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
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

    uint16_t cstatus = 0; // command status 받을 예정
    uint8_t  rxdata  = 0; // data 받을 예정

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, &cstatus, 0);
    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_FILESYS_CC_DELFILE, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_FILESYS_CC_DELFILE, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_FILESYS_DELFILE_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_FILESYS_CC_DELFILE, ret_status);
        STX_rptsend(STX_FILESYS_CC_DELFILE, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_FILESYS_CC_DELFILE, cstatus);
        STX_rptsend(STX_FILESYS_CC_DELFILE, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
    }

   
}

void STX_DELALLFILECmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = FILESYS_CC_DELALLFILE;
    uint16_t type    = FILESYS_TP_NA;

    uint16_t cstatus = 0; // command status 받을 예정
    uint8_t  rxdata  = 0; // data 받을 예정

    int32_t ret_status = ESUP(status, command, type, NULL, 0, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_FILESYS_CC_DELALLFILE, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_FILESYS_CC_DELALLFILE, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_FILESYS_DELALLFILE_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_FILESYS_CC_DELALLFILE, ret_status);
        STX_rptsend(STX_FILESYS_CC_DELALLFILE, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_FILESYS_CC_DELALLFILE, cstatus);
        STX_rptsend(STX_FILESYS_CC_DELALLFILE, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
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

    uint16_t          cstatus = 0; // command status 받을 예정
    STX_FILE_CREATE_t rxdata  = {
        0,
    }; // data 받을 예정

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_FILESYS_CC_CREATEFILE, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_FILESYS_CC_CREATEFILE, STK_rpt_tpye_REULT, ret_status, 0, NULL);
        file_handle = rxdata.file_handle;
        OS_printf("Creating file: %s, size: %u, txlen: %u\n", cmd->Payload.filename_max, cmd->Payload.file_size,
                 txlength);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_FILESYS_CREATEFILE_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_FILESYS_CC_CREATEFILE, ret_status);
        STX_rptsend(STX_FILESYS_CC_CREATEFILE, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_FILESYS_CC_CREATEFILE, cstatus);
        STX_rptsend(STX_FILESYS_CC_CREATEFILE, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
    }

   

    OS_printf("file_handle: %u\n", rxdata.file_handle);
}

#define WRITE_OPEN_ERROR -1001
#define WRITE_READ_ERROR -1002

/* 실험 필요*/
void STX_WRITEFILECmd(const STX_WRITEFILE_t *Msg)
{
    
    const STX_WRITEFILE_t *cmd = (const STX_WRITEFILE_t *)Msg;

    FILE    *fp;
    size_t   remaining;
    uint32_t packet_number = 0;

    OS_printf("STX_WRITEFILECmd called: filename=%s, size=%u, offset=%u, interpacket_delay=%u\n", cmd->Payload.filename,
             cmd->Payload.size, cmd->Payload.offset, cmd->Payload.interpacket_delay);

    struct __attribute__((__packed__))
    {
        int      ret;
        uint32_t packet_number;
        uint8_t  cstatus;
    } write_status = {0, 0, 0};

    int16_t                      status  = ESUP_INSIG;
    uint16_t                     command = FILESYS_CC_WRITEFILE;
    uint16_t                     type    = FILESYS_TP_NA;
    uint16_t                     cstatus = 0; // command status 받을 예정
    uint8_t                      rxdata  = 0;
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
    }
    else
    {
        remaining = cmd->Payload.size;
        fseek(fp, cmd->Payload.offset, SEEK_SET);
    }

    writePacket.file_handle = file_handle;
    OS_printf("write file ready: size=%zu, offset=%u, handle=%u\n", remaining, cmd->Payload.offset, file_handle);

    while (remaining > 0)
    {
        size_t bytes_to_read = (remaining > ESUP_MAX_WRITE_LENGTH) ? ESUP_MAX_WRITE_LENGTH : remaining;
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
        int32_t  ret_status = ESUP(status, command, type, txdata, txlength, &cstatus, 0);

        int32_t ret_status2 = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, &rxdata, sizeof(rxdata));

        OS_printf("---------------------------------------------------\n");

        if (ret_status2 != CFE_SUCCESS)
        {
            write_status.ret           = ret_status;
            write_status.packet_number = packet_number;
            STX_rptsend(STX_FILESYS_CC_WRITEFILE, STK_rpt_tpye_CFE_ERROR, ret_status, sizeof(write_status), &write_status);
            return;
        }
        else if (cstatus != ESUP_ACK)
        {
            write_status.ret           = ret_status;
            write_status.packet_number = packet_number;
            write_status.cstatus       = cstatus;
            STX_rptsend(STX_FILESYS_CC_WRITEFILE, STK_rpt_tpye_ACK_ERROR, cstatus, sizeof(write_status), &write_status);
            return;
        }
        remaining -= bytes_read;
        packet_number++;

        if (cmd->Payload.interpacket_delay)
            usleep(cmd->Payload.interpacket_delay * 1000);
    }
    fclose(fp);

    write_status.ret           = 0;
    write_status.packet_number = packet_number;
    STX_rptsend(STX_FILESYS_CC_WRITEFILE, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(write_status), &write_status);
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

    uint16_t        cstatus = 0; // command status 받을 예정
    STX_FILE_OPEN_t rxdata  = {
        0,
    };

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, &cstatus, 0);
    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status  = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, &rxdata, sizeof(rxdata));
        if (ret_status == DEVICE_SUCCESS)
        {
            file_handle = rxdata.file_handle;
            STX_rptsend(STX_FILESYS_CC_OPENFILE, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        }
        else
            STX_rptsend(STX_FILESYS_CC_OPENFILE, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        STX_rptsend(STX_FILESYS_CC_OPENFILE, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        STX_rptsend(STX_FILESYS_CC_OPENFILE, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
    }
}

void STX_READFILECmd(const STX_READFILE_t *Msg)
{
    

    void    *txdata   = (void *)&file_handle;
    uint16_t txlength = sizeof(file_handle);

    int16_t  status  = ESUP_INSIG;
    uint16_t command = FILESYS_CC_READFILE;
    uint16_t type    = FILESYS_TP_NA;

    uint16_t        cstatus = 0; // command status 받을 예정
    STX_FILE_READ_t rxdata  = {
        0,
    }; // data 받을 예정

    uint16_t STX_timeout = 30;
    long t1 = latch_ms();
    
    int32_t ret_status;
    
    while(open_file_size && latch_ms() - t1 < STX_timeout){
        
        ret_status = ESUP(status, command, type, txdata, txlength, &cstatus, 0);
        
        if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
        {
            ret_status = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, &rxdata, sizeof(rxdata));
            
            OS_printf("read byte : %d \n", (int)rxdata.Packet_length);
            
            open_file_size -= (uint32_t)rxdata.Packet_length;

            OS_printf("remain byte : %d \n", open_file_size);
            
            if (ret_status == DEVICE_SUCCESS && rxdata.commad_status == 0)
            {
                STX_rptsend(STX_FILESYS_CC_READFILE, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);

                uint8_t file_retdata[rxdata.Packet_length];
                memcpy(file_retdata, rxdata.file_data, rxdata.Packet_length);

                OS_printf("file_retdata: %s\n", file_retdata);
                
                t1 = latch_ms();
            }
            else
            {
                CFE_EVS_SendEvent(STX_FILESYS_READFILE_ERR_EID, CFE_EVS_EventType_ERROR,
                                "STX : CC=%u failed Status=%" PRId32, STX_FILESYS_CC_READFILE, ret_status);
                STX_rptsend(STX_FILESYS_CC_READFILE, STK_rpt_tpye_REULT, ret_status, 0, NULL);
            }
        }
        else if (ret_status != CFE_SUCCESS)
        {
            OS_printf("ReadFile ERR \n");
            CFE_EVS_SendEvent(STX_FILESYS_READFILE_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                            STX_FILESYS_CC_READFILE, ret_status);
            STX_rptsend(STX_FILESYS_CC_READFILE, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
        }
        else if (cstatus != ESUP_ACK)
        {
            OS_printf("Command status ERR\n");
            CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                            STX_FILESYS_CC_READFILE, cstatus);
            STX_rptsend(STX_FILESYS_CC_READFILE, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
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
    
    uint16_t cstatus = 0;
    uint8_t rxdata = 0;

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, &cstatus, 0);
    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        // 확인필요
        //*************************************************************************************************************************
        uint16_t request_data = 0x0049;
        ret_status = ESUP(status, GETRES_CC_GETRES, command, (void *)&request_data, sizeof(request_data), &rxdata, sizeof(rxdata));
        //************************************************************************************************************************* 
        //ret_status = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, &rxdata, sizeof(rxdata));

        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_FILESYS_CC_SENDFILE, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_FILESYS_CC_SENDFILE, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        STX_rptsend(STX_FILESYS_CC_SENDFILE, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        STX_rptsend(STX_FILESYS_CC_SENDFILE, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
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

    uint16_t cstatus = 0; // command status 받을 예정
    uint8_t  rxdata  = 0; // data 받을 예정

    int32_t ret_status = ESUP(status, command, type, txdata, txlength, &cstatus, 0);
    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        // 확인필요
        //*************************************************************************************************************************
        uint16_t request_data = 0x0049;
        ret_status = ESUP(status, GETRES_CC_GETRES, command, (void *)&request_data, sizeof(request_data), &rxdata, sizeof(rxdata));
        //************************************************************************************************************************* 
        //ret_status = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, &rxdata, sizeof(rxdata));

        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_FILESYS_CC_SENDFILE, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_FILESYS_CC_SENDFILE, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        STX_rptsend(STX_FILESYS_CC_SENDFILE, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        STX_rptsend(STX_FILESYS_CC_SENDFILE, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
    }
}

/*  mode command    */

void STX_SYSCONF_CC_TRANSMITMODECmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = SYSCONF_CC_TRANSMITMODE;
    uint16_t type    = SYSCONF_TP_NA;

    uint16_t cstatus = 0; // command status 받을 예정
    uint8_t  rxdata;      // data 받을 예정

    int32_t ret_status = ESUP(status, command, type, NULL, 0, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        sleep(50);
        ret_status = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_SYSCONF_CC_TRANSMITMODE, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_SYSCONF_CC_TRANSMITMODE, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SYSCONF_TRANSMITMODE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed Status=%" PRId32, STX_SYSCONF_CC_TRANSMITMODE, ret_status);
        STX_rptsend(STX_SYSCONF_CC_TRANSMITMODE, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_SYSCONF_CC_TRANSMITMODE, cstatus);
        STX_rptsend(STX_SYSCONF_CC_TRANSMITMODE, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
    }

   
}

void STX_SYSCONF_CC_IDLEMODECmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = SYSCONF_CC_IDLEMODE;
    uint16_t type    = SYSCONF_TP_NA;

    uint16_t cstatus = 0; // command status 받을 예정
    uint8_t  rxdata;      // data 받을 예정

    int32_t ret_status = ESUP(status, command, type, NULL, 0, &cstatus, 0);
    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_SYSCONF_CC_IDLEMODE, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_SYSCONF_CC_IDLEMODE, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SYSCONF_IDLEMODE_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_SYSCONF_CC_IDLEMODE, ret_status);
        STX_rptsend(STX_SYSCONF_CC_IDLEMODE, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_SYSCONF_CC_IDLEMODE, cstatus);
        STX_rptsend(STX_SYSCONF_CC_IDLEMODE, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
    }

   
}

void STX_SYSCONF_CC_SAFESHUTDOWNCmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = SYSCONF_CC_SAFESHUTDOWN;
    uint16_t type    = SYSCONF_TP_NA;

    uint16_t cstatus = 0; // command status 받을 예정
    uint8_t  rxdata;      // data 받을 예정

    int32_t ret_status = ESUP(status, command, type, NULL, 0, &cstatus, 0);
    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, NULL, 0, &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_SYSCONF_CC_SAFESHUTDOWN, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_SYSCONF_CC_SAFESHUTDOWN, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_SYSCONF_SAFESHUTDOWN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed Status=%" PRId32, STX_SYSCONF_CC_SAFESHUTDOWN, ret_status);
        STX_rptsend(STX_SYSCONF_CC_SAFESHUTDOWN, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_SYSCONF_CC_SAFESHUTDOWN, cstatus);
        STX_rptsend(STX_SYSCONF_CC_SAFESHUTDOWN, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
    }

   
}

/* Get command */

void STX_GET_SYMBOL_RATECmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = CONFIG_TP_SYMBOLRATE;

    uint16_t     cstatus = 0; // command status 받을 예정
    STX_GET_U8_t rxdata;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_GET_SYMBOL_RATE, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_GET_SYMBOL_RATE, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_GET_SYMBOL_RATE_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_SYMBOL_RATE, ret_status);
        STX_rptsend(STX_GET_SYMBOL_RATE, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_SYMBOL_RATE, cstatus);
        STX_rptsend(STX_GET_SYMBOL_RATE, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
    }

   
}

void STX_GET_TX_POWERCmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = CONFIG_TP_TRANSMITPW;

    uint16_t     cstatus = 0;
    STX_GET_U8_t rxdata;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_GET_TX_POWER, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_GET_TX_POWER, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_GET_TX_POWER_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_TX_POWER, ret_status);
        STX_rptsend(STX_GET_TX_POWER, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_TX_POWER, cstatus);
        STX_rptsend(STX_GET_TX_POWER, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
    }

   
}

void STX_GET_CENTER_FREQCmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = CONFIG_TP_CENTERFREQ;

    uint16_t        cstatus = 0;
    STX_GET_FLOAT_t rxdata;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_GET_CENTER_FREQ, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_GET_CENTER_FREQ, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_GET_CENTER_FREQ_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_CENTER_FREQ, ret_status);
        STX_rptsend(STX_GET_CENTER_FREQ, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_CENTER_FREQ, cstatus);
        STX_rptsend(STX_GET_CENTER_FREQ, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
    }

   
}

void STX_GET_MODCODCmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = CONFIG_TP_MODCOD;

    uint16_t     cstatus = 0;
    STX_GET_U8_t rxdata;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_GET_MODCOD, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_GET_MODCOD, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_GET_MODCOD_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_MODCOD, ret_status);
        STX_rptsend(STX_GET_MODCOD, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_MODCOD, cstatus);
        STX_rptsend(STX_GET_MODCOD, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
    }

   
}

void STX_GET_ROLL_OFFCmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = CONFIG_TP_ROLLOFF;

    uint16_t     cstatus = 0;
    STX_GET_U8_t rxdata;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_GET_ROLL_OFF, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_GET_ROLL_OFF, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_GET_ROLL_OFF_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_ROLL_OFF, ret_status);
        STX_rptsend(STX_GET_ROLL_OFF, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_ROLL_OFF, cstatus);
        STX_rptsend(STX_GET_ROLL_OFF, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
    }

   
}

void STX_GET_PILOT_SIGNALCmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = CONFIG_TP_PILOTSIG;

    uint16_t     cstatus = 0;
    STX_GET_U8_t rxdata;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_GET_PILOT_SIGNAL, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_GET_PILOT_SIGNAL, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_GET_PILOT_SIGNAL_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_PILOT_SIGNAL, ret_status);
        STX_rptsend(STX_GET_PILOT_SIGNAL, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_PILOT_SIGNAL, cstatus);
        STX_rptsend(STX_GET_PILOT_SIGNAL, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
    }

   
}

void STX_GET_FEC_FRAME_SIZECmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = CONFIG_TP_FECFRAMESZ;

    uint16_t     cstatus = 0;
    STX_GET_U8_t rxdata;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_GET_FEC_FRAME_SIZE, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_GET_FEC_FRAME_SIZE, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_GET_FEC_FRAME_SIZE_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_FEC_FRAME_SIZE, ret_status);
        STX_rptsend(STX_GET_FEC_FRAME_SIZE, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_FEC_FRAME_SIZE, cstatus);
        STX_rptsend(STX_GET_FEC_FRAME_SIZE, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
    }

   
}

void STX_GET_PRETX_DELAYCmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = CONFIG_TP_PRETXSTUFFDEL;

    uint16_t     cstatus = 0;
    STX_GET_U8_t rxdata;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_GET_PRETX_DELAY, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_GET_PRETX_DELAY, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_GET_PRETX_DELAY_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_PRETX_DELAY, ret_status);
        STX_rptsend(STX_GET_PRETX_DELAY, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_PRETX_DELAY, cstatus);
        STX_rptsend(STX_GET_PRETX_DELAY, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
    }

   
}

void STX_GET_ALL_PRAMETERSCmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = CONFIG_TP_ALLPARAM;

    uint16_t          cstatus = 0;
    STX_GET_ALLPRAM_t rxdata;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_GET_ALL_PRAMETERS, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_GET_ALL_PRAMETERS, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_GET_ALL_PRAMETERS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_ALL_PRAMETERS, ret_status);
        STX_rptsend(STX_GET_ALL_PRAMETERS, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_ALL_PRAMETERS, cstatus);
        STX_rptsend(STX_GET_ALL_PRAMETERS, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
    }

   
}

void STX_GET_REPORTCmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = STATUS_TP_SIMPLE_REPORT;

    uint16_t         cstatus = 0;
    STX_GET_Report_t rxdata;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_GET_REPORT, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_GET_REPORT, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_GET_REPORT_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_REPORT, ret_status);
        STX_rptsend(STX_GET_REPORT, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_REPORT, cstatus);
        STX_rptsend(STX_GET_REPORT, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
    }

   
}

void STX_GET_MODULATOR_DATA_INTERFACECmd(void)
{
    
    int16_t  status  = ESUP_INSIG;
    uint16_t command = CONFIG_CC_GET;
    uint16_t type    = CONFIG_TP_MODULATORDTIFC;

    uint16_t                      cstatus = 0;
    STX_GET_ModulationInterface_t rxdata;

    int32_t ret_status = ESUP(status, command, type, NULL, 0, &cstatus, 0);

    if (cstatus == ESUP_ACK && ret_status == CFE_SUCCESS)
    {
        ret_status = ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type), &rxdata, sizeof(rxdata));
        
        if (ret_status == DEVICE_SUCCESS)
            STX_rptsend(STX_GET_MODULATOR_DATA_INTERFACE, STK_rpt_tpye_REULT, DEVICE_SUCCESS, sizeof(rxdata), &rxdata);
        else
            STX_rptsend(STX_GET_MODULATOR_DATA_INTERFACE, STK_rpt_tpye_REULT, ret_status, 0, NULL);
    }
    else if (ret_status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STX_GET_MODULATOR_DATA_INTERFACE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STX : CC=%u failed Status=%" PRId32, STX_GET_MODULATOR_DATA_INTERFACE, ret_status);
        STX_rptsend(STX_GET_MODULATOR_DATA_INTERFACE, STK_rpt_tpye_CFE_ERROR, ret_status, 0, NULL);
    }
    else if (cstatus != ESUP_ACK)
    {
        CFE_EVS_SendEvent(STX_COMMAND_STATUS_ERR_EID, CFE_EVS_EventType_ERROR, "STX : CC=%u failed Status=%" PRId32,
                          STX_GET_MODULATOR_DATA_INTERFACE, cstatus);
        STX_rptsend(STX_GET_MODULATOR_DATA_INTERFACE, STK_rpt_tpye_ACK_ERROR, cstatus, 0, NULL);
    }

   
}
