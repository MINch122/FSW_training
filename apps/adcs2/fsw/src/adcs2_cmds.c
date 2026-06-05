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
 *	This file contains the source code for the Adcs App Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "adcs2_app.h"
#include "adcs2_cmds.h"
#include "adcs2_msgids.h"
#include "adcs2_tbl.h"
#include "adcs2_utils.h"
#include "adcs2_msg.h"
#include "adcs2_eventids.h"
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*																			*/
/*  Purpose:																  */
/*		 This function is triggered in response to a task telemetry request */
/*		 from the housekeeping task. This function will gather the Apps	 */
/*		 telemetry, packetize it and send it to the housekeeping task via	*/
/*		 the software bus													*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*																			*/
CFE_Status_t ADCS2_SendHkCmd(const ADCS2_SendHkCmd_t *Msg)
{
    ADCS2_ControlEstimationMode_Cmn_Payload_t ControlMode = {0,};
    ADCS2_PowerState_Cmn_Payload_t PowerState = {0,};
    ADCS2_RawMAGSensorTlm_Paylaod_t RawMAG = {0,};
    ADCS2_RawCSSSensorTlm_Payload_t RawCSS = {0,};
    ADCS2_RawGYRSensorTlm_Payload_t RawGYR = {0,};
    ADCS2_RawRWLSensorTlm_Payload_t RawRWL = {0,};
    ADCS2_CalibratedCSSSensorTlm_Payload_t CalCSS = {0,};
    ADCS2_CalibratedGYRSensorTlm_Payload_t CalGYR = {0,};
    ADCS2_Estimator_Cmn_Payload_t MainEstimator = {0,};
    ADCS2_Estimator_Cmn_Payload_t BackupEstimator = {0,};
    ADCS2_ControllerTlm_Payload_t Controller = {0,};
    ADCS2_CurrentUnixTimeTlm_Payload_t CurrentUnixTime = {0,};

    memset(&ADCS2_AppData.HkTlm.Payload, 0, sizeof(ADCS2_AppData.HkTlm.Payload));

    if (ADCS2_GetCurrentUnixTime(&CurrentUnixTime) == CFE_SUCCESS) {
        ADCS2_AppData.HkTlm.Payload.CurrentUnixseconds = CurrentUnixTime.CurrentUnixseconds;
        ADCS2_AppData.HkTlm.Payload.CurrentUnixNanoseconds = CurrentUnixTime.CurrentUnixNanoseconds;
    }

    if (ADCS2_GetControlEstimationMode(&ControlMode) == CFE_SUCCESS) {
        ADCS2_AppData.HkTlm.Payload.ControlMode = ControlMode.ControlMode;
        ADCS2_AppData.HkTlm.Payload.MainEstimatorMode = ControlMode.MainEstimatorMode;
        ADCS2_AppData.HkTlm.Payload.BackupEstimatorMode = ControlMode.BackupEstimatorMode;
        ADCS2_AppData.HkTlm.Payload.ControlTimeout = ControlMode.ControlTimeout;
    }

    if (ADCS2_GetPowerState(&PowerState) == CFE_SUCCESS) {
        ADCS2_AppData.HkTlm.Payload.PowerState = ((PowerState.RWL0 & 1u) << 6) | ((PowerState.RWL1 & 1u) << 5) |
                                                ((PowerState.RWL2 & 1u) << 4) | ((PowerState.MAG0 & 1u) << 3) |
                                                ((PowerState.GYR0 & 1u) << 2) | ((PowerState.FSS0 & 1u) << 1) |
                                                (PowerState.HSS0 & 1u);
    }

    if (ADCS2_GetRawMAGSensor(&RawMAG) == CFE_SUCCESS) {
        ADCS2_AppData.HkTlm.Payload.MAG0RawVecX = RawMAG.MAG0RawVecX;
        ADCS2_AppData.HkTlm.Payload.MAG0RawVecY = RawMAG.MAG0RawVecY;
        ADCS2_AppData.HkTlm.Payload.MAG0RawVecZ = RawMAG.MAG0RawVecZ;
        ADCS2_AppData.HkTlm.Payload.MAG0ValidFlag = RawMAG.MAG0ValidFlag;
    }

    if (ADCS2_GetRawCSSSensor(&RawCSS) == CFE_SUCCESS) {
        ADCS2_AppData.HkTlm.Payload.CSS0 = RawCSS.CSS0;
        ADCS2_AppData.HkTlm.Payload.CSS1 = RawCSS.CSS1;
        ADCS2_AppData.HkTlm.Payload.CSS2 = RawCSS.CSS2;
        ADCS2_AppData.HkTlm.Payload.CSS3 = RawCSS.CSS3;
        ADCS2_AppData.HkTlm.Payload.CSS4 = RawCSS.CSS4;
        ADCS2_AppData.HkTlm.Payload.CSS5 = RawCSS.CSS5;
        ADCS2_AppData.HkTlm.Payload.CSSValidFlag = RawCSS.CSSValidFlag;
    }

    if (ADCS2_GetRawGYRSensor(&RawGYR) == CFE_SUCCESS) {
        ADCS2_AppData.HkTlm.Payload.GYR0RawRateX = RawGYR.GYR0RawRateX;
        ADCS2_AppData.HkTlm.Payload.GYR0RawRateY = RawGYR.GYR0RawRateY;
        ADCS2_AppData.HkTlm.Payload.GYR0RawRateZ = RawGYR.GYR0RawRateZ;
        ADCS2_AppData.HkTlm.Payload.GYR0ValidFlag = RawGYR.GYR0ValidFlag;
    }

    if (ADCS2_GetRawRWLSensor(&RawRWL) == CFE_SUCCESS) {
        ADCS2_AppData.HkTlm.Payload.RWL0MeasSpeed = RawRWL.RWL0MeasSpeed;
        ADCS2_AppData.HkTlm.Payload.RWL1MeasSpeed = RawRWL.RWL1MeasSpeed;
        ADCS2_AppData.HkTlm.Payload.RWL2MeasSpeed = RawRWL.RWL2MeasSpeed;
        ADCS2_AppData.HkTlm.Payload.RWL3MeasSpeed = RawRWL.RWL3MeasSpeed;
    }

    if (ADCS2_GetCalibratedCSSSensor(&CalCSS) == CFE_SUCCESS) {
        ADCS2_AppData.HkTlm.Payload.CSSCalUnitVecX = CalCSS.CSSCalUnitVecX;
        ADCS2_AppData.HkTlm.Payload.CSSCalUnitVecY = CalCSS.CSSCalUnitVecY;
        ADCS2_AppData.HkTlm.Payload.CSSCalUnitVecZ = CalCSS.CSSCalUnitVecZ;
        ADCS2_AppData.HkTlm.Payload.CalCSSValidFlag = CalCSS.CSSValidFlag;
    }

    if (ADCS2_GetCalibratedGYRSensor(&CalGYR) == CFE_SUCCESS) {
        ADCS2_AppData.HkTlm.Payload.GYR0CalibratedRateX = CalGYR.GYR0CalibratedRateX;
        ADCS2_AppData.HkTlm.Payload.GYR0CalibratedRateY = CalGYR.GYR0CalibratedRateY;
        ADCS2_AppData.HkTlm.Payload.GYR0CalibratedRateZ = CalGYR.GYR0CalibratedRateZ;
        ADCS2_AppData.HkTlm.Payload.CalGYR0ValidFlag = CalGYR.GYR0ValidFlag;
    }

    if (ADCS2_GetMainEstTlm(&MainEstimator) == CFE_SUCCESS) {
        ADCS2_AppData.HkTlm.Payload.MainEstRoll = MainEstimator.EstRoll;
        ADCS2_AppData.HkTlm.Payload.MainEstPitch = MainEstimator.EstPitch;
        ADCS2_AppData.HkTlm.Payload.MainEstYaw = MainEstimator.EstYaw;
        ADCS2_AppData.HkTlm.Payload.MainEstIRCBodyRateX = MainEstimator.EstIRCBodyRateX;
        ADCS2_AppData.HkTlm.Payload.MainEstIRCBodyRateY = MainEstimator.EstIRCBodyRateY;
        ADCS2_AppData.HkTlm.Payload.MainEstIRCBodyRateZ = MainEstimator.EstIRCBodyRateZ;
        ADCS2_AppData.HkTlm.Payload.MainActiveEstMode = MainEstimator.ActiveEstMode;
    }

    if (ADCS2_GetBackupEstTlm(&BackupEstimator) == CFE_SUCCESS) {
        ADCS2_AppData.HkTlm.Payload.BackupEstRoll = BackupEstimator.EstRoll;
        ADCS2_AppData.HkTlm.Payload.BackupEstPitch = BackupEstimator.EstPitch;
        ADCS2_AppData.HkTlm.Payload.BackupEstYaw = BackupEstimator.EstYaw;
        ADCS2_AppData.HkTlm.Payload.BackupActiveEstMode = BackupEstimator.ActiveEstMode;
    }

    if (ADCS2_GetControllerTlm(&Controller) == CFE_SUCCESS) {
        ADCS2_AppData.HkTlm.Payload.ControllerTimeout = Controller.ControlTimeout;
        ADCS2_AppData.HkTlm.Payload.ActiveContMode = Controller.ActiveContMode;
    }

    ADCS2_APP_printf("ADCS2: HK report requested\n");
    ADCS2_HandleReport(CFE_SUCCESS, 0, &ADCS2_AppData.HkTlm.Payload, sizeof(ADCS2_AppData.HkTlm.Payload));

    return CFE_SUCCESS;
}

