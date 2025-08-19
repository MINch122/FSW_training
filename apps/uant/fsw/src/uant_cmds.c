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
#include "uant.h"
#include "uant_app.h"
#include "uant_cmds.h"
#include "uant_msgids.h"
#include "uant_eventids.h"
#include "uant_msg.h"

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
CFE_Status_t UANT_SendHkCmd(const UANT_SendHkCmd_t *Msg) // Msg는 트리거일 뿐
{   
    /*
    ** Get command execution counters...
    */
    CFE_Status_t status;

    status=ISIS_UANT_ReportDeploymentStatus(&UANT_Data.HkTlm.Payload.deploystatus); 
    
    
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(UANT_GET_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                        "UANT: Failed to get Hk data, status = 0x%08X", status);
        UANT_Data.ErrCounter++;
    }


    for (uint8_t ant = 1; ant <= 4; ant++)
    {
        switch (ant)
        {
            case 1:
                status=ISIS_UANT_ReportAntennaActivationCount(1, &UANT_Data.HkTlm.Payload.ant1actvcnt);
                if (status != CFE_SUCCESS)
                {
                    CFE_EVS_SendEvent(UANT_GET_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                                    "UANT: Failed to get Hk data, status = 0x%08X", status);
                    UANT_Data.ErrCounter++;
                }
                break;
            case 2:
                status=ISIS_UANT_ReportAntennaActivationCount(2, &UANT_Data.HkTlm.Payload.ant2actvcnt);
                if (status != CFE_SUCCESS)
                {
                    CFE_EVS_SendEvent(UANT_GET_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                                    "UANT: Failed to get Hk data, status = 0x%08X", status);
                    UANT_Data.ErrCounter++;
                }
                break;
            case 3:
                status=ISIS_UANT_ReportAntennaActivationCount(3, &UANT_Data.HkTlm.Payload.ant3actvcnt);
                if (status != CFE_SUCCESS)
                {
                    CFE_EVS_SendEvent(UANT_GET_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                                    "UANT: Failed to get Hk data, status = 0x%08X", status);
                    UANT_Data.ErrCounter++;
                }
                break;
            case 4:
                status=ISIS_UANT_ReportAntennaActivationCount(4, &UANT_Data.HkTlm.Payload.ant4actvcnt);
                
                break;
        }
    }


    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(UANT_GET_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                        "UANT: Failed to get Hk data, status = 0x%08X", status);
        UANT_Data.ErrCounter++;
    }
    else
    {
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_Data.HkTlm.TelemetryHeader));
        CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_Data.HkTlm.TelemetryHeader), true);
        
        //OS_printf("%x", UANT_Data.HkTlm.deploystatus);
    }

    return CFE_SUCCESS;
}

CFE_Status_t UANT_SendBcnCmd(const UANT_SendBcnCmd_t *Msg) 
{
    CFE_Status_t status;

    status = ISIS_UANT_ReportDeploymentStatus(&UANT_Data.bcn.Payload.deploystatus); 

    
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(UANT_GET_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                        "UANT: Failed to get beacon data, status = 0x%08X", status);
        UANT_Data.ErrCounter++;
    }
    else
    {
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_Data.bcn.TelemetryHeader));
        CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_Data.bcn.TelemetryHeader), true);
        UANT_Data.CmdCounter++;
        OS_printf("0x%04X\n", UANT_Data.bcn.Payload.deploystatus);
    }

    
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* UANT NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t UANT_NoopCmd(const UANT_NoopCmd_t *Msg)
{
    UANT_Data.CmdCounter++;

    CFE_EVS_SendEvent(UANT_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "UANT: NOOP command received.");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t UANT_ResetCountersCmd(const UANT_ResetCountersCmd_t *Msg)
{
    UANT_Data.CmdCounter = 0;
    UANT_Data.ErrCounter = 0;

    CFE_EVS_SendEvent(UANT_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "UANT: RESET command");

    return CFE_SUCCESS;
}


CFE_Status_t UANT_Reset(const UANT_ISIS_ResetCmd_t *Msg)
{
    CFE_Status_t     status;
    status = ISIS_UANT_Reset();

    /* RPT 데이터 구성 */
    RPT_Report_t report = (RPT_Report_t){0};
    report.MsgID        = UANT_CMD_MID;
    report.CommandCode  = UANT_RESET_CC;
    report.ReturnType   = (status == CFE_SUCCESS)
                          ? RPT_RETTYPE_SUCCESS
                          : RPT_RETTYPE_CFE;
    report.ReturnCode   = status;
    report.ReturnDataSize = 0; /* 반환 데이터 없음 */

    /* 전역 RPT 패킷 전송 */
    UANT_Data.rpt.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader));
    (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader), true);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(UANT_RESET_ERR_EID, CFE_EVS_EventType_ERROR,
                        "UANT: Reset command failed, status = 0x%08X", status);
        UANT_Data.ErrCounter++;
    }
    else
    {
        UANT_Data.CmdCounter++;
    }
    return status;
}


