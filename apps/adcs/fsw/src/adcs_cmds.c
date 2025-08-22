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
    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(ADCS_AppData.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(ADCS_AppData.HkTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SendBcnCmd(const ADCS_SendBcnCmd_t *Msg)
{
    int32 Status;
    ADCS_PowerStateTlm_Payload_t PwrStt = {0,};
    ADCS_ControlModeTlm_Payload_t CtrlMode = {0,};
    ADCS_CalibratedGYRSensorTlm_Payload_t CalGYR = {0,};

    Status = ADCS_GetPowerState(&PwrStt);
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
    if (Status == CFE_SUCCESS) {
        ADCS_AppData.BcnTlm.Payload.ControlMode = CtrlMode.ControlMode;
    }
    Status = ADCS_GetCalibratedGYRSensor(&CalGYR);
    if (Status == CFE_SUCCESS) {
        ADCS_AppData.BcnTlm.Payload.GYR0CalibratedRateXComponent = CalGYR.GYR0CalibratedRateX;
        ADCS_AppData.BcnTlm.Payload.GYR0CalibratedRateYComponent = CalGYR.GYR0CalibratedRateY;
        ADCS_AppData.BcnTlm.Payload.GYR0CalibratedRateZComponent = CalGYR.GYR0CalibratedRateZ;
    }
    
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(ADCS_AppData.BcnTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(ADCS_AppData.BcnTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* ADCS NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t ADCS_NoopCmd(const ADCS_NoopCmd_t *Msg)
{
    ADCS_AppData.CmdCounter++;

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

    CFE_EVS_SendEvent(ADCS_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "ADCS: RESET command");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* ADCS RELATED COMMANDS COME HERE                                            */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CFE_Status_t ADCS_EN_HighCmd(void) {
    CFE_Status_t               status;

    CFE_SRL_GPIO_Handle_t *Handle = CFE_SRL_ApiGetGpioHandle(CFE_SRL_ADCS_EN_GPIO_INDEXER);
    status = CFE_SRL_ApiGpioSet(Handle, true);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Enable Enable pin: 0x%08lx", (unsigned long)status);
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_EN_LowCmd(void){
    CFE_Status_t               status;

    CFE_SRL_GPIO_Handle_t *Handle = CFE_SRL_ApiGetGpioHandle(CFE_SRL_ADCS_EN_GPIO_INDEXER);
    status = CFE_SRL_ApiGpioSet(Handle, false);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Disable Enable pin: 0x%08lx", (unsigned long)status);
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_Boot_HighCmd(void){
    CFE_Status_t               status;

    CFE_SRL_GPIO_Handle_t *Handle = CFE_SRL_ApiGetGpioHandle(CFE_SRL_ADCS_BOOT_GPIO_INDEXER);
    status = CFE_SRL_ApiGpioSet(Handle, true);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Enable Boot pin: 0x%08lx", (unsigned long)status);
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_Boot_LowCmd(void){
    CFE_Status_t               status;

    CFE_SRL_GPIO_Handle_t *Handle = CFE_SRL_ApiGetGpioHandle(CFE_SRL_ADCS_BOOT_GPIO_INDEXER);
    status = CFE_SRL_ApiGpioSet(Handle, false);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Disable Boot pin: 0x%08lx", (unsigned long)status);
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_ExitBootloader(void){
    CFE_Status_t               status = 0;

    // status = setJumpToDefaultApp_CAN();

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Exit Bootloader: 0x%08lx", (unsigned long)status);
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetReset(void){
    CFE_Status_t               status;

    status = ADCS_Reset();

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Reset: 0x%08lx", (unsigned long)status);
        return status;
    }

    return CFE_SUCCESS;
}


/********************************************************
 * 
 * COSMIC Actual invoked command function
 * Upper functions are just the references
 * 
 ********************************************************/

/* Set function */
CFE_Status_t ADCS_SetCurrentUnixTimeCmd(ADCS_CurrentUnixTimeCmd_t *msg) {
    // ID 2
    CFE_Status_t               status;

    status = ADCS_SetCurrentUnixTime(&msg->Payload);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Current Unix Time: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetControlEstimationModeCmd(ADCS_ControlEstimationModeCmd_t *msg) {
    // ID 42
    CFE_Status_t               status;

    status = ADCS_SetControlEstimationMode(&msg->Payload);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Current Unix Time: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetReferenceLLHTargetCmd(ADCS_ReferenceLLHTargetCmd_t *msg) {
    // ID 48
    CFE_Status_t               status;

    status = ADCS_SetReferenceLLHTarget(&msg->Payload);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Current Unix Time: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetOrbitModeCmd(ADCS_OrbitModeCmd_t *msg) {
    // ID 48
    CFE_Status_t               status;

    status = ADCS_SetOrbitMode(&msg->Payload);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Current Unix Time: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetReferenceRPYValuesCmd(ADCS_ReferenceRPYvaluesCmd_t *msg) {
    // ID 54
    CFE_Status_t               status;

    status = ADCS_SetReferenceRPYValues(&msg->Payload);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Current Unix Time: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

CFE_Status_t ADCS_SetSatOrbitParamConfigCmd(ADCS_SatOrbitParamConfigCmd_t *msg) {
    // ID 68
    CFE_Status_t               status;

    status = ADCS_SetSatOrbitParamConfig(&msg->Payload);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Current Unix Time: 0x%08lx", (unsigned long)status);
        return status;
    }
    OS_printf("ADCS cmd Success.");

    return CFE_SUCCESS;
}

/* Get function */
CFE_Status_t ADCS_GetCurrentUnixTimeCmd(void) {
    // ID 133
    CFE_Status_t               status;
    ADCS_CurrentUnixTimeTlm_Payload_t RetVal = {0,};

    status = ADCS_GetCurrentUnixTime(&RetVal);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Current Unix Time: 0x%08lx", (unsigned long)status);
        return status;
    }
    
    // Handling Retval
    OS_printf("Unix Time sec: %u || Unix Time subsec: %u\n", RetVal.CurrentUnixseconds, RetVal.CurrentUnixNanoseconds);
    
    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetControlEstimationModeCmd(void) {
    // ID 150
    CFE_Status_t               status;
    ADCS_ControlEstimationModeTlm_Payload_t RetVal = {0,};

    status = ADCS_GetControlEstimationMode(&RetVal);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Current Unix Time: 0x%08lx", (unsigned long)status);
        return status;
    }
    
    // Handling Retval
    OS_printf("Control Mode: %u || Main Estimator Mode: %u || Backup Estimator Mode: %u || Control Timeout: %u\n",
                RetVal.ControlMode, RetVal.MainEstimatorMode, RetVal.BackupEstimatorMode, RetVal.ControlTimeout);
    
    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetReferenceLLHTargetCmd(void) {
    // ID 157
    CFE_Status_t               status;
    ADCS_ReferenceLLHTargetTlm_Payload_t RetVal = {0,};

    status = ADCS_GetReferenceLLHTarget(&RetVal);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Current Unix Time: 0x%08lx", (unsigned long)status);
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

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Current Unix Time: 0x%08lx", (unsigned long)status);
        return status;
    }
    
    // Handling Retval
    OS_printf("Orbit Mode: %u\n", RetVal.OrbitMode);
    
    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetRawCubeSenseSunCmd(void) {
    // ID 170
    CFE_Status_t               status;
    ADCS_RawCubeSenseSunTlm_Payload_t RetVal = {0,};

    status = ADCS_GetRawCubeSenseSun(&RetVal);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Current Unix Time: 0x%08lx", (unsigned long)status);
        return status;
    }
    
    // Handling Retval
    OS_printf("TimeSec: %u || TimeNanoSec: %u\n", RetVal.TimeSecond, RetVal.TimeNanoSecond);
    OS_printf("FSS0 Alpha: %d || Beta: %d || Capture Res: %u || Detection Res: %u\n", RetVal.FSS0AlphaAngle, RetVal.FSS0BetaAngle, RetVal.FSS0CaptureResult, RetVal.FSS0DetectionResult);
    OS_printf("FSS1 Alpha: %d || Beta: %d || Capture Res: %u || Detection Res: %u\n", RetVal.FSS1AlphaAngle, RetVal.FSS1BetaAngle, RetVal.FSS1CaptureResult, RetVal.FSS1DetectionResult);
    OS_printf("FSS2 Alpha: %d || Beta: %d || Capture Res: %u || Detection Res: %u\n", RetVal.FSS2AlphaAngle, RetVal.FSS2BetaAngle, RetVal.FSS2CaptureResult, RetVal.FSS2DetectionResult);
    OS_printf("FSS3 Alpha: %d || Beta: %d || Capture Res: %u || Detection Res: %u\n", RetVal.FSS3AlphaAngle, RetVal.FSS3BetaAngle, RetVal.FSS3CaptureResult, RetVal.FSS3DetectionResult);
    OS_printf("Valid Res: %s || %s || %s || %s", RetVal.FSS0ValidResult ? "true":"false",
                                                RetVal.FSS1ValidResult ? "true":"false",
                                            RetVal.FSS2ValidResult ? "true":"false",
                                        RetVal.FSS3ValidResult ? "true":"false");
    
    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetPowerStateCmd(void) {
    // ID 183
    CFE_Status_t               status;
    ADCS_PowerStateTlm_Payload_t RetVal = {0,};

    status = ADCS_GetPowerState(&RetVal);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Current Unix Time: 0x%08lx", (unsigned long)status);
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

CFE_Status_t ADCS_GetSatOrbitParamConfigCmd(void) {
    // ID 196
    CFE_Status_t               status;
    ADCS_SatOrbitParamConfigTlm_Payload_t RetVal = {0,};

    status = ADCS_GetSatOrbitParamConfig(&RetVal);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Current Unix Time: 0x%08lx", (unsigned long)status);
        return status;
    }
    
    // Handling Retval
    OS_printf("Epoch: %lf || Inclination: %lf || RAAN: %lf || Eccenctricity: %lf\n", RetVal.Epoch, RetVal.Inclination, RetVal.RAAN, RetVal.Eccentricity);
    OS_printf("AOP: %lf || Mean anomaly: %lf || Mean Motion: %lf || B_starDrag: %lf\n", RetVal.AOP, RetVal.MeanAnomaly, RetVal.MeanMotion, RetVal.B_StarDrag);
    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetRawCSSSensorCmd(void) {
    // ID 203
    CFE_Status_t               status;
    ADCS_RawCSSSensorTlm_Payload_t RetVal = {0,};

    status = ADCS_GetRawCSSSensor(&RetVal);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Current Unix Time: 0x%08lx", (unsigned long)status);
        return status;
    }
    
    // Handling Retval
    OS_printf("TimeSec : %u || TimeNanoSec : %u\n", RetVal.TimeSeconds, RetVal.TimeNanoSeconds);
    OS_printf("CSS 0 : %u, 1 : %u, 2 : %u, 3 : %u, 4 : %u, 5 : %u, 6 : %u, 7 : %u, 8 : %u, 9 : %u",
                RetVal.CSS0, RetVal.CSS1, RetVal.CSS2, RetVal.CSS3, RetVal.CSS4, RetVal.CSS5, RetVal.CSS6, RetVal.CSS7, RetVal.CSS8, RetVal.CSS9);
    
    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetRawGYRSensorCmd(void) {
    // ID 204
    CFE_Status_t               status;
    ADCS_RawGYRSensorTlm_Paylaod_t RetVal = {0,};

    status = ADCS_GetRawGYRSensor(&RetVal);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Current Unix Time: 0x%08lx", (unsigned long)status);
        return status;
    }
    
    // Handling Retval
    OS_printf("TimeSec : %u || TimeNanoSec : %u\n", RetVal.TimeSeconds, RetVal.TimeNanoSeconds);
    OS_printf("GYR0 RawRate X: %f || RawRate Y: %f || RawRate Z: %f\n", RetVal.GYR0RawRateX, RetVal.GYR0RawRateY, RetVal.GYR0RawRateZ);
    OS_printf("GYR1 RawRate X: %f || RawRate Y: %f || RawRate Z: %f\n", RetVal.GYR1RawRateX, RetVal.GYR1RawRateY, RetVal.GYR1RawRateZ);
    OS_printf("Valid Flag GYR0 : %s || GYR1 : %s", RetVal.GYR0ValidFlag ? "true":"false",
                                                RetVal.GYR1ValidFlag ? "true":"false");
    
    return CFE_SUCCESS;
}

CFE_Status_t ADCS_GetCalibratedGYRSensorCmd(void) {
    // ID 207
    CFE_Status_t               status;
    ADCS_CalibratedGYRSensorTlm_Payload_t RetVal = {0,};

    status = ADCS_GetCalibratedGYRSensor(&RetVal);

    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Adcs App: Fail to Set Current Unix Time: 0x%08lx", (unsigned long)status);
        return status;
    }
    
    // Handling Retval
    OS_printf("TimeSec : %u || TimeNanoSec : %u\n", RetVal.TimeSeconds, RetVal.TimeNanoSeconds);
    OS_printf("GYR0 Cal Rate X: %f || Cal Rate Y: %f || Cal Rate Z: %f\n", RetVal.GYR0CalibratedRateX, RetVal.GYR0CalibratedRateY, RetVal.GYR0CalibratedRateZ);
    OS_printf("GYR1 Cal Rate X: %f || Cal Rate Y: %f || Cal Rate Z: %f\n", RetVal.GYR1CalibratedRateX, RetVal.GYR1CalibratedRateY, RetVal.GYR1CalibratedRateZ);
    OS_printf("Ext GYR0 Cal Rate X: %f || Cal Rate Y: %f || Cal Rate Z: %f\n", RetVal.ExtGYR0CalibratedRateX, RetVal.ExtGYR0CalibratedRateY, RetVal.ExtGYR0CalibratedRateZ);
    OS_printf("Ext GYR1 Cal Rate X: %f || Cal Rate Y: %f || Cal Rate Z: %f\n", RetVal.ExtGYR1CalibratedRateX, RetVal.ExtGYR1CalibratedRateY, RetVal.ExtGYR1CalibratedRateZ);
    OS_printf("Valid flag GYR0 : %s || GYR1 : %s || ExtGYR0 : %s || ExtGYR1 : %s",
                RetVal.GYR0ValidFlag ? "true":"false",
                RetVal.GYR1ValidFlag ? "true":"false",
                RetVal.ExtGYR0ValidFlag ? "true":"false",
                RetVal.ExtGYR1ValidFlag ? "true":"false");    
    
    return CFE_SUCCESS;
}