CFE_Status_t ADCS2_SendBcnCmd(const ADCS2_SendBcnCmd_t *Msg)
{
    ADCS2_PowerState_Cmn_Payload_t PowerState = {0,};
    ADCS2_ControlEstimationMode_Cmn_Payload_t ControlMode = {0,};
    ADCS2_CalibratedGYRSensorTlm_Payload_t CalGYR = {0,};
    ADCS2_RawCSSSensorTlm_Payload_t RawCSS = {0,};

    memset(&ADCS2_AppData.BcnTlm.Payload, 0, sizeof(ADCS2_AppData.BcnTlm.Payload));

    if (ADCS2_GetPowerState(&PowerState) == CFE_SUCCESS) {
        ADCS2_AppData.BcnTlm.Payload.PowerState = ((PowerState.RWL0 & 1u) << 6) | ((PowerState.RWL1 & 1u) << 5) |
                                                 ((PowerState.RWL2 & 1u) << 4) | ((PowerState.MAG0 & 1u) << 3) |
                                                 ((PowerState.GYR0 & 1u) << 2) | ((PowerState.FSS0 & 1u) << 1) |
                                                 (PowerState.HSS0 & 1u);
    }

    if (ADCS2_GetControlEstimationMode(&ControlMode) == CFE_SUCCESS) {
        ADCS2_AppData.BcnTlm.Payload.ControlMode = ControlMode.ControlMode;
    }

    if (ADCS2_GetCalibratedGYRSensor(&CalGYR) == CFE_SUCCESS) {
        ADCS2_AppData.BcnTlm.Payload.GYR0CalibratedRateXComponent = CalGYR.GYR0CalibratedRateX;
        ADCS2_AppData.BcnTlm.Payload.GYR0CalibratedRateYComponent = CalGYR.GYR0CalibratedRateY;
        ADCS2_AppData.BcnTlm.Payload.GYR0CalibratedRateZComponent = CalGYR.GYR0CalibratedRateZ;
    }

    if (ADCS2_GetRawCSSSensor(&RawCSS) == CFE_SUCCESS) {
        ADCS2_AppData.BcnTlm.Payload.CSS[0] = RawCSS.CSS0;
        ADCS2_AppData.BcnTlm.Payload.CSS[1] = RawCSS.CSS1;
        ADCS2_AppData.BcnTlm.Payload.CSS[2] = RawCSS.CSS2;
        ADCS2_AppData.BcnTlm.Payload.CSS[3] = RawCSS.CSS3;
        ADCS2_AppData.BcnTlm.Payload.CSS[4] = RawCSS.CSS4;
        ADCS2_AppData.BcnTlm.Payload.CSS[5] = RawCSS.CSS5;
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(ADCS2_AppData.BcnTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(ADCS2_AppData.BcnTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

/* ADCS NOOP commands																			*/
/*																			*/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t ADCS2_NoopCmd(const ADCS2_NoopCmd_t *Msg)
{
	ADCS2_AppData.CmdCounter++;

	uint8_t Cmds[2] = {ADCS2_AppData.CmdCounter, ADCS2_AppData.ErrCounter};
	ADCS2_HandleReport(CFE_SUCCESS, ADCS2_NOOP_CC, Cmds, sizeof(Cmds));

	CFE_EVS_SendEvent(ADCS2_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "ADCS: NOOP command received.");

	return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*																			*/
/*  Purpose:																  */
/*		 This function resets all the global counter variables that are	 */
/*		 part of the task telemetry.										*/
/*																			*/
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t ADCS2_ResetCountersCmd(const ADCS2_ResetCountersCmd_t *Msg)
{
	ADCS2_AppData.CmdCounter = 0;
	ADCS2_AppData.ErrCounter = 0;

	uint8_t Cmds[2] = {ADCS2_AppData.CmdCounter, ADCS2_AppData.ErrCounter};
	ADCS2_HandleReport(CFE_SUCCESS, ADCS2_RESET_COUNTERS_CC, Cmds, sizeof(Cmds));

	CFE_EVS_SendEvent(ADCS2_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "ADCS: RESET command");

	return CFE_SUCCESS;
}

CFE_Status_t ADCS2_SetInterfaceTransportCmd(const ADCS2_InterfaceTransportCmd_t *Msg)
{
	CFE_Status_t Status;

	Status = ADCS2_SetInterfaceTransport(Msg->Payload.TransportType);

	ADCS2_HandleReport(Status, ADCS2_SET_INTERFACE_TRANSPORT_CC, (void *)&Msg->Payload, sizeof(Msg->Payload));

	if (Status != CFE_SUCCESS)
	{
		CFE_ES_WriteToSysLog("Adcs App: Fail to Set Interface Transport: 0x%08lx", (unsigned long)Status);
		return Status;
	}

	return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*																			*/
/* ADCS RELATED COMMANDS COME HERE											*/
/*																			*/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t ADCS2_SetReset(void){
	// This command has no reply
	CFE_Status_t				status;

	status = ADCS2_Reset();

	ADCS2_HandleReport(status, ADCS2_SET_RESET_CC, NULL, 0);

	if (status != CFE_SUCCESS)
	{
		CFE_ES_WriteToSysLog("Adcs App: Fail to Set Reset: 0x%08lx", (unsigned long)status);
		return status;
	}
	// OS_printf("Set reset success.\n");

	return CFE_SUCCESS;
}


/********************************************************
 * 
 * BEE1006 Actual invoked command function
 * Upper functions are just the references
 * 
 ********************************************************/

CFE_Status_t ADCS2_SetCurrentUnixTimeCmd(const ADCS2_CurrentUnixTimeCmd_t *msg) {
	// ID 2
	CFE_Status_t				status;

	status = ADCS2_SetCurrentUnixTime(&msg->Payload);

	ADCS2_HandleReport(status, ADCS2_SET_CURRENT_UNIX_TIME_CC, NULL, 0);

	if (status != CFE_SUCCESS)
	{
		CFE_ES_WriteToSysLog("Adcs App: Fail to Set Current Unix Time: 0x%08lx", (unsigned long)status);
		return status;
	}
	// OS_printf("ADCS cmd Success.");

	return CFE_SUCCESS;
}

CFE_Status_t ADCS2_SetPersistConfigCmd(const ADCS2_PersistConfigCmd_t *msg) {
    // ID 7
    CFE_Status_t               status;

    status = ADCS2_SetPersistConfig();

    ADCS2_HandleReport(status, ADCS2_SET_PERSIST_CONFIG_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Satellite Orbit Param Config: 0x%08lx", (unsigned long)status);
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t ADCS2_SetControlEstimationModeCmd(const ADCS2_ControlEstimationModeCmd_t *msg) {
    // ID 42
    CFE_Status_t               status;

    status = ADCS2_SetControlEstimationMode(&msg->Payload);

    ADCS2_HandleReport(status, ADCS2_SET_CONTROL_ESTIMATION_MODE_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Control Estimation Mode: 0x%08lx", (unsigned long)status);
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t ADCS2_SetOrbitModeCmd(const ADCS2_OrbitModeCmd_t *msg) {
    // ID 51
    CFE_Status_t               status;

    status = ADCS2_SetOrbitMode(&msg->Payload);

    ADCS2_HandleReport(status, ADCS2_SET_ORBIT_MODE_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Orbit Mode: 0x%08lx", (unsigned long)status);
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t ADCS2_SetPowerStateCmd(const ADCS2_PowerStateCmd_t *msg) {
    // ID 56
    CFE_Status_t               status;

    status = ADCS2_SetPowerState(&msg->Payload);

    ADCS2_HandleReport(status, ADCS2_SET_POWER_STATE_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Power State: 0x%08lx", (unsigned long)status);
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t ADCS2_SetMountingConfigCmd(const ADCS2_MountingConfigCmd_t *msg) {
    // ID 65
    CFE_Status_t               status;

    status = ADCS2_SetMountingConfig(&msg->Payload);

    ADCS2_HandleReport(status, ADCS2_SET_MOUNTING_CONFIG_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Mounting Config: 0x%08lx", (unsigned long)status);
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t ADCS2_SetEstimatorConfigCmd(const ADCS2_EstimatorConfigCmd_t *msg) {
    // ID 67
    CFE_Status_t               status;

    status = ADCS2_SetEstimatorConfig(&msg->Payload);

    ADCS2_HandleReport(status, ADCS2_SET_ESTIMATOR_CONFIG_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Estimator Config: 0x%08lx", (unsigned long)status);
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t ADCS2_SetSatOrbitParamConfigCmd(const ADCS2_SatOrbitParamConfigCmd_t *msg) {
    // ID 68
    CFE_Status_t               status;

    status = ADCS2_SetSatOrbitParamConfig(&msg->Payload);

    ADCS2_HandleReport(status, ADCS2_SET_SAT_ORBIT_PARAM_CONFIG_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Satellite Orbit Parameters Config: 0x%08lx", (unsigned long)status);
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t ADCS2_SetOpenLoopCmdHxyzRWCmd(const ADCS2_OpenLoopCmdHxyzRWCmd_t *msg) {
    // ID 76
    CFE_Status_t               status;

    status = ADCS2_SetOpenLoopCmdHxyzRW(&msg->Payload);

    ADCS2_HandleReport(status, ADCS2_SET_OPENLOOP_CMD_HXYZ_RW_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Open Loop Command Hxyz RW: 0x%08lx", (unsigned long)status);
        return status;
    }

    return CFE_SUCCESS;
}

/********************************************************
 * 
 * BEE1006 Actual Get Command Function (Get tlm)
 * 
 ********************************************************/

 CFE_Status_t ADCS2_GetCurrentUnixTimeCmd(void) {
	// ID 133
	CFE_Status_t				status;
	ADCS2_CurrentUnixTimeTlm_Payload_t RetVal = {0,};

	status = ADCS2_GetCurrentUnixTime(&RetVal);

	ADCS2_HandleReport(status, ADCS2_GET_CURRENT_UNIX_TIME_CC, &RetVal, sizeof(RetVal));

	if (status != CFE_SUCCESS)
	{
		CFE_ES_WriteToSysLog("Adcs App: Fail to Get Current Unix Time: 0x%08lx", (unsigned long)status);
		return status;
	}
	
	return CFE_SUCCESS;
}

CFE_Status_t ADCS2_GetControlEstimationModeCmd(void) {
    // ID 150
    CFE_Status_t               status;
    ADCS2_ControlEstimationMode_Cmn_Payload_t RetVal = {0,};

    status = ADCS2_GetControlEstimationMode(&RetVal);

    ADCS2_HandleReport(status, ADCS2_GET_CONTROL_ESTIMATION_MODE_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Control Estimation Mode: 0x%08lx", (unsigned long)status);
        return status;
    }
    
    return CFE_SUCCESS;
}

CFE_Status_t ADCS2_GetRawMAGSensorCmd(void) {
    // ID 180
    CFE_Status_t               status;
    ADCS2_RawMAGSensorTlm_Paylaod_t RetVal = {0,};

    status = ADCS2_GetRawMAGSensor(&RetVal);

    ADCS2_HandleReport(status, ADCS2_GET_RAW_MAG_SENSOR_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Control Estimation Mode: 0x%08lx", (unsigned long)status);
        return status;
    }
    
    return CFE_SUCCESS;
}

CFE_Status_t ADCS2_GetPowerStateCmd(void) {
    // ID 183
    CFE_Status_t               status;
    ADCS2_PowerState_Cmn_Payload_t RetVal = {0,};

    status = ADCS2_GetPowerState(&RetVal);

    ADCS2_HandleReport(status, ADCS2_GET_POWER_STATE_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Power State: 0x%08lx", (unsigned long)status);
        return status;
    }
    
    return CFE_SUCCESS;
}

CFE_Status_t ADCS2_GetMountingConfigCmd(void) {
    // ID 193
    CFE_Status_t               status;
    ADCS2_MountingConfig_Cmn_Payload_t RetVal = {0,};

    status = ADCS2_GetMountingConfig(&RetVal);

    ADCS2_HandleReport(status, ADCS2_GET_MOUNTING_CONFIG_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Calibrated GYR Sensor: 0x%08lx", (unsigned long)status);
        return status;
    }
    
    return CFE_SUCCESS;
}

CFE_Status_t ADCS2_GetEstimatorConfigCmd(void) {
    // ID 195
    CFE_Status_t               status;
    ADCS2_EstimatorConfig_Cmn_Payload_t RetVal = {0,};

    status = ADCS2_GetEstimatorConfig(&RetVal);

    ADCS2_HandleReport(status, ADCS2_GET_ESTIMATOR_CONFIG_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Estimator Config: 0x%08lx", (unsigned long)status);
        return status;
    }
    
    return CFE_SUCCESS;
}

CFE_Status_t ADCS2_GetSatOrbitParamConfigCmd(void) {
    // ID 196
    CFE_Status_t               status;
    ADCS2_SatOrbitParamConfig_Cmn_Payload_t RetVal = {0,};

    status = ADCS2_GetSatOrbitParamConfig(&RetVal);

    ADCS2_HandleReport(status, ADCS2_GET_SAT_ORBIT_PARAM_CONFIG_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Satellite Orbit Parameters Config: 0x%08lx", (unsigned long)status);
        return status;
    }
    
    return CFE_SUCCESS;
}

CFE_Status_t ADCS2_GetRawGYRSensorCmd(void) {
    // ID 204
    CFE_Status_t               status;
    ADCS2_RawGYRSensorTlm_Payload_t RetVal = {0,};

    status = ADCS2_GetRawGYRSensor(&RetVal);

    ADCS2_HandleReport(status, ADCS2_GET_RAW_GYR_SENSOR_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Raw GYR Sensor: 0x%08lx", (unsigned long)status);
        return status;
    }
    
    return CFE_SUCCESS;
}

CFE_Status_t ADCS2_GetRawRWLSensorCmd(void) {
    // ID 205
    CFE_Status_t status;
    ADCS2_RawRWLSensorTlm_Payload_t RetVal = {0,};

    status = ADCS2_GetRawRWLSensor(&RetVal);

    ADCS2_HandleReport(status, ADCS2_GET_RAW_RWL_SENSOR_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Raw RWL Sensor: 0x%08lx", (unsigned long)status);
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t ADCS2_GetMainEstimatorTlmCmd(void) {
    // ID 210
    CFE_Status_t               status;
    ADCS2_Estimator_Cmn_Payload_t RetVal = {0,};

    status = ADCS2_GetMainEstTlm(&RetVal);

    ADCS2_HandleReport(status, ADCS2_GET_MAIN_ESTIMATOR_TLM_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Main Estimator Telemetry: 0x%08lx", (unsigned long)status);
        return status;
    }
    
    return CFE_SUCCESS;
}



/********************************************************
 * 
 * BEE1006 Actual Commissioing Sequence Function
 * 
 ********************************************************/

CFE_Status_t ADCS2_Comm01Cmd(const ADCS2_Comm01Cmd_t *msg) {
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

	uint8 cnt_try = 0;
	uint8 flag_tlmtype = msg->Payload.flag_tlmtype;
	if ((flag_tlmtype != 0) && (flag_tlmtype != 1))
	{
		status = -1;		// Something is wrong!
		ADCS2_HandleReport(status, ADCS2_COMM_01_CC, &flag_tlmtype, sizeof(flag_tlmtype));
		return status;
	}
	
	// 1) Power On ( GYR0, MAG0 )
	ADCS2_PowerState_Cmn_Payload_t		SetVal_056 = {0,};
	ADCS2_PowerState_Cmn_Payload_t		RetVal_183 = {0,};
	SetVal_056.MAG0		= 1;
	SetVal_056.GYR0		= 1;
	interstatus = ADCS2_SetPowerState(&SetVal_056);		// Set GYR0 & MAG0 --> ON
	OS_TaskDelay(10);
	interstatus = ADCS2_GetPowerState(&RetVal_183);		// Get GYR0 & MAG0 power state
	OS_TaskDelay(5000);
	cnt_try = 0;
	while (cnt_try < 3)
	{		
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1)) break;	// Check Power On well

		// if not, try again until 3 times
		interstatus = ADCS2_SetPowerState(&SetVal_056);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetPowerState(&RetVal_183);
		cnt_try++;
	}

	interstatus = ADCS2_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -1;		// Something is wrong! ::::: Power State
		ADCS2_HandleReport(status, ADCS2_COMM_01_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}


	// 3) Check Status of Sensors
	ADCS2_RawMAGSensorTlm_Paylaod_t	RetVal_180 = {0,};
	ADCS2_RawGYRSensorTlm_Payload_t	RetVal_204 = {0,};

	OS_TaskDelay(1000);
	interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro

	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1)) break;	// Check Sensors Status Good

		// if not, wait changing status until 3 times
		OS_TaskDelay(100);
		interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);
		cnt_try++;
	}

	
	interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -3;		// Something is wrong! ::::: Magnetometer Status
		ADCS2_HandleReport(status, ADCS2_COMM_01_CC, &RetVal_180, sizeof(RetVal_180));
		return status;
	}
	OS_TaskDelay(10);
	interstatus = ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	if ((interstatus != CFE_SUCCESS) || (RetVal_204.GYR0ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -4;		// Something is wrong! ::::: Gyro Status
		ADCS2_HandleReport(status, ADCS2_COMM_01_CC, &RetVal_204, sizeof(RetVal_204));
		return status;
	}


	// 2) Set Est. mode ( Main: EstGyro (1) / Backup: EstMagRkf (2) )
	ADCS2_ControlEstimationMode_Cmn_Payload_t	SetVal_042 = {0,};
	ADCS2_ControlEstimationMode_Cmn_Payload_t	RetVal_150 = {0,};

	SetVal_042.MainEstimatorMode 	= 1;
	SetVal_042.BackupEstimatorMode 	= 2;
	interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);	// Set Main & Backup Est mode = 1 & 2
	OS_TaskDelay(100);
	interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);	// Get Main & Backup Est mode
	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_150.MainEstimatorMode == 1) && (RetVal_150.BackupEstimatorMode == 2)) break;	// Check Mode Change well

		// if not, try again until 3 times
		interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);
		cnt_try++;
	}

	interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != 1) || (RetVal_150.BackupEstimatorMode != 2))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -2;		// Something is wrong! ::::: Estimation Mode
		ADCS2_HandleReport(status, ADCS2_COMM_01_CC, &RetVal_150, sizeof(RetVal_150));
		return status;

	}

	// 4) Make "adcs_comm_01.txt" file to log the TLMs
	FILE *fp;
	size_t written;
	fp = fopen("./cf/adcs_comm_01.bin","wb");
	if (fp == NULL)
	{
		status = -5;		// Something is wrong! ::::: File open error
		ADCS2_HandleReport(status, ADCS2_COMM_01_CC, NULL, 0);
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
	ADCS2_Estimator_Cmn_Payload_t	RetVal_210 = {0,};
	ADCS2_Estimator_Cmn_Payload_t	RetVal_173 = {0,};

	ADCS2_COMM_01_COMP_Payload_t	Comm_01_Tlm_Set_Comp = {0,};
	ADCS2_COMM_01_FULL_Payload_t	Comm_01_Tlm_Set_Full = {0,};

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
		interstatus = ADCS2_GetMainEstTlm(&RetVal_210);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetBackupEstTlm(&RetVal_173);
		OS_TaskDelay(10);

		// 		6-2) Read Sensor Raw Values - Gyro & Magnetometer	
		interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetRawGYRSensor(&RetVal_204);
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
				ADCS2_HandleReport(status, ADCS2_COMM_01_CC, NULL, 0);
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
				ADCS2_HandleReport(status, ADCS2_COMM_01_CC, NULL, 0);
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
	fclose(fp);

	ADCS2_HandleReport(CFE_SUCCESS, ADCS2_COMM_01_CC, NULL, 0);

	return CFE_SUCCESS;
}


CFE_Status_t ADCS2_Comm02Cmd(const ADCS2_Comm02Cmd_t *msg) {
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

	uint8 cnt_try = 0;
	uint8 flag_tlmtype = msg->Payload.flag_tlmtype;
	uint8 flag_contmode = msg->Payload.flag_contmode;

	if ((flag_contmode != 0) && (flag_contmode != 1) && (flag_contmode != 3) && (flag_contmode != 4)) 
	{
		status = -2;
		ADCS2_HandleReport(status, ADCS2_COMM_02_CC, &flag_contmode, sizeof(flag_contmode));
		return status;
	}
	
	if ((flag_tlmtype != 0) && (flag_tlmtype != 1))
	{
		status = -1;		// Something is wrong!
		ADCS2_HandleReport(status, ADCS2_COMM_02_CC, &flag_tlmtype, sizeof(flag_tlmtype));
		return status;
	}

	// 1) Power On ( GYR0, MAG0 )
	ADCS2_PowerState_Cmn_Payload_t		SetVal_056 = {0,};
	ADCS2_PowerState_Cmn_Payload_t		RetVal_183 = {0,};
	SetVal_056.MAG0		= 1;
	SetVal_056.GYR0		= 1;
	interstatus = ADCS2_SetPowerState(&SetVal_056);		// Set GYR0 & MAG0 --> ON
	OS_TaskDelay(10);
	interstatus = ADCS2_GetPowerState(&RetVal_183);		// Get GYR0 & MAG0 power state
	OS_TaskDelay(5000);
	cnt_try = 0;
	while (cnt_try < 3)
	{		
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1)) break;	// Check Power On well

		// if not, try again until 3 times
		interstatus = ADCS2_SetPowerState(&SetVal_056);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetPowerState(&RetVal_183);
		cnt_try++;
	}

	interstatus = ADCS2_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -1;		// Something is wrong! ::::: Power State
		ADCS2_HandleReport(status, ADCS2_COMM_02_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}


	// 3) Check Status of Sensors
	ADCS2_RawMAGSensorTlm_Paylaod_t	RetVal_180 = {0,};
	ADCS2_RawGYRSensorTlm_Payload_t	RetVal_204 = {0,};
	OS_TaskDelay(1000);
	interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro

	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1)) break;	// Check Sensors Status Good

		// if not, wait changing status until 3 times
		OS_TaskDelay(100);
		interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);
		cnt_try++;
	}

	
	interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -3;		// Something is wrong! ::::: Magnetometer Status
		ADCS2_HandleReport(status, ADCS2_COMM_02_CC, &RetVal_180, sizeof(RetVal_180));
		return status;
	}
	OS_TaskDelay(10);
	interstatus = ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	if ((interstatus != CFE_SUCCESS) || (RetVal_204.GYR0ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -4;		// Something is wrong! ::::: Gyro Status
		ADCS2_HandleReport(status, ADCS2_COMM_02_CC, &RetVal_204, sizeof(RetVal_204));
		return status;
	}

	// 2) Set Est. mode & Cont. mode ( Main: EstGyro (1) / Backup: EstMagRkf (2) / Control: ConBdot or ConBdot3 / Timeout: 600 s )
	ADCS2_ControlEstimationMode_Cmn_Payload_t	SetVal_042 = {0,};
	ADCS2_ControlEstimationMode_Cmn_Payload_t	RetVal_150 = {0,};

	SetVal_042.MainEstimatorMode 	= 1;
	SetVal_042.BackupEstimatorMode 	= 2;
	SetVal_042.ControlMode 			= flag_contmode;
	SetVal_042.ControlTimeout		= 600;
	interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);	// Set Main & Backup Est mode = 1 & 2 / Control mode & Time out = 0 or 1 or 3 or 4 & 600 s
	OS_TaskDelay(100);
	interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);	// Get Main & Backup Est mode
	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_150.MainEstimatorMode == 1) && (RetVal_150.ControlMode == flag_contmode)) break;	// Check Mode Change well

		// if not, try again until 3 times
		interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);
		cnt_try++;
	}

	interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != 1) || (RetVal_150.ControlMode != flag_contmode))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -2;		// Something is wrong! ::::: Estimation Mode
		ADCS2_HandleReport(status, ADCS2_COMM_02_CC, &RetVal_150, sizeof(RetVal_150));
		return status;

	}


	// 4) Make "adcs_comm_02.txt" file to log the TLMs
	FILE *fp;
	size_t written;
	fp = fopen("./cf/adcs_comm_02.bin","wb");
	if (fp == NULL)
	{
		status = -5;		// Something is wrong! ::::: File open error
		ADCS2_HandleReport(status, ADCS2_COMM_02_CC, NULL, 0);
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
	ADCS2_Estimator_Cmn_Payload_t		RetVal_210 = {0,};
	ADCS2_RawCSSSensorTlm_Payload_t		RetVal_203 = {0,};
	ADCS2_ControllerTlm_Payload_t		RetVal_172 = {0,};

	ADCS2_COMM_02_COMP_Payload_t	Comm_02_Tlm_Set_Comp = {0,};
	ADCS2_COMM_02_FULL_Payload_t	Comm_02_Tlm_Set_Full = {0,};

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
		interstatus = ADCS2_GetMainEstTlm(&RetVal_210);
		OS_TaskDelay(10);

		// 		6-2) Read Sensor Raw Values - Gyro & Magnetometer	
		interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetRawGYRSensor(&RetVal_204);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetRawCSSSensor(&RetVal_203);
		OS_TaskDelay(10);

		//		6-3) Read Tlm of Controller
		interstatus = ADCS2_GetControllerTlm(&RetVal_172);
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
				ADCS2_HandleReport(status, ADCS2_COMM_02_CC, NULL, 0);
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
				ADCS2_HandleReport(status, ADCS2_COMM_02_CC, NULL, 0);
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
	fclose(fp);

	ADCS2_HandleReport(CFE_SUCCESS, ADCS2_COMM_02_CC, NULL, 0);

	return CFE_SUCCESS;
}

