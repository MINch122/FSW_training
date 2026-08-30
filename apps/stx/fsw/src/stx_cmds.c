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
#include "enduro_stx.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "rpt_interface_cfg.h"

static esup_wire_t wire;
static esup_wire_linux_ctx_t wire_ctx;
static stx_t dev;
static esup_engine_t* engine;
static bool drivers_started;

static uint32_t file_handle = -1;
static uint16_t module_id = STX_DEFAULT_MODULE_ID;

static stx_t* GetDeviceByModuleId(uint16_t module_id)
{
    if (engine != NULL && esup_engine_module_id(engine) != module_id)
    {
        esup_engine_set_module_id(engine, module_id);
    }

    return &dev;
}

void STX_SendReportForMid(uint16 MsgId, uint8 CommandCode, uint8 ReturnType, int32 ReturnCode,
                          size_t DataLength, const void *Data)
{
    memset(&STX_Data.RptPkt, 0, sizeof(STX_Data.RptPkt));
    CFE_MSG_Init(CFE_MSG_PTR(STX_Data.RptPkt.TelemetryHeader), CFE_SB_ValueToMsgId(STX_APP_RPT_TLM_MID),
                 sizeof(STX_Data.RptPkt));

    STX_Data.RptPkt.Report.MsgID       = MsgId;
    STX_Data.RptPkt.Report.CommandCode = CommandCode;
    STX_Data.RptPkt.Report.ReturnType  = ReturnType;
    STX_Data.RptPkt.Report.ReturnCode  = ReturnCode;

    size_t datasize = DataLength > RPT_RET_VALUE_BUF_SIZE ? RPT_RET_VALUE_BUF_SIZE : DataLength;
    STX_Data.RptPkt.Report.ReturnDataSize = (uint16_t)datasize;
    if (datasize > 0 && Data != NULL)
    {
        memcpy(STX_Data.RptPkt.Report.ReturnValue, Data, datasize);
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(STX_Data.RptPkt.TelemetryHeader));
    (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(STX_Data.RptPkt.TelemetryHeader), true);
}

static void STX_rptsend(uint8 CommandCode, uint8 ReturnType, int32 ReturnCode, size_t DataLength,
                        const void *Data)
{
    STX_SendReportForMid(STX_CMD_MID, CommandCode, ReturnType, ReturnCode, DataLength, Data);
}

static uint8 STX_GetEsupReportType(esup_ret_t Ret)
{
    if (Ret == ESUP_OK)
    {
        return RPT_RETTYPE_SUCCESS;
    }

    if (ESUP_RET_EXEC_STATUS(Ret) != 0)
    {
        return RPT_RETTYPE_HW;
    }

    if (ESUP_RET_DEVICE_LOCAL(Ret) != 0)
    {
        return RPT_RETTYPE_APP;
    }

    return RPT_RETTYPE_LIB;
}

static void STX_SendEsupReport(uint8 CommandCode, esup_ret_t Ret, size_t DataLength, const void *Data)
{
    STX_rptsend(CommandCode, STX_GetEsupReportType(Ret), Ret, DataLength, Data);
}

static bool STX_CopyFilename(char *Destination, size_t DestinationSize, const char *Source, int8 SourceLength,
                             uint8 CommandCode)
{
    if (SourceLength <= 0 || (size_t)SourceLength >= DestinationSize)
    {
        esup_ret_t ret = STX_ERR_RANGE;
        STX_SendEsupReport(CommandCode, ret, sizeof(SourceLength), &SourceLength);
        return false;
    }

    memcpy(Destination, Source, (size_t)SourceLength);
    Destination[SourceLength] = '\0';
    return true;
}


