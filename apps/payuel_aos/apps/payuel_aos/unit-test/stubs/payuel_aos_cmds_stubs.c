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
 * @file
 *
 * Auto-Generated stub implementations for functions defined in payuel_aos_cmds header
 */

#include "payuel_aos_cmds.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for PAYUEL_AOS_DisplayParamCmd()
 * ----------------------------------------------------
 */
CFE_Status_t PAYUEL_AOS_DisplayParamCmd(const PAYUEL_AOS_DisplayParamCmd_t *Msg)
{
    UT_GenStub_SetupReturnBuffer(PAYUEL_AOS_DisplayParamCmd, CFE_Status_t);

    UT_GenStub_AddParam(PAYUEL_AOS_DisplayParamCmd, const PAYUEL_AOS_DisplayParamCmd_t *, Msg);

    UT_GenStub_Execute(PAYUEL_AOS_DisplayParamCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(PAYUEL_AOS_DisplayParamCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for PAYUEL_AOS_NoopCmd()
 * ----------------------------------------------------
 */
CFE_Status_t PAYUEL_AOS_NoopCmd(const PAYUEL_AOS_NoopCmd_t *Msg)
{
    UT_GenStub_SetupReturnBuffer(PAYUEL_AOS_NoopCmd, CFE_Status_t);

    UT_GenStub_AddParam(PAYUEL_AOS_NoopCmd, const PAYUEL_AOS_NoopCmd_t *, Msg);

    UT_GenStub_Execute(PAYUEL_AOS_NoopCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(PAYUEL_AOS_NoopCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for PAYUEL_AOS_ProcessCmd()
 * ----------------------------------------------------
 */
CFE_Status_t PAYUEL_AOS_ProcessCmd(const PAYUEL_AOS_ProcessCmd_t *Msg)
{
    UT_GenStub_SetupReturnBuffer(PAYUEL_AOS_ProcessCmd, CFE_Status_t);

    UT_GenStub_AddParam(PAYUEL_AOS_ProcessCmd, const PAYUEL_AOS_ProcessCmd_t *, Msg);

    UT_GenStub_Execute(PAYUEL_AOS_ProcessCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(PAYUEL_AOS_ProcessCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for PAYUEL_AOS_ResetCountersCmd()
 * ----------------------------------------------------
 */
CFE_Status_t PAYUEL_AOS_ResetCountersCmd(const PAYUEL_AOS_ResetCountersCmd_t *Msg)
{
    UT_GenStub_SetupReturnBuffer(PAYUEL_AOS_ResetCountersCmd, CFE_Status_t);

    UT_GenStub_AddParam(PAYUEL_AOS_ResetCountersCmd, const PAYUEL_AOS_ResetCountersCmd_t *, Msg);

    UT_GenStub_Execute(PAYUEL_AOS_ResetCountersCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(PAYUEL_AOS_ResetCountersCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for PAYUEL_AOS_SendHkCmd()
 * ----------------------------------------------------
 */
CFE_Status_t PAYUEL_AOS_SendHkCmd(const PAYUEL_AOS_SendHkCmd_t *Msg)
{
    UT_GenStub_SetupReturnBuffer(PAYUEL_AOS_SendHkCmd, CFE_Status_t);

    UT_GenStub_AddParam(PAYUEL_AOS_SendHkCmd, const PAYUEL_AOS_SendHkCmd_t *, Msg);

    UT_GenStub_Execute(PAYUEL_AOS_SendHkCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(PAYUEL_AOS_SendHkCmd, CFE_Status_t);
}
