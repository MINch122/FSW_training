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

#include <string.h>

static void GPIO_SendReport(uint8 CC, int32 Status, const void *Data, uint16 DataSize, uint8 ReturnType)
{
    CFE_SB_Buffer_t *BufPtr = CFE_SB_AllocateMessageBuffer(sizeof(GPIO_ReportTlm_t));
    uint16 CopySize = 0;

    if (BufPtr == NULL)
    {
        return;
    }

    GPIO_ReportTlm_t *Report = (GPIO_ReportTlm_t *)BufPtr;
    if (CFE_MSG_Init(CFE_MSG_PTR(Report->TelemetryHeader), CFE_SB_ValueToMsgId(GPIO_RPT_TLM_MID),
                     sizeof(GPIO_ReportTlm_t)) != CFE_SUCCESS)
    {
        CFE_SB_ReleaseMessageBuffer(BufPtr);
        return;
    }

    if (Data != NULL && DataSize > 0)
    {
        CopySize = (DataSize > RPT_RET_VALUE_BUF_SIZE) ? RPT_RET_VALUE_BUF_SIZE : DataSize;
    }

    Report->Report.MsgID = GPIO_CMD_MID;
    Report->Report.CommandCode = CC;
    Report->Report.ReturnType = ReturnType;
    Report->Report.ReturnCode = Status;
    Report->Report.ReturnDataSize = CopySize;
    memset(Report->Report.ReturnValue, 0, sizeof(Report->Report.ReturnValue));
    if (CopySize > 0)
    {
        memcpy(Report->Report.ReturnValue, Data, CopySize);
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(Report->TelemetryHeader));
    if (CFE_SB_TransmitBuffer(BufPtr, true) != CFE_SUCCESS)
    {
        CFE_SB_ReleaseMessageBuffer(BufPtr);
    }
}

