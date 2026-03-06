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
#include "uant_app.h"
#include "uant_app_cmds.h"
#include "uant_app_msgids.h"
#include "uant_app_eventids.h"
#include "uant_app_version.h"
#include "uant_app_tbl.h"
#include "uant_app_utils.h"
#include "uant_app_msg.h"
#include <gs/gssb/gssb_ant6.h>

#include <gs/gssb/internal/gssb_common.h>   /* gs_gssb_common_* 함수 선언 */
#include <gs/gssb/internal/gssb_cmd_id.h>   /* GSSB_CMD_ 열거형 */
#include <gs/gssb/gssb_autodeploy.h>
// !!!! Most of the cmds are in the device directory 
/* The uant_lib module provides the UANT_Function() prototype */
//#include "uant_lib.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */

CFE_Status_t UANT_APP_SendBcnCmd(const UANT_APP_SendBcnCmd_t *Msg)
{
    (void)Msg; /* 헤더만 있는 명령 */

    const uint8 ADDR_A = 0x05; /* 보드 A */
    const uint8 ADDR_B = 0x06; /* 보드 B */

    /* 원천 데이터 */
    gs_gssb_ant6_release_status_t relA = (gs_gssb_ant6_release_status_t){0};
    gs_gssb_ant6_release_status_t relB = (gs_gssb_ant6_release_status_t){0};
    gs_gssb_backup_settings_t      bksA = (gs_gssb_backup_settings_t){0};
    gs_gssb_backup_settings_t      bksB = (gs_gssb_backup_settings_t){0};

    /* 보드 A 읽기 */
    bool ok_relA = (gs_gssb_ant6_get_release_status(ADDR_A, UANT_I2C_TIMEOUT_MS, &relA) == GS_OK);
    bool ok_bksA = (gs_gssb_ant6_get_backup_settings(ADDR_A, UANT_I2C_TIMEOUT_MS, &bksA) == GS_OK);
    // bool ok_bstA = (gs_gssb_ant6_get_backup_status (ADDR_A, UANT_I2C_TIMEOUT_MS, &bstA) == GS_OK);

    /* 보드 B 읽기 */
    bool ok_relB = (gs_gssb_ant6_get_release_status(ADDR_B, UANT_I2C_TIMEOUT_MS, &relB) == GS_OK);
    bool ok_bksB = (gs_gssb_ant6_get_backup_settings(ADDR_B, UANT_I2C_TIMEOUT_MS, &bksB) == GS_OK);
    // bool ok_bstB = (gs_gssb_ant6_get_backup_status (ADDR_B, UANT_I2C_TIMEOUT_MS, &bstB) == GS_OK);

    /* 하나라도 실패하면 공통 에러 한 번만 기록 */
    if (!(ok_relA && ok_bksA && ok_relB && ok_bksB)) {
        CFE_EVS_SendEvent(UANT_APP_BCN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Beacon read error (A=0x%02X, B=0x%02X)", ADDR_A, ADDR_B);
        UANT_APP_Data.ErrCounter++;
    }

    /* Beacon 채우기: 각 보드에서 ch0/ch1 모두, 그리고 backup/state */
    /* 보드 A (0x05) */
    UANT_APP_Data.bcn.Payload.ch0_status_A    = ok_relA ? relA.channel_0_status : (uint8)GS_ERROR_NO_DATA;
    UANT_APP_Data.bcn.Payload.ch1_status_A    = ok_relA ? relA.channel_1_status : (uint8)GS_ERROR_NO_DATA;
    UANT_APP_Data.bcn.Payload.backup_active_A = ok_bksA ? bksA.backup_active    : (uint8)GS_ERROR_NO_DATA;
    // UANT_APP_Data.bcn.state_A         = ok_bstA ? bstA.state            : (uint8)GS_ERROR_NO_DATA;

    /* 보드 B (0x06) */
    UANT_APP_Data.bcn.Payload.ch0_status_B    = ok_relB ? relB.channel_0_status : (uint8)GS_ERROR_NO_DATA;
    UANT_APP_Data.bcn.Payload.ch1_status_B    = ok_relB ? relB.channel_1_status : (uint8)GS_ERROR_NO_DATA;
    UANT_APP_Data.bcn.Payload.backup_active_B = ok_bksB ? bksB.backup_active    : (uint8)GS_ERROR_NO_DATA;
    // UANT_APP_Data.bcn.state_B         = ok_bstB ? bstB.state            : (uint8)GS_ERROR_NO_DATA;

    /* 전송 */
    CFE_MSG_SetSize(CFE_MSG_PTR(UANT_APP_Data.bcn.TelemetryHeader), sizeof(UANT_APP_bcnTlm_t));
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_APP_Data.bcn.TelemetryHeader));
    (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_APP_Data.bcn.TelemetryHeader), true);

    return CFE_SUCCESS;
}