CFE_Status_t ADCS2_Comm03Cmd(const ADCS2_Comm03Cmd_t *msg) {
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

	uint8 cnt_try = 0;
	uint8 flag_tlmtype = msg->Payload.flag_tlmtype;
	if ((flag_tlmtype != 0) && (flag_tlmtype != 1))
	{
		status = -1;		// Something is wrong!
		ADCS2_HandleReport(status, ADCS2_COMM_03_CC, &flag_tlmtype, sizeof(flag_tlmtype));
		return status;
	}
	
	// 1) Power On ( GYR0, MAG0 )
	ADCS2_PowerState_Cmn_Payload_t		SetVal_056 = {0,};
	ADCS2_PowerState_Cmn_Payload_t		RetVal_183 = {0,};
	SetVal_056.MAG0		= 1;
	SetVal_056.GYR0		= 1;
	interstatus = ADCS2_SetPowerState(&SetVal_056);		// Set GYR0 & MAG0 --> ON
	OS_TaskDelay(10);
	interstatus = ADCS2_GetPowerState(&RetVal_183);		// Get GYR0 & MAG0 power state
	OS_TaskDelay(5000);
	cnt_try = 0;
	while (cnt_try < 3)
	{		
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1)) break;	// Check Power On well

		// if not, try again until 3 times
		interstatus = ADCS2_SetPowerState(&SetVal_056);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetPowerState(&RetVal_183);
		cnt_try++;
	}

	interstatus = ADCS2_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -1;		// Something is wrong! ::::: Power State
		ADCS2_HandleReport(status, ADCS2_COMM_03_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}


	// 3) Check Status of Sensors
	ADCS2_RawMAGSensorTlm_Paylaod_t	RetVal_180 = {0,};
	ADCS2_RawGYRSensorTlm_Payload_t	RetVal_204 = {0,};
	OS_TaskDelay(1000);
	interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro

	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1)) break;	// Check Sensors Status Good

		// if not, wait changing status until 3 times
		OS_TaskDelay(100);
		interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);
		cnt_try++;
	}

	
	interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -3;		// Something is wrong! ::::: Magnetometer Status
		ADCS2_HandleReport(status, ADCS2_COMM_03_CC, &RetVal_180, sizeof(RetVal_180));
		return status;
	}
	OS_TaskDelay(10);
	interstatus = ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	if ((interstatus != CFE_SUCCESS) || (RetVal_204.GYR0ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -4;		// Something is wrong! ::::: Gyro Status
		ADCS2_HandleReport(status, ADCS2_COMM_03_CC, &RetVal_204, sizeof(RetVal_204));
		return status;
	}

	// 2) Set Est. mode & Cont. mode ( Main: EstGyro (1) / Control: ConBdot3 / Timeout: 6000 s )
	ADCS2_ControlEstimationMode_Cmn_Payload_t	SetVal_042 = {0,};
	ADCS2_ControlEstimationMode_Cmn_Payload_t	RetVal_150 = {0,};

	SetVal_042.MainEstimatorMode 	= 1;
	SetVal_042.BackupEstimatorMode 	= 2;
	SetVal_042.ControlMode 			= 3;
	SetVal_042.ControlTimeout		= 6000;
	interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);	// Set Main Est mode = 1 / Control mode & Time out = 3 & 6000 s
	OS_TaskDelay(100);
	interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);	// Get Main Est mode
	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_150.MainEstimatorMode == 1) && (RetVal_150.ControlMode == 3)) break;	// Check Mode Change well

		// if not, try again until 3 times
		interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);
		cnt_try++;
	}

	interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != 1) || (RetVal_150.ControlMode != 3))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -2;		// Something is wrong! ::::: Estimation Mode
		ADCS2_HandleReport(status, ADCS2_COMM_03_CC, &RetVal_150, sizeof(RetVal_150));
		return status;

	}


	// 4) Make "adcs_comm_03.txt" file to log the TLMs
	FILE *fp;
	size_t written;
	fp = fopen("./cf/adcs_comm_03.bin","wb");
	if (fp == NULL)
	{
		status = -5;		// Something is wrong! ::::: File open error
		ADCS2_HandleReport(status, ADCS2_COMM_03_CC, NULL, 0);
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
	ADCS2_Estimator_Cmn_Payload_t		RetVal_210 = {0,};
	ADCS2_CalibratedMAGSensorTlm_Payload_t		RetVal_177 = {0,};

	ADCS2_COMM_03_COMP_Payload_t	Comm_03_Tlm_Set_Comp = {0,};
	ADCS2_COMM_03_FULL_Payload_t	Comm_03_Tlm_Set_Full = {0,};

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
		interstatus = ADCS2_GetMainEstTlm(&RetVal_210);
		OS_TaskDelay(10);

		// 		6-2) Read Sensor Calibrated Values - Magnetometer
		interstatus = ADCS2_GetCalibratedMAGSensor(&RetVal_177);
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
				ADCS2_HandleReport(status, ADCS2_COMM_03_CC, NULL, 0);
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
				ADCS2_HandleReport(status, ADCS2_COMM_03_CC, NULL, 0);
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
	fclose(fp);

	ADCS2_HandleReport(CFE_SUCCESS, ADCS2_COMM_03_CC, NULL, 0);

	return CFE_SUCCESS;
}

