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
 *   This file contains the source code for the Adcs App Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "adcs_app.h"
#include "adcs_cmds.h"
#include "adcs_msgids.h"
#include "adcs_tbl.h"
#include "adcs_utils.h"
#include "adcs_msg.h"
#include "adcs_eventids.h"

static void ADCS_ReportCommandPhase(uint8 CommandCode, uint8 Phase)
{
    ADCS_HandleReport(CFE_SUCCESS, CommandCode, &Phase, sizeof(Phase));
}

static void ADCS_RecordFirstFailure(CFE_Status_t *OverallStatus, CFE_Status_t Status)
{
    if ((*OverallStatus == CFE_SUCCESS) && (Status != CFE_SUCCESS))
    {
        *OverallStatus = Status;
    }
}

static CFE_Status_t ADCS_ReportCommandCompletion(uint8 CommandCode, CFE_Status_t OverallStatus)
{
    if (OverallStatus != CFE_SUCCESS)
    {
        ADCS_HandleReport(OverallStatus, CommandCode, NULL, 0);
        return OverallStatus;
    }

    ADCS_ReportCommandPhase(CommandCode, ADCS_RPT_PHASE_COMPLETED);
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t ADCS_SendHkCmd(const ADCS_SendHkCmd_t *Msg)
{
    CFE_Status_t Status;
    CFE_Status_t OverallStatus = CFE_SUCCESS;
    ADCS_HealthTlmMMTTlm_Payload_t MmtHealth = {0,};
    ADCS_RawCubeSenseSunTlm_Payload_t RawSun = {0,};
    ADCS_OpenLoopCmdMTQTlm_Payload_t MtqOpenLoop = {0,};
    ADCS_MTQConfigTlm_Payload_t MtqConfig = {0,};
    ADCS_RawCSSSensorTlm_Payload_t RawCSS = {0,};

    memset(&ADCS_AppData.HkTlm.Payload, 0, sizeof(ADCS_AppData.HkTlm.Payload));

    Status = ADCS_GetHealthTlmMMT(&MmtHealth);
    ADCS_RecordFirstFailure(&OverallStatus, Status);
    if (Status == CFE_SUCCESS) {
        ADCS_AppData.HkTlm.Payload.MAG0MCUCurrent = MmtHealth.Mag0MCUCurrent;
    }

    Status = ADCS_GetRawCubeSenseSun(&RawSun);
    ADCS_RecordFirstFailure(&OverallStatus, Status);
    if (Status == CFE_SUCCESS) {
        ADCS_AppData.HkTlm.Payload.FSS0CaptureResult = RawSun.FSS0CaptureResult;
        ADCS_AppData.HkTlm.Payload.FSS0DetectionResult = RawSun.FSS0DetectionResult;
    }

    Status = ADCS_GetOpenLoopCmdMTQ(&MtqOpenLoop);
    ADCS_RecordFirstFailure(&OverallStatus, Status);
    if (Status == CFE_SUCCESS) {
        ADCS_AppData.HkTlm.Payload.MTQ0OpenLoopOnTimeCommand = MtqOpenLoop.MTQ0_OpenLoopCmd;
        ADCS_AppData.HkTlm.Payload.MTQ1OpenLoopOnTimeCommand = MtqOpenLoop.MTQ1_OpenLoopCmd;
        ADCS_AppData.HkTlm.Payload.MTQ2OpenLoopOnTimeCommand = MtqOpenLoop.MTQ2_OpenLoopCmd;
    }

    Status = ADCS_GetMTQConfig(&MtqConfig);
    ADCS_RecordFirstFailure(&OverallStatus, Status);
    if (Status == CFE_SUCCESS) {
        ADCS_AppData.HkTlm.Payload.mtq0Mmax = MtqConfig.MTQ0MaxDipoleMoment;
        ADCS_AppData.HkTlm.Payload.mtq1Mmax = MtqConfig.MTQ1MaxDipoleMoment;
        ADCS_AppData.HkTlm.Payload.mtq2Mmax = MtqConfig.MTQ2MaxDipoleMoment;
        ADCS_AppData.HkTlm.Payload.onTimeMax = MtqConfig.MaxMTQOnTime;
        ADCS_AppData.HkTlm.Payload.mtqFfac = MtqConfig.MagneticControlFilterFactor;
    }

    Status = ADCS_GetRawCSSSensor(&RawCSS);
    ADCS_RecordFirstFailure(&OverallStatus, Status);
    if (Status == CFE_SUCCESS) {
        ADCS_AppData.HkTlm.Payload.css0Raw = RawCSS.CSS0;
        ADCS_AppData.HkTlm.Payload.css1Raw = RawCSS.CSS1;
        ADCS_AppData.HkTlm.Payload.css2Raw = RawCSS.CSS2;
        ADCS_AppData.HkTlm.Payload.css3Raw = RawCSS.CSS3;
        ADCS_AppData.HkTlm.Payload.css4Raw = RawCSS.CSS4;
        ADCS_AppData.HkTlm.Payload.css5Raw = RawCSS.CSS5;
        ADCS_AppData.HkTlm.Payload.css6Raw = RawCSS.CSS6;
        ADCS_AppData.HkTlm.Payload.css7Raw = RawCSS.CSS7;
        ADCS_AppData.HkTlm.Payload.css8Raw = RawCSS.CSS8;
        ADCS_AppData.HkTlm.Payload.css9Raw = RawCSS.CSS9;
        ADCS_AppData.HkTlm.Payload.rawCssIsValid = RawCSS.CSSValidFlag;
    }

    ADCS_APP_printf("ADCS: HK report requested\n");
    ADCS_HandleReport(OverallStatus, 0, &ADCS_AppData.HkTlm.Payload, sizeof(ADCS_AppData.HkTlm.Payload));

    return OverallStatus;
}

CFE_Status_t ADCS_SendBcnCmd(const ADCS_SendBcnCmd_t *Msg)
{
    CFE_Status_t Status;
    CFE_Status_t OverallStatus = CFE_SUCCESS;
    ADCS_PowerStateTlm_Payload_t PwrStt = {0,};
    ADCS_ControlModeTlm_Payload_t CtrlMode = {0,};
    ADCS_CalibratedGYRSensorTlm_Payload_t CalGYR = {0,};
    ADCS_RawCSSSensorTlm_Payload_t RawCSS = {0,};

    memset(&ADCS_AppData.BcnTlm.Payload, 0, sizeof(ADCS_AppData.BcnTlm.Payload));

    Status = ADCS_GetPowerState(&PwrStt);
    ADCS_RecordFirstFailure(&OverallStatus, Status);
    if (Status == CFE_SUCCESS) {
        uint8_t CombinedPowerState = 0;
        CombinedPowerState |= ((PwrStt.RWL0 & 1u) << 6);
        CombinedPowerState |= ((PwrStt.RWL1 & 1u) << 5);
        CombinedPowerState |= ((PwrStt.RWL2 & 1u) << 4);
        CombinedPowerState |= ((PwrStt.MAG0 & 1u) << 3);
        CombinedPowerState |= ((PwrStt.GYR0 & 1u) << 2);
        CombinedPowerState |= ((PwrStt.FSS0 & 1u) << 1);
        CombinedPowerState |= ((PwrStt.HSS0 & 1u) << 0);

        ADCS_AppData.BcnTlm.Payload.PowerState = CombinedPowerState;
    }
    Status = ADCS_GetControlMode(&CtrlMode);
    ADCS_RecordFirstFailure(&OverallStatus, Status);
    if (Status == CFE_SUCCESS) {
        ADCS_AppData.BcnTlm.Payload.ControlMode = CtrlMode.ControlMode;
    }
    Status = ADCS_GetCalibratedGYRSensor(&CalGYR);
    ADCS_RecordFirstFailure(&OverallStatus, Status);
    if (Status == CFE_SUCCESS) {
        ADCS_AppData.BcnTlm.Payload.GYR0CalibratedRateXComponent = CalGYR.GYR0CalibratedRateX;
        ADCS_AppData.BcnTlm.Payload.GYR0CalibratedRateYComponent = CalGYR.GYR0CalibratedRateY;
        ADCS_AppData.BcnTlm.Payload.GYR0CalibratedRateZComponent = CalGYR.GYR0CalibratedRateZ;
    }
    Status = ADCS_GetRawCSSSensor(&RawCSS);
    ADCS_RecordFirstFailure(&OverallStatus, Status);
    if (Status == CFE_SUCCESS) {
        ADCS_AppData.BcnTlm.Payload.CSS[0] = RawCSS.CSS0;
        ADCS_AppData.BcnTlm.Payload.CSS[1] = RawCSS.CSS1;
        ADCS_AppData.BcnTlm.Payload.CSS[2] = RawCSS.CSS2;
        ADCS_AppData.BcnTlm.Payload.CSS[3] = RawCSS.CSS3;
        ADCS_AppData.BcnTlm.Payload.CSS[4] = RawCSS.CSS4;
        ADCS_AppData.BcnTlm.Payload.CSS[5] = RawCSS.CSS5;
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(ADCS_AppData.BcnTlm.TelemetryHeader));
    Status = CFE_SB_TransmitMsg(CFE_MSG_PTR(ADCS_AppData.BcnTlm.TelemetryHeader), true);
    ADCS_RecordFirstFailure(&OverallStatus, Status);

    return OverallStatus;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* ADCS NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t ADCS_NoopCmd(const ADCS_NoopCmd_t *Msg)
{
    static const char NoopReport[] = "Yosi In Space";
    ADCS_HandleReport(CFE_SUCCESS, ADCS_NOOP_CC, (void *)NoopReport, sizeof(NoopReport));

    CFE_EVS_SendEvent(ADCS_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "ADCS: NOOP command received.");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t ADCS_ResetCountersCmd(const ADCS_ResetCountersCmd_t *Msg)
{
    ADCS_AppData.CmdCounter = 0;
    ADCS_AppData.ErrCounter = 0;

    uint8_t Cmds[2] = {ADCS_AppData.CmdCounter, ADCS_AppData.ErrCounter};
    ADCS_HandleReport(CFE_SUCCESS, ADCS_RESET_COUNTERS_CC, Cmds, sizeof(Cmds));

    CFE_EVS_SendEvent(ADCS_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "ADCS: RESET command");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetInterfaceTransportCmd(const ADCS_InterfaceTransportCmd_t *Msg)
{
    CFE_Status_t Status;

    Status = ADCS_SetInterfaceTransport(Msg->Payload.TransportType);

    ADCS_HandleReport(Status, ADCS_SET_INTERFACE_TRANSPORT_CC, (void *)&Msg->Payload, sizeof(Msg->Payload));

    if (Status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Interface Transport: 0x%08lx", (unsigned long)Status);
        return Status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetReset(void){
    // This command has no reply
    CFE_Status_t               status;

    status = ADCS_Reset();

    ADCS_HandleReport(status, ADCS_SET_RESET_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Reset: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("Set reset success.\n");

    return CFE_SUCCESS;
}


/********************************************************
 *
 * COSMIC Actual invoked command function
 * Upper functions are just the references
 *
 ********************************************************/

/* Set function */
CFE_Status_t ADCS_SetCurrentUnixTimeCmd(const ADCS_CurrentUnixTimeCmd_t *msg) {
    // ID 2
    CFE_Status_t               status;

    status = ADCS_SetCurrentUnixTime(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_CURRENT_UNIX_TIME_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Current Unix Time: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetErrorLogSettingCmd(const ADCS_ErrorLogSettingCmd_t *msg) {
    // ID 6
    CFE_Status_t               status;

    status = ADCS_SetErrorLogSetting(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_ERROR_LOG_SETTING_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Error Log Seetings: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetPersistConfigCmd(const ADCS_PersistConfigCmd_t *msg) {
    // ID 7
    CFE_Status_t               status;

    status = ADCS_SetPersistConfig();

    ADCS_HandleReport(status, ADCS_SET_PERSIST_CONFIG_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Satellite Orbit Param Config: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetControlEstimationModeCmd(const ADCS_ControlEstimationModeCmd_t *msg) {
    // ID 42
    CFE_Status_t               status;

    status = ADCS_SetControlEstimationMode(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_CONTROL_ESTIMATION_MODE_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Control Estimation Mode: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetDisableMagRwlMntMngCmd(const ADCS_DisableMagRwlMntMngCmd_t *msg) {
    // ID 43
    CFE_Status_t               status;

    status = ADCS_SetDisableMagRwlMntMng(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_DISABLE_MAG_RWL_MNT_MNG_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Disable Magnetic RWL Momentum Management: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetReferenceIRCVectorCmd(const ADCS_ReferenceIRCVectorCmd_t *msg) {
    // ID 47
    CFE_Status_t               status;

    status = ADCS_SetReferenceIRCVector(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_REFERENCE_IRC_VECTOR_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Reference IRC Vector: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetReferenceLLHTargetCmd(const ADCS_ReferenceLLHTargetCmd_t *msg) {
    // ID 48
    CFE_Status_t               status;

    status = ADCS_SetReferenceLLHTarget(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_REFERENCE_LLH_TARGET_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Reference LLH Target: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetCommandedGNSSMeasurementsCmd(const ADCS_CommandedGNSSMeasurementsCmd_t *msg) {
    // ID 49
    CFE_Status_t status;

    status = ADCS_SetCommandedGNSSMeasurements(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_COMMANDED_GNSS_MEASUREMENTS_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Commanded GNSS Measurements: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetOrbitModeCmd(const ADCS_OrbitModeCmd_t *msg) {
    // ID 51
    CFE_Status_t               status;

    status = ADCS_SetOrbitMode(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_ORBIT_MODE_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Orbit Mode: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetMagDeployCmd(const ADCS_MagDeployCmd_t *msg) {
    // ID 52
    CFE_Status_t               status;

    status = ADCS_SetMagDeploy(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_MAG_DEPLOY_CMD_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set MAG Deploy: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetReferenceRPYValuesCmd(const ADCS_ReferenceRPYvaluesCmd_t *msg) {
    // ID 54
    CFE_Status_t               status;

    status = ADCS_SetReferenceRPYValues(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_REFERENCE_RPY_VALUES_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Reference RPY Values: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetOpenLoopCmdMTQCmd(const ADCS_OpenLoopCmdMTQCmd_t *msg) {
    // ID 55
    CFE_Status_t               status;

    status = ADCS_SetOpenLoopCmdMTQ(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_OPENLOOPCMD_MTQ_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Open Loop Command MTQ: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}


CFE_Status_t ADCS_SetOpenLoopCmdRWLCmd(const ADCS_OpenLoopCmdRWLCmd_t *msg) {
    CFE_Status_t status = ADCS_SetOpenLoopCmdRWL(&msg->Payload);
    ADCS_HandleReport(status, ADCS_SET_OPENLOOPCMD_RWL_CC, NULL, 0);
    if (status != CFE_SUCCESS) {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Open Loop Command RWL: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");
    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetOpenLoopCmdHxyzRWCmd(const ADCS_OpenLoopCmdHxyzRWCmd_t *msg) {
    CFE_Status_t status = ADCS_SetOpenLoopCmdHxyzRW(&msg->Payload);
    ADCS_HandleReport(status, ADCS_SET_OPENLOOP_CMD_HXYZ_RW_CC, NULL, 0);
    if (status != CFE_SUCCESS) {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Open Loop Command Hxyz RW: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");
    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetPowerStateCmd(const ADCS_PowerStateCmd_t *msg) {
    // ID 56
    CFE_Status_t               status;

    status = ADCS_SetPowerState(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_POWER_STATE_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Power State: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetRunModeCmd(const ADCS_RunModeCmd_t *msg) {
    // ID 57
    CFE_Status_t               status;

    status = ADCS_SetRunMode(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_RUN_MODE_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Satellite Orbit Param Config: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetControlModeCmd(const ADCS_ControlModeCmd_t *msg) {
    // ID 58
    CFE_Status_t               status;

    status = ADCS_SetControlMode(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_CONTROL_MODE_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Control Mode: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetWhlConfigCmd(const ADCS_WhlConfigCmd_t *msg) {
    // ID 59
    CFE_Status_t               status;

    status = ADCS_SetWhlConfig(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_WHL_CONFIG_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Wheel Config: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetSatelliteConfigCmd(const ADCS_SatConfigCmd_t *msg) {
    // ID 61
    CFE_Status_t               status;

    status = ADCS_SetSatelliteConfig(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_SATELLITE_CONFIG_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Satellite Orbit Param Config: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetControllerConfigCmd(const ADCS_ControllerConfig_t *msg) {
    // ID 62
    CFE_Status_t               status;

    status = ADCS_SetControllerConfig(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_CONTROLLER_CONFIG_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Satellite Orbit Param Config: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetMag0MMTCalibConfigCmd(const ADCS_Mag0MMTCalibConfigCmd_t *msg) {
    // ID 63
    CFE_Status_t               status;

    status = ADCS_SetMag0MMTCalibConfig(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_MAG0_MMT_CALIB_CONFIG_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set MAG0 MMT Calibration Config: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetDefaultModeConfigCmd(const ADCS_DefaultModeConfigCmd_t *msg) {
    // ID 64
    CFE_Status_t               status;

    status = ADCS_SetDefaultModeConfig(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_DEFAULT_MODE_CONFIG_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Satellite Orbit Param Config: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetMountingConfigCmd(const ADCS_MountingConfigCmd_t *msg) {
    // ID 65
    CFE_Status_t               status;

    status = ADCS_SetMountingConfig(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_MOUNTING_CONFIG_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Satellite Orbit Param Config: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetMag1MMTCalibConfigCmd(const ADCS_Mag1MMTCalibConfigCmd_t *msg) {
    // ID 66
    CFE_Status_t               status;

    status = ADCS_SetMag1MMTCalibConfig(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_MAG1_MMT_CALIB_CONFIG_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set MAG1 MMT Calibration Config: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetEstimatorConfigCmd(const ADCS_EstimatorConfigCmd_t *msg) {
    // ID 67
    CFE_Status_t               status;

    status = ADCS_SetEstimatorConfig(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_ESTIMATOR_CONFIG_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Estimator Config: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetSatOrbitParamConfigCmd(const ADCS_SatOrbitParamConfigCmd_t *msg) {
    // ID 68
    CFE_Status_t               status;

    status = ADCS_SetSatOrbitParamConfig(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_SAT_ORBIT_PARAMS_CONFIG_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Satellite Orbit Param Config: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetNodeSelectionConfigCmd(const ADCS_NodeSelectionConfigCmd_t *msg) {
    // ID 69
    CFE_Status_t               status;

    status = ADCS_SetNodeSelectionConfig(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_NODE_SELECTION_CONFIG_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Node Selection Config: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetMTQConfigCmd(const ADCS_MTQConfigCmd_t *msg) {
    // ID 70
    CFE_Status_t               status;

    status = ADCS_SetMTQConfig(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_MTQ_CONFIG_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set MTQ Config: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetEstimationModeCmd(const ADCS_EstimationModeCmd_t *msg) {
    // ID 71
    CFE_Status_t               status;

    status = ADCS_SetEstimationMode(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_ESTIMATION_MODE_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Estimation Mode: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetOperationalStateCmd(const ADCS_OperationalStateCmd_t *msg) {
    // ID 72
    CFE_Status_t               status;

    status = ADCS_SetOperationalState(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_OPERATIONAL_STATE_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Operational State: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetMagSensingElmConfigCmd(const ADCS_MagSensingElmConfigCmd_t *msg) {
    // ID 77
    CFE_Status_t               status;

    status = ADCS_SetMagSensingElmConfig(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_MAG_SENSING_ELM_CONFIG_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set MAG Sensing Element Config: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetUnsolicitTlmMsgSetupCmd(const ADCS_UnsolicitTlmMsgSetupCmd_t *msg) {
    // ID 112
    CFE_Status_t               status;

    status = ADCS_SetUnsolicitTlmMsgSetup(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_UNSOLICIT_TLM_MSG_SETUP_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Unsolicit Telemetry Messeage Setup: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetInitiateEventLogTransferCmd(const ADCS_InitiateEventLogTransferCmd_t *msg) {
    // ID 120
    CFE_Status_t               status;

    status = ADCS_SetInitiateEventLogTransfer(&msg->Payload);

    ADCS_HandleReport(status, ADCS_SET_INITIATE_EVENT_LOG_TRANSFER_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Satellite Orbit Param Config: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

/********************************************************
 *
 * COSMIC Actual Get Command Function (Get tlm)
 *
 ********************************************************/
CFE_Status_t ADCS_GetErrorLogSettingCmd(void) {
    // ID 132
    CFE_Status_t               status;
    ADCS_ErrorLogSettingTlm_Payload_t RetVal = {0,};

    status = ADCS_GetErrorLogSetting(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_ERROR_LOG_SETTING_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Error Log Settings: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("ActiveState: %u || BufferFullAction: %u\n", RetVal.ActiveState, RetVal.BufferFullAction);

    return CFE_SUCCESS;
}

 CFE_Status_t ADCS_GetCurrentUnixTimeCmd(void) {
    // ID 133
    CFE_Status_t               status;
    ADCS_CurrentUnixTimeTlm_Payload_t RetVal = {0,};

    status = ADCS_GetCurrentUnixTime(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_CURRENT_UNIX_TIME_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Current Unix Time: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("Unix Time sec: %u || Unix Time subsec: %u\n", RetVal.CurrentUnixseconds, RetVal.CurrentUnixNanoseconds);

    return CFE_SUCCESS;
}


CFE_Status_t ADCS_GetPersistConfigDiagnosticCmd(void) {
    // ID 134
    CFE_Status_t               status;
    ADCS_PersistConfigDiagnosticTlm_Payload_t RetVal = {0,};

    status = ADCS_GetPersistConfigDiagnostic(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_PERSIST_CONFIG_DIAGNOSTIC_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Persist Config Diagonostic: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("State: %u\nLast Result: %u\nTimeStamp: %u\n",
                RetVal.State, RetVal.LastResult, RetVal.Timestamp);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetCommunicationStatusCmd(void) {
    // ID 135
    CFE_Status_t               status;
    ADCS_CommunicationStatusTlm_Payload_t RetVal = {0,};

    status = ADCS_GetCommunicationStatus(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_COMMUNICATION_STATUS_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Communication Status: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("[CAN Status]\n");
    OS_printf("TC counter          : %u\n", RetVal.CAN_TcCnt);
    OS_printf("TM Request counter  : %u\n", RetVal.CAN_TlmCnt);
    OS_printf("Errors in SW checkes: %u\n", RetVal.CAN_ErrSW);
    OS_printf("Erross by HW flags  : %u\n", RetVal.CAN_ErrHW);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetControlEstimationModeCmd(void) {
    // ID 150
    CFE_Status_t               status;
    ADCS_ControlEstimationModeTlm_Payload_t RetVal = {0,};

    status = ADCS_GetControlEstimationMode(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_CONTROL_ESTIMATION_MODE_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Control Estimation Mode: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("Control Mode: %u || Main Estimator Mode: %u || Backup Estimator Mode: %u || Control Timeout: %u\n",
                RetVal.ControlMode, RetVal.MainEstimatorMode, RetVal.BackupEstimatorMode, RetVal.ControlTimeout);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetReferenceIRCVectorCmd(void) {
    // ID 156
    CFE_Status_t               status;
    ADCS_ReferenceIRCVectorTlm_Payload_t RetVal = {0,};

    status = ADCS_GetReferenceIRCVector(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_REFERENCE_IRC_VECTOR_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get ReferenceIRC Vector: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("ECI pointing vector X: %f || ECI pointing vector Y: %f || ECI pointing vector Z: %f\n", RetVal.ECIPointingVectorX, RetVal.ECIPointingVectorY, RetVal.ECIPointingVectorZ);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetReferenceLLHTargetCmd(void) {
    // ID 157
    CFE_Status_t               status;
    ADCS_ReferenceLLHTargetTlm_Payload_t RetVal = {0,};

    status = ADCS_GetReferenceLLHTarget(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_REFERENCE_LLH_TARGET_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Reference LLH Target: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("Latitude: %f || Longitude: %f || Altitude: %f\n", RetVal.Latitude, RetVal.Longitude, RetVal.Altitude);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetOrbitModeCmd(void) {
    // ID 162
    CFE_Status_t               status;
    ADCS_OrbitModeTlm_Payload_t RetVal = {0,};

    status = ADCS_GetOrbitMode(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_ORBIT_MODE_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Orbit Mode: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("Orbit Mode: %u\n", RetVal.OrbitMode);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetHealthTlmMMTCmd(void) {
    // ID 167
    CFE_Status_t               status;
    ADCS_HealthTlmMMTTlm_Payload_t RetVal = {0,};

    status = ADCS_GetHealthTlmMMT(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_HEALTH_TLM_MMT_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Health Telemetry MMT: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("MAG0::\n");
    OS_printf("MCU Temp        : %d || MCU Current    : %u || MCU Voltage  : %u\n", RetVal.Mag0MCUTemperature, RetVal.Mag0MCUCurrent, RetVal.Mag0MCUVoltage);
    OS_printf("Primary Temp    : %d || Redundant Temp : %d || Burn Current : %u\n", RetVal.Mag0PrimaryTemperature, RetVal.Mag0RedundantTemperature, RetVal.Mag0BurnCurrent);
    OS_printf("DeployPinState  : %u || BurnPinState   : %u\n", RetVal.Mag0DeployPinState, RetVal.Mag0BurnPinState);
    OS_printf("BurnUnderCurrent: %u || BurnOverCurrent: %u || DeployTimeout: %u\n", RetVal.Mag0BurnUnderCurrent, RetVal.Mag0BurnOverCurrent, RetVal.Mag0DeployTimeout);

    OS_printf("MAG1::\n");
    OS_printf("MCU Temp        : %d || MCU Current    : %u || MCU Voltage  : %u\n", RetVal.Mag1MCUTemperature, RetVal.Mag1MCUCurrent, RetVal.Mag1MCUVoltage);
    OS_printf("Primary Temp    : %d || Redundant Temp : %d || Burn Current : %u\n", RetVal.Mag1PrimaryTemperature, RetVal.Mag1RedundantTemperature, RetVal.Mag1BurnCurrent);
    OS_printf("DeployPinState  : %u || BurnPinState   : %u\n", RetVal.Mag1DeployPinState, RetVal.Mag1BurnPinState);
    OS_printf("BurnUnderCurrent: %u || BurnOverCurrent: %u || DeployTimeout: %u\n", RetVal.Mag1BurnUnderCurrent, RetVal.Mag1BurnOverCurrent, RetVal.Mag1DeployTimeout);
    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetRawCalibratedCubeSenseSunCmd(void) {
    // IDs 170 (raw) and 178 (calibrated FSS)
    CFE_Status_t status;
    ADCS_RawCalibratedFSSReport_Payload_t RetVal = {0,};

    status = ADCS_GetRawCubeSenseSun(&RetVal.Raw);
    if (status == CFE_SUCCESS)
    {
        status = ADCS_Comm_GetCalibratedFSSSensor(&RetVal.Calibrated);
    }

    ADCS_HandleReport(status, ADCS_GET_RAW_CALIBRATED_CUBESENSE_SUN_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Raw CubeSense Sun: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("TimeSec: %u || TimeNanoSec: %u\n", RetVal.Raw.TimeSecond, RetVal.Raw.TimeNanoSecond);
    OS_printf("FSS0 Alpha: %d || Beta: %d || Capture Res: %u || Detection Res: %u\n", RetVal.Raw.FSS0AlphaAngle, RetVal.Raw.FSS0BetaAngle, RetVal.Raw.FSS0CaptureResult, RetVal.Raw.FSS0DetectionResult);
    OS_printf("FSS1 Alpha: %d || Beta: %d || Capture Res: %u || Detection Res: %u\n", RetVal.Raw.FSS1AlphaAngle, RetVal.Raw.FSS1BetaAngle, RetVal.Raw.FSS1CaptureResult, RetVal.Raw.FSS1DetectionResult);
    OS_printf("FSS2 Alpha: %d || Beta: %d || Capture Res: %u || Detection Res: %u\n", RetVal.Raw.FSS2AlphaAngle, RetVal.Raw.FSS2BetaAngle, RetVal.Raw.FSS2CaptureResult, RetVal.Raw.FSS2DetectionResult);
    OS_printf("FSS3 Alpha: %d || Beta: %d || Capture Res: %u || Detection Res: %u\n", RetVal.Raw.FSS3AlphaAngle, RetVal.Raw.FSS3BetaAngle, RetVal.Raw.FSS3CaptureResult, RetVal.Raw.FSS3DetectionResult);
    OS_printf("Valid Res: 0x%02X\n", RetVal.Raw.ValidResult);
    OS_printf("Cal TimeSec: %u || TimeNanoSec: %u\n", RetVal.Calibrated.TimeSeconds,
              RetVal.Calibrated.TimeNanoSeconds);
    OS_printf("Cal FSS0 X: %d || Y: %d || Z: %d || Valid: %u || Best: %u\n",
              RetVal.Calibrated.FSS0CalVecX, RetVal.Calibrated.FSS0CalVecY, RetVal.Calibrated.FSS0CalVecZ,
              RetVal.Calibrated.FSS0ValidFlag, RetVal.Calibrated.FSS0BestFlag);
    OS_printf("Cal FSS1 X: %d || Y: %d || Z: %d || Valid: %u || Best: %u\n",
              RetVal.Calibrated.FSS1CalVecX, RetVal.Calibrated.FSS1CalVecY, RetVal.Calibrated.FSS1CalVecZ,
              RetVal.Calibrated.FSS1ValidFlag, RetVal.Calibrated.FSS1BestFlag);
    OS_printf("Cal FSS2 X: %d || Y: %d || Z: %d || Valid: %u || Best: %u\n",
              RetVal.Calibrated.FSS2CalVecX, RetVal.Calibrated.FSS2CalVecY, RetVal.Calibrated.FSS2CalVecZ,
              RetVal.Calibrated.FSS2ValidFlag, RetVal.Calibrated.FSS2BestFlag);
    OS_printf("Cal FSS3 X: %d || Y: %d || Z: %d || Valid: %u || Best: %u\n",
              RetVal.Calibrated.FSS3CalVecX, RetVal.Calibrated.FSS3CalVecY, RetVal.Calibrated.FSS3CalVecZ,
              RetVal.Calibrated.FSS3ValidFlag, RetVal.Calibrated.FSS3BestFlag);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetReferenceRPYvaluesCmd(void) {
    // ID 181
    CFE_Status_t               status;
    ADCS_ReferenceRPYvaluesTlm_Payload_t RetVal = {0,};

    status = ADCS_GetReferenceRPYvalues(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_REFERENCE_RPY_VALUES_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Reference RPY Values: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("Roll: %f || Pitch: %f || Yaw: %f\n", RetVal.Roll, RetVal.Pitch, RetVal.Yaw);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetOpenLoopCmdMTQCmd(void) {
    // ID 182
    CFE_Status_t               status;
    ADCS_OpenLoopCmdMTQTlm_Payload_t RetVal = {0,};

    status = ADCS_GetOpenLoopCmdMTQ(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_OPENLOOPCMD_MTQ_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Open Loop Cmd MTQ: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("MTQ0_OpenLoopCmd: %d || MTQ1_OpenLoopCmd: %d || MTQ2_OpenLoopCmd: %d\n", RetVal.MTQ0_OpenLoopCmd, RetVal.MTQ1_OpenLoopCmd, RetVal.MTQ2_OpenLoopCmd);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetPowerStateCmd(void) {
    // ID 183
    CFE_Status_t               status;
    ADCS_PowerStateTlm_Payload_t RetVal = {0,};

    status = ADCS_GetPowerState(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_POWER_STATE_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Power State: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("RWL0: %u || RWL1: %u || RWL2: %u || RWL3: %u\n", RetVal.RWL0, RetVal.RWL1, RetVal.RWL2, RetVal.RWL3);
    OS_printf("MAG0: %u || MAG1: %u || GYR0: %u || GYR1: %u\n", RetVal.MAG0, RetVal.MAG1, RetVal.GYR0, RetVal.GYR1);
    OS_printf("FSS0: %u || FSS1: %u || FSS2: %u || FSS3: %u\n", RetVal.FSS0, RetVal.FSS1, RetVal.FSS2, RetVal.FSS3);
    OS_printf("HSS0: %u || HSS1: %u || STR0: %u || STR1: %u\n", RetVal.HSS0, RetVal.HSS1, RetVal.STR0, RetVal.STR1);
    OS_printf("EXT: Sensor0 %u || Sensor1: %u || GYR0: %u || GYR1: %u\n", RetVal.ExtSensor0, RetVal.ExtSensor1, RetVal.ExtGYR0, RetVal.ExtGYR1);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetRunModeCmd(void) {
    // ID 184
    CFE_Status_t               status;
    ADCS_RunModeTlm_Payload_t RetVal = {0,};

    status = ADCS_GetRunMode(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_RUN_MODE_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Calibrated GYR Sensor: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("Run Mode: %u\n", RetVal.RunMode);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetControlModeCmd(void) {
    // ID 185
    CFE_Status_t               status;
    ADCS_ControlModeTlm_Payload_t RetVal = {0,};

    status = ADCS_GetControlMode(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_CONTROL_MODE_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Control Mode: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("Control Mode   : %u\n", RetVal.ControlMode);
    OS_printf("Control Timeout: %u\n", RetVal.ControlTimeout);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetWhlConfigCmd(void) {
    // ID 186
    CFE_Status_t               status;
    ADCS_WhlConfigTlm_Payload_t RetVal = {0,};

    status = ADCS_GetWhlConfig(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_WHL_CONFIG_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Wheel Config Telemetry: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("Rwl0Inertia: %f || Rwl0MaxMomentum: %f || Rwl0MaxToque: %f\n", RetVal.Rwl0Inertia, RetVal.Rwl0MaxMomentum, RetVal.Rwl0MaxToque);
    OS_printf("Rwl1Inertia: %f || Rwl1MaxMomentum: %f || Rwl1MaxToque: %f\n", RetVal.Rwl0Inertia, RetVal.Rwl0MaxMomentum, RetVal.Rwl0MaxToque);
    OS_printf("Rwl2Inertia: %f || Rwl2MaxMomentum: %f || Rwl2MaxToque: %f\n", RetVal.Rwl0Inertia, RetVal.Rwl0MaxMomentum, RetVal.Rwl0MaxToque);
    OS_printf("Rwl3Inertia: %f || Rwl3MaxMomentum: %f || Rwl3MaxToque: %f\n", RetVal.Rwl0Inertia, RetVal.Rwl0MaxMomentum, RetVal.Rwl0MaxToque);
    OS_printf("WheelRampTorque: %f\n", RetVal.WheelRampTorque);
    OS_printf("WheelScheme: %u || FailedWheelID: %u\n", RetVal.WheelScheme, RetVal.FailedWheelID);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetSatelliteConfigCmd(void) {
    // ID 189
    CFE_Status_t               status;
    ADCS_SatelliteConfigTlm_Payload_t RetVal = {0,};

    status = ADCS_GetSatelliteConfig(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_SATELLITE_CONFIG_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Satellite Config: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("[MOI]\n");
    OS_printf("Ixx: %f || Iyy: %f || Izz: %f\n", RetVal.Ixx, RetVal.Iyy, RetVal.Izz);
    OS_printf("Ixy: %f || Ixz: %f || Iyz: %f\n", RetVal.Ixy, RetVal.Ixz, RetVal.Iyz);
    OS_printf("---------\n");
    OS_printf("[SunPointing Body Vector]\n");
    OS_printf("X_RAW : %d  || Y_RAW : %d  || Z_RAW : %d\n", RetVal.SunPointingBodyVectorX, RetVal.SunPointingBodyVectorY, RetVal.SunPointingBodyVectorZ);
    OS_printf("X_Real: %f || Y_Real: %f || Z_Real: %f\n", RetVal.SunPointingBodyVectorX*0.0001, RetVal.SunPointingBodyVectorY*0.0001, RetVal.SunPointingBodyVectorZ*0.0001);
    OS_printf("---------\n");
    OS_printf("[TargetTracking Body Vector]\n");
    OS_printf("X_RAW : %d  || Y_RAW : %d  || Z_RAW : %d\n", RetVal.TargetTrackingBodyVectorX, RetVal.TargetTrackingBodyVectorY, RetVal.TargetTrackingBodyVectorZ);
    OS_printf("X_Real: %f || Y_Real: %f || Z_Real: %f\n", RetVal.TargetTrackingBodyVectorX*0.0001, RetVal.TargetTrackingBodyVectorY*0.0001, RetVal.TargetTrackingBodyVectorZ*0.0001);
    OS_printf("---------\n");
    OS_printf("[SatTracking Body Vector]\n");
    OS_printf("X_RAW : %d  || Y_RAW : %d  || Z_RAW : %d\n", RetVal.SatTrackingBodyVectorX, RetVal.SatTrackingBodyVectorY, RetVal.SatTrackingBodyVectorZ);
    OS_printf("X_Real: %f || Y_Real: %f || Z_Real: %f\n", RetVal.SatTrackingBodyVectorX*0.0001, RetVal.SatTrackingBodyVectorY*0.0001, RetVal.SatTrackingBodyVectorZ*0.0001);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetControllerConfigCmd(void) {
    // ID 190
    CFE_Status_t               status;
    ADCS_ControllerConfigTlm_Payload_t RetVal = {0,};

    status = ADCS_GetControllerConfig(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_CONTROLLER_CONFIG_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Controller Config: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("Default Control Mode         : %u\n", RetVal.DefaultControlMode);
    OS_printf("[Gains]\n");
    OS_printf("DetumblingDampingGain        : %f\n", RetVal.DetumblingDampingGain);
    OS_printf("SunSpinGain_Sunlit        : %f\n", RetVal.SunSpinGain_Sunlit);
    OS_printf("SunSpinGain_Eclipse        : %f\n", RetVal.SunSpinGain_Eclipse);
    OS_printf("DetumblingSpinGain        : %f\n", RetVal.DetumblingSpinGain);
    OS_printf("FastBDotGain        : %f\n", RetVal.FastBDotGain);
    OS_printf("YMomNutationDampingGain        : %f\n", RetVal.YMomNutationDampingGain);
    OS_printf("YMomNutationDampingQuatGain        : %f\n", RetVal.YMomNutationDampingQuatGain);
    OS_printf("XGGQuatGain        : %f\n", RetVal.XGGQuatGain);
    OS_printf("YGGQuatGain        : %f\n", RetVal.YGGQuatGain);
    OS_printf("ZGGQuatGain        : %f\n", RetVal.ZGGQuatGain);
    OS_printf("WheelDesatControlGain        : %f\n", RetVal.WheelDesatControlGain);
    OS_printf("YMomProportionalGain        : %f\n", RetVal.YMomProportionalGain);
    OS_printf("YMomDerivativeGain        : %f\n", RetVal.YMomDerivativeGain);
    OS_printf("RWheelProportionalGain        : %f\n", RetVal.RWheelProportionalGain);
    OS_printf("RWheelDerivativeGain        : %f\n", RetVal.RWheelDerivativeGain);
    OS_printf("TrackingProportionalGain        : %f\n", RetVal.TrackingProportionalGain);
    OS_printf("TrackingDerivativeGain        : %f\n", RetVal.TrackingDerivativeGain);
    OS_printf("TrackingIntegralGain        : %f\n", RetVal.TrackingIntegralGain);
    OS_printf("ReferenceSpinRate_degps        : %f\n", RetVal.ReferenceSpinRate_degps);
    OS_printf("ReferenceWheelMomentum_Nms        : %f\n", RetVal.ReferenceWheelMomentum_Nms);
    OS_printf("YWheelBiasMomentum_Nms        : %f\n", RetVal.YWheelBiasMomentum_Nms);
    OS_printf("RefSpinRate_RW_degps        : %f\n", RetVal.RefSpinRate_RW_degps);
    OS_printf("SunKeepOutAngle_deg        : %f\n", RetVal.SunKeepOutAngle_deg);
    OS_printf("RollLimitAngle_deg        : %f\n", RetVal.RollLimitAngle_deg);

    OS_printf("[Flags]\n");
    OS_printf("YawCompensationForEarthRotation: %u\n", RetVal.flags.YawCompensationForEarthRotation);
    OS_printf("Enable SunTracking In Eclipse: %u\n", RetVal.flags.EnableSunTrackingInEclipse);
    OS_printf("Enable SunAvoidance          : %u\n", RetVal.flags.EnableSunAvoidance);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetMag0MMTCalibConfigCmd(void) {
    // ID 191
    CFE_Status_t               status;
    ADCS_Mag0MMTCalibConfigTlm_Payload_t RetVal = {0,};

    status = ADCS_GetMag0MMTCalibConfig(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_MAG0_MMT_CALIB_CONFIG_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get MAG0 MMT Calibration Config: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("[Offset]\n");
    OS_printf("(RAW)  Channel 1: %d || Channel 2: %d || Channel 3: %d\n", RetVal.MMT_Ch1Offset, RetVal.MMT_Ch2Offset, RetVal.MMT_Ch3Offset);
    OS_printf("(Real) Channel 1: %f || Channel 2: %f || Channel 3: %f\n", RetVal.MMT_Ch1Offset*0.001, RetVal.MMT_Ch2Offset*0.001, RetVal.MMT_Ch3Offset*0.001);
    OS_printf("\n");
    OS_printf("[Sensitivity Matrix]\n");
    OS_printf("(RAW)\n");
    OS_printf("S11: %d || S12: %d || S13: %d\n", RetVal.MMT_SensitivityMAT_S11, RetVal.MMT_SensitivityMAT_S12, RetVal.MMT_SensitivityMAT_S13);
    OS_printf("S21: %d || S22: %d || S23: %d\n", RetVal.MMT_SensitivityMAT_S21, RetVal.MMT_SensitivityMAT_S22, RetVal.MMT_SensitivityMAT_S23);
    OS_printf("S31: %d || S32: %d || S33: %d\n", RetVal.MMT_SensitivityMAT_S31, RetVal.MMT_SensitivityMAT_S32, RetVal.MMT_SensitivityMAT_S33);
    OS_printf("(Real)\n");
    OS_printf("S11: %f || S12: %f || S13: %f\n", RetVal.MMT_SensitivityMAT_S11*0.001, RetVal.MMT_SensitivityMAT_S12*0.001, RetVal.MMT_SensitivityMAT_S13*0.001);
    OS_printf("S21: %f || S22: %f || S23: %f\n", RetVal.MMT_SensitivityMAT_S21*0.001, RetVal.MMT_SensitivityMAT_S22*0.001, RetVal.MMT_SensitivityMAT_S23*0.001);
    OS_printf("S31: %f || S32: %f || S33: %f\n", RetVal.MMT_SensitivityMAT_S31*0.001, RetVal.MMT_SensitivityMAT_S32*0.001, RetVal.MMT_SensitivityMAT_S33*0.001);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetDefaultModeConfigCmd(void) {
    // ID 192
    CFE_Status_t               status;
    ADCS_DefaultModeConfigTlm_Payload_t RetVal = {0,};

    status = ADCS_GetDefaultModeConfig(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_DEFAULT_MODE_CONFIG_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Default Mode COnfig: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("Default Run Mode            : %u\n", RetVal.DefaultRunMode);
    OS_printf("Default Operational State   : %u\n", RetVal.DefaultOperationalState);
    OS_printf("Default Control Mode in Safe: %u\n", RetVal.DefaultControlModeInOpStateSafe);
    OS_printf("Default Control Mode in Auto: %u\n", RetVal.DefaultControlModeInOpStateAuto);


    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetMountingConfigCmd(void) {
    // ID 193
    CFE_Status_t               status;
    ADCS_MountingConfigTlm_Payload_t RetVal = {0,};

    status = ADCS_GetMountingConfig(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_MOUNTING_CONFIG_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Calibrated GYR Sensor: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("[Direction ENUM]\n");
    OS_printf("| +X | -X | +Y | -Y | +Z | -Z |\n");
    OS_printf("|  1 |  2 |  3 |  4 |  5 |  6 |\n");

    OS_printf("\n");
    OS_printf("[Stack]\n");
    OS_printf("X-Y-Z: %u-%u-%u\n", RetVal.StackX_mounting, RetVal.StackY_mounting, RetVal.StackZ_mounting);

    OS_printf("\n");
    OS_printf("[MTQ]\n");
    OS_printf("0-1-2: %u-%u-%u\n", RetVal.MTQ0_mounting, RetVal.MTQ1_mounting, RetVal.MTQ2_mounting);

    OS_printf("\n");
    OS_printf("[Wheel]\n");
    OS_printf("0-1-2-3: %u-%u-%u-%u\n", RetVal.Wheel0_mounting, RetVal.Wheel1_mounting, RetVal.Wheel2_mounting, RetVal.Wheel3_mounting);
    OS_printf("Pyramid alpha-beta-gamma: %u-%u-%u\n", RetVal.PyramidRWL_alpha, RetVal.PyramidRWL_beta, RetVal.PyramidRWL_gamma);

    OS_printf("\n");
    OS_printf("[CSS]\n");
    OS_printf("0-1-2-3-4: %u-%u-%u-%u-%u\n", RetVal.CSS0_mounting, RetVal.CSS1_mounting, RetVal.CSS2_mounting, RetVal.CSS3_mounting, RetVal.CSS4_mounting);
    OS_printf("5-6-7-8-9: %u-%u-%u-%u-%u\n", RetVal.CSS5_mounting, RetVal.CSS6_mounting, RetVal.CSS7_mounting, RetVal.CSS8_mounting, RetVal.CSS9_mounting);

    OS_printf("\n");
    OS_printf("[FSS0]\n");
    OS_printf("(RAW)  alpha-beta-gamma: %d-%d-%d\n", RetVal.FSS0_alpha, RetVal.FSS0_beta, RetVal.FSS0_gamma);
    OS_printf("(Real) alpha-beta-gamma: %f-%f-%f\n", RetVal.FSS0_alpha*0.01, RetVal.FSS0_beta*0.01, RetVal.FSS0_gamma*0.01);

    OS_printf("\n");
    OS_printf("[FSS1]\n");
    OS_printf("(RAW)  alpha-beta-gamma: %d-%d-%d\n", RetVal.FSS1_alpha, RetVal.FSS1_beta, RetVal.FSS1_gamma);
    OS_printf("(Real) alpha-beta-gamma: %f-%f-%f\n", RetVal.FSS1_alpha*0.01, RetVal.FSS1_beta*0.01, RetVal.FSS1_gamma*0.01);

    OS_printf("\n");
    OS_printf("[HSS0]\n");
    OS_printf("(RAW)  alpha-beta-gamma: %d-%d-%d\n", RetVal.HSS0_alpha, RetVal.HSS0_beta, RetVal.HSS0_gamma);
    OS_printf("(Real) alpha-beta-gamma: %f-%f-%f\n", RetVal.HSS0_alpha*0.01, RetVal.HSS0_beta*0.01, RetVal.HSS0_gamma*0.01);

    OS_printf("\n");
    OS_printf("[HSS1]\n");
    OS_printf("(RAW)  alpha-beta-gamma: %d-%d-%d\n", RetVal.HSS1_alpha, RetVal.HSS1_beta, RetVal.HSS1_gamma);
    OS_printf("(Real) alpha-beta-gamma: %f-%f-%f\n", RetVal.HSS1_alpha*0.01, RetVal.HSS1_beta*0.01, RetVal.HSS1_gamma*0.01);

    OS_printf("\n");
    OS_printf("[MAG0]\n");
    OS_printf("(RAW)  alpha-beta-gamma: %d-%d-%d\n", RetVal.MAG0_alpha, RetVal.MAG0_beta, RetVal.MAG0_gamma);
    OS_printf("(Real) alpha-beta-gamma: %f-%f-%f\n", RetVal.MAG0_alpha*0.01, RetVal.MAG0_beta*0.01, RetVal.MAG0_gamma*0.01);

    OS_printf("\n");
    OS_printf("[MAG1]\n");
    OS_printf("(RAW)  alpha-beta-gamma: %d-%d-%d\n", RetVal.MAG1_alpha, RetVal.MAG1_beta, RetVal.MAG1_gamma);
    OS_printf("(Real) alpha-beta-gamma: %f-%f-%f\n", RetVal.MAG1_alpha*0.01, RetVal.MAG1_beta*0.01, RetVal.MAG1_gamma*0.01);


    OS_printf("\n");
    OS_printf("[STR0]\n");
    OS_printf("(RAW)  alpha-beta-gamma: %d-%d-%d\n", RetVal.STR0_alpha, RetVal.STR0_beta, RetVal.STR0_gamma);
    OS_printf("(Real) alpha-beta-gamma: %f-%f-%f\n", RetVal.STR0_alpha*0.01, RetVal.STR0_beta*0.01, RetVal.STR0_gamma*0.01);

    OS_printf("\n");
    OS_printf("[STR1]\n");
    OS_printf("(RAW)  alpha-beta-gamma: %d-%d-%d\n", RetVal.STR1_alpha, RetVal.STR1_beta, RetVal.STR1_gamma);
    OS_printf("(Real) alpha-beta-gamma: %f-%f-%f\n", RetVal.STR1_alpha*0.01, RetVal.STR1_beta*0.01, RetVal.STR1_gamma*0.01);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetMag1MMTCalibConfigCmd(void) {
    // ID 194
    CFE_Status_t               status;
    ADCS_Mag1MMTCalibConfigTlm_Payload_t RetVal = {0,};

    status = ADCS_GetMag1MMTCalibConfig(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_MAG1_MMT_CALIB_CONFIG_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get MAG1 MMT Calibration Config: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("[Offset]\n");
    OS_printf("(RAW)  Channel 1: %d || Channel 2: %d || Channel 3: %d\n", RetVal.MMT_Ch1Offset, RetVal.MMT_Ch2Offset, RetVal.MMT_Ch3Offset);
    OS_printf("(Real) Channel 1: %f || Channel 2: %f || Channel 3: %f\n", RetVal.MMT_Ch1Offset*0.001, RetVal.MMT_Ch2Offset*0.001, RetVal.MMT_Ch3Offset*0.001);
    OS_printf("\n");
    OS_printf("[Sensitivity Matrix]\n");
    OS_printf("(RAW)\n");
    OS_printf("S11: %d || S12: %d || S13: %d\n", RetVal.MMT_SensitivityMAT_S11, RetVal.MMT_SensitivityMAT_S12, RetVal.MMT_SensitivityMAT_S13);
    OS_printf("S21: %d || S22: %d || S23: %d\n", RetVal.MMT_SensitivityMAT_S21, RetVal.MMT_SensitivityMAT_S22, RetVal.MMT_SensitivityMAT_S23);
    OS_printf("S31: %d || S32: %d || S33: %d\n", RetVal.MMT_SensitivityMAT_S31, RetVal.MMT_SensitivityMAT_S32, RetVal.MMT_SensitivityMAT_S33);
    OS_printf("(Real)\n");
    OS_printf("S11: %f || S12: %f || S13: %f\n", RetVal.MMT_SensitivityMAT_S11*0.001, RetVal.MMT_SensitivityMAT_S12*0.001, RetVal.MMT_SensitivityMAT_S13*0.001);
    OS_printf("S21: %f || S22: %f || S23: %f\n", RetVal.MMT_SensitivityMAT_S21*0.001, RetVal.MMT_SensitivityMAT_S22*0.001, RetVal.MMT_SensitivityMAT_S23*0.001);
    OS_printf("S31: %f || S32: %f || S33: %f\n", RetVal.MMT_SensitivityMAT_S31*0.001, RetVal.MMT_SensitivityMAT_S32*0.001, RetVal.MMT_SensitivityMAT_S33*0.001);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetEstimatorConfigCmd(void) {
    // ID 195
    CFE_Status_t               status;
    ADCS_EstimatorConfigTlm_Payload_t RetVal = {0,};

    status = ADCS_GetEstimatorConfig(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_ESTIMATOR_CONFIG_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Estimator Config: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("[Default Estimator Mode]\n");
    OS_printf("Main / Backup: %u / %u\n", RetVal.DefaultMainEstimatorMode, RetVal.DefaultBackupEstimatorMode);

    OS_printf("[Measurement Noise]\n");
    OS_printf("MAG: %f\n", RetVal.MAGMeasurementNoise);
    OS_printf("CSS: %f\n", RetVal.CSSMeasurementNoise);
    OS_printf("FSS: %f\n", RetVal.FSSMeasurementNoise);
    OS_printf("HSS: %f\n", RetVal.HSSMeasurementNoise);
    OS_printf("STR: %f\n", RetVal.STRMeasurementNoise);
    OS_printf("MMTRKF: %f\n", RetVal.MMTRKFSystemNoise);
    OS_printf("EKFSys: %f\n", RetVal.EKFSystemNoise);

    OS_printf("[Nutation]\n");
    OS_printf("Nut Eps: %f\n", RetVal.NutationEpsilonCorrection);
    OS_printf("Nut Psi: %f\n", RetVal.NutationPsiCorrection);

    OS_printf("[Use Sensors in EKF]\n");
    OS_printf("FSS: %u\n", RetVal.UseFSSinEKF);
    OS_printf("CSS: %u\n", RetVal.UseFSSinEKF);
    OS_printf("HSS: %u\n", RetVal.UseFSSinEKF);
    OS_printf("STR: %u\n", RetVal.UseFSSinEKF);

    OS_printf("[Triad Vector]\n");
    OS_printf("Vector 1 / 2: %u / %u\n", RetVal.TriadVector1, RetVal.TriadVector2);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetSatOrbitParamConfigCmd(void) {
    // ID 196
    CFE_Status_t               status;
    ADCS_SatOrbitParamConfigTlm_Payload_t RetVal = {0,};

    status = ADCS_GetSatOrbitParamConfig(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_SAT_ORBIT_PARAM_CONFIG_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Satellite Orbit Param Config: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("Epoch: %lf || Inclination: %lf || RAAN: %lf || Eccenctricity: %lf\n", RetVal.Epoch, RetVal.Inclination, RetVal.RAAN, RetVal.Eccentricity);
    OS_printf("AOP: %lf || Mean anomaly: %lf || Mean Motion: %lf || B_starDrag: %lf\n", RetVal.AOP, RetVal.MeanAnomaly, RetVal.MeanMotion, RetVal.B_StarDrag);
    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetNodeSelectionConfigCmd(void) {
    // ID 197
    CFE_Status_t               status;
    ADCS_NodeSelectionConfigTlm_Payload_t RetVal = {0,};

    status = ADCS_GetNodeSelectionConfig(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_NODE_SELECTION_CONFIG_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Node Selection Config: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval

    OS_printf("RWA: ");
    for (int i = 7; i >= 0; i--) {
        if (RetVal.RWLSelectionFlags & (1 << i)) {
            OS_printf("1");
        } else {
            OS_printf("0");
        }
    }
    OS_printf("\n");
    OS_printf("MAG: ");
    for (int i = 7; i >= 0; i--) {
        if (RetVal.MAGSelectionFlags & (1 << i)) {
            OS_printf("1");
        } else {
            OS_printf("0");
        }
    }
    OS_printf("\n");
    OS_printf("FSS: ");
    for (int i = 7; i >= 0; i--) {
        if (RetVal.FSSSelectionFlags & (1 << i)) {
            OS_printf("1");
        } else {
            OS_printf("0");
        }
    }
    OS_printf("\n");
    OS_printf("HSS: ");
    for (int i = 7; i >= 0; i--) {
        if (RetVal.HSSSelectionFlags & (1 << i)) {
            OS_printf("1");
        } else {
            OS_printf("0");
        }
    }
    OS_printf("\n");
    OS_printf("GYR: ");
    for (int i = 7; i >= 0; i--) {
        if (RetVal.GYRSelectionFlags & (1 << i)) {
            OS_printf("1");
        } else {
            OS_printf("0");
        }
    }
    OS_printf("\n");
    OS_printf("STR: ");
    for (int i = 7; i >= 0; i--) {
        if (RetVal.STRSelectionFlags & (1 << i)) {
            OS_printf("1");
        } else {
            OS_printf("0");
        }
    }
    OS_printf("\n");
    OS_printf("GNSS: ");
    for (int i = 7; i >= 0; i--) {
        if (RetVal.GNSSSelectionFlags & (1 << i)) {
            OS_printf("1");
        } else {
            OS_printf("0");
        }
    }
    OS_printf("\n");
    OS_printf("EXT: ");
    for (int i = 7; i >= 0; i--) {
        if (RetVal.ExtSensorSelectionFlags & (1 << i)) {
            OS_printf("1");
        } else {
            OS_printf("0");
        }
    }
    OS_printf("\n");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetMTQConfigCmd(void) {
    // ID 198
    CFE_Status_t               status;
    ADCS_MTQConfigTlm_Payload_t RetVal = {0,};

    status = ADCS_GetMTQConfig(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_MTQ_CONFIG_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get MTQ Config: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("[Maximum Dipole Moment (Am2)]\n");
    OS_printf("MTQ0: %f || MTQ1: %f || MTQ2: %f\n", RetVal.MTQ0MaxDipoleMoment, RetVal.MTQ1MaxDipoleMoment, RetVal.MTQ2MaxDipoleMoment);
    OS_printf("[MTQ On Time (ms)]\n");
    OS_printf("Max: %u || Min: %u\n", RetVal.MaxMTQOnTime, RetVal.MinMTQOnTime);
    OS_printf("Control Filter Factor: %f\n", RetVal.MagneticControlFilterFactor);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetEstimationModeCmd(void) {
    // ID 199
    CFE_Status_t               status;
    ADCS_EstimationModeTlm_Payload_t RetVal = {0,};

    status = ADCS_GetEstimationMode(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_ESTIMATION_MODE_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Estimation Mode: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("[Estimator Mode]\n");
    OS_printf("Main / Backup: %u / %u\n", RetVal.MainEstimatorMode, RetVal.BackupEstimatorMode);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetOperationalStateCmd(void) {
    // ID 200
    CFE_Status_t               status;
    ADCS_OperationalStateTlm_Payload_t RetVal = {0,};

    status = ADCS_GetOperationalState(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_OPERATIONAL_STATE_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Operational State: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("Operational Mode: %u\n", RetVal.OperationalMode);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetRawCalibratedCSSSensorCmd(void) {
    // IDs 203 (raw) and 206 (calibrated)
    CFE_Status_t status;
    ADCS_RawCalibratedCSSReport_Payload_t RetVal = {0,};

    status = ADCS_GetRawCSSSensor(&RetVal.Raw);
    if (status == CFE_SUCCESS)
    {
        status = ADCS_Comm_GetCalibratedCSSSensor(&RetVal.Calibrated);
    }

    ADCS_HandleReport(status, ADCS_GET_RAW_CALIBRATED_CSS_SENSOR_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Raw CSS Sensor: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("TimeSec : %u || TimeNanoSec : %u\n", RetVal.Raw.TimeSeconds, RetVal.Raw.TimeNanoSeconds);
    OS_printf("CSS 0 : %u, 1 : %u, 2 : %u, 3 : %u, 4 : %u, 5 : %u, 6 : %u, 7 : %u, 8 : %u, 9 : %u",
                RetVal.Raw.CSS0, RetVal.Raw.CSS1, RetVal.Raw.CSS2, RetVal.Raw.CSS3, RetVal.Raw.CSS4,
                RetVal.Raw.CSS5, RetVal.Raw.CSS6, RetVal.Raw.CSS7, RetVal.Raw.CSS8, RetVal.Raw.CSS9);
    OS_printf("Cal TimeSec : %u || TimeNanoSec : %u\n", RetVal.Calibrated.TimeSeconds, RetVal.Calibrated.TimeNanoSeconds);
    OS_printf("CSS Cal UnitVec X: %d || UnitVec Y: %d || UnitVec Z: %d\n",
              RetVal.Calibrated.CSSCalUnitVecX, RetVal.Calibrated.CSSCalUnitVecY, RetVal.Calibrated.CSSCalUnitVecZ);
    OS_printf("Cal Valid Flag CSS : 0x%02X\n", RetVal.Calibrated.CSSValidFlag);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetRawCalibratedGYRSensorCmd(void) {
    // IDs 204 (raw) and 207 (calibrated)
    CFE_Status_t status;
    ADCS_RawCalibratedGYRReport_Payload_t RetVal = {0,};

    status = ADCS_GetRawGYRSensor(&RetVal.Raw);
    if (status == CFE_SUCCESS)
    {
        status = ADCS_GetCalibratedGYRSensor(&RetVal.Calibrated);
    }

    ADCS_HandleReport(status, ADCS_GET_RAW_CALIBRATED_GYR_SENSOR_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Raw GYR Sensor: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("TimeSec : %u || TimeNanoSec : %u\n", RetVal.Raw.TimeSeconds, RetVal.Raw.TimeNanoSeconds);
    OS_printf("GYR0 RawRate X: %f || RawRate Y: %f || RawRate Z: %f\n", RetVal.Raw.GYR0RawRateX, RetVal.Raw.GYR0RawRateY, RetVal.Raw.GYR0RawRateZ);
    OS_printf("GYR1 RawRate X: %f || RawRate Y: %f || RawRate Z: %f\n", RetVal.Raw.GYR1RawRateX, RetVal.Raw.GYR1RawRateY, RetVal.Raw.GYR1RawRateZ);
    OS_printf("Valid Flag GYR0 : 0x%02X\n", RetVal.Raw.GYR0ValidFlag);
    OS_printf("Cal TimeSec : %u || TimeNanoSec : %u\n", RetVal.Calibrated.TimeSeconds, RetVal.Calibrated.TimeNanoSeconds);
    OS_printf("GYR0 Cal Rate X: %f || Cal Rate Y: %f || Cal Rate Z: %f\n",
              RetVal.Calibrated.GYR0CalibratedRateX, RetVal.Calibrated.GYR0CalibratedRateY,
              RetVal.Calibrated.GYR0CalibratedRateZ);
    OS_printf("GYR1 Cal Rate X: %f || Cal Rate Y: %f || Cal Rate Z: %f\n",
              RetVal.Calibrated.GYR1CalibratedRateX, RetVal.Calibrated.GYR1CalibratedRateY,
              RetVal.Calibrated.GYR1CalibratedRateZ);
    OS_printf("Ext GYR0 Cal Rate X: %f || Cal Rate Y: %f || Cal Rate Z: %f\n",
              RetVal.Calibrated.ExtGYR0CalibratedRateX, RetVal.Calibrated.ExtGYR0CalibratedRateY,
              RetVal.Calibrated.ExtGYR0CalibratedRateZ);
    OS_printf("Ext GYR1 Cal Rate X: %f || Cal Rate Y: %f || Cal Rate Z: %f\n",
              RetVal.Calibrated.ExtGYR1CalibratedRateX, RetVal.Calibrated.ExtGYR1CalibratedRateY,
              RetVal.Calibrated.ExtGYR1CalibratedRateZ);
    OS_printf("Cal Valid Flag GYR0 : 0x%02X || GYR1 : 0x%02X || EXTGYR0 : 0x%02X || EXTGYR1 : 0x%02X\n",
              RetVal.Calibrated.GYR0ValidFlag, RetVal.Calibrated.GYR1ValidFlag,
              RetVal.Calibrated.EXTGYR0ValidFlag, RetVal.Calibrated.EXTGYR1ValidFlag);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetRawCalibratedRWLSensorCmd(void) {
    // IDs 205 (raw) and 209 (calibrated)
    CFE_Status_t status;
    ADCS_RawCalibratedRWLReport_Payload_t RetVal = {0,};

    status = ADCS_GetRawRWLSensor(&RetVal.Raw);
    if (status == CFE_SUCCESS)
    {
        status = ADCS_Comm_GetCalibratedRWLSensor(&RetVal.Calibrated);
    }

    ADCS_HandleReport(status, ADCS_GET_RAW_CALIBRATED_RWL_SENSOR_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Raw RWL Sensor: 0x%08lx", (unsigned long)status);
        return status;
    }

    OS_printf("TimeSec : %u || TimeNanoSec : %u\n", RetVal.Raw.TimeSeconds, RetVal.Raw.TimeNanoSeconds);
    OS_printf("RWL0 Measured Speed: %f\n", RetVal.Raw.RWL0MeasuredSpeed);
    OS_printf("RWL1 Measured Speed: %f\n", RetVal.Raw.RWL1MeasuredSpeed);
    OS_printf("RWL2 Measured Speed: %f\n", RetVal.Raw.RWL2MeasuredSpeed);
    OS_printf("RWL3 Measured Speed: %f\n", RetVal.Raw.RWL3MeasuredSpeed);
    OS_printf("Valid Flag RWL0 : 0x%02X || RWL1 : 0x%02X || RWL2 : 0x%02X || RWL3 : 0x%02X\n",
              RetVal.Raw.RWL0ValidFlag, RetVal.Raw.RWL1ValidFlag, RetVal.Raw.RWL2ValidFlag, RetVal.Raw.RWL3ValidFlag);
    OS_printf("Cal TimeSec : %u || TimeNanoSec : %u\n", RetVal.Calibrated.TimeSeconds, RetVal.Calibrated.TimeNanoSeconds);
    OS_printf("WhlSBC Trq X: %f || Trq Y: %f || Trq Z: %f\n",
              RetVal.Calibrated.WhlSBCTrqX, RetVal.Calibrated.WhlSBCTrqY, RetVal.Calibrated.WhlSBCTrqZ);
    OS_printf("WhlSBC Mom X: %f || Mom Y: %f || Mom Z: %f\n",
              RetVal.Calibrated.WhlSBCMomX, RetVal.Calibrated.WhlSBCMomY, RetVal.Calibrated.WhlSBCMomZ);
    OS_printf("Cal Valid Flag RWL : 0x%02X\n", RetVal.Calibrated.RWLValidFlag);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetCalibratedGYRSensorCmd(void) {
    // ID 207
    CFE_Status_t               status;
    ADCS_CalibratedGYRSensorTlm_Payload_t RetVal = {0,};

    status = ADCS_GetCalibratedGYRSensor(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_CALIBRATED_GYR_SENSOR_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Calibrated GYR Sensor: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("TimeSec : %u || TimeNanoSec : %u\n", RetVal.TimeSeconds, RetVal.TimeNanoSeconds);
    OS_printf("GYR0 Cal Rate X: %f || Cal Rate Y: %f || Cal Rate Z: %f\n", RetVal.GYR0CalibratedRateX, RetVal.GYR0CalibratedRateY, RetVal.GYR0CalibratedRateZ);
    OS_printf("GYR1 Cal Rate X: %f || Cal Rate Y: %f || Cal Rate Z: %f\n", RetVal.GYR1CalibratedRateX, RetVal.GYR1CalibratedRateY, RetVal.GYR1CalibratedRateZ);
    OS_printf("Ext GYR0 Cal Rate X: %f || Cal Rate Y: %f || Cal Rate Z: %f\n", RetVal.ExtGYR0CalibratedRateX, RetVal.ExtGYR0CalibratedRateY, RetVal.ExtGYR0CalibratedRateZ);
    OS_printf("Ext GYR1 Cal Rate X: %f || Cal Rate Y: %f || Cal Rate Z: %f\n", RetVal.ExtGYR1CalibratedRateX, RetVal.ExtGYR1CalibratedRateY, RetVal.ExtGYR1CalibratedRateZ);
    OS_printf("Valid flag GYR0 : 0x%02X\n", RetVal.GYR0ValidFlag);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetMagSensingElmConfigCmd(void) {
    // ID 221
    CFE_Status_t               status;
    ADCS_MagSensingElmConfigTlm_Payload_t RetVal = {0,};

    status = ADCS_GetMagSensingElmConfig(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_MAG_SENSING_ELM_CONFIG_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Mag Sensing Element Config: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("Mag0SensingElement : %u || Mag0SensingElement : %u\n", RetVal.Mag0SensingElement, RetVal.Mag1SensingElement);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetTlmLogInclMaskCmd(void) {
    // ID 227
    CFE_Status_t               status;
    ADCS_TlmLogInclMaskTlm_Payload_t RetVal = {0,};

    status = ADCS_GetTlmLogInclMask(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_TLM_LOG_INCLMASK_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Telemetry Inclution BitMask: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("[Fast Inclusion Bitmask]\n");
    for (int j = 0; j < 5; j++) {
        OS_printf("%d: ",j);
        for (int i = 7; i >= 0; i--) {
            if (RetVal.FastInclusionBitmask[j] & (1 << i)) {
                OS_printf("1");
            } else {
                OS_printf("0");
            }
        }
        OS_printf("\n");
    }
    OS_printf("\n");
	OS_printf("[Slow Inclusion Bitmask]\n");
    for (int j = 0; j < 5; j++) {
        OS_printf("%d: ",j);
        for (int i = 7; i >= 0; i--) {
            if (RetVal.SlowInclusionBitmask[j] & (1 << i)) {
                OS_printf("1");
            } else {
                OS_printf("0");
            }
        }
        OS_printf("\n");
    }
    OS_printf("\n");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetUnsolicitTlmMsgSetupCmd(void) {
    // ID 228
    CFE_Status_t               status;
    ADCS_UnsolicitTlmMsgSetupTlm_Payload_t RetVal = {0,};

    status = ADCS_GetUnsolicitTlmMsgSetup(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_UNSOLICIT_TLM_MSG_SETUP_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Unsolicit Telemetry Message Setup: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("[CAN Telemetry ID Inclusion Bitmask]\n");
    for (int j = 0; j < 5; j++) {
        OS_printf("%d: ",j);
        for (int i = 7; i >= 0; i--) {
            if (RetVal.CANTlmIDInclusionBitmask[j] & (1 << i)) {
                OS_printf("1");
            } else {
                OS_printf("0");
            }
        }
        OS_printf("\n");
    }
    OS_printf("\n");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetEventLogStatusResponseCmd(void) {
    // ID 235
    CFE_Status_t               status;
    ADCS_EventLogStatusResponseTlm_Payload_t RetVal = {0,};

    status = ADCS_GetEventLogStatusResponse(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_EVENT_LOG_STATUS_RESPONSE_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Event Log Status Response: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("Number of queued entries      : %u\n", RetVal.NumQueuedEntry);
    OS_printf("Number of buffered entries    : %u\n", RetVal.NumBufferedEntry);
    OS_printf("Number of entries             : %u\n", RetVal.NumEntry);
    OS_printf("Number of empty entries       : %u\n", RetVal.NumEmptyEntry);
    OS_printf("Oldest entry unix time        : %u\n", RetVal.OldEntryUnixTime);
    OS_printf("Latest entry unix time        : %u\n", RetVal.LastEntryUnixTime);
    OS_printf("Number of critical events     : %u\n", RetVal.NumCriticalEVS);
    OS_printf("Number of major warning events: %u\n", RetVal.NumMajorWarningEVS);
    OS_printf("Number of minor warning events: %u\n", RetVal.NumMinorWarningEVS);
    OS_printf("Number of info events         : %u\n", RetVal.NumInfoEVS);
    OS_printf("Write counter                 : %u\n", RetVal.WriteCnt);
    OS_printf("Read-Queue state              : %u\n", RetVal.ReadQueState);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetPortMapCmd(void) {
    // ID 239
    CFE_Status_t               status;
    ADCS_PortMapTlm_Payload_t RetVal = {0,};

    status = ADCS_GetPortMap(&RetVal);

    ADCS_HandleReport(status, ADCS_GET_PORTMAP_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Port Map: 0x%08lx", (unsigned long)status);
        return status;
    }

    // Handling Retval
    OS_printf("[Sensors]\n");
    OS_printf("|Sensor #|Node Type|Abstract Node Type|Serial Number Integer|Address (CAN)|\n");
    OS_printf("|1|%u|%u|%u|%u|\n", RetVal.NodeType_Sensor1, RetVal.AbstNodeType_Sensor1, RetVal.SerialNum_Sensor1, RetVal.Address_Sensor1);
    OS_printf("|2|%u|%u|%u|%u|\n", RetVal.NodeType_Sensor2, RetVal.AbstNodeType_Sensor2, RetVal.SerialNum_Sensor2, RetVal.Address_Sensor2);
    OS_printf("|3|%u|%u|%u|%u|\n", RetVal.NodeType_Sensor3, RetVal.AbstNodeType_Sensor3, RetVal.SerialNum_Sensor3, RetVal.Address_Sensor3);
    OS_printf("|4|%u|%u|%u|%u|\n", RetVal.NodeType_Sensor4, RetVal.AbstNodeType_Sensor4, RetVal.SerialNum_Sensor4, RetVal.Address_Sensor4);
    OS_printf("|5|%u|%u|%u|%u|\n", RetVal.NodeType_Sensor5, RetVal.AbstNodeType_Sensor5, RetVal.SerialNum_Sensor5, RetVal.Address_Sensor5);
    OS_printf("|6|%u|%u|%u|%u|\n", RetVal.NodeType_Sensor6, RetVal.AbstNodeType_Sensor6, RetVal.SerialNum_Sensor6, RetVal.Address_Sensor6);
    OS_printf("|7|%u|%u|%u|%u|\n", RetVal.NodeType_Sensor7, RetVal.AbstNodeType_Sensor7, RetVal.SerialNum_Sensor7, RetVal.Address_Sensor7);
    OS_printf("|8|%u|%u|%u|%u|\n", RetVal.NodeType_Sensor8, RetVal.AbstNodeType_Sensor8, RetVal.SerialNum_Sensor8, RetVal.Address_Sensor8);
    OS_printf("\n");
    OS_printf("[Wheels]\n");
    OS_printf("|Wheel #|Node Type|Abstract Node Type|Serial Number Integer|Address (CAN)|\n");
    OS_printf("|1|%u|%u|%u|%u|\n", RetVal.NodeType_Wheel1, RetVal.AbstNodeType_Wheel1, RetVal.SerialNum_Wheel1, RetVal.Address_Wheel1);
    OS_printf("|2|%u|%u|%u|%u|\n", RetVal.NodeType_Wheel2, RetVal.AbstNodeType_Wheel2, RetVal.SerialNum_Wheel2, RetVal.Address_Wheel2);
    OS_printf("|3|%u|%u|%u|%u|\n", RetVal.NodeType_Wheel3, RetVal.AbstNodeType_Wheel3, RetVal.SerialNum_Wheel3, RetVal.Address_Wheel3);
    OS_printf("|4|%u|%u|%u|%u|\n", RetVal.NodeType_Wheel4, RetVal.AbstNodeType_Wheel4, RetVal.SerialNum_Wheel4, RetVal.Address_Wheel4);


    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetErrorLogClearCmd(const ADCS_ErrorLogClearCmd_t *msg) {
    // ID 5
    CFE_Status_t               status;

	ADCS_UnsolicitTlmMsgSetupCmd_Payload_t SetVal_112 = {0,};
	SetVal_112.CANTlmRetrunInterval = 0;				// Return interval 1s
    SetVal_112.CANTlmEDInclusionBitmask[2] = 0;			// 0b0000 0001
    status = ADCS_SetUnsolicitTlmMsgSetup(&SetVal_112);


    status = ADCS_SetErrorLogClear();

    ADCS_HandleReport(status, ADCS_SET_ERROR_LOG_CLEAR_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Error Log Clear: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

/* CFE_Status_t ADCS_SequenceCmd_Detumbling(void) */
// Simple Version
CFE_Status_t ADCS_SequenceCmd_Detumbling(void) {	// Detumbling w/o Commissioning
	CFE_Status_t status;
	ADCS_PowerStateTlm_Payload_t RetVal_183 = {0,};
	ADCS_PowerStateCmd_Payload_t SetVal_56 = {0,};
	ADCS_ControllerConfig_Payload_t SetVal_62 = {0,};
	ADCS_ControllerConfigTlm_Payload_t RetVal_190 = {0,};
	ADCS_ControlEstimationModeCmd_Payload_t SetVal_42 = {0,};

	ADCS_ReportCommandPhase(ADCS_SEQ_DTUMB_CC, ADCS_RPT_PHASE_STARTED);

	status = ADCS_GetPowerState(&RetVal_183);
	if (status != CFE_SUCCESS)
	{
		ADCS_HandleReport(status, ADCS_SEQ_DTUMB_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}

	if (sizeof(SetVal_56) != sizeof(RetVal_183))
	{
		status = CFE_STATUS_WRONG_MSG_LENGTH;
		ADCS_HandleReport(status, ADCS_SEQ_DTUMB_CC, NULL, 0);
		return status;
	}

	memcpy(&SetVal_56, &RetVal_183, sizeof(SetVal_56));
	SetVal_56.GYR0 = 1;
	SetVal_56.MAG0 = 1;
	status = ADCS_SetPowerState(&SetVal_56);
	if (status != CFE_SUCCESS)
	{
		ADCS_HandleReport(status, ADCS_SEQ_DTUMB_CC, &SetVal_56, sizeof(SetVal_56));
		return status;
	}

	OS_TaskDelay(5000);

	status = ADCS_GetControllerConfig(&RetVal_190);
	if (status != CFE_SUCCESS)
	{
		ADCS_HandleReport(status, ADCS_SEQ_DTUMB_CC, &RetVal_190, sizeof(RetVal_190));
		return status;
	}

	if (sizeof(SetVal_62) != sizeof(RetVal_190))
	{
		status = CFE_STATUS_WRONG_MSG_LENGTH;
		ADCS_HandleReport(status, ADCS_SEQ_DTUMB_CC, NULL, 0);
		return status;
	}

	memcpy(&SetVal_62, &RetVal_190, sizeof(SetVal_62));
	SetVal_62.DefaultControlMode = 3;
	SetVal_62.flags.EnableSunTrackingInEclipse = 1;
	SetVal_62.flags.EnableSunAvoidance = 0;
	SetVal_42.MainEstimatorMode = 1;
	SetVal_42.ControlMode = 3;
	SetVal_42.ControlTimeout = 0;

	status = ADCS_SetControllerConfig(&SetVal_62);
	if (status != CFE_SUCCESS)
	{
		ADCS_HandleReport(status, ADCS_SEQ_DTUMB_CC, &SetVal_62, sizeof(SetVal_62));
		return status;
	}

	OS_TaskDelay(100);
	status = ADCS_SetControlEstimationMode(&SetVal_42);
	if (status != CFE_SUCCESS)
	{
		ADCS_HandleReport(status, ADCS_SEQ_DTUMB_CC, &SetVal_42, sizeof(SetVal_42));
		return status;
	}

	OS_TaskDelay(100);
	ADCS_ReportCommandPhase(ADCS_SEQ_DTUMB_CC, ADCS_RPT_PHASE_COMPLETED);
	return CFE_SUCCESS;
}

/* Other Pointing Commands */
CFE_Status_t ADCS_SequenceCmd_GNDpointing(const ADCS_SequenceCmdGNDpointingCmd_t *msg)
{
	ADCS_ReportCommandPhase(ADCS_SEQ_GNDPT_CC, ADCS_RPT_PHASE_STARTED);

	CFE_Status_t status = CFE_SUCCESS;
	int32 interstatus = CFE_SUCCESS;
	uint8 cnt_try = 0;
	uint8 estimator_mode = msg->Payload.flag_estmode;
	uint16 target_duration = msg->Payload.target_duration;
	ADCS_Comm_PowerState_Cmn_Payload_t SetVal_056 = {0,};
	ADCS_Comm_PowerState_Cmn_Payload_t RetVal_183 = {0,};
	ADCS_Comm_RawMAGSensorTlm_Paylaod_t RetVal_180 = {0,};
	ADCS_Comm_RawGYRSensorTlm_Payload_t RetVal_204 = {0,};
	ADCS_Comm_RawRWLSensorTlm_Payload_t RetVal_205 = {0,};
	ADCS_Comm_OpenLoopCmdHxyzRWCmd_Payload_t SetVal_076 = {0,};
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t SetVal_042 = {0,};
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t RetVal_150 = {0,};

	if (estimator_mode == 0)
	{
		estimator_mode = 6;
	}
	if ((estimator_mode != 5) && (estimator_mode != 6))
	{
		status = -1;
		ADCS_HandleReport(status, ADCS_SEQ_GNDPT_CC, (void *)&msg->Payload, sizeof(msg->Payload));
		return status;
	}
	if (target_duration == 0)
	{
		target_duration = 600;
	}

	// 1) Power on the sensors and reaction wheels required for target tracking.
	SetVal_056.MAG0 = 1;
	SetVal_056.GYR0 = 1;
	SetVal_056.RWL0 = 1;
	SetVal_056.RWL1 = 1;
	SetVal_056.RWL2 = 1;
	SetVal_056.RWL3 = 1;
	interstatus = ADCS_Comm_SetPowerState(&SetVal_056);
	OS_TaskDelay(500);
	interstatus += ADCS_Comm_GetPowerState(&RetVal_183);
	OS_TaskDelay(5000);

	for (cnt_try = 0; cnt_try < 3; ++cnt_try)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1) &&
			(RetVal_183.RWL0 == 1) && (RetVal_183.RWL1 == 1) &&
			(RetVal_183.RWL2 == 1) && (RetVal_183.RWL3 == 1))
		{
			break;
		}

		interstatus = ADCS_Comm_SetPowerState(&SetVal_056);
		OS_TaskDelay(500);
		interstatus += ADCS_Comm_GetPowerState(&RetVal_183);
		OS_TaskDelay(500);
	}

	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1) ||
		(RetVal_183.RWL0 != 1) || (RetVal_183.RWL1 != 1) ||
		(RetVal_183.RWL2 != 1) || (RetVal_183.RWL3 != 1))
	{
		status = -2;
		ADCS_HandleReport(status, ADCS_SEQ_GNDPT_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}

	// 2) Verify sensor and wheel telemetry before changing control modes.
	OS_TaskDelay(2000);
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);
	OS_TaskDelay(100);
	interstatus += ADCS_Comm_GetRawGYRSensor(&RetVal_204);
	OS_TaskDelay(100);
	interstatus += ADCS_Comm_GetRawRWLSensor(&RetVal_205);

	for (cnt_try = 0; cnt_try < 3; ++cnt_try)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) &&
			(RetVal_204.GYR0ValidFlag == 1) && (RetVal_205.RWL0ValidFlag == 1) &&
			(RetVal_205.RWL1ValidFlag == 1) && (RetVal_205.RWL2ValidFlag == 1) &&
			(RetVal_205.RWL3ValidFlag == 1))
		{
			break;
		}

		OS_TaskDelay(500);
		interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(100);
		interstatus += ADCS_Comm_GetRawGYRSensor(&RetVal_204);
		OS_TaskDelay(100);
		interstatus += ADCS_Comm_GetRawRWLSensor(&RetVal_205);
	}

	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1) ||
		(RetVal_204.GYR0ValidFlag != 1) || (RetVal_205.RWL0ValidFlag != 1) ||
		(RetVal_205.RWL1ValidFlag != 1) || (RetVal_205.RWL2ValidFlag != 1) ||
		(RetVal_205.RWL3ValidFlag != 1))
	{
		status = -3;
		ADCS_HandleReport(status, ADCS_SEQ_GNDPT_CC, &RetVal_205, sizeof(RetVal_205));
		return status;
	}

	// 3) Clear the open-loop wheel momentum command.
	SetVal_076.cmdHx = 0.0;
	SetVal_076.cmdHy = 0.0;
	SetVal_076.cmdHz = 0.0;
	interstatus = ADCS_Comm_SetOpenLoopCmdHxyzRW(&SetVal_076);
	OS_TaskDelay(500);
	if (interstatus != CFE_SUCCESS)
	{
		status = -8;
		ADCS_HandleReport(status, ADCS_SEQ_GNDPT_CC, &SetVal_076, sizeof(SetVal_076));
		return status;
	}

	// 4) Prepare control: mode 3 -> mode 51 -> mode 12.
	SetVal_042.MainEstimatorMode = estimator_mode;
	SetVal_042.BackupEstimatorMode = 5;
	SetVal_042.ControlMode = 3;
	SetVal_042.ControlTimeout = 0;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	OS_TaskDelay(1000);
	interstatus += ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != estimator_mode) ||
		(RetVal_150.ControlMode != 3))
	{
		status = -12;
		ADCS_HandleReport(status, ADCS_SEQ_GNDPT_CC, &RetVal_150, sizeof(RetVal_150));
		return status;
	}

	SetVal_042.ControlMode = 51;
	SetVal_042.ControlTimeout = 125;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	OS_TaskDelay(1000);
	interstatus += ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != estimator_mode) ||
		(RetVal_150.ControlMode != 51))
	{
		status = -9;
		ADCS_HandleReport(status, ADCS_SEQ_GNDPT_CC, &RetVal_150, sizeof(RetVal_150));
		SetVal_042.ControlMode = 3;
		SetVal_042.ControlTimeout = 0;
		(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		return status;
	}
	OS_TaskDelay(120000);

	SetVal_042.ControlMode = 12;
	SetVal_042.ControlTimeout = 305;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	OS_TaskDelay(1000);
	interstatus += ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != estimator_mode) ||
		(RetVal_150.ControlMode != 12))
	{
		status = -10;
		ADCS_HandleReport(status, ADCS_SEQ_GNDPT_CC, &RetVal_150, sizeof(RetVal_150));
		SetVal_042.ControlMode = 3;
		SetVal_042.ControlTimeout = 0;
		(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		return status;
	}
	OS_TaskDelay(300000);

	// 5) Start ConGndTrack (mode 16) using the preconfigured LLH and body vector.
	SetVal_042.ControlMode = ADCS_CONMODE_RW_GS_TARGET_TRACK;
	SetVal_042.ControlTimeout = target_duration;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	OS_TaskDelay(1000);
	interstatus += ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != estimator_mode) ||
		(RetVal_150.ControlMode != ADCS_CONMODE_RW_GS_TARGET_TRACK))
	{
		status = -11;
		ADCS_HandleReport(status, ADCS_SEQ_GNDPT_CC, &RetVal_150, sizeof(RetVal_150));
		SetVal_042.ControlMode = 3;
		SetVal_042.ControlTimeout = 0;
		(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		return status;
	}

	ADCS_ReportCommandPhase(ADCS_SEQ_GNDPT_CC, ADCS_RPT_PHASE_COMPLETED);
	return CFE_SUCCESS;
}

CFE_Status_t ADCS_SequenceCmd_TGTpointing(const ADCS_SequenceCmdTGTpointingCmd_t *msg)
{
	ADCS_ReportCommandPhase(ADCS_SEQ_TGT_CC, ADCS_RPT_PHASE_STARTED);

	CFE_Status_t status = CFE_SUCCESS;
	int32 interstatus = CFE_SUCCESS;
	uint8 cnt_try = 0;
	uint8 estimator_mode = msg->Payload.flag_estmode;
	uint16 target_duration = msg->Payload.target_duration;
	ADCS_Comm_PowerState_Cmn_Payload_t SetVal_056 = {0,};
	ADCS_Comm_PowerState_Cmn_Payload_t RetVal_183 = {0,};
	ADCS_Comm_RawMAGSensorTlm_Paylaod_t RetVal_180 = {0,};
	ADCS_Comm_RawGYRSensorTlm_Payload_t RetVal_204 = {0,};
	ADCS_Comm_RawRWLSensorTlm_Payload_t RetVal_205 = {0,};
	ADCS_Comm_OpenLoopCmdHxyzRWCmd_Payload_t SetVal_076 = {0,};
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t SetVal_042 = {0,};
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t RetVal_150 = {0,};

	if (estimator_mode == 0)
	{
		estimator_mode = 6;
	}
	if ((estimator_mode != 5) && (estimator_mode != 6))
	{
		status = -1;
		ADCS_HandleReport(status, ADCS_SEQ_TGT_CC, (void *)&msg->Payload, sizeof(msg->Payload));
		return status;
	}
	if (target_duration == 0)
	{
		target_duration = 600;
	}

	// 1) Power on the sensors and reaction wheels required for target tracking.
	SetVal_056.MAG0 = 1;
	SetVal_056.GYR0 = 1;
	SetVal_056.RWL0 = 1;
	SetVal_056.RWL1 = 1;
	SetVal_056.RWL2 = 1;
	SetVal_056.RWL3 = 1;
	interstatus = ADCS_Comm_SetPowerState(&SetVal_056);
	OS_TaskDelay(500);
	interstatus += ADCS_Comm_GetPowerState(&RetVal_183);
	OS_TaskDelay(5000);

	for (cnt_try = 0; cnt_try < 3; ++cnt_try)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1) &&
			(RetVal_183.RWL0 == 1) && (RetVal_183.RWL1 == 1) &&
			(RetVal_183.RWL2 == 1) && (RetVal_183.RWL3 == 1))
		{
			break;
		}

		interstatus = ADCS_Comm_SetPowerState(&SetVal_056);
		OS_TaskDelay(500);
		interstatus += ADCS_Comm_GetPowerState(&RetVal_183);
		OS_TaskDelay(500);
	}

	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1) ||
		(RetVal_183.RWL0 != 1) || (RetVal_183.RWL1 != 1) ||
		(RetVal_183.RWL2 != 1) || (RetVal_183.RWL3 != 1))
	{
		status = -2;
		ADCS_HandleReport(status, ADCS_SEQ_TGT_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}

	// 2) Verify sensor and wheel telemetry before changing control modes.
	OS_TaskDelay(2000);
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);
	OS_TaskDelay(100);
	interstatus += ADCS_Comm_GetRawGYRSensor(&RetVal_204);
	OS_TaskDelay(100);
	interstatus += ADCS_Comm_GetRawRWLSensor(&RetVal_205);

	for (cnt_try = 0; cnt_try < 3; ++cnt_try)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) &&
			(RetVal_204.GYR0ValidFlag == 1) && (RetVal_205.RWL0ValidFlag == 1) &&
			(RetVal_205.RWL1ValidFlag == 1) && (RetVal_205.RWL2ValidFlag == 1) &&
			(RetVal_205.RWL3ValidFlag == 1))
		{
			break;
		}

		OS_TaskDelay(500);
		interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(100);
		interstatus += ADCS_Comm_GetRawGYRSensor(&RetVal_204);
		OS_TaskDelay(100);
		interstatus += ADCS_Comm_GetRawRWLSensor(&RetVal_205);
	}

	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1) ||
		(RetVal_204.GYR0ValidFlag != 1) || (RetVal_205.RWL0ValidFlag != 1) ||
		(RetVal_205.RWL1ValidFlag != 1) || (RetVal_205.RWL2ValidFlag != 1) ||
		(RetVal_205.RWL3ValidFlag != 1))
	{
		status = -3;
		ADCS_HandleReport(status, ADCS_SEQ_TGT_CC, &RetVal_205, sizeof(RetVal_205));
		return status;
	}

	// 3) Clear the open-loop wheel momentum command.
	SetVal_076.cmdHx = 0.0;
	SetVal_076.cmdHy = 0.0;
	SetVal_076.cmdHz = 0.0;
	interstatus = ADCS_Comm_SetOpenLoopCmdHxyzRW(&SetVal_076);
	OS_TaskDelay(500);
	if (interstatus != CFE_SUCCESS)
	{
		status = -8;
		ADCS_HandleReport(status, ADCS_SEQ_TGT_CC, &SetVal_076, sizeof(SetVal_076));
		return status;
	}

	// 4) Prepare control: mode 3 -> mode 51 -> mode 12.
	SetVal_042.MainEstimatorMode = estimator_mode;
	SetVal_042.BackupEstimatorMode = 5;
	SetVal_042.ControlMode = 3;
	SetVal_042.ControlTimeout = 0;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	OS_TaskDelay(1000);
	interstatus += ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != estimator_mode) ||
		(RetVal_150.ControlMode != 3))
	{
		status = -12;
		ADCS_HandleReport(status, ADCS_SEQ_TGT_CC, &RetVal_150, sizeof(RetVal_150));
		return status;
	}

	SetVal_042.ControlMode = 51;
	SetVal_042.ControlTimeout = 125;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	OS_TaskDelay(1000);
	interstatus += ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != estimator_mode) ||
		(RetVal_150.ControlMode != 51))
	{
		status = -9;
		ADCS_HandleReport(status, ADCS_SEQ_TGT_CC, &RetVal_150, sizeof(RetVal_150));
		SetVal_042.ControlMode = 3;
		SetVal_042.ControlTimeout = 0;
		(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		return status;
	}
	OS_TaskDelay(120000);

	SetVal_042.ControlMode = 12;
	SetVal_042.ControlTimeout = 305;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	OS_TaskDelay(1000);
	interstatus += ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != estimator_mode) ||
		(RetVal_150.ControlMode != 12))
	{
		status = -10;
		ADCS_HandleReport(status, ADCS_SEQ_TGT_CC, &RetVal_150, sizeof(RetVal_150));
		SetVal_042.ControlMode = 3;
		SetVal_042.ControlTimeout = 0;
		(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		return status;
	}
	OS_TaskDelay(300000);

	// 5) Start ConTgtTrack (mode 14) using the preconfigured LLH and fixed +Z_B axis.
	SetVal_042.ControlMode = ADCS_CONMODE_RW_EO_TARGET_TRACK;
	SetVal_042.ControlTimeout = target_duration;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	OS_TaskDelay(1000);
	interstatus += ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != estimator_mode) ||
		(RetVal_150.ControlMode != ADCS_CONMODE_RW_EO_TARGET_TRACK))
	{
		status = -14;
		ADCS_HandleReport(status, ADCS_SEQ_TGT_CC, &RetVal_150, sizeof(RetVal_150));
		SetVal_042.ControlMode = 3;
		SetVal_042.ControlTimeout = 0;
		(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		return status;
	}

	ADCS_ReportCommandPhase(ADCS_SEQ_TGT_CC, ADCS_RPT_PHASE_COMPLETED);
	return CFE_SUCCESS;
}

CFE_Status_t ADCS_SequenceCmd_Nadirpointing(const ADCS_SequenceCmdNadirpointingCmd_t *msg)
{
	ADCS_ReportCommandPhase(ADCS_SEQ_NADIR_CC, ADCS_RPT_PHASE_STARTED);

	// Nadir Pointing with Yaw Towards Ground Target
	// [Procedure]
	// 1) Power on MAG0, GYR0, and RWL0..3.
	// 2) Check sensor and wheel status.
	// 3) Clear the open-loop wheel momentum command.
	// 4) Set estimation/control modes: mode 3 -> mode 51 -> mode 12.
	// 5) Enter mode 21 using the preconfigured orbit, Unix time, and reference LLH.

	CFE_Status_t status = CFE_SUCCESS;
	int32 interstatus = CFE_SUCCESS;
	uint8 cnt_try = 0;
	uint8 estimator_mode = msg->Payload.flag_estmode;
	uint16 target_duration = msg->Payload.target_duration;
	ADCS_Comm_PowerState_Cmn_Payload_t SetVal_056 = {0,};
	ADCS_Comm_PowerState_Cmn_Payload_t RetVal_183 = {0,};
	ADCS_Comm_RawMAGSensorTlm_Paylaod_t RetVal_180 = {0,};
	ADCS_Comm_RawGYRSensorTlm_Payload_t RetVal_204 = {0,};
	ADCS_Comm_RawRWLSensorTlm_Payload_t RetVal_205 = {0,};
	ADCS_Comm_OpenLoopCmdHxyzRWCmd_Payload_t SetVal_076 = {0,};
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t SetVal_042 = {0,};
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t RetVal_150 = {0,};

	if (estimator_mode == 0)
	{
		estimator_mode = 6;
	}
	if ((estimator_mode != 5) && (estimator_mode != 6))
	{
		status = -1;
		ADCS_HandleReport(status, ADCS_SEQ_NADIR_CC, (void *)&msg->Payload, sizeof(msg->Payload));
		return status;
	}
	if (target_duration == 0)
	{
		target_duration = 600;
	}

	// Orbit, Unix time, and reference target LLH must be set before this sequence.
	// 1) Power on the sensors and reaction wheels required for nadir tracking.
	SetVal_056.MAG0 = 1;
	SetVal_056.GYR0 = 1;
	SetVal_056.RWL0 = 1;
	SetVal_056.RWL1 = 1;
	SetVal_056.RWL2 = 1;
	SetVal_056.RWL3 = 1;
	interstatus = ADCS_Comm_SetPowerState(&SetVal_056);
	OS_TaskDelay(500);
	interstatus += ADCS_Comm_GetPowerState(&RetVal_183);
	OS_TaskDelay(5000);

	for (cnt_try = 0; cnt_try < 3; ++cnt_try)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1) &&
			(RetVal_183.RWL0 == 1) && (RetVal_183.RWL1 == 1) &&
			(RetVal_183.RWL2 == 1) && (RetVal_183.RWL3 == 1))
		{
			break;
		}

		interstatus = ADCS_Comm_SetPowerState(&SetVal_056);
		OS_TaskDelay(500);
		interstatus += ADCS_Comm_GetPowerState(&RetVal_183);
		OS_TaskDelay(500);
	}

	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1) ||
		(RetVal_183.RWL0 != 1) || (RetVal_183.RWL1 != 1) ||
		(RetVal_183.RWL2 != 1) || (RetVal_183.RWL3 != 1))
	{
		status = -2;
		ADCS_HandleReport(status, ADCS_SEQ_NADIR_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}

	// 2) Verify sensor and wheel telemetry before changing control modes.
	OS_TaskDelay(2000);
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);
	OS_TaskDelay(100);
	interstatus += ADCS_Comm_GetRawGYRSensor(&RetVal_204);
	OS_TaskDelay(100);
	interstatus += ADCS_Comm_GetRawRWLSensor(&RetVal_205);

	for (cnt_try = 0; cnt_try < 3; ++cnt_try)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) &&
			(RetVal_204.GYR0ValidFlag == 1) && (RetVal_205.RWL0ValidFlag == 1) &&
			(RetVal_205.RWL1ValidFlag == 1) && (RetVal_205.RWL2ValidFlag == 1) &&
			(RetVal_205.RWL3ValidFlag == 1))
		{
			break;
		}

		OS_TaskDelay(500);
		interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(100);
		interstatus += ADCS_Comm_GetRawGYRSensor(&RetVal_204);
		OS_TaskDelay(100);
		interstatus += ADCS_Comm_GetRawRWLSensor(&RetVal_205);
	}

	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1) ||
		(RetVal_204.GYR0ValidFlag != 1) || (RetVal_205.RWL0ValidFlag != 1) ||
		(RetVal_205.RWL1ValidFlag != 1) || (RetVal_205.RWL2ValidFlag != 1) ||
		(RetVal_205.RWL3ValidFlag != 1))
	{
		status = -3;
		ADCS_HandleReport(status, ADCS_SEQ_NADIR_CC, &RetVal_205, sizeof(RetVal_205));
		return status;
	}

	// 3) Clear the open-loop wheel momentum command.
	SetVal_076.cmdHx = 0.0;
	SetVal_076.cmdHy = 0.0;
	SetVal_076.cmdHz = 0.0;
	interstatus = ADCS_Comm_SetOpenLoopCmdHxyzRW(&SetVal_076);
	OS_TaskDelay(500);
	if (interstatus != CFE_SUCCESS)
	{
		status = -8;
		ADCS_HandleReport(status, ADCS_SEQ_NADIR_CC, &SetVal_076, sizeof(SetVal_076));
		return status;
	}

	// 4) Prepare control: mode 3 -> mode 51 -> mode 12.
	SetVal_042.MainEstimatorMode = estimator_mode;
	SetVal_042.BackupEstimatorMode = 5;
	SetVal_042.ControlMode = 3;
	SetVal_042.ControlTimeout = 0;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	OS_TaskDelay(1000);
	interstatus += ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != estimator_mode) ||
		(RetVal_150.ControlMode != 3))
	{
		status = -12;
		ADCS_HandleReport(status, ADCS_SEQ_NADIR_CC, &RetVal_150, sizeof(RetVal_150));
		return status;
	}

	SetVal_042.ControlMode = 51;
	SetVal_042.ControlTimeout = 125;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	OS_TaskDelay(1000);
	interstatus += ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != estimator_mode) ||
		(RetVal_150.ControlMode != 51))
	{
		status = -9;
		ADCS_HandleReport(status, ADCS_SEQ_NADIR_CC, &RetVal_150, sizeof(RetVal_150));
		SetVal_042.ControlMode = 3;
		SetVal_042.ControlTimeout = 0;
		(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		return status;
	}
	OS_TaskDelay(120000);

	SetVal_042.ControlMode = 12;
	SetVal_042.ControlTimeout = 305;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	OS_TaskDelay(1000);
	interstatus += ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != estimator_mode) ||
		(RetVal_150.ControlMode != 12))
	{
		status = -10;
		ADCS_HandleReport(status, ADCS_SEQ_NADIR_CC, &RetVal_150, sizeof(RetVal_150));
		SetVal_042.ControlMode = 3;
		SetVal_042.ControlTimeout = 0;
		(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		return status;
	}
	OS_TaskDelay(300000);

	// 5) Start nadir pointing with yaw towards the configured ground target (mode 21).
	SetVal_042.ControlMode = ADCS_CONMODE_NADIR_YAW_GROUND_TARGET;
	SetVal_042.ControlTimeout = target_duration;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	OS_TaskDelay(1000);
	interstatus += ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != estimator_mode) ||
		(RetVal_150.ControlMode != ADCS_CONMODE_NADIR_YAW_GROUND_TARGET))
	{
		status = -15;
		ADCS_HandleReport(status, ADCS_SEQ_NADIR_CC, &RetVal_150, sizeof(RetVal_150));
		SetVal_042.ControlMode = 3;
		SetVal_042.ControlTimeout = 0;
		(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		return status;
	}

	ADCS_ReportCommandPhase(ADCS_SEQ_NADIR_CC, ADCS_RPT_PHASE_COMPLETED);
	return CFE_SUCCESS;
}

/********************************************************
 * 
 * BEE1006 Actual Commissioing Sequence Function
 * 
 ********************************************************/

CFE_Status_t ADCS_Comm01Cmd(const ADCS_Comm01Cmd_t *msg) {
	ADCS_ReportCommandPhase(ADCS_COMM_01_CC, ADCS_RPT_PHASE_STARTED);

	// COMM 01:	Determine initial angular rates
	// [Procedure]
	// 1) Power On ( GYR0, MAG0 )
	// 2) Set Est. mode ( Main: EstGyro (1) / Backup: EstMagRkf (2) )
	// 3) Check Status of Sensors
	// 4) Make "adcs_comm_01.bin" file to log the TLMs
	// 5) Start to count time ( Waiting time before Logging: 10 sec / Tlm period: 10 sec / Comm. Duration: 300 sec )
	// 		* Backup Estimation mode - EstMagRkf needs 10 seconds for convergence!
	// 6) Read Tlm of Estimator & Sensors
	// 		6-1) Read Tlm of Main / Backup Estimator:
	// 				Time integer seconds
	// 				Time nanoseconds
	// 				Estimated body rate (ORC) X-Y-Z
	// 				Estimated body rate (IRC) X-Y-Z
	// 				Active estimator mode
	// 		6-2) Read Sensor Raw Values - Gyro & Magnetometer
	// 				Time integer seconds
	// 				Time nanoseconds
	// 				GYR0 / MAG0 raw X-Y-Z component
	// 				GYR0 / MAG0 valid flag
	// 7) Append the Tlm to the file
	// 8) Check time from beginning of Step 6 to end of Step 7 (dt) -> Wait the remain time (10 - dt)
	// 9) Go back to Step 6 until total time meets 300 sec
	// ** Status between every step must be reported!!


	CFE_Status_t						status;
	CFE_Status_t						interstatus;
	CFE_Status_t                        OverallStatus = CFE_SUCCESS;

	uint8 cnt_try = 0;
	uint8 flag_tlmtype = msg->Payload.flag_tlmtype;
	if ((flag_tlmtype != 0) && (flag_tlmtype != 1))
	{
		status = -1;		// Something is wrong!
		ADCS_HandleReport(status, ADCS_COMM_01_CC, &flag_tlmtype, sizeof(flag_tlmtype));
		return status;
	}

	// 1) Power On ( GYR0, MAG0 )
	ADCS_Comm_PowerState_Cmn_Payload_t		SetVal_056 = {0,};
	ADCS_Comm_PowerState_Cmn_Payload_t		RetVal_183 = {0,};
	SetVal_056.MAG0		= 1;
	SetVal_056.GYR0		= 1;
	interstatus = ADCS_Comm_SetPowerState(&SetVal_056);		// Set GYR0 & MAG0 --> ON
	OS_TaskDelay(10);
	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);		// Get GYR0 & MAG0 power state
	OS_TaskDelay(5000);
	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1)) break;	// Check Power On well

		// if not, try again until 3 times
		interstatus = ADCS_Comm_SetPowerState(&SetVal_056);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
		cnt_try++;
	}

	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -1;		// Something is wrong! ::::: Power State
		ADCS_HandleReport(status, ADCS_COMM_01_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}


	// 3) Check Status of Sensors
	ADCS_Comm_RawMAGSensorTlm_Paylaod_t	RetVal_180 = {0,};
	ADCS_Comm_RawGYRSensorTlm_Payload_t	RetVal_204 = {0,};

	OS_TaskDelay(1000);
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS_Comm_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro

	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1)) break;	// Check Sensors Status Good

		// if not, wait changing status until 3 times
		OS_TaskDelay(100);
		interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS_Comm_GetRawGYRSensor(&RetVal_204);
		cnt_try++;
	}

	
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -3;		// Something is wrong! ::::: Magnetometer Status
		ADCS_HandleReport(status, ADCS_COMM_01_CC, &RetVal_180, sizeof(RetVal_180));
		return status;
	}
	OS_TaskDelay(10);
	interstatus = ADCS_Comm_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	if ((interstatus != CFE_SUCCESS) || (RetVal_204.GYR0ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -4;		// Something is wrong! ::::: Gyro Status
		ADCS_HandleReport(status, ADCS_COMM_01_CC, &RetVal_204, sizeof(RetVal_204));
		return status;
	}


	// 2) Set Est. mode ( Main: EstGyro (1) / Backup: EstMagRkf (2) )
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t	SetVal_042 = {0,};
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t	RetVal_150 = {0,};

	SetVal_042.MainEstimatorMode 	= 1;
	SetVal_042.BackupEstimatorMode 	= 2;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);	// Set Main & Backup Est mode = 1 & 2
	OS_TaskDelay(100);
	interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);	// Get Main & Backup Est mode
	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_150.MainEstimatorMode == 1) && (RetVal_150.BackupEstimatorMode == 2)) break;	// Check Mode Change well

		// if not, try again until 3 times
		interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);
		cnt_try++;
	}

	interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != 1) || (RetVal_150.BackupEstimatorMode != 2))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -2;		// Something is wrong! ::::: Estimation Mode
		ADCS_HandleReport(status, ADCS_COMM_01_CC, &RetVal_150, sizeof(RetVal_150));
		return status;

	}

	// 4) Make "adcs_comm_01.txt" file to log the TLMs
	FILE *fp;
	size_t written;
	fp = fopen("./cf/adcs_comm_01.bin","wb");
	if (fp == NULL)
	{
		status = -5;		// Something is wrong! ::::: File open error
		ADCS_HandleReport(status, ADCS_COMM_01_CC, NULL, 0);
		return status;
	}

	// 5) Start to count time ( Waiting time before Logging: 10 sec / Tlm period: 10 sec / Comm. Duration: 300 sec )
	CFE_TIME_SysTime_t		t0_10, t0_300, tnow;
	t0_10 = CFE_TIME_GetTime();

	uint32 dt = 0;
	while (dt < 10)
	{	// Wait until dt > 10 sec
		OS_TaskDelay(100);
		tnow = CFE_TIME_GetTime();
		dt = tnow.Seconds - t0_10.Seconds;
	}

	// 6) Read Tlm of Estimator & Sensors
	ADCS_Comm_Estimator_Cmn_Payload_t	RetVal_210 = {0,};
	ADCS_Comm_Estimator_Cmn_Payload_t	RetVal_173 = {0,};

	ADCS_Comm_COMM_01_COMP_Payload_t	Comm_01_Tlm_Set_Comp = {0,};
	ADCS_Comm_COMM_01_FULL_Payload_t	Comm_01_Tlm_Set_Full = {0,};

	Comm_01_Tlm_Set_Comp.sync_word 	= 0xADC5;
	Comm_01_Tlm_Set_Full.sync_word 	= 0xADC5;

	t0_300 = CFE_TIME_GetTime();
	uint32 dt300 = 0;
	while (dt300 < 300)
	{
		t0_10 = CFE_TIME_GetTime();	// Initialize 10 seconds Counter
		dt = 0;	// Initialize time gap


		memset(&RetVal_210, 0, sizeof(RetVal_210));
		memset(&RetVal_173, 0, sizeof(RetVal_173));
		memset(&RetVal_180, 0, sizeof(RetVal_180));
		memset(&RetVal_204, 0, sizeof(RetVal_204));
		
		// Comm_01_Tlm_Set.sync_word 	= 0xADC5;

		// 		6-1) Read Tlm of Main / Backup Estimator:	
		interstatus = ADCS_Comm_GetMainEstTlm(&RetVal_210);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetBackupEstTlm(&RetVal_173);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(10);

		// 		6-2) Read Sensor Raw Values - Gyro & Magnetometer	
		interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetRawGYRSensor(&RetVal_204);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(10);
		
		// 7) Write the Tlm to the file
		if (flag_tlmtype == 0)	// Compact telemetry set
		{			
			
			Comm_01_Tlm_Set_Comp.MainEst_TimeSecond		= RetVal_210.TimeSecond;
			Comm_01_Tlm_Set_Comp.MainEst_TimeNanoSecond	= RetVal_210.TimeNanoSecond;
			Comm_01_Tlm_Set_Comp.MainEst_EstORCBodyRateX	= RetVal_210.EstORCBodyRateX;
			Comm_01_Tlm_Set_Comp.MainEst_EstORCBodyRateY	= RetVal_210.EstORCBodyRateY;
			Comm_01_Tlm_Set_Comp.MainEst_EstORCBodyRateZ	= RetVal_210.EstORCBodyRateZ;
			Comm_01_Tlm_Set_Comp.MainEst_EstIRCBodyRateX	= RetVal_210.EstIRCBodyRateX;
			Comm_01_Tlm_Set_Comp.MainEst_EstIRCBodyRateY	= RetVal_210.EstIRCBodyRateY;
			Comm_01_Tlm_Set_Comp.MainEst_EstIRCBodyRateZ	= RetVal_210.EstIRCBodyRateZ;
			Comm_01_Tlm_Set_Comp.MainEst_ActiveEstMode	= RetVal_210.ActiveEstMode;
			
			Comm_01_Tlm_Set_Comp.BackupEst_TimeSecond		= RetVal_173.TimeSecond;
			Comm_01_Tlm_Set_Comp.BackupEst_TimeNanoSecond	= RetVal_173.TimeNanoSecond;
			Comm_01_Tlm_Set_Comp.BackupEst_EstORCBodyRateX	= RetVal_173.EstORCBodyRateX;
			Comm_01_Tlm_Set_Comp.BackupEst_EstORCBodyRateY	= RetVal_173.EstORCBodyRateY;
			Comm_01_Tlm_Set_Comp.BackupEst_EstORCBodyRateZ	= RetVal_173.EstORCBodyRateZ;
			Comm_01_Tlm_Set_Comp.BackupEst_EstIRCBodyRateX	= RetVal_173.EstIRCBodyRateX;
			Comm_01_Tlm_Set_Comp.BackupEst_EstIRCBodyRateY	= RetVal_173.EstIRCBodyRateY;
			Comm_01_Tlm_Set_Comp.BackupEst_EstIRCBodyRateZ	= RetVal_173.EstIRCBodyRateZ;
			Comm_01_Tlm_Set_Comp.BackupEst_ActiveEstMode		= RetVal_173.ActiveEstMode;

			Comm_01_Tlm_Set_Comp.GYR0_TimeSeconds = RetVal_204.TimeSeconds;
			Comm_01_Tlm_Set_Comp.GYR0_TimeNanoSeconds = RetVal_204.TimeNanoSeconds;
			Comm_01_Tlm_Set_Comp.GYR0RawRateX = RetVal_204.GYR0RawRateX;
			Comm_01_Tlm_Set_Comp.GYR0RawRateY = RetVal_204.GYR0RawRateY;
			Comm_01_Tlm_Set_Comp.GYR0RawRateZ = RetVal_204.GYR0RawRateZ;
			Comm_01_Tlm_Set_Comp.GYR0ValidFlag = RetVal_204.GYR0ValidFlag;
			
			Comm_01_Tlm_Set_Comp.MAG0_TimeSeconds = RetVal_180.TimeSeconds;
			Comm_01_Tlm_Set_Comp.MAG0_TimeNanoSeconds = RetVal_180.TimeNanoSeconds;
			Comm_01_Tlm_Set_Comp.MAG0RawVecX = RetVal_180.MAG0RawVecX;
			Comm_01_Tlm_Set_Comp.MAG0RawVecY = RetVal_180.MAG0RawVecY;
			Comm_01_Tlm_Set_Comp.MAG0RawVecZ = RetVal_180.MAG0RawVecZ;
			Comm_01_Tlm_Set_Comp.MAG0ValidFlag = RetVal_180.MAG0ValidFlag;


			if ((written = fwrite(&Comm_01_Tlm_Set_Comp, sizeof(Comm_01_Tlm_Set_Comp), 1, fp)) != 1)
			{
				status = -6;		// Something is wrong! ::::: File write error
				ADCS_HandleReport(status, ADCS_COMM_01_CC, NULL, 0);
				fclose(fp);
				return status;
			}
		}
		else if (flag_tlmtype == 1) // Full telemetry set
		{			
			
			Comm_01_Tlm_Set_Full.MainEst 	= RetVal_210;
			Comm_01_Tlm_Set_Full.BackupEst 	= RetVal_173;
			Comm_01_Tlm_Set_Full.GYR0 		= RetVal_204;
			Comm_01_Tlm_Set_Full.MAG0 		= RetVal_180;

				if ((written = fwrite(&Comm_01_Tlm_Set_Full, sizeof(Comm_01_Tlm_Set_Full), 1, fp)) != 1)
			{
				status = -6;		// Something is wrong! ::::: File write error
				ADCS_HandleReport(status, ADCS_COMM_01_CC, NULL, 0);
				fclose(fp);
				return status;
			}
		}


		// 8) Check time from beginning of Step 6 to end of Step 7 (dt) -> Wait the remain time (10 - dt)
		tnow = CFE_TIME_GetTime();
		dt300 = tnow.Seconds - t0_300.Seconds;
		dt = tnow.Seconds - t0_10.Seconds;

		while (dt < 10)
		{	// Wait until dt > 10 sec
			OS_TaskDelay(100);
			tnow = CFE_TIME_GetTime();
			dt = tnow.Seconds - t0_10.Seconds;
		}
		
	// 9) Go back to Step 6 until total time meets 300 sec
	}
	if (fclose(fp) != 0)
	{
		ADCS_RecordFirstFailure(&OverallStatus, CFE_STATUS_EXTERNAL_RESOURCE_FAIL);
	}

	return ADCS_ReportCommandCompletion(ADCS_COMM_01_CC, OverallStatus);
}

CFE_Status_t ADCS_Comm02Cmd(const ADCS_Comm02Cmd_t *msg) {
	ADCS_ReportCommandPhase(ADCS_COMM_02_CC, ADCS_RPT_PHASE_STARTED);

	// COMM 02:	Detumbling
	// [Procedure]
	// 1) Power On ( GYR0, MAG0 )
	// 2) Set Est. mode & Cont. mode ( Main: EstGyro (1) / Backup: EstMagRkf (2) / Control: ConBdot or ConBdot3 / Timeout: 600 s )
	// 3) Check Status of Sensors
	// 4) Make "adcs_comm_02.bin" file to log the TLMs
	// 5) Start to count time ( Waiting time before Logging: 1 sec / Tlm period: 10 sec / Comm. Duration: 600 sec )
	// 		* Backup Estimation mode is not considered, so the convergence time is not required.
	// 6) Read Tlm of Estimator & Sensors
	// 		6-1) Read Tlm of Main Estimator:
	// 				Time integer seconds
	// 				Time nanoseconds
	// 				Estimated body rate (ORC) X-Y-Z
	// 				Estimated body rate (IRC) X-Y-Z
	// 				Active estimator mode
	// 		6-2) Read Sensor Raw Values - Gyro & Magnetometer & CSS
	// 				Time integer seconds
	// 				Time nanoseconds
	// 				GYR0 / MAG0 raw X-Y-Z component
	// 				GYR0 / MAG0 valid flag
	//		6-3) Read Tlm of Controller:
	// 				Time integer seconds
	// 				Time nanoseconds
	// 				Control timeout
	// 				Active control mode
	// 7) Append the Tlm to the file
	// 8) Check time from beginning of Step 6 to end of Step 7 (dt) -> Wait the remain time (10 - dt)
	// 9) Go back to Step 6 until total time meets 600 sec
	// ** Status between every step must be reported!!


	CFE_Status_t						status;
	CFE_Status_t						interstatus;
	CFE_Status_t                        OverallStatus = CFE_SUCCESS;

	uint8 cnt_try = 0;
	uint8 flag_tlmtype = msg->Payload.flag_tlmtype;
	uint8 flag_contmode = msg->Payload.flag_contmode;

	if ((flag_contmode != 0) && (flag_contmode != 1) && (flag_contmode != 3) && (flag_contmode != 4)) 
	{
		status = -2;
		ADCS_HandleReport(status, ADCS_COMM_02_CC, &flag_contmode, sizeof(flag_contmode));
		return status;
	}
	
	if ((flag_tlmtype != 0) && (flag_tlmtype != 1))
	{
		status = -1;		// Something is wrong!
		ADCS_HandleReport(status, ADCS_COMM_02_CC, &flag_tlmtype, sizeof(flag_tlmtype));
		return status;
	}

	// 1) Power On ( GYR0, MAG0 )
	ADCS_Comm_PowerState_Cmn_Payload_t		SetVal_056 = {0,};
	ADCS_Comm_PowerState_Cmn_Payload_t		RetVal_183 = {0,};
	SetVal_056.MAG0		= 1;
	SetVal_056.GYR0		= 1;
	interstatus = ADCS_Comm_SetPowerState(&SetVal_056);		// Set GYR0 & MAG0 --> ON
	OS_TaskDelay(10);
	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);		// Get GYR0 & MAG0 power state
	OS_TaskDelay(5000);
	cnt_try = 0;
	while (cnt_try < 3)
	{		
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1)) break;	// Check Power On well

		// if not, try again until 3 times
		interstatus = ADCS_Comm_SetPowerState(&SetVal_056);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
		cnt_try++;
	}

	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -1;		// Something is wrong! ::::: Power State
		ADCS_HandleReport(status, ADCS_COMM_02_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}


	// 3) Check Status of Sensors
	ADCS_Comm_RawMAGSensorTlm_Paylaod_t	RetVal_180 = {0,};
	ADCS_Comm_RawGYRSensorTlm_Payload_t	RetVal_204 = {0,};
	OS_TaskDelay(1000);
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS_Comm_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro

	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1)) break;	// Check Sensors Status Good

		// if not, wait changing status until 3 times
		OS_TaskDelay(100);
		interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS_Comm_GetRawGYRSensor(&RetVal_204);
		cnt_try++;
	}

	
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -3;		// Something is wrong! ::::: Magnetometer Status
		ADCS_HandleReport(status, ADCS_COMM_02_CC, &RetVal_180, sizeof(RetVal_180));
		return status;
	}
	OS_TaskDelay(10);
	interstatus = ADCS_Comm_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	if ((interstatus != CFE_SUCCESS) || (RetVal_204.GYR0ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -4;		// Something is wrong! ::::: Gyro Status
		ADCS_HandleReport(status, ADCS_COMM_02_CC, &RetVal_204, sizeof(RetVal_204));
		return status;
	}

	// 2) Set Est. mode & Cont. mode ( Main: EstGyro (1) / Backup: EstMagRkf (2) / Control: ConBdot or ConBdot3 / Timeout: 600 s )
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t	SetVal_042 = {0,};
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t	RetVal_150 = {0,};

	SetVal_042.MainEstimatorMode 	= 1;
	SetVal_042.BackupEstimatorMode 	= 2;
	SetVal_042.ControlMode 			= flag_contmode;
	SetVal_042.ControlTimeout		= 600;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);	// Set Main & Backup Est mode = 1 & 2 / Control mode & Time out = 0 or 1 or 3 or 4 & 600 s
	OS_TaskDelay(100);
	interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);	// Get Main & Backup Est mode
	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_150.MainEstimatorMode == 1) && (RetVal_150.ControlMode == flag_contmode)) break;	// Check Mode Change well

		// if not, try again until 3 times
		interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);
		cnt_try++;
	}

	interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != 1) || (RetVal_150.ControlMode != flag_contmode))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -2;		// Something is wrong! ::::: Estimation Mode
		ADCS_HandleReport(status, ADCS_COMM_02_CC, &RetVal_150, sizeof(RetVal_150));
		return status;

	}


	// 4) Make "adcs_comm_02.txt" file to log the TLMs
	FILE *fp;
	size_t written;
	fp = fopen("./cf/adcs_comm_02.bin","wb");
	if (fp == NULL)
	{
		status = -5;		// Something is wrong! ::::: File open error
		ADCS_HandleReport(status, ADCS_COMM_02_CC, NULL, 0);
		return status;
	}

	// 5) Start to count time ( Waiting time before Logging: 1 sec / Tlm period: 10 sec / Comm. Duration: 600 sec )
	CFE_TIME_SysTime_t		t0_10, t0_600, tnow;
	t0_10 = CFE_TIME_GetTime();

	uint32 dt = 0;
	while (dt < 1)
	{	// Wait until dt > 1 sec
		OS_TaskDelay(100);
		tnow = CFE_TIME_GetTime();
		dt = tnow.Seconds - t0_10.Seconds;
	}

	// 6) Read Tlm of Estimator & Sensors	
	ADCS_Comm_Estimator_Cmn_Payload_t		RetVal_210 = {0,};
	ADCS_Comm_RawCSSSensorTlm_Payload_t		RetVal_203 = {0,};
	ADCS_Comm_ControllerTlm_Payload_t		RetVal_172 = {0,};

	ADCS_Comm_COMM_02_COMP_Payload_t	Comm_02_Tlm_Set_Comp = {0,};
	ADCS_Comm_COMM_02_FULL_Payload_t	Comm_02_Tlm_Set_Full = {0,};

	Comm_02_Tlm_Set_Comp.sync_word 	= 0xADC5;
	Comm_02_Tlm_Set_Full.sync_word 	= 0xADC5;

	t0_600 = CFE_TIME_GetTime();
	uint32 dt600 = 0;
	while (dt600 < 600)
	{
		t0_10 = CFE_TIME_GetTime();	// Initialize 10 seconds Counter
		dt = 0;	// Initialize time gap
		
		memset(&RetVal_210, 0, sizeof(RetVal_210));
		memset(&RetVal_180, 0, sizeof(RetVal_180));
		memset(&RetVal_204, 0, sizeof(RetVal_204));
		memset(&RetVal_203, 0, sizeof(RetVal_203));
		memset(&RetVal_172, 0, sizeof(RetVal_172));
		

		// 		6-1) Read Tlm of Main
		interstatus = ADCS_Comm_GetMainEstTlm(&RetVal_210);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(10);

		// 		6-2) Read Sensor Raw Values - Gyro & Magnetometer	
		interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetRawGYRSensor(&RetVal_204);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetRawCSSSensor(&RetVal_203);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(10);

		//		6-3) Read Tlm of Controller
		interstatus = ADCS_Comm_GetControllerTlm(&RetVal_172);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(10);
		
		// 7) Write the Tlm to the file
		if (flag_tlmtype == 0)	// Compact telemetry set
		{
			
			Comm_02_Tlm_Set_Comp.MainEst_TimeSecond		= RetVal_210.TimeSecond;
			Comm_02_Tlm_Set_Comp.MainEst_TimeNanoSecond	= RetVal_210.TimeNanoSecond;
			Comm_02_Tlm_Set_Comp.MainEst_EstIRCBodyRateX	= RetVal_210.EstIRCBodyRateX;
			Comm_02_Tlm_Set_Comp.MainEst_EstIRCBodyRateY	= RetVal_210.EstIRCBodyRateY;
			Comm_02_Tlm_Set_Comp.MainEst_EstIRCBodyRateZ	= RetVal_210.EstIRCBodyRateZ;
			Comm_02_Tlm_Set_Comp.MainEst_ActiveEstMode	= RetVal_210.ActiveEstMode;
			
			Comm_02_Tlm_Set_Comp.GYR0_TimeSeconds = RetVal_204.TimeSeconds;
			Comm_02_Tlm_Set_Comp.GYR0_TimeNanoSeconds = RetVal_204.TimeNanoSeconds;
			Comm_02_Tlm_Set_Comp.GYR0RawRateX = RetVal_204.GYR0RawRateX;
			Comm_02_Tlm_Set_Comp.GYR0RawRateY = RetVal_204.GYR0RawRateY;
			Comm_02_Tlm_Set_Comp.GYR0RawRateZ = RetVal_204.GYR0RawRateZ;
			Comm_02_Tlm_Set_Comp.GYR0ValidFlag = RetVal_204.GYR0ValidFlag;
			
			Comm_02_Tlm_Set_Comp.MAG0_TimeSeconds = RetVal_180.TimeSeconds;
			Comm_02_Tlm_Set_Comp.MAG0_TimeNanoSeconds = RetVal_180.TimeNanoSeconds;
			Comm_02_Tlm_Set_Comp.MAG0RawVecX = RetVal_180.MAG0RawVecX;
			Comm_02_Tlm_Set_Comp.MAG0RawVecY = RetVal_180.MAG0RawVecY;
			Comm_02_Tlm_Set_Comp.MAG0RawVecZ = RetVal_180.MAG0RawVecZ;
			Comm_02_Tlm_Set_Comp.MAG0ValidFlag = RetVal_180.MAG0ValidFlag;

			Comm_02_Tlm_Set_Comp.CSS_TimeSeconds = RetVal_203.TimeSeconds;
			Comm_02_Tlm_Set_Comp.CSS_TimeNanoSeconds = RetVal_203.TimeNanoSeconds;
			Comm_02_Tlm_Set_Comp.CSS0 = RetVal_203.CSS0;
			Comm_02_Tlm_Set_Comp.CSS1 = RetVal_203.CSS1;
			Comm_02_Tlm_Set_Comp.CSS2 = RetVal_203.CSS2;
			Comm_02_Tlm_Set_Comp.CSS3 = RetVal_203.CSS3;
			Comm_02_Tlm_Set_Comp.CSS4 = RetVal_203.CSS4;
			Comm_02_Tlm_Set_Comp.CSS5 = RetVal_203.CSS5;
			Comm_02_Tlm_Set_Comp.CSS6 = RetVal_203.CSS6;
			Comm_02_Tlm_Set_Comp.CSS7 = RetVal_203.CSS7;
			Comm_02_Tlm_Set_Comp.CSS8 = RetVal_203.CSS8;
			Comm_02_Tlm_Set_Comp.CSS9 = RetVal_203.CSS9;
			Comm_02_Tlm_Set_Comp.CSSValidFlag = RetVal_203.CSSValidFlag;

			Comm_02_Tlm_Set_Comp.Cont_TimeSeconds = RetVal_172.TimeSeconds;
			Comm_02_Tlm_Set_Comp.Cont_TimeNanoSeconds = RetVal_172.TimeNanoSeconds;
			Comm_02_Tlm_Set_Comp.ControlTimeout = RetVal_172.ControlTimeout;
			Comm_02_Tlm_Set_Comp.ActiveContMode = RetVal_172.ActiveContMode;

			if ((written = fwrite(&Comm_02_Tlm_Set_Comp, sizeof(Comm_02_Tlm_Set_Comp), 1, fp)) != 1)
			{
				status = -6;		// Something is wrong! ::::: File write error
				ADCS_HandleReport(status, ADCS_COMM_02_CC, NULL, 0);
				fclose(fp);
				return status;
			}
		}
		else if (flag_tlmtype == 1) // Full telemetry set
		{

			Comm_02_Tlm_Set_Full.MainEst 	= RetVal_210;
			Comm_02_Tlm_Set_Full.GYR0 		= RetVal_204;
			Comm_02_Tlm_Set_Full.MAG0 		= RetVal_180;
			Comm_02_Tlm_Set_Full.CSS 		= RetVal_203;
			Comm_02_Tlm_Set_Full.Cont 		= RetVal_172;

			if ((written = fwrite(&Comm_02_Tlm_Set_Full, sizeof(Comm_02_Tlm_Set_Full), 1, fp)) != 1)
			{
				status = -6;		// Something is wrong! ::::: File write error
				ADCS_HandleReport(status, ADCS_COMM_02_CC, NULL, 0);
				fclose(fp);
				return status;
			}
		}


		// 8) Check time from beginning of Step 6 to end of Step 7 (dt) -> Wait the remain time (10 - dt)
		tnow = CFE_TIME_GetTime();
		dt600 = tnow.Seconds - t0_600.Seconds;
		dt = tnow.Seconds - t0_10.Seconds;

		while (dt < 10)
		{	// Wait until dt > 10 sec
			OS_TaskDelay(100);
			tnow = CFE_TIME_GetTime();
			dt = tnow.Seconds - t0_10.Seconds;
		}
		
	// 9) Go back to Step 6 until total time meets 600 sec
	}
	if (fclose(fp) != 0)
	{
		ADCS_RecordFirstFailure(&OverallStatus, CFE_STATUS_EXTERNAL_RESOURCE_FAIL);
	}

	return ADCS_ReportCommandCompletion(ADCS_COMM_02_CC, OverallStatus);
}

CFE_Status_t ADCS_Comm03Cmd(const ADCS_Comm03Cmd_t *msg) {
	ADCS_ReportCommandPhase(ADCS_COMM_03_CC, ADCS_RPT_PHASE_STARTED);

	// COMM 03:	Detumbling
	// [Procedure]
	// 1) Power On ( GYR0, MAG0 )
	// 2) Set Est. mode & Cont. mode ( Main: EstGyro (1) / Control: ConBdot3 / Timeout: 6000 s )
	// 3) Check Status of Sensors
	// 4) Make "adcs_comm_03.bin" file to log the TLMs
	// 5) Start to count time ( Waiting time before Logging: 1 sec / Tlm period: 10 sec / Comm. Duration: 6000 sec )
	// 		* Backup Estimation mode is not considered, so the convergence time is not required.
	// 6) Read Tlm of Estimator & Sensors
	// 		6-1) Read Tlm of Main Estimator:
	// 				Time integer seconds
	// 				Time nanoseconds
	// 				Estimated body rate (ORC) X-Y-Z
	// 				Estimated body rate (IRC) X-Y-Z
	// 				Active estimator mode
	// 		6-2) Read Sensor Calibrated Values - Magnetometer
	// 				Time integer seconds
	// 				Time nanoseconds
	// 				MAG0 calibrated X-Y-Z component
	// 				MAG0 valid flag
	//		* Only when |w| < 3 deg/s, MAG0 data is valid!!
	// 7) Append the Tlm to the file
	// 8) Check time from beginning of Step 6 to end of Step 7 (dt) -> Wait the remain time (10 - dt)
	// 9) Go back to Step 6 until total time meets 6000 sec
	// ** Status between every step must be reported!!


	CFE_Status_t						status;
	CFE_Status_t						interstatus;
	CFE_Status_t                        OverallStatus = CFE_SUCCESS;

	uint8 cnt_try = 0;
	uint8 flag_tlmtype = msg->Payload.flag_tlmtype;
	if ((flag_tlmtype != 0) && (flag_tlmtype != 1))
	{
		status = -1;		// Something is wrong!
		ADCS_HandleReport(status, ADCS_COMM_03_CC, &flag_tlmtype, sizeof(flag_tlmtype));
		return status;
	}
	
	// 1) Power On ( GYR0, MAG0 )
	ADCS_Comm_PowerState_Cmn_Payload_t		SetVal_056 = {0,};
	ADCS_Comm_PowerState_Cmn_Payload_t		RetVal_183 = {0,};
	SetVal_056.MAG0		= 1;
	SetVal_056.GYR0		= 1;
	interstatus = ADCS_Comm_SetPowerState(&SetVal_056);		// Set GYR0 & MAG0 --> ON
	OS_TaskDelay(10);
	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);		// Get GYR0 & MAG0 power state
	OS_TaskDelay(5000);
	cnt_try = 0;
	while (cnt_try < 3)
	{		
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1)) break;	// Check Power On well

		// if not, try again until 3 times
		interstatus = ADCS_Comm_SetPowerState(&SetVal_056);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
		cnt_try++;
	}

	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -1;		// Something is wrong! ::::: Power State
		ADCS_HandleReport(status, ADCS_COMM_03_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}


	// 3) Check Status of Sensors
	ADCS_Comm_RawMAGSensorTlm_Paylaod_t	RetVal_180 = {0,};
	ADCS_Comm_RawGYRSensorTlm_Payload_t	RetVal_204 = {0,};
	OS_TaskDelay(1000);
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS_Comm_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro

	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1)) break;	// Check Sensors Status Good

		// if not, wait changing status until 3 times
		OS_TaskDelay(100);
		interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS_Comm_GetRawGYRSensor(&RetVal_204);
		cnt_try++;
	}

	
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -3;		// Something is wrong! ::::: Magnetometer Status
		ADCS_HandleReport(status, ADCS_COMM_03_CC, &RetVal_180, sizeof(RetVal_180));
		return status;
	}
	OS_TaskDelay(10);
	interstatus = ADCS_Comm_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	if ((interstatus != CFE_SUCCESS) || (RetVal_204.GYR0ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -4;		// Something is wrong! ::::: Gyro Status
		ADCS_HandleReport(status, ADCS_COMM_03_CC, &RetVal_204, sizeof(RetVal_204));
		return status;
	}

	// 2) Set Est. mode & Cont. mode ( Main: EstGyro (1) / Control: ConBdot3 / Timeout: 6000 s )
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t	SetVal_042 = {0,};
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t	RetVal_150 = {0,};

	SetVal_042.MainEstimatorMode 	= 1;
	SetVal_042.BackupEstimatorMode 	= 2;
	SetVal_042.ControlMode 			= 3;
	SetVal_042.ControlTimeout		= 6000;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);	// Set Main Est mode = 1 / Control mode & Time out = 3 & 6000 s
	OS_TaskDelay(100);
	interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);	// Get Main Est mode
	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_150.MainEstimatorMode == 1) && (RetVal_150.ControlMode == 3)) break;	// Check Mode Change well

		// if not, try again until 3 times
		interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);
		cnt_try++;
	}

	interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != 1) || (RetVal_150.ControlMode != 3))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -2;		// Something is wrong! ::::: Estimation Mode
		ADCS_HandleReport(status, ADCS_COMM_03_CC, &RetVal_150, sizeof(RetVal_150));
		return status;

	}


	// 4) Make "adcs_comm_03.txt" file to log the TLMs
	FILE *fp;
	size_t written;
	fp = fopen("./cf/adcs_comm_03.bin","wb");
	if (fp == NULL)
	{
		status = -5;		// Something is wrong! ::::: File open error
		ADCS_HandleReport(status, ADCS_COMM_03_CC, NULL, 0);
		return status;
	}

	// 5) Start to count time ( Waiting time before Logging: 1 sec / Tlm period: 10 sec / Comm. Duration: 6000 sec )
	CFE_TIME_SysTime_t		t0_10, t0_600, tnow;
	t0_10 = CFE_TIME_GetTime();

	uint32 dt = 0;
	while (dt < 1)
	{	// Wait until dt > 1 sec
		OS_TaskDelay(100);
		tnow = CFE_TIME_GetTime();
		dt = tnow.Seconds - t0_10.Seconds;
	}

	// 6) Read Tlm of Estimator & Sensors	
	ADCS_Comm_Estimator_Cmn_Payload_t		RetVal_210 = {0,};
	ADCS_Comm_CalibratedMAGSensorTlm_Payload_t		RetVal_177 = {0,};

	ADCS_Comm_COMM_03_COMP_Payload_t	Comm_03_Tlm_Set_Comp = {0,};
	ADCS_Comm_COMM_03_FULL_Payload_t	Comm_03_Tlm_Set_Full = {0,};

	Comm_03_Tlm_Set_Comp.sync_word 	= 0xADC5;
	Comm_03_Tlm_Set_Full.sync_word 	= 0xADC5;

	t0_600 = CFE_TIME_GetTime();
	uint32 dt600 = 0;
	while (dt600 < 6000)
	{
		t0_10 = CFE_TIME_GetTime();	// Initialize 10 seconds Counter
		dt = 0;	// Initialize time gap		

		memset(&RetVal_210, 0, sizeof(RetVal_210));
		memset(&RetVal_177, 0, sizeof(RetVal_177));
		

		// 		6-1) Read Tlm of Main
		interstatus = ADCS_Comm_GetMainEstTlm(&RetVal_210);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(10);

		// 		6-2) Read Sensor Calibrated Values - Magnetometer
		interstatus = ADCS_Comm_GetCalibratedMAGSensor(&RetVal_177);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(10);

		
		// 7) Write the Tlm to the file
		if (flag_tlmtype == 0)	// Compact telemetry set
		{
			Comm_03_Tlm_Set_Comp.MainEst_TimeSecond		= RetVal_210.TimeSecond;
			Comm_03_Tlm_Set_Comp.MainEst_TimeNanoSecond	= RetVal_210.TimeNanoSecond;
			Comm_03_Tlm_Set_Comp.MainEst_EstIRCBodyRateX	= RetVal_210.EstIRCBodyRateX;
			Comm_03_Tlm_Set_Comp.MainEst_EstIRCBodyRateY	= RetVal_210.EstIRCBodyRateY;
			Comm_03_Tlm_Set_Comp.MainEst_EstIRCBodyRateZ	= RetVal_210.EstIRCBodyRateZ;
			Comm_03_Tlm_Set_Comp.MainEst_ActiveEstMode	= RetVal_210.ActiveEstMode;
						
			Comm_03_Tlm_Set_Comp.MAG0_TimeSeconds = RetVal_177.TimeSeconds;
			Comm_03_Tlm_Set_Comp.MAG0_TimeNanoSeconds = RetVal_177.TimeNanoSeconds;
			Comm_03_Tlm_Set_Comp.MAG0CalVecX = RetVal_177.MAG0CalVecX;
			Comm_03_Tlm_Set_Comp.MAG0CalVecY = RetVal_177.MAG0CalVecY;
			Comm_03_Tlm_Set_Comp.MAG0CalVecZ = RetVal_177.MAG0CalVecZ;
			Comm_03_Tlm_Set_Comp.MAG0ValidFlag = RetVal_177.MAG0ValidFlag;

			if ((written = fwrite(&Comm_03_Tlm_Set_Comp, sizeof(Comm_03_Tlm_Set_Comp), 1, fp)) != 1)
			{
				status = -6;		// Something is wrong! ::::: File write error
				ADCS_HandleReport(status, ADCS_COMM_03_CC, NULL, 0);
				fclose(fp);
				return status;
			}

		}
		else if (flag_tlmtype == 1) // Full telemetry set
		{
			Comm_03_Tlm_Set_Full.MainEst 	= RetVal_210;
			Comm_03_Tlm_Set_Full.MAG0 		= RetVal_177;

			if ((written = fwrite(&Comm_03_Tlm_Set_Full, sizeof(Comm_03_Tlm_Set_Full), 1, fp)) != 1)
			{
				status = -6;		// Something is wrong! ::::: File write error
				ADCS_HandleReport(status, ADCS_COMM_03_CC, NULL, 0);
				fclose(fp);
				return status;
			}
		}		

		// 8) Check time from beginning of Step 6 to end of Step 7 (dt) -> Wait the remain time (10 - dt)
		tnow = CFE_TIME_GetTime();
		dt600 = tnow.Seconds - t0_600.Seconds;
		dt = tnow.Seconds - t0_10.Seconds;

		while (dt < 10)
		{	// Wait until dt > 10 sec
			OS_TaskDelay(100);
			tnow = CFE_TIME_GetTime();
			dt = tnow.Seconds - t0_10.Seconds;
		}
		
	// 9) Go back to Step 6 until total time meets 6000 sec
	}
	if (fclose(fp) != 0)
	{
		ADCS_RecordFirstFailure(&OverallStatus, CFE_STATUS_EXTERNAL_RESOURCE_FAIL);
	}

	return ADCS_ReportCommandCompletion(ADCS_COMM_03_CC, OverallStatus);
}

CFE_Status_t ADCS_Comm04Cmd(const ADCS_Comm04Cmd_t *msg) {
	ADCS_ReportCommandPhase(ADCS_COMM_04_CC, ADCS_RPT_PHASE_STARTED);

	// COMM 04:	EKF Commissioning - CSS
	// [Procedure]
	// 1) Power On ( GYR0, MAG0 )
	// 2) Check Status of Sensors
	// 3) Set Est. mode & Cont. mode ( Main: EstGyroEkf  (6) / Main: EstFullEkf  (5) / Control: ConBdot3 / Timeout: 6000 s )
	//	  Set Flag of UseEkf: All elements false <-- GS Command (already set-up)
	// 4) Make "adcs_comm_04.bin" file to log the TLMs
	// 5) Start to count time ( Waiting time before Logging: 10 sec / Tlm period: 10 sec / Comm. Duration: 6000 sec )
	// 6) Read Tlm of Estimator & Sensors
	// 		6-1) Read Tlm of Main Estimator
	// 		6-2) Read Sensor Raw Values - CSS
	// 		6-3) Read Sensor Calibrated Values - CSS
	// 		6-4) Read Models Telemetery Values
	// 7) Append the Tlm to the file
	// 8) Check time from beginning of Step 6 to end of Step 7 (dt) -> Wait the remain time (10 - dt)
	// 9) Go back to Step 6 until total time meets 300 sec
	// ** Status between every step must be reported!!


	CFE_Status_t						status;
	CFE_Status_t						interstatus;
	CFE_Status_t                        OverallStatus = CFE_SUCCESS;

	uint8 cnt_try = 0;
	uint8 flag_tlmtype = msg->Payload.flag_tlmtype;
	if ((flag_tlmtype != 0) && (flag_tlmtype != 1))
	{
		status = -1;		// Something is wrong!
		ADCS_HandleReport(status, ADCS_COMM_04_CC, &flag_tlmtype, sizeof(flag_tlmtype));
		return status;
	}
	
	// 1) Power On ( GYR0, MAG0 )
	ADCS_Comm_PowerState_Cmn_Payload_t		SetVal_056 = {0,};
	ADCS_Comm_PowerState_Cmn_Payload_t		RetVal_183 = {0,};
	SetVal_056.MAG0		= 1;
	SetVal_056.GYR0		= 1;
	interstatus = ADCS_Comm_SetPowerState(&SetVal_056);		// Set GYR0 & MAG0 --> ON
	OS_TaskDelay(10);
	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);		// Get GYR0 & MAG0 power state
	OS_TaskDelay(5000);
	cnt_try = 0;
	while (cnt_try < 3)
	{		
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1)) break;	// Check Power On well

		// if not, try again until 3 times
		interstatus = ADCS_Comm_SetPowerState(&SetVal_056);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
		cnt_try++;
	}

	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -1;		// Something is wrong! ::::: Power State
		ADCS_HandleReport(status, ADCS_COMM_04_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}


	// 2) Check Status of Sensors
	ADCS_Comm_RawMAGSensorTlm_Paylaod_t	RetVal_180 = {0,};

	OS_TaskDelay(2000);
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);

	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1)) break;	// Check Sensors Status Good

		// if not, wait changing status until 3 times
		OS_TaskDelay(100);
		interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(10);
		cnt_try++;
	}

	
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -3;		// Something is wrong! ::::: Magnetometer Status
		ADCS_HandleReport(status, ADCS_COMM_04_CC, &RetVal_180, sizeof(RetVal_180));
		return status;
	}
	OS_TaskDelay(10);

	// 3) Set Est. mode & Cont. mode ( Main: EstGyroEkf  (6) / Main: EstFullEkf  (5) / Control: ConBdot3 / Timeout: 6000 s )
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t	SetVal_042 = {0,};
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t	RetVal_150 = {0,};

	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 5;
	SetVal_042.ControlMode 			= 3;
	SetVal_042.ControlTimeout		= 6000;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);	// Set Main Est mode = 6 / Set Backup Est mode = 5 / Control mode & Time out = 3 & 6000 s
	OS_TaskDelay(100);
	interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);	// Get Main Est mode
	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_150.MainEstimatorMode == 6) && (RetVal_150.ControlMode == 3)) break;	// Check Mode Change well

		// if not, try again until 3 times
		interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);
		cnt_try++;
	}

	interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != 6) || (RetVal_150.ControlMode != 3))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -2;		// Something is wrong! ::::: Estimation Mode
		ADCS_HandleReport(status, ADCS_COMM_04_CC, &RetVal_150, sizeof(RetVal_150));
		return status;

	}

	// 4) Make "adcs_comm_04.bin" file to log the TLMs
	FILE *fp;
	size_t written;
	fp = fopen("./cf/adcs_comm_04.bin","wb");
	if (fp == NULL)
	{
		status = -5;		// Something is wrong! ::::: File open error
		ADCS_HandleReport(status, ADCS_COMM_04_CC, NULL, 0);
		return status;
	}

	// 5) Start to count time ( Waiting time before Logging: 10 sec / Tlm period: 10 sec / Comm. Duration: 300 sec )
	CFE_TIME_SysTime_t		t0_10, t0_300, tnow;
	t0_10 = CFE_TIME_GetTime();

	uint32 dt = 0;
	while (dt < 10)
	{	// Wait until dt > 10 sec
		OS_TaskDelay(100);
		tnow = CFE_TIME_GetTime();
		dt = tnow.Seconds - t0_10.Seconds;
	}

	// 6) Read Tlm of Estimator & Sensors
	ADCS_Comm_Estimator_Cmn_Payload_t	RetVal_210 = {0,};
	ADCS_Comm_Estimator_Cmn_Payload_t	RetVal_173 = {0,};

	ADCS_Comm_RawCSSSensorTlm_Payload_t 	RetVal_203 = {0,};
	ADCS_Comm_CalibratedCSSSensorTlm_Payload_t 	RetVal_206 = {0,};
	ADCS_Comm_ModelsTlm_Payload_t 			RetVal_174 = {0,};

	ADCS_Comm_COMM_04_COMP_Payload_t	Comm_04_Tlm_Set_Comp = {0,};
	ADCS_Comm_COMM_04_FULL_Payload_t	Comm_04_Tlm_Set_Full = {0,};

	Comm_04_Tlm_Set_Comp.sync_word 	= 0xADC5;
	Comm_04_Tlm_Set_Full.sync_word 	= 0xADC5;

	t0_300 = CFE_TIME_GetTime();
	uint32 dt300 = 0;
	while (dt300 < 6000)
	{
		t0_10 = CFE_TIME_GetTime();	// Initialize 10 seconds Counter
		dt = 0;	// Initialize time gap
		

		memset(&RetVal_210, 0, sizeof(RetVal_210));
		memset(&RetVal_173, 0, sizeof(RetVal_173));
		memset(&RetVal_203, 0, sizeof(RetVal_203));
		memset(&RetVal_206, 0, sizeof(RetVal_206));
		memset(&RetVal_174, 0, sizeof(RetVal_174));
		
		// Comm_04_Tlm_Set.sync_word 	= 0xADC5;

		// 		6-1) Read Tlm of Main / Backup Estimator:	
		interstatus = ADCS_Comm_GetMainEstTlm(&RetVal_210);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetBackupEstTlm(&RetVal_173);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(10);

		// 		6-2) Read Sensor Raw Values - CSS	
		interstatus = ADCS_Comm_GetRawCSSSensor(&RetVal_203);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(10);

		// 		6-3) Read Sensor Calibrated Values - CSS
		interstatus = ADCS_Comm_GetCalibratedCSSSensor(&RetVal_206);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(10);

		// 		6-4) Read Models Telemetery Values
		interstatus = ADCS_Comm_GetModelsTlm(&RetVal_174);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(10);
		
		// 7) Write the Tlm to the file
		if (flag_tlmtype == 0)	// Compact telemetry set
		{			
			
			Comm_04_Tlm_Set_Comp.MainEst		= RetVal_210;
			Comm_04_Tlm_Set_Comp.RawCSS			= RetVal_203;
			Comm_04_Tlm_Set_Comp.CalCSS			= RetVal_206;
			Comm_04_Tlm_Set_Comp.Models 		= RetVal_174;

			if ((written = fwrite(&Comm_04_Tlm_Set_Comp, sizeof(Comm_04_Tlm_Set_Comp), 1, fp)) != 1)
			{
				status = -6;		// Something is wrong! ::::: File write error
				ADCS_HandleReport(status, ADCS_COMM_04_CC, NULL, 0);
				fclose(fp);
				return status;
			}
		}
		else if (flag_tlmtype == 1) // Full telemetry set
		{			
			
			Comm_04_Tlm_Set_Full.MainEst		= RetVal_210;
			Comm_04_Tlm_Set_Full.BackupEst		= RetVal_173;
			Comm_04_Tlm_Set_Full.RawCSS			= RetVal_203;
			Comm_04_Tlm_Set_Full.CalCSS			= RetVal_206;
			Comm_04_Tlm_Set_Comp.Models 		= RetVal_174;

				if ((written = fwrite(&Comm_04_Tlm_Set_Full, sizeof(Comm_04_Tlm_Set_Full), 1, fp)) != 1)
			{
				status = -6;		// Something is wrong! ::::: File write error
				ADCS_HandleReport(status, ADCS_COMM_04_CC, NULL, 0);
				fclose(fp);
				return status;
			}
		}


		// 8) Check time from beginning of Step 6 to end of Step 7 (dt) -> Wait the remain time (10 - dt)
		tnow = CFE_TIME_GetTime();
		dt300 = tnow.Seconds - t0_300.Seconds;
		dt = tnow.Seconds - t0_10.Seconds;

		while (dt < 10)
		{	// Wait until dt > 10 sec
			OS_TaskDelay(100);
			tnow = CFE_TIME_GetTime();
			dt = tnow.Seconds - t0_10.Seconds;
		}
		
	// 9) Go back to Step 6 until total time meets 6000 sec
	}
	if (fclose(fp) != 0)
	{
		ADCS_RecordFirstFailure(&OverallStatus, CFE_STATUS_EXTERNAL_RESOURCE_FAIL);
	}

	return ADCS_ReportCommandCompletion(ADCS_COMM_04_CC, OverallStatus);
}

CFE_Status_t ADCS_Comm05Cmd(const ADCS_Comm05Cmd_t *msg) {
	ADCS_ReportCommandPhase(ADCS_COMM_05_CC, ADCS_RPT_PHASE_STARTED);

	// COMM 05:	FSS Commissioning
	// [Procedure]
	// 1) Power On ( GYR0, MAG0, FSS0 )
	// 2) Check Status of Sensors
	// 3) Set Est. mode & Cont. mode ( Main: EstGyroEkf  (6) / Main: EstFullEkf  (5) / Control: ConBdot3 / Timeout: 6000 s )
	//	  Set Flag of UseEkf: FSS, HSS, STT elements false <-- GS Command (already set-up)
	// 4) Make "adcs_comm_05.bin" file to log the TLMs
	// 5) Start to count time ( Waiting time before Logging: 10 sec / Tlm period: 10 sec / Comm. Duration: 6000 sec )
	// 6) Read Tlm of Estimator & Sensors
	// 		6-1) Read Tlm of Main Estimator
	// 		6-2) Read Sensor Raw Values - CSS
	// 		6-3) Read Sensor Calibrated Values - CSS
	// 		6-4) Read Sensor Raw Values - FSS
	// 		6-5) Read Sensor Calibrated Values - FSS
	// 7) Append the Tlm to the file
	// 8) Check time from beginning of Step 6 to end of Step 7 (dt) -> Wait the remain time (10 - dt)
	// 9) Go back to Step 6 until total time meets 6000 sec
	// ** Status between every step must be reported!!


	CFE_Status_t						status;
	CFE_Status_t						interstatus;
	CFE_Status_t                        OverallStatus = CFE_SUCCESS;

	uint8 cnt_try = 0;
	// uint8 flag_tlmtype = msg->Payload.flag_tlmtype;
	// if ((flag_tlmtype != 0) && (flag_tlmtype != 1))
	// {
	// 	status = -1;		// Something is wrong!
	// 	ADCS_HandleReport(status, ADCS_COMM_05_CC, &flag_tlmtype, sizeof(flag_tlmtype));
	// 	return status;
	// }
	
	// 1) Power On ( GYR0, MAG0, FSS0 )
	ADCS_Comm_PowerState_Cmn_Payload_t		SetVal_056 = {0,};
	ADCS_Comm_PowerState_Cmn_Payload_t		RetVal_183 = {0,};
	SetVal_056.MAG0		= 1;
	SetVal_056.GYR0		= 1;
	SetVal_056.FSS0		= 1;
	interstatus = ADCS_Comm_SetPowerState(&SetVal_056);		// Set GYR0 & MAG0 & FSS0 --> ON
	OS_TaskDelay(10);
	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);		// Get GYR0 & MAG0 & FSS0 power state
	OS_TaskDelay(5000);
	cnt_try = 0;
	while (cnt_try < 3)
	{		
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1) && (RetVal_183.FSS0 == 1)) break;	// Check Power On well

		// if not, try again until 3 times
		interstatus = ADCS_Comm_SetPowerState(&SetVal_056);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
		cnt_try++;
	}

	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1) || (RetVal_183.FSS0 != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -1;		// Something is wrong! ::::: Power State
		ADCS_HandleReport(status, ADCS_COMM_05_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}


	// 2) Check Status of Sensors
	ADCS_Comm_RawMAGSensorTlm_Paylaod_t	RetVal_180 = {0,};
	ADCS_Comm_RawGYRSensorTlm_Payload_t	RetVal_204 = {0,};
	// ADCS_Comm_RawCubeSenseSunTlm_Payload_t RetVal_170 = {0,};		// "FSS0 valid" is not identified yet. After idetification, use this status to check the state of FSS0. -> (line 1578, 1584)

	OS_TaskDelay(2000);
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS_Comm_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	// OS_TaskDelay(10);
	// interstatus = interstatus + ADCS_Comm_GetRawCubeSenseSun(&RetVal_170);	// Get Status of FSS

	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1)) break;	// Check Sensors Status Good
		// if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1) && (RetVal_170.ValidResult0 == 1)) break;	// Check Sensors Status Good

		// if not, wait changing status until 3 times
		OS_TaskDelay(100);
		interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS_Comm_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
		cnt_try++;
	}

	
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS_Comm_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1) || (RetVal_204.GYR0ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -3;		// Something is wrong! ::::: Magnetometer Status
		ADCS_HandleReport(status, ADCS_COMM_05_CC, &RetVal_180, sizeof(RetVal_180));
		return status;
	}
	OS_TaskDelay(10);

	// 3) Set Est. mode & Cont. mode ( Main: EstGyroEkf  (6) / Main: EstFullEkf  (5) / Control: ConBdot3 / Timeout: 6000 s )
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t	SetVal_042 = {0,};
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t	RetVal_150 = {0,};

	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 5;
	SetVal_042.ControlMode 			= 3;
	SetVal_042.ControlTimeout		= 6000;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);	// Set Main Est mode = 6 / Set Backup Est mode = 5 / Control mode & Time out = 3 & 6000 s
	OS_TaskDelay(100);
	interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);	// Get Main Est mode
	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_150.MainEstimatorMode == 6) && (RetVal_150.ControlMode == 3)) break;	// Check Mode Change well

		// if not, try again until 3 times
		interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);
		cnt_try++;
	}

	interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != 6) || (RetVal_150.ControlMode != 3))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -2;		// Something is wrong! ::::: Estimation Mode
		ADCS_HandleReport(status, ADCS_COMM_05_CC, &RetVal_150, sizeof(RetVal_150));
		return status;

	}

	// 4) Make "adcs_comm_05.bin" file to log the TLMs
	FILE *fp;
	size_t written;
	fp = fopen("./cf/adcs_comm_05.bin","wb");
	if (fp == NULL)
	{
		status = -5;		// Something is wrong! ::::: File open error
		ADCS_HandleReport(status, ADCS_COMM_05_CC, NULL, 0);
		return status;
	}

	// 5) Start to count time ( Waiting time before Logging: 10 sec / Tlm period: 10 sec / Comm. Duration: 300 sec )
	CFE_TIME_SysTime_t		t0_10, t0_300, tnow;
	t0_10 = CFE_TIME_GetTime();

	uint32 dt = 0;
	while (dt < 10)
	{	// Wait until dt > 10 sec
		OS_TaskDelay(100);
		tnow = CFE_TIME_GetTime();
		dt = tnow.Seconds - t0_10.Seconds;
	}

	// 6) Read Tlm of Estimator & Sensors
	ADCS_Comm_Estimator_Cmn_Payload_t	RetVal_210 = {0,};

	ADCS_Comm_RawCSSSensorTlm_Payload_t 	RetVal_203 = {0,};
	ADCS_Comm_CalibratedCSSSensorTlm_Payload_t 	RetVal_206 = {0,};
	ADCS_Comm_RawCubeSenseSunTlm_Payload_t 		RetVal_170 = {0,};
	ADCS_Comm_CalibratedFSSSensorTlm_Payload_t	RetVal_178 = {0,};

	ADCS_Comm_COMM_05_Payload_t	Comm_05_Tlm_Set = {0,};

	Comm_05_Tlm_Set.sync_word 	= 0xADC5;

	t0_300 = CFE_TIME_GetTime();
	uint32 dt300 = 0;
	while (dt300 < 6000)
	{
		t0_10 = CFE_TIME_GetTime();	// Initialize 10 seconds Counter
		dt = 0;	// Initialize time gap
		

		memset(&RetVal_210, 0, sizeof(RetVal_210));
		memset(&RetVal_203, 0, sizeof(RetVal_203));
		memset(&RetVal_206, 0, sizeof(RetVal_206));
		memset(&RetVal_170, 0, sizeof(RetVal_170));
		memset(&RetVal_178, 0, sizeof(RetVal_178));
		
		// Comm_05_Tlm_Set.sync_word 	= 0xADC5;

		// 		6-1) Read Tlm of Main:	
		interstatus = ADCS_Comm_GetMainEstTlm(&RetVal_210);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(20);

		// 		6-2) Read Sensor Raw Values - CSS	
		interstatus = ADCS_Comm_GetRawCSSSensor(&RetVal_203);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(20);

		// 		6-3) Read Sensor Calibrated Values - CSS
		interstatus = ADCS_Comm_GetCalibratedCSSSensor(&RetVal_206);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(20);

		// 		6-4) Read Sensor Raw Values - FSS
		interstatus = ADCS_Comm_GetRawCubeSenseSun(&RetVal_170);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(20);

		// 		6-5) Read Sensor Calibrated Values - FSS
		interstatus = ADCS_Comm_GetCalibratedFSSSensor(&RetVal_178);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(20);
		
		// 7) Write the Tlm to the file
		Comm_05_Tlm_Set.MainEst		= RetVal_210;
		Comm_05_Tlm_Set.RawCSS		= RetVal_203;
		Comm_05_Tlm_Set.CalCSS		= RetVal_206;
		Comm_05_Tlm_Set.RawFSS 		= RetVal_170;
		Comm_05_Tlm_Set.CalFSS 		= RetVal_178;

		if ((written = fwrite(&Comm_05_Tlm_Set, sizeof(Comm_05_Tlm_Set), 1, fp)) != 1)
		{
			status = -6;		// Something is wrong! ::::: File write error
			ADCS_HandleReport(status, ADCS_COMM_05_CC, NULL, 0);
			fclose(fp);
			return status;
		}


		// 8) Check time from beginning of Step 6 to end of Step 7 (dt) -> Wait the remain time (10 - dt)
		tnow = CFE_TIME_GetTime();
		dt300 = tnow.Seconds - t0_300.Seconds;
		dt = tnow.Seconds - t0_10.Seconds;

		while (dt < 10)
		{	// Wait until dt > 10 sec
			OS_TaskDelay(100);
			tnow = CFE_TIME_GetTime();
			dt = tnow.Seconds - t0_10.Seconds;
		}
		
	// 9) Go back to Step 6 until total time meets 6000 sec
	}
	if (fclose(fp) != 0)
	{
		ADCS_RecordFirstFailure(&OverallStatus, CFE_STATUS_EXTERNAL_RESOURCE_FAIL);
	}

	return ADCS_ReportCommandCompletion(ADCS_COMM_05_CC, OverallStatus);
}

CFE_Status_t ADCS_Comm06Cmd(const ADCS_Comm06Cmd_t *msg) {
	ADCS_ReportCommandPhase(ADCS_COMM_06_CC, ADCS_RPT_PHASE_STARTED);

	// COMM 06:	Wheel Commissioning
	// [Procedure]
	// 1) Power On ( GYR0, MAG0, FSS0, RWLX )
	// 2) Check Status of Sensors
	// 3) Set Est. mode & Cont. mode ( Main: EstGyroEkf  (6) / Main: EstGyro  (1) / Control: ConHxyzRW / Timeout: 120+60 s )
	//	  Set Flag of UseEkf: HSS, STT elements false <-- GS Command (already set-up)
	// 4) Make "adcs_comm_06_X.bin" file to log the TLMs
	// 5) Start to count time ( Waiting time before Logging: 10 sec / Tlm period: 1 sec / Comm. Duration: 120+60+alpha sec )
	// 6) Read Tlm of Estimator & Sensors
	// 		6-1) Read Tlm of Main Estimator
	// 		6-2) Read Sensor Raw Values - RWL
	// 		6-3) Read Sensor Calibrated Values - RWL
	// 7) Append the Tlm to the file
	// 8) Check time from beginning of Step 6 to end of Step 7 (dt) -> Wait the remain time (10 - dt)
	// 9) Go back to Step 6 until total time meets 120 sec
	// ** Status between every step must be reported!!


	CFE_Status_t						status;
	CFE_Status_t						interstatus;
	CFE_Status_t                        OverallStatus = CFE_SUCCESS;

	uint8 cnt_try = 0;	
	uint8 flag_estmode = msg->Payload.flag_estmode;
	uint8 flag_contmode = msg->Payload.flag_contmode;
	// flag_contmode = 
	// 					0:  Initial pyramid reaction wheel commissioning
	// 					1:	Three-axis orthogonal wheel commissioning - X-axis
	// 					2:	Three-axis orthogonal wheel commissioning - Y-axis
	// 					3:	Three-axis orthogonal wheel commissioning - Z-axis
	if ((flag_contmode != 0) && (flag_contmode != 1) && (flag_contmode != 2) && (flag_contmode != 3))
	{
		status = -7;		// Something is wrong!
		ADCS_HandleReport(status, ADCS_COMM_06_CC, &flag_contmode, sizeof(flag_contmode));
		return status;
	}
	if ((flag_estmode != 0) && (flag_estmode != 1) && (flag_estmode != 2))
	{
		status = -8;		// Something is wrong!
		ADCS_HandleReport(status, ADCS_COMM_06_CC, &flag_estmode, sizeof(flag_estmode));
		return status;
	}
	
	// 1) Power On ( GYR0, MAG0, FSS0, RWLX )
	ADCS_Comm_PowerState_Cmn_Payload_t		SetVal_056 = {0,};
	ADCS_Comm_PowerState_Cmn_Payload_t		RetVal_183 = {0,};
	SetVal_056.MAG0		= 1;
	SetVal_056.GYR0		= 1;
	SetVal_056.FSS0		= 1;
	SetVal_056.RWL0		= 1;
	SetVal_056.RWL1		= 1;
	SetVal_056.RWL2		= 1;
	SetVal_056.RWL3		= 1;

	interstatus = ADCS_Comm_SetPowerState(&SetVal_056);		// Set GYR0 & MAG0 & FSS0 & RWLX --> ON
	OS_TaskDelay(10);
	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);		// Get GYR0 & MAG0 & FSS0 & RWLX power state
	OS_TaskDelay(5000);
	cnt_try = 0;
	while (cnt_try < 3)
	{		
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1) && (RetVal_183.FSS0 == 1) && (RetVal_183.RWL0 == 1) && (RetVal_183.RWL1 == 1) && (RetVal_183.RWL2 == 1) && (RetVal_183.RWL3 == 1)) break;	// Check Power On well

		// if not, try again until 3 times
		interstatus = ADCS_Comm_SetPowerState(&SetVal_056);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
		cnt_try++;
	}

	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1) || (RetVal_183.FSS0 != 1) || (RetVal_183.RWL0 != 1) || (RetVal_183.RWL1 != 1) || (RetVal_183.RWL2 != 1) || (RetVal_183.RWL3 != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -1;		// Something is wrong! ::::: Power State
		ADCS_HandleReport(status, ADCS_COMM_06_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}


	// 2) Check Status of Sensors
	ADCS_Comm_RawMAGSensorTlm_Paylaod_t	RetVal_180 = {0,};
	ADCS_Comm_RawGYRSensorTlm_Payload_t	RetVal_204 = {0,};
	// ADCS_Comm_RawCubeSenseSunTlm_Payload_t RetVal_170 = {0,};		// "FSS0 valid" is not identified yet. After idetification, use this status to check the state of FSS0. -> (line 1578, 1584)
	ADCS_Comm_RawRWLSensorTlm_Payload_t RetVal_205 = {0,};

	OS_TaskDelay(2000);
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS_Comm_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	// OS_TaskDelay(10);
	// interstatus = interstatus + ADCS_Comm_GetRawCubeSenseSun(&RetVal_170);	// Get Status of FSS
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS_Comm_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels
	

	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1) && (RetVal_205.RWL0ValidFlag == 1) && (RetVal_205.RWL1ValidFlag == 1) && (RetVal_205.RWL2ValidFlag == 1) && (RetVal_205.RWL3ValidFlag == 1)) break;	// Check Sensors Status Good
		// if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1) && (RetVal_170.ValidResult0 == 1)) break;	// Check Sensors Status Good

		// if not, wait changing status until 3 times
		OS_TaskDelay(100);
		interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS_Comm_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS_Comm_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels
		cnt_try++;
	}

	
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS_Comm_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS_Comm_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels

	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1) || (RetVal_204.GYR0ValidFlag != 1) || (RetVal_205.RWL0ValidFlag != 1) || (RetVal_205.RWL1ValidFlag != 1) || (RetVal_205.RWL2ValidFlag != 1) || (RetVal_205.RWL3ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -3;		// Something is wrong! ::::: Magnetometer Status
		ADCS_HandleReport(status, ADCS_COMM_06_CC, &RetVal_180, sizeof(RetVal_180));
		return status;
	}
	OS_TaskDelay(10);

	// 3) Set Est. mode & Cont. mode ( Main: EstGyroEkf  (6) / Main: EstGyro  (1) / Control: ConHxyzRW / Timeout: 120+60 s )
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t	SetVal_042 = {0,};
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t	RetVal_150 = {0,};

	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 1;
	// SetVal_042.ControlMode 			= 51;
	// SetVal_042.ControlTimeout		= 120;
	SetVal_042.ControlMode 			= 3;
	SetVal_042.ControlTimeout		= 0;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);	// Set Main Est mode = 6 / Set Backup Est mode = 1 / Control mode & Time out = 3 & 0 s
	OS_TaskDelay(100);
	interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);	// Get Main Est mode
	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_150.MainEstimatorMode == 6) && (RetVal_150.ControlMode == 3)) break;	// Check Mode Change well

		// if not, try again until 3 times
		interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);
		cnt_try++;
	}

	interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != 6) || (RetVal_150.ControlMode != 3))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -2;		// Something is wrong! ::::: Estimation Mode
		ADCS_HandleReport(status, ADCS_COMM_06_CC, &RetVal_150, sizeof(RetVal_150));
		return status;

	}

	// 4) Make "adcs_comm_06_X.bin" file to log the TLMs
	FILE *fp;
	size_t written;

	if (flag_contmode == 0) {
		fp = fopen("./cf/adcs_comm_06_0.bin","wb");
	}
	else if (flag_contmode == 1) {
		fp = fopen("./cf/adcs_comm_06_1.bin","wb");
	}
	else if (flag_contmode == 2) {
		fp = fopen("./cf/adcs_comm_06_2.bin","wb");
	}
	else if (flag_contmode == 3) {
		fp = fopen("./cf/adcs_comm_06_3.bin","wb");
	}
	else {
		fp = NULL;
	}

	if (fp == NULL)
	{
		status = -5;		// Something is wrong! ::::: File open error
		ADCS_HandleReport(status, ADCS_COMM_06_CC, NULL, 0);

		SetVal_042.MainEstimatorMode 	= 6;
		SetVal_042.BackupEstimatorMode 	= 5;
		SetVal_042.ControlMode 			= 3;
		SetVal_042.ControlTimeout		= 0;
		interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);

		SetVal_056.RWL0		= 0;
		SetVal_056.RWL1		= 0;
		SetVal_056.RWL2		= 0;
		SetVal_056.RWL3		= 0;
		interstatus = ADCS_Comm_SetPowerState(&SetVal_056);		// Set RWLX --> OFF
		return status;
	}

	float cmdH = 0.0;
	if (flag_estmode == 0) {
		cmdH = 0.005;
	}
	else if (flag_estmode == 1) {
		cmdH = 0.002;
	}
	else if (flag_estmode == 2) {
		cmdH = 0.001;
	}
	else {
		cmdH = 0.0;
	}

	// Before 5) Pre-define Hxyz Command values
	ADCS_Comm_OpenLoopCmdHxyzRWCmd_Payload_t SetVal_076 = {0,};
	
	// 5) Start to count time ( Waiting time before Logging: 10 sec / Tlm period: 1 sec / Comm. Duration: 120+60+alpha sec )
	CFE_TIME_SysTime_t		t0_10, t0_300, tnow;
	t0_10 = CFE_TIME_GetTime();

	uint32 dt = 0;
	while (dt < 10)
	{	// Wait until dt > 10 sec
		OS_TaskDelay(100);
		tnow = CFE_TIME_GetTime();
		dt = tnow.Seconds - t0_10.Seconds;
	}

	// 6) Read Tlm of Estimator & Sensors
	ADCS_Comm_Estimator_Cmn_Payload_t	RetVal_210 = {0,};
	ADCS_Comm_CalibratedRWLSensorTlm_Payload_t 	RetVal_209 = {0,};

	ADCS_Comm_COMM_06_Payload_t	Comm_06_Tlm_Set = {0,};

	Comm_06_Tlm_Set.sync_word 	= 0xADC5;

	t0_300 = CFE_TIME_GetTime();
	uint32 dt300 = 0;
	bool phase_rw_51_set = false;
	bool phase_rw_50_set = false;
	bool phase_rw_03_set = false;
	bool phase_vec_0_set = false;

	while (dt300 < 300)
	{
		if ((dt300 > 250) && (phase_rw_03_set == false)) {
			phase_rw_03_set = true;
			SetVal_042.MainEstimatorMode 	= 6;
			SetVal_042.BackupEstimatorMode 	= 1;
			SetVal_042.ControlMode 			= 3;
			SetVal_042.ControlTimeout		= 0;
			interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
			ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		}
		else if ((dt300 > 150) && (phase_rw_50_set == false)) {
			phase_rw_50_set = true;
			SetVal_042.MainEstimatorMode 	= 6;
			SetVal_042.BackupEstimatorMode 	= 1;
			SetVal_042.ControlMode 			= 50;
			SetVal_042.ControlTimeout		= 80;
			interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
			ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		}
		else if (dt300 > 140 && (phase_vec_0_set == false)) {
			phase_vec_0_set = true;
			SetVal_076.cmdHx = 0.0;
			SetVal_076.cmdHy = 0.0;
			SetVal_076.cmdHz = 0.0;
			interstatus = ADCS_Comm_SetOpenLoopCmdHxyzRW(&SetVal_076);
			ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		}
		else if ((dt300 > 20) && (phase_rw_51_set == false)) {
			phase_rw_51_set = true;
			SetVal_042.MainEstimatorMode 	= 6;
			SetVal_042.BackupEstimatorMode 	= 1;
			SetVal_042.ControlMode 			= 51;
			SetVal_042.ControlTimeout		= 120;
			interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
			ADCS_RecordFirstFailure(&OverallStatus, interstatus);

			OS_TaskDelay(100);

				if (flag_contmode == 0) {
					SetVal_076.cmdHx = 0.0;
					SetVal_076.cmdHy = 0.0;
					SetVal_076.cmdHz = 0.0;
					interstatus = ADCS_Comm_SetOpenLoopCmdHxyzRW(&SetVal_076);
					ADCS_RecordFirstFailure(&OverallStatus, interstatus);
				}
				else if (flag_contmode == 1) {
					SetVal_076.cmdHx = cmdH;
					SetVal_076.cmdHy = 0.0;
					SetVal_076.cmdHz = 0.0;
					interstatus = ADCS_Comm_SetOpenLoopCmdHxyzRW(&SetVal_076);
					ADCS_RecordFirstFailure(&OverallStatus, interstatus);
				}
				else if (flag_contmode == 2) {
					SetVal_076.cmdHx = 0.0;
					SetVal_076.cmdHy = cmdH;
					SetVal_076.cmdHz = 0.0;
					interstatus = ADCS_Comm_SetOpenLoopCmdHxyzRW(&SetVal_076);
					ADCS_RecordFirstFailure(&OverallStatus, interstatus);
				}
				else if (flag_contmode == 3) {
					SetVal_076.cmdHx = 0.0;
					SetVal_076.cmdHy = 0.0;
					SetVal_076.cmdHz = cmdH;
					interstatus = ADCS_Comm_SetOpenLoopCmdHxyzRW(&SetVal_076);
					ADCS_RecordFirstFailure(&OverallStatus, interstatus);
				}
				else {
					SetVal_076.cmdHx = 0.0;
					SetVal_076.cmdHy = 0.0;
					SetVal_076.cmdHz = 0.0;
					interstatus = ADCS_Comm_SetOpenLoopCmdHxyzRW(&SetVal_076);
					ADCS_RecordFirstFailure(&OverallStatus, interstatus);
				}
		}

		t0_10 = CFE_TIME_GetTime();	// Initialize 1 seconds Counter
		dt = 0;	// Initialize time gap
		

		memset(&RetVal_210, 0, sizeof(RetVal_210));
		memset(&RetVal_205, 0, sizeof(RetVal_205));
		memset(&RetVal_209, 0, sizeof(RetVal_209));
		
		// Comm_06_Tlm_Set.sync_word 	= 0xADC5;

		// 		6-1) Read Tlm of Main:	
		interstatus = ADCS_Comm_GetMainEstTlm(&RetVal_210);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(20);

		// 		6-2) Read Sensor Raw Values - RWL	
		interstatus = ADCS_Comm_GetRawRWLSensor(&RetVal_205);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(20);

		// 		6-3) Read Sensor Calibrated Values - RWL
		interstatus = ADCS_Comm_GetCalibratedRWLSensor(&RetVal_209);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(20);
		
		// 7) Write the Tlm to the file
		Comm_06_Tlm_Set.MainEst		= RetVal_210;
		Comm_06_Tlm_Set.RawRWL		= RetVal_205;
		Comm_06_Tlm_Set.CalRWL		= RetVal_209;

		if ((written = fwrite(&Comm_06_Tlm_Set, sizeof(Comm_06_Tlm_Set), 1, fp)) != 1)
		{
			status = -6;		// Something is wrong! ::::: File write error
			ADCS_HandleReport(status, ADCS_COMM_06_CC, NULL, 0);
			fclose(fp);

			SetVal_042.MainEstimatorMode 	= 6;
			SetVal_042.BackupEstimatorMode 	= 5;
			SetVal_042.ControlMode 			= 3;
			SetVal_042.ControlTimeout		= 0;
			interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);

			SetVal_056.RWL0		= 0;
			SetVal_056.RWL1		= 0;
			SetVal_056.RWL2		= 0;
			SetVal_056.RWL3		= 0;
			interstatus = ADCS_Comm_SetPowerState(&SetVal_056);		// Set RWLX --> OFF
			return status;
		}


		// 8) Check time from beginning of Step 6 to end of Step 7 (dt) -> Wait the remain time (10 - dt)
		tnow = CFE_TIME_GetTime();
		dt300 = tnow.Seconds - t0_300.Seconds;
		dt = tnow.Seconds - t0_10.Seconds;

		while (dt < 1)
		{	// Wait until dt > 1 sec
			OS_TaskDelay(50);
			tnow = CFE_TIME_GetTime();
			dt = tnow.Seconds - t0_10.Seconds;
		}
		
	// 9) Go back to Step 6 until total time meets 180 sec
	}
	if (fclose(fp) != 0)
	{
		ADCS_RecordFirstFailure(&OverallStatus, CFE_STATUS_EXTERNAL_RESOURCE_FAIL);
	}

	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 5;
	SetVal_042.ControlMode 			= 3;
	SetVal_042.ControlTimeout		= 0;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	ADCS_RecordFirstFailure(&OverallStatus, interstatus);

	SetVal_056.RWL0		= 0;
	SetVal_056.RWL1		= 0;
	SetVal_056.RWL2		= 0;
	SetVal_056.RWL3		= 0;
	interstatus = ADCS_Comm_SetPowerState(&SetVal_056);		// Set RWLX --> OFF
	ADCS_RecordFirstFailure(&OverallStatus, interstatus);

	return ADCS_ReportCommandCompletion(ADCS_COMM_06_CC, OverallStatus);
}

CFE_Status_t ADCS_Comm07Cmd(const ADCS_Comm07Cmd_t *msg) {
	ADCS_ReportCommandPhase(ADCS_COMM_07_CC, ADCS_RPT_PHASE_STARTED);

	// COMM 07:	Sun Tracking 3-axis Control Commissioning
	// [Procedure]
	// 1) Power On ( GYR0, MAG0, FSS0, RWLX )
	// 2) Check Status of Sensors
	// 3) Set Est. mode & Cont. mode ( Main: EstGyroEkf  (6) / Main: EstGyro  (5) / Control: ConHxyzRW, ConXYZwheel, ConSunTrack / Timeout: 120+300+500 s )
	//	  Set Flag of UseEkf: HSS, STT elements false <-- GS Command (already set-up)
	// 4) Make "adcs_comm_07.bin" file to log the TLMs
	// 5) Start to count time ( Waiting time before Logging: 2800 sec / Tlm period: 1 sec / Comm. Duration: 720 sec )
	// 6) Read Tlm of Estimator & Sensors
	// 		6-1) Read Tlm of Main Estimator
	// 		6-2) Read Sensor Raw Values - RWL
	// 		6-3) Read Sensor Calibrated Values - RWL
	// 		6-3) Read Sensor Calibrated Values - FSS
	// 7) Append the Tlm to the file
	// 8) Check time from beginning of Step 6 to end of Step 7 (dt) -> Wait the remain time (10 - dt)
	// 9) Go back to Step 6 until total time meets XXXX sec
	// ** Status between every step must be reported!!


	CFE_Status_t						status;
	CFE_Status_t						interstatus;
	CFE_Status_t                        OverallStatus = CFE_SUCCESS;

	uint8 cnt_try = 0;
		
	// 1) Power On ( GYR0, MAG0, FSS0, RWLX )
	ADCS_Comm_PowerState_Cmn_Payload_t		SetVal_056 = {0,};
	ADCS_Comm_PowerState_Cmn_Payload_t		RetVal_183 = {0,};
	SetVal_056.MAG0		= 1;
	SetVal_056.GYR0		= 1;
	SetVal_056.FSS0		= 1;
	SetVal_056.RWL0		= 1;
	SetVal_056.RWL1		= 1;
	SetVal_056.RWL2		= 1;
	SetVal_056.RWL3		= 1;

	interstatus = ADCS_Comm_SetPowerState(&SetVal_056);		// Set GYR0 & MAG0 & FSS0 & RWLX --> ON
	OS_TaskDelay(10);
	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);		// Get GYR0 & MAG0 & FSS0 & RWLX power state
	OS_TaskDelay(5000);
	cnt_try = 0;
	while (cnt_try < 3)
	{		
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1) && (RetVal_183.FSS0 == 1) && (RetVal_183.RWL0 == 1) && (RetVal_183.RWL1 == 1) && (RetVal_183.RWL2 == 1) && (RetVal_183.RWL3 == 1)) break;	// Check Power On well

		// if not, try again until 3 times
		interstatus = ADCS_Comm_SetPowerState(&SetVal_056);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
		cnt_try++;
	}

	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1) || (RetVal_183.FSS0 != 1) || (RetVal_183.RWL0 != 1) || (RetVal_183.RWL1 != 1) || (RetVal_183.RWL2 != 1) || (RetVal_183.RWL3 != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -1;		// Something is wrong! ::::: Power State
		ADCS_HandleReport(status, ADCS_COMM_07_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}


	// 2) Check Status of Sensors
	ADCS_Comm_RawMAGSensorTlm_Paylaod_t	RetVal_180 = {0,};
	ADCS_Comm_RawGYRSensorTlm_Payload_t	RetVal_204 = {0,};
	// ADCS_Comm_RawCubeSenseSunTlm_Payload_t RetVal_170 = {0,};		// "FSS0 valid" is not identified yet. After idetification, use this status to check the state of FSS0. -> (line 1578, 1584)
	ADCS_Comm_RawRWLSensorTlm_Payload_t RetVal_205 = {0,};

	OS_TaskDelay(2000);
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS_Comm_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	// OS_TaskDelay(10);
	// interstatus = interstatus + ADCS_Comm_GetRawCubeSenseSun(&RetVal_170);	// Get Status of FSS
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS_Comm_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels
	

	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1) && (RetVal_205.RWL0ValidFlag == 1) && (RetVal_205.RWL1ValidFlag == 1) && (RetVal_205.RWL2ValidFlag == 1) && (RetVal_205.RWL3ValidFlag == 1)) break;	// Check Sensors Status Good
		// if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1) && (RetVal_170.ValidResult0 == 1)) break;	// Check Sensors Status Good

		// if not, wait changing status until 3 times
		OS_TaskDelay(100);
		interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS_Comm_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS_Comm_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels
		cnt_try++;
	}

	
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS_Comm_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS_Comm_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels

	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1) || (RetVal_204.GYR0ValidFlag != 1) || (RetVal_205.RWL0ValidFlag != 1) || (RetVal_205.RWL1ValidFlag != 1) || (RetVal_205.RWL2ValidFlag != 1) || (RetVal_205.RWL3ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -3;		// Something is wrong! ::::: Magnetometer Status
		ADCS_HandleReport(status, ADCS_COMM_07_CC, &RetVal_180, sizeof(RetVal_180));
		return status;
	}
	OS_TaskDelay(10);

	// 3) Set Est. mode & Cont. mode ( Main: EstGyroEkf  (6) / Main: EstGyro  (5) / Control: ConHxyzRW, ConXYZwheel, ConSunTrack / Timeout: 120+300
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t	SetVal_042 = {0,};
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t	RetVal_150 = {0,};

	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 5;
	// SetVal_042.ControlMode 			= 51;
	// SetVal_042.ControlTimeout		= 120;
	SetVal_042.ControlMode 			= 3;
	SetVal_042.ControlTimeout		= 0;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);	// Set Main Est mode = 6 / Set Backup Est mode = 5 / Control mode & Time out = 3 & 0 s
	OS_TaskDelay(100);
	interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);	// Get Main Est mode
	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_150.MainEstimatorMode == 6) && (RetVal_150.ControlMode == 3)) break;	// Check Mode Change well

		// if not, try again until 3 times
		interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);
		cnt_try++;
	}

	interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != 6) || (RetVal_150.ControlMode != 3))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -2;		// Something is wrong! ::::: Estimation Mode
		ADCS_HandleReport(status, ADCS_COMM_07_CC, &RetVal_150, sizeof(RetVal_150));
		return status;

	}

	// Before 5) Pre-define Hxyz Command values
	ADCS_Comm_OpenLoopCmdHxyzRWCmd_Payload_t SetVal_076 = {0,};
	SetVal_076.cmdHx = 0.0;
	SetVal_076.cmdHy = 0.0;
	SetVal_076.cmdHz = 0.0;
	interstatus = ADCS_Comm_SetOpenLoopCmdHxyzRW(&SetVal_076);
	ADCS_RecordFirstFailure(&OverallStatus, interstatus);
	
	// 5) Start to count time ( Waiting time before Logging: 2800 sec / Tlm period: 1 sec / Comm. Duration: 720 sec )
	CFE_TIME_SysTime_t		t0_10, t0_300, tnow;
	t0_10 = CFE_TIME_GetTime();

	uint32 dt = 0;
	while (dt < 2800)
	{	// Wait until dt > 2800 sec
		OS_TaskDelay(1000);
		tnow = CFE_TIME_GetTime();
		dt = tnow.Seconds - t0_10.Seconds;
	}

	// 4) Make "adcs_comm_07.bin" file to log the TLMs
	FILE *fp;
	size_t written;

	fp = fopen("./cf/adcs_comm_07.bin","wb");

	if (fp == NULL)
	{
		status = -5;		// Something is wrong! ::::: File open error
		ADCS_HandleReport(status, ADCS_COMM_07_CC, NULL, 0);

		SetVal_042.MainEstimatorMode 	= 6;
		SetVal_042.BackupEstimatorMode 	= 5;
		SetVal_042.ControlMode 			= 3;
		SetVal_042.ControlTimeout		= 0;
		interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);

		SetVal_056.RWL0		= 0;
		SetVal_056.RWL1		= 0;
		SetVal_056.RWL2		= 0;
		SetVal_056.RWL3		= 0;
		interstatus = ADCS_Comm_SetPowerState(&SetVal_056);		// Set RWLX --> OFF
		return status;
	}

	// 6) Read Tlm of Estimator & Sensors
	ADCS_Comm_ReferenceRPYvaluesCmd_Payload_t SetVal_054 = {0,};

	ADCS_Comm_Estimator_Cmn_Payload_t	RetVal_210 = {0,};
	ADCS_Comm_CalibratedRWLSensorTlm_Payload_t 	RetVal_209 = {0,};
	ADCS_Comm_CalibratedFSSSensorTlm_Payload_t	RetVal_178 = {0,};

	ADCS_Comm_COMM_07_Payload_t	Comm_07_Tlm_Set = {0,};

	Comm_07_Tlm_Set.sync_word 	= 0xADC5;

	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 5;
	SetVal_042.ControlMode 			= 51;
	SetVal_042.ControlTimeout		= 125;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	ADCS_RecordFirstFailure(&OverallStatus, interstatus);

	t0_300 = CFE_TIME_GetTime();
	uint32 dt300 = 0;
	bool phase_sun_12_set = false;
	bool phase_sun_13_set = false;
	bool phase_sun_3_set = false;
	bool phase_sun_50_set = false;
	while (dt300 < 850)
	{		

		if ((dt300 > 820) && (phase_sun_3_set == false)) {
			phase_sun_3_set = true;
			SetVal_042.MainEstimatorMode 	= 6;
			SetVal_042.BackupEstimatorMode 	= 5;
			SetVal_042.ControlMode 			= 3;
			SetVal_042.ControlTimeout		= 0;
			interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
			ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		}
		else if ((dt300 > 720) && (phase_sun_50_set == false)) {
			phase_sun_50_set = true;
			SetVal_042.MainEstimatorMode 	= 6;
			SetVal_042.BackupEstimatorMode 	= 5;
			SetVal_042.ControlMode 			= 50;
			SetVal_042.ControlTimeout		= 80;
			interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
			ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		}

		else if ((dt300 > 420) && (phase_sun_13_set == false)) {
			phase_sun_13_set = true;
			SetVal_042.MainEstimatorMode 	= 6;
			SetVal_042.BackupEstimatorMode 	= 5;
			SetVal_042.ControlMode 			= 13;
			SetVal_042.ControlTimeout		= 305;
			interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
			ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		}
		else if ((dt300 > 120) && (phase_sun_12_set == false)) {
			phase_sun_12_set = true;
			SetVal_054.Roll = 0.0;
			SetVal_054.Pitch = 0.0;
			SetVal_054.Yaw = 0.0;
			interstatus = ADCS_Comm_SetReferenceRPYValues(&SetVal_054);
			ADCS_RecordFirstFailure(&OverallStatus, interstatus);
			OS_TaskDelay(10);

			SetVal_042.MainEstimatorMode 	= 6;
			SetVal_042.BackupEstimatorMode 	= 5;
			SetVal_042.ControlMode 			= 12;
			SetVal_042.ControlTimeout		= 305;
			interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
			ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		}

		t0_10 = CFE_TIME_GetTime();	// Initialize 1 seconds Counter
		dt = 0;	// Initialize time gap
		

		memset(&RetVal_210, 0, sizeof(RetVal_210));
		memset(&RetVal_205, 0, sizeof(RetVal_205));
		memset(&RetVal_209, 0, sizeof(RetVal_209));
		memset(&RetVal_178, 0, sizeof(RetVal_178));
		
		// Comm_07_Tlm_Set.sync_word 	= 0xADC5;

		// 		6-1) Read Tlm of Main:	
		interstatus = ADCS_Comm_GetMainEstTlm(&RetVal_210);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(20);

		// 		6-2) Read Sensor Raw Values - RWL	
		interstatus = ADCS_Comm_GetRawRWLSensor(&RetVal_205);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(20);

		// 		6-3) Read Sensor Calibrated Values - RWL
		interstatus = ADCS_Comm_GetCalibratedRWLSensor(&RetVal_209);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(20);
		
		// 		6-4) Read Sensor Calibrated Values - FSS
		interstatus = ADCS_Comm_GetCalibratedFSSSensor(&RetVal_178);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(20);
		
		// 7) Write the Tlm to the file
		Comm_07_Tlm_Set.MainEst		= RetVal_210;
		Comm_07_Tlm_Set.RawRWL		= RetVal_205;
		Comm_07_Tlm_Set.CalRWL		= RetVal_209;
		Comm_07_Tlm_Set.CalFSS		= RetVal_178;

		if ((written = fwrite(&Comm_07_Tlm_Set, sizeof(Comm_07_Tlm_Set), 1, fp)) != 1)
		{
			status = -6;		// Something is wrong! ::::: File write error
			ADCS_HandleReport(status, ADCS_COMM_07_CC, NULL, 0);
			fclose(fp);

			SetVal_042.MainEstimatorMode 	= 6;
			SetVal_042.BackupEstimatorMode 	= 5;
			SetVal_042.ControlMode 			= 3;
			SetVal_042.ControlTimeout		= 0;
			interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);

			SetVal_056.RWL0		= 0;
			SetVal_056.RWL1		= 0;
			SetVal_056.RWL2		= 0;
			SetVal_056.RWL3		= 0;
			interstatus = ADCS_Comm_SetPowerState(&SetVal_056);		// Set RWLX --> OFF
			return status;
		}


		// 8) Check time from beginning of Step 6 to end of Step 7 (dt) -> Wait the remain time (10 - dt)
		tnow = CFE_TIME_GetTime();
		dt300 = tnow.Seconds - t0_300.Seconds;
		dt = tnow.Seconds - t0_10.Seconds;

		while (dt < 1)
		{	// Wait until dt > 1 sec
			OS_TaskDelay(50);
			tnow = CFE_TIME_GetTime();
			dt = tnow.Seconds - t0_10.Seconds;
		}
		
	// 9) Go back to Step 6 until total time meets 720 sec
	}
	if (fclose(fp) != 0)
	{
		ADCS_RecordFirstFailure(&OverallStatus, CFE_STATUS_EXTERNAL_RESOURCE_FAIL);
	}

	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 5;
	SetVal_042.ControlMode 			= 3;
	SetVal_042.ControlTimeout		= 0;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	ADCS_RecordFirstFailure(&OverallStatus, interstatus);

	SetVal_056.RWL0		= 0;
	SetVal_056.RWL1		= 0;
	SetVal_056.RWL2		= 0;
	SetVal_056.RWL3		= 0;
	interstatus = ADCS_Comm_SetPowerState(&SetVal_056);		// Set RWLX --> OFF
	ADCS_RecordFirstFailure(&OverallStatus, interstatus);

	return ADCS_ReportCommandCompletion(ADCS_COMM_07_CC, OverallStatus);
}

