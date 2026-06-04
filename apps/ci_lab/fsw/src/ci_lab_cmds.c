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
 *   This file contains the command handler functions for the Command Ingest task.
 */

/*
**   Include Files:
*/

#include "ci_lab_app.h"
#include "ci_lab_cmds.h"
#include "ci_lab_msgids.h"
#include "ci_lab_version.h"

#include "cfe_rf_interface_cfg.h"
#include "rpt_interface_cfg.h"

static void CI_LAB_SendCmdReport(const void *Cmd, int32 Status, const void *Data, size_t DataSize)
{
    CI_LAB_ReportTlm_t Report;
    CFE_SB_MsgId_t     CmdMid = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t  CmdCode = 0;
    size_t             CopySize = DataSize;

    memset(&Report, 0, sizeof(Report));

    CFE_MSG_GetMsgId(Cmd, &CmdMid);
    CFE_MSG_GetFcnCode(Cmd, &CmdCode);

    if (CopySize > sizeof(Report.Payload.ReturnValue))
    {
        CopySize = sizeof(Report.Payload.ReturnValue);
    }

    CFE_MSG_Init(CFE_MSG_PTR(Report.TelemetryHeader), CFE_SB_ValueToMsgId(CI_LAB_REPORT_TLM_MID), sizeof(Report));
    Report.Payload.MsgID          = (uint16_t)CFE_SB_MsgIdToValue(CmdMid);
    Report.Payload.CommandCode    = CmdCode;
    Report.Payload.ReturnType     = (Status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_APP;
    Report.Payload.ReturnCode     = Status;
    Report.Payload.ReturnDataSize = (uint16_t)CopySize;

    if (Data != NULL && CopySize > 0)
    {
        memcpy(Report.Payload.ReturnValue, Data, CopySize);
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(Report.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(Report.TelemetryHeader), true);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/*  Purpose:                                                                   */
/*     Handle NOOP command packets                                             */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t CI_LAB_NoopCmd(const CI_LAB_NoopCmd_t *cmd)
{
    /* Does everything the name implies */
    CI_LAB_Global.HkTlm.Payload.CommandCounter++;
    {
        uint16_t counters[2] = {
            CI_LAB_Global.HkTlm.Payload.CommandCounter,
            CI_LAB_Global.HkTlm.Payload.CommandErrorCounter
        };
        CI_LAB_SendCmdReport(cmd, CFE_SUCCESS, counters, sizeof(counters));
    }

    CFE_EVS_SendEvent(CI_LAB_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "CI: NOOP command. Version %d.%d.%d.%d",
                      CI_LAB_MAJOR_VERSION, CI_LAB_MINOR_VERSION, CI_LAB_REVISION, CI_LAB_MISSION_REV);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/*  Purpose:                                                                   */
/*     Handle ResetCounters command packets                                    */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t CI_LAB_ResetCountersCmd(const CI_LAB_ResetCountersCmd_t *cmd)
{
    CFE_EVS_SendEvent(CI_LAB_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "CI: RESET command");
    CI_LAB_ResetCounters_Internal();
    {
        uint16_t counters[2] = {
            CI_LAB_Global.HkTlm.Payload.CommandCounter,
            CI_LAB_Global.HkTlm.Payload.CommandErrorCounter
        };
        CI_LAB_SendCmdReport(cmd, CFE_SUCCESS, counters, sizeof(counters));
    }
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                             */
/*  Purpose:                                                                   */
/*     Create Child Task                                                       */
/*                                                                             */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t CI_LAB_CreateChildTaskCmd(const CI_LAB_CreateChildTaskCmd_t *cmd)
{
    CFE_Status_t Status;

    Status = CFE_ES_CreateChildTask(&CI_LAB_Global.ChildTaskId, "CI_TASK", CFE_RF_CommandIngestTask,
                                    CFE_ES_TASK_STACK_ALLOCATE, 4096*2, 100, 0);
    CI_LAB_SendCmdReport(cmd, Status, NULL, 0);
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the CI task  */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t CI_LAB_SendHkCmd(const CI_LAB_SendHkCmd_t *cmd)
{
    CI_LAB_Global.HkTlm.Payload.SocketConnected = CI_LAB_Global.SocketConnected;
    CI_LAB_SendCmdReport(cmd, CFE_SUCCESS, &CI_LAB_Global.HkTlm.Payload, sizeof(CI_LAB_Global.HkTlm.Payload));
    return CFE_SUCCESS;
}

CFE_Status_t CI_LAB_SendBcnCmd(const CI_LAB_SendHkCmd_t *cmd)
{
    /* Calculate the elapsed time sec */
    CFE_TIME_SysTime_t Time = CFE_TIME_GetTime();
    Time = CFE_TIME_Subtract(Time, CI_LAB_Global.LastContactTime);

    CI_LAB_Global.BcnTlm.Payload.ElapsedTimeSec = Time.Seconds;
                        
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(CI_LAB_Global.BcnTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(CI_LAB_Global.BcnTlm.TelemetryHeader), true);
    return CFE_SUCCESS;
}

CFE_Status_t CI_LAB_ReadUplinkCmd(const CI_LAB_ReadUplinkCmd_t *cmd)
{
    /* Any occurrence of this request will cause CI to read ONLY on this request thereafter */
    CI_LAB_Global.Scheduled = true;
    CI_LAB_ReadUpLink();
    CI_LAB_SendCmdReport(cmd, CFE_SUCCESS, &CI_LAB_Global.Scheduled, sizeof(CI_LAB_Global.Scheduled));
    return CFE_SUCCESS;
}
