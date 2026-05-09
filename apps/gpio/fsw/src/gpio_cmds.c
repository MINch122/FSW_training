/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Gpioace Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the gpioecific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 *   This file contains the source code for the Gpio Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "gpio.h"
#include "gpio_cmds.h"
#include "gpio_msgids.h"
#include "gpio_eventids.h"
#include "gpio_version.h"
#include "gpio_msg.h"
#include "cfe_srl.h"

static CFE_Status_t GPIO_SetOutput(const char *Name, CFE_SRL_GPIO_Indexer_t Index, bool Value)
{
    CFE_SRL_GPIO_Handle_t *Out = CFE_SRL_ApiGetGpioHandle(Index);
    int32                  Status;

    if (Out == NULL)
    {
        GPIO_Data.ErrCounter++;
        CFE_EVS_SendEvent(GPIO_CC_ERR_EID, CFE_EVS_EventType_ERROR, "GPIO: %s handle is NULL", Name);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    Status = CFE_SRL_ApiGpioSet(Out, Value);
    if (Status != CFE_SUCCESS)
    {
        GPIO_Data.ErrCounter++;
        CFE_EVS_SendEvent(GPIO_CC_ERR_EID, CFE_EVS_EventType_ERROR, "GPIO: failed to set %s %s, RC=0x%08lX", Name,
                          Value ? "ON" : "OFF", (unsigned long)Status);
        return Status;
    }

    GPIO_Data.CmdCounter++;
    CFE_EVS_SendEvent(GPIO_VALUE_INF_EID, CFE_EVS_EventType_INFORMATION, "GPIO: %s %s", Name,
                      Value ? "ON" : "OFF");

    return CFE_SUCCESS;
}

static CFE_Status_t GPIO_ReadInputFor5Seconds(const char *Name, CFE_SRL_GPIO_Indexer_t Index)
{
    CFE_SRL_GPIO_Handle_t *In = CFE_SRL_ApiGetGpioHandle(Index);
    bool                   Value = false;
    bool                   LastValue = false;
    bool                   SawHigh = false;
    int32                  Status = CFE_SUCCESS;
    int32                  ReadStatus;
    int                    Sample;

    if (In == NULL)
    {
        GPIO_Data.ErrCounter++;
        CFE_EVS_SendEvent(GPIO_CC_ERR_EID, CFE_EVS_EventType_ERROR, "GPIO: %s handle is NULL", Name);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    for (Sample = 0; Sample < 50; ++Sample)
    {
        ReadStatus = CFE_SRL_ApiGpioGet(In, &Value);
        if (ReadStatus != CFE_SUCCESS)
        {
            Status = ReadStatus;
            GPIO_Data.ErrCounter++;
            CFE_EVS_SendEvent(GPIO_CC_ERR_EID, CFE_EVS_EventType_ERROR, "GPIO: failed to read %s, RC=0x%08lX", Name,
                              (unsigned long)ReadStatus);
            break;
        }

        LastValue = Value;
        if (Value)
        {
            SawHigh = true;
        }

        OS_TaskDelay(100);
    }

    if (Status == CFE_SUCCESS)
    {
        GPIO_Data.CmdCounter++;
        CFE_EVS_SendEvent(GPIO_VALUE_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "GPIO: %s read for 5s, last=%u, saw_high=%u", Name, (unsigned int)LastValue,
                          (unsigned int)SawHigh);
    }

    return Status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in regpioonse to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t GPIO_SendHkCmd(const GPIO_SendHkCmd_t *Msg)
{
    /*
    ** Get command execution counters...
    */
    GPIO_Data.HkTlm.Payload.CommandErrorCounter = GPIO_Data.ErrCounter;
    GPIO_Data.HkTlm.Payload.CommandCounter      = GPIO_Data.CmdCounter;

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(GPIO_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(GPIO_Data.HkTlm.TelemetryHeader), true);

    /*
    ** Manage any pending table loads, validations, etc.
    */
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SAMPLE NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t GPIO_NoopCmd(const GPIO_NoopCmd_t *Msg)
{
    GPIO_Data.CmdCounter++;

    CFE_EVS_SendEvent(GPIO_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "GPIO: NOOP command %s",
                      GPIO_VERSION);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t GPIO_ResetCountersCmd(const GPIO_ResetCountersCmd_t *Msg)
{
    GPIO_Data.CmdCounter = 0;
    GPIO_Data.ErrCounter = 0;

    CFE_EVS_SendEvent(GPIO_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "GPIO: RESET command");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function Process Ground Station Command                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t GPIO_ProcessCmd(const GPIO_ProcessCmd_t *Msg)
{

    // /* Invoke a function provided by GPIO_LIB */

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* A simple example command that digpiolays a passed-in value                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t GPIO_DigpiolayParamCmd(const GPIO_DigpiolayParamCmd_t *Msg)
{
    CFE_EVS_SendEvent(GPIO_VALUE_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "GPIO: ValU32=%lu, ValI16=%d, ValStr=%s", (unsigned long)Msg->Payload.ValU32,
                      (int)Msg->Payload.ValI16, Msg->Payload.ValStr);

    return CFE_SUCCESS;
}

CFE_Status_t GPIO_LtrxEnOnCmd(const GPIO_LtrxEnOnCmd_t *Msg)
{
    return GPIO_SetOutput("LTRX_EN", CFE_SRL_LTRX_EN_GPIO_INDEXER, true);
}

CFE_Status_t GPIO_LtrxEnOffCmd(const GPIO_LtrxEnOffCmd_t *Msg)
{
    return GPIO_SetOutput("LTRX_EN", CFE_SRL_LTRX_EN_GPIO_INDEXER, false);
}

CFE_Status_t GPIO_Dep1EnOnCmd(const GPIO_Dep1EnOnCmd_t *Msg)
{
    return GPIO_SetOutput("DEP1_EN", CFE_SRL_DEP1_EN_GPIO_INDEXER, true);
}

CFE_Status_t GPIO_Dep1EnOffCmd(const GPIO_Dep1EnOffCmd_t *Msg)
{
    return GPIO_SetOutput("DEP1_EN", CFE_SRL_DEP1_EN_GPIO_INDEXER, false);
}

CFE_Status_t GPIO_Dep2EnOnCmd(const GPIO_Dep2EnOnCmd_t *Msg)
{
    return GPIO_SetOutput("DEP2_EN", CFE_SRL_DEP2_EN_GPIO_INDEXER, true);
}

CFE_Status_t GPIO_Dep2EnOffCmd(const GPIO_Dep2EnOffCmd_t *Msg)
{
    return GPIO_SetOutput("DEP2_EN", CFE_SRL_DEP2_EN_GPIO_INDEXER, false);
}

CFE_Status_t GPIO_SpInRead5sCmd(const GPIO_SpInRead5sCmd_t *Msg)
{
    return GPIO_ReadInputFor5Seconds("SP_IN", CFE_SRL_SP_IN_GPIO_INDEXER);
}
