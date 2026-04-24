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
 * Common definitions for all payuel_roma coverage tests
 */

#ifndef PAYUEL_ROMA_COVERAGETEST_COMMON_H
#define PAYUEL_ROMA_COVERAGETEST_COMMON_H

/*
 * Includes
 */

#include "utassert.h"
#include "uttest.h"
#include "utstubs.h"

#include "setup.h"
#include "eventcheck.h"

#include "cfe.h"
#include "payuel_roma_eventids.h"
#include "payuel_roma.h"
#include "payuel_roma_dispatch.h"
#include "payuel_roma_cmds.h"
#include "payuel_roma_utils.h"
#include "payuel_roma_msgids.h"
#include "payuel_roma_msg.h"
#include "payuel_roma_tbl.h"

/*
 * Macro to add a test case to the list of tests to execute
 */
#define ADD_TEST(test) UtTest_Add((Test_##test), Uelpay_Roma_UT_Setup, Uelpay_Roma_UT_TearDown, #test)

#endif /* PAYUEL_ROMA_COVERAGETEST_COMMON_H */
