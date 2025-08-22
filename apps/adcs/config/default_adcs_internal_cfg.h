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
 *   ADCS Application Private Config Definitions
 *
 * This provides default values for configurable items that are internal
 * to this module and do NOT affect the interface(s) of this module.  Changes
 * to items in this file only affect the local module and will be transparent
 * to external entities that are using the public interface(s).
 *
 * @note This file may be overridden/superceded by mission-provided defintions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef ADCS_INTERNAL_CFG_H
#define ADCS_INTERNAL_CFG_H

/***********************************************************************/
#define ADCS_PIPE_DEPTH 32 /* Depth of the Command Pipe for Application */



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                              EndPoint ID                                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * */
/* Set fucntion : ID 1 ~ 122 */
#define	ADCS_ID_SET_RESET		                    1
#define	ADCS_ID_SET_CURRENT_UNIX_TIME               2
#define ADCS_ID_SET_CONTROL_ESTIMATION_MODE         42
#define ADCS_ID_SET_REFERENCE_LLH_TARGET            48
#define ADCS_ID_SET_ORBIT_MODE                      51
#define ADCS_ID_SET_REFERENCE_RPY_VALUES            54
#define ADCS_ID_SET_SAT_ORBIT_PARAM_CONFIG          68
/* Set fucntion : ID 128 ~ 244 */
#define	ADCS_ID_GET_CURRENT_UNIX_TIME_TELEMETRY     133
#define ADCS_ID_GET_CONTROL_ESTIMATION_MODE         150
#define ADCS_ID_GET_REFERENCE_LLH_TARGET            157
#define ADCS_ID_GET_ORBIT_MODE                      162
#define ADCS_ID_GET_RAW_CUBESENSE_SUN               170
#define ADCS_ID_GET_POWERSTATE                      183
#define ADCS_ID_GET_CONTROL_MODE                    185
#define ADCS_ID_GET_SAT_ORBIT_PARAM_CONFIG          196
#define ADCS_ID_GET_RAW_CSS_SENSOR                  203
#define ADCS_ID_GET_RAW_GYR_SENSOR                  204
#define ADCS_ID_GET_CALIBRATED_GYR_SENSOR           207
/* End of EndPoint ID */

#endif