CFE_Status_t UANT_APP_SendHkCmd(const UANT_APP_SendHkCmd_t *Msg)
{
    (void)Msg;

    const uint8 ADDR_A = 0x05;
    const uint8 ADDR_B = 0x06;

    gs_gssb_ant6_release_status_t relA = {0}, relB = {0};
    gs_gssb_backup_settings_t     cfgA = {0}, cfgB = {0};
    gs_gssb_backup_status_t       bstA = {0}, bstB = {0};
    gs_gssb_board_status_t        brdA = {0}, brdB = {0};

    /* 읽기 */
    bool ok_relA = (gs_gssb_ant6_get_release_status(ADDR_A, UANT_I2C_TIMEOUT_MS, &relA) == GS_OK);
    bool ok_cfgA = (gs_gssb_ant6_get_backup_settings(ADDR_A, UANT_I2C_TIMEOUT_MS, &cfgA) == GS_OK);
    bool ok_bstA = (gs_gssb_ant6_get_backup_status (ADDR_A, UANT_I2C_TIMEOUT_MS, &bstA) == GS_OK);
    bool ok_brdA = (gs_gssb_ant6_get_board_status  (ADDR_A, UANT_I2C_TIMEOUT_MS, &brdA) == GS_OK);

    bool ok_relB = (gs_gssb_ant6_get_release_status(ADDR_B, UANT_I2C_TIMEOUT_MS, &relB) == GS_OK);
    bool ok_cfgB = (gs_gssb_ant6_get_backup_settings(ADDR_B, UANT_I2C_TIMEOUT_MS, &cfgB) == GS_OK);
    bool ok_bstB = (gs_gssb_ant6_get_backup_status (ADDR_B, UANT_I2C_TIMEOUT_MS, &bstB) == GS_OK);
    bool ok_brdB = (gs_gssb_ant6_get_board_status  (ADDR_B, UANT_I2C_TIMEOUT_MS, &brdB) == GS_OK);

    if (!(ok_relA && ok_cfgA && ok_bstA && ok_brdA &&
          ok_relB && ok_cfgB && ok_bstB && ok_brdB))
    {
        CFE_EVS_SendEvent(UANT_APP_SUB_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                          "HK read error (A=0x%02X ok=%d%d%d%d, B=0x%02X ok=%d%d%d%d)",
                          ADDR_A, ok_relA, ok_cfgA, ok_bstA, ok_brdA,
                          ADDR_B, ok_relB, ok_cfgB, ok_bstB, ok_brdB);
        UANT_APP_Data.ErrCounter++;
    }
    else
    {
        UANT_APP_Data.CmdCounter++;
    }

    /* Board A */
    UANT_APP_Data.HkTlm.ch0_status_A     = ok_relA ? relA.channel_0_status     : (uint8)GS_ERROR_NO_DATA;
    UANT_APP_Data.HkTlm.ch1_status_A     = ok_relA ? relA.channel_1_status     : (uint8)GS_ERROR_NO_DATA;
    UANT_APP_Data.HkTlm.backup_active_A  = ok_cfgA ? cfgA.backup_active        : (uint8)GS_ERROR_NO_DATA;
    UANT_APP_Data.HkTlm.state_A          = ok_bstA ? bstA.state                : (uint8)GS_ERROR_NO_DATA;
    UANT_APP_Data.HkTlm.ch0_burn_tries_A = ok_relA ? relA.channel_0_burn_tries : (uint8)GS_ERROR_NO_DATA;
    UANT_APP_Data.HkTlm.ch1_burn_tries_A = ok_relA ? relA.channel_1_burn_tries : (uint8)GS_ERROR_NO_DATA;
    UANT_APP_Data.HkTlm.reboot_count_A   = ok_brdA ? brdA.reboot_count         : (uint8)GS_ERROR_NO_DATA;

    /* Board B */
    UANT_APP_Data.HkTlm.ch0_status_B     = ok_relB ? relB.channel_0_status     : (uint8)GS_ERROR_NO_DATA;
    UANT_APP_Data.HkTlm.ch1_status_B     = ok_relB ? relB.channel_1_status     : (uint8)GS_ERROR_NO_DATA;
    UANT_APP_Data.HkTlm.backup_active_B  = ok_cfgB ? cfgB.backup_active        : (uint8)GS_ERROR_NO_DATA;
    UANT_APP_Data.HkTlm.state_B          = ok_bstB ? bstB.state                : (uint8)GS_ERROR_NO_DATA;
    UANT_APP_Data.HkTlm.ch0_burn_tries_B = ok_relB ? relB.channel_0_burn_tries : (uint8)GS_ERROR_NO_DATA;
    UANT_APP_Data.HkTlm.ch1_burn_tries_B = ok_relB ? relB.channel_1_burn_tries : (uint8)GS_ERROR_NO_DATA;
    UANT_APP_Data.HkTlm.reboot_count_B   = ok_brdB ? brdB.reboot_count         : (uint8)GS_ERROR_NO_DATA;

    /* 송신 */
    CFE_SB_TimeStampMsg(&UANT_APP_Data.HkTlm.TelemetryHeader.Msg);
    CFE_SB_TransmitMsg(&UANT_APP_Data.HkTlm.TelemetryHeader.Msg, true);

    return CFE_SUCCESS;
}




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* UANT NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t UANT_APP_NoopCmd(const UANT_APP_NoopCmd_t *Msg)
{
    UANT_APP_Data.CmdCounter++;

    CFE_EVS_SendEvent(UANT_APP_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "UANT: NOOP command %s",
                      UANT_APP_VERSION);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t UANT_APP_ResetCountersCmd(const UANT_APP_ResetCountersCmd_t *Msg)
{
    UANT_APP_Data.CmdCounter = 0;
    UANT_APP_Data.ErrCounter = 0;

    CFE_EVS_SendEvent(UANT_APP_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "UANT: RESET command");

    return CFE_SUCCESS;
}

gs_error_t UANT_APP_SoftReboot(const UANT_APP_SoftRebootCmd_t *Msg)
{
    gs_error_t gs_st = gs_gssb_soft_reset(Msg->Addr, UANT_I2C_TIMEOUT_MS);

    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(UANT_APP_I2C_XFER_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ANT-6F soft-reset failed, gs_err=0x%02X", gs_st);
        UANT_APP_Data.ErrCounter++;
    }
    else
    {
        UANT_APP_Data.CmdCounter++;
    }

    /* RPT 데이터 구성: 전체를 0으로 초기화 후 필요한 필드만 설정 */
    RPT_Report_t report = (RPT_Report_t){0};
    report.MsgID         = UANT_APP_CMD_MID;
    report.CommandCode   = UANT_APP_SOFT_REBOOT_CC;
    report.ReturnCode    = (int32)gs_st;
    report.ReturnDataSize = 0;
    report.ReturnType    = (gs_st == GS_OK) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_LIB;
    /* report.ReturnValue 등 나머지 버퍼 필드는 위의 0 초기화로 모두 0 */

    /* 전역 RPT 패킷으로 전송 */
    UANT_APP_Data.rpt.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_APP_Data.rpt.TelemetryHeader));
    (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_APP_Data.rpt.TelemetryHeader), true);

    return gs_st;
}