CFE_Status_t ADCS2_Comm04Cmd(const ADCS2_Comm04Cmd_t *msg) {
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

	uint8 cnt_try = 0;
	uint8 flag_tlmtype = msg->Payload.flag_tlmtype;
	if ((flag_tlmtype != 0) && (flag_tlmtype != 1))
	{
		status = -1;		// Something is wrong!
		ADCS2_HandleReport(status, ADCS2_COMM_04_CC, &flag_tlmtype, sizeof(flag_tlmtype));
		return status;
	}
	
	// 1) Power On ( GYR0, MAG0 )
	ADCS2_PowerState_Cmn_Payload_t		SetVal_056 = {0,};
	ADCS2_PowerState_Cmn_Payload_t		RetVal_183 = {0,};
	SetVal_056.MAG0		= 1;
	SetVal_056.GYR0		= 1;
	interstatus = ADCS2_SetPowerState(&SetVal_056);		// Set GYR0 & MAG0 --> ON
	OS_TaskDelay(10);
	interstatus = ADCS2_GetPowerState(&RetVal_183);		// Get GYR0 & MAG0 power state
	OS_TaskDelay(5000);
	cnt_try = 0;
	while (cnt_try < 3)
	{		
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1)) break;	// Check Power On well

		// if not, try again until 3 times
		interstatus = ADCS2_SetPowerState(&SetVal_056);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetPowerState(&RetVal_183);
		cnt_try++;
	}

	interstatus = ADCS2_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -1;		// Something is wrong! ::::: Power State
		ADCS2_HandleReport(status, ADCS2_COMM_04_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}


	// 2) Check Status of Sensors
	ADCS2_RawMAGSensorTlm_Paylaod_t	RetVal_180 = {0,};

	OS_TaskDelay(2000);
	interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);

	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1)) break;	// Check Sensors Status Good

		// if not, wait changing status until 3 times
		OS_TaskDelay(100);
		interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(10);
		cnt_try++;
	}

	
	interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -3;		// Something is wrong! ::::: Magnetometer Status
		ADCS2_HandleReport(status, ADCS2_COMM_04_CC, &RetVal_180, sizeof(RetVal_180));
		return status;
	}
	OS_TaskDelay(10);

	// 3) Set Est. mode & Cont. mode ( Main: EstGyroEkf  (6) / Main: EstFullEkf  (5) / Control: ConBdot3 / Timeout: 6000 s )
	ADCS2_ControlEstimationMode_Cmn_Payload_t	SetVal_042 = {0,};
	ADCS2_ControlEstimationMode_Cmn_Payload_t	RetVal_150 = {0,};

	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 5;
	SetVal_042.ControlMode 			= 3;
	SetVal_042.ControlTimeout		= 6000;
	interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);	// Set Main Est mode = 6 / Set Backup Est mode = 5 / Control mode & Time out = 3 & 6000 s
	OS_TaskDelay(100);
	interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);	// Get Main Est mode
	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_150.MainEstimatorMode == 6) && (RetVal_150.ControlMode == 3)) break;	// Check Mode Change well

		// if not, try again until 3 times
		interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);
		cnt_try++;
	}

	interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != 6) || (RetVal_150.ControlMode != 3))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -2;		// Something is wrong! ::::: Estimation Mode
		ADCS2_HandleReport(status, ADCS2_COMM_04_CC, &RetVal_150, sizeof(RetVal_150));
		return status;

	}

	// 4) Make "adcs_comm_04.bin" file to log the TLMs
	FILE *fp;
	size_t written;
	fp = fopen("./cf/adcs_comm_04.bin","wb");
	if (fp == NULL)
	{
		status = -5;		// Something is wrong! ::::: File open error
		ADCS2_HandleReport(status, ADCS2_COMM_04_CC, NULL, 0);
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
	ADCS2_Estimator_Cmn_Payload_t	RetVal_210 = {0,};
	ADCS2_Estimator_Cmn_Payload_t	RetVal_173 = {0,};

	ADCS2_RawCSSSensorTlm_Payload_t 	RetVal_203 = {0,};
	ADCS2_CalibratedCSSSensorTlm_Payload_t 	RetVal_206 = {0,};
	ADCS2_ModelsTlm_Payload_t 			RetVal_174 = {0,};

	ADCS2_COMM_04_COMP_Payload_t	Comm_04_Tlm_Set_Comp = {0,};
	ADCS2_COMM_04_FULL_Payload_t	Comm_04_Tlm_Set_Full = {0,};

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
		interstatus = ADCS2_GetMainEstTlm(&RetVal_210);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetBackupEstTlm(&RetVal_173);
		OS_TaskDelay(10);

		// 		6-2) Read Sensor Raw Values - CSS	
		interstatus = ADCS2_GetRawCSSSensor(&RetVal_203);
		OS_TaskDelay(10);

		// 		6-3) Read Sensor Calibrated Values - CSS
		interstatus = ADCS2_GetCalibratedCSSSensor(&RetVal_206);
		OS_TaskDelay(10);

		// 		6-4) Read Models Telemetery Values
		interstatus = ADCS2_GetModelsTlm(&RetVal_174);
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
				ADCS2_HandleReport(status, ADCS2_COMM_04_CC, NULL, 0);
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
				ADCS2_HandleReport(status, ADCS2_COMM_04_CC, NULL, 0);
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
	fclose(fp);

	ADCS2_HandleReport(CFE_SUCCESS, ADCS2_COMM_04_CC, NULL, 0);

	return CFE_SUCCESS;
}