CFE_Status_t ADCS_Comm08Cmd(const ADCS_Comm08Cmd_t *msg) {
	ADCS_ReportCommandPhase(ADCS_COMM_08_CC, ADCS_RPT_PHASE_STARTED);

	// COMM 08:	HSS Commissioning
	// [Procedure]
	// 1) Power On ( GYR0, MAG0, FSS0, HSS0, RWLX )
	// 2) Check Status of Sensors
	// 3) Set Est. mode & Cont. mode ( Main: EstGyroEkf  (6) / Main: EstFullEkf  (5) / Control: ConXYXwheel / Timeout: 6000 s )
	//	  Set Flag of UseEkf: HSS, STT elements false <-- GS Command (already set-up)
	// 4) Make "adcs_comm_08.bin" file to log the TLMs
	// 5) Start to count time ( Waiting time before Logging: 10 sec / Tlm period: 10 sec / Comm. Duration: 6000 sec )
	// 6) Read Tlm of Estimator & Sensors
	// 		6-1) Read Tlm of Main Estimator
	// 		6-2) Read Sensor Raw Values - HSS
	// 		6-3) Read Sensor Calibrated Values - HSS
	// 7) Append the Tlm to the file
	// 8) Check time from beginning of Step 6 to end of Step 7 (dt) -> Wait the remain time (10 - dt)
	// 9) Go back to Step 6 until total time meets 6000 sec
	// ** Status between every step must be reported!!


	CFE_Status_t						status;
	CFE_Status_t						interstatus;
	CFE_Status_t                        OverallStatus = CFE_SUCCESS;

	uint8 cnt_try = 0;
	
	// 1) Power On ( GYR0, MAG0, FSS0, HSS0, RWLX )
	ADCS_Comm_PowerState_Cmn_Payload_t		SetVal_056 = {0,};
	ADCS_Comm_PowerState_Cmn_Payload_t		RetVal_183 = {0,};
	SetVal_056.MAG0		= 1;
	SetVal_056.GYR0		= 1;
	SetVal_056.FSS0		= 1;
	SetVal_056.HSS0		= 1;
	SetVal_056.RWL0		= 1;
	SetVal_056.RWL1		= 1;
	SetVal_056.RWL2		= 1;
	SetVal_056.RWL3		= 1;
	interstatus = ADCS_Comm_SetPowerState(&SetVal_056);		// Set GYR0 & MAG0 & FSS0 & HSS0 & RWLX --> ON
	OS_TaskDelay(10);
	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);		// Get GYR0 & MAG0 & FSS0 & HSS0 & RWLX power state
	OS_TaskDelay(5000);
	cnt_try = 0;
	while (cnt_try < 3)
	{		
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1) && (RetVal_183.FSS0 == 1) && (RetVal_183.HSS0 == 1) && (RetVal_183.RWL0 == 1) && (RetVal_183.RWL1 == 1) && (RetVal_183.RWL2 == 1) && (RetVal_183.RWL3 == 1)) break;	// Check Power On well

		// if not, try again until 3 times
		interstatus = ADCS_Comm_SetPowerState(&SetVal_056);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
		cnt_try++;
	}

	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1) || (RetVal_183.FSS0 != 1) || (RetVal_183.HSS0 != 1) || (RetVal_183.RWL0 != 1) || (RetVal_183.RWL1 != 1) || (RetVal_183.RWL2 != 1) || (RetVal_183.RWL3 != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -1;		// Something is wrong! ::::: Power State
		ADCS_HandleReport(status, ADCS_COMM_08_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}


	// 2) Check Status of Sensors
	ADCS_Comm_RawMAGSensorTlm_Paylaod_t	RetVal_180 = {0,};
	ADCS_Comm_RawGYRSensorTlm_Payload_t	RetVal_204 = {0,};
	// ADCS_Comm_RawCubeSenseSunTlm_Payload_t RetVal_170 = {0,};		// "FSS0 valid" is not identified yet. After idetification, use this status to check the state of FSS0.
	ADCS_Comm_RawRWLSensorTlm_Payload_t RetVal_205 = {0,};
	// ADCS_Comm_RawCubeSenseEarthTlm_Payload_t RetVal_179 = {0,};		// "HSS0 valid" is not identified yet. After idetification, use this status to check the state of FSS0.

	OS_TaskDelay(2000);
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS_Comm_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS_Comm_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels
	

	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1) && (RetVal_205.RWL0ValidFlag == 1) && (RetVal_205.RWL1ValidFlag == 1) && (RetVal_205.RWL2ValidFlag == 1) && (RetVal_205.RWL3ValidFlag == 1)) break;	// Check Sensors Status Good		

		// if not, wait changing status until 3 times
		OS_TaskDelay(100);
		interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS_Comm_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS_Comm_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels
		cnt_try++;
	}

	
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS_Comm_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS_Comm_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels

	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1) || (RetVal_204.GYR0ValidFlag != 1) || (RetVal_205.RWL0ValidFlag != 1) || (RetVal_205.RWL1ValidFlag != 1) || (RetVal_205.RWL2ValidFlag != 1) || (RetVal_205.RWL3ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -3;		// Something is wrong! ::::: Magnetometer Status
		ADCS_HandleReport(status, ADCS_COMM_08_CC, &RetVal_180, sizeof(RetVal_180));
		return status;
	}
	OS_TaskDelay(10);


	// 3) Set Est. mode & Cont. mode ( Main: EstGyroEkf  (6) / Main: EstFullEkf  (5) / Control: ConXYXwheel / Timeout: 6000 s )
	ADCS_Comm_ReferenceRPYvaluesCmd_Payload_t 		SetVal_054 = {0,};
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t	SetVal_042 = {0,};
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t	RetVal_150 = {0,};

	SetVal_054.Roll = 0.0;
	SetVal_054.Pitch = 0.0;
	SetVal_054.Yaw = 0.0;
	interstatus = ADCS_Comm_SetReferenceRPYValues(&SetVal_054);
	ADCS_RecordFirstFailure(&OverallStatus, interstatus);

	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 5;
	SetVal_042.ControlMode 			= 12;
	SetVal_042.ControlTimeout		= 6000;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);	// Set Main Est mode = 6 / Set Backup Est mode = 5 / Control mode & Time out = 3 & 6000 s
	OS_TaskDelay(100);
	interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);	// Get Main Est mode
	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_150.MainEstimatorMode == 6) && (RetVal_150.ControlMode == 12)) break;	// Check Mode Change well

		// if not, try again until 3 times
		interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);
		cnt_try++;
	}

	interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != 6) || (RetVal_150.ControlMode != 12))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -2;		// Something is wrong! ::::: Estimation Mode
		ADCS_HandleReport(status, ADCS_COMM_08_CC, &RetVal_150, sizeof(RetVal_150));

		
		SetVal_042.MainEstimatorMode 	= 6;
		SetVal_042.BackupEstimatorMode 	= 5;
		SetVal_042.ControlMode 			= 3;
		SetVal_042.ControlTimeout		= 0;
		interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);

		SetVal_056.HSS0		= 0;
		SetVal_056.RWL0		= 0;
		SetVal_056.RWL1		= 0;
		SetVal_056.RWL2		= 0;
		SetVal_056.RWL3		= 0;
		interstatus = ADCS_Comm_SetPowerState(&SetVal_056);		// Set RWLX --> OFF

		return status;

	}

	// 4) Make "adcs_comm_08.bin" file to log the TLMs
	FILE *fp;
	size_t written;
	fp = fopen("./cf/adcs_comm_08.bin","wb");
	if (fp == NULL)
	{
		status = -5;		// Something is wrong! ::::: File open error
		ADCS_HandleReport(status, ADCS_COMM_08_CC, NULL, 0);
		
		SetVal_042.MainEstimatorMode 	= 6;
		SetVal_042.BackupEstimatorMode 	= 5;
		SetVal_042.ControlMode 			= 3;
		SetVal_042.ControlTimeout		= 0;
		interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);

		SetVal_056.HSS0		= 0;
		SetVal_056.RWL0		= 0;
		SetVal_056.RWL1		= 0;
		SetVal_056.RWL2		= 0;
		SetVal_056.RWL3		= 0;
		interstatus = ADCS_Comm_SetPowerState(&SetVal_056);		// Set RWLX --> OFF
		return status;
	}

	// 5) Start to count time ( Waiting time before Logging: 10 sec / Tlm period: 10 sec / Comm. Duration: 6000 sec )
	CFE_TIME_SysTime_t		t0_10, t0_300, tnow;
	t0_10 = CFE_TIME_GetTime();

	uint32 dt = 0;
	while (dt < 10)
	{	// Wait until dt > 10 sec
		OS_TaskDelay(100);
		tnow = CFE_TIME_GetTime();
		dt = tnow.Seconds - t0_10.Seconds;
	}

	// 6) Read Tlm of Estimator & Sensors
	ADCS_Comm_Estimator_Cmn_Payload_t	RetVal_210 = {0,};

	ADCS_Comm_RawCubeSenseEarthTlm_Payload_t 	RetVal_179 = {0,};
	ADCS_Comm_CalibratedHSSSensorTlm_Payload_t	RetVal_176 = {0,};

	ADCS_Comm_COMM_08_Payload_t	Comm_08_Tlm_Set = {0,};

	Comm_08_Tlm_Set.sync_word 	= 0xADC5;

	t0_300 = CFE_TIME_GetTime();
	uint32 dt300 = 0;
	while (dt300 < 6000)
	{
		t0_10 = CFE_TIME_GetTime();	// Initialize 10 seconds Counter
		dt = 0;	// Initialize time gap
		

		memset(&RetVal_210, 0, sizeof(RetVal_210));
		memset(&RetVal_179, 0, sizeof(RetVal_179));
		memset(&RetVal_176, 0, sizeof(RetVal_176));

		// Comm_08_Tlm_Set.sync_word 	= 0xADC5;

		// 		6-1) Read Tlm of Main:	
		interstatus = ADCS_Comm_GetMainEstTlm(&RetVal_210);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(20);

		// 		6-2) Read Sensor Raw Values - HSS	
		interstatus = ADCS_Comm_GetRawCubeSenseEarth(&RetVal_179);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(20);

		// 		6-3) Read Sensor Calibrated Values - HSS
		interstatus = ADCS_Comm_GetCalibratedHSSSensor(&RetVal_176);
		ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		OS_TaskDelay(20);

		
		// 7) Write the Tlm to the file
		Comm_08_Tlm_Set.MainEst		= RetVal_210;
		Comm_08_Tlm_Set.RawHSS 		= RetVal_179;
		Comm_08_Tlm_Set.CalHSS 		= RetVal_176;

		if ((written = fwrite(&Comm_08_Tlm_Set, sizeof(Comm_08_Tlm_Set), 1, fp)) != 1)
		{
			status = -6;		// Something is wrong! ::::: File write error
			ADCS_HandleReport(status, ADCS_COMM_08_CC, NULL, 0);
			fclose(fp);
			
			SetVal_042.MainEstimatorMode 	= 6;
			SetVal_042.BackupEstimatorMode 	= 5;
			SetVal_042.ControlMode 			= 3;
			SetVal_042.ControlTimeout		= 0;
			interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);

			SetVal_056.HSS0		= 0;
			SetVal_056.RWL0		= 0;
			SetVal_056.RWL1		= 0;
			SetVal_056.RWL2		= 0;
			SetVal_056.RWL3		= 0;
			interstatus = ADCS_Comm_SetPowerState(&SetVal_056);		// Set RWLX --> OFF
			return status;
		}


		// 8) Check time from beginning of Step 6 to end of Step 7 (dt) -> Wait the remain time (10 - dt)
		tnow = CFE_TIME_GetTime();
		dt300 = tnow.Seconds - t0_300.Seconds;
		dt = tnow.Seconds - t0_10.Seconds;

		while (dt < 10)
		{	// Wait until dt > 10 sec
			OS_TaskDelay(100);
			tnow = CFE_TIME_GetTime();
			dt = tnow.Seconds - t0_10.Seconds;
		}
		
	// 9) Go back to Step 6 until total time meets 6000 sec
	}
	if (fclose(fp) != 0)
	{
		ADCS_RecordFirstFailure(&OverallStatus, CFE_STATUS_EXTERNAL_RESOURCE_FAIL);
	}

	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 5;
	SetVal_042.ControlMode 			= 3;
	SetVal_042.ControlTimeout		= 0;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	ADCS_RecordFirstFailure(&OverallStatus, interstatus);

	SetVal_056.HSS0		= 0;
	SetVal_056.RWL0		= 0;
	SetVal_056.RWL1		= 0;
	SetVal_056.RWL2		= 0;
	SetVal_056.RWL3		= 0;
	interstatus = ADCS_Comm_SetPowerState(&SetVal_056);		// Set RWLX --> OFF
	ADCS_RecordFirstFailure(&OverallStatus, interstatus);

	return ADCS_ReportCommandCompletion(ADCS_COMM_08_CC, OverallStatus);
}

