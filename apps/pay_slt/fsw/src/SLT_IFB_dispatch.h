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
 * Main header file for the SAMPLE application
 */

#ifndef SLT_IFB_DISPATCH_H
#define SLT_IFB_DISPATCH_H

/*
** Required header files.
*/
#include "cfe.h"
#include "SLT_IFB_msg.h"

void SLT_IFB_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr);
void SLT_IFB_ProcessBasicCommand(const CFE_SB_Buffer_t *SBBufPtr);
void SLT_IFB_ProcessTransactionCommand(const CFE_SB_Buffer_t *SBBufPtr);
void SLT_IFB_ProcessGetRParamCommand(const CFE_SB_Buffer_t *SBBufPtr);
void SLT_IFB_ProcessSaveTableCommand(const CFE_SB_Buffer_t *SBBufPtr);
bool SLT_IFB_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);

#endif /* SLT_IFB_DISPATCH_H */