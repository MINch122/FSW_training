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
} UANT_HkTlm_Payload_t;

typedef struct {
    uint16  deploystatus;
} UANT_BcnTlm_Payload_t;

#endif