void STX_SendHkCmd(void)
{
    esup_ret_t ret = 0;

    stx_params_t params;
    stx_mod_data_iface_t iface;
    stx_report_t report;

    ret = stx_get_params(GetDeviceByModuleId(module_id), &params);
    if (ret == ESUP_OK){
        STX_Data.HkTlm.Payload.ALLPRAM.symbol_rate = params.symbol_rate;
        STX_Data.HkTlm.Payload.ALLPRAM.transmit_power = params.tx_power_dbm;
        STX_Data.HkTlm.Payload.ALLPRAM.modcod = params.modcod;
        STX_Data.HkTlm.Payload.ALLPRAM.roll_off = params.roll_off;
        STX_Data.HkTlm.Payload.ALLPRAM.pilot_signal = params.pilot;
        STX_Data.HkTlm.Payload.ALLPRAM.fec_frame_size = params.fec_frame_short;
        STX_Data.HkTlm.Payload.ALLPRAM.pretransmission_delay = params.pretx_delay_ms;
        STX_Data.HkTlm.Payload.ALLPRAM.center_frequency = params.center_freq_mhz;
    }
    OS_TaskDelay(3);

    ret = stx_get_mod_data_iface(GetDeviceByModuleId(module_id), &iface);
    if (ret == ESUP_OK){
        STX_Data.HkTlm.Payload.Modulator.modulator_interface_type = iface.interface_type;
        STX_Data.HkTlm.Payload.Modulator.lvds_io_type = iface.lvds_io_type;
    }
    OS_TaskDelay(3);
    ret = stx_get_report(GetDeviceByModuleId(module_id), &report);
    if (ret == ESUP_OK){
        STX_Data.HkTlm.Payload.SystemState = report.state;
        STX_Data.HkTlm.Payload.StatusFlags = report.flags;
        STX_Data.HkTlm.Payload.cputemperature = report.cpu_temp_c;
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(STX_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(STX_Data.HkTlm.TelemetryHeader), true);
}

CFE_Status_t STX_NoopCmd(const STX_NoopCmd_t *Msg)
{
    static const char NoopReport[] = "STX NOOP CMD: YOSI IN SPACE";

    CFE_EVS_SendEvent(STX_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "SAMPLE: NOOP command %s", STX_VERSION);
    STX_rptsend(STX_NOOP_CC, RPT_RETTYPE_SUCCESS, DEVICE_SUCCESS, sizeof(NoopReport), NoopReport);

    return CFE_SUCCESS;
}

CFE_Status_t STX_ResetCountersCmd(const STX_ResetCountersCmd_t *Msg)
{
    
    STX_Data.CmdCounter = 0;
    STX_Data.ErrCounter = 0;

    CFE_EVS_SendEvent(STX_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "SAMPLE: RESET command");
    uint16_t txdata[2] = {STX_Data.CmdCounter, STX_Data.ErrCounter};
    STX_rptsend(STX_RESET_COUNTERS_CC, RPT_RETTYPE_SUCCESS, DEVICE_SUCCESS, sizeof(txdata), &txdata);
    return CFE_SUCCESS;
}

void STX_InitializeCmd(void)
{
    esup_ret_t ret = ESUP_OK;
    if (wire.ops == NULL){
        ret = esup_wire_linux_open(&wire_ctx, &wire, "/dev/ttyS1", 250000, ESUP_WIRE_LINUX_RS485);
        if (ret != ESUP_OK)
        {
            STX_SendEsupReport(STX_INITIALIZE_CC, ret, sizeof(ret), &ret);
            OS_printf("wire initialization failed \n");
            return;
        }
    }

    if (drivers_started)
    {
        STX_rptsend(STX_INITIALIZE_CC, RPT_RETTYPE_SUCCESS, DEVICE_SUCCESS, sizeof(ret), &ret);
        return;
    }

    if (engine == NULL)
    {
        engine = esup_engine_create(&wire, module_id);
    }

    if (dev.engine == NULL)
    {
        ret = stx_init(&dev, engine);
    }

    if (ret != ESUP_OK)
    {
        STX_SendEsupReport(STX_INITIALIZE_CC, ret, sizeof(ret), &ret);
        return;
    }

    ret = esup_start(engine);

    if (ret != ESUP_OK)
    {
        if (ret == ESUP_SESSION_IO_ERROR) OS_printf("Fail to create STX driver thread\n");
        STX_SendEsupReport(STX_INITIALIZE_CC, ret, sizeof(ret), &ret);
        return;
    }

    drivers_started = true;
    OS_printf("STX driver started for module id %u\n", module_id);
    STX_rptsend(STX_INITIALIZE_CC, RPT_RETTYPE_SUCCESS, DEVICE_SUCCESS, sizeof(ret), &ret);
    return;
}

void STX_SetModuleIdCmd(const STX_SetModuleIdCmd_t *Msg)
{
    module_id = Msg->Payload.ModuleId;
    if (engine != NULL)
    {
        esup_engine_set_module_id(engine, module_id);
    }

    OS_printf("module id set to %u\n", module_id);
    STX_rptsend(STX_SET_MODULE_ID_CC, RPT_RETTYPE_SUCCESS, DEVICE_SUCCESS, sizeof(module_id), &module_id);
    return;
}

void STX_GET_FileHandleCmd(const STX_GET_FILEHANDLECmd_t *Msg)
{
    OS_printf("File Handle : 0x%08X \n", file_handle);
    STX_rptsend(STX_GET_FILEHANDLE_CC, RPT_RETTYPE_SUCCESS, DEVICE_SUCCESS, sizeof(file_handle), &file_handle);
    return;
}

/* SET COMMNAD */
void STX_SET_SYMBOLRATECmd(const STX_Set_SYMBOLRAtE_t *cmd)
{
   uint8_t msps = cmd->Payload.data;

   esup_ret_t ret = stx_set_symbol_rate(GetDeviceByModuleId(module_id),msps);
   print_status("Set Sym Rate", ret);
   STX_SendEsupReport(STX_SET_SYMBOLRATE, ret, sizeof(ret), &ret);
   return;
}

void STX_Set_TRANSMITPWCmd(const STX_Set_TRANSMITPW_t *Msg)
{
    uint8_t dbm = Msg->Payload.data;
   
    esup_ret_t ret = stx_set_tx_power(GetDeviceByModuleId(module_id), dbm);
    print_status("Set Tx Pwr", ret);
    STX_SendEsupReport(STX_SET_TRANSMITPW, ret, sizeof(ret), &ret);
    return;
}

void STX_Set_CENTERFREQCmd(const STX_Set_CENTERFREQ_t *Msg)
{
    float mhz = Msg->Payload.data;

    esup_ret_t ret = stx_set_center_freq(GetDeviceByModuleId(module_id), mhz);
    print_status("Set Center Freq", ret);
    STX_SendEsupReport(STX_SET_CENTERFREQ, ret, sizeof(ret), &ret);
    return;
}

void STX_Set_MODCODCmd(const STX_Set_MODCOD_t *Msg)
{
    uint8_t modcod = Msg->Payload.data;

    esup_ret_t ret = stx_set_modcod(GetDeviceByModuleId(module_id), modcod);
    print_status("Set MODCOD", ret);
    STX_SendEsupReport(STX_SET_MODCOD, ret, sizeof(ret), &ret);
    return;
}

void STX_Set_ROLLOFFCmd(const STX_Set_ROLLOFF_t *Msg)
{
    uint8_t roll_off = Msg->Payload.data;

    esup_ret_t ret = stx_set_roll_off(GetDeviceByModuleId(module_id), roll_off);
    print_status("Set RollOff", ret);
    STX_SendEsupReport(STX_SET_ROLLOFF, ret, sizeof(ret), &ret);
    return;
}

void STX_Set_PILOTSIGCmd(const STX_Set_PILOTSIG_t *Msg)
{
    uint8_t pilot = Msg->Payload.data;

    esup_ret_t ret = stx_set_pilot(GetDeviceByModuleId(module_id), pilot);
    print_status("Set Pilot Sig", ret);
    STX_SendEsupReport(STX_SET_PILOTSIG, ret, sizeof(ret), &ret);
    return;
}

void STX_Set_FECFRAMECmd(const STX_Set_FECFRAME_t *Msg)
{
    uint8_t frame = Msg->Payload.data;

    esup_ret_t ret = stx_set_fec_frame(GetDeviceByModuleId(module_id), frame);
    print_status("Set Fec Frame", ret);
    STX_SendEsupReport(STX_SET_FECFRAMESZ, ret, sizeof(ret), &ret);
    return;
}

void STX_Set_PRETX_DELAYCmd(const STX_Set_PRETX_DELAY_t *Msg)
{
    uint16_t ms = Msg->Payload.data;

    esup_ret_t ret = stx_set_pretx_delay(GetDeviceByModuleId(module_id), ms);
    print_status("Set PreTx Delay", ret);
    STX_SendEsupReport(STX_SET_PRETX_DELAY, ret, sizeof(ret), &ret);
    return;
}

void STX_Set_ALLPRAMCmd(const STX_Set_ALLPRAM_t *Msg)
{
    stx_params_t params = {
        .symbol_rate    = Msg->Payload.symbol_rate,
        .tx_power_dbm   = Msg->Payload.transmit_power,
        .modcod         = Msg->Payload.modcod,
        .roll_off       = Msg->Payload.roll_off,
        .pilot          = Msg->Payload.pilot_signal,
        .fec_frame_short = Msg->Payload.fec_frame_size,
        .pretx_delay_ms = Msg->Payload.pretransmission_delay,
        .center_freq_mhz = Msg->Payload.center_frequency
    };
    
    esup_ret_t ret = stx_set_params(GetDeviceByModuleId(module_id), &params);
    print_status("Set ALL Params", ret);
    STX_SendEsupReport(STX_SET_ALL_PRAMETERS, ret, sizeof(ret), &ret);
    return;
}

void STX_Set_RS485Cmd(const STX_Set_RS485_t *Msg)
{
    uint8_t baud = Msg->Payload.data;

    esup_ret_t ret = stx_set_rs485_baud(GetDeviceByModuleId(module_id), baud);
    print_status("Set RS485", ret);
    STX_SendEsupReport(STX_SET_RS485BAUD, ret, sizeof(ret), &ret);
    return;
}

void STX_Set_MODULATION_INTERFACECmd(const STX_Set_MODULATION_INTERFACE_t *Msg)
{
    stx_mod_data_iface_t iface = {
        .interface_type = Msg->Payload.modulator_interface_type,
        .lvds_io_type   = Msg->Payload.lvds_io_type
    };

    esup_ret_t ret = stx_set_mod_data_iface(GetDeviceByModuleId(module_id), &iface);
    print_status("Set Modulation Interface", ret);
    STX_SendEsupReport(STX_SET_MODULATOR_DATA_INTERFACE, ret, sizeof(ret), &ret);
    return;
}

/*  FILE COMMAND */
static void print_dir_listing(const stx_rep_file_dir_t* rep)
{
    /* Size column width, from the widest value present; total in 1 KiB blocks. */
    int      size_width   = 1;
    uint32_t total_blocks = 0;

    for (uint16_t i = 0; i < rep->entries_len; i++) {
        int w = snprintf(NULL, 0, "%u", rep->entries[i].length);

        if (w > size_width)
            size_width = w;

        total_blocks += (rep->entries[i].length + 1023u) / 1024u;
    }

    printf("total %u\n", total_blocks);

    for (uint16_t i = 0; i < rep->entries_len; i++)
        printf("%*u  %s\n", size_width, rep->entries[i].length,
               rep->entries[i].name);

    if (rep->entries_len < rep->count)
        printf("(%u more not shown: entry buffer full)\n",
               (unsigned)(rep->count - rep->entries_len));

    if (rep->more_files)
        printf("(more files available; run 'dirnext')\n");
}

void STX_DIRCmd(void)
{
    stx_dir_entry_t   entries[STX_DIR_ENTRIES_MAX];
    stx_rep_file_dir_t rep = {0};
    rep.entries     = entries;
    rep.entries_cap = STX_DIR_ENTRIES_MAX;

    DIR_REPLY_t reply = {0};

    esup_ret_t ret = stx_file_dir(GetDeviceByModuleId(module_id), &rep);
    print_status("DIR Cmd", ret);
    reply.ret = ret;

    if (ret == ESUP_OK){
        print_dir_listing(&rep);

        for (int i = 0; i < rep.entries_len; i++){
            reply.entries[i] = rep.entries[i];
        }
    }

    STX_SendEsupReport(STX_FILESYS_CC_DIR, ret,
                       sizeof(esup_ret_t) + sizeof(stx_dir_entry_t) * rep.entries_len, &reply);

    return;
}

void STX_DIRNEXTCmd(void)
{
    stx_dir_entry_t   entries[STX_DIR_ENTRIES_MAX];
    stx_rep_file_dir_t rep = {0};
    rep.entries     = entries;
    rep.entries_cap = STX_DIR_ENTRIES_MAX;

    DIR_REPLY_t reply = {0};

    esup_ret_t ret = stx_file_dir_next(GetDeviceByModuleId(module_id), &rep);
    print_status("DIR Next", ret);
    reply.ret = ret;

    if (ret == ESUP_OK){
        print_dir_listing(&rep);
        for (int i = 0 ; i < rep.entries_len ; i++)
        {
            reply.entries[i] = rep.entries[i];
        }
    }

    STX_SendEsupReport(STX_FILESYS_CC_DIRNEXT, ret,
                       sizeof(esup_ret_t) + sizeof(stx_dir_entry_t) * rep.entries_len, &reply);

    return;
}

void STX_DELFILECmd(const STX_DELFILE_t *Msg)
{
    stx_cmd_file_delete_t cmd = {0};
    if (!STX_CopyFilename(cmd.name, sizeof(cmd.name), Msg->Payload.filename_max, Msg->Payload.filename_len,
                          STX_FILESYS_CC_DELFILE))
    {
        return;
    }

    esup_ret_t ret = stx_file_delete(GetDeviceByModuleId(module_id), &cmd);
    print_status("Del File", ret);
    STX_SendEsupReport(STX_FILESYS_CC_DELFILE, ret, sizeof(ret), &ret);
    return;
}

void STX_DELALLFILECmd(void)
{
    esup_ret_t ret = stx_file_delete_all(GetDeviceByModuleId(module_id));
    print_status("Del All File", ret);
    STX_SendEsupReport(STX_FILESYS_CC_DELALLFILE, ret, sizeof(ret), &ret);
    return;
}

void STX_CREATEFILECmd(const STX_CREATEFILE_t *Msg)
{
    stx_cmd_file_create_t cmd = {0};
    stx_rep_file_create_t rep = {0};

    cmd.file_length = Msg->Payload.file_size;
    if (!STX_CopyFilename(cmd.name, sizeof(cmd.name), Msg->Payload.filename_max, Msg->Payload.filename_len,
                          STX_FILESYS_CC_CREATEFILE))
    {
        return;
    }

    esup_ret_t ret = stx_file_create(GetDeviceByModuleId(module_id), &cmd, &rep);
    print_status("Create File", ret);

    CREATEFILE_REPLY_t reply = {0};
    reply.ret = ret;

    if (ret == ESUP_OK) {
        file_handle = rep.handle;
        reply.file_handle = file_handle;
        OS_printf("File Handle : 0x%08X\n", file_handle);
    }

    STX_SendEsupReport(STX_FILESYS_CC_CREATEFILE, ret, sizeof(reply), &reply);
    return;
}

void STX_WRITEFILECmd(const STX_WRITEFILE_t *Msg)
{
    
    const STX_WRITEFILE_t *cmd = (const STX_WRITEFILE_t *)Msg;
    char filename[sizeof(cmd->Payload.filename) + 1];

    FILE    *fp;
    size_t   remaining;
    uint32_t packet_number = cmd->Payload.offset;

    WRITEFILE_REPLY_t reply = {0};

    memcpy(filename, cmd->Payload.filename, sizeof(cmd->Payload.filename));
    filename[sizeof(cmd->Payload.filename)] = '\0';

    OS_printf("STX_WRITEFILECmd called: filename=%s, size=%u, offset=%u, interpacket_delay=%u\n", filename,
             cmd->Payload.size, cmd->Payload.offset, cmd->Payload.interpacket_delay);

    fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        OS_printf("file open failed\n");
        STX_rptsend(STX_FILESYS_CC_WRITEFILE, RPT_RETTYPE_HW, STX_FOPEN_EID, sizeof(reply), &reply);
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
        remaining = ftell(fp) - cmd->Payload.offset*STX_WRITE_DATA_MAX;
        fseek(fp, cmd->Payload.offset * STX_WRITE_DATA_MAX, SEEK_SET);
    }

    else if (cmd->Payload.size != 0 && cmd->Payload.offset == 0)
    {
        remaining = cmd->Payload.size * STX_WRITE_DATA_MAX;
    }

    else //cmd->Payload.size != 0 && cmd->Payload.offset != 0
    {
        remaining = cmd->Payload.size * STX_WRITE_DATA_MAX;
        fseek(fp, cmd->Payload.offset * STX_WRITE_DATA_MAX, SEEK_SET);
    }

    stx_cmd_file_write_t w = {0};
    w.handle = (int32_t)file_handle;
    OS_printf("write file ready: size=%zu, offset=%u, handle=%u\n", remaining, cmd->Payload.offset, file_handle);

    reply.file_handle = file_handle;
    reply.remaining = remaining;
    STX_rptsend(STX_FILESYS_CC_WRITEFILE, RPT_RETTYPE_SUCCESS, DEVICE_SUCCESS, sizeof(reply), &reply); //write start

    esup_ret_t ret = 0;
    uint8_t buf[STX_WRITE_DATA_MAX];
    uint8_t err_count = 0;

    while (remaining > 0 && err_count < 5)
    {
        memset(buf, 0, STX_WRITE_DATA_MAX);
        size_t bytes_to_read = (remaining > STX_WRITE_DATA_MAX) ? STX_WRITE_DATA_MAX : remaining;
        OS_printf("remain : %zu / max_length : %d / bytes_to_read : %zu \n", remaining, STX_WRITE_DATA_MAX, bytes_to_read);
        size_t bytes_read    = fread(buf, 1, bytes_to_read, fp);
        
        if (bytes_read != bytes_to_read)
        {
            OS_printf("read length error\n");
            err_count += 1;
            fseek(fp, -(long)bytes_read, SEEK_CUR);

            reply.ret = ret;
            reply.err_count = err_count;
            reply.packet_number = packet_number;
            reply.remaining = remaining;

            STX_rptsend(STX_FILESYS_CC_WRITEFILE, RPT_RETTYPE_LIB, STX_FREAD_EID, sizeof(reply), &reply);
            continue;
        }

        w.data = buf;
        w.data_len = (uint16_t)bytes_read;
        w.packet_number = packet_number;

        ret = stx_file_write(GetDeviceByModuleId(module_id), &w);
        print_status("File Send", ret);

        if (ret == ESUP_OK)
        {
            OS_printf("Write %uth packet Success\n", packet_number);
            remaining -= (size_t)w.data_len;

            reply.ret = ret;
            reply.err_count = err_count;
            reply.packet_number = packet_number;
            reply.remaining = remaining;

            packet_number += 1;
        }
        else if (ESUP_RET_EXEC_STATUS(ret) == 0x05)
        {
            err_count += 1;
            OS_printf("Error occur... Error Count : %d... Current packet : %d\n", (int)err_count, (int)packet_number);

            reply.ret = ret;
            reply.err_count = err_count;
            reply.packet_number = packet_number;
            reply.remaining = remaining;

            STX_SendEsupReport(STX_FILESYS_CC_WRITEFILE, ret, sizeof(reply), &reply);

            remaining -= (size_t)w.data_len;
            packet_number += 1;
        }
        else
        {
            err_count += 1;
            OS_printf("Error occur... Error Count : %d... Current packet : %d\n", (int)err_count, (int)packet_number);
            fseek(fp, -(long)bytes_read, SEEK_CUR);

            reply.ret = ret;
            reply.err_count = err_count;
            reply.packet_number = packet_number;
            reply.remaining = remaining;

            STX_SendEsupReport(STX_FILESYS_CC_WRITEFILE, ret, sizeof(reply), &reply);
        }

        OS_TaskDelay(3);
    }

    if (err_count == 0) STX_rptsend(STX_FILESYS_CC_WRITEFILE, RPT_RETTYPE_SUCCESS, DEVICE_SUCCESS, sizeof(reply), &reply);

    fclose(fp);

    return;
}

