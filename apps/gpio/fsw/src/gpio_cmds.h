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
 * @file
 *   This file contains the prototypes for the Sample App Ground Command-handling functions
 */

#ifndef GPIO_CMDS_H
#define GPIO_CMDS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "gpio_msg.h"

CFE_Status_t GPIO_SendHkCmd(const GPIO_SendHkCmd_t *Msg);
CFE_Status_t GPIO_SendBcnCmd(const GPIO_SendBcnCmd_t *Msg);
void         GPIO_InitOutputDefaults(void);
CFE_Status_t GPIO_ResetCountersCmd(const GPIO_ResetCountersCmd_t *Msg);
CFE_Status_t GPIO_ProcessCmd(const GPIO_ProcessCmd_t *Msg);
CFE_Status_t GPIO_NoopCmd(const GPIO_NoopCmd_t *Msg);
CFE_Status_t GPIO_DigpiolayParamCmd(const GPIO_DigpiolayParamCmd_t *Msg);
CFE_Status_t GPIO_LtrxEnOnCmd(const GPIO_LtrxEnOnCmd_t *Msg);
CFE_Status_t GPIO_LtrxEnOffCmd(const GPIO_LtrxEnOffCmd_t *Msg);
CFE_Status_t GPIO_Dep1EnOnCmd(const GPIO_Dep1EnOnCmd_t *Msg);
CFE_Status_t GPIO_Dep1EnOffCmd(const GPIO_Dep1EnOffCmd_t *Msg);
CFE_Status_t GPIO_Dep2EnOnCmd(const GPIO_Dep2EnOnCmd_t *Msg);
CFE_Status_t GPIO_Dep2EnOffCmd(const GPIO_Dep2EnOffCmd_t *Msg);
CFE_Status_t GPIO_SpInRead5sCmd(const GPIO_SpInRead5sCmd_t *Msg);
CFE_Status_t GPIO_StxEnOnCmd(const GPIO_StxEnOnCmd_t *Msg);
CFE_Status_t GPIO_StxEnOffCmd(const GPIO_StxEnOffCmd_t *Msg);
CFE_Status_t GPIO_AdcsEnOnCmd(const GPIO_AdcsEnOnCmd_t *Msg);
CFE_Status_t GPIO_AdcsEnOffCmd(const GPIO_AdcsEnOffCmd_t *Msg);
CFE_Status_t GPIO_AdcsBootOnCmd(const GPIO_AdcsBootOnCmd_t *Msg);
CFE_Status_t GPIO_AdcsBootOffCmd(const GPIO_AdcsBootOffCmd_t *Msg);


#endif /* GPIO_CMDS_H */
