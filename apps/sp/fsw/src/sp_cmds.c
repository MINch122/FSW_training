/**
 * @file
 *   SP App command handlers.
 *
 *   Communicates with two NanoPower DSP boards via local GSSB (I2C1 bus).
 *   Each DSP board contains two AR6 burn-wire release devices (Board A and Board B)
 *   that must both fire to deploy one solar panel.
 *
 *   Reference: NanoPower DSP Datasheet DS-1018088-1-32
 */

#include "sp_task.h"
#include "sp_cmds.h"
#include "sp_msgids.h"
#include "sp_eventids.h"
#include "sp_utils.h"
#include "sp_msg.h"
#include "sp_interface_cfg.h"

#include "rpt_interface_cfg.h"

#include <gs/gssb/gssb.h>
#include <gs/gssb/gssb_ar6.h>
#include <gs/gssb/gssb_autodeploy.h>
#include <gs/gssb/gssb_dev.h>

/* AR6 Board A I2C addresses indexed by DspNum (0=DSP1, 1=DSP2) */
static const uint8_t SP_Ar6AAddrs[2] = { SP_DSP1_AR6A_I2C_ADDR, SP_DSP2_AR6A_I2C_ADDR };

/* AR6 Board B I2C addresses indexed by DspNum (0=DSP1, 1=DSP2) */
static const uint8_t SP_Ar6BAddrs[2] = { SP_DSP1_AR6B_I2C_ADDR, SP_DSP2_AR6B_I2C_ADDR };

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t SP_SendHkCmd(const SP_SendHkCmd_t *Msg)
{
    SP_AppData.HkTlm.Payload.CommandCounter      = SP_AppData.CmdCounter;
    SP_AppData.HkTlm.Payload.CommandErrorCounter = SP_AppData.ErrCounter;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(SP_AppData.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(SP_AppData.HkTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t SP_SendBcnCmd(const SP_SendBcnCmd_t *Msg)
{
    static const uint8_t dsp_addrs[SP_DSP_BCN_COUNT] = {
        SP_DSP_BCN_I2C_ADDR_0,
        SP_DSP_BCN_I2C_ADDR_1,
        SP_DSP_BCN_I2C_ADDR_2,
        SP_DSP_BCN_I2C_ADDR_3
    };

    for (int i = 0; i < SP_DSP_BCN_COUNT; i++)
    {
        gs_gssb_ar6_release_status_t rel = {0};
        gs_error_t err = gs_gssb_ar6_get_release_status(dsp_addrs[i], SP_DSP_I2C_TIMEOUT_MS, &rel);
        if (err == GS_OK)
        {
            SP_AppData.BcnTlm.Payload.Dsp[i].status        = rel.status;
            SP_AppData.BcnTlm.Payload.Dsp[i].backup_status = rel.state;
        }
        else
        {
            SP_AppData.BcnTlm.Payload.Dsp[i].status        = 0;
            SP_AppData.BcnTlm.Payload.Dsp[i].backup_status = 0;
        }
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(SP_AppData.BcnTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(SP_AppData.BcnTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t SP_ReportBcnCmd(const SP_ReportBcnCmd_t *Msg)
{
    SP_BcnTlm_Payload_t *bcn = &SP_AppData.BcnTlm.Payload;

    OS_printf("\n================[SP BCN Report]===================\n");
    for (int i = 0; i < SP_DSP_BCN_COUNT; i++)
    {
        OS_printf("[DSP%d] Status: %u | BackupStatus: %u\n",
                  i, bcn->Dsp[i].status, bcn->Dsp[i].backup_status);
    }
    OS_printf("[SP] IsRunning: %u | IsDeploy: %u / %u | MaxTry: %u\n",
              SP_AppData.BcnTlm.IsRunning,
              SP_AppData.BcnTlm.IsDeploy[0], SP_AppData.BcnTlm.IsDeploy[1],
              SP_AppData.BcnTlm.MaxTry);
    OS_printf("=======================================================\n");

    SP_HandleReport(CFE_SUCCESS, SP_REPORT_BCN_CC, bcn, sizeof(*bcn));

    CFE_EVS_SendEvent(SP_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "SP: BCN report sent (%u bytes)", (unsigned)sizeof(*bcn));

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t SP_NoopCmd(const SP_NoopCmd_t *Msg)
{
    SP_AppData.CmdCounter++;
    uint16_t counters[2] = {SP_AppData.CmdCounter, SP_AppData.ErrCounter};

    CFE_EVS_SendEvent(SP_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "SP: NOOP command received.");
    SP_HandleReport(CFE_SUCCESS, SP_NOOP_CC, counters, sizeof(counters));

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t SP_ResetCounterCmd(const SP_ResetCountersCmd_t *Msg)
{
    SP_AppData.CmdCounter = 0;
    SP_AppData.ErrCounter = 0;
    uint16_t counters[2] = {SP_AppData.CmdCounter, SP_AppData.ErrCounter};

    CFE_EVS_SendEvent(SP_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "SP: RESET command");
    SP_HandleReport(CFE_SUCCESS, SP_RESET_COUNTERS_CC, counters, sizeof(counters));

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/*  Purpose:                                                                   */
/*     Queries AR6 release status and temperature from both AR6 boards on     */
/*     each of the two DSPs via local GSSB (I2C1), then populates HK tlm.   */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t SP_GetHkCmd(const SP_GetHkCmd_t *Msg)
{
    SP_AppData.CmdCounter++;

    for (int dsp = 0; dsp < 2; dsp++)
    {
        SP_DSP_Tlm_Payload_t         *tlm  = &SP_AppData.HkTlm.Payload.Dsp[dsp];
        uint8_t                       addrA = SP_Ar6AAddrs[dsp];
        uint8_t                       addrB = SP_Ar6BAddrs[dsp];
        gs_gssb_ar6_release_status_t  relA  = {0};
        gs_gssb_ar6_release_status_t  relB  = {0};
        gs_gssb_board_status_t        brd   = {0};
        int16_t                       tmpA  = 0;
        int16_t                       tmpB  = 0;
        gs_error_t                    err;

        /* --- AR6 Board A --- */
        err = gs_gssb_ar6_get_release_status(addrA, SP_DSP_I2C_TIMEOUT_MS, &relA);
        if (err == GS_OK)
        {
            tlm->BoardA.BurnState     = relA.state;
            tlm->BoardA.ReleaseStatus = relA.status;
            tlm->BoardA.BurnTimeLeft  = relA.burn_time_left;
            tlm->BoardA.BurnTries     = relA.burn_tries;
        }
        else
        {
            SP_AppData.ErrCounter++;
            CFE_EVS_SendEvent(SP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SP: DSP%d Board A release status failed, err=%d", dsp + 1, err);
        }

        err = gs_gssb_ar6_get_board_status(addrA, SP_DSP_I2C_TIMEOUT_MS, &brd);
        if (err == GS_OK)
        {
            tlm->SecondsSinceBoot = brd.seconds_since_boot;
            tlm->RebootCount      = brd.reboot_count;
        }

        err = gs_gssb_ar6_get_internal_temp(addrA, SP_DSP_I2C_TIMEOUT_MS, &tmpA);
        if (err == GS_OK) tlm->TempA = (int16)tmpA;

        /* --- AR6 Board B --- */
        err = gs_gssb_ar6_get_release_status(addrB, SP_DSP_I2C_TIMEOUT_MS, &relB);
        if (err == GS_OK)
        {
            tlm->BoardB.BurnState     = relB.state;
            tlm->BoardB.ReleaseStatus = relB.status;
            tlm->BoardB.BurnTimeLeft  = relB.burn_time_left;
            tlm->BoardB.BurnTries     = relB.burn_tries;
        }
        else
        {
            SP_AppData.ErrCounter++;
            CFE_EVS_SendEvent(SP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SP: DSP%d Board B release status failed, err=%d", dsp + 1, err);
        }

        err = gs_gssb_ar6_get_internal_temp(addrB, SP_DSP_I2C_TIMEOUT_MS, &tmpB);
        if (err == GS_OK) tlm->TempB = (int16)tmpB;

        /* Update beacon deploy status: both boards must be released */
        bool deployed = (relA.status == 1) && (relB.status == 1);
        SP_AppData.BcnTlm.IsDeploy[dsp] = deployed;

        OS_printf("\n===[SP DSP%d Status (addrA=0x%02X, addrB=0x%02X)]===\n",
                  dsp + 1, addrA, addrB);
        OS_printf("[AR6-A] Released=%u Burning=%u TimeLeft=%us Tries=%u Temp=%d ddegC\n",
                  tlm->BoardA.ReleaseStatus, tlm->BoardA.BurnState,
                  tlm->BoardA.BurnTimeLeft,  tlm->BoardA.BurnTries, tlm->TempA);
        OS_printf("[AR6-B] Released=%u Burning=%u TimeLeft=%us Tries=%u Temp=%d ddegC\n",
                  tlm->BoardB.ReleaseStatus, tlm->BoardB.BurnState,
                  tlm->BoardB.BurnTimeLeft,  tlm->BoardB.BurnTries, tlm->TempB);
        OS_printf("[BOARD] Uptime=%us Reboots=%u\n", tlm->SecondsSinceBoot, tlm->RebootCount);
        OS_printf("==========================================\n");
    }

    CFE_EVS_SendEvent(SP_HK_TLM_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "SP: HK - DSP1 A/B=%u/%u  DSP2 A/B=%u/%u",
                      SP_AppData.HkTlm.Payload.Dsp[0].BoardA.ReleaseStatus,
                      SP_AppData.HkTlm.Payload.Dsp[0].BoardB.ReleaseStatus,
                      SP_AppData.HkTlm.Payload.Dsp[1].BoardA.ReleaseStatus,
                      SP_AppData.HkTlm.Payload.Dsp[1].BoardB.ReleaseStatus);

    SP_HandleReport(CFE_SUCCESS, SP_GET_HK_CC, NULL, 0);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/*  Purpose:                                                                   */
/*     Triggers burn-wire deployment on the specified DSP (Board A then B).  */
/*     Both AR6 boards on the DSP must fire to release the solar panel.      */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t SP_DeployCmd(const SP_DeployCmd_t *Msg)
{
    SP_AppData.CmdCounter++;

    uint8_t dspNum  = Msg->Payload.DspNum & 0x01;
    uint8_t burnDur = Msg->Payload.BurnDuration ? Msg->Payload.BurnDuration
                                                : SP_DSP_BURN_DURATION_S;
    uint8_t addrA   = SP_Ar6AAddrs[dspNum];
    uint8_t addrB   = SP_Ar6BAddrs[dspNum];

    gs_error_t errA, errB;

    /* Fire AR6 Board A */
    errA = gs_gssb_ar6_burn(addrA, SP_DSP_I2C_TIMEOUT_MS, burnDur);
    if (errA != GS_OK)
    {
        SP_AppData.ErrCounter++;
        CFE_EVS_SendEvent(SP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SP: DSP%d Board A burn failed, err=%d", dspNum + 1, errA);
    }

    /* Fire AR6 Board B */
    errB = gs_gssb_ar6_burn(addrB, SP_DSP_I2C_TIMEOUT_MS, burnDur);
    if (errB != GS_OK)
    {
        SP_AppData.ErrCounter++;
        CFE_EVS_SendEvent(SP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SP: DSP%d Board B burn failed, err=%d", dspNum + 1, errB);
    }

    SP_AppData.BcnTlm.IsRunning = (errA == GS_OK || errB == GS_OK);

    int32 status = (errA == GS_OK && errB == GS_OK) ? CFE_SUCCESS
                                                     : CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    SP_HandleReport(status, SP_DEPLOY_CC, NULL, 0);

    OS_printf("[SP] Deploy DSP%d addrA=0x%02X addrB=0x%02X burn=%us errA=%d errB=%d\n",
              dspNum + 1, addrA, addrB, burnDur, errA, errB);

    CFE_EVS_SendEvent(SP_DEPLOY_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "SP: Deploy DSP%d (addrA=0x%02X addrB=0x%02X burn=%us) A_err=%d B_err=%d",
                      dspNum + 1, addrA, addrB, burnDur, errA, errB);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/*  Purpose:                                                                   */
/*     Stops burn on both AR6 boards of the specified DSP.                   */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t SP_StopBurnCmd(const SP_StopBurnCmd_t *Msg)
{
    SP_AppData.CmdCounter++;

    uint8_t dspNum = Msg->Payload.DspNum & 0x01;
    uint8_t addrA  = SP_Ar6AAddrs[dspNum];
    uint8_t addrB  = SP_Ar6BAddrs[dspNum];

    gs_error_t errA, errB;

    errA = gs_gssb_ar6_stop_burn(addrA, SP_DSP_I2C_TIMEOUT_MS);
    if (errA != GS_OK)
    {
        SP_AppData.ErrCounter++;
        CFE_EVS_SendEvent(SP_STOP_BURN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SP: DSP%d Board A stop burn failed, err=%d", dspNum + 1, errA);
    }

    errB = gs_gssb_ar6_stop_burn(addrB, SP_DSP_I2C_TIMEOUT_MS);
    if (errB != GS_OK)
    {
        SP_AppData.ErrCounter++;
        CFE_EVS_SendEvent(SP_STOP_BURN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SP: DSP%d Board B stop burn failed, err=%d", dspNum + 1, errB);
    }

    if (errA == GS_OK && errB == GS_OK)
        SP_AppData.BcnTlm.IsRunning = false;

    {
        int32 status = (errA == GS_OK && errB == GS_OK) ? CFE_SUCCESS : CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        int32 details[2] = {errA, errB};
        SP_HandleReport(status, SP_STOP_BURN_CC, details, sizeof(details));
    }

    OS_printf("[SP] StopBurn DSP%d addrA=0x%02X addrB=0x%02X errA=%d errB=%d\n",
              dspNum + 1, addrA, addrB, errA, errB);

    CFE_EVS_SendEvent(SP_STOP_BURN_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "SP: Stop burn DSP%d (addrA=0x%02X addrB=0x%02X) A_err=%d B_err=%d",
                      dspNum + 1, addrA, addrB, errA, errB);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/*  Purpose:                                                                   */
/*     Auto deploy both DSPs using gs_autodeploy_release_two_dsp().          */
/*     Internally retries with incrementing burn time until max_burn_time.   */
/*     This call blocks until the deployment algorithm completes.            */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t SP_AutoDeployCmd(const SP_AutoDeployCmd_t *Msg)
{
    SP_AppData.CmdCounter++;

    uint8_t startBurn = Msg->Payload.StartBurnTime ? Msg->Payload.StartBurnTime
                                                   : SP_AUTO_DEPLOY_START_BURNTIME_S;
    uint8_t increment = Msg->Payload.Increment     ? Msg->Payload.Increment
                                                   : SP_AUTO_DEPLOY_INCREMENT_S;
    uint8_t maxBurn   = Msg->Payload.MaxBurnTime   ? Msg->Payload.MaxBurnTime
                                                   : SP_AUTO_DEPLOY_MAX_BURNTIME_S;

    SP_AppData.BcnTlm.IsRunning = true;

    gs_error_t err = gs_autodeploy_release_two_dsp(
        SP_DSP1_AR6A_I2C_ADDR,
        SP_DSP1_AR6B_I2C_ADDR,
        SP_DSP2_AR6A_I2C_ADDR,
        SP_DSP2_AR6B_I2C_ADDR,
        startBurn,
        increment,
        maxBurn);

    SP_AppData.BcnTlm.IsRunning = false;

    int32 status = (err == GS_OK) ? CFE_SUCCESS : CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    SP_HandleReport(status, SP_AUTO_DEPLOY_CC, NULL, 0);

    if (err == GS_OK)
    {
        OS_printf("[SP] AutoDeploy OK start=%us inc=%us max=%us\n",
                  startBurn, increment, maxBurn);
        CFE_EVS_SendEvent(SP_AUTO_DEPLOY_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "SP: Auto deploy complete (start=%us inc=%us max=%us)",
                          startBurn, increment, maxBurn);
    }
    else
    {
        OS_printf("[SP] AutoDeploy FAILED err=%d start=%us inc=%us max=%us\n",
                  err, startBurn, increment, maxBurn);
        SP_AppData.ErrCounter++;
        CFE_EVS_SendEvent(SP_AUTO_DEPLOY_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SP: Auto deploy failed, err=%d (start=%us inc=%us max=%us)",
                          err, startBurn, increment, maxBurn);
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Helper: convert gs_gssb_model_t to human-readable string                   */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
static const char *SP_GssbModelName(gs_gssb_model_t model)
{
    switch (model)
    {
        case GS_GSSB_MODEL_MSP:    return "MSP";
        case GS_GSSB_MODEL_AR6:    return "AR6";
        case GS_GSSB_MODEL_ISTAGE: return "ISTAGE";
        case GS_GSSB_MODEL_ANT6:   return "ANT6";
        case GS_GSSB_MODEL_I4:     return "I4";
        default:                   return "UNKNOWN";
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/*  Purpose:                                                                   */
/*     Scans I2C1 bus for GSSB devices (addr 1-127) and logs results via      */
/*     events. For each responding address, queries the device model so the   */
/*     operator can confirm it is AR6 (not MSP, ANT6, etc.).                 */
/*     Used to discover AR6 board addresses during commissioning.             */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t SP_ScanAr6Cmd(const SP_ScanAr6Cmd_t *Msg)
{
    SP_AppData.CmdCounter++;

    int8_t devices[128];
    memset(devices, -1, sizeof(devices));

    gs_gssb_bus_scan(1, 127, SP_DSP_I2C_TIMEOUT_MS, devices);

    OS_printf("\n================[SP I2C Bus Scan]===================\n");

    int found = 0;
    for (int addr = 1; addr <= 127; addr++)
    {
        if (devices[addr] == 0)
        {
            found++;

            gs_gssb_model_t model = 0;
            gs_error_t      merr  = gs_gssb_get_model(addr, SP_DSP_I2C_TIMEOUT_MS, &model);

            if (merr == GS_OK)
            {
                OS_printf("[SCAN] addr=0x%02X model=%s\n", addr, SP_GssbModelName(model));
                CFE_EVS_SendEvent(SP_SCAN_AR6_INF_EID, CFE_EVS_EventType_INFORMATION,
                                  "SP: GSSB device found at I2C addr 0x%02X model=%s",
                                  addr, SP_GssbModelName(model));
            }
            else
            {
                OS_printf("[SCAN] addr=0x%02X model=? (err=%d)\n", addr, merr);
                CFE_EVS_SendEvent(SP_SCAN_AR6_INF_EID, CFE_EVS_EventType_INFORMATION,
                                  "SP: GSSB device found at I2C addr 0x%02X (model query failed err=%d)",
                                  addr, merr);
            }
        }
    }

    OS_printf("[SCAN] Total devices found: %d\n", found);
    OS_printf("====================================================\n");

    if (found == 0)
    {
        SP_AppData.ErrCounter++;
        CFE_EVS_SendEvent(SP_SCAN_AR6_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SP: Bus scan complete - no GSSB devices found on I2C1");
    }
    else
    {
        CFE_EVS_SendEvent(SP_SCAN_AR6_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "SP: Bus scan complete - %d device(s) found", found);
    }

    {
        uint16_t found_count = (uint16_t)found;
        SP_HandleReport((found > 0) ? CFE_SUCCESS : CFE_STATUS_EXTERNAL_RESOURCE_FAIL,
                        SP_SCAN_AR6_CC, &found_count, sizeof(found_count));
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/*  Purpose:                                                                   */
/*     Programs a new I2C address into a single AR6 board and commits to NVM. */
/*     Used during hardware commissioning (one board at a time on the bus).   */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t SP_SetAr6AddrCmd(const SP_SetAr6AddrCmd_t *Msg)
{
    SP_AppData.CmdCounter++;

    uint8_t curAddr = Msg->Payload.CurrentAddr;
    uint8_t newAddr = Msg->Payload.NewAddr;

    gs_error_t err = gs_gssb_set_i2c_addr(curAddr, SP_DSP_I2C_TIMEOUT_MS, newAddr);
    if (err != GS_OK)
    {
        SP_AppData.ErrCounter++;
        CFE_EVS_SendEvent(SP_SET_ADDR_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SP: Set I2C addr failed (cur=0x%02X new=0x%02X err=%d)",
                          curAddr, newAddr, err);
        OS_printf("[SP] SetAr6Addr FAILED set cur=0x%02X new=0x%02X err=%d\n",
                  curAddr, newAddr, err);
        {
            uint8_t addr_pair[2] = {curAddr, newAddr};
            SP_HandleReport(CFE_STATUS_EXTERNAL_RESOURCE_FAIL, SP_SET_AR6_ADDR_CC,
                            addr_pair, sizeof(addr_pair));
        }
        return CFE_SUCCESS;
    }

    err = gs_gssb_commit_i2c_addr(newAddr, SP_DSP_I2C_TIMEOUT_MS);
    if (err != GS_OK)
    {
        SP_AppData.ErrCounter++;
        CFE_EVS_SendEvent(SP_SET_ADDR_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SP: Commit I2C addr failed (new=0x%02X err=%d)", newAddr, err);
        OS_printf("[SP] SetAr6Addr FAILED commit new=0x%02X err=%d\n", newAddr, err);
        {
            uint8_t addr_pair[2] = {curAddr, newAddr};
            SP_HandleReport(CFE_STATUS_EXTERNAL_RESOURCE_FAIL, SP_SET_AR6_ADDR_CC,
                            addr_pair, sizeof(addr_pair));
        }
        return CFE_SUCCESS;
    }

    OS_printf("[SP] SetAr6Addr OK 0x%02X -> 0x%02X (NVM committed)\n", curAddr, newAddr);

    CFE_EVS_SendEvent(SP_SET_ADDR_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "SP: AR6 addr programmed 0x%02X -> 0x%02X (committed to NVM)",
                      curAddr, newAddr);

    {
        uint8_t addr_pair[2] = {curAddr, newAddr};
        SP_HandleReport(CFE_SUCCESS, SP_SET_AR6_ADDR_CC, addr_pair, sizeof(addr_pair));
    }

    return CFE_SUCCESS;
}