void STX_OPENFILECmd(const STX_OPENFILE_t *Msg)
{
    stx_cmd_file_open_t oc = {0};
    if (!STX_CopyFilename(oc.name, sizeof(oc.name), Msg->Payload.filename_max, Msg->Payload.filename_len,
                          STX_FILESYS_CC_OPENFILE))
    {
        return;
    }

    OPENFILE_REPLY_t reply = {0};

    stx_rep_file_open_t orp = {0};
    esup_ret_t ret = stx_file_open(GetDeviceByModuleId(module_id), &oc, &orp);
    print_status("Open File", ret);
    if (ret == ESUP_OK){
        OS_printf("File handle : 0x%08X , length : %u\n", (unsigned)orp.handle, orp.file_length);
        file_handle = orp.handle;
    }

    reply.ret = ret;
    reply.orp = orp;
    STX_SendEsupReport(STX_FILESYS_CC_OPENFILE, ret, sizeof(reply), &reply);

    return;
}

void STX_READFILECmd(const STX_READFILE_t *Msg)
{
    uint8_t buf[ESUP_DATA_MAX] = {0};
    stx_cmd_file_read_t rq ={.handle = file_handle};
    stx_rep_file_read_t rr = {.data = buf, .data_cap = sizeof(buf)};
    READFILE_REPLY_t reply = {0};

    esup_ret_t ret = stx_file_read(GetDeviceByModuleId(module_id), &rq, &rr);
    print_status("Read File", ret);

    reply.ret           = ret;
    reply.packet_len    = rr.packet_len;
    reply.packet_number = rr.packet_number;
    if (ret == ESUP_OK)
    {
        size_t copy_len = rr.packet_len > sizeof(reply.data) ? sizeof(reply.data) : rr.packet_len;
        memcpy(reply.data, rr.data, copy_len);
    }

    STX_SendEsupReport(STX_FILESYS_CC_READFILE, ret,
                       sizeof(reply.ret) + sizeof(reply.packet_len) + sizeof(reply.packet_number) + rr.packet_len,
                       &reply);

    return;
}