CFE_Status_t ADCS2_Comm05Cmd(const ADCS2_Comm05Cmd_t *msg) {
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

	uint8 cnt_try = 0;
	// uint8 flag_tlmtype = msg->Payload.flag_tlmtype;
	// if ((flag_tlmtype != 0) && (flag_tlmtype != 1))
	// {
	// 	status = -1;		// Something is wrong!
	// 	ADCS2_HandleReport(status, ADCS2_COMM_05_CC, &flag_tlmtype, sizeof(flag_tlmtype));
	// 	return status;
	// }
	
	// 1) Power On ( GYR0, MAG0, FSS0 )
	ADCS2_PowerState_Cmn_Payload_t		SetVal_056 = {0,};
	ADCS2_PowerState_Cmn_Payload_t		RetVal_183 = {0,};
	SetVal_056.MAG0		= 1;
	SetVal_056.GYR0		= 1;
	SetVal_056.FSS0		= 1;
	interstatus = ADCS2_SetPowerState(&SetVal_056);		// Set GYR0 & MAG0 & FSS0 --> ON
	OS_TaskDelay(10);
	interstatus = ADCS2_GetPowerState(&RetVal_183);		// Get GYR0 & MAG0 & FSS0 power state
	OS_TaskDelay(5000);
	cnt_try = 0;
	while (cnt_try < 3)
	{		
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1) && (RetVal_183.FSS0 == 1)) break;	// Check Power On well

		// if not, try again until 3 times
		interstatus = ADCS2_SetPowerState(&SetVal_056);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetPowerState(&RetVal_183);
		cnt_try++;
	}

	interstatus = ADCS2_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1) || (RetVal_183.FSS0 != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -1;		// Something is wrong! ::::: Power State
		ADCS2_HandleReport(status, ADCS2_COMM_05_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}


	// 2) Check Status of Sensors
	ADCS2_RawMAGSensorTlm_Paylaod_t	RetVal_180 = {0,};
	ADCS2_RawGYRSensorTlm_Payload_t	RetVal_204 = {0,};
	// ADCS2_RawCubeSenseSunTlm_Payload_t RetVal_170 = {0,};		// "FSS0 valid" is not identified yet. After idetification, use this status to check the state of FSS0. -> (line 1578, 1584)

	OS_TaskDelay(2000);
	interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	// OS_TaskDelay(10);
	// interstatus = interstatus + ADCS2_GetRawCubeSenseSun(&RetVal_170);	// Get Status of FSS

	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1)) break;	// Check Sensors Status Good
		// if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1) && (RetVal_170.ValidResult0 == 1)) break;	// Check Sensors Status Good

		// if not, wait changing status until 3 times
		OS_TaskDelay(100);
		interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
		cnt_try++;
	}

	
	interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1) || (RetVal_204.GYR0ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -3;		// Something is wrong! ::::: Magnetometer Status
		ADCS2_HandleReport(status, ADCS2_COMM_05_CC, &RetVal_180, sizeof(RetVal_180));
		return status;
	}
	OS_TaskDelay(10);

	// 3) Set Est. mode & Cont. mode ( Main: EstGyroEkf  (6) / Main: EstFullEkf  (5) / Control: ConBdot3 / Timeout: 6000 s )
	ADCS2_ControlEstimationMode_Cmn_Payload_t	SetVal_042 = {0,};
	ADCS2_ControlEstimationMode_Cmn_Payload_t	RetVal_150 = {0,};

	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 5;
	SetVal_042.ControlMode 			= 3;
	SetVal_042.ControlTimeout		= 6000;
	interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);	// Set Main Est mode = 6 / Set Backup Est mode = 5 / Control mode & Time out = 3 & 6000 s
	OS_TaskDelay(100);
	interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);	// Get Main Est mode
	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_150.MainEstimatorMode == 6) && (RetVal_150.ControlMode == 3)) break;	// Check Mode Change well

		// if not, try again until 3 times
		interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);
		cnt_try++;
	}

	interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != 6) || (RetVal_150.ControlMode != 3))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -2;		// Something is wrong! ::::: Estimation Mode
		ADCS2_HandleReport(status, ADCS2_COMM_05_CC, &RetVal_150, sizeof(RetVal_150));
		return status;

	}

	// 4) Make "adcs_comm_05.bin" file to log the TLMs
	FILE *fp;
	size_t written;
	fp = fopen("./cf/adcs_comm_05.bin","wb");
	if (fp == NULL)
	{
		status = -5;		// Something is wrong! ::::: File open error
		ADCS2_HandleReport(status, ADCS2_COMM_05_CC, NULL, 0);
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
	ADCS2_Estimator_Cmn_Payload_t	RetVal_210 = {0,};

	ADCS2_RawCSSSensorTlm_Payload_t 	RetVal_203 = {0,};
	ADCS2_CalibratedCSSSensorTlm_Payload_t 	RetVal_206 = {0,};
	ADCS2_RawCubeSenseSunTlm_Payload_t 		RetVal_170 = {0,};
	ADCS2_CalibratedFSSSensorTlm_Payload_t	RetVal_178 = {0,};

	ADCS2_COMM_05_Payload_t	Comm_05_Tlm_Set = {0,};

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
		interstatus = ADCS2_GetMainEstTlm(&RetVal_210);
		OS_TaskDelay(20);

		// 		6-2) Read Sensor Raw Values - CSS	
		interstatus = ADCS2_GetRawCSSSensor(&RetVal_203);
		OS_TaskDelay(20);

		// 		6-3) Read Sensor Calibrated Values - CSS
		interstatus = ADCS2_GetCalibratedCSSSensor(&RetVal_206);
		OS_TaskDelay(20);

		// 		6-4) Read Sensor Raw Values - FSS
		interstatus = ADCS2_GetRawCubeSenseSun(&RetVal_170);
		OS_TaskDelay(20);

		// 		6-5) Read Sensor Calibrated Values - FSS
		interstatus = ADCS2_GetCalibratedFSSSensor(&RetVal_178);
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
			ADCS2_HandleReport(status, ADCS2_COMM_05_CC, NULL, 0);
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
	fclose(fp);

	ADCS2_HandleReport(CFE_SUCCESS, ADCS2_COMM_05_CC, NULL, 0);

	return CFE_SUCCESS;
}