gs_error_t UANT_APP_BurnChannel(const UANT_APP_BurnChannelCmd_t *Msg)
{
    gs_error_t gs_st = gs_gssb_ant6_burn_channel(Msg->Addr,
                                                 UANT_I2C_TIMEOUT_MS,
                                                 Msg->Channel,
                                                 Msg->Duration);

    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(UANT_APP_BURN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Burn failed, gs_err=0x%02X", gs_st);
        UANT_APP_Data.ErrCounter++;
    }
    else
    {
        UANT_APP_Data.CmdCounter++;
    }

    /* RPT 데이터 구성: 전체를 0으로 초기화 후 필요한 필드만 설정 */
    RPT_Report_t report = (RPT_Report_t){0};
    report.MsgID          = UANT_APP_CMD_MID;
    report.CommandCode    = UANT_APP_BURN_CHANNEL_CC;
    report.ReturnCode     = (int32)gs_st;
    report.ReturnDataSize = 0;
    report.ReturnType     = (gs_st == GS_OK) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_LIB;
    /* report.ReturnValue 등 나머지 버퍼 필드는 위의 0 초기화로 모두 0 */

    /* 전역 RPT 패킷으로 전송 */
    UANT_APP_Data.rpt.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_APP_Data.rpt.TelemetryHeader));
    (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_APP_Data.rpt.TelemetryHeader), true);

    return gs_st;
}

