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
 *   This file contains the source code for the GPS App Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "gps_app.h"
#include "gps_cmds.h"
#include "gps_msg.h"
#include "gps_eventids.h"
#include "gps_version.h"
#include "gps_msg.h"
#include "gps_cmds_oem.h"

#include "gps_dev_oem.h"
#include "oem.h"

/*
** OEM receiver commands.
*/
void GPS_OEMCmd_LogCmd(const GPS_OEMCmd_LogCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Cmd_LOG(Msg->portIndex,
                      Msg->Payload.msgId,
                      Msg->Payload.port,
                      Msg->Payload.type,
                      Msg->Payload.trigger,
                      Msg->Payload.period,
                      Msg->Payload.offset,
                      Msg->Payload.hold);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMCmd_LogOnceCmd(const GPS_OEMCmd_LogOnceCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Cmd_LogOnce(Msg->portIndex,
                          Msg->Payload.msgId,
                          Msg->Payload.port);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMCmd_LogOnTimeCmd(const GPS_OEMCmd_LogOnTimeCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Cmd_LogOnTime(Msg->portIndex,
                            Msg->Payload.msgId,
                            Msg->Payload.port,
                            Msg->Payload.period,
                            Msg->Payload.offset);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMCmd_LogOnChangedCmd(const GPS_OEMCmd_LogOnChangedCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Cmd_LogOnChanged(Msg->portIndex,
                               Msg->Payload.msgId,
                               Msg->Payload.port);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMCmd_LogOnNewCmd(const GPS_OEMCmd_LogOnNewCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Cmd_LogOnNew(Msg->portIndex,
                           Msg->Payload.msgId,
                           Msg->Payload.port);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMCmd_UnlogCmd(const GPS_OEMCmd_UnlogCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Cmd_UNLOG(Msg->portIndex,
                        Msg->Payload.port,
                        Msg->Payload.msgId,
                        Msg->Payload.type);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMCmd_UnlogAllCmd(const GPS_OEMCmd_UnlogAllCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Cmd_UNLOGALL(Msg->portIndex,
                           Msg->Payload.port,
                           Msg->Payload.held);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMCmd_ElevationCutoffCmd(const GPS_OEMCmd_ElevationCutoffCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Cmd_ELEVATIONCUTOFF(Msg->portIndex,
                                  Msg->Payload.constellation,
                                  Msg->Payload.cutoff);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMCmd_InterfaceModeCmd(const GPS_OEMCmd_InterfaceModeCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Cmd_INTERFACEMODE(Msg->portIndex,
                                Msg->Payload.port,
                                Msg->Payload.rxType,
                                Msg->Payload.txType,
                                Msg->Payload.responses);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMCmd_SerialConfigCmd(const GPS_OEMCmd_SerialConfigCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Cmd_SERIALCONFIG(Msg->portIndex,
                               Msg->Payload.port,
                               Msg->Payload.baud,
                               Msg->Payload.parity,
                               Msg->Payload.databits,
                               Msg->Payload.stopbits,
                               Msg->Payload.handshake,
                               Msg->Payload._break);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMCmd_PublishCmd(const GPS_OEMCmd_PublishCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    if (Msg->Payload.bodylength > sizeof(Msg->Payload.body)) {
        GPS_AppData.Counters.ErrCounter++;
        GPS_SendReport(Msg,
                       &Msg->Payload.bodylength,
                       sizeof(Msg->Payload.bodylength),
                       OEM_ERR_RANGE,
                       RPT_RETTYPE_HW);
        return;
    }

    ret = OEM_AssemblePublishCmd(Msg->portIndex,
                                 Msg->Payload.msgId,
                                 Msg->Payload.bodylength,
                                 Msg->Payload.body);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMLog_GetHandlerHkCmd(const GPS_OEMLog_GetHandlerHkCmd_t* Msg)
{
    int ret;
    oem_log_handler_hk_t hk;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Log_GethandlerHousekeeping(Msg->Payload.msgId,
                                         &hk);
    if (ret != OEM_OK) {
        GPS_AppData.Counters.ErrCounter++;
        GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
        return;
    }
    
    GPS_SendReport(Msg, &hk, sizeof(hk), ret, RPT_RETTYPE_HW);
}

void GPS_OEMLog_GetMsgStatCmd(const GPS_OEMLog_GetMsgStatCmd_t* Msg)
{
    int ret;
    oem_log_handler_stat_t hk;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Log_GetMessageStatistics(Msg->Payload.msgId,
                                       &hk);
    if (ret != OEM_OK) {
        GPS_AppData.Counters.ErrCounter++;
        GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
        return;
    }
    
    GPS_SendReport(Msg, &hk, sizeof(hk), ret, RPT_RETTYPE_HW);
}

void GPS_OEMLog_SetHandlerStatusCmd(const GPS_OEMLog_SetHandlerStatusCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Log_SetHandlerStatus(Msg->Payload.msgId,
                                   Msg->Payload.status,
                                   Msg->Payload.override);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMLog_GetHandlerStatusCmd(const GPS_OEMLog_GetHandlerStatusCmd_t* Msg)
{
    int ret;
    uint8_t hk;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Log_GetHandlerStatus(Msg->Payload.msgId,
                                   &hk);
    if (ret != OEM_OK) {
        GPS_AppData.Counters.ErrCounter++;
        GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
        return;
    }
    
    GPS_SendReport(Msg, &hk, sizeof(hk), ret, RPT_RETTYPE_HW);
}

void GPS_OEMLog_HandlerActivateCmd(const GPS_OEMLog_HandlerActivateCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Log_HandlerAcivate(Msg->Payload.msgId);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMLog_HandlerDeactivateCmd(const GPS_OEMLog_HandlerDeactivateCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Log_HandlerDeacivate(Msg->Payload.msgId);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMLog_HandlerGoDormantCmd(const GPS_OEMLog_HandlerGoDormantCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Log_HandlerGoDormant(Msg->Payload.msgId);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMLog_HandlerWakeupCmd(const GPS_OEMLog_HandlerWakeupCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Log_HandlerWakeup(Msg->Payload.msgId);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMLog_HandlerActivateAllCmd(const GPS_OEMLog_HandlerActivateAllCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Log_HandlerActivateAll();
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMLog_HandlerDeactivateAllCmd(const GPS_OEMLog_HandlerDeactivateAllCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Log_HandlerDeactivateAll();
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMLog_HandlerRegisterCmd(const GPS_OEMLog_HandlerRegisterCmd_t* Msg)
{
    int ret;
    char name[OEM_LOG_HANDLER_NAME_LEN];

    GPS_AppData.Counters.CmdCounter++;

    memcpy(name, Msg->Payload.name, OEM_LOG_HANDLER_NAME_LEN);
    name[OEM_LOG_HANDLER_NAME_LEN - 1] = '\0';

    ret = OEM_Log_RegisterHandler(name,
                                  Msg->Payload.msgId,
                                  Msg->Payload.msgLength);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, name, OEM_LOG_HANDLER_NAME_LEN, ret, RPT_RETTYPE_HW);
}

void GPS_OEMLog_HandlerUnregisterCmd(const GPS_OEMLog_HandlerUnregisterCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Log_UnregisterHandler(Msg->Payload.msgId);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMLog_AddCallbackCmd(const GPS_OEMLog_AddCallbackCmd_t* Msg)
{
    int ret;
    char err[64];
    oem_log_callback_t callback;

    GPS_AppData.Counters.CmdCounter++;

    memset(err, 0, sizeof(err));

    ret = GPS_Device_LoadFunctionSymbol(Msg->Payload.libpath,
                                        Msg->Payload.funcName,
                                        &callback,
                                        Msg->Payload.options,
                                        err);
    if (ret != GPS_DEV_SUCCESS) {
        GPS_AppData.Counters.ErrCounter++;
        GPS_SendReport(Msg, err, sizeof(err), ret, RPT_RETTYPE_HW);
        return;
    }

    ret = OEM_Log_AddCallback(Msg->Payload.msgId,
                              callback);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMLog_ClearCallbackCmd(const GPS_OEMLog_ClearCallbackCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Log_ClearCallbacks(Msg->Payload.msgId);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMLog_HandlerSetBrokenCmd(const GPS_OEMLog_HandlerSetBrokenCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Log_HandlerSetBroken(Msg->Payload.msgId);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMLog_GetHandlerMsgLengthCmd(const GPS_OEMLog_GetHandlerMsgLengthCmd_t* Msg)
{
    int ret;
    oem_ushort len = 0;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Log_GetMessageLength(Msg->Payload.msgId,
                                   &len);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, &len, sizeof(len), ret, RPT_RETTYPE_HW);
}

void GPS_OEMLog_GetHandlerNameCmd(const GPS_OEMLog_GetHandlerNameCmd_t* Msg)
{
    int ret;
    char name[OEM_LOG_HANDLER_NAME_LEN];
    size_t retSize = sizeof(name);

    GPS_AppData.Counters.CmdCounter++;

    memset(name, 0, sizeof(name));

    ret = OEM_Log_GetHandlerName(Msg->Payload.msgId,
                                 name);
    if (ret != OEM_OK) {
        retSize = 0;
        GPS_AppData.Counters.ErrCounter++;
    }
    
    GPS_SendReport(Msg, name, retSize, ret, RPT_RETTYPE_HW);
}

void GPS_OEMLog_ResetHandlerCountersCmd(const GPS_OEMLog_ResetHandlerCountersCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Log_ResetHandlerCounters(Msg->Payload.msgId);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMLog_DumpRecentMsgCmd(const GPS_OEMLog_DumpRecentMsgCmd_t* Msg)
{
    int ret;
    size_t copied;
    uint8 buffer[RPT_RET_VALUE_BUF_SIZE];

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Log_DumpRecentMessage(Msg->Payload.msgId,
                                    buffer,
                                    RPT_RET_VALUE_BUF_SIZE,
                                    &copied);
    if (ret != OEM_OK) {
        copied = 0;
        GPS_AppData.Counters.ErrCounter++;
    }
    
    GPS_SendReport(Msg,
                   buffer,
                   copied > RPT_RET_VALUE_BUF_SIZE
                          ? RPT_RET_VALUE_BUF_SIZE
                          : copied,
                   ret,
                   RPT_RETTYPE_HW);
}

void GPS_OEMLog_IgnoreChecksumCmd(const GPS_OEMLog_IgnoreChecksumCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Log_EnableCsVerification(Msg->Payload.msgId);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMLog_DonotIgnoreChecksumCmd(const GPS_OEMLog_DonotIgnoreChecksumCmd_t* Msg)
{
    int ret;

    GPS_AppData.Counters.CmdCounter++;

    ret = OEM_Log_DisableCsVerification(Msg->Payload.msgId);
    if (ret != OEM_OK)
        GPS_AppData.Counters.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, RPT_RETTYPE_HW);
}

void GPS_OEMLog_LockHandlersCmd(const GPS_OEMLog_LockHandlersCmd_t* Msg)
{
    return;
}

void GPS_OEMLog_UnlockHandlersCmd(const GPS_OEMLog_UnlockHandlersCmd_t* Msg)
{
    return;
}
