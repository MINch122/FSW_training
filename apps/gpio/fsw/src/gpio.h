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
 *
 * Main header file for the Sample application
 */

#ifndef GPIO_H
#define GPIO_H

/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_config.h"

#include "gpio_mission_cfg.h"
#include "gpio_platform_cfg.h"

#include "gpio_perfids.h"
#include "gpio_msgids.h"
#include "gpio_msg.h"

/************************************************************************
** Type Definitions
*************************************************************************/

/*
** Global Data
*/
typedef struct
{
    /*
    ** Command interface counters...
    */
    uint8 CmdCounter;
    uint8 ErrCounter;

    /*
    ** Housekeeping telemetry packet...
    */
    GPIO_HkTlm_t HkTlm;
    GPIO_BcnTlm_t BcnTlm;
    uint16        OutputStateBits;
    uint16        OutputCommandedBits;
    uint8         IsDeployed;

    /*
    ** Run Status variable used in the main processing loop
    */
    uint32 RunStatus;

    /*
    ** Operational data (not reported in housekeeping)...
    */
    CFE_SB_PipeId_t CommandPipe;

    /*
    ** Initialization data (not reported in housekeeping)...
    */
    char   PipeName[CFE_MISSION_MAX_API_LEN];
    uint16 PipeDepth;

} GPIO_Data_t;

/*
** Global data structure
*/
extern GPIO_Data_t GPIO_Data;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (GPIO_Main), these
**       functions are not called from any other source module.
*/
void         GPIO_Main(void);
CFE_Status_t GPIO_Init(void);

#endif /* GPIO_H */