gs_error_t UANT_APP_BurnChannelInternal(const UANT_APP_BurnChannelCmd_t *Msg)
{
    OS_printf("%s: UANT Burn cmd received.\n", __func__);
    gs_error_t gs_st = gs_gssb_ant6_burn_channel(Msg->Addr,
                                                 UANT_I2C_TIMEOUT_MS,
                                                 Msg->Channel,
                                                 Msg->Duration);

    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(UANT_APP_BURN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Burn failed, gs_err=0x%02X", gs_st);
        UANT_APP_Data.ErrCounter++;
    }
    else
    {
        UANT_APP_Data.CmdCounter++;
    }

    return gs_st;
}



gs_error_t UANT_APP_StopBurn(const UANT_APP_StopBurnCmd_t *Msg)
{
    gs_error_t gs_st = gs_gssb_common_stop_burn(Msg->Addr, UANT_I2C_TIMEOUT_MS);

    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(UANT_APP_STOP_BURN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Stop-burn failed, gs_err=0x%02X", gs_st);
        UANT_APP_Data.ErrCounter++;
    }
    else
    {
        UANT_APP_Data.CmdCounter++;
    }

    /* RPT 데이터 구성: 전체를 0으로 초기화 후 필요한 필드만 설정 */
    RPT_Report_t report = (RPT_Report_t){0};
    report.MsgID          = UANT_APP_CMD_MID;
    report.CommandCode    = UANT_APP_STOP_BURN_CC;
    report.ReturnCode     = (int32)gs_st;
    report.ReturnDataSize = 0;
    report.ReturnType     = (gs_st == GS_OK) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_LIB;
    /* report.ReturnValue 등 나머지 버퍼 필드는 위의 0 초기화로 모두 0 */

    /* 전역 RPT 패킷으로 전송 */
    UANT_APP_Data.rpt.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_APP_Data.rpt.TelemetryHeader));
    (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_APP_Data.rpt.TelemetryHeader), true);

    return gs_st;
}



