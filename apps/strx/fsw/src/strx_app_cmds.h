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
 *   This file contains the prototypes for the Strx App Ground Command-handling functions
 */

#ifndef STRX_APP_CMDS_H
#define STRX_APP_CMDS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "strx_app_msg.h"

#include "strx_app.h"
#include "strx_app_dispatch.h"
#include "strx_app_cmds.h"

void STRX_ReportHousekeeping(void);
void STRX_ReportBeacon(void);
CFE_Status_t STRX_APP_ResetCountersCmd(const STRX_APP_ResetCountersCmd_t *Msg);
CFE_Status_t STRX_APP_NoopCmd(const STRX_APP_NoopCmd_t *Msg);
CFE_Status_t STRX_APP_ResetAppCmdCountersCmd(const STRX_APP_ResetAppCmdCountersCmd_t *Msg);
CFE_Status_t STRX_APP_ResetDeviceCmdCountersCmd(const STRX_APP_ResetDeviceCmdCountersCmd_t *Msg);


void STRX_GndwdtClearCmd(void);
void STRX_RebootCmd(void);
void STRX_RXCONF_SetBaudCmd(const STRX_U32ArgsCmd_t * SBBufPtr);
void STRX_TXCONF_SetBaudCmd(const STRX_U32ArgsCmd_t * SBBufPtr);
void STRX_RXCONF_SetFreqCmd(const STRX_U32ArgsCmd_t * SBBufPtr);
void STRX_TXCONF_SetFreqCmd(const STRX_U32ArgsCmd_t * SBBufPtr);
void STRX_TLM_SET_KISS_USARTCmd(const STRX_8ArgsCmd_t * SBBufPtr);
void STRX_TLM_SET_GOSH_USARTTCmd(const STRX_U8ArgsCmd_t * SBBufPtr);
void STRX_SetDefaultBaudCmd(void);
void STRX_RparamSave0Cmd(void);
void STRX_RparamSave1Cmd(void);
void STRX_RparamSave4Cmd(void);
void STRX_RparamSave5Cmd(void);
void STRX_RparamSaveAllCmd(void);
void csp_checkstate_pingCmd(void);


void STRX_RXCONF_GetBaudCmd(void);
void STRX_RXCONF_GetGuardCmd(void);
void STRX_RXCONF_GetFreqCmd(void);
void STRX_TXCONF_GetBaudCmd(void);
void STRX_TXCONF_GetFreqCmd(void);
void STRX_TLM_GetTempBrdCmd(void);
void STRX_TLM_GetLastRssiCmd(void);
void STRX_TLM_GetLastRferrCmd(void);
void STRX_TLM_GetBootCountCmd(void);
void STRX_TLM_GetBootCauseCmd(void);
void STRX_TLM_GetLastContactCmd(void);
void STRX_TLM_GetTotTxBytesCmd(void);
void STRX_TLM_GetTotRxBytesCmd(void);
void STRX_TLM_GET_RXMODECmd(void);
void STRX_TLM_GET_GND_WDT_CNTCmd(void);
void STRX_TLM_GET_GND_WDT_LEFTCmd(void);
void STRX_TLM_GET_KISS_USARTCmd(void);
void STRX_TLM_GET_GOSH_USARTCmd(void);

void CmdErrCounter(uint8 *CmdCounter,
                   uint8 *ErrCounter,
                   uint8 *DeviceErrCounter,
                   uint8  retType,   /* CMD_RETCODE_TYPE_* */
                   int32  retCode); 
                   
static inline void STRX_CountFromReport(void)
{
    CmdErrCounter(&STRX_APP_Data.AppCnt.AppCmdCounter,
                  &STRX_APP_Data.AppCnt.AppErrCounter,
                  &STRX_APP_Data.AppCnt.DeviceErrCounter,
                  STRX_APP_Data.RptPkt.Report.ReturnType,
                  STRX_APP_Data.RptPkt.Report.ReturnCode);
}


#endif 
/* STRX_APP_CMDS_H */
