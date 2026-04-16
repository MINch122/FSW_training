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
/* ADCS NOOP commands														*/
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
	OS_printf("Set reset success.\n");

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
	OS_printf("ADCS cmd Success.");

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
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS2_SetControlEstimationModeCmd(const ADCS2_ControlEstimationModeCmd_t *msg) {
    // ID 42
    CFE_Status_t               status;

    status = ADCS2_SetControlEstimationMode(&msg->Payload);

    ADCS2_HandleReport(status, ADCS2_SET_CONTROL_ESTIMATION_MODE_CC, NULL, 0);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Mounting Config: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

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
    OS_printf("ADCS cmd Success.");

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
    OS_printf("ADCS cmd Success.");

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
	
	// Handling Retval
	OS_printf("Unix Time sec: %u || Unix Time subsec: %u\n", RetVal.CurrentUnixseconds, RetVal.CurrentUnixNanoseconds);
	
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
    
    // Handling Retval
    OS_printf("Control Mode: %u || Main Estimator Mode: %u || Backup Estimator Mode: %u || Control Timeout: %u\n",
                RetVal.ControlMode, RetVal.MainEstimatorMode, RetVal.BackupEstimatorMode, RetVal.ControlTimeout);
    
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
    
    // Handling Retval
    OS_printf("MAG0 Raw X: %d || MAG0 Raw Y: %d || MAG0 Raw Z: %d || MAG0 Valid Flag: %u\n",
                RetVal.MAG0RawVecX, RetVal.MAG0RawVecY, RetVal.MAG0RawVecZ, RetVal.MAG0ValidFlag);
    
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
    
    // Handling Retval
    OS_printf("RWL0: %u || RWL1: %u || RWL2: %u || RWL3: %u\n", RetVal.RWL0, RetVal.RWL1, RetVal.RWL2, RetVal.RWL3);
    OS_printf("MAG0: %u || MAG1: %u || GYR0: %u || GYR1: %u\n", RetVal.MAG0, RetVal.MAG1, RetVal.GYR0, RetVal.GYR1);
    OS_printf("FSS0: %u || FSS1: %u || FSS2: %u || FSS3: %u\n", RetVal.FSS0, RetVal.FSS1, RetVal.FSS2, RetVal.FSS3);
    OS_printf("HSS0: %u || HSS1: %u || STR0: %u || STR1: %u\n", RetVal.HSS0, RetVal.HSS1, RetVal.STR0, RetVal.STR1);
    OS_printf("EXT: Sensor0 %u || Sensor1: %u || GYR0: %u || GYR1: %u\n", RetVal.ExtSensor0, RetVal.ExtSensor1, RetVal.ExtGYR0, RetVal.ExtGYR1);
    
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
    
    // Handling Retval
    // OS_printf("[Direction ENUM]\n");
    // OS_printf("| +X | -X | +Y | -Y | +Z | -Z |\n");
    // OS_printf("|  1 |  2 |  3 |  4 |  5 |  6 |\n");

    // OS_printf("\n");
    // OS_printf("[Stack]\n");
    // OS_printf("X-Y-Z: %u-%u-%u\n", RetVal.StackX_mounting, RetVal.StackY_mounting, RetVal.StackZ_mounting);

    // OS_printf("\n");
    // OS_printf("[MTQ]\n");
    // OS_printf("0-1-2: %u-%u-%u\n", RetVal.MTQ0_mounting, RetVal.MTQ1_mounting, RetVal.MTQ2_mounting);    
    
    // OS_printf("\n");
    // OS_printf("[Wheel]\n");
    // OS_printf("0-1-2-3: %u-%u-%u-%u\n", RetVal.Wheel0_mounting, RetVal.Wheel1_mounting, RetVal.Wheel2_mounting, RetVal.Wheel3_mounting);
    // OS_printf("Pyramid alpha-beta-gamma: %u-%u-%u\n", RetVal.PyramidRWL_alpha, RetVal.PyramidRWL_beta, RetVal.PyramidRWL_gamma);
    
    // OS_printf("\n");
    // OS_printf("[CSS]\n");
    // OS_printf("0-1-2-3-4: %u-%u-%u-%u-%u\n", RetVal.CSS0_mounting, RetVal.CSS1_mounting, RetVal.CSS2_mounting, RetVal.CSS3_mounting, RetVal.CSS4_mounting);
    // OS_printf("5-6-7-8-9: %u-%u-%u-%u-%u\n", RetVal.CSS5_mounting, RetVal.CSS6_mounting, RetVal.CSS7_mounting, RetVal.CSS8_mounting, RetVal.CSS9_mounting);

    // OS_printf("\n");
    // OS_printf("[FSS0]\n");
    // OS_printf("(RAW)  alpha-beta-gamma: %d-%d-%d\n", RetVal.FSS0_alpha, RetVal.FSS0_beta, RetVal.FSS0_gamma);
    // OS_printf("(Real) alpha-beta-gamma: %f-%f-%f\n", RetVal.FSS0_alpha*0.01, RetVal.FSS0_beta*0.01, RetVal.FSS0_gamma*0.01);

    // OS_printf("\n");
    // OS_printf("[FSS1]\n");
    // OS_printf("(RAW)  alpha-beta-gamma: %d-%d-%d\n", RetVal.FSS1_alpha, RetVal.FSS1_beta, RetVal.FSS1_gamma);
    // OS_printf("(Real) alpha-beta-gamma: %f-%f-%f\n", RetVal.FSS1_alpha*0.01, RetVal.FSS1_beta*0.01, RetVal.FSS1_gamma*0.01);

    // OS_printf("\n");
    // OS_printf("[HSS0]\n");
    // OS_printf("(RAW)  alpha-beta-gamma: %d-%d-%d\n", RetVal.HSS0_alpha, RetVal.HSS0_beta, RetVal.HSS0_gamma);
    // OS_printf("(Real) alpha-beta-gamma: %f-%f-%f\n", RetVal.HSS0_alpha*0.01, RetVal.HSS0_beta*0.01, RetVal.HSS0_gamma*0.01);

    // OS_printf("\n");
    // OS_printf("[HSS1]\n");
    // OS_printf("(RAW)  alpha-beta-gamma: %d-%d-%d\n", RetVal.HSS1_alpha, RetVal.HSS1_beta, RetVal.HSS1_gamma);
    // OS_printf("(Real) alpha-beta-gamma: %f-%f-%f\n", RetVal.HSS1_alpha*0.01, RetVal.HSS1_beta*0.01, RetVal.HSS1_gamma*0.01);

    // OS_printf("\n");
    // OS_printf("[MAG0]\n");
    // OS_printf("(RAW)  alpha-beta-gamma: %d-%d-%d\n", RetVal.MAG0_alpha, RetVal.MAG0_beta, RetVal.MAG0_gamma);
    // OS_printf("(Real) alpha-beta-gamma: %f-%f-%f\n", RetVal.MAG0_alpha*0.01, RetVal.MAG0_beta*0.01, RetVal.MAG0_gamma*0.01);

    // OS_printf("\n");
    // OS_printf("[MAG1]\n");
    // OS_printf("(RAW)  alpha-beta-gamma: %d-%d-%d\n", RetVal.MAG1_alpha, RetVal.MAG1_beta, RetVal.MAG1_gamma);
    // OS_printf("(Real) alpha-beta-gamma: %f-%f-%f\n", RetVal.MAG1_alpha*0.01, RetVal.MAG1_beta*0.01, RetVal.MAG1_gamma*0.01);


    // OS_printf("\n");
    // OS_printf("[STR0]\n");
    // OS_printf("(RAW)  alpha-beta-gamma: %d-%d-%d\n", RetVal.STR0_alpha, RetVal.STR0_beta, RetVal.STR0_gamma);
    // OS_printf("(Real) alpha-beta-gamma: %f-%f-%f\n", RetVal.STR0_alpha*0.01, RetVal.STR0_beta*0.01, RetVal.STR0_gamma*0.01);

    // OS_printf("\n");
    // OS_printf("[STR1]\n");
    // OS_printf("(RAW)  alpha-beta-gamma: %d-%d-%d\n", RetVal.STR1_alpha, RetVal.STR1_beta, RetVal.STR1_gamma);
    // OS_printf("(Real) alpha-beta-gamma: %f-%f-%f\n", RetVal.STR1_alpha*0.01, RetVal.STR1_beta*0.01, RetVal.STR1_gamma*0.01);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS2_GetRawGYRSensorCmd(void) {
    // ID 204
    CFE_Status_t               status;
    ADCS2_RawGYRSensorTlm_Paylaod_t RetVal = {0,};

    status = ADCS2_GetRawGYRSensor(&RetVal);

    ADCS2_HandleReport(status, ADCS2_GET_RAW_GYR_SENSOR_CC, &RetVal, sizeof(RetVal));

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Get Raw GYR Sensor: 0x%08lx", (unsigned long)status);
        return status;
    }
    
    // Handling Retval
    OS_printf("TimeSec : %u || TimeNanoSec : %u\n", RetVal.TimeSeconds, RetVal.TimeNanoSeconds);
    OS_printf("GYR0 RawRate X: %f || RawRate Y: %f || RawRate Z: %f\n", RetVal.GYR0RawRateX, RetVal.GYR0RawRateY, RetVal.GYR0RawRateZ);
    OS_printf("GYR1 RawRate X: %f || RawRate Y: %f || RawRate Z: %f\n", RetVal.GYR1RawRateX, RetVal.GYR1RawRateY, RetVal.GYR1RawRateZ);
    OS_printf("Valid Flag GYR0 : 0x%02X\n", RetVal.GYR0ValidFlag);
    
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
	if ((flag_tlmtype != 0) || (flag_tlmtype != 1))
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

	// 2) Set Est. mode ( Main: EstGyro (1) / Backup: EstMagRkf (2) )
	ADCS2_ControlEstimationMode_Cmn_Payload_t	SetVal_042 = {0,};
	ADCS2_ControlEstimationMode_Cmn_Payload_t	RetVal_150 = {0,};

	SetVal_042.MainEstimatorMode 	= 1;
	SetVal_042.BackupEstimatorMode 	= 2;
	interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);	// Set Main & Backup Est mode = 1 & 2
	OS_TaskDelay(10);
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

	// 3) Check Status of Sensors
	ADCS2_RawMAGSensorTlm_Paylaod_t	RetVal_180 = {0,};
	ADCS2_RawGYRSensorTlm_Paylaod_t	RetVal_204 = {0,};

	interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro

	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1)) break;	// Check Sensors Status Good

		// if not, wait changing status until 3 times
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

	if ((flag_contmode != 0) || (flag_contmode != 1) || (flag_contmode != 3) || (flag_contmode != 4)) 
	{
		status = -2;
		ADCS2_HandleReport(status, ADCS2_COMM_02_CC, &flag_contmode, sizeof(flag_contmode));
		return status;
	}
	
	if ((flag_tlmtype != 0) || (flag_tlmtype != 1))
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

	// 2) Set Est. mode & Cont. mode ( Main: EstGyro (1) / Backup: EstMagRkf (2) / Control: ConBdot or ConBdot3 / Timeout: 600 s )
	ADCS2_ControlEstimationMode_Cmn_Payload_t	SetVal_042 = {0,};
	ADCS2_ControlEstimationMode_Cmn_Payload_t	RetVal_150 = {0,};

	SetVal_042.MainEstimatorMode 	= 1;
	SetVal_042.BackupEstimatorMode 	= 2;
	SetVal_042.ControlMode 			= flag_contmode;
	SetVal_042.ControlTimeout		= 600;
	interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);	// Set Main & Backup Est mode = 1 & 2 / Control mode & Time out = 0 or 1 or 3 or 4 & 600 s
	OS_TaskDelay(10);
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

	// 3) Check Status of Sensors
	ADCS2_RawMAGSensorTlm_Paylaod_t	RetVal_180 = {0,};
	ADCS2_RawGYRSensorTlm_Paylaod_t	RetVal_204 = {0,};

	interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro

	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1)) break;	// Check Sensors Status Good

		// if not, wait changing status until 3 times
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
	if ((flag_tlmtype != 0) || (flag_tlmtype != 1))
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

	// 2) Set Est. mode & Cont. mode ( Main: EstGyro (1) / Control: ConBdot3 / Timeout: 6000 s )
	ADCS2_ControlEstimationMode_Cmn_Payload_t	SetVal_042 = {0,};
	ADCS2_ControlEstimationMode_Cmn_Payload_t	RetVal_150 = {0,};

	SetVal_042.MainEstimatorMode 	= 1;
	SetVal_042.BackupEstimatorMode 	= 2;
	SetVal_042.ControlMode 			= 3;
	SetVal_042.ControlTimeout		= 6000;
	interstatus = ADCS2_SetControlEstimationMode(&SetVal_042);	// Set Main Est mode = 1 / Control mode & Time out = 3 & 6000 s
	OS_TaskDelay(10);
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

	// 3) Check Status of Sensors
	ADCS2_RawMAGSensorTlm_Paylaod_t	RetVal_180 = {0,};
	ADCS2_RawGYRSensorTlm_Paylaod_t	RetVal_204 = {0,};

	interstatus = ADCS2_GetRawMAGSensor(&RetVal_180);	// Get Status of magnetometer
	OS_TaskDelay(10);
	interstatus = interstatus + ADCS2_GetRawGYRSensor(&RetVal_204);	// Get Status of gyro

	cnt_try = 0;
	while (cnt_try < 3)
	{
		if ((interstatus == CFE_SUCCESS) && (RetVal_180.MAG0ValidFlag == 1) && (RetVal_204.GYR0ValidFlag == 1)) break;	// Check Sensors Status Good

		// if not, wait changing status until 3 times
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