void STX_SENDFILE_WITH_ERROR_Cmd(const STX_SENDFILE_t * Msg)
{
    stx_cmd_file_send_t s = {0};
    if (!STX_CopyFilename(s.name, sizeof(s.name), Msg->Payload.filename_max, Msg->Payload.filename_len,
                          STX_FILESYS_CC_SENDFILERTI))
    {
        return;
    }
    s.send_type = STX_SEND_RF_TRACT_ISSUE;

    esup_ret_t ret = stx_file_send(GetDeviceByModuleId(module_id), &s, 300000);
    print_status("File Send RF Track Issue", ret);

    STX_SendEsupReport(STX_FILESYS_CC_SENDFILERTI, ret, sizeof(ret), &ret);
    return;
}

void STX_SENDFILECmd(const STX_SENDFILE_t *Msg)
{
    stx_cmd_file_send_t s ={0};
    if (!STX_CopyFilename(s.name, sizeof(s.name), Msg->Payload.filename_max, Msg->Payload.filename_len,
                          STX_FILESYS_CC_SENDFILE))
    {
        return;
    }
    s.send_type = STX_SEND_NORMAL;

    esup_ret_t ret = stx_file_send(GetDeviceByModuleId(module_id), &s, 300000);
    print_status("File Send", ret);

    STX_SendEsupReport(STX_FILESYS_CC_SENDFILE, ret, sizeof(ret), &ret);
    return;
}