CFE_Status_t ADCS2_Comm06Cmd(const ADCS2_Comm06Cmd_t *msg) {
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
		ADCS2_HandleReport(status, ADCS2_COMM_06_CC, &flag_contmode, sizeof(flag_contmode));
		return status;
	}
	if ((flag_estmode != 0) && (flag_estmode != 1) && (flag_estmode != 2))
	{
		status = -8;		// Something is wrong!
		ADCS2_HandleReport(status, ADCS2_COMM_06_CC, &flag_estmode, sizeof(flag_estmode));
		return status;
	}
	
	// 1) Power On ( GYR0, MAG0, FSS0, RWLX )
	ADCS2_PowerState_Cmn_Payload_t		SetVal_056 = {0,};
	ADCS2_PowerState_Cmn_Payload_t		RetVal_183 = {0,};
	SetVal_056.MAG0		= 1;
	SetVal_056.GYR0		= 1;
	SetVal_056.FSS0		= 1;
	SetVal_056.RWL0		= 1;
	SetVal_056.RWL1		= 1;
	SetVal_056.RWL2		= 1;
	SetVal_056.RWL3		= 1;

	interstatus = ADCS2_SetPowerState(&SetVal_056);		// Set GYR0 & MAG0 & FSS0 & RWLX --> ON
	OS_TaskDelay(10);
	interstatus = ADCS2_GetPowerState(&RetVal_183);		// Get GYR0 & MAG0 & FSS0 & RWLX power state
	OS_TaskDelay(5000);
	cnt_try = 0;
	while (cnt_try < 3)
	{		
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1) && (RetVal_183.FSS0 == 1) && (RetVal_183.RWL0 == 1) && (RetVal_183.RWL1 == 1) && (RetVal_183.RWL2 == 1) && (RetVal_183.RWL3 == 1)) break;	// Check Power On well

		// if not, try again until 3 times
		interstatus = ADCS2_SetPowerState(&SetVal_056);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetPowerState(&RetVal_183);
		cnt_try++;
	}

	interstatus = ADCS2_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1) || (RetVal_183.FSS0 != 1) || (RetVal_183.RWL0 != 1) || (RetVal_183.RWL1 != 1) || (RetVal_183.RWL2 != 1) || (RetVal_183.RWL3 != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -1;		// Something is wrong! ::::: Power State
		ADCS2_HandleReport(status, ADCS2_COMM_06_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}


	// 2) Check Status of Sensors
	ADCS2_RawMAGSensorTlm_Paylaod_t	RetVal_180 = {0,};
	ADCS2_RawGYRSensorTlm_Payload_t	RetVal_204 = {0,};
	// ADCS2_RawCubeSenseSunTlm_Payload_t RetVal_170 = {0,};		// "FSS0 valid" is not identified yet. After idetification, use this status to check the state of FSS0. -> (line 1578, 1584)
	ADCS2_RawRWLSensorTlm_Payload_t RetVal_205 = {0,};

	OS_TaskDelay(2000);
	interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	// OS_TaskDelay(10);
	// interstatus = interstatus + ADCS2_GetRawCubeSenseSun(&RetVal_170);	// Get Status of FSS
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels
	

	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1) && (RetVal_205.RWL0ValidFlag == 1) && (RetVal_205.RWL1ValidFlag == 1) && (RetVal_205.RWL2ValidFlag == 1) && (RetVal_205.RWL3ValidFlag == 1)) break;	// Check Sensors Status Good
		// if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1) && (RetVal_170.ValidResult0 == 1)) break;	// Check Sensors Status Good

		// if not, wait changing status until 3 times
		OS_TaskDelay(100);
		interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS2_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels
		cnt_try++;
	}

	
	interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels

	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1) || (RetVal_204.GYR0ValidFlag != 1) || (RetVal_205.RWL0ValidFlag != 1) || (RetVal_205.RWL1ValidFlag != 1) || (RetVal_205.RWL2ValidFlag != 1) || (RetVal_205.RWL3ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -3;		// Something is wrong! ::::: Magnetometer Status
		ADCS2_HandleReport(status, ADCS2_COMM_06_CC, &RetVal_180, sizeof(RetVal_180));
		return status;
	}
	OS_TaskDelay(10);

	// 3) Set Est. mode & Cont. mode ( Main: EstGyroEkf  (6) / Main: EstGyro  (1) / Control: ConHxyzRW / Timeout: 120+60 s )
	ADCS2_ControlEstimationMode_Cmn_Payload_t	SetVal_042 = {0,};
	ADCS2_ControlEstimationMode_Cmn_Payload_t	RetVal_150 = {0,};

	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 1;
	// SetVal_042.ControlMode 			= 51;
	// SetVal_042.ControlTimeout		= 120;
	SetVal_042.ControlMode 			= 3;
	SetVal_042.ControlTimeout		= 0;
	interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);	// Set Main Est mode = 6 / Set Backup Est mode = 1 / Control mode & Time out = 3 & 0 s
	OS_TaskDelay(100);
	interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);	// Get Main Est mode
	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_150.MainEstimatorMode == 6) && (RetVal_150.ControlMode == 3)) break;	// Check Mode Change well

		// if not, try again until 3 times
		interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);
		cnt_try++;
	}

	interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != 6) || (RetVal_150.ControlMode != 3))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -2;		// Something is wrong! ::::: Estimation Mode
		ADCS2_HandleReport(status, ADCS2_COMM_06_CC, &RetVal_150, sizeof(RetVal_150));
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
		ADCS2_HandleReport(status, ADCS2_COMM_06_CC, NULL, 0);

		SetVal_042.MainEstimatorMode 	= 6;
		SetVal_042.BackupEstimatorMode 	= 5;
		SetVal_042.ControlMode 			= 3;
		SetVal_042.ControlTimeout		= 0;
		interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);

		SetVal_056.RWL0		= 0;
		SetVal_056.RWL1		= 0;
		SetVal_056.RWL2		= 0;
		SetVal_056.RWL3		= 0;
		interstatus = ADCS2_SetPowerState(&SetVal_056);		// Set RWLX --> OFF
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
	ADCS2_OpenLoopCmdHxyzRWCmd_Payload_t SetVal_076 = {0,};
	
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
	ADCS2_Estimator_Cmn_Payload_t	RetVal_210 = {0,};
	ADCS2_CalibratedRWLSensorTlm_Payload_t 	RetVal_209 = {0,};

	ADCS2_COMM_06_Payload_t	Comm_06_Tlm_Set = {0,};

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
			interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);
		}
		else if ((dt300 > 150) && (phase_rw_50_set == false)) {
			phase_rw_50_set = true;
			SetVal_042.MainEstimatorMode 	= 6;
			SetVal_042.BackupEstimatorMode 	= 1;
			SetVal_042.ControlMode 			= 50;
			SetVal_042.ControlTimeout		= 80;
			interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);			
		}
		else if (dt300 > 140 && (phase_vec_0_set == false)) {
			phase_vec_0_set = true;
			SetVal_076.cmdHx = 0.0;
			SetVal_076.cmdHy = 0.0;
			SetVal_076.cmdHz = 0.0;
			interstatus = ADCS2_SetOpenLoopCmdHxyzRW(&SetVal_076);
		}
		else if ((dt300 > 20) && (phase_rw_51_set == false)) {
			phase_rw_51_set = true;
			SetVal_042.MainEstimatorMode 	= 6;
			SetVal_042.BackupEstimatorMode 	= 1;
			SetVal_042.ControlMode 			= 51;
			SetVal_042.ControlTimeout		= 120;
			interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);			

			OS_TaskDelay(100);

				if (flag_contmode == 0) {
					SetVal_076.cmdHx = 0.0;
					SetVal_076.cmdHy = 0.0;
					SetVal_076.cmdHz = 0.0;
					interstatus = ADCS2_SetOpenLoopCmdHxyzRW(&SetVal_076);
				}
				else if (flag_contmode == 1) {
					SetVal_076.cmdHx = cmdH;
					SetVal_076.cmdHy = 0.0;
					SetVal_076.cmdHz = 0.0;
					interstatus = ADCS2_SetOpenLoopCmdHxyzRW(&SetVal_076);
				}
				else if (flag_contmode == 2) {
					SetVal_076.cmdHx = 0.0;
					SetVal_076.cmdHy = cmdH;
					SetVal_076.cmdHz = 0.0;
					interstatus = ADCS2_SetOpenLoopCmdHxyzRW(&SetVal_076);		
				}
				else if (flag_contmode == 3) {
					SetVal_076.cmdHx = 0.0;
					SetVal_076.cmdHy = 0.0;
					SetVal_076.cmdHz = cmdH;
					interstatus = ADCS2_SetOpenLoopCmdHxyzRW(&SetVal_076);		
				}
				else {
					SetVal_076.cmdHx = 0.0;
					SetVal_076.cmdHy = 0.0;
					SetVal_076.cmdHz = 0.0;
					interstatus = ADCS2_SetOpenLoopCmdHxyzRW(&SetVal_076);		
				}
		}

		t0_10 = CFE_TIME_GetTime();	// Initialize 1 seconds Counter
		dt = 0;	// Initialize time gap
		

		memset(&RetVal_210, 0, sizeof(RetVal_210));
		memset(&RetVal_205, 0, sizeof(RetVal_205));
		memset(&RetVal_209, 0, sizeof(RetVal_209));
		
		// Comm_06_Tlm_Set.sync_word 	= 0xADC5;

		// 		6-1) Read Tlm of Main:	
		interstatus = ADCS2_GetMainEstTlm(&RetVal_210);
		OS_TaskDelay(20);

		// 		6-2) Read Sensor Raw Values - RWL	
		interstatus = ADCS2_GetRawRWLSensor(&RetVal_205);
		OS_TaskDelay(20);

		// 		6-3) Read Sensor Calibrated Values - RWL
		interstatus = ADCS2_GetCalibratedRWLSensor(&RetVal_209);
		OS_TaskDelay(20);
		
		// 7) Write the Tlm to the file
		Comm_06_Tlm_Set.MainEst		= RetVal_210;
		Comm_06_Tlm_Set.RawRWL		= RetVal_205;
		Comm_06_Tlm_Set.CalRWL		= RetVal_209;

		if ((written = fwrite(&Comm_06_Tlm_Set, sizeof(Comm_06_Tlm_Set), 1, fp)) != 1)
		{
			status = -6;		// Something is wrong! ::::: File write error
			ADCS2_HandleReport(status, ADCS2_COMM_06_CC, NULL, 0);
			fclose(fp);

			SetVal_042.MainEstimatorMode 	= 6;
			SetVal_042.BackupEstimatorMode 	= 5;
			SetVal_042.ControlMode 			= 3;
			SetVal_042.ControlTimeout		= 0;
			interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);

			SetVal_056.RWL0		= 0;
			SetVal_056.RWL1		= 0;
			SetVal_056.RWL2		= 0;
			SetVal_056.RWL3		= 0;
			interstatus = ADCS2_SetPowerState(&SetVal_056);		// Set RWLX --> OFF
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
	fclose(fp);

	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 5;
	SetVal_042.ControlMode 			= 3;
	SetVal_042.ControlTimeout		= 0;
	interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);

	SetVal_056.RWL0		= 0;
	SetVal_056.RWL1		= 0;
	SetVal_056.RWL2		= 0;
	SetVal_056.RWL3		= 0;
	interstatus = ADCS2_SetPowerState(&SetVal_056);		// Set RWLX --> OFF

	ADCS2_HandleReport(CFE_SUCCESS, ADCS2_COMM_06_CC, NULL, 0);

	return CFE_SUCCESS;
}

CFE_Status_t ADCS2_Comm07Cmd(const ADCS2_Comm07Cmd_t *msg) {
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

	uint8 cnt_try = 0;
		
	// 1) Power On ( GYR0, MAG0, FSS0, RWLX )
	ADCS2_PowerState_Cmn_Payload_t		SetVal_056 = {0,};
	ADCS2_PowerState_Cmn_Payload_t		RetVal_183 = {0,};
	SetVal_056.MAG0		= 1;
	SetVal_056.GYR0		= 1;
	SetVal_056.FSS0		= 1;
	SetVal_056.RWL0		= 1;
	SetVal_056.RWL1		= 1;
	SetVal_056.RWL2		= 1;
	SetVal_056.RWL3		= 1;

	interstatus = ADCS2_SetPowerState(&SetVal_056);		// Set GYR0 & MAG0 & FSS0 & RWLX --> ON
	OS_TaskDelay(10);
	interstatus = ADCS2_GetPowerState(&RetVal_183);		// Get GYR0 & MAG0 & FSS0 & RWLX power state
	OS_TaskDelay(5000);
	cnt_try = 0;
	while (cnt_try < 3)
	{		
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1) && (RetVal_183.FSS0 == 1) && (RetVal_183.RWL0 == 1) && (RetVal_183.RWL1 == 1) && (RetVal_183.RWL2 == 1) && (RetVal_183.RWL3 == 1)) break;	// Check Power On well

		// if not, try again until 3 times
		interstatus = ADCS2_SetPowerState(&SetVal_056);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetPowerState(&RetVal_183);
		cnt_try++;
	}

	interstatus = ADCS2_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1) || (RetVal_183.FSS0 != 1) || (RetVal_183.RWL0 != 1) || (RetVal_183.RWL1 != 1) || (RetVal_183.RWL2 != 1) || (RetVal_183.RWL3 != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -1;		// Something is wrong! ::::: Power State
		ADCS2_HandleReport(status, ADCS2_COMM_07_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}


	// 2) Check Status of Sensors
	ADCS2_RawMAGSensorTlm_Paylaod_t	RetVal_180 = {0,};
	ADCS2_RawGYRSensorTlm_Payload_t	RetVal_204 = {0,};
	// ADCS2_RawCubeSenseSunTlm_Payload_t RetVal_170 = {0,};		// "FSS0 valid" is not identified yet. After idetification, use this status to check the state of FSS0. -> (line 1578, 1584)
	ADCS2_RawRWLSensorTlm_Payload_t RetVal_205 = {0,};

	OS_TaskDelay(2000);
	interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	// OS_TaskDelay(10);
	// interstatus = interstatus + ADCS2_GetRawCubeSenseSun(&RetVal_170);	// Get Status of FSS
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels
	

	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1) && (RetVal_205.RWL0ValidFlag == 1) && (RetVal_205.RWL1ValidFlag == 1) && (RetVal_205.RWL2ValidFlag == 1) && (RetVal_205.RWL3ValidFlag == 1)) break;	// Check Sensors Status Good
		// if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1) && (RetVal_170.ValidResult0 == 1)) break;	// Check Sensors Status Good

		// if not, wait changing status until 3 times
		OS_TaskDelay(100);
		interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS2_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels
		cnt_try++;
	}

	
	interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels

	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1) || (RetVal_204.GYR0ValidFlag != 1) || (RetVal_205.RWL0ValidFlag != 1) || (RetVal_205.RWL1ValidFlag != 1) || (RetVal_205.RWL2ValidFlag != 1) || (RetVal_205.RWL3ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -3;		// Something is wrong! ::::: Magnetometer Status
		ADCS2_HandleReport(status, ADCS2_COMM_07_CC, &RetVal_180, sizeof(RetVal_180));
		return status;
	}
	OS_TaskDelay(10);

	// 3) Set Est. mode & Cont. mode ( Main: EstGyroEkf  (6) / Main: EstGyro  (5) / Control: ConHxyzRW, ConXYZwheel, ConSunTrack / Timeout: 120+300
	ADCS2_ControlEstimationMode_Cmn_Payload_t	SetVal_042 = {0,};
	ADCS2_ControlEstimationMode_Cmn_Payload_t	RetVal_150 = {0,};

	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 5;
	// SetVal_042.ControlMode 			= 51;
	// SetVal_042.ControlTimeout		= 120;
	SetVal_042.ControlMode 			= 3;
	SetVal_042.ControlTimeout		= 0;
	interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);	// Set Main Est mode = 6 / Set Backup Est mode = 5 / Control mode & Time out = 3 & 0 s
	OS_TaskDelay(100);
	interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);	// Get Main Est mode
	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_150.MainEstimatorMode == 6) && (RetVal_150.ControlMode == 3)) break;	// Check Mode Change well

		// if not, try again until 3 times
		interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);
		cnt_try++;
	}

	interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != 6) || (RetVal_150.ControlMode != 3))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -2;		// Something is wrong! ::::: Estimation Mode
		ADCS2_HandleReport(status, ADCS2_COMM_07_CC, &RetVal_150, sizeof(RetVal_150));
		return status;

	}

	// Before 5) Pre-define Hxyz Command values
	ADCS2_OpenLoopCmdHxyzRWCmd_Payload_t SetVal_076 = {0,};
	SetVal_076.cmdHx = 0.0;
	SetVal_076.cmdHy = 0.0;
	SetVal_076.cmdHz = 0.0;
	interstatus = ADCS2_SetOpenLoopCmdHxyzRW(&SetVal_076);
	
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
		ADCS2_HandleReport(status, ADCS2_COMM_07_CC, NULL, 0);

		SetVal_042.MainEstimatorMode 	= 6;
		SetVal_042.BackupEstimatorMode 	= 5;
		SetVal_042.ControlMode 			= 3;
		SetVal_042.ControlTimeout		= 0;
		interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);

		SetVal_056.RWL0		= 0;
		SetVal_056.RWL1		= 0;
		SetVal_056.RWL2		= 0;
		SetVal_056.RWL3		= 0;
		interstatus = ADCS2_SetPowerState(&SetVal_056);		// Set RWLX --> OFF
		return status;
	}

	// 6) Read Tlm of Estimator & Sensors
	ADCS2_ReferenceRPYvaluesCmd_Payload_t SetVal_054 = {0,};

	ADCS2_Estimator_Cmn_Payload_t	RetVal_210 = {0,};
	ADCS2_CalibratedRWLSensorTlm_Payload_t 	RetVal_209 = {0,};
	ADCS2_CalibratedFSSSensorTlm_Payload_t	RetVal_178 = {0,};

	ADCS2_COMM_07_Payload_t	Comm_07_Tlm_Set = {0,};

	Comm_07_Tlm_Set.sync_word 	= 0xADC5;

	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 5;
	SetVal_042.ControlMode 			= 51;
	SetVal_042.ControlTimeout		= 125;
	interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);

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
			interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);
		}
		else if ((dt300 > 720) && (phase_sun_50_set == false)) {
			phase_sun_50_set = true;
			SetVal_042.MainEstimatorMode 	= 6;
			SetVal_042.BackupEstimatorMode 	= 5;
			SetVal_042.ControlMode 			= 50;
			SetVal_042.ControlTimeout		= 80;
			interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);			
		}

		else if ((dt300 > 420) && (phase_sun_13_set == false)) {
			phase_sun_13_set = true;
			SetVal_042.MainEstimatorMode 	= 6;
			SetVal_042.BackupEstimatorMode 	= 5;
			SetVal_042.ControlMode 			= 13;
			SetVal_042.ControlTimeout		= 305;
			interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);
		}
		else if ((dt300 > 120) && (phase_sun_12_set == false)) {
			phase_sun_12_set = true;
			SetVal_054.Roll = 0.0;
			SetVal_054.Pitch = 0.0;
			SetVal_054.Yaw = 0.0;
			interstatus = ADCS_SetReferenceRPYValues(&SetVal_054);
			OS_TaskDelay(10);

			SetVal_042.MainEstimatorMode 	= 6;
			SetVal_042.BackupEstimatorMode 	= 5;
			SetVal_042.ControlMode 			= 12;
			SetVal_042.ControlTimeout		= 305;
			interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);
		}

		t0_10 = CFE_TIME_GetTime();	// Initialize 1 seconds Counter
		dt = 0;	// Initialize time gap
		

		memset(&RetVal_210, 0, sizeof(RetVal_210));
		memset(&RetVal_205, 0, sizeof(RetVal_205));
		memset(&RetVal_209, 0, sizeof(RetVal_209));
		memset(&RetVal_178, 0, sizeof(RetVal_178));
		
		// Comm_07_Tlm_Set.sync_word 	= 0xADC5;

		// 		6-1) Read Tlm of Main:	
		interstatus = ADCS2_GetMainEstTlm(&RetVal_210);
		OS_TaskDelay(20);

		// 		6-2) Read Sensor Raw Values - RWL	
		interstatus = ADCS2_GetRawRWLSensor(&RetVal_205);
		OS_TaskDelay(20);

		// 		6-3) Read Sensor Calibrated Values - RWL
		interstatus = ADCS2_GetCalibratedRWLSensor(&RetVal_209);
		OS_TaskDelay(20);
		
		// 		6-4) Read Sensor Calibrated Values - FSS
		interstatus = ADCS2_GetCalibratedFSSSensor(&RetVal_178);
		OS_TaskDelay(20);
		
		// 7) Write the Tlm to the file
		Comm_07_Tlm_Set.MainEst		= RetVal_210;
		Comm_07_Tlm_Set.RawRWL		= RetVal_205;
		Comm_07_Tlm_Set.CalRWL		= RetVal_209;
		Comm_07_Tlm_Set.CalFSS		= RetVal_178;

		if ((written = fwrite(&Comm_07_Tlm_Set, sizeof(Comm_07_Tlm_Set), 1, fp)) != 1)
		{
			status = -6;		// Something is wrong! ::::: File write error
			ADCS2_HandleReport(status, ADCS2_COMM_07_CC, NULL, 0);
			fclose(fp);

			SetVal_042.MainEstimatorMode 	= 6;
			SetVal_042.BackupEstimatorMode 	= 5;
			SetVal_042.ControlMode 			= 3;
			SetVal_042.ControlTimeout		= 0;
			interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);

			SetVal_056.RWL0		= 0;
			SetVal_056.RWL1		= 0;
			SetVal_056.RWL2		= 0;
			SetVal_056.RWL3		= 0;
			interstatus = ADCS2_SetPowerState(&SetVal_056);		// Set RWLX --> OFF
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
	fclose(fp);

	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 5;
	SetVal_042.ControlMode 			= 3;
	SetVal_042.ControlTimeout		= 0;
	interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);

	SetVal_056.RWL0		= 0;
	SetVal_056.RWL1		= 0;
	SetVal_056.RWL2		= 0;
	SetVal_056.RWL3		= 0;
	interstatus = ADCS2_SetPowerState(&SetVal_056);		// Set RWLX --> OFF

	ADCS2_HandleReport(CFE_SUCCESS, ADCS2_COMM_07_CC, NULL, 0);

	return CFE_SUCCESS;
}