gs_error_t UANT_APP_AutoDeploy(const UANT_APP_AutodeployCmd_t *Msg)
{
    int16 gs_st = gs_autodeploy_six_u(Msg->SecondsDelay,
                                      Msg->AddrAnt6_0,
                                      Msg->AddrAnt6_1,
                                      0x00, 0x00, 0x00, 0x00);

    switch (gs_st)
    {
        case STARTING:
            CFE_EVS_SendEvent(UANT_APP_AUTODEPLOY_START_EID, CFE_EVS_EventType_INFORMATION,
                              "ANT-6 autodeploy START (delay=%u s)", Msg->SecondsDelay);
            UANT_APP_Data.CmdCounter++;
            break;

        case RUNNING:
            CFE_EVS_SendEvent(UANT_APP_AUTODEPLOY_RUN_EID, CFE_EVS_EventType_DEBUG,
                              "ANT-6 autodeploy already RUNNING");
            break;

        case FINISHED_SUCCESSFULLY:
            CFE_EVS_SendEvent(UANT_APP_AUTODEPLOY_DONE_EID, CFE_EVS_EventType_INFORMATION,
                              "ANT-6 autodeploy FINISHED OK");
            UANT_APP_Data.CmdCounter++;
            break;

        default:
            CFE_EVS_SendEvent(UANT_APP_AUTODEPLOY_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ANT-6 autodeploy FAILED, state=%d", gs_st);
            UANT_APP_Data.ErrCounter++;
            break;
    }

    /* RPT 데이터 구성: 전체를 0으로 초기화 후 필요한 필드만 설정 */
    RPT_Report_t report = (RPT_Report_t){0};
    report.MsgID          = UANT_APP_CMD_MID;
    report.CommandCode    = UANT_APP_AUTODEPLOY_CC;
    report.ReturnCode     = (int32)gs_st;
    report.ReturnDataSize = 0;
    report.ReturnType     = ((gs_st == STARTING) || (gs_st == FINISHED_SUCCESSFULLY))
                                ? RPT_RETTYPE_SUCCESS
                                : RPT_RETTYPE_LIB;
    /* report.ReturnValue 등 나머지 버퍼 필드는 위의 0 초기화로 모두 0 */

    /* 전역 RPT 패킷으로 전송 */
    UANT_APP_Data.rpt.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_APP_Data.rpt.TelemetryHeader));
    (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_APP_Data.rpt.TelemetryHeader), true);

    return gs_st;
}




gs_error_t UANT_APP_GetBoardStatus(const UANT_APP_GetBoardStatusCmd_t *Msg)
{
    gs_error_t gs_st;
    gs_gssb_board_status_t bs;

    /* 보드 상태 조회 */
    gs_st = gs_gssb_ant6_get_board_status(Msg->Addr, UANT_I2C_TIMEOUT_MS, &bs);

    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(UANT_APP_GET_BOARD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "get_board_status fail, gs_err=0x%02X", gs_st);
        UANT_APP_Data.ErrCounter++;
    }
    else
    {
        UANT_APP_Data.CmdCounter++;
    }

    /* ===== RPT 전송 ===== */
    RPT_Report_t report = (RPT_Report_t){0};

    report.MsgID       = UANT_APP_CMD_MID;
    report.CommandCode = UANT_APP_GET_BOARD_STATUS_CC;
    report.ReturnType  = (gs_st == GS_OK) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_LIB;
    report.ReturnCode  = gs_st;
    report.ReturnDataSize = sizeof(bs);
    memcpy(report.ReturnValue, &bs, report.ReturnDataSize);
    

    /* 전역 RPT 패킷 전송 */
    UANT_APP_Data.rpt.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_APP_Data.rpt.TelemetryHeader));
    (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_APP_Data.rpt.TelemetryHeader), true);

    return gs_st;
}


