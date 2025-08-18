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
#include "sant_app.h"
#include "sant_app_cmds.h"
#include "sant_app_msgids.h"
#include "sant_app_eventids.h"
#include "sant_app_utils.h"
#include "sant_app_msg.h"
#include <gs/gssb/gssb_ar6.h>

#include <gs/gssb/internal/gssb_common.h>   /* gs_gssb_common_* 함수 선언 */
#include <gs/gssb/internal/gssb_cmd_id.h>   /* GSSB_CMD_ 열거형 */
#include <gs/gssb/gssb_autodeploy.h>

// !!!! Most of the cmds are in the device directory 
/* The sant_lib module provides the SANT_Function() prototype */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t SANT_APP_SendHkCmd(const SANT_APP_SendHkCmd_t *Msg)
{
    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(SANT_APP_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(SANT_APP_Data.HkTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

CFE_Status_t SANT_APP_SendBcnCmd(const SANT_APP_SendBcnCmd_t *Msg)
{
    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(SANT_APP_Data.BcnTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(SANT_APP_Data.BcnTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

CFE_Status_t SANT_APP_SendOpCmd(const SANT_APP_SendOpCmd_t *Msg)
{
    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(SANT_APP_Data.OperationTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(SANT_APP_Data.OperationTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SANT NOOP commands                                                         */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t SANT_APP_NoopCmd(const SANT_APP_NoopCmd_t *Msg)
{
    SANT_APP_Data.CmdCounter++;

    CFE_EVS_SendEvent(SANT_APP_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "SANT: NOOP command received.");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t SANT_APP_ResetCountersCmd(const SANT_APP_ResetCountersCmd_t *Msg)
{
    SANT_APP_Data.CmdCounter = 0;
    SANT_APP_Data.ErrCounter = 0;

    CFE_EVS_SendEvent(SANT_APP_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "SANT: RESET command");

    return CFE_SUCCESS;
}

CFE_Status_t SANT_APP_SoftRebootCmd(const SANT_APP_SoftRebootCmd_t *Msg)
{
    SANT_APP_Data.CmdCounter++;
    CFE_Status_t gs_st;
    uint8_t SANT_I2C_ADDR = 0x05;

    gs_st = gs_gssb_soft_reset(SANT_I2C_ADDR, SANT_I2C_TIMEOUT_MS);
    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(SANT_APP_I2C_XFER_ERR_EID, CFE_EVS_EventType_ERROR,
                          "AM2150-O soft-reset failed, gs_err=0x%02X", gs_st);
        SANT_APP_Data.ErrCounter++;
    }

    return gs_st;
}

CFE_Status_t SANT_APP_BurnCmd(const SANT_APP_BurnCmd_t *Msg)
{
    SANT_APP_Data.CmdCounter++;
    CFE_Status_t gs_st;

    if (Msg->Duration > 60)
    {
        CFE_EVS_SendEvent(SANT_APP_BURN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Burn param out of range (dur=%u)", Msg->Duration);
        SANT_APP_Data.ErrCounter++;
    }

    uint8_t SANT_I2C_ADDR = 0x05;
    gs_st = gs_gssb_ar6_burn(SANT_I2C_ADDR, SANT_I2C_TIMEOUT_MS, Msg->Duration);
    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(SANT_APP_BURN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Burn failed, gs_err=0x%02X", gs_st);
        SANT_APP_Data.ErrCounter++;
    }

    return gs_st;
}

CFE_Status_t SANT_APP_StopBurnCmd(const SANT_APP_StopBurnCmd_t *Msg)
{
    SANT_APP_Data.CmdCounter++;
    CFE_Status_t gs_st;

    uint8_t SANT_I2C_ADDR = 0x05;
    gs_st = gs_gssb_ar6_stop_burn(SANT_I2C_ADDR, SANT_I2C_TIMEOUT_MS);
    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(SANT_APP_STOP_BURN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Stop-burn failed, gs_err=0x%02X", gs_st);
        SANT_APP_Data.ErrCounter++;
    }

    return gs_st;
}

CFE_Status_t SANT_APP_GetBoardStatusCmd(const SANT_APP_GetBoardStatusCmd_t *Msg)
{
    SANT_APP_Data.CmdCounter++;
    CFE_Status_t gs_st;
    gs_gssb_board_status_t board_status;

    uint8_t SANT_I2C_ADDR = 0x05;
    gs_st = gs_gssb_ar6_get_board_status(SANT_I2C_ADDR, SANT_I2C_TIMEOUT_MS, &board_status);
    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(SANT_APP_GET_BOARD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Get board status failed, gs_err=0x%02X", gs_st);
        SANT_APP_Data.ErrCounter++;
    }

    return gs_st;
}

CFE_Status_t SANT_APP_GetTemperatureCmd(const SANT_APP_GetTemperatureCmd_t *Msg)
{
    SANT_APP_Data.CmdCounter++;
    CFE_Status_t gs_st;

    int16_t temperature;

    uint8_t SANT_I2C_ADDR = 0x05;
    gs_st = gs_gssb_ar6_get_internal_temp(SANT_I2C_ADDR, SANT_I2C_TIMEOUT_MS, &temperature);
    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(SANT_APP_GET_TEMP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Get temperature failed, gs_err=0x%02X", gs_st);
        SANT_APP_Data.ErrCounter++;
    }

    return gs_st;
}

CFE_Status_t SANT_APP_GetStatusCmd(const SANT_APP_GetStatusCmd_t *Msg)
{
    SANT_APP_Data.CmdCounter++;
    CFE_Status_t gs_st;
    gs_gssb_ar6_release_status_t release_status;

    uint8_t SANT_I2C_ADDR = 0x05;
    gs_st = gs_gssb_ar6_get_release_status(SANT_I2C_ADDR, SANT_I2C_TIMEOUT_MS, &release_status);
    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(SANT_APP_GET_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Get release status failed, gs_err=0x%02X", gs_st);
        SANT_APP_Data.ErrCounter++;
    }

    return gs_st;
}

CFE_Status_t SANT_APP_GetBackupStatusCmd(const SANT_APP_GetBackupStatusCmd_t *Msg)
{
    SANT_APP_Data.CmdCounter++;
    CFE_Status_t gs_st;
    gs_gssb_backup_status_t backup_status;

    uint8_t SANT_I2C_ADDR = 0x05; 
    gs_st = gs_gssb_ar6_get_backup_status(SANT_I2C_ADDR, SANT_I2C_TIMEOUT_MS, &backup_status);
    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(SANT_APP_GET_BACKUP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Get backup status failed, gs_err=0x%02X", gs_st);
        SANT_APP_Data.ErrCounter++;
    }

    return gs_st;
}

CFE_Status_t SANT_APP_GetBackupSettingsCmd(const SANT_APP_GetSettingsCmd_t *Msg)
{
    SANT_APP_Data.CmdCounter++;
    CFE_Status_t gs_st;
    gs_gssb_backup_settings_t cfg;

    uint8_t SANT_I2C_ADDR = 0x05;
    gs_st = gs_gssb_ar6_get_backup_settings(SANT_I2C_ADDR, SANT_I2C_TIMEOUT_MS, &cfg);
    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(SANT_APP_GET_SETTINGS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "get_backup_settings fail, gs_err=0x%02X", gs_st);
        SANT_APP_Data.ErrCounter++;
    }

    return gs_st;
}

CFE_Status_t SANT_APP_SetBackupSettingsCmd(const SANT_APP_SetSettingsCmd_t *Msg)
{
    SANT_APP_Data.CmdCounter++;
    CFE_Status_t gs_st;
    gs_gssb_backup_settings_t cfg = {
        .minutes           = Msg->MinutesUntilDeploy,
        .backup_active     = Msg->BackupActive,
        .max_burn_duration = Msg->MaxBurnDuration
    };

    /* 범위 검사 */
    if (cfg.minutes > 5000 || cfg.max_burn_duration > 60 || cfg.backup_active > 1)
    {
        CFE_EVS_SendEvent(SANT_APP_SET_SETTINGS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "backup_cfg param out of range");
        SANT_APP_Data.ErrCounter++;
    }

    uint8_t SANT_I2C_ADDR = 0x05;
    gs_st = gs_gssb_ar6_set_backup_settings(SANT_I2C_ADDR, SANT_I2C_TIMEOUT_MS, cfg);
    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(SANT_APP_SET_SETTINGS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "set_backup_settings fail, gs_err=0x%02X", gs_st);
        SANT_APP_Data.ErrCounter++;
    }

    return gs_st;
}