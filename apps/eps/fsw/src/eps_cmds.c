/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as "core Flight System: Bootes"
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
#include "eps_utils.h"
#include "cfe_srl_csp.h"
#include "eps_interface_cfg.h"

#include <gs/param/internal/types.h>
#include <gs/param/table.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "eps_p80_drv.h"
#include "eps_bp8_drv.h"

#define EPS_CSP_PING_DEFAULT_SIZE 1U
#define EPS_CSP_PING_MAX_SIZE     128U

static bool EPS_IsP80PowerIfNode(uint8_t csp_node)
{
    return (csp_node == EPS_P80_PMU_CSP_NODE) || (csp_node == EPS_P80_PDU_CSP_NODE);
}

static void EPS_SendQueryReport(const void *Msg, uint8_t source, uint8_t dataId,
                                uint8_t arg0, uint8_t arg1,
                                const void *data, size_t dataSize)
{
    const uint8_t *bytes = (const uint8_t *)data;
    size_t reportSize = (data == NULL) ? 0 : dataSize;
    size_t offset = 0;
    uint16_t sequence = 0;

    if (reportSize > UINT16_MAX)
    {
        reportSize = UINT16_MAX;
    }

    do
    {
        EPS_Query_Report_Payload_t report = {0};
        size_t remaining = reportSize - offset;
        size_t chunkSize = (remaining > sizeof(report.data)) ? sizeof(report.data) : remaining;
        uint16_t payloadSize = (uint16_t)(sizeof(report) - sizeof(report.data) + chunkSize);

        report.source     = source;
        report.data_id    = dataId;
        report.arg0       = arg0;
        report.arg1       = arg1;
        report.sequence   = sequence;
        report.offset     = (uint16_t)offset;
        report.total_size = (uint16_t)reportSize;
        report.chunk_size = (uint16_t)chunkSize;

        if (chunkSize > 0)
        {
            memcpy(report.data, &bytes[offset], chunkSize);
        }

        EPS_SendReport(Msg, &report, payloadSize, CFE_SUCCESS, RPT_RETTYPE_SUCCESS);

        offset += chunkSize;
        sequence++;
    }
    while (offset < reportSize);
}

static void EPS_SendHkQueryReport(const void *Msg, uint8_t cspNode)
{
    switch (cspNode)
    {
        case EPS_P80_PMU_CSP_NODE:
            EPS_SendQueryReport(Msg, cspNode, EPS_QUERY_REPORT_P80_PMU_HK, 0, 0,
                                &EPS_AppData.PMU_HkTlm.Payload, sizeof(EPS_AppData.PMU_HkTlm.Payload));
            break;

        case EPS_P80_PDU_CSP_NODE:
            EPS_SendQueryReport(Msg, cspNode, EPS_QUERY_REPORT_P80_PDU_HK, 0, 0,
                                &EPS_AppData.PDU_HkTlm.Payload, sizeof(EPS_AppData.PDU_HkTlm.Payload));
            break;

        case EPS_P80_ACU1_CSP_NODE:
        case EPS_P80_ACU2_CSP_NODE:
            EPS_SendQueryReport(Msg, cspNode, EPS_QUERY_REPORT_P80_ACU_HK, 0, 0,
                                &EPS_AppData.ACU_HkTlm.Payload, sizeof(EPS_AppData.ACU_HkTlm.Payload));
            break;

        case EPS_BP8_CSP_NODE:
            EPS_SendQueryReport(Msg, cspNode, EPS_QUERY_REPORT_BP8_HK, 0, 0,
                                &EPS_AppData.BP8_HkTlm.Payload, sizeof(EPS_AppData.BP8_HkTlm.Payload));
            break;

        default:
            break;
    }
}

CFE_Status_t EPS_SendBcnCmd(const EPS_SendBcnCmd_t *Msg)
{
    CFE_Status_t status;

    (void)Msg;

    status = EPS_UpdateBcnTlmFromHw();
    if (status != CFE_SUCCESS)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: BCN send aborted, hardware fetch failed");
        return status;
    }

    /* Timestamp and transmit beacon on SB */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(EPS_AppData.BcnTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(EPS_AppData.BcnTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}
CFE_Status_t EPS_ReportBcnCmd(const EPS_ReportBcnCmd_t *Msg)
{
    EPS_BcnTlm_Full_Payload_t *bcn = &EPS_AppData.BcnTlm.Payload;
    CFE_Status_t status;

    (void)Msg;

    status = EPS_UpdateBcnTlmFromHw();
    if (status != CFE_SUCCESS)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: BCN report aborted, hardware fetch failed");
        return status;
    }

    EPS_PrintBcnReport(bcn);

    EPS_SendReport(Msg, bcn, sizeof(*bcn), CFE_SUCCESS, RPT_RETTYPE_SUCCESS);

    CFE_EVS_SendEvent(EPS_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "EPS: BCN report sent (%u bytes)", (unsigned)sizeof(*bcn));

    return CFE_SUCCESS;
}

CFE_Status_t EPS_NoopCmd(const EPS_NoopCmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    CFE_EVS_SendEvent(EPS_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "EPS: NOOP command %s",
                      EPS_VERSION);
    EPS_SendReport(Msg, &EPS_AppData.Counters, sizeof(EPS_AppData.Counters),
                   CFE_SUCCESS, RPT_RETTYPE_SUCCESS);

    return CFE_SUCCESS;
}

CFE_Status_t EPS_ResetCountersCmd(const EPS_ResetCountersCmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter = 0;
    EPS_AppData.Counters.ErrCounter = 0;
    EPS_AppData.Counters.GetHkErrCounter = 0;
    EPS_AppData.Counters.GetBcnErrCounter = 0;

    CFE_EVS_SendEvent(EPS_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "EPS: RESET Counters command");
    EPS_SendReport(Msg, &EPS_AppData.Counters, sizeof(EPS_AppData.Counters),
                   CFE_SUCCESS, RPT_RETTYPE_SUCCESS);

    return CFE_SUCCESS;
}


/* ========================================================================== */
/*  P80 Power Interface Commands                                              */
/* ========================================================================== */

CFE_Status_t EPS_P80_Power_If_Get_Cmd(const EPS_P80_Power_If_Get_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    if (!EPS_IsP80PowerIfNode(Msg->Payload.csp_node))
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Power Interface Get rejected, node %u is not PMU/PDU",
                          Msg->Payload.csp_node);
        OS_printf("[EPS] Power Interface Get REJECTED node=%u: valid nodes are PMU(1) or PDU(4)\n",
                  Msg->Payload.csp_node);
        return CFE_STATUS_RANGE_ERROR;
    }

    power_if_ch_status_t status = {0};
    gs_error_t err = EPS_P80_Drv_PowerIfGet(Msg->Payload.csp_node, Msg->Payload.name,
                                             &status, CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Power Interface Get command failed, err=%d", err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    EPS_PrintP80PowerIfStatus("EPS: Power Interface Get command succeeded", &status);
    EPS_SendQueryReport(Msg, Msg->Payload.csp_node, EPS_QUERY_REPORT_P80_POWER_IF_STATUS, 0, 0,
                        &status, sizeof(status));

    return CFE_SUCCESS;
}

CFE_Status_t EPS_P80_Power_If_Set_Cmd(const EPS_P80_Power_If_Set_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    const char *device = EPS_GetCspNodeDeviceName(Msg->Payload.csp_node);
    char name[EPS_P80_POWER_IF_NAME_LEN + 1];
    power_if_ch_status_t status = {0};

    EPS_CopyCmdString(name, sizeof(name), Msg->Payload.name, sizeof(Msg->Payload.name));

    if (!EPS_IsP80PowerIfNode(Msg->Payload.csp_node))
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Power Interface Set rejected, node %u is not PMU/PDU",
                          Msg->Payload.csp_node);
        OS_printf("[EPS] Power Interface Set REJECTED node=%u request_args: name=%s mode=%u on_cnt=%u off_cnt=%u; valid nodes are PMU(1) or PDU(4)\n",
                  Msg->Payload.csp_node, name, Msg->Payload.mode,
                  Msg->Payload.on_cnt, Msg->Payload.off_cnt);
        return CFE_STATUS_RANGE_ERROR;
    }

    gs_error_t err = EPS_P80_Drv_PowerIfSet(Msg->Payload.csp_node, Msg->Payload.name,
                                             Msg->Payload.mode, Msg->Payload.on_cnt,
                                             Msg->Payload.off_cnt, &status, CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Power Interface Set command failed, err=%d", err);
        OS_printf("[EPS] Power Interface Set FAILED device=%s node=%u request_args: name=%s mode=%u on_cnt=%u off_cnt=%u err=%d\n",
                  device, Msg->Payload.csp_node, name, Msg->Payload.mode,
                  Msg->Payload.on_cnt, Msg->Payload.off_cnt, err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("[EPS] Power Interface Set OK device=%s node=%u\n", device, Msg->Payload.csp_node);
    OS_printf("  request_args: name=%s mode=%u on_cnt=%u off_cnt=%u\n",
              name, Msg->Payload.mode, Msg->Payload.on_cnt, Msg->Payload.off_cnt);
    OS_printf("  no readback in this response; run Power Interface Get for applied status fields such as voltage/current/latchup\n");

    return CFE_SUCCESS;
}


CFE_Status_t EPS_P80_Power_If_List_Cmd(const EPS_P80_Power_If_List_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    if (!EPS_IsP80PowerIfNode(Msg->Payload.csp_node))
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Power Interface List rejected, node %u is not PMU/PDU",
                          Msg->Payload.csp_node);
        OS_printf("[EPS] Power Interface List REJECTED node=%u: valid nodes are PMU(1) or PDU(4)\n",
                  Msg->Payload.csp_node);
        return CFE_STATUS_RANGE_ERROR;
    }

    power_if_cmd_list_response_t list = {0};
    gs_error_t err = EPS_P80_Drv_PowerIfList(Msg->Payload.csp_node, &list, CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Power Interface command failed, err=%d", err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    EPS_PrintP80PowerIfList(&list);
    EPS_SendQueryReport(Msg, Msg->Payload.csp_node, EPS_QUERY_REPORT_P80_POWER_IF_LIST, 0, 0,
                        &list, sizeof(list));

    return CFE_SUCCESS;
}