CFE_Status_t ADCS2_Comm08Cmd(const ADCS2_Comm08Cmd_t *msg) {
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

	uint8 cnt_try = 0;
	
	// 1) Power On ( GYR0, MAG0, FSS0, HSS0, RWLX )
	ADCS2_PowerState_Cmn_Payload_t		SetVal_056 = {0,};
	ADCS2_PowerState_Cmn_Payload_t		RetVal_183 = {0,};
	SetVal_056.MAG0		= 1;
	SetVal_056.GYR0		= 1;
	SetVal_056.FSS0		= 1;
	SetVal_056.HSS0		= 1;
	SetVal_056.RWL0		= 1;
	SetVal_056.RWL1		= 1;
	SetVal_056.RWL2		= 1;
	SetVal_056.RWL3		= 1;
	interstatus = ADCS2_SetPowerState(&SetVal_056);		// Set GYR0 & MAG0 & FSS0 & HSS0 & RWLX --> ON
	OS_TaskDelay(10);
	interstatus = ADCS2_GetPowerState(&RetVal_183);		// Get GYR0 & MAG0 & FSS0 & HSS0 & RWLX power state
	OS_TaskDelay(5000);
	cnt_try = 0;
	while (cnt_try < 3)
	{		
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1) && (RetVal_183.FSS0 == 1) && (RetVal_183.HSS0 == 1) && (RetVal_183.RWL0 == 1) && (RetVal_183.RWL1 == 1) && (RetVal_183.RWL2 == 1) && (RetVal_183.RWL3 == 1)) break;	// Check Power On well

		// if not, try again until 3 times
		interstatus = ADCS2_SetPowerState(&SetVal_056);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetPowerState(&RetVal_183);
		cnt_try++;
	}

	interstatus = ADCS2_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1) || (RetVal_183.FSS0 != 1) || (RetVal_183.HSS0 != 1) || (RetVal_183.RWL0 != 1) || (RetVal_183.RWL1 != 1) || (RetVal_183.RWL2 != 1) || (RetVal_183.RWL3 != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -1;		// Something is wrong! ::::: Power State
		ADCS2_HandleReport(status, ADCS2_COMM_08_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}


	// 2) Check Status of Sensors
	ADCS2_RawMAGSensorTlm_Paylaod_t	RetVal_180 = {0,};
	ADCS2_RawGYRSensorTlm_Payload_t	RetVal_204 = {0,};
	// ADCS2_RawCubeSenseSunTlm_Payload_t RetVal_170 = {0,};		// "FSS0 valid" is not identified yet. After idetification, use this status to check the state of FSS0.
	ADCS2_RawRWLSensorTlm_Payload_t RetVal_205 = {0,};
	// ADCS2_RawCubeSenseEarthTlm_Payload_t RetVal_179 = {0,};		// "HSS0 valid" is not identified yet. After idetification, use this status to check the state of FSS0.

	OS_TaskDelay(2000);
	interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels
	

	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1) && (RetVal_205.RWL0ValidFlag == 1) && (RetVal_205.RWL1ValidFlag == 1) && (RetVal_205.RWL2ValidFlag == 1) && (RetVal_205.RWL3ValidFlag == 1)) break;	// Check Sensors Status Good		

		// if not, wait changing status until 3 times
		OS_TaskDelay(100);
		interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS2_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels
		cnt_try++;
	}

	
	interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels

	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1) || (RetVal_204.GYR0ValidFlag != 1) || (RetVal_205.RWL0ValidFlag != 1) || (RetVal_205.RWL1ValidFlag != 1) || (RetVal_205.RWL2ValidFlag != 1) || (RetVal_205.RWL3ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -3;		// Something is wrong! ::::: Magnetometer Status
		ADCS2_HandleReport(status, ADCS2_COMM_06_CC, &RetVal_180, sizeof(RetVal_180));
		return status;
	}
	OS_TaskDelay(10);


	// 3) Set Est. mode & Cont. mode ( Main: EstGyroEkf  (6) / Main: EstFullEkf  (5) / Control: ConXYXwheel / Timeout: 6000 s )
	ADCS2_ReferenceRPYvaluesCmd_Payload_t 		SetVal_054 = {0,};
	ADCS2_ControlEstimationMode_Cmn_Payload_t	SetVal_042 = {0,};
	ADCS2_ControlEstimationMode_Cmn_Payload_t	RetVal_150 = {0,};

	SetVal_054.Roll = 0.0;
	SetVal_054.Pitch = 0.0;
	SetVal_054.Yaw = 0.0;
	interstatus = ADCS_SetReferenceRPYValues(&SetVal_054);

	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 5;
	SetVal_042.ControlMode 			= 12;
	SetVal_042.ControlTimeout		= 6000;
	interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);	// Set Main Est mode = 6 / Set Backup Est mode = 5 / Control mode & Time out = 3 & 6000 s
	OS_TaskDelay(100);
	interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);	// Get Main Est mode
	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_150.MainEstimatorMode == 6) && (RetVal_150.ControlMode == 12)) break;	// Check Mode Change well

		// if not, try again until 3 times
		interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);
		cnt_try++;
	}

	interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != 6) || (RetVal_150.ControlMode != 12))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -2;		// Something is wrong! ::::: Estimation Mode
		ADCS2_HandleReport(status, ADCS2_COMM_08_CC, &RetVal_150, sizeof(RetVal_150));

		
		SetVal_042.MainEstimatorMode 	= 6;
		SetVal_042.BackupEstimatorMode 	= 5;
		SetVal_042.ControlMode 			= 3;
		SetVal_042.ControlTimeout		= 0;
		interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);

		SetVal_056.HSS0		= 0;
		SetVal_056.RWL0		= 0;
		SetVal_056.RWL1		= 0;
		SetVal_056.RWL2		= 0;
		SetVal_056.RWL3		= 0;
		interstatus = ADCS2_SetPowerState(&SetVal_056);		// Set RWLX --> OFF

		return status;

	}

	// 4) Make "adcs_comm_08.bin" file to log the TLMs
	FILE *fp;
	size_t written;
	fp = fopen("./cf/adcs_comm_08.bin","wb");
	if (fp == NULL)
	{
		status = -5;		// Something is wrong! ::::: File open error
		ADCS2_HandleReport(status, ADCS2_COMM_08_CC, NULL, 0);
		
		SetVal_042.MainEstimatorMode 	= 6;
		SetVal_042.BackupEstimatorMode 	= 5;
		SetVal_042.ControlMode 			= 3;
		SetVal_042.ControlTimeout		= 0;
		interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);

		SetVal_056.HSS0		= 0;
		SetVal_056.RWL0		= 0;
		SetVal_056.RWL1		= 0;
		SetVal_056.RWL2		= 0;
		SetVal_056.RWL3		= 0;
		interstatus = ADCS2_SetPowerState(&SetVal_056);		// Set RWLX --> OFF
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
	ADCS2_Estimator_Cmn_Payload_t	RetVal_210 = {0,};

	ADCS2_RawCubeSenseEarthTlm_Payload_t 	RetVal_179 = {0,};
	ADCS2_CalibratedHSSSensorTlm_Payload_t	RetVal_176 = {0,};

	ADCS2_COMM_08_Payload_t	Comm_08_Tlm_Set = {0,};

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
		interstatus = ADCS2_GetMainEstTlm(&RetVal_210);
		OS_TaskDelay(20);

		// 		6-2) Read Sensor Raw Values - HSS	
		interstatus = ADCS2_GetRawCubeSenseEarth(&RetVal_179);
		OS_TaskDelay(20);

		// 		6-3) Read Sensor Calibrated Values - HSS
		interstatus = ADCS2_GetCalibratedHSSSensor(&RetVal_176);
		OS_TaskDelay(20);

		
		// 7) Write the Tlm to the file
		Comm_08_Tlm_Set.MainEst		= RetVal_210;
		Comm_08_Tlm_Set.RawHSS 		= RetVal_179;
		Comm_08_Tlm_Set.CalHSS 		= RetVal_176;

		if ((written = fwrite(&Comm_08_Tlm_Set, sizeof(Comm_08_Tlm_Set), 1, fp)) != 1)
		{
			status = -6;		// Something is wrong! ::::: File write error
			ADCS2_HandleReport(status, ADCS2_COMM_08_CC, NULL, 0);
			fclose(fp);
			
			SetVal_042.MainEstimatorMode 	= 6;
			SetVal_042.BackupEstimatorMode 	= 5;
			SetVal_042.ControlMode 			= 3;
			SetVal_042.ControlTimeout		= 0;
			interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);

			SetVal_056.HSS0		= 0;
			SetVal_056.RWL0		= 0;
			SetVal_056.RWL1		= 0;
			SetVal_056.RWL2		= 0;
			SetVal_056.RWL3		= 0;
			interstatus = ADCS2_SetPowerState(&SetVal_056);		// Set RWLX --> OFF
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
	fclose(fp);

	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 5;
	SetVal_042.ControlMode 			= 3;
	SetVal_042.ControlTimeout		= 0;
	interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);

	SetVal_056.HSS0		= 0;
	SetVal_056.RWL0		= 0;
	SetVal_056.RWL1		= 0;
	SetVal_056.RWL2		= 0;
	SetVal_056.RWL3		= 0;
	interstatus = ADCS2_SetPowerState(&SetVal_056);		// Set RWLX --> OFF

	ADCS2_HandleReport(CFE_SUCCESS, ADCS2_COMM_08_CC, NULL, 0);

	return CFE_SUCCESS;
}