gs_error_t UANT_APP_GetTemperature(const UANT_APP_GetTemperatureCmd_t *Msg)
{
    gs_error_t gs_st;
    int16_t temp;

    /* 내부 온도 조회 */
    gs_st = gs_gssb_ant6_get_internal_temp(Msg->Addr, UANT_I2C_TIMEOUT_MS, &temp);

    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(UANT_APP_GET_TEMP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "get_internal_temp fail, gs_err=0x%02X", gs_st);
        UANT_APP_Data.ErrCounter++;
    }
    else
    {
        UANT_APP_Data.CmdCounter++;
        
    }

    /* ===== RPT 전송 ===== */
    RPT_Report_t report = (RPT_Report_t){0};

    report.MsgID         = UANT_APP_CMD_MID;
    report.CommandCode   = UANT_APP_GET_TEMPERATURE_CC;
    report.ReturnType    = (gs_st == GS_OK) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_LIB;
    report.ReturnCode    = gs_st;
    report.ReturnDataSize = sizeof(temp);
    memcpy(report.ReturnValue, &temp, report.ReturnDataSize);

    /* 전역 RPT 패킷 전송 */
    UANT_APP_Data.rpt.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_APP_Data.rpt.TelemetryHeader));
    (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_APP_Data.rpt.TelemetryHeader), true);

    return gs_st;
}


gs_error_t UANT_APP_GetReleaseStatus(const UANT_APP_GetStatusCmd_t *Msg)
{
    gs_error_t gs_st;
    gs_gssb_ant6_release_status_t st;

    /* 릴리즈 상태 조회 */
    gs_st = gs_gssb_ant6_get_release_status(Msg->Addr, UANT_I2C_TIMEOUT_MS, &st);

    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(UANT_APP_GET_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "get_release_status fail, gs_err=0x%02X", gs_st);
        UANT_APP_Data.ErrCounter++;
    }
    else
    {
        UANT_APP_Data.CmdCounter++;
    }

    /* ===== RPT 전송 (간소화 형식) ===== */
    RPT_Report_t report = (RPT_Report_t){0};

    report.MsgID         = UANT_APP_CMD_MID;
    report.CommandCode   = UANT_APP_GET_STATUS_CC;
    report.ReturnType    = (gs_st == GS_OK) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_LIB;
    report.ReturnCode    = gs_st;
    report.ReturnDataSize = sizeof(st);
    memcpy(report.ReturnValue, &st, report.ReturnDataSize);

    UANT_APP_Data.rpt.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_APP_Data.rpt.TelemetryHeader));
    (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_APP_Data.rpt.TelemetryHeader), true);

    return gs_st;
}


gs_error_t UANT_APP_GetBackupStatus(const UANT_APP_GetBackupStatusCmd_t *Msg)
{
    gs_error_t gs_st;
    gs_gssb_backup_status_t bs;

    /* 백업 상태 조회 */
    gs_st = gs_gssb_ant6_get_backup_status(Msg->Addr, UANT_I2C_TIMEOUT_MS, &bs);

    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(UANT_APP_GET_BACKUP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "get_backup_status fail, gs_err=0x%02X", gs_st);
        UANT_APP_Data.ErrCounter++;
    }
    else
    {
        UANT_APP_Data.CmdCounter++;
    }

    /* ===== RPT 전송 ===== */
    RPT_Report_t report = (RPT_Report_t){0};

    report.MsgID         = UANT_APP_CMD_MID;
    report.CommandCode   = UANT_APP_GET_BACKUP_STATUS_CC;
    report.ReturnType    = (gs_st == GS_OK) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_LIB;
    report.ReturnCode    = gs_st;
    report.ReturnDataSize = sizeof(bs);
    memcpy(report.ReturnValue, &bs, report.ReturnDataSize);

    UANT_APP_Data.rpt.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_APP_Data.rpt.TelemetryHeader));
    (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_APP_Data.rpt.TelemetryHeader), true);

    return gs_st;
}


