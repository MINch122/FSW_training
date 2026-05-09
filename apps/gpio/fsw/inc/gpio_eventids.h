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
 * Define Sample App Events IDs
 */

#ifndef GPIO_EVENTS_H
#define GPIO_EVENTS_H

#define GPIO_RESERVED_EID      0
#define GPIO_INIT_INF_EID      1
#define GPIO_CC_ERR_EID        2
#define GPIO_NOOP_INF_EID      3
#define GPIO_RESET_INF_EID     4
#define GPIO_MID_ERR_EID       5
#define GPIO_CMD_LEN_ERR_EID   6
#define GPIO_PIPE_ERR_EID      7
#define GPIO_VALUE_INF_EID     8
#define GPIO_CR_PIPE_ERR_EID   9
#define GPIO_SUB_HK_ERR_EID    10
#define GPIO_SUB_CMD_ERR_EID   11

#endif /* GPIO_EVENTS_H */