/*  mode command    */

void STX_SYSCONF_CC_TRANSMITMODECmd(void)
{
    esup_ret_t ret = stx_enter_transmit(GetDeviceByModuleId(module_id));
    print_status("Tx Mode", ret);

    STX_SendEsupReport(STX_SYSCONF_CC_TRANSMITMODE, ret, sizeof(ret), &ret);
    return;
}

void STX_SYSCONF_CC_IDLEMODECmd(void)
{
    esup_ret_t ret = stx_enter_idle(GetDeviceByModuleId(module_id));
    print_status("Idle Mode", ret);

    STX_SendEsupReport(STX_SYSCONF_CC_IDLEMODE, ret, sizeof(ret), &ret);
    return;
}

void STX_SYSCONF_CC_SAFESHUTDOWNCmd(void)
{
    printf("shutdown: 'unreachable' is the expected success indication\n");
    esup_ret_t ret = stx_safe_shutdown(GetDeviceByModuleId(module_id));
    print_status("Safe Shutdown", ret);

    STX_SendEsupReport(STX_SYSCONF_CC_SAFESHUTDOWN, ret, sizeof(ret), &ret);
    return;
}

void STX_SYSCONF_CC_UPDATEFWCmd(const STX_FWUPDATE_t *Msg)
{
    stx_cmd_update_fw_t uc = {0};
    if (!STX_CopyFilename(uc.name, sizeof(uc.name), Msg->Payload.filename, Msg->Payload.filename_len,
                          STX_SYSCONF_CC_UPDATEFW))
    {
        return;
    }

    esup_ret_t ret = stx_update_fw(GetDeviceByModuleId(module_id), &uc);
    print_status("Update FW", ret);

    STX_SendEsupReport(STX_SYSCONF_CC_UPDATEFW, ret, sizeof(ret), &ret);
    return;
}

