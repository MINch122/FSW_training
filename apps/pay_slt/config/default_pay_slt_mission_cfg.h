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
 * SAMPLE_APP Application Mission Configuration Header File
 *
 * This is a compatibility header for the "mission_cfg.h" file that has
 * traditionally provided public config definitions for each CFS app.
 *
 * @note This file may be overridden/superceded by mission-provided defintions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef SLT_IFB_MISSION_CFG_H
#define SLT_IFB_MISSION_CFG_H

#include "pay_slt_interface_cfg.h"

#ifndef PAY_SLT_DEBUG
#define PAY_SLT_DEBUG false
#endif

#ifndef DEBUG_PAY_SLT
#define DEBUG_PAY_SLT PAY_SLT_DEBUG
#endif

#if DEBUG_PAY_SLT
#define PAY_SLT_APP_printf(...) OS_printf(__VA_ARGS__)
#else
#define PAY_SLT_APP_printf(...) do { if (false) { OS_printf(__VA_ARGS__); } } while (0)
#endif

#ifndef PAY_SLT_SB_BCN_ENABLED
#define PAY_SLT_SB_BCN_ENABLED false
#endif

#if defined(CFE_SRL_I2C1_HANDLE_INDEXER) && !defined(PAY_SLT_I2C_HANDLE_INDEXER)
#define PAY_SLT_I2C_HANDLE_INDEXER CFE_SRL_I2C1_HANDLE_INDEXER
#endif

#endif
