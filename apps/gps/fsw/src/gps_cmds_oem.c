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

#include "gps_report.h"
#include "oem.h"

#include <string.h>
#include <stdio.h>

/**
 * One error event per failed OEM command. The command report carries the
 * outcome either way; the event is what reaches an operator watching the
 * event log without decoding telemetry.
 */
static void OemCmdFailed(const char* Name, int Ret)
{
    CFE_EVS_SendEvent(GPS_OEM_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                      "GPS: %s failed, RC = %d", Name, Ret);
}

/*
** OEM receiver commands.
*/
void GPS_OEM_Cmd_Log(const GPS_OEM_Cmd_Log_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_cmd_LOG(Msg->Payload.interfaceIndex,
                      Msg->Payload.msgId,
                      Msg->Payload.port,
                      Msg->Payload.type,
                      Msg->Payload.trigger,
                      Msg->Payload.period,
                      Msg->Payload.offset,
                      Msg->Payload.hold);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("Log", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Cmd_LogOnce(const GPS_OEM_Cmd_LogOnce_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_cmd_LOG_once(Msg->Payload.interfaceIndex,
                           Msg->Payload.msgId,
                           Msg->Payload.port);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("LogOnce", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Cmd_LogOnTime(const GPS_OEM_Cmd_LogOnTime_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_cmd_LOG_ontime(Msg->Payload.interfaceIndex,
                             Msg->Payload.msgId,
                             Msg->Payload.port,
                             Msg->Payload.period,
                             Msg->Payload.offset);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("LogOnTime", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Cmd_LogOnChanged(const GPS_OEM_Cmd_LogOnChanged_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_cmd_LOG_onchanged(Msg->Payload.interfaceIndex,
                                Msg->Payload.msgId,
                                Msg->Payload.port);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("LogOnChanged", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Cmd_LogOnNew(const GPS_OEM_Cmd_LogOnNew_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_cmd_LOG_onnew(Msg->Payload.interfaceIndex,
                            Msg->Payload.msgId,
                            Msg->Payload.port);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("LogOnNew", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Cmd_Unlog(const GPS_OEM_Cmd_Unlog_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_cmd_UNLOG(Msg->Payload.interfaceIndex,
                        Msg->Payload.port,
                        Msg->Payload.msgId,
                        Msg->Payload.type);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("Unlog", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Cmd_UnlogAll(const GPS_OEM_Cmd_UnlogAll_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_cmd_UNLOGALL(Msg->Payload.interfaceIndex,
                           Msg->Payload.port,
                           Msg->Payload.held);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("UnlogAll", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Cmd_ElevationCutoff(const GPS_OEM_Cmd_ElevationCutoff_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_cmd_ELEVATIONCUTOFF(Msg->Payload.interfaceIndex,
                                  Msg->Payload.constellation,
                                  Msg->Payload.cutoff);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("ElevationCutoff", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Cmd_InterfaceMode(const GPS_OEM_Cmd_InterfaceMode_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_cmd_INTERFACEMODE(Msg->Payload.interfaceIndex,
                                Msg->Payload.port,
                                Msg->Payload.rxType,
                                Msg->Payload.txType,
                                Msg->Payload.responses);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("InterfaceMode", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Cmd_SerialConfig(const GPS_OEM_Cmd_SerialConfig_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_cmd_SERIALCONFIG(Msg->Payload.interfaceIndex,
                               Msg->Payload.port,
                               Msg->Payload.baud,
                               Msg->Payload.parity,
                               Msg->Payload.databits,
                               Msg->Payload.stopbits,
                               Msg->Payload.handshake,
                               Msg->Payload._break);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("SerialConfig", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Cmd_Publish(const GPS_OEM_Cmd_Publish_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    if (Msg->Payload.bodylength > sizeof(Msg->Payload.body)) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("Publish", OEM_ERR_RANGE);
        GPS_SendReport(Msg, NULL, 0, OEM_ERR_RANGE, GPS_MISSION_REPORT_RETTYPE_HW);
        return;
    }

    ret = oem_cmd_publish(Msg->Payload.interfaceIndex,
                          Msg->Payload.msgId,
                          Msg->Payload.bodylength,
                          Msg->Payload.body);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("Publish", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Log_GetHandlerHk(const GPS_OEM_Log_GetHandlerHk_t* Msg)
{
    int ret;
    oem_log_handler_hk_t hk;

    GPS_AppData.CmdCounter++;

    ret = oem_log_get_handler_hk(Msg->Payload.msgId,
                                 &hk);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("GetHandlerHk", ret);
        GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
        return;
    }
    
    GPS_SendReport(Msg, &hk, sizeof(hk), ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Log_GetStat(const GPS_OEM_Log_GetStat_t* Msg)
{
    int ret;
    oem_log_stat_t hk;

    GPS_AppData.CmdCounter++;

    ret = oem_log_get_stat(Msg->Payload.msgId,
                           &hk);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("GetStat", ret);
        GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
        return;
    }
    
    GPS_SendReport(Msg, &hk, sizeof(hk), ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Log_HandlerSetStatus(const GPS_OEM_Log_HandlerSetStatus_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_log_handler_set_status(Msg->Payload.msgId,
                                     Msg->Payload.status,
                                     Msg->Payload.override);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("HandlerSetStatus", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Log_HandlerGetStatus(const GPS_OEM_Log_HandlerGetStatus_t* Msg)
{
    int ret;
    uint8_t hk;

    GPS_AppData.CmdCounter++;

    ret = oem_log_handler_get_status(Msg->Payload.msgId,
                                     &hk);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("HandlerGetStatus", ret);
        GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
        return;
    }
    
    GPS_SendReport(Msg, &hk, sizeof(hk), ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Log_HandlerActivate(const GPS_OEM_Log_HandlerActivate_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_log_handler_activate(Msg->Payload.msgId);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("HandlerActivate", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Log_HandlerDeactivate(const GPS_OEM_Log_HandlerDeactivate_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_log_handler_deactivate(Msg->Payload.msgId);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("HandlerDeactivate", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Log_HandlerGoDormant(const GPS_OEM_Log_HandlerGoDormant_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_log_handler_go_dormant(Msg->Payload.msgId);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("HandlerGoDormant", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Log_HandlerWakeup(const GPS_OEM_Log_HandlerWakeup_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_log_handler_wakeup(Msg->Payload.msgId);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("HandlerWakeup", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Log_HandlerActivateAll(const GPS_OEM_Log_HandlerActivateAll_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_log_handler_activate_all();
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("HandlerActivateAll", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Log_HandlerDeactivateAll(const GPS_OEM_Log_HandlerDeactivateAll_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_log_handler_deactivate_all();
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("HandlerDeactivateAll", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Log_HandlerRegister(const GPS_OEM_Log_HandlerRegister_t* Msg)
{
    int ret;
    char name[OEM_LOG_HANDLER_NAME_LEN];

    GPS_AppData.CmdCounter++;

    memcpy(name, Msg->Payload.name, OEM_LOG_HANDLER_NAME_LEN);
    name[OEM_LOG_HANDLER_NAME_LEN - 1] = '\0';

    ret = oem_log_handler_register(name,
                                   Msg->Payload.msgId,
                                   Msg->Payload.msgLength);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("HandlerRegister", ret);
    }
    
    GPS_SendReport(Msg, name, OEM_LOG_HANDLER_NAME_LEN, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Log_HandlerUnregister(const GPS_OEM_Log_HandlerUnregister_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_log_handler_unregister(Msg->Payload.msgId);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("HandlerUnregister", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

/** Reject anything outside the module directory, and any traversal. */
static bool ModulePathOk(const char* Path)
{
    return strncmp(Path, GPS_PLATFORM_MODULE_DIR, sizeof(GPS_PLATFORM_MODULE_DIR) - 1) == 0
           && strstr(Path, "..") == NULL;
}

/**
 * Attach a log callback, loading it from a module if the symbol is not already
 * present. Looking the symbol up first means a built-in callback needs no file
 * at all, and a second callback out of an already-loaded module does not
 * consume another module slot.
 *
 * A module is never unloaded once a callback from it is registered: the
 * registered address must stay valid for the life of the handler.
 */
void GPS_OEM_Log_AddCallback(const GPS_OEM_Log_AddCallback_t* Msg)
{
    static uint32 ModuleCount;

    char      LibPath[sizeof(Msg->Payload.libpath) + 1];
    char      FuncName[sizeof(Msg->Payload.funcName) + 1];
    char      ModuleName[OS_MAX_API_NAME];
    cpuaddr   Symbol = 0;
    osal_id_t Module = OS_OBJECT_ID_UNDEFINED;
    int32     OsStatus;
    int       Ret;

    GPS_AppData.CmdCounter++;

    /* Ground strings carry no guaranteed terminator. */
    memcpy(LibPath, Msg->Payload.libpath, sizeof(Msg->Payload.libpath));
    LibPath[sizeof(Msg->Payload.libpath)] = '\0';
    memcpy(FuncName, Msg->Payload.funcName, sizeof(Msg->Payload.funcName));
    FuncName[sizeof(Msg->Payload.funcName)] = '\0';

    if (Msg->Payload.options != 0 || FuncName[0] == '\0') {
        GPS_AppData.ErrCounter++;
        GPS_SendReport(Msg, NULL, 0, GPS_ERR_ARG, GPS_MISSION_REPORT_RETTYPE_APP);
        return;
    }

    if (OS_SymbolLookup(&Symbol, FuncName) != OS_SUCCESS) {
        if (!ModulePathOk(LibPath)) {
            CFE_EVS_SendEvent(GPS_ADDCB_ERR_EID, CFE_EVS_EventType_ERROR,
                              "GPS: callback module path rejected: %s", LibPath);
            GPS_AppData.ErrCounter++;
            GPS_SendReport(Msg, NULL, 0, GPS_ERR_PATH, GPS_MISSION_REPORT_RETTYPE_APP);
            return;
        }

        snprintf(ModuleName, sizeof(ModuleName), "GPSCB%02u", (unsigned int)(ModuleCount % 100));

        OsStatus = OS_ModuleLoad(&Module, ModuleName, LibPath, OS_MODULE_FLAG_LOCAL_SYMBOLS);
        if (OsStatus != OS_SUCCESS) {
            CFE_EVS_SendEvent(GPS_ADDCB_ERR_EID, CFE_EVS_EventType_ERROR,
                              "GPS: module load failed for %s, RC = %ld", LibPath, (long)OsStatus);
            GPS_AppData.ErrCounter++;
            GPS_SendReport(Msg, NULL, 0, GPS_ERR_MODULE, GPS_MISSION_REPORT_RETTYPE_APP);
            return;
        }
        ModuleCount++;

        OsStatus = OS_ModuleSymbolLookup(Module, &Symbol, FuncName);
        if (OsStatus != OS_SUCCESS) {
            CFE_EVS_SendEvent(GPS_ADDCB_ERR_EID, CFE_EVS_EventType_ERROR,
                              "GPS: symbol %s not found in %s, RC = %ld",
                              FuncName, LibPath, (long)OsStatus);
            OS_ModuleUnload(Module);
            GPS_AppData.ErrCounter++;
            GPS_SendReport(Msg, NULL, 0, GPS_ERR_SYMBOL, GPS_MISSION_REPORT_RETTYPE_APP);
            return;
        }
    }

    Ret = oem_log_add_callback(Msg->Payload.msgId, (oem_log_callback_t)Symbol);
    if (Ret != OEM_OK) {
        if (OS_ObjectIdDefined(Module))
            OS_ModuleUnload(Module);
        GPS_AppData.ErrCounter++;
    }
    else {
        CFE_EVS_SendEvent(GPS_ADDCB_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "GPS: callback %s attached to log %u", FuncName,
                          (unsigned int)Msg->Payload.msgId);
    }

    GPS_SendReport(Msg, NULL, 0, Ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Log_ClearCallbacks(const GPS_OEM_Log_ClearCallbacks_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_log_clear_callbacks(Msg->Payload.msgId);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("ClearCallbacks", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Log_HandlerMarkBroken(const GPS_OEM_Log_HandlerMarkBroken_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_log_handler_mark_broken(Msg->Payload.msgId);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("HandlerMarkBroken", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Log_GetMessageLength(const GPS_OEM_Log_GetMessageLength_t* Msg)
{
    int ret;
    oem_ushort len = 0;

    GPS_AppData.CmdCounter++;

    ret = oem_log_get_message_length(Msg->Payload.msgId,
                                     &len);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("GetMessageLength", ret);
    }
    
    GPS_SendReport(Msg, &len, sizeof(len), ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Log_GetHandlerName(const GPS_OEM_Log_GetHandlerName_t* Msg)
{
    int ret;
    char name[OEM_LOG_HANDLER_NAME_LEN];
    size_t retSize = sizeof(name);

    GPS_AppData.CmdCounter++;

    memset(name, 0, sizeof(name));

    ret = oem_log_get_handler_name(Msg->Payload.msgId,
                                   name);
    if (ret != OEM_OK) {
        retSize = 0;
        GPS_AppData.ErrCounter++;
        OemCmdFailed("GetHandlerName", ret);
    }
    
    GPS_SendReport(Msg, name, retSize, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Log_ResetStat(const GPS_OEM_Log_ResetStat_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_log_reset_stat(Msg->Payload.msgId);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("ResetStat", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Log_GetRecentMessage(const GPS_OEM_Log_GetRecentMessage_t* Msg)
{
    int ret;
    size_t copied;
    uint8 buffer[GPS_MISSION_REPORT_DATA_SIZE];

    GPS_AppData.CmdCounter++;

    ret = oem_log_get_recent_message(Msg->Payload.msgId,
                                     buffer,
                                     Msg->Payload.offset,
                                     GPS_MISSION_REPORT_DATA_SIZE,
                                     &copied);
    if (ret != OEM_OK) {
        copied = 0;
        GPS_AppData.ErrCounter++;
        OemCmdFailed("GetRecentMessage", ret);
    }
    
    GPS_SendReport(Msg, buffer, copied > GPS_MISSION_REPORT_DATA_SIZE ? GPS_MISSION_REPORT_DATA_SIZE : copied, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Log_RejectMissingCrc(const GPS_OEM_Log_RejectMissingCrc_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_log_reject_missing_crc(Msg->Payload.msgId);
    if (ret != OEM_OK) {
        GPS_AppData.ErrCounter++;
        OemCmdFailed("RejectMissingCrc", ret);
    }
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

void GPS_OEM_Log_IgnoreMissingCrc(const GPS_OEM_Log_IgnoreMissingCrc_t* Msg)
{
    int ret;

    GPS_AppData.CmdCounter++;

    ret = oem_log_ignore_missing_crc(Msg->Payload.msgId);
    if (ret != OEM_OK)
        GPS_AppData.ErrCounter++;
    
    GPS_SendReport(Msg, NULL, 0, ret, GPS_MISSION_REPORT_RETTYPE_HW);
}

/**
 * The driver exposes no handler lock: oem_log_lock_handlers() and its
 * counterpart are commented out in oem_log.h. Both commands therefore
 * validate the key and then refuse, so the ground learns that the lock did
 * not happen instead of reading silence as success.
 */
static void LockCommand(const CFE_MSG_CommandHeader_t* Header, uint32 Magic, uint32 Expected)
{
    GPS_AppData.ErrCounter++;

    if (Magic != Expected) {
        CFE_EVS_SendEvent(GPS_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "GPS: handler lock command rejected: bad key");
        GPS_SendReport(Header, NULL, 0, GPS_ERR_ARG, GPS_MISSION_REPORT_RETTYPE_APP);
        return;
    }

    CFE_EVS_SendEvent(GPS_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                      "GPS: handler lock is not implemented by the driver");
    GPS_SendReport(Header, NULL, 0, GPS_ERR_UNIMPL, GPS_MISSION_REPORT_RETTYPE_APP);
}

void GPS_OEM_Log_LockHandlers(const GPS_OEM_Log_LockHandlers_t* Msg)
{
    GPS_AppData.CmdCounter++;

    LockCommand(&Msg->CommandHeader, Msg->Payload.Magic, GPS_OEM_LOG_LOCK_MAGIC);
}

void GPS_OEM_Log_UnlockHandlers(const GPS_OEM_Log_UnlockHandlers_t* Msg)
{
    GPS_AppData.CmdCounter++;

    LockCommand(&Msg->CommandHeader, Msg->Payload.Magic, GPS_OEM_LOG_UNLOCK_MAGIC);
}