CFE_Status_t ADCS_SequenceCmd_Sunpointing(void) {
	ADCS_ReportCommandPhase(ADCS_SEQ_SUN_CC, ADCS_RPT_PHASE_STARTED);

	// Sun Tracking 3-axis Control
	// [Procedure]
	// 1) Power On ( GYR0, MAG0, FSS0, RWLX )
	// 2) Check Status of Sensors
	// 3) Set Est. mode & Cont. mode ( Main: EstGyroEkf  (6) / Main: EstGyro  (5) / Control: ConHxyzRW, ConXYZwheel, ConSunTrack / Timeout: 120+300+500 s )
	//	  Set Flag of UseEkf: (HSS), STT elements false <-- GS Command (already set-up)
	// ** Status between every step must be reported!!

	CFE_Status_t						status;
	CFE_Status_t						interstatus;
	CFE_Status_t                        OverallStatus = CFE_SUCCESS;

	uint8 cnt_try = 0;
		
	// 1) Power On ( GYR0, MAG0, FSS0, RWLX )
	ADCS_Comm_PowerState_Cmn_Payload_t		SetVal_056 = {0,};
	ADCS_Comm_PowerState_Cmn_Payload_t		RetVal_183 = {0,};
	SetVal_056.MAG0		= 1;
	SetVal_056.GYR0		= 1;
	SetVal_056.FSS0		= 1;
	SetVal_056.RWL0		= 1;
	SetVal_056.RWL1		= 1;
	SetVal_056.RWL2		= 1;
	SetVal_056.RWL3		= 1;

	interstatus = ADCS_Comm_SetPowerState(&SetVal_056);		// Set GYR0 & MAG0 & FSS0 & RWLX --> ON
	OS_TaskDelay(10);
	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);		// Get GYR0 & MAG0 & FSS0 & RWLX power state
	OS_TaskDelay(5000);
	cnt_try = 0;
	while (cnt_try < 3)
	{		
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1) && (RetVal_183.FSS0 == 1) && (RetVal_183.RWL0 == 1) && (RetVal_183.RWL1 == 1) && (RetVal_183.RWL2 == 1) && (RetVal_183.RWL3 == 1)) break;	// Check Power On well

		// if not, try again until 3 times
		interstatus = ADCS_Comm_SetPowerState(&SetVal_056);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
		cnt_try++;
	}

	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1) || (RetVal_183.FSS0 != 1) || (RetVal_183.RWL0 != 1) || (RetVal_183.RWL1 != 1) || (RetVal_183.RWL2 != 1) || (RetVal_183.RWL3 != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -1;		// Something is wrong! ::::: Power State
		ADCS_HandleReport(status, ADCS_SEQ_SUN_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}


	// 2) Check Status of Sensors
	ADCS_Comm_RawMAGSensorTlm_Paylaod_t	RetVal_180 = {0,};
	ADCS_Comm_RawGYRSensorTlm_Payload_t	RetVal_204 = {0,};
	// ADCS_Comm_RawCubeSenseSunTlm_Payload_t RetVal_170 = {0,};		// "FSS0 valid" is not identified yet. After idetification, use this status to check the state of FSS0. -> (line 1578, 1584)
	ADCS_Comm_RawRWLSensorTlm_Payload_t RetVal_205 = {0,};

	OS_TaskDelay(2000);
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS_Comm_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	// OS_TaskDelay(10);
	// interstatus = interstatus + ADCS_Comm_GetRawCubeSenseSun(&RetVal_170);	// Get Status of FSS
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS_Comm_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels
	

	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1) && (RetVal_205.RWL0ValidFlag == 1) && (RetVal_205.RWL1ValidFlag == 1) && (RetVal_205.RWL2ValidFlag == 1) && (RetVal_205.RWL3ValidFlag == 1)) break;	// Check Sensors Status Good
		// if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1) && (RetVal_170.ValidResult0 == 1)) break;	// Check Sensors Status Good

		// if not, wait changing status until 3 times
		OS_TaskDelay(100);
		interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS_Comm_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS_Comm_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels
		cnt_try++;
	}

	
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS_Comm_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS_Comm_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels

	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1) || (RetVal_204.GYR0ValidFlag != 1) || (RetVal_205.RWL0ValidFlag != 1) || (RetVal_205.RWL1ValidFlag != 1) || (RetVal_205.RWL2ValidFlag != 1) || (RetVal_205.RWL3ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -3;		// Something is wrong! ::::: Magnetometer Status
		ADCS_HandleReport(status, ADCS_SEQ_SUN_CC, &RetVal_180, sizeof(RetVal_180));
		return status;
	}
	OS_TaskDelay(10);

	// 3) Set Est. mode & Cont. mode ( Main: EstGyroEkf  (6) / Main: EstGyro  (5) / Control: ConHxyzRW, ConXYZwheel, ConSunTrack / Timeout: 120+300
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t	SetVal_042 = {0,};
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t	RetVal_150 = {0,};

	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 5;
	// SetVal_042.ControlMode 			= 51;
	// SetVal_042.ControlTimeout		= 120;
	SetVal_042.ControlMode 			= 3;
	SetVal_042.ControlTimeout		= 0;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);	// Set Main Est mode = 6 / Set Backup Est mode = 5 / Control mode & Time out = 3 & 0 s
	OS_TaskDelay(100);
	interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);	// Get Main Est mode
	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_150.MainEstimatorMode == 6) && (RetVal_150.ControlMode == 3)) break;	// Check Mode Change well

		// if not, try again until 3 times
		interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		OS_TaskDelay(10);
		interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);
		cnt_try++;
	}

	interstatus = ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != 6) || (RetVal_150.ControlMode != 3))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -2;		// Something is wrong! ::::: Estimation Mode
		ADCS_HandleReport(status, ADCS_SEQ_SUN_CC, &RetVal_150, sizeof(RetVal_150));
		return status;

	}

	// Before 5) Pre-define Hxyz Command values
	ADCS_Comm_OpenLoopCmdHxyzRWCmd_Payload_t SetVal_076 = {0,};
	SetVal_076.cmdHx = 0.0;
	SetVal_076.cmdHy = 0.0;
	SetVal_076.cmdHz = 0.0;
	interstatus = ADCS_Comm_SetOpenLoopCmdHxyzRW(&SetVal_076);
	ADCS_RecordFirstFailure(&OverallStatus, interstatus);
	
	// 5) Start to count time ( Waiting time before Logging: 2800 sec / Tlm period: 1 sec / Comm. Duration: 720 sec )
	CFE_TIME_SysTime_t		t0_10, t0_300, tnow;
	t0_10 = CFE_TIME_GetTime();

	uint32 dt = 0;
	while (dt < 2800)
	{	// Wait until dt > 2800 sec
		OS_TaskDelay(1000);
		tnow = CFE_TIME_GetTime();
		dt = tnow.Seconds - t0_10.Seconds;
	}

	ADCS_Comm_ReferenceRPYvaluesCmd_Payload_t SetVal_054 = {0,};
	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 5;
	SetVal_042.ControlMode 			= 51;
	SetVal_042.ControlTimeout		= 125;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	ADCS_RecordFirstFailure(&OverallStatus, interstatus);

	t0_300 = CFE_TIME_GetTime();
	uint32 dt300 = 0;
	bool phase_ctrl_12_set = false;
	bool phase_ctrl_13_set = false;
	while (dt300 < 720)
	{
		if ((dt300 > 420) && (phase_ctrl_13_set == false)) {
			phase_ctrl_13_set = true;
			SetVal_042.MainEstimatorMode 	= 6;
			SetVal_042.BackupEstimatorMode 	= 5;
			SetVal_042.ControlMode 			= 13;
			SetVal_042.ControlTimeout		= 0;
			interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
			ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		}
		else if ((dt300 > 120) && (phase_ctrl_12_set == false)) {
			phase_ctrl_12_set = true;
			SetVal_054.Roll = 0.0;
			SetVal_054.Pitch = 0.0;
			SetVal_054.Yaw = 0.0;
			interstatus = ADCS_Comm_SetReferenceRPYValues(&SetVal_054);
			ADCS_RecordFirstFailure(&OverallStatus, interstatus);
			OS_TaskDelay(10);

			SetVal_042.MainEstimatorMode 	= 6;
			SetVal_042.BackupEstimatorMode 	= 5;
			SetVal_042.ControlMode 			= 12;
			SetVal_042.ControlTimeout		= 305;
			interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
			ADCS_RecordFirstFailure(&OverallStatus, interstatus);
		}

		t0_10 = CFE_TIME_GetTime();	// Initialize 1 seconds Counter
		dt = 0;	// Initialize time gap


		// 8) Check time from beginning of Step 6 to end of Step 7 (dt) -> Wait the remain time (10 - dt)
		tnow = CFE_TIME_GetTime();
		dt300 = tnow.Seconds - t0_300.Seconds;

		OS_TaskDelay(1000);

	}

	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 5;
	SetVal_042.ControlMode 			= 13;
	SetVal_042.ControlTimeout		= 0;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	ADCS_RecordFirstFailure(&OverallStatus, interstatus);

	return ADCS_ReportCommandCompletion(ADCS_SEQ_SUN_CC, OverallStatus);
}