CFE_Status_t ADCS2_Comm09Cmd(const ADCS2_Comm09Cmd_t *msg) {
	// COMM 09:	Sun Tracking 3-axis Control
	// [Procedure]
	// 1) Power On ( GYR0, MAG0, FSS0, RWLX )
	// 2) Check Status of Sensors
	// 3) Set Est. mode & Cont. mode ( Main: EstGyroEkf  (6) / Main: EstGyro  (5) / Control: ConHxyzRW, ConXYZwheel, ConSunTrack / Timeout: 120+300+500 s )
	//	  Set Flag of UseEkf: (HSS), STT elements false <-- GS Command (already set-up)
	// ** Status between every step must be reported!!

	CFE_Status_t						status;
	CFE_Status_t						interstatus;

	uint8 cnt_try = 0;
		
	// 1) Power On ( GYR0, MAG0, FSS0, RWLX )
	ADCS2_PowerState_Cmn_Payload_t		SetVal_056 = {0,};
	ADCS2_PowerState_Cmn_Payload_t		RetVal_183 = {0,};
	SetVal_056.MAG0		= 1;
	SetVal_056.GYR0		= 1;
	SetVal_056.FSS0		= 1;
	SetVal_056.RWL0		= 1;
	SetVal_056.RWL1		= 1;
	SetVal_056.RWL2		= 1;
	SetVal_056.RWL3		= 1;

	interstatus = ADCS2_SetPowerState(&SetVal_056);		// Set GYR0 & MAG0 & FSS0 & RWLX --> ON
	OS_TaskDelay(10);
	interstatus = ADCS2_GetPowerState(&RetVal_183);		// Get GYR0 & MAG0 & FSS0 & RWLX power state
	OS_TaskDelay(5000);
	cnt_try = 0;
	while (cnt_try < 3)
	{		
		if ((interstatus == CFE_SUCCESS) && (RetVal_183.MAG0 == 1) && (RetVal_183.GYR0 == 1) && (RetVal_183.FSS0 == 1) && (RetVal_183.RWL0 == 1) && (RetVal_183.RWL1 == 1) && (RetVal_183.RWL2 == 1) && (RetVal_183.RWL3 == 1)) break;	// Check Power On well

		// if not, try again until 3 times
		interstatus = ADCS2_SetPowerState(&SetVal_056);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetPowerState(&RetVal_183);
		cnt_try++;
	}

	interstatus = ADCS2_GetPowerState(&RetVal_183);
	if ((interstatus != CFE_SUCCESS) || (RetVal_183.MAG0 != 1) || (RetVal_183.GYR0 != 1) || (RetVal_183.FSS0 != 1) || (RetVal_183.RWL0 != 1) || (RetVal_183.RWL1 != 1) || (RetVal_183.RWL2 != 1) || (RetVal_183.RWL3 != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -1;		// Something is wrong! ::::: Power State
		ADCS2_HandleReport(status, ADCS2_COMM_09_CC, &RetVal_183, sizeof(RetVal_183));
		return status;
	}


	// 2) Check Status of Sensors
	ADCS2_RawMAGSensorTlm_Paylaod_t	RetVal_180 = {0,};
	ADCS2_RawGYRSensorTlm_Payload_t	RetVal_204 = {0,};
	// ADCS2_RawCubeSenseSunTlm_Payload_t RetVal_170 = {0,};		// "FSS0 valid" is not identified yet. After idetification, use this status to check the state of FSS0. -> (line 1578, 1584)
	ADCS2_RawRWLSensorTlm_Payload_t RetVal_205 = {0,};

	OS_TaskDelay(2000);
	interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	// OS_TaskDelay(10);
	// interstatus = interstatus + ADCS2_GetRawCubeSenseSun(&RetVal_170);	// Get Status of FSS
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels
	

	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1) && (RetVal_205.RWL0ValidFlag == 1) && (RetVal_205.RWL1ValidFlag == 1) && (RetVal_205.RWL2ValidFlag == 1) && (RetVal_205.RWL3ValidFlag == 1)) break;	// Check Sensors Status Good
		// if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1) && (RetVal_170.ValidResult0 == 1)) break;	// Check Sensors Status Good

		// if not, wait changing status until 3 times
		OS_TaskDelay(100);
		interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
		OS_TaskDelay(10);
		interstatus = interstatus + ADCS2_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels
		cnt_try++;
	}

	
	interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawRWLSensor(&RetVal_205);	// Get Status of wheels

	if ((interstatus != CFE_SUCCESS) || (RetVal_180.MAG0ValidFlag != 1) || (RetVal_204.GYR0ValidFlag != 1) || (RetVal_205.RWL0ValidFlag != 1) || (RetVal_205.RWL1ValidFlag != 1) || (RetVal_205.RWL2ValidFlag != 1) || (RetVal_205.RWL3ValidFlag != 1))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -3;		// Something is wrong! ::::: Magnetometer Status
		ADCS2_HandleReport(status, ADCS2_COMM_09_CC, &RetVal_180, sizeof(RetVal_180));
		return status;
	}
	OS_TaskDelay(10);

	// 3) Set Est. mode & Cont. mode ( Main: EstGyroEkf  (6) / Main: EstGyro  (5) / Control: ConHxyzRW, ConXYZwheel, ConSunTrack / Timeout: 120+300
	ADCS2_ControlEstimationMode_Cmn_Payload_t	SetVal_042 = {0,};
	ADCS2_ControlEstimationMode_Cmn_Payload_t	RetVal_150 = {0,};

	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 5;
	// SetVal_042.ControlMode 			= 51;
	// SetVal_042.ControlTimeout		= 120;
	SetVal_042.ControlMode 			= 3;
	SetVal_042.ControlTimeout		= 0;
	interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);	// Set Main Est mode = 6 / Set Backup Est mode = 5 / Control mode & Time out = 3 & 0 s
	OS_TaskDelay(100);
	interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);	// Get Main Est mode
	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_150.MainEstimatorMode == 6) && (RetVal_150.ControlMode == 3)) break;	// Check Mode Change well

		// if not, try again until 3 times
		interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);
		OS_TaskDelay(10);
		interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);
		cnt_try++;
	}

	interstatus = ADCS2_GetControlEstimationMode(&RetVal_150);
	if ((interstatus != CFE_SUCCESS) || (RetVal_150.MainEstimatorMode != 6) || (RetVal_150.ControlMode != 3))
	{	// after full try, if still something is wrong, STOP COMM Sequence!
		status = -2;		// Something is wrong! ::::: Estimation Mode
		ADCS2_HandleReport(status, ADCS2_COMM_09_CC, &RetVal_150, sizeof(RetVal_150));
		return status;

	}

	// Before 5) Pre-define Hxyz Command values
	ADCS2_OpenLoopCmdHxyzRWCmd_Payload_t SetVal_076 = {0,};
	SetVal_076.cmdHx = 0.0;
	SetVal_076.cmdHy = 0.0;
	SetVal_076.cmdHz = 0.0;
	interstatus = ADCS2_SetOpenLoopCmdHxyzRW(&SetVal_076);
	
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
	

	ADCS2_ReferenceRPYvaluesCmd_Payload_t SetVal_054 = {0,};
	SetVal_042.MainEstimatorMode 	= 6;
	SetVal_042.BackupEstimatorMode 	= 5;
	SetVal_042.ControlMode 			= 51;
	SetVal_042.ControlTimeout		= 125;
	interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);

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
			interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);
		}
		else if ((dt300 > 120) && (phase_ctrl_12_set == false)) {
			phase_ctrl_12_set = true;
			SetVal_054.Roll = 0.0;
			SetVal_054.Pitch = 0.0;
			SetVal_054.Yaw = 0.0;
			interstatus = ADCS_SetReferenceRPYValues(&SetVal_054);
			OS_TaskDelay(10);

			SetVal_042.MainEstimatorMode 	= 6;
			SetVal_042.BackupEstimatorMode 	= 5;
			SetVal_042.ControlMode 			= 12;
			SetVal_042.ControlTimeout		= 305;
			interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);
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
	interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);

	ADCS2_HandleReport(CFE_SUCCESS, ADCS2_COMM_09_CC, NULL, 0);

	return CFE_SUCCESS;
}
