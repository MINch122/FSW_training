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
 *   This file contains the source code for the EPS App Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "eps_app.h"
#include "eps_cmds.h"
#include "eps_msgids.h"
#include "eps_eventids.h"
#include "eps_msg.h"

#include "eps_device_p31u.h"
#include "p31u.h"


CFE_Status_t EPS_SendHkCmd(const EPS_SendHkCmd_t *Msg)
{
    /**
     * This command is supposed to be called by the scheduler and
     * should not increment the command counter.
     */

    EPS_P31U_GetDeviceHkData(&EPS_AppData.HkTlm.Payload);

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(EPS_AppData.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(EPS_AppData.HkTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

CFE_Status_t EPS_SendBcnCmd(const EPS_SendBcnCmd_t *Msg)
{
    /**
     * This command is supposed to be called by the scheduler and
     * should not increment the command counter.
     */

    EPS_P31U_GetDeviceBcnData(&EPS_AppData.BcnTlm.Payload);

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(EPS_AppData.BcnTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(EPS_AppData.BcnTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

CFE_Status_t EPS_ReportAppDataCmd(const EPS_ReportAppDataCmd_t *Msg)
{


    CFE_SB_TransmitMsg(CFE_MSG_PTR(EPS_AppData.Report.TelemetryHeader), true);
    return CFE_SUCCESS;
}

CFE_Status_t EPS_NoopCmd(const EPS_NoopCmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter++;


    CFE_EVS_SendEvent(EPS_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "EPS: NOOP command received.");

    return CFE_SUCCESS;
}

CFE_Status_t EPS_ResetCountersCmd(const EPS_ResetCountersCmd_t *Msg)
{
    EPS_AppData.Counters.CmdCounter = 0;
    EPS_AppData.Counters.ErrCounter = 0;
    EPS_AppData.Counters.GetHkErrCounter = 0;
    EPS_AppData.Counters.GetBcnErrCounter = 0;

    CFE_EVS_SendEvent(EPS_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "EPS: RESET command");

    return CFE_SUCCESS;
}