CFE_Status_t ADCS_Comm10Cmd(const ADCS_Comm10Cmd_t *msg)
{
	ADCS_ReportCommandPhase(ADCS_COMM_10_CC, ADCS_RPT_PHASE_STARTED);

	// COMM 10: ConGndTrack (mode 16) commissioning.
	// Reference LLH and ConfigAdcsSatellite.TgtTrackBodyVec are preconfigured by ground command.
	CFE_Status_t status = CFE_SUCCESS;
	int32 interstatus = CFE_SUCCESS;
	uint8 cnt_try = 0;
	uint32 sample_index;
	uint8 estimator_mode = msg->Payload.flag_estmode;
	uint16 target_duration = msg->Payload.target_duration;
	ADCS_Comm_PowerState_Cmn_Payload_t SetVal_056 = {0,};
	ADCS_Comm_PowerState_Cmn_Payload_t RetVal_183 = {0,};
	ADCS_Comm_RawMAGSensorTlm_Paylaod_t RetVal_180 = {0,};
	ADCS_Comm_RawGYRSensorTlm_Payload_t RetVal_204 = {0,};
	ADCS_Comm_RawRWLSensorTlm_Payload_t RetVal_205 = {0,};
	ADCS_Comm_OpenLoopCmdHxyzRWCmd_Payload_t SetVal_076 = {0,};
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t SetVal_042 = {0,};
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t RetVal_150 = {0,};
	ADCS_Comm_TargetTracking_Payload_t telemetry = {0,};
	FILE *fp;

	if (estimator_mode == 0u)
	{
		estimator_mode = 6u;
	}
	if ((estimator_mode != 5u) && (estimator_mode != 6u))
	{
		status = -1;
		ADCS_HandleReport(status, ADCS_COMM_10_CC, (void *)&msg->Payload, sizeof(msg->Payload));
		return status;
	}
	if (target_duration == 0u)
	{
		target_duration = 600u;
	}
	// 1) Power on the sensors and reaction wheels required for target tracking.
	SetVal_056.MAG0 = 1;
	SetVal_056.GYR0 = 1;
	SetVal_056.RWL0 = 1;
	SetVal_056.RWL1 = 1;
	SetVal_056.RWL2 = 1;
	SetVal_056.RWL3 = 1;
	interstatus = ADCS_Comm_SetPowerState(&SetVal_056);
	OS_TaskDelay(500);
	interstatus += ADCS_Comm_GetPowerState(&RetVal_183);
	OS_TaskDelay(5000);

	for (cnt_try = 0; cnt_try < 3; ++cnt_try)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1) &&
			(RetVal_183.RWL0 == 1) && (RetVal_183.RWL1 == 1) &&
			(RetVal_183.RWL2 == 1) && (RetVal_183.RWL3 == 1))
		{
			break;
		}

		interstatus = ADCS_Comm_SetPowerState(&SetVal_056);
		OS_TaskDelay(500);
		interstatus += ADCS_Comm_GetPowerState(&RetVal_183);
		OS_TaskDelay(500);
	}

	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1) ||
		(RetVal_183.RWL0 != 1) || (RetVal_183.RWL1 != 1) ||
		(RetVal_183.RWL2 != 1) || (RetVal_183.RWL3 != 1))
	{
		status = -2;
		ADCS_HandleReport(status, ADCS_COMM_10_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}

	// 2) Verify sensor and wheel telemetry.
	OS_TaskDelay(2000);
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);
	OS_TaskDelay(100);
	interstatus += ADCS_Comm_GetRawGYRSensor(&RetVal_204);
	OS_TaskDelay(100);
	interstatus += ADCS_Comm_GetRawRWLSensor(&RetVal_205);

	for (cnt_try = 0; cnt_try < 3; ++cnt_try)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) &&
			(RetVal_204.GYR0ValidFlag == 1) && (RetVal_205.RWL0ValidFlag == 1) &&
			(RetVal_205.RWL1ValidFlag == 1) && (RetVal_205.RWL2ValidFlag == 1) &&
			(RetVal_205.RWL3ValidFlag == 1))
		{
			break;
		}

		OS_TaskDelay(500);
		interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(100);
		interstatus += ADCS_Comm_GetRawGYRSensor(&RetVal_204);
		OS_TaskDelay(100);
		interstatus += ADCS_Comm_GetRawRWLSensor(&RetVal_205);
	}

	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1) ||
		(RetVal_204.GYR0ValidFlag != 1) || (RetVal_205.RWL0ValidFlag != 1) ||
		(RetVal_205.RWL1ValidFlag != 1) || (RetVal_205.RWL2ValidFlag != 1) ||
		(RetVal_205.RWL3ValidFlag != 1))
	{
		status = -3;
		ADCS_HandleReport(status, ADCS_COMM_10_CC, &RetVal_205, sizeof(RetVal_205));
		return status;
	}

	// 3) Clear the open-loop wheel momentum command.
	SetVal_076.cmdHx = 0.0;
	SetVal_076.cmdHy = 0.0;
	SetVal_076.cmdHz = 0.0;
	interstatus = ADCS_Comm_SetOpenLoopCmdHxyzRW(&SetVal_076);
	OS_TaskDelay(500);
	if (interstatus != CFE_SUCCESS)
	{
		status = -8;
		ADCS_HandleReport(status, ADCS_COMM_10_CC, &SetVal_076, sizeof(SetVal_076));
		return status;
	}

	// 4) Prepare control: mode 3 -> mode 51 -> mode 12.
	SetVal_042.MainEstimatorMode = estimator_mode;
	SetVal_042.BackupEstimatorMode = 5;
	SetVal_042.ControlMode = 3;
	SetVal_042.ControlTimeout = 0;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	OS_TaskDelay(1000);
	interstatus += ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != estimator_mode) ||
		(RetVal_150.ControlMode != 3))
	{
		status = -12;
		ADCS_HandleReport(status, ADCS_COMM_10_CC, &RetVal_150, sizeof(RetVal_150));
		return status;
	}

	SetVal_042.ControlMode = 51;
	SetVal_042.ControlTimeout = 125;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	OS_TaskDelay(1000);
	interstatus += ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != estimator_mode) ||
		(RetVal_150.ControlMode != 51))
	{
		status = -9;
		ADCS_HandleReport(status, ADCS_COMM_10_CC, &RetVal_150, sizeof(RetVal_150));
		SetVal_042.ControlMode = 3;
		SetVal_042.ControlTimeout = 0;
		(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		return status;
	}
	OS_TaskDelay(120000);

	SetVal_042.ControlMode = 12;
	SetVal_042.ControlTimeout = 305;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	OS_TaskDelay(1000);
	interstatus += ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != estimator_mode) ||
		(RetVal_150.ControlMode != 12))
	{
		status = -10;
		ADCS_HandleReport(status, ADCS_COMM_10_CC, &RetVal_150, sizeof(RetVal_150));
		SetVal_042.ControlMode = 3;
		SetVal_042.ControlTimeout = 0;
		(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		return status;
	}
	OS_TaskDelay(300000);

	// 5) Open the commissioning log before starting target control.
	fp = fopen("./cf/adcs_comm_10_gnd.bin", "wb");
	if (fp == NULL)
	{
		status = -20;
		SetVal_042.ControlMode = 3;
		SetVal_042.ControlTimeout = 0;
		(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		ADCS_HandleReport(status, ADCS_COMM_10_CC, NULL, 0);
		return status;
	}

	// 6) Start ConGndTrack (mode 16).
	SetVal_042.ControlMode = ADCS_CONMODE_RW_GS_TARGET_TRACK;
	SetVal_042.ControlTimeout = 0;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	OS_TaskDelay(1000);
	interstatus += ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != estimator_mode) ||
		(RetVal_150.ControlMode != ADCS_CONMODE_RW_GS_TARGET_TRACK))
	{
		status = -21;
		fclose(fp);
		ADCS_HandleReport(status, ADCS_COMM_10_CC, &RetVal_150, sizeof(RetVal_150));
		SetVal_042.ControlMode = 3;
		SetVal_042.ControlTimeout = 0;
		(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		return status;
	}

	// 7) Log the estimator and raw/calibrated reaction-wheel telemetry at 1 Hz.
	telemetry.sync_word = 0xADC5u;
	for (sample_index = 0u; sample_index < target_duration; ++sample_index)
	{
		interstatus = ADCS_Comm_GetMainEstHighResTlm(&telemetry.MainEstHighRes);
		if (interstatus == CFE_SUCCESS)
		{
			OS_TaskDelay(10);
			interstatus = ADCS_Comm_GetRawRWLSensor(&telemetry.RawRWL);
		}
		if (interstatus == CFE_SUCCESS)
		{
			OS_TaskDelay(10);
			interstatus = ADCS_Comm_GetCalibratedRWLSensor(&telemetry.CalibratedRWL);
		}
		if (interstatus != CFE_SUCCESS)
		{
			fclose(fp);
			SetVal_042.ControlMode = 3;
			SetVal_042.ControlTimeout = 0;
			(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
			ADCS_HandleReport(interstatus, ADCS_COMM_10_CC, &telemetry, sizeof(telemetry));
			return interstatus;
		}

		if (fwrite(&telemetry, sizeof(telemetry), 1, fp) != 1u)
		{
			fclose(fp);
			status = -22;
			SetVal_042.ControlMode = 3;
			SetVal_042.ControlTimeout = 0;
			(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
			ADCS_HandleReport(status, ADCS_COMM_10_CC, NULL, 0);
			return status;
		}

		if ((sample_index + 1u) < target_duration)
		{
			OS_TaskDelay(1000);
		}
	}

	if (fclose(fp) != 0)
	{
		status = -23;
		SetVal_042.ControlMode = 3;
		SetVal_042.ControlTimeout = 0;
		(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		ADCS_HandleReport(status, ADCS_COMM_10_CC, NULL, 0);
		return status;
	}

	// 8) Return to mode 3 after commissioning.
	SetVal_042.ControlMode = 3;
	SetVal_042.ControlTimeout = 0;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	if (interstatus != CFE_SUCCESS)
	{
		ADCS_HandleReport(interstatus, ADCS_COMM_10_CC, NULL, 0);
		return interstatus;
	}

	ADCS_ReportCommandPhase(ADCS_COMM_10_CC, ADCS_RPT_PHASE_COMPLETED);
	return CFE_SUCCESS;
}

CFE_Status_t ADCS_Comm11Cmd(const ADCS_Comm11Cmd_t *msg)
{
	ADCS_ReportCommandPhase(ADCS_COMM_11_CC, ADCS_RPT_PHASE_STARTED);

	// COMM 11: ConTgtTrack (mode 14) commissioning.
	// Reference LLH is preconfigured; ConTgtTrack always points the fixed +Z_B axis.
	CFE_Status_t status = CFE_SUCCESS;
	int32 interstatus = CFE_SUCCESS;
	uint8 cnt_try = 0;
	uint32 sample_index;
	uint8 estimator_mode = msg->Payload.flag_estmode;
	uint16 target_duration = msg->Payload.target_duration;
	ADCS_Comm_PowerState_Cmn_Payload_t SetVal_056 = {0,};
	ADCS_Comm_PowerState_Cmn_Payload_t RetVal_183 = {0,};
	ADCS_Comm_RawMAGSensorTlm_Paylaod_t RetVal_180 = {0,};
	ADCS_Comm_RawGYRSensorTlm_Payload_t RetVal_204 = {0,};
	ADCS_Comm_RawRWLSensorTlm_Payload_t RetVal_205 = {0,};
	ADCS_Comm_OpenLoopCmdHxyzRWCmd_Payload_t SetVal_076 = {0,};
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t SetVal_042 = {0,};
	ADCS_Comm_ControlEstimationMode_Cmn_Payload_t RetVal_150 = {0,};
	ADCS_Comm_TargetTracking_Payload_t telemetry = {0,};
	FILE *fp;

	if (estimator_mode == 0u)
	{
		estimator_mode = 6u;
	}
	if ((estimator_mode != 5u) && (estimator_mode != 6u))
	{
		status = -1;
		ADCS_HandleReport(status, ADCS_COMM_11_CC, (void *)&msg->Payload, sizeof(msg->Payload));
		return status;
	}
	if (target_duration == 0u)
	{
		target_duration = 600u;
	}
	// 1) Power on the sensors and reaction wheels required for target tracking.
	SetVal_056.MAG0 = 1;
	SetVal_056.GYR0 = 1;
	SetVal_056.RWL0 = 1;
	SetVal_056.RWL1 = 1;
	SetVal_056.RWL2 = 1;
	SetVal_056.RWL3 = 1;
	interstatus = ADCS_Comm_SetPowerState(&SetVal_056);
	OS_TaskDelay(500);
	interstatus += ADCS_Comm_GetPowerState(&RetVal_183);
	OS_TaskDelay(5000);

	for (cnt_try = 0; cnt_try < 3; ++cnt_try)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1) &&
			(RetVal_183.RWL0 == 1) && (RetVal_183.RWL1 == 1) &&
			(RetVal_183.RWL2 == 1) && (RetVal_183.RWL3 == 1))
		{
			break;
		}

		interstatus = ADCS_Comm_SetPowerState(&SetVal_056);
		OS_TaskDelay(500);
		interstatus += ADCS_Comm_GetPowerState(&RetVal_183);
		OS_TaskDelay(500);
	}

	interstatus = ADCS_Comm_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1) ||
		(RetVal_183.RWL0 != 1) || (RetVal_183.RWL1 != 1) ||
		(RetVal_183.RWL2 != 1) || (RetVal_183.RWL3 != 1))
	{
		status = -2;
		ADCS_HandleReport(status, ADCS_COMM_11_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}

	// 2) Verify sensor and wheel telemetry.
	OS_TaskDelay(2000);
	interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);
	OS_TaskDelay(100);
	interstatus += ADCS_Comm_GetRawGYRSensor(&RetVal_204);
	OS_TaskDelay(100);
	interstatus += ADCS_Comm_GetRawRWLSensor(&RetVal_205);

	for (cnt_try = 0; cnt_try < 3; ++cnt_try)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) &&
			(RetVal_204.GYR0ValidFlag == 1) && (RetVal_205.RWL0ValidFlag == 1) &&
			(RetVal_205.RWL1ValidFlag == 1) && (RetVal_205.RWL2ValidFlag == 1) &&
			(RetVal_205.RWL3ValidFlag == 1))
		{
			break;
		}

		OS_TaskDelay(500);
		interstatus = ADCS_Comm_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(100);
		interstatus += ADCS_Comm_GetRawGYRSensor(&RetVal_204);
		OS_TaskDelay(100);
		interstatus += ADCS_Comm_GetRawRWLSensor(&RetVal_205);
	}

	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1) ||
		(RetVal_204.GYR0ValidFlag != 1) || (RetVal_205.RWL0ValidFlag != 1) ||
		(RetVal_205.RWL1ValidFlag != 1) || (RetVal_205.RWL2ValidFlag != 1) ||
		(RetVal_205.RWL3ValidFlag != 1))
	{
		status = -3;
		ADCS_HandleReport(status, ADCS_COMM_11_CC, &RetVal_205, sizeof(RetVal_205));
		return status;
	}

	// 3) Clear the open-loop wheel momentum command.
	SetVal_076.cmdHx = 0.0;
	SetVal_076.cmdHy = 0.0;
	SetVal_076.cmdHz = 0.0;
	interstatus = ADCS_Comm_SetOpenLoopCmdHxyzRW(&SetVal_076);
	OS_TaskDelay(500);
	if (interstatus != CFE_SUCCESS)
	{
		status = -8;
		ADCS_HandleReport(status, ADCS_COMM_11_CC, &SetVal_076, sizeof(SetVal_076));
		return status;
	}

	// 4) Prepare control: mode 3 -> mode 51 -> mode 12.
	SetVal_042.MainEstimatorMode = estimator_mode;
	SetVal_042.BackupEstimatorMode = 5;
	SetVal_042.ControlMode = 3;
	SetVal_042.ControlTimeout = 0;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	OS_TaskDelay(1000);
	interstatus += ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != estimator_mode) ||
		(RetVal_150.ControlMode != 3))
	{
		status = -12;
		ADCS_HandleReport(status, ADCS_COMM_11_CC, &RetVal_150, sizeof(RetVal_150));
		return status;
	}

	SetVal_042.ControlMode = 51;
	SetVal_042.ControlTimeout = 125;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	OS_TaskDelay(1000);
	interstatus += ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != estimator_mode) ||
		(RetVal_150.ControlMode != 51))
	{
		status = -9;
		ADCS_HandleReport(status, ADCS_COMM_11_CC, &RetVal_150, sizeof(RetVal_150));
		SetVal_042.ControlMode = 3;
		SetVal_042.ControlTimeout = 0;
		(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		return status;
	}
	OS_TaskDelay(120000);

	SetVal_042.ControlMode = 12;
	SetVal_042.ControlTimeout = 305;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	OS_TaskDelay(1000);
	interstatus += ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != estimator_mode) ||
		(RetVal_150.ControlMode != 12))
	{
		status = -10;
		ADCS_HandleReport(status, ADCS_COMM_11_CC, &RetVal_150, sizeof(RetVal_150));
		SetVal_042.ControlMode = 3;
		SetVal_042.ControlTimeout = 0;
		(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		return status;
	}
	OS_TaskDelay(300000);

	// 5) Open the commissioning log before starting target control.
	fp = fopen("./cf/adcs_comm_11_tgt.bin", "wb");
	if (fp == NULL)
	{
		status = -20;
		SetVal_042.ControlMode = 3;
		SetVal_042.ControlTimeout = 0;
		(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		ADCS_HandleReport(status, ADCS_COMM_11_CC, NULL, 0);
		return status;
	}

	// 6) Start ConTgtTrack (mode 14).
	SetVal_042.ControlMode = ADCS_CONMODE_RW_EO_TARGET_TRACK;
	SetVal_042.ControlTimeout = 0;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	OS_TaskDelay(1000);
	interstatus += ADCS_Comm_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != estimator_mode) ||
		(RetVal_150.ControlMode != ADCS_CONMODE_RW_EO_TARGET_TRACK))
	{
		status = -21;
		fclose(fp);
		ADCS_HandleReport(status, ADCS_COMM_11_CC, &RetVal_150, sizeof(RetVal_150));
		SetVal_042.ControlMode = 3;
		SetVal_042.ControlTimeout = 0;
		(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		return status;
	}

	// 7) Log the estimator and raw/calibrated reaction-wheel telemetry at 1 Hz.
	telemetry.sync_word = 0xADC5u;
	for (sample_index = 0u; sample_index < target_duration; ++sample_index)
	{
		interstatus = ADCS_Comm_GetMainEstHighResTlm(&telemetry.MainEstHighRes);
		if (interstatus == CFE_SUCCESS)
		{
			OS_TaskDelay(10);
			interstatus = ADCS_Comm_GetRawRWLSensor(&telemetry.RawRWL);
		}
		if (interstatus == CFE_SUCCESS)
		{
			OS_TaskDelay(10);
			interstatus = ADCS_Comm_GetCalibratedRWLSensor(&telemetry.CalibratedRWL);
		}
		if (interstatus != CFE_SUCCESS)
		{
			fclose(fp);
			SetVal_042.ControlMode = 3;
			SetVal_042.ControlTimeout = 0;
			(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
			ADCS_HandleReport(interstatus, ADCS_COMM_11_CC, &telemetry, sizeof(telemetry));
			return interstatus;
		}

		if (fwrite(&telemetry, sizeof(telemetry), 1, fp) != 1u)
		{
			fclose(fp);
			status = -22;
			SetVal_042.ControlMode = 3;
			SetVal_042.ControlTimeout = 0;
			(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
			ADCS_HandleReport(status, ADCS_COMM_11_CC, NULL, 0);
			return status;
		}

		if ((sample_index + 1u) < target_duration)
		{
			OS_TaskDelay(1000);
		}
	}

	if (fclose(fp) != 0)
	{
		status = -23;
		SetVal_042.ControlMode = 3;
		SetVal_042.ControlTimeout = 0;
		(void)ADCS_Comm_SetControlEstimationMode(&SetVal_042);
		ADCS_HandleReport(status, ADCS_COMM_11_CC, NULL, 0);
		return status;
	}

	// 8) Return to mode 3 after commissioning.
	SetVal_042.ControlMode = 3;
	SetVal_042.ControlTimeout = 0;
	interstatus = ADCS_Comm_SetControlEstimationMode(&SetVal_042);
	if (interstatus != CFE_SUCCESS)
	{
		ADCS_HandleReport(interstatus, ADCS_COMM_11_CC, NULL, 0);
		return interstatus;
	}

	ADCS_ReportCommandPhase(ADCS_COMM_11_CC, ADCS_RPT_PHASE_COMPLETED);
	return CFE_SUCCESS;
}
