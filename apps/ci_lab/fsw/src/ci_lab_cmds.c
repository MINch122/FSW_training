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
#include "ci_lab_version.h"

#include "cfe_rf_interface_cfg.h"

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
    CFE_ES_CreateChildTask(&CI_LAB_Global.ChildTaskId, "CI_TASK", CFE_RF_CommandIngestTask,
                            CFE_ES_TASK_STACK_ALLOCATE, 4096*2, 100, 0);
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
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(CI_LAB_Global.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(CI_LAB_Global.HkTlm.TelemetryHeader), true);
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
    return CFE_SUCCESS;
}


CFE_Status_t CI_UpdateContactTime(const CFE_RF_ContactTimeTlm_t *Msg) {
    uint8_t TimeSec[4];
    memcpy(TimeSec, Msg->TelemetryHeader.Sec.Time, sizeof(TimeSec));

    // OS_MutSemTake(CI_LAB_Global.MutexId);
    /* Convert the Time Stamp to Sec */
    CI_LAB_Global.LastContactTime.Seconds =
            CFE_PLATFORM_TBL_U32FROM4CHARS(TimeSec[0], TimeSec[1], TimeSec[2], TimeSec[3]);

    /* Store to file */
    CI_StoreContactTime();
    // OS_MutSemGive(CI_LAB_Global.MutexId);

    OS_printf("%s: Last Contact Time Sec: %u\n", __func__, CI_LAB_Global.LastContactTime.Seconds);

    return CFE_SUCCESS;
}

CFE_Status_t CI_CompareTime(void) {
    
    // OS_MutSemTake(CI_LAB_Global.MutexId);
    CFE_TIME_SysTime_t LastContactTime = CI_LAB_Global.LastContactTime;
    // OS_MutSemGive(CI_LAB_Global.MutexId);

    CFE_TIME_SysTime_t CurTime = CFE_TIME_GetTime();

    CFE_TIME_SysTime_t Result = CFE_TIME_Subtract(CurTime, LastContactTime);

    if (Result.Seconds > CFE_RF_MAX_MISSING_TIME) {
        /* If specified time is elapsed from last contact, */
        /* Do Emergency Protocol !! */
        /* 1. Deploy UANT - UANT has it's own MCU, so if deployed this command just ignored. */
        CI_UantArm(); /* Arm */
        OS_TaskDelay(10);
        CI_UantAutoDeploy(); /* Auto Deploy - Later, GS will check UANT deploy status */
        OS_TaskDelay(1000 * 5 * 4); /* 5 sec per Ant. Total 20 sec sleep */
        CI_UantDisArm(); /* Dis Arm */

        /* Send TO to dual emission */
        CI_SetEmissionMode(true);
    }
    else {
        /* If not, Send TO to Normal */
        CI_SetEmissionMode(false);
    }

    OS_printf("%s: Elapsed Time sec: %u\n", __func__, Result.Seconds);

    return CFE_SUCCESS;
}