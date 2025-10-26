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
#include "sant_cmds.h"
#include "sant_msgids.h"
#include "sant_eventids.h"
#include "sant_utils.h"
#include "sant_msg.h"
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
CFE_Status_t SANT_SendHkCmd(const SANT_SendHkCmd_t *Msg)
{
    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(SANT_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(SANT_Data.HkTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

CFE_Status_t SANT_SendBcnCmd(const SANT_SendBcnCmd_t *Msg)
{
    /**
     * Send beacon telemetry packet...
     */
    gs_gssb_ar6_release_status_t Reply;

    if(gs_gssb_ar6_get_release_status(SANT_I2C_ADDR, SANT_I2C_TIMEOUT_MS, &Reply) != GS_OK) {
        SANT_Data.BcnTlm.Payload.DeployStatus = 0xFF; /* Error Indicator */
    }
    else SANT_Data.BcnTlm.Payload.DeployStatus = Reply.status;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(SANT_Data.BcnTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(SANT_Data.BcnTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

CFE_Status_t SANT_SendOpCmd(const SANT_SendOpCmd_t *Msg)
{
    /*
    ** Send housekeeping telemetry packet...
    */
    OS_printf("SANT Operation tlm requests.\n");
    CFE_Status_t gs_st;
    gs_gssb_ar6_release_status_t release_status;

    gs_st = gs_gssb_ar6_get_release_status(SANT_I2C_ADDR, SANT_I2C_TIMEOUT_MS, &release_status);

    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(SANT_GET_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                            "Get release status failed, gs_err=0x%02X", gs_st);
        SANT_Data.ErrCounter++;
    }
    OS_printf("State: %u || Status: %u || Burn time left: %u || Burn tries: %u\n",
                    release_status.state, release_status.status, release_status.burn_time_left, release_status.burn_tries);

    CFE_MSG_Init(CFE_MSG_PTR(SANT_Data.OperationTlm.TelemetryHeader), CFE_SB_ValueToMsgId(SANT_OP_TLM_MID), sizeof(SANT_Data.OperationTlm));
    memcpy(&SANT_Data.OperationTlm.Payload, &release_status, sizeof(SANT_Data.OperationTlm.Payload));
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(SANT_Data.OperationTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(SANT_Data.OperationTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SANT NOOP commands                                                         */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t SANT_NoopCmd(const SANT_NoopCmd_t *Msg)
{
    SANT_Data.CmdCounter++;

    uint8_t Cmds[2] = {SANT_Data.CmdCounter, SANT_Data.ErrCounter};

    SANT_HandleReport(CFE_SUCCESS, SANT_NOOP_CC, Cmds, sizeof(Cmds));

    CFE_EVS_SendEvent(SANT_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "SANT: NOOP command received.");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t SANT_ResetCountersCmd(const SANT_ResetCountersCmd_t *Msg)
{
    SANT_Data.CmdCounter = 0;
    SANT_Data.ErrCounter = 0;

    uint8_t Cmds[2] = {SANT_Data.CmdCounter, SANT_Data.ErrCounter};

    SANT_HandleReport(CFE_SUCCESS, SANT_RESET_COUNTERS_CC, Cmds, sizeof(Cmds));

    CFE_EVS_SendEvent(SANT_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "SANT: RESET command");

    return CFE_SUCCESS;
}

CFE_Status_t SANT_SoftRebootCmd(const SANT_SoftRebootCmd_t *Msg)
{
    SANT_Data.CmdCounter++;
    CFE_Status_t gs_st;

    gs_st = gs_gssb_soft_reset(SANT_I2C_ADDR, SANT_I2C_TIMEOUT_MS);

    SANT_HandleReport(gs_st, SANT_SOFT_REBOOT_CC, NULL, 0);

    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(SANT_I2C_XFER_ERR_EID, CFE_EVS_EventType_ERROR,
                          "AM2150-O soft-reset failed, gs_err=0x%02X", gs_st);
        SANT_Data.ErrCounter++;
    }

    return gs_st;
}

CFE_Status_t SANT_BurnCmd(const SANT_BurnCmd_t *Msg)
{
    SANT_Data.CmdCounter++;
    CFE_Status_t gs_st;

    if (Msg->Duration > 60)
    {
        CFE_EVS_SendEvent(SANT_BURN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Burn param out of range (dur=%u)", Msg->Duration);
        SANT_Data.ErrCounter++;
    }

    gs_st = gs_gssb_ar6_burn(SANT_I2C_ADDR, SANT_I2C_TIMEOUT_MS, Msg->Duration);

    SANT_HandleReport(gs_st, SANT_BURN_CC, NULL, 0);

    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(SANT_BURN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Burn failed, gs_err=0x%02X", gs_st);
        SANT_Data.ErrCounter++;
    }

    return gs_st;
}

CFE_Status_t SANT_StopBurnCmd(const SANT_StopBurnCmd_t *Msg)
{
    SANT_Data.CmdCounter++;
    CFE_Status_t gs_st;

    gs_st = gs_gssb_ar6_stop_burn(SANT_I2C_ADDR, SANT_I2C_TIMEOUT_MS);

    SANT_HandleReport(gs_st, SANT_STOP_BURN_CC, NULL, 0);

    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(SANT_STOP_BURN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Stop-burn failed, gs_err=0x%02X", gs_st);
        SANT_Data.ErrCounter++;
    }

    return gs_st;
}

CFE_Status_t SANT_GetBoardStatusCmd(const SANT_GetBoardStatusCmd_t *Msg)
{
    SANT_Data.CmdCounter++;
    CFE_Status_t gs_st;
    gs_gssb_board_status_t board_status;

    gs_st = gs_gssb_ar6_get_board_status(SANT_I2C_ADDR, SANT_I2C_TIMEOUT_MS, &board_status);

    SANT_HandleReport(gs_st, SANT_GET_BOARD_STATUS_CC, &board_status, sizeof(board_status));

    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(SANT_GET_BOARD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Get board status failed, gs_err=0x%02X", gs_st);
        SANT_Data.ErrCounter++;
    }

    return gs_st;
}

CFE_Status_t SANT_GetTemperatureCmd(const SANT_GetTemperatureCmd_t *Msg)
{
    SANT_Data.CmdCounter++;
    CFE_Status_t gs_st;

    int16_t temperature;

    gs_st = gs_gssb_ar6_get_internal_temp(SANT_I2C_ADDR, SANT_I2C_TIMEOUT_MS, &temperature);

    SANT_HandleReport(gs_st, SANT_GET_TEMPERATURE_CC, &temperature, sizeof(temperature));

    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(SANT_GET_TEMP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Get temperature failed, gs_err=0x%02X", gs_st);
        SANT_Data.ErrCounter++;
    }

    return gs_st;
}

CFE_Status_t SANT_GetStatusCmd(const SANT_GetStatusCmd_t *Msg)
{
    SANT_Data.CmdCounter++;
    CFE_Status_t gs_st;
    gs_gssb_ar6_release_status_t release_status;

    gs_st = gs_gssb_ar6_get_release_status(SANT_I2C_ADDR, SANT_I2C_TIMEOUT_MS, &release_status);

    SANT_HandleReport(gs_st, SANT_GET_STATUS_CC, &release_status, sizeof(release_status));

    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(SANT_GET_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Get release status failed, gs_err=0x%02X", gs_st);
        SANT_Data.ErrCounter++;
    }
    else OS_printf("State: %u || Status: %u || Burn time left: %u || Burn tries: %u\n",
                    release_status.state, release_status.status, release_status.burn_time_left, release_status.burn_tries);

    return gs_st;
}

CFE_Status_t SANT_GetBackupStatusCmd(const SANT_GetBackupStatusCmd_t *Msg)
{
    SANT_Data.CmdCounter++;
    CFE_Status_t gs_st;
    gs_gssb_backup_status_t backup_status;
 
    gs_st = gs_gssb_ar6_get_backup_status(SANT_I2C_ADDR, SANT_I2C_TIMEOUT_MS, &backup_status);

    SANT_HandleReport(gs_st, SANT_GET_BACKUP_STATUS_CC, &backup_status, sizeof(backup_status));

    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(SANT_GET_BACKUP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Get backup status failed, gs_err=0x%02X", gs_st);
        SANT_Data.ErrCounter++;
    }

    return gs_st;
}

CFE_Status_t SANT_GetBackupSettingsCmd(const SANT_GetSettingsCmd_t *Msg)
{
    SANT_Data.CmdCounter++;
    CFE_Status_t gs_st;
    gs_gssb_backup_settings_t cfg;

    gs_st = gs_gssb_ar6_get_backup_settings(SANT_I2C_ADDR, SANT_I2C_TIMEOUT_MS, &cfg);

    SANT_HandleReport(gs_st, SANT_GET_SETTINGS_CC, &cfg, sizeof(cfg));

    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(SANT_GET_SETTINGS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "get_backup_settings fail, gs_err=0x%02X", gs_st);
        SANT_Data.ErrCounter++;
    }

    return gs_st;
}

CFE_Status_t SANT_SetBackupSettingsCmd(const SANT_SetSettingsCmd_t *Msg)
{
    SANT_Data.CmdCounter++;
    CFE_Status_t gs_st;
    gs_gssb_backup_settings_t cfg = {
        .minutes           = Msg->MinutesUntilDeploy,
        .backup_active     = Msg->BackupActive,
        .max_burn_duration = Msg->MaxBurnDuration
    };

    /* 범위 검사 */
    if (cfg.minutes > 5000 || cfg.max_burn_duration > 60 || cfg.backup_active > 1)
    {
        CFE_EVS_SendEvent(SANT_SET_SETTINGS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "backup_cfg param out of range");
        SANT_Data.ErrCounter++;
    }

    gs_st = gs_gssb_ar6_set_backup_settings(SANT_I2C_ADDR, SANT_I2C_TIMEOUT_MS, cfg);

    SANT_HandleReport(gs_st, SANT_SET_SETTINGS_CC, NULL, 0);

    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(SANT_SET_SETTINGS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "set_backup_settings fail, gs_err=0x%02X", gs_st);
        SANT_Data.ErrCounter++;
    }

    return gs_st;
}