gs_error_t UANT_APP_GetBackupSettings(const UANT_APP_GetSettingsCmd_t *Msg)
{
    gs_error_t gs_st;
    gs_gssb_backup_settings_t cfg;

    /* 백업 설정 조회 */
    gs_st = gs_gssb_ant6_get_backup_settings(Msg->Addr, UANT_I2C_TIMEOUT_MS, &cfg);

    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(UANT_APP_GET_SETTINGS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "get_backup_settings fail, gs_err=0x%02X", gs_st);
        UANT_APP_Data.ErrCounter++;
    }
    else
    {
        UANT_APP_Data.CmdCounter++;
    }

    
    RPT_Report_t report = (RPT_Report_t){0};

    report.MsgID          = UANT_APP_CMD_MID;
    report.CommandCode    = UANT_APP_GET_SETTINGS_CC;
    report.ReturnType     = (gs_st == GS_OK) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_LIB;
    report.ReturnCode     = gs_st;
    report.ReturnDataSize = sizeof(cfg);
    memcpy(report.ReturnValue, &cfg, report.ReturnDataSize);

    UANT_APP_Data.rpt.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_APP_Data.rpt.TelemetryHeader));
    (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_APP_Data.rpt.TelemetryHeader), true);

    return gs_st;
}


gs_error_t UANT_APP_SetBackupSettings(const UANT_APP_SetSettingsCmd_t *Msg)
{
    gs_error_t gs_st;
    gs_gssb_backup_settings_t cfg = {
        .minutes           = Msg->MinutesUntilDeploy,
        .backup_active     = Msg->BackupActive,
        .max_burn_duration = Msg->MaxBurnDuration
    };

    /* 백업 설정 적용 */
    gs_st = gs_gssb_ant6_set_backup_settings(Msg->Addr, UANT_I2C_TIMEOUT_MS, cfg);

    if (gs_st != GS_OK)
    {
        CFE_EVS_SendEvent(UANT_APP_SET_SETTINGS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "set_backup_settings fail, gs_err=0x%02X", gs_st);
        UANT_APP_Data.ErrCounter++;
    }
    else
    {
        UANT_APP_Data.CmdCounter++;
    }

    /* ===== RPT 전송 (SET: ReturnValue 없음) ===== */
    RPT_Report_t report = (RPT_Report_t){0};

    report.MsgID          = UANT_APP_CMD_MID;
    report.CommandCode    = UANT_APP_SET_SETTINGS_CC;
    report.ReturnType     = (gs_st == GS_OK) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_LIB;
    report.ReturnCode     = gs_st;
    report.ReturnDataSize = 0;   /* set 함수: 반환 데이터 없음 */

    UANT_APP_Data.rpt.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_APP_Data.rpt.TelemetryHeader));
    (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_APP_Data.rpt.TelemetryHeader), true);

    return gs_st;
}

/**
 * @brief Simplified get deploy status function
 * @param address [in] Uant Board address, should be `0x05` or `0x06`
 * @param channel [in] Uant channel in specified Board. Should be `0` or `1`
 * @return `1` for deploy, `0` for not deploy. `-1` If error
 */
CFE_Status_t UANT_APP_IsReleased(const UANT_RequestRelease_t *Msg)
{
    OS_printf("%s: UANT get deploy status received.\n", __func__);
    gs_error_t gs_st;
    gs_gssb_ant6_release_status_t st;

    UANT_InternalTlm_t Tlm;
    CFE_MSG_Init(CFE_MSG_PTR(Tlm.TelemetryHeader), CFE_SB_ValueToMsgId(UANT_APP_INTERNAL_TLM_MID), sizeof(Tlm));
    
    /* 릴리즈 상태 조회 */
    gs_st = gs_gssb_ant6_get_release_status(Msg->Payload.address, UANT_I2C_TIMEOUT_MS, &st);

    /* If error, send negative val */
    if (gs_st != GS_OK) {
        OS_printf("%s: uant get status fail.\n", __func__);
        Tlm.Payload.IsDeploy = -1;
        goto transmit;
    }
    /* If OK, send specified deploy status */
    else {
        if (Msg->Payload.channel == 0) Tlm.Payload.IsDeploy = st.channel_0_status;
        else Tlm.Payload.IsDeploy = st.channel_1_status;
    }
    OS_printf("%s: 0x%02X - %u Deploy status %u\n", __func__, Msg->Payload.address, Msg->Payload.channel, Tlm.Payload.IsDeploy);

transmit:
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(Tlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(Tlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}