/* ========================================================================== */
/*  Housekeeping Command                                                      */
/* ========================================================================== */

CFE_Status_t EPS_Get_HK_Cmd(const EPS_Get_HK_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;
    const char *device = EPS_GetCspNodeDeviceName(Msg->Payload.csp_node);
    gs_error_t err = EPS_ReadHk(Msg->Payload.csp_node, CSP_TIMEOUT(1));

    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        if (err == GS_ERROR_ARG)
        {
            CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "EPS: Get HK command failed (wrong node arg)");
            return CFE_STATUS_RANGE_ERROR;
        }

        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Get HK command failed on %s node %u, err=%d",
                          device, Msg->Payload.csp_node, err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    EPS_PrintHk(Msg->Payload.csp_node);
    EPS_SendHkQueryReport(Msg, Msg->Payload.csp_node);

    return CFE_SUCCESS;
}

CFE_Status_t EPS_Get_HK_All_Cmd(const EPS_Get_HK_All_Cmd_t *Msg)
{
    const uint8_t nodes[] = {
        EPS_P80_PMU_CSP_NODE,
        EPS_P80_PDU_CSP_NODE,
        EPS_P80_ACU1_CSP_NODE,
        EPS_P80_ACU2_CSP_NODE,
        EPS_BP8_CSP_NODE,
    };
    uint8_t fail_count = 0;

    (void)Msg;
    EPS_AppData.Counters.CmdCounter++;

    OS_printf("\n================[EPS HK ALL]===================\n");
    for (size_t i = 0; i < (sizeof(nodes) / sizeof(nodes[0])); i++)
    {
        uint8_t node = nodes[i];
        const char *device = EPS_GetCspNodeDeviceName(node);
        gs_error_t err = EPS_ReadHk(node, CSP_TIMEOUT(1));

        if (err != GS_OK)
        {
            fail_count++;
            OS_printf("[EPS] Get HK All FAILED device=%s node=%u err=%d\n", device, node, err);
        }
        else
        {
            EPS_PrintHk(node);
            EPS_SendHkQueryReport(Msg, node);
        }
    }
    OS_printf("[EPS] Get HK All DONE success=%u fail=%u\n",
              (unsigned int)((sizeof(nodes) / sizeof(nodes[0])) - fail_count),
              (unsigned int)fail_count);
    OS_printf("=======================================================\n");

    if (fail_count > 0)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Get HK All failed on %u node(s)", fail_count);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}


/* ========================================================================== */
/*  P80 Ground Watchdog Clear                                                 */
/* ========================================================================== */

CFE_Status_t EPS_P80_Gnd_Watchdog_Clear_Cmd(const EPS_P80_Gnd_Watchdog_Clear_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;
    gs_error_t err;
    const char *device = EPS_GetCspNodeDeviceName(Msg->Payload.csp_node);

    switch (Msg->Payload.csp_node)
    {
        case EPS_P80_PMU_CSP_NODE:
            err = EPS_P80_Drv_PMU_GndWdtClear(EPS_P80_PMU_CSP_NODE, CSP_TIMEOUT(1));
            break;

        case EPS_P80_PDU_CSP_NODE:
            err = EPS_P80_Drv_PDU_GndWdtClear(EPS_P80_PDU_CSP_NODE, CSP_TIMEOUT(1));
            break;

        case EPS_P80_ACU1_CSP_NODE:
            err = EPS_P80_Drv_ACU_GndWdtClear(EPS_P80_ACU1_CSP_NODE, CSP_TIMEOUT(1));
            break;

        case EPS_P80_ACU2_CSP_NODE:
            err = EPS_P80_Drv_ACU_GndWdtClear(EPS_P80_ACU2_CSP_NODE, CSP_TIMEOUT(1));
            break;

        default:
            EPS_AppData.Counters.ErrCounter++;
            CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "EPS: Get Gnd Watchdog command failed (wrong node arg)");
            return CFE_STATUS_RANGE_ERROR;
    }

    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: GND WDT clear failed on %s node %u, err=%d",
                          device, Msg->Payload.csp_node, err);
        OS_printf("[EPS] GND WDT Clear FAILED device=%s node=%u err=%d\n",
                  device, Msg->Payload.csp_node, err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("[EPS] GND WDT Clear OK device=%s node=%u\n", device, Msg->Payload.csp_node);

    return CFE_SUCCESS;
}

CFE_Status_t EPS_P80_Gnd_Watchdog_Clear_All_Cmd(const EPS_P80_Gnd_Watchdog_Clear_All_Cmd_t *Msg)
{
    const uint8_t nodes[] = {
        EPS_P80_PMU_CSP_NODE,
        EPS_P80_PDU_CSP_NODE,
        EPS_P80_ACU1_CSP_NODE,
        EPS_P80_ACU2_CSP_NODE,
    };
    uint8_t fail_count = 0;

    (void)Msg;
    EPS_AppData.Counters.CmdCounter++;

    for (size_t i = 0; i < (sizeof(nodes) / sizeof(nodes[0])); i++)
    {
        uint8_t node = nodes[i];
        const char *device = EPS_GetCspNodeDeviceName(node);
        gs_error_t err;

        switch (node)
        {
            case EPS_P80_PMU_CSP_NODE:
                err = EPS_P80_Drv_PMU_GndWdtClear(node, CSP_TIMEOUT(1));
                break;

            case EPS_P80_PDU_CSP_NODE:
                err = EPS_P80_Drv_PDU_GndWdtClear(node, CSP_TIMEOUT(1));
                break;

            case EPS_P80_ACU1_CSP_NODE:
            case EPS_P80_ACU2_CSP_NODE:
                err = EPS_P80_Drv_ACU_GndWdtClear(node, CSP_TIMEOUT(1));
                break;

            default:
                err = GS_ERROR_ARG;
                break;
        }

        if (err != GS_OK)
        {
            fail_count++;
            OS_printf("[EPS] GND WDT Clear All FAILED device=%s node=%u err=%d\n", device, node, err);
        }
        else
        {
            OS_printf("[EPS] GND WDT Clear All OK device=%s node=%u\n", device, node);
        }
    }

    if (fail_count > 0)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: GND WDT clear all failed on %u P80 node(s)", fail_count);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}


/* ========================================================================== */
/*  Remote Parameter Commands                                                 */
/* ========================================================================== */

CFE_Status_t EPS_RParam_Set_Cmd(const EPS_RParam_Set_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    const char *device = EPS_GetCspNodeDeviceName(Msg->Payload.csp_node);
    const char *table = EPS_GetRParamTableName(Msg->Payload.csp_node, Msg->Payload.table_id);

    if (Msg->Payload.size > sizeof(Msg->Payload.data))
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: RParam Set size too large, size=%u max=%u",
                          Msg->Payload.size, (unsigned int)sizeof(Msg->Payload.data));
        OS_printf("[EPS] RParam Set REJECTED device=%s node=%u table=%u(%s) addr=%u size=%u max=%u\n",
                  device, Msg->Payload.csp_node, Msg->Payload.table_id, table,
                  Msg->Payload.addr, Msg->Payload.size, (unsigned int)sizeof(Msg->Payload.data));
        return CFE_STATUS_RANGE_ERROR;
    }

    gs_error_t err = EPS_RParamSet(Msg->Payload.csp_node,
                                    Msg->Payload.table_id,
                                    Msg->Payload.addr,
                                    Msg->Payload.type,
                                    Msg->Payload.data,
                                    Msg->Payload.size,
                                    CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: RParam Set command failed, err=%d", err);
        OS_printf("[EPS] RParam Set FAILED device=%s node=%u table=%u(%s) addr=%u type=%u size=%u request_data=",
                  device, Msg->Payload.csp_node, Msg->Payload.table_id, table,
                  Msg->Payload.addr, Msg->Payload.type, Msg->Payload.size);
        for (uint16_t i = 0; i < Msg->Payload.size && i < sizeof(Msg->Payload.data); i++)
            OS_printf("%02X ", Msg->Payload.data[i]);
        OS_printf("err=%d\n", err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("[EPS] RParam Set OK device=%s node=%u table=%u(%s) addr=%u type=%u size=%u request_data=",
              device, Msg->Payload.csp_node, Msg->Payload.table_id, table,
              Msg->Payload.addr, Msg->Payload.type, Msg->Payload.size);
    for (uint16_t i = 0; i < Msg->Payload.size && i < sizeof(Msg->Payload.data); i++)
        OS_printf("%02X ", Msg->Payload.data[i]);
    OS_printf("\n");

    return CFE_SUCCESS;
}

CFE_Status_t EPS_RParam_Get_Cmd(const EPS_RParam_Get_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    const char *device = EPS_GetCspNodeDeviceName(Msg->Payload.csp_node);
    const char *table = EPS_GetRParamTableName(Msg->Payload.csp_node, Msg->Payload.table_id);
    uint8_t data[EPS_RPARAM_DATA_MAX_LEN] = {0};

    if (Msg->Payload.size > EPS_RPARAM_DATA_MAX_LEN)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: RParam Get size too large, size=%u max=%u",
                          Msg->Payload.size, (unsigned int)EPS_RPARAM_DATA_MAX_LEN);
        OS_printf("[EPS] RParam Get REJECTED device=%s node=%u table=%u(%s) addr=%u size=%u max=%u\n",
                  device, Msg->Payload.csp_node, Msg->Payload.table_id, table,
                  Msg->Payload.addr, Msg->Payload.size, (unsigned int)EPS_RPARAM_DATA_MAX_LEN);
        return CFE_STATUS_RANGE_ERROR;
    }

    gs_error_t err = EPS_RParamGet(Msg->Payload.csp_node,
                                    Msg->Payload.table_id,
                                    Msg->Payload.addr,
                                    Msg->Payload.type,
                                    data,
                                    Msg->Payload.size,
                                    CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: RParam Get command failed, err=%d", err);
        OS_printf("[EPS] RParam Get FAILED device=%s node=%u table=%u(%s) addr=%u type=%u size=%u err=%d\n",
                  device, Msg->Payload.csp_node, Msg->Payload.table_id, table,
                  Msg->Payload.addr, Msg->Payload.type, Msg->Payload.size, err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("[EPS] RParam Get OK device=%s node=%u table=%u(%s) addr=%u type=%u size=%u data=",
              device, Msg->Payload.csp_node, Msg->Payload.table_id, table,
              Msg->Payload.addr, Msg->Payload.type, Msg->Payload.size);
    for (uint16_t i = 0; i < Msg->Payload.size && i < sizeof(data); i++)
        OS_printf("%02X ", data[i]);
    OS_printf("\n");

    EPS_RParam_Get_Report_Payload_t report = {0};
    report.csp_node = Msg->Payload.csp_node;
    report.table_id = Msg->Payload.table_id;
    report.addr     = Msg->Payload.addr;
    report.type     = Msg->Payload.type;
    report.size     = Msg->Payload.size;
    memcpy(report.data, data, Msg->Payload.size);
    EPS_SendQueryReport(Msg, Msg->Payload.csp_node, EPS_QUERY_REPORT_RPARAM_VALUE,
                        Msg->Payload.table_id, Msg->Payload.type, &report, sizeof(report));

    return CFE_SUCCESS;
}

CFE_Status_t EPS_RParam_Get_Full_Table_Cmd(const EPS_RParam_Get_Full_Table_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    const char *device = EPS_GetCspNodeDeviceName(Msg->Payload.csp_node);
    const char *table = EPS_GetRParamTableName(Msg->Payload.csp_node, Msg->Payload.table_id);
    gs_param_table_instance_t tinst = {0};
    gs_error_t err = EPS_RParamFetchFullTable(Msg->Payload.csp_node,
                                               Msg->Payload.table_id,
                                               &tinst,
                                               CSP_TIMEOUT(3));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: RParam Get Full Table command failed, err=%d", err);
        OS_printf("[EPS] RParam Get Full Table FAILED device=%s node=%u table=%u(%s) err=%d\n",
                  device, Msg->Payload.csp_node, Msg->Payload.table_id, table, err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    EPS_PrintRParamFullTable(Msg->Payload.csp_node, Msg->Payload.table_id, &tinst);
    if (tinst.rows != NULL)
    {
        EPS_SendQueryReport(Msg, Msg->Payload.csp_node, EPS_QUERY_REPORT_RPARAM_TABLE_ROWS,
                            Msg->Payload.table_id, 0,
                            tinst.rows, tinst.row_count * sizeof(*tinst.rows));
    }

    if (tinst.memory != NULL && tinst.rows != NULL)
    {
        size_t memorySize = gs_param_calc_table_size(tinst.rows, tinst.row_count);
        EPS_SendQueryReport(Msg, Msg->Payload.csp_node, EPS_QUERY_REPORT_RPARAM_TABLE_MEMORY,
                            Msg->Payload.table_id, 0, tinst.memory, memorySize);
    }
    gs_param_table_free(&tinst);

    return CFE_SUCCESS;
}


/* ========================================================================== */
/*  Table Save/Load Commands                                                  */
/* ========================================================================== */

CFE_Status_t EPS_RParam_Table_Save_Cmd(const EPS_RParam_Table_Save_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    const char *device = EPS_GetCspNodeDeviceName(Msg->Payload.csp_node);
    const char *table = EPS_GetRParamTableName(Msg->Payload.csp_node, Msg->Payload.table_id);
    gs_error_t err = EPS_RParamTableSave(Msg->Payload.csp_node,
                                          Msg->Payload.table_id,
                                          CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Table Save command failed, err=%d", err);
        OS_printf("[EPS] Table Save FAILED device=%s node=%u table=%u(%s) err=%d\n",
                  device, Msg->Payload.csp_node, Msg->Payload.table_id, table, err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("[EPS] Table Save OK device=%s node=%u table=%u(%s)\n",
              device, Msg->Payload.csp_node, Msg->Payload.table_id, table);

    return CFE_SUCCESS;
}

CFE_Status_t EPS_RParam_Table_Load_Cmd(const EPS_RParam_Table_Load_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    const char *device = EPS_GetCspNodeDeviceName(Msg->Payload.csp_node);
    const char *table = EPS_GetRParamTableName(Msg->Payload.csp_node, Msg->Payload.table_id);
    gs_error_t err = EPS_RParamTableLoad(Msg->Payload.csp_node,
                                          Msg->Payload.table_id,
                                          CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Table Load command failed, err=%d", err);
        OS_printf("[EPS] Table Load FAILED device=%s node=%u table=%u(%s) err=%d\n",
                  device, Msg->Payload.csp_node, Msg->Payload.table_id, table, err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("[EPS] Table Load OK device=%s node=%u table=%u(%s)\n",
              device, Msg->Payload.csp_node, Msg->Payload.table_id, table);

    return CFE_SUCCESS;
}

CFE_Status_t EPS_RParam_Save_To_Store_Cmd(const EPS_RParam_Save_To_Store_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    char store[EPS_RPARAM_STORE_NAME_LEN];
    char slot[EPS_RPARAM_STORE_SLOT_LEN];
    const char *device = EPS_GetCspNodeDeviceName(Msg->Payload.csp_node);
    const char *table = EPS_GetRParamTableName(Msg->Payload.csp_node, Msg->Payload.table_id);

    EPS_CopyCmdString(store, sizeof(store), Msg->Payload.store, sizeof(Msg->Payload.store));
    EPS_CopyCmdString(slot, sizeof(slot), Msg->Payload.slot, sizeof(Msg->Payload.slot));

    if (store[0] == '\0')
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: RParam SaveToStore rejected, empty store");
        OS_printf("[EPS] RParam SaveToStore REJECTED device=%s node=%u table=%u(%s): empty store\n",
                  device, Msg->Payload.csp_node, Msg->Payload.table_id, table);
        return CFE_STATUS_RANGE_ERROR;
    }

    const char *slot_arg = (slot[0] == '\0') ? NULL : slot;
    const char *slot_print = (slot_arg == NULL) ? "<default>" : slot_arg;
    gs_error_t err = EPS_RParamSaveToStore(Msg->Payload.csp_node,
                                           Msg->Payload.table_id,
                                           store,
                                           slot_arg,
                                           CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: RParam SaveToStore command failed, err=%d", err);
        OS_printf("[EPS] RParam SaveToStore FAILED device=%s node=%u table=%u(%s) store=%s slot=%s err=%d\n",
                  device, Msg->Payload.csp_node, Msg->Payload.table_id, table, store, slot_print, err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("[EPS] RParam SaveToStore OK device=%s node=%u table=%u(%s) store=%s slot=%s\n",
              device, Msg->Payload.csp_node, Msg->Payload.table_id, table, store, slot_print);

    return CFE_SUCCESS;
}

CFE_Status_t EPS_RParam_Load_From_Store_Cmd(const EPS_RParam_Load_From_Store_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    char store[EPS_RPARAM_STORE_NAME_LEN];
    char slot[EPS_RPARAM_STORE_SLOT_LEN];
    const char *device = EPS_GetCspNodeDeviceName(Msg->Payload.csp_node);
    const char *table = EPS_GetRParamTableName(Msg->Payload.csp_node, Msg->Payload.table_id);

    EPS_CopyCmdString(store, sizeof(store), Msg->Payload.store, sizeof(Msg->Payload.store));
    EPS_CopyCmdString(slot, sizeof(slot), Msg->Payload.slot, sizeof(Msg->Payload.slot));

    if (store[0] == '\0')
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: RParam LoadFromStore rejected, empty store");
        OS_printf("[EPS] RParam LoadFromStore REJECTED device=%s node=%u table=%u(%s): empty store\n",
                  device, Msg->Payload.csp_node, Msg->Payload.table_id, table);
        return CFE_STATUS_RANGE_ERROR;
    }

    const char *slot_arg = (slot[0] == '\0') ? NULL : slot;
    const char *slot_print = (slot_arg == NULL) ? "<default>" : slot_arg;
    gs_error_t err = EPS_RParamLoadFromStore(Msg->Payload.csp_node,
                                             Msg->Payload.table_id,
                                             store,
                                             slot_arg,
                                             CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: RParam LoadFromStore command failed, err=%d", err);
        OS_printf("[EPS] RParam LoadFromStore FAILED device=%s node=%u table=%u(%s) store=%s slot=%s err=%d\n",
                  device, Msg->Payload.csp_node, Msg->Payload.table_id, table, store, slot_print, err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("[EPS] RParam LoadFromStore OK device=%s node=%u table=%u(%s) store=%s slot=%s\n",
              device, Msg->Payload.csp_node, Msg->Payload.table_id, table, store, slot_print);

    return CFE_SUCCESS;
}

CFE_Status_t EPS_RParam_Save_All_Cmd(const EPS_RParam_Save_All_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    const char *device = EPS_GetCspNodeDeviceName(Msg->Payload.csp_node);
    gs_error_t err = EPS_RParamSaveAll(Msg->Payload.csp_node, CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: Param Save (all tables) command failed, err=%d", err);
        OS_printf("[EPS] Param SaveAll FAILED device=%s node=%u err=%d\n",
                  device, Msg->Payload.csp_node, err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("[EPS] Param SaveAll OK device=%s node=%u\n", device, Msg->Payload.csp_node);

    CFE_EVS_SendEvent(EPS_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "EPS: Param Save (all tables) succeeded on node %u", Msg->Payload.csp_node);

    return CFE_SUCCESS;
}


/* ========================================================================== */
/*  CSP Standard Service Commands                                             */
/* ========================================================================== */

static int EPS_CSP_PrintPs(const EPS_CSP_PS_Cmd_t *Msg, uint8_t node, uint32_t timeout)
{
    bool         received_any = false;
    csp_conn_t  *conn         = csp_connect(CSP_PRIO_NORM, node, CSP_PS, 0, 0);
    csp_packet_t *packet;

    if (conn == NULL)
    {
        return CSP_ERR_TIMEDOUT;
    }

    packet = csp_buffer_get(1);
    if (packet == NULL)
    {
        csp_close(conn);
        return CSP_ERR_NOMEM;
    }

    packet->data[0] = 0x55;
    packet->length = 1;

    if (!csp_send(conn, packet, 0))
    {
        csp_buffer_free(packet);
        csp_close(conn);
        return CSP_ERR_TIMEDOUT;
    }

    packet = NULL;

    while ((packet = csp_read(conn, timeout)) != NULL)
    {
        size_t data_len = csp_buffer_data_size();
        size_t text_len = (packet->length < data_len) ? packet->length : (data_len - 1U);

        packet->data[text_len] = 0;
        OS_printf("%s", (char *)packet->data);
        EPS_SendQueryReport(Msg, node, EPS_QUERY_REPORT_CSP_PS_TEXT, 0, 0,
                            packet->data, text_len);

        csp_buffer_free(packet);
        packet = NULL;
        received_any = true;
    }

    csp_close(conn);

    return received_any ? CSP_ERR_NONE : CSP_ERR_TIMEDOUT;
}

CFE_Status_t EPS_CSP_PS_Cmd(const EPS_CSP_PS_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    const char *device = EPS_GetCspNodeDeviceName(Msg->Payload.csp_node);
    int         err    = 0;

    OS_printf("[EPS] CSP PS BEGIN device=%s node=%u\n", device, Msg->Payload.csp_node);
    err = EPS_CSP_PrintPs(Msg, Msg->Payload.csp_node, CSP_TIMEOUT(1));
    if (err != CSP_ERR_NONE)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: CSP PS failed on node %u, err=%d", Msg->Payload.csp_node, err);
        OS_printf("[EPS] CSP PS FAILED device=%s node=%u err=%d\n",
                  device, Msg->Payload.csp_node, err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("\n[EPS] CSP PS END device=%s node=%u\n", device, Msg->Payload.csp_node);

    return CFE_SUCCESS;
}

CFE_Status_t EPS_CSP_MemFree_Cmd(const EPS_CSP_MemFree_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    const char *device = EPS_GetCspNodeDeviceName(Msg->Payload.csp_node);
    uint32_t    memfree = 0;
    int         err     = csp_get_memfree(Msg->Payload.csp_node, CSP_TIMEOUT(1), &memfree);

    if (err != CSP_ERR_NONE)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: CSP MemFree failed on node %u, err=%d", Msg->Payload.csp_node, err);
        OS_printf("[EPS] CSP MemFree FAILED device=%s node=%u err=%d\n",
                  device, Msg->Payload.csp_node, err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("[EPS] CSP MemFree OK device=%s node=%u bytes=%lu\n",
              device, Msg->Payload.csp_node, (unsigned long)memfree);
    EPS_SendQueryReport(Msg, Msg->Payload.csp_node, EPS_QUERY_REPORT_CSP_MEMFREE, 0, 0,
                        &memfree, sizeof(memfree));

    return CFE_SUCCESS;
}

CFE_Status_t EPS_CSP_BufFree_Cmd(const EPS_CSP_BufFree_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    const char *device = EPS_GetCspNodeDeviceName(Msg->Payload.csp_node);
    uint32_t    buf_free = 0;
    int         err      = csp_get_buf_free(Msg->Payload.csp_node, CSP_TIMEOUT(1), &buf_free);

    if (err != CSP_ERR_NONE)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: CSP BufFree failed on node %u, err=%d", Msg->Payload.csp_node, err);
        OS_printf("[EPS] CSP BufFree FAILED device=%s node=%u err=%d\n",
                  device, Msg->Payload.csp_node, err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("[EPS] CSP BufFree OK device=%s node=%u buffers=%lu\n",
              device, Msg->Payload.csp_node, (unsigned long)buf_free);
    EPS_SendQueryReport(Msg, Msg->Payload.csp_node, EPS_QUERY_REPORT_CSP_BUF_FREE, 0, 0,
                        &buf_free, sizeof(buf_free));

    return CFE_SUCCESS;
}

CFE_Status_t EPS_CSP_Uptime_Cmd(const EPS_CSP_Uptime_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    const char *device = EPS_GetCspNodeDeviceName(Msg->Payload.csp_node);
    uint32_t    uptime = 0;
    int         err    = csp_get_uptime(Msg->Payload.csp_node, CSP_TIMEOUT(1), &uptime);

    if (err != CSP_ERR_NONE)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: CSP Uptime failed on node %u, err=%d", Msg->Payload.csp_node, err);
        OS_printf("[EPS] CSP Uptime FAILED device=%s node=%u err=%d\n",
                  device, Msg->Payload.csp_node, err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("[EPS] CSP Uptime OK device=%s node=%u seconds=%lu\n",
              device, Msg->Payload.csp_node, (unsigned long)uptime);
    EPS_SendQueryReport(Msg, Msg->Payload.csp_node, EPS_QUERY_REPORT_CSP_UPTIME, 0, 0,
                        &uptime, sizeof(uptime));

    return CFE_SUCCESS;
}

CFE_Status_t EPS_CSP_Ping_Cmd(const EPS_CSP_Ping_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    const char *device = EPS_GetCspNodeDeviceName(Msg->Payload.csp_node);
    unsigned int size = (Msg->Payload.size == 0) ? EPS_CSP_PING_DEFAULT_SIZE : Msg->Payload.size;

    if (size > EPS_CSP_PING_MAX_SIZE)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: CSP Ping size too large, size=%u max=%u",
                          size, EPS_CSP_PING_MAX_SIZE);
        OS_printf("[EPS] CSP Ping REJECTED device=%s node=%u size=%u max=%u\n",
                  device, Msg->Payload.csp_node, size, EPS_CSP_PING_MAX_SIZE);
        return CFE_STATUS_RANGE_ERROR;
    }

    int elapsed_ms = csp_ping(Msg->Payload.csp_node, CSP_TIMEOUT(1), size, Msg->Payload.opts);
    if (elapsed_ms < 0)
    {
        EPS_AppData.Counters.ErrCounter++;
        CFE_EVS_SendEvent(EPS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "EPS: CSP Ping failed on node %u", Msg->Payload.csp_node);
        OS_printf("[EPS] CSP Ping FAILED device=%s node=%u size=%u opts=0x%02X\n",
                  device, Msg->Payload.csp_node, size, Msg->Payload.opts);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("[EPS] CSP Ping OK device=%s node=%u size=%u opts=0x%02X rtt=%d ms\n",
              device, Msg->Payload.csp_node, size, Msg->Payload.opts, elapsed_ms);
    int32_t elapsedReport = elapsed_ms;
    EPS_SendQueryReport(Msg, Msg->Payload.csp_node, EPS_QUERY_REPORT_CSP_PING_MS, 0, 0,
                        &elapsedReport, sizeof(elapsedReport));

    return CFE_SUCCESS;
}

CFE_Status_t EPS_CSP_Reboot_Cmd(const EPS_CSP_Reboot_Cmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;

    const char *device = EPS_GetCspNodeDeviceName(Msg->Payload.csp_node);

    csp_reboot(Msg->Payload.csp_node);

    OS_printf("[EPS] CSP Reboot sent device=%s node=%u\n", device, Msg->Payload.csp_node);
    CFE_EVS_SendEvent(EPS_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "EPS: CSP Reboot sent to node %u", Msg->Payload.csp_node);

    return CFE_SUCCESS;
}
