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
 *   Specification for the UANT command and telemetry 
 *   message constant definitions.
 *
 *  For UANT this is only the function/command code definitions
 */
#ifndef UANT_MSGDEFS_H
#define UANT_MSGDEFS_H

#include "common_types.h"
#include "uant_fcncodes.h"

typedef struct {
    uint16  deploystatus;
    uint8 ant1actvcnt;
    uint8 ant2actvcnt;
    uint8 ant3actvcnt;
    uint8 ant4actvcnt;
} UANT_HkTlm_Payload_t; //하나의 cmd로 모든 ant정보를 알고 싶어 다 때려박음 -> 주기적으로 저장하면 개꿀


typedef struct {
    uint16  deploystatus;
} UANT_BcnTlm_Payload_t;  // -> 방이 모자라 근데 ant의 정보를 알아야해 그래서난 active 뺨 -> 주기적으로 저장하면 더 곤란, 상태를 잘 파악이 안됨, 하지만 최소한 고장인지 아닌지, 등등 판단 가능/ 가장 중요한 parm만 들어감 보통


#endif
