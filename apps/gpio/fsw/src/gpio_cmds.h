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
CFE_Status_t GPIO_LtrxEnHighCmd(const GPIO_LtrxEnHighCmd_t *Msg);
CFE_Status_t GPIO_LtrxEnLowCmd(const GPIO_LtrxEnLowCmd_t *Msg);
CFE_Status_t GPIO_Dep1EnHighCmd(const GPIO_Dep1EnHighCmd_t *Msg);
CFE_Status_t GPIO_Dep1EnLowCmd(const GPIO_Dep1EnLowCmd_t *Msg);
CFE_Status_t GPIO_Dep2EnHighCmd(const GPIO_Dep2EnHighCmd_t *Msg);
CFE_Status_t GPIO_Dep2EnLowCmd(const GPIO_Dep2EnLowCmd_t *Msg);
CFE_Status_t GPIO_DepBurnCmd(const GPIO_DepBurnCmd_t *Msg);
CFE_Status_t GPIO_SpInRead5sCmd(const GPIO_SpInRead5sCmd_t *Msg);
CFE_Status_t GPIO_StxEnHighCmd(const GPIO_StxEnHighCmd_t *Msg);
CFE_Status_t GPIO_StxEnLowCmd(const GPIO_StxEnLowCmd_t *Msg);
CFE_Status_t GPIO_AdcsEnHighCmd(const GPIO_AdcsEnHighCmd_t *Msg);
CFE_Status_t GPIO_AdcsEnLowCmd(const GPIO_AdcsEnLowCmd_t *Msg);
CFE_Status_t GPIO_AdcsBootHighCmd(const GPIO_AdcsBootHighCmd_t *Msg);
CFE_Status_t GPIO_AdcsBootLowCmd(const GPIO_AdcsBootLowCmd_t *Msg);


#endif /* GPIO_CMDS_H */
