/**
 * \file
 *   This file contains the source code for the BATT App Ground Command-handling functions.
 *
 *   Communicates with the NanoPower BP8 battery pack via CSP using the rparam library.
 */

/*
** Include Files:
*/
#include "batt_app.h"
#include "batt_cmds.h"
#include "batt_msgids.h"
#include "batt_eventids.h"
#include "batt_version.h"
#include "batt_msg.h"
#include "cfe_srl_csp.h"
#include <gs/param/internal/types.h>
#include <gs/param/rparam.h>
#include <gs/param/table.h>
#include <stdbool.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     Sends the housekeeping telemetry packet on the software bus.           */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t BATT_SendHkCmd(const BATT_SendHkCmd_t *Msg)
{
    /*
    ** Get command execution counters
    */
    BATT_Data.HkTlm.Payload.CommandCounter      = BATT_Data.CmdCounter;
    BATT_Data.HkTlm.Payload.CommandErrorCounter  = BATT_Data.ErrCounter;

    /*
    ** Send housekeeping telemetry packet
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(BATT_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(BATT_Data.HkTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* BATT NOOP command                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t BATT_NoopCmd(const BATT_NoopCmd_t *Msg)
{
    BATT_Data.CmdCounter++;

    CFE_EVS_SendEvent(BATT_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "BATT: NOOP command %s",
                      BATT_VERSION);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     Resets all the global counter variables.                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t BATT_ResetCountersCmd(const BATT_ResetCountersCmd_t *Msg)
{
    BATT_Data.CmdCounter = 0;
    BATT_Data.ErrCounter = 0;

    CFE_EVS_SendEvent(BATT_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "BATT: RESET command");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     Retrieves full telemetry from the NanoPower BP8 via rparam and         */
/*     populates the HK telemetry packet.                                     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t BATT_GetHkCmd(const BATT_GetHkCmd_t *Msg)
{
    BATT_Data.CmdCounter++;

    gs_param_table_instance_t tinst = {0};
    uint16_t checksum;
    gs_error_t err;

    /* Download table spec for telemetry table */
    err = gs_rparam_download_table_spec(&tinst, NULL,
                                         BATT_BP8_CSP_NODE,
                                         BATT_BP8_TABLE_TELEMETRY,
                                         CSP_TIMEOUT(1),
                                         &checksum);
    if (err != GS_OK)
    {
        BATT_Data.ErrCounter++;
        CFE_EVS_SendEvent(BATT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "BATT: Failed to download BP8 telemetry table spec, err=%d", err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    /* Download full telemetry table data */
    err = gs_rparam_get_full_table(&tinst,
                                    BATT_BP8_CSP_NODE,
                                    BATT_BP8_TABLE_TELEMETRY,
                                    checksum,
                                    CSP_TIMEOUT(1));
    if (err != GS_OK)
    {
        BATT_Data.ErrCounter++;
        CFE_EVS_SendEvent(BATT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "BATT: Failed to get BP8 telemetry data, err=%d", err);
        if (tinst.memory) free(tinst.memory);
        if (tinst.rows) free((void *)tinst.rows);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    BATT_HkTlm_Payload_t *hk = &BATT_Data.HkTlm.Payload;
    bool bat_fault = false;
    gs_error_t perr = GS_OK;

    perr |= gs_param_get_uint32(&tinst, BATT_BP8_TLM_UPTIME,       &hk->Uptime,        0);
    perr |= gs_param_get_uint16(&tinst, BATT_BP8_TLM_BOOTCOUNT,    &hk->BootCount,     0);
    perr |= gs_param_get_uint16(&tinst, BATT_BP8_TLM_BOOTCAUSE,    &hk->BootCause,     0);
    perr |= gs_param_get_uint16(&tinst, BATT_BP8_TLM_RESETCAUSE,   &hk->ResetCause,    0);
    perr |= gs_param_get_uint16(&tinst, BATT_BP8_TLM_VBAT,         &hk->Vbat,          0);
    perr |= gs_param_get_float (&tinst, BATT_BP8_TLM_SOC,          &hk->Soc,           0);
    perr |= gs_param_get_float (&tinst, BATT_BP8_TLM_I,            &hk->Current,       0);
    perr |= gs_param_get_uint16(&tinst, BATT_BP8_TLM_IN_I,         &hk->InCurrent,     0);
    perr |= gs_param_get_uint16(&tinst, BATT_BP8_TLM_OUT_I,        &hk->OutCurrent,    0);
    perr |= gs_param_get_uint16(&tinst, BATT_BP8_TLM_HEATER_I,     &hk->HeaterCurrent, 0);
    perr |= gs_param_get_int16 (&tinst, BATT_BP8_TLM_INT_TEMP,     &hk->IntTemp,       0);
    perr |= gs_param_get_float (&tinst, BATT_BP8_TLM_BAT_AVR_TEMP, &hk->BatAvrTemp,    0);
    perr |= gs_param_get_int16 (&tinst, BATT_BP8_TLM_BAT_1_TEMP,   &hk->BatTemp[0],    0);
    perr |= gs_param_get_int16 (&tinst, BATT_BP8_TLM_BAT_2_TEMP,   &hk->BatTemp[1],    0);
    perr |= gs_param_get_int16 (&tinst, BATT_BP8_TLM_BAT_3_TEMP,   &hk->BatTemp[2],    0);
    perr |= gs_param_get_int16 (&tinst, BATT_BP8_TLM_BAT_4_TEMP,   &hk->BatTemp[3],    0);
    perr |= gs_param_get_uint16(&tinst, BATT_BP8_TLM_O_VOLT_COUNT,  &hk->OVoltCount,    0);
    perr |= gs_param_get_bool  (&tinst, BATT_BP8_TLM_BAT_FAULT,     &bat_fault,         0);
    hk->BatFault = (uint8_t)bat_fault;

    if (perr != GS_OK)
    {
        BATT_Data.ErrCounter++;
        CFE_EVS_SendEvent(BATT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "BATT: Failed to parse BP8 telemetry params, err=%d", perr);
        if (tinst.memory) free(tinst.memory);
        if (tinst.rows)   free((void *)tinst.rows);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    /* Print telemetry summary */
    OS_printf("\n================[NanoPower BP8 Telemetry]===================\n");
    OS_printf("[SYSTEM]  Uptime: %u s | BootCount: %u | Cause(Boot/Reset): %u / %u\n",
              hk->Uptime, hk->BootCount, hk->BootCause, hk->ResetCause);
    OS_printf("[BATTERY] Vbat: %u mV | SOC: %.2f | Current: %.3f A\n",
              hk->Vbat, (double)hk->Soc, (double)hk->Current);
    OS_printf("[CURRENT] In: %u mA | Out: %u mA | Heater: %u mA\n",
              hk->InCurrent, hk->OutCurrent, hk->HeaterCurrent);
    OS_printf("[TEMP]    MCU: %d ddegC | Avg: %.1f degC\n",
              hk->IntTemp, (double)hk->BatAvrTemp);
    OS_printf("[TEMP]    Bat1: %d | Bat2: %d | Bat3: %d | Bat4: %d (ddegC)\n",
              hk->BatTemp[0], hk->BatTemp[1], hk->BatTemp[2], hk->BatTemp[3]);
    OS_printf("[STATUS]  OVoltCount: %u | BatFault: %s\n",
              hk->OVoltCount, hk->BatFault ? "YES" : "NO");
    OS_printf("=============================================================\n");

    CFE_EVS_SendEvent(BATT_HK_TLM_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "BATT: BP8 Telemetry - Vbat=%u mV, SOC=%.2f, Fault=%u",
                      hk->Vbat, (double)hk->Soc, hk->BatFault);

    /* Free allocated memory */
    if (tinst.memory) free(tinst.memory);
    if (tinst.rows) free((void *)tinst.rows);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     Sets the manual heater timer on the BP8 via rparam.                    */
/*     Duration in seconds (1-600). Setting to 0 stops the heater.           */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t BATT_SetHeaterCmd(const BATT_SetHeaterCmd_t *Msg)
{
    BATT_Data.CmdCounter++;

    uint16_t duration = Msg->Payload.Duration;

    gs_error_t err = gs_rparam_set_uint16(BATT_BP8_CSP_NODE,
                                           BATT_BP8_TABLE_CONTROL,
                                           BATT_BP8_CTRL_HEAT_MANUAL,
                                           GS_RPARAM_MAGIC_CHECKSUM,
                                           CSP_TIMEOUT(1),
                                           duration);

    if (err != GS_OK)
    {
        BATT_Data.ErrCounter++;
        CFE_EVS_SendEvent(BATT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "BATT: Set heater command failed, err=%d", err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    CFE_EVS_SendEvent(BATT_HEATER_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "BATT: Manual heater set to %u seconds", duration);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     Resets the battery fault signal on the BP8 via rparam.                 */
/*     Sets the fault_reset parameter in the control table to true.          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t BATT_ResetFaultCmd(const BATT_ResetFaultCmd_t *Msg)
{
    BATT_Data.CmdCounter++;

    uint8_t value = 1; /* true */
    gs_error_t err = gs_rparam_set_uint8(BATT_BP8_CSP_NODE,
                                          BATT_BP8_TABLE_CONTROL,
                                          BATT_BP8_CTRL_FAULT_RESET,
                                          GS_RPARAM_MAGIC_CHECKSUM,
                                          CSP_TIMEOUT(1),
                                          value);

    if (err != GS_OK)
    {
        BATT_Data.ErrCounter++;
        CFE_EVS_SendEvent(BATT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "BATT: Reset fault command failed, err=%d", err);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    CFE_EVS_SendEvent(BATT_FAULT_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "BATT: Battery fault reset command sent");

    return CFE_SUCCESS;
}