static uint8 GPIO_StatusToReportType(int32 Status)
{
    return (Status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_HW;
}

static CFE_Status_t GPIO_SetOutput(const char *Name, CFE_SRL_GPIO_Indexer_t Index, uint8 StateBit, bool Value)
{
    CFE_SRL_GPIO_Handle_t *Out = CFE_SRL_ApiGetGpioHandle(Index);
    int32                  Status;

    if (Value)
    {
        GPIO_Data.OutputStateBits |= (uint16)(1u << StateBit);
    }
    else
    {
        GPIO_Data.OutputStateBits &= (uint16)~(1u << StateBit);
    }
    GPIO_Data.OutputCommandedBits |= (uint16)(1u << StateBit);

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

void GPIO_InitOutputDefaults(void)
{
    CFE_SRL_GPIO_Handle_t *LtrxEn   = CFE_SRL_ApiGetGpioHandle(CFE_SRL_LTRX_EN_GPIO_INDEXER);
    CFE_SRL_GPIO_Handle_t *Dep1En   = CFE_SRL_ApiGetGpioHandle(CFE_SRL_DEP1_EN_GPIO_INDEXER);
    CFE_SRL_GPIO_Handle_t *Dep2En   = CFE_SRL_ApiGetGpioHandle(CFE_SRL_DEP2_EN_GPIO_INDEXER);
    CFE_SRL_GPIO_Handle_t *StxEn    = CFE_SRL_ApiGetGpioHandle(CFE_SRL_STX_EN_GPIO_INDEXER);
    CFE_SRL_GPIO_Handle_t *AdcsEn   = CFE_SRL_ApiGetGpioHandle(CFE_SRL_ADCS_EN_GPIO_INDEXER);
    CFE_SRL_GPIO_Handle_t *AdcsBoot = CFE_SRL_ApiGetGpioHandle(CFE_SRL_ADCS_BOOT_GPIO_INDEXER);

    GPIO_Data.OutputStateBits = (uint16)((1u << GPIO_OUTPUT_STX_EN_BIT) | (1u << GPIO_OUTPUT_ADCS_EN_BIT));
    GPIO_Data.OutputCommandedBits = 0;
    if (LtrxEn != NULL && CFE_SRL_ApiGpioSet(LtrxEn, false) != CFE_SUCCESS)
    {
        GPIO_Data.ErrCounter++;
        CFE_EVS_SendEvent(GPIO_CC_ERR_EID, CFE_EVS_EventType_ERROR, "GPIO: failed to set LTRX_EN default OFF");
    }
    if (Dep1En != NULL && CFE_SRL_ApiGpioSet(Dep1En, false) != CFE_SUCCESS)
    {
        GPIO_Data.ErrCounter++;
        CFE_EVS_SendEvent(GPIO_CC_ERR_EID, CFE_EVS_EventType_ERROR, "GPIO: failed to set DEP1_EN default OFF");
    }
    if (Dep2En != NULL && CFE_SRL_ApiGpioSet(Dep2En, false) != CFE_SUCCESS)
    {
        GPIO_Data.ErrCounter++;
        CFE_EVS_SendEvent(GPIO_CC_ERR_EID, CFE_EVS_EventType_ERROR, "GPIO: failed to set DEP2_EN default OFF");
    }
    if (AdcsBoot != NULL && CFE_SRL_ApiGpioSet(AdcsBoot, false) != CFE_SUCCESS)
    {
        GPIO_Data.ErrCounter++;
        CFE_EVS_SendEvent(GPIO_CC_ERR_EID, CFE_EVS_EventType_ERROR, "GPIO: failed to set ADCS_BOOT default OFF");
    }
    if (StxEn != NULL && CFE_SRL_ApiGpioSet(StxEn, true) != CFE_SUCCESS)
    {
        GPIO_Data.ErrCounter++;
        CFE_EVS_SendEvent(GPIO_CC_ERR_EID, CFE_EVS_EventType_ERROR, "GPIO: failed to set STX_EN default ON");
    }
    if (AdcsEn != NULL && CFE_SRL_ApiGpioSet(AdcsEn, true) != CFE_SUCCESS)
    {
        GPIO_Data.ErrCounter++;
        CFE_EVS_SendEvent(GPIO_CC_ERR_EID, CFE_EVS_EventType_ERROR, "GPIO: failed to set ADCS_EN default ON");
    }
}

static CFE_Status_t GPIO_ReadInputFor1Second(const char *Name, CFE_SRL_GPIO_Indexer_t Index, bool *SawHighOut, bool CountCommand)
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

    for (Sample = 0; Sample < 10; ++Sample)
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

    if (SawHighOut != NULL)
    {
        *SawHighOut = SawHigh;
    }

    if (Status == CFE_SUCCESS && CountCommand)
    {
        GPIO_Data.CmdCounter++;
        CFE_EVS_SendEvent(GPIO_VALUE_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "GPIO: %s read for 1s, last=%u, saw_high=%u", Name, (unsigned int)LastValue,
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

    GPIO_Data.HkTlm.Payload.GpioState[0] = (uint8)((GPIO_Data.OutputStateBits >> GPIO_OUTPUT_LTRX_EN_BIT) & 1u);
    GPIO_Data.HkTlm.Payload.GpioState[1] = (uint8)((GPIO_Data.OutputStateBits >> GPIO_OUTPUT_DEP1_EN_BIT) & 1u);
    GPIO_Data.HkTlm.Payload.GpioState[2] = (uint8)((GPIO_Data.OutputStateBits >> GPIO_OUTPUT_DEP2_EN_BIT) & 1u);
    GPIO_Data.HkTlm.Payload.GpioState[3] = (uint8)((GPIO_Data.OutputStateBits >> GPIO_OUTPUT_STX_EN_BIT) & 1u);
    GPIO_Data.HkTlm.Payload.GpioState[4] = (uint8)((GPIO_Data.OutputStateBits >> GPIO_OUTPUT_ADCS_EN_BIT) & 1u);
    GPIO_Data.HkTlm.Payload.GpioState[5] = (uint8)((GPIO_Data.OutputStateBits >> GPIO_OUTPUT_ADCS_BOOT_BIT) & 1u);

    /*
    ** Send housekeeping telemetry packet...
    */
    GPIO_APP_printf("GPIO: HK report requested\n");
    GPIO_SendReport(0, CFE_SUCCESS, &GPIO_Data.HkTlm.Payload, sizeof(GPIO_Data.HkTlm.Payload), RPT_RETTYPE_SUCCESS);

    /*
    ** Manage any pending table loads, validations, etc.
    */
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
CFE_Status_t GPIO_SendBcnCmd(const GPIO_SendBcnCmd_t *Msg)
{
    bool IsDeployed = false;

    GPIO_Data.BcnTlm.Payload.GpioState = (uint8)(GPIO_Data.OutputStateBits & GPIO_BCN_OUTPUT_STATE_MASK);
    GPIO_Data.BcnTlm.Payload.Padding = 0;
    GPIO_Data.BcnTlm.Payload.isDeployed = 0;

    if (GPIO_ReadInputFor1Second("SP_IN", CFE_SRL_SP_IN_GPIO_INDEXER, &IsDeployed, false) == CFE_SUCCESS)
    {
        GPIO_Data.BcnTlm.Payload.isDeployed = IsDeployed ? 1 : 0;
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(GPIO_Data.BcnTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(GPIO_Data.BcnTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

/* SAMPLE NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t GPIO_NoopCmd(const GPIO_NoopCmd_t *Msg)
{
    static const char NoopReport[] = "Yosi In Space";

    (void)Msg;
    GPIO_Data.CmdCounter++;

    CFE_EVS_SendEvent(GPIO_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "GPIO: NOOP command %s",
                      GPIO_VERSION);
    GPIO_SendReport(GPIO_NOOP_CC, CFE_SUCCESS, NoopReport, sizeof(NoopReport), RPT_RETTYPE_SUCCESS);

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
    uint8 Counters[2];

    (void)Msg;
    GPIO_Data.CmdCounter = 0;
    GPIO_Data.ErrCounter = 0;
    Counters[0] = GPIO_Data.CmdCounter;
    Counters[1] = GPIO_Data.ErrCounter;

    CFE_EVS_SendEvent(GPIO_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "GPIO: RESET command");
    GPIO_SendReport(GPIO_RESET_COUNTERS_CC, CFE_SUCCESS, Counters, sizeof(Counters), RPT_RETTYPE_SUCCESS);

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
    (void)Msg;
    GPIO_SendReport(GPIO_PROCESS_CC, CFE_SUCCESS, NULL, 0, RPT_RETTYPE_SUCCESS);

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
    GPIO_SendReport(GPIO_DIGPIOLAY_PARAM_CC, CFE_SUCCESS, &Msg->Payload, sizeof(Msg->Payload), RPT_RETTYPE_SUCCESS);

    return CFE_SUCCESS;
}

CFE_Status_t GPIO_LtrxEnOnCmd(const GPIO_LtrxEnOnCmd_t *Msg)
{
    CFE_Status_t Status = GPIO_SetOutput("LTRX_EN", CFE_SRL_LTRX_EN_GPIO_INDEXER, GPIO_OUTPUT_LTRX_EN_BIT, true);
    GPIO_SendReport(GPIO_LTRX_EN_ON_CC, Status, NULL, 0, GPIO_StatusToReportType(Status));
    return Status;
}

CFE_Status_t GPIO_LtrxEnOffCmd(const GPIO_LtrxEnOffCmd_t *Msg)
{
    CFE_Status_t Status = GPIO_SetOutput("LTRX_EN", CFE_SRL_LTRX_EN_GPIO_INDEXER, GPIO_OUTPUT_LTRX_EN_BIT, false);
    GPIO_SendReport(GPIO_LTRX_EN_OFF_CC, Status, NULL, 0, GPIO_StatusToReportType(Status));
    return Status;
}

CFE_Status_t GPIO_Dep1EnOnCmd(const GPIO_Dep1EnOnCmd_t *Msg)
{
    CFE_Status_t Status = GPIO_SetOutput("DEP1_EN", CFE_SRL_DEP1_EN_GPIO_INDEXER, GPIO_OUTPUT_DEP1_EN_BIT, true);
    GPIO_SendReport(GPIO_DEP1_EN_ON_CC, Status, NULL, 0, GPIO_StatusToReportType(Status));
    return Status;
}

CFE_Status_t GPIO_Dep1EnOffCmd(const GPIO_Dep1EnOffCmd_t *Msg)
{
    CFE_Status_t Status = GPIO_SetOutput("DEP1_EN", CFE_SRL_DEP1_EN_GPIO_INDEXER, GPIO_OUTPUT_DEP1_EN_BIT, false);
    GPIO_SendReport(GPIO_DEP1_EN_OFF_CC, Status, NULL, 0, GPIO_StatusToReportType(Status));
    return Status;
}

CFE_Status_t GPIO_Dep2EnOnCmd(const GPIO_Dep2EnOnCmd_t *Msg)
{
    CFE_Status_t Status = GPIO_SetOutput("DEP2_EN", CFE_SRL_DEP2_EN_GPIO_INDEXER, GPIO_OUTPUT_DEP2_EN_BIT, true);
    GPIO_SendReport(GPIO_DEP2_EN_ON_CC, Status, NULL, 0, GPIO_StatusToReportType(Status));
    return Status;
}

CFE_Status_t GPIO_Dep2EnOffCmd(const GPIO_Dep2EnOffCmd_t *Msg)
{
    CFE_Status_t Status = GPIO_SetOutput("DEP2_EN", CFE_SRL_DEP2_EN_GPIO_INDEXER, GPIO_OUTPUT_DEP2_EN_BIT, false);
    GPIO_SendReport(GPIO_DEP2_EN_OFF_CC, Status, NULL, 0, GPIO_StatusToReportType(Status));
    return Status;
}

CFE_Status_t GPIO_Dep1Dep2En90sCmd(const GPIO_Dep1Dep2En90sCmd_t *Msg)
{
    int32                     Statuses[2];
    int32                     FinalStatus;
    const char               *Name;
    CFE_SRL_GPIO_Indexer_t    Index;
    uint8                     StateBit;
    uint8                     Channel = Msg->Payload.Channel;

    if (Channel == 1)
    {
        Name     = "DEP1_EN";
        Index    = CFE_SRL_DEP1_EN_GPIO_INDEXER;
        StateBit = GPIO_OUTPUT_DEP1_EN_BIT;
    }
    else if (Channel == 2)
    {
        Name     = "DEP2_EN";
        Index    = CFE_SRL_DEP2_EN_GPIO_INDEXER;
        StateBit = GPIO_OUTPUT_DEP2_EN_BIT;
    }
    else
    {
        GPIO_Data.ErrCounter++;
        CFE_EVS_SendEvent(GPIO_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "GPIO: invalid DEP burn channel %u, expected 1 or 2", (unsigned int)Channel);
        FinalStatus = CFE_STATUS_BAD_COMMAND_CODE;
        GPIO_SendReport(GPIO_DEP1_DEP2_EN_90S_CC, FinalStatus, &Channel, sizeof(Channel),
                        GPIO_StatusToReportType(FinalStatus));
        return FinalStatus;
    }

    CFE_EVS_SendEvent(GPIO_VALUE_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "GPIO: %s burn ON for 90s sequence started", Name);

    Statuses[0] = GPIO_SetOutput(Name, Index, StateBit, true);
    FinalStatus = Statuses[0];

    if (Statuses[0] == CFE_SUCCESS)
    {
        OS_TaskDelay(90000);
    }

    Statuses[1] = GPIO_SetOutput(Name, Index, StateBit, false);
    if (Statuses[1] != CFE_SUCCESS && FinalStatus == CFE_SUCCESS)
    {
        FinalStatus = Statuses[1];
    }

    CFE_EVS_SendEvent(GPIO_VALUE_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "GPIO: %s burn 90s sequence finished", Name);
    GPIO_SendReport(GPIO_DEP1_DEP2_EN_90S_CC, FinalStatus, Statuses, sizeof(Statuses),
                    GPIO_StatusToReportType(FinalStatus));

    return FinalStatus;
}

CFE_Status_t GPIO_StxEnOnCmd(const GPIO_StxEnOnCmd_t *Msg)
{
    CFE_Status_t Status = GPIO_SetOutput("STX_EN", CFE_SRL_STX_EN_GPIO_INDEXER, GPIO_OUTPUT_STX_EN_BIT, true);
    GPIO_SendReport(GPIO_STX_EN_ON_CC, Status, NULL, 0, GPIO_StatusToReportType(Status));
    return Status;
}

CFE_Status_t GPIO_StxEnOffCmd(const GPIO_StxEnOffCmd_t *Msg)
{
    CFE_Status_t Status = GPIO_SetOutput("STX_EN", CFE_SRL_STX_EN_GPIO_INDEXER, GPIO_OUTPUT_STX_EN_BIT, false);
    GPIO_SendReport(GPIO_STX_EN_OFF_CC, Status, NULL, 0, GPIO_StatusToReportType(Status));
    return Status;
}

CFE_Status_t GPIO_AdcsEnOnCmd(const GPIO_AdcsEnOnCmd_t *Msg)
{
    CFE_Status_t Status = GPIO_SetOutput("ADCS_EN", CFE_SRL_ADCS_EN_GPIO_INDEXER, GPIO_OUTPUT_ADCS_EN_BIT, true);
    GPIO_SendReport(GPIO_ADCS_EN_ON_CC, Status, NULL, 0, GPIO_StatusToReportType(Status));
    return Status;
}

CFE_Status_t GPIO_AdcsEnOffCmd(const GPIO_AdcsEnOffCmd_t *Msg)
{
    CFE_Status_t Status = GPIO_SetOutput("ADCS_EN", CFE_SRL_ADCS_EN_GPIO_INDEXER, GPIO_OUTPUT_ADCS_EN_BIT, false);
    GPIO_SendReport(GPIO_ADCS_EN_OFF_CC, Status, NULL, 0, GPIO_StatusToReportType(Status));
    return Status;
}

CFE_Status_t GPIO_AdcsBootOnCmd(const GPIO_AdcsBootOnCmd_t *Msg)
{
    CFE_Status_t Status = GPIO_SetOutput("ADCS_BOOT", CFE_SRL_ADCS_BOOT_GPIO_INDEXER, GPIO_OUTPUT_ADCS_BOOT_BIT, true);
    GPIO_SendReport(GPIO_ADCS_BOOT_ON_CC, Status, NULL, 0, GPIO_StatusToReportType(Status));
    return Status;
}

CFE_Status_t GPIO_AdcsBootOffCmd(const GPIO_AdcsBootOffCmd_t *Msg)
{
    CFE_Status_t Status = GPIO_SetOutput("ADCS_BOOT", CFE_SRL_ADCS_BOOT_GPIO_INDEXER, GPIO_OUTPUT_ADCS_BOOT_BIT, false);
    GPIO_SendReport(GPIO_ADCS_BOOT_OFF_CC, Status, NULL, 0, GPIO_StatusToReportType(Status));
    return Status;
}

CFE_Status_t GPIO_SpInRead5sCmd(const GPIO_SpInRead5sCmd_t *Msg)
{
    CFE_Status_t Status = GPIO_ReadInputFor1Second("SP_IN", CFE_SRL_SP_IN_GPIO_INDEXER, NULL, true);
    GPIO_SendReport(GPIO_SP_IN_READ_5S_CC, Status, NULL, 0, GPIO_StatusToReportType(Status));
    return Status;
}
