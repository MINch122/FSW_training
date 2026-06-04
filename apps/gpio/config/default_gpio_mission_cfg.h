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
 * GPIO Application Mission Configuration Header File
 *
 * This is a compatibility header for the "mission_cfg.h" file that has
 * traditionally provided public config definitions for each CFS app.
 *
 * @note This file may be overridden/superceded by mission-provided defintions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef GPIO_MISSION_CFG_H
#define GPIO_MISSION_CFG_H

#define GPIO_OUTPUT_LTRX_EN_BIT 0
#define GPIO_OUTPUT_DEP1_EN_BIT 1
#define GPIO_OUTPUT_DEP2_EN_BIT 2
#define GPIO_OUTPUT_STX_EN_BIT 3
#define GPIO_OUTPUT_ADCS_EN_BIT 4
#define GPIO_OUTPUT_ADCS_BOOT_BIT 5
#define GPIO_OUTPUT_STATE_MASK 0x003F
#define GPIO_OUTPUT_COMMANDED_SHIFT 8
#define GPIO_OUTPUT_COMMANDED_MASK 0x3F00

#ifndef DEBUG_GPIO
#define DEBUG_GPIO false
#endif

#if DEBUG_GPIO
#define GPIO_APP_printf(...) OS_printf(__VA_ARGS__)
#else
#define GPIO_APP_printf(...) do { } while (0)
#endif

#include "gpio_interface_cfg.h"

#endif
