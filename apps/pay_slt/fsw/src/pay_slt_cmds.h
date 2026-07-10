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

#ifndef PAY_SLT_CMDS_H
#define PAY_SLT_CMDS_H

#include "cfe_error.h"
#include "pay_slt_msg.h"
#include "pay_slt.h"
#include <gs/param/rparam.h>

CFE_Status_t SLT_IFB_SendHkCmd(const SLT_IFB_SendHkCmd_t *Msg);
CFE_Status_t PAY_SLT_SendBeaconCmd(const SLT_IFB_SendBcnCmd_t *Msg);
CFE_Status_t SLT_IFB_NoopCmd(const SLT_IFB_NoopCmd_t *Msg);
CFE_Status_t SLT_IFB_ResetCountersCmd(const SLT_IFB_ResetCountersCmd_t *Msg);
CFE_Status_t PAY_SLT_SetBcnEnabledCmd(const PAY_SLT_SetBcnEnabledCmd_t *Msg);

CFE_Status_t PAY_SLT_IFB_CSP_CMP_Cmd(const PAY_SLT_IFB_CSP_CMP_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_CSP_PING_Cmd(const PAY_SLT_IFB_CSP_PING_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_CSP_PS_Cmd(const PAY_SLT_IFB_CSP_PS_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_CSP_MEM_FREE_Cmd(const PAY_SLT_IFB_CSP_MEM_FREE_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_CSP_REBOOT_Cmd(const PAY_SLT_IFB_CSP_REBOOT_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_CSP_BUF_FREE_Cmd(const PAY_SLT_IFB_CSP_BUF_FREE_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_CSP_UPTIME_Cmd(const PAY_SLT_IFB_CSP_UPTIME_Cmd_t *Msg);
CFE_Status_t PAY_SLT_IFB_CSP_GNDWDT_Cmd(const PAY_SLT_IFB_CSP_GNDWDT_Cmd_t *Msg);


CFE_Status_t PAY_SLT_ParGetCmd(const PAY_SLT_ParGetCmd_t *Msg);
CFE_Status_t PAY_SLT_ParSetCmd(const PAY_SLT_ParSetCmd_t *Msg);
CFE_Status_t PAY_SLT_ScanFilesCmd(const PAY_SLT_ScanFilesCmd_t *Msg);
CFE_Status_t PAY_SLT_DownloadFileCmd(const PAY_SLT_DownloadFileCmd_t *Msg);

CFE_Status_t PAY_SLT_GetFullTableCmd(const PAY_SLT_GetFullTableCmd_t *Msg);

#endif
