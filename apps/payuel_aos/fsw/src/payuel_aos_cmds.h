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
 *   This file contains the prototypes for the Payuel Aos Ground Command-handling functions
 */

#ifndef PAYUEL_AOS_CMDS_H
#define PAYUEL_AOS_CMDS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "payuel_aos_msg.h"

CFE_Status_t PAYUEL_AOS_SendHkCmd(const PAYUEL_AOS_SendHkCmd_t *Msg);
CFE_Status_t PAYUEL_AOS_ResetCountersCmd(const PAYUEL_AOS_ResetCountersCmd_t *Msg);
CFE_Status_t PAYUEL_AOS_NoopCmd(const PAYUEL_AOS_NoopCmd_t *Msg);

/*
** ADS1115 Reset
*/
CFE_Status_t PAYUEL_AOS_ResetCmd(const PAYUEL_AOS_ResetCmd_t *Msg);

/*
** ADS1115 Register Write
*/
CFE_Status_t PAYUEL_AOS_Write_RegisterCmd(const PAYUEL_AOS_Write_RegisterCmd_t *Msg);
/*
** ADS1115 Register Read
*/
CFE_Status_t PAYUEL_AOS_Read_RegisterCmd(const PAYUEL_AOS_Read_RegisterCmd_t *Msg);
/*
** ADS1115 Register Read
*/
CFE_Status_t PAYUEL_AOS_ReadAllChannelsCmd(const PAYUEL_AOS_ReadAllChannelsCmd_t *Msg);


#endif /* PAYUEL_AOS_CMDS_H */