/* Get command */

void STX_GET_SYMBOL_RATECmd(void)
{
    uint8_t msps;
    esup_ret_t ret;

    GET_SYMBOLRATE_REPLY_t reply = {0};

    ret = stx_get_symbol_rate(GetDeviceByModuleId(module_id), &msps);
    print_status("GET Sym Rate", ret);
    if (ret == ESUP_OK) {
        OS_printf("Symbol Rate : %u\n", msps);
        reply.msps = msps;
    }
    reply.ret = ret;
    STX_SendEsupReport(STX_GET_SYMBOL_RATE, ret, sizeof(reply), &reply);
    return;
}

void STX_GET_TX_POWERCmd(void)
{
    uint8_t dbm;
    esup_ret_t ret;

    GET_TX_POWER_REPLY_t reply = {0};

    ret = stx_get_tx_power(GetDeviceByModuleId(module_id), &dbm);
    print_status("Get Tx Pwr", ret);
    if (ret == ESUP_OK) {
        OS_printf("TX pwr : %u\n", dbm);
        reply.dbm = dbm;
    }
    reply.ret = ret;
    STX_SendEsupReport(STX_GET_TX_POWER, ret, sizeof(reply), &reply);
    return;
}

void STX_GET_CENTER_FREQCmd(void)
{
    float mhz;
    esup_ret_t ret;

    GET_CENTERFREQ_REPLY_t reply = {0};

    ret = stx_get_center_freq(GetDeviceByModuleId(module_id), &mhz);
    print_status("Get Center Freq", ret);
    if (ret == ESUP_OK) {
        OS_printf("Center Freq : %f\n", mhz);
        reply.mhz = mhz;
    }
    reply.ret = ret;
    STX_SendEsupReport(STX_GET_CENTER_FREQ, ret, sizeof(reply), &reply);
    return;
}

void STX_GET_MODCODCmd(void)
{
    uint8_t modcod;
    esup_ret_t ret;

    GET_MODCOD_REPLY_t reply = {0};

    ret = stx_get_modcod(GetDeviceByModuleId(module_id), &modcod);
    print_status("Get MODCOD", ret);
    if (ret == ESUP_OK) {
        OS_printf("Mod Cod : %u\n", modcod);
        reply.modcod = modcod;
    }
    reply.ret = ret;
    STX_SendEsupReport(STX_GET_MODCOD, ret, sizeof(reply), &reply);
    return;
}

void STX_GET_ROLL_OFFCmd(void)
{
    uint8_t roll_off;
    esup_ret_t ret;

    GET_ROLLOFF_REPLY_t reply = {0};

    ret = stx_get_roll_off(GetDeviceByModuleId(module_id), &roll_off);
    print_status("Get Roll Off", ret);
    if (ret == ESUP_OK) {
        OS_printf("Roll Off : %u\n", roll_off);
        reply.roll_off = roll_off;
    }
    reply.ret = ret;
    STX_SendEsupReport(STX_GET_ROLL_OFF, ret, sizeof(reply), &reply);
    return;
}

void STX_GET_PILOT_SIGNALCmd(void)
{
    uint8_t pilot;
    esup_ret_t ret;

    GET_PILOT_REPLY_t reply = {0};

    ret = stx_get_pilot(GetDeviceByModuleId(module_id), &pilot);
    print_status("Get Pilot", ret);
    if (ret == ESUP_OK) {
        OS_printf("Pilot Signal : %u\n", pilot);
        reply.pilot = pilot;
    }
    reply.ret = ret;
    STX_SendEsupReport(STX_GET_PILOT_SIGNAL, ret, sizeof(reply), &reply);
    return;
}

void STX_GET_FEC_FRAME_SIZECmd(void)
{
    uint8_t frame;
    esup_ret_t ret;

    GET_FECFRAME_REPLY_t reply = {0};

    ret = stx_get_fec_frame(GetDeviceByModuleId(module_id), &frame);
    print_status("Get FEC Frame Size", ret);
    if (ret == ESUP_OK) {
        OS_printf("FEC Frame : %u\n", frame);
        reply.frame = frame;
    }
    reply.ret = ret;
    STX_SendEsupReport(STX_GET_FEC_FRAME_SIZE, ret, sizeof(reply), &reply);
    return;
}

void STX_GET_PRETX_DELAYCmd(void)
{
    uint16_t ms;
    esup_ret_t ret;

    GET_PRETXDELAY_REPLY_t reply = {0};

    ret = stx_get_pretx_delay(GetDeviceByModuleId(module_id), &ms);
    print_status("Get PreTx Dely", ret);
    if (ret == ESUP_OK) {
        OS_printf("Pre Tx Delay : %u\n", ms);
        reply.ms = ms;
    }
    reply.ret = ret;
    STX_SendEsupReport(STX_GET_PRETX_DELAY, ret, sizeof(reply), &reply);
    return;
}

void STX_GET_ALL_PRAMETERSCmd(void)
{
    stx_params_t params;
    esup_ret_t ret;

    GET_ALLPARAM_REPLY_t reply = {0};

    ret = stx_get_params(GetDeviceByModuleId(module_id), &params);
    print_status("Get All Parmas", ret);
    if(ret == ESUP_OK){
        OS_printf("Symbol Rate : %u", params.symbol_rate);
        OS_printf("Tx Pwr : %u\n", params.tx_power_dbm);
        OS_printf("Center Freq : %f\n", params.center_freq_mhz);
        OS_printf("MODCOD : %u\n", params.modcod);
        OS_printf("Roll Off : %u\n", params.roll_off);
        OS_printf("Pilot Signal : %u\n", params.pilot);
        OS_printf("FEC Frame : %u\n", params.fec_frame_short);
        OS_printf("Pre Tx Delay : %u\n", params.pretx_delay_ms);
        reply.params = params;
    }
    reply.ret = ret;
    STX_SendEsupReport(STX_GET_ALL_PRAMETERS, ret, sizeof(reply), &reply);
    return;
}

void STX_GET_REPORTCmd(void)
{
    stx_report_t report;
    esup_ret_t ret;

    GET_REPORT_REPLY_t reply = {0};

    ret = stx_get_report(GetDeviceByModuleId(module_id), &report);
    print_status("Get Report", ret);
    if (ret == ESUP_OK){
        OS_printf("System Status : %u\n", report.state);
        OS_printf("Status Flags : 0x%02x\n", report.flags);
        OS_printf("CPU Temp : %f\n", report.cpu_temp_c);
        OS_printf("Firmware Version : %u\n", report.fw_version);
        reply.report = report;
    }
    reply.ret = ret;
    STX_SendEsupReport(STX_GET_REPORT, ret, sizeof(reply), &reply);
    return;
}

void STX_GET_MODULATOR_DATA_INTERFACECmd(void)
{
    stx_mod_data_iface_t iface;
    esup_ret_t ret;

    GET_MODDATAIFACE_REPLY_t reply = {0};

    ret = stx_get_mod_data_iface(GetDeviceByModuleId(module_id), &iface);
    print_status("Get Modulator Data Interface", ret);
    if (ret == ESUP_OK){
        OS_printf("Interface Type : %un", iface.interface_type);
        OS_printf("lVDS IO Type : %u\n", iface.lvds_io_type);
        reply.iface = iface;
    }
    reply.ret = ret;
    STX_SendEsupReport(STX_GET_MODULATOR_DATA_INTERFACE, ret, sizeof(reply), &reply);
    return;
}

void STX_Param_init(void)
{
    esup_ret_t ret[5] = {0};
    esup_ret_t final_ret = ESUP_OK;
    stx_t *device = GetDeviceByModuleId(module_id);

    ret[0] = stx_set_symbol_rate(device, 2);
    ret[1] = stx_set_center_freq(device, 2403.5f);
    ret[2] = stx_set_modcod(device, 1);
    ret[3] = stx_set_roll_off(device, STX_ROLL_OFF_0_20);
    ret[4] = stx_set_tx_power(device, 33);

    for (size_t i = 0; i < (sizeof(ret) / sizeof(ret[0])); ++i)
    {
        if (final_ret == ESUP_OK && ret[i] != ESUP_OK)
        {
            final_ret = ret[i];
        }
    }

    STX_SendEsupReport(STX_PARAM_INIT_CC, final_ret, sizeof(ret), ret);
}
