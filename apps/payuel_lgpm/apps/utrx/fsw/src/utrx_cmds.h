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
 *   This file contains the prototypes for the Utrx App Ground Command-handling functions
 */

#ifndef UTRX_CMDS_H
#define UTRX_CMDS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "utrx_msg.h"

CFE_Status_t UTRX_SendHkCmd(const CFE_SB_Buffer_t *SBBufPtr);
CFE_Status_t UTRX_ResetCountersCmd(const UTRX_ResetCountersCmd_t *Msg);
CFE_Status_t UTRX_NoopCmd(const UTRX_NoopCmd_t *Msg);
CFE_Status_t UTRX_ResetAppCmdCountersCmd(const UTRX_ResetAppCmdCountersCmd_t *Msg);
CFE_Status_t UTRX_ResetDeviceCmdCountersCmd(const UTRX_ResetDeviceCmdCountersCmd_t *Msg);
void UTRX_ReportHousekeeping(void);
void UTRX_ReportBeacon(void);
void CmdErrCounter(uint8 *CmdCounter,
                   uint8 *ErrCounter,
                   uint8 *DeviceErrCounter,
                   uint8  retType,   /* CMD_RETCODE_TYPE_* */
                   int32  retCode); 



#endif 
/* UTRX_CMDS_H */