CFE_Status_t UANT_Arm(const UANT_ISIS_ArmAntennaSystemsCmd_t *Msg)
{
    CFE_Status_t status;

    status = ISIS_UANT_ArmAntennaSystems();

    /* RPT 데이터 구성 */
    RPT_Report_t report = (RPT_Report_t){0};
    report.MsgID        = UANT_CMD_MID;
    report.CommandCode  = UANT_ARM_ANTENNA_SYSTEMS_CC;
    report.ReturnType   = (status == CFE_SUCCESS)
                          ? RPT_RETTYPE_SUCCESS
                          : RPT_RETTYPE_CFE;
    report.ReturnCode   = status;
    report.ReturnDataSize = 0; /* 반환 데이터 없음 */

    /* 전역 RPT 패킷 전송 */
    UANT_Data.rpt.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader));
    (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(UANT_ARM_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UANT: ArmAntennaSystems failed, status = 0x%08X",
                          status);
        UANT_Data.ErrCounter++;
    }
    else
    {
        UANT_Data.CmdCounter++;
    }

    return status;
}

CFE_Status_t UANT_Disarm(const UANT_ISIS_DisarmCmd_t *Msg)
{
    CFE_Status_t status;

    status = ISIS_UANT_Disarm();

    /* RPT 데이터 구성 */
    RPT_Report_t report = (RPT_Report_t){0};
    report.MsgID        = UANT_CMD_MID;
    report.CommandCode  = UANT_DISARM_CC;
    report.ReturnType   = (status == CFE_SUCCESS)
                          ? RPT_RETTYPE_SUCCESS
                          : RPT_RETTYPE_CFE;
    report.ReturnCode   = status;
    report.ReturnDataSize = 0; /* 반환 데이터 없음 */

    /* 전역 RPT 패킷 전송 */
    UANT_Data.rpt.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader));
    (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(UANT_DISARM_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UANT: Disarm failed, status = 0x%08X",
                          status);
        UANT_Data.ErrCounter++;
    }
    else
    {
        UANT_Data.CmdCounter++;
    }

    return status;
}

CFE_Status_t UANT_AutomatedDeployment(const UANT_ISIS_AutomatedDeploymentCmd_t *Msg)
{
    CFE_Status_t status;

    status = ISIS_UANT_AutomatedSequentialDeployment(Msg->Arg);

    /* RPT 결과 보고 (데이터 없음) */
    {
        RPT_Report_t report = (RPT_Report_t){0};
        report.MsgID         = UANT_CMD_MID;
        report.CommandCode   = UANT_AUTOMATED_DEPLOYMENT_CC;
        report.ReturnType    = (status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
        report.ReturnCode    = status;
        report.ReturnDataSize = 0;

        UANT_Data.rpt.Payload = report;
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader));
        (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader), true);
    }

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(UANT_AUTO_DEPLOY_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UANT: AutomatedSequentialDeployment failed, status = 0x%08X",
                          (unsigned)status);
        UANT_Data.ErrCounter++;
    }
    else
    {
        UANT_Data.CmdCounter++;
    }

    return status;
}


CFE_Status_t UANT_DeployAnt1(const UANT_ISIS_DeployAnt1Cmd_t *Msg)
{
    CFE_Status_t status;

    status = ISIS_UANT_DeployAntenna1(Msg->Arg);

    /* RPT 결과 보고 (데이터 없음) */
    {
        RPT_Report_t report = (RPT_Report_t){0};
        report.MsgID         = UANT_CMD_MID;
        report.CommandCode   = UANT_DEPLOY_ANT1_CC;
        report.ReturnType    = (status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
        report.ReturnCode    = status;
        report.ReturnDataSize = 0;

        UANT_Data.rpt.Payload = report;
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader));
        (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader), true);
    }

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(UANT_DEPLOY_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UANT: DeployAntenna1 failed, status = 0x%08X",
                          (unsigned)status);
        UANT_Data.ErrCounter++;
    }
    else
    {
        UANT_Data.CmdCounter++;
    }

    return status;
}




CFE_Status_t UANT_DeployAnt2(const UANT_ISIS_DeployAnt2Cmd_t *Msg)
{
    CFE_Status_t status;

    status = ISIS_UANT_DeployAntenna2(Msg->Arg);

    /* RPT 결과 보고 (데이터 없음) */
    {
        RPT_Report_t report = (RPT_Report_t){0};
        report.MsgID         = UANT_CMD_MID;
        report.CommandCode   = UANT_DEPLOY_ANT2_CC;
        report.ReturnType    = (status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
        report.ReturnCode    = status;
        report.ReturnDataSize = 0;

        UANT_Data.rpt.Payload = report;
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader));
        (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader), true);
    }

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(UANT_DEPLOY_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UANT: DeployAntenna2 failed, status = 0x%08X",
                          (unsigned)status);
        UANT_Data.ErrCounter++;
    }
    else
    {
        UANT_Data.CmdCounter++;
    }

    return status;
}

CFE_Status_t UANT_DeployAnt3(const UANT_ISIS_DeployAnt3Cmd_t *Msg)
{
    CFE_Status_t status;

    status = ISIS_UANT_DeployAntenna3(Msg->Arg);

    /* RPT 결과 보고 (데이터 없음) */
    {
        RPT_Report_t report = (RPT_Report_t){0};
        report.MsgID         = UANT_CMD_MID;
        report.CommandCode   = UANT_DEPLOY_ANT3_CC;
        report.ReturnType    = (status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
        report.ReturnCode    = status;
        report.ReturnDataSize = 0;

        UANT_Data.rpt.Payload = report;
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader));
        (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader), true);
    }

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(UANT_DEPLOY_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UANT: DeployAntenna3 failed, status = 0x%08X",
                          (unsigned)status);
        UANT_Data.ErrCounter++;
    }
    else
    {
        UANT_Data.CmdCounter++;
    }

    return status;
}

CFE_Status_t UANT_DeployAnt4(const UANT_ISIS_DeployAnt4Cmd_t *Msg)
{
    CFE_Status_t status;

    status = ISIS_UANT_DeployAntenna4(Msg->Arg);

    /* RPT 결과 보고 (데이터 없음) */
    {
        RPT_Report_t report = (RPT_Report_t){0};
        report.MsgID         = UANT_CMD_MID;
        report.CommandCode   = UANT_DEPLOY_ANT4_CC;
        report.ReturnType    = (status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
        report.ReturnCode    = status;
        report.ReturnDataSize = 0;

        UANT_Data.rpt.Payload = report;
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader));
        (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader), true);
    }

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(UANT_DEPLOY_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UANT: DeployAntenna4 failed, status = 0x%08X",
                          (unsigned)status);
        UANT_Data.ErrCounter++;
    }
    else
    {
        UANT_Data.CmdCounter++;
    }

    return status;
}

CFE_Status_t UANT_DeployAnt1_Override(const UANT_ISIS_DeployAnt1OverrideCmd_t *Msg)
{
    CFE_Status_t status;

    status = ISIS_UANT_DeployAntenna1WithOverride(Msg->Arg);

    /* RPT 결과 보고 (데이터 없음) */
    {
        RPT_Report_t report = (RPT_Report_t){0};
        report.MsgID         = UANT_CMD_MID;
        report.CommandCode   = UANT_DEPLOY_ANT1_OVERRIDE_CC;
        report.ReturnType    = (status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
        report.ReturnCode    = status;
        report.ReturnDataSize = 0;

        UANT_Data.rpt.Payload = report;
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader));
        (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader), true);
    }

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(UANT_DEPLOY_OVRD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UANT: DeployAntenna1WithOverride failed, status = 0x%08X", (unsigned)status);
        UANT_Data.ErrCounter++;
    }
    else
    {
        UANT_Data.CmdCounter++;
    }

    return status;
}

CFE_Status_t UANT_DeployAnt2_Override(const UANT_ISIS_DeployAnt2OverrideCmd_t *Msg)
{
    CFE_Status_t status;

    status = ISIS_UANT_DeployAntenna2WithOverride(Msg->Arg);

    /* RPT 결과 보고 (데이터 없음) */
    {
        RPT_Report_t report = (RPT_Report_t){0};
        report.MsgID         = UANT_CMD_MID;
        report.CommandCode   = UANT_DEPLOY_ANT2_OVERRIDE_CC;
        report.ReturnType    = (status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
        report.ReturnCode    = status;
        report.ReturnDataSize = 0;

        UANT_Data.rpt.Payload = report;
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader));
        (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader), true);
    }

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(UANT_DEPLOY_OVRD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UANT: DeployAntenna2WithOverride failed, status = 0x%08X", (unsigned)status);
        UANT_Data.ErrCounter++;
    }
    else
    {
        UANT_Data.CmdCounter++;
    }

    return status;
}

CFE_Status_t UANT_DeployAnt3_Override(const UANT_ISIS_DeployAnt3OverrideCmd_t *Msg)
{
    CFE_Status_t status;

    status = ISIS_UANT_DeployAntenna3WithOverride(Msg->Arg);

    /* RPT 결과 보고 (데이터 없음) */
    {
        RPT_Report_t report = (RPT_Report_t){0};
        report.MsgID         = UANT_CMD_MID;
        report.CommandCode   = UANT_DEPLOY_ANT3_OVERRIDE_CC;
        report.ReturnType    = (status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
        report.ReturnCode    = status;
        report.ReturnDataSize = 0;

        UANT_Data.rpt.Payload = report;
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader));
        (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader), true);
    }

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(UANT_DEPLOY_OVRD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UANT: DeployAntenna3WithOverride failed, status = 0x%08X", (unsigned)status);
        UANT_Data.ErrCounter++;
    }
    else
    {
        UANT_Data.CmdCounter++;
    }

    return status;
}

CFE_Status_t UANT_DeployAnt4_Override(const UANT_ISIS_DeployAnt4OverrideCmd_t *Msg)
{
    CFE_Status_t status;

    status = ISIS_UANT_DeployAntenna4WithOverride(Msg->Arg);

    /* RPT 결과 보고 (데이터 없음) */
    {
        RPT_Report_t report = (RPT_Report_t){0};
        report.MsgID         = UANT_CMD_MID;
        report.CommandCode   = UANT_DEPLOY_ANT4_OVERRIDE_CC;
        report.ReturnType    = (status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
        report.ReturnCode    = status;
        report.ReturnDataSize = 0;

        UANT_Data.rpt.Payload = report;
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader));
        (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader), true);
    }

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(UANT_DEPLOY_OVRD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UANT: DeployAntenna4WithOverride failed, status = 0x%08X", (unsigned)status);
        UANT_Data.ErrCounter++;
    }
    else
    {
        UANT_Data.CmdCounter++;
    }

    return status;
}


CFE_Status_t UANT_CancleDeployment(const UANT_ISIS_CancelDeploymentActivationCmd_t *Msg)
{
    CFE_Status_t status;

    status = ISIS_UANT_CancelDeploymentSystemActivation();

    /* RPT 결과 보고 (데이터 없음) */
    {
        RPT_Report_t report = (RPT_Report_t){0};
        report.MsgID         = UANT_CMD_MID;
        report.CommandCode   = UANT_CANCEL_DEPLOYMENT_ACTIVATION_CC;
        report.ReturnType    = (status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
        report.ReturnCode    = status;
        report.ReturnDataSize = 0;

        UANT_Data.rpt.Payload = report;
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader));
        (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader), true);
    }

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(UANT_DEPLOY_CANCEL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UANT: CancelDeploymentSystemActivation failed, status = 0x%08X", (unsigned)status);
        UANT_Data.ErrCounter++;
    }
    else
    {
        UANT_Data.CmdCounter++;
    }

    return status;
}




CFE_Status_t UANT_GetDeploymentStatus(const UANT_ISIS_ReportDeploymentStatusCmd_t *Msg)
{
    CFE_Status_t status;
    uint16       deploy_status;

    status = ISIS_UANT_ReportDeploymentStatus(&deploy_status);

    /* RPT 데이터 구성 */
    RPT_Report_t report = (RPT_Report_t){0};
    report.MsgID        = UANT_CMD_MID;
    report.CommandCode  = UANT_GET_DEPLOYMENT_STATUS_CC; /* CC 매크로명 맞춤 */
    report.ReturnType   = (status == CFE_SUCCESS)
                          ? RPT_RETTYPE_SUCCESS
                          : RPT_RETTYPE_CFE;
    report.ReturnCode   = status;

    report.ReturnDataSize = sizeof(deploy_status);
    memcpy(report.ReturnValue, &deploy_status, report.ReturnDataSize);

    
    UANT_Data.rpt.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader));
    (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(UANT_GET_STATUS_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "UANT: Failed to get deployment status, hw_status=0x%08X",
                          status);
        UANT_Data.ErrCounter++;
    }
    else
    {
        UANT_Data.CmdCounter++;
        
    }

    return status;
}



CFE_Status_t UANT_MeasureAntSystemTemperature(const UANT_ISIS_MeasureSystemTemperatureCmd_t *Msg)
{
    CFE_Status_t status;
    uint16 raw;

    status = ISIS_UANT_MeasureAntennaSystemTemperature(&raw);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(UANT_MEASURE_TEMP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UANT: Failed to measure temperature, status = 0x%08X", (unsigned)status);
        UANT_Data.ErrCounter++;
    }
    else
    {
        RPT_Report_t report = (RPT_Report_t){0};
        report.MsgID        = UANT_CMD_MID;               
        report.CommandCode  = UANT_MEASURE_SYSTEM_TEMPERATURE_CC; 
        report.ReturnType   = (status == CFE_SUCCESS)
                              ? RPT_RETTYPE_SUCCESS
                              : RPT_RETTYPE_CFE;                 
        report.ReturnCode   = status;

        report.ReturnDataSize = sizeof(raw);
        memcpy(report.ReturnValue, &raw, report.ReturnDataSize);

        UANT_Data.rpt.Payload = report;
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader));
        (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader), true);

        UANT_Data.CmdCounter++;
    }

    return status;
}


CFE_Status_t UANT_ReportAntActivationCnt(const UANT_ISIS_ReportAntActivationCntCmd_t *Msg)
{
    CFE_Status_t status;

    uint8 count;     
    uint8 ant = Msg->Arg; 
    status = ISIS_UANT_ReportAntennaActivationCount(ant, &count);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(UANT_GET_ACT_CNT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UANT: Failed to report activation count for ANT-%d, status = 0x%08X",
                          ant, (unsigned)status);
        UANT_Data.ErrCounter++;
    }
    else
    {
        RPT_Report_t report = (RPT_Report_t){0};
        report.MsgID        = UANT_CMD_MID;
        report.CommandCode  = UANT_REPORT_ANT_ACTIVATION_CNT_CC;
        report.ReturnType   = (status == CFE_SUCCESS)
                              ? RPT_RETTYPE_SUCCESS
                              : RPT_RETTYPE_CFE; 
        report.ReturnCode   = status;

        report.ReturnDataSize = sizeof(count);
        memcpy(report.ReturnValue, &count, report.ReturnDataSize);

        
        UANT_Data.rpt.Payload = report;
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader));
        (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader), true);

        UANT_Data.CmdCounter++;
    }

    return status;
}


CFE_Status_t UANT_ReportAntActivationTime(const UANT_ISIS_ReportAntActivationTimeCmd_t *Msg)
{
    CFE_Status_t status;
    uint16 time;
    uint8  ant = Msg->Arg;

    status = ISIS_UANT_ReportAntennaActivationTime(ant, &time);
    if (status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(UANT_GET_ACT_TIME_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UANT: Failed to report activation time for ANT-%d, status = 0x%08X",
                          ant, (unsigned)status);
        UANT_Data.ErrCounter++;
    } else {
        RPT_Report_t report = (RPT_Report_t){0};
        report.MsgID        = UANT_CMD_MID;
        report.CommandCode  = UANT_REPORT_ANT_ACTIVATION_TIME_CC;
        report.ReturnType   = (status == CFE_SUCCESS)
                             ? RPT_RETTYPE_SUCCESS
                             : RPT_RETTYPE_CFE;
        report.ReturnCode   = status;
        report.ReturnDataSize = sizeof(time);
        memcpy(report.ReturnValue, &time, report.ReturnDataSize);

        
        UANT_Data.rpt.Payload = report;
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader));
        (void)CFE_SB_TransmitMsg(CFE_MSG_PTR(UANT_Data.rpt.TelemetryHeader), true);

        UANT_Data.CmdCounter++;
    }
    return status;
}
