/************************************************************************
 * NASA Docket No. GSC-18,917-1, and identified as “CFS Data Storage
 * (DS) application version 2.6.1”
 *
 * Copyright (c) 2021 United States Government as represented by the
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
 *  The CFS Data Storage (DS) Application Message IDs header file
 */
#ifndef DS_MSGIDS_H
#define DS_MSGIDS_H

/**
 * \defgroup cfsdscmdmid CFS Data Storage Message IDs
 * \{
 */

#define DS_CMD_MID     0x1813 /**< \brief DS Ground Commands Message ID */
#define DS_SEND_HK_MID 0x1814 /**< \brief DS Send Hk Data Cmd Message ID*/

/**\}*/

/**
 * \defgroup cfsdstlmmid CFS Data Storage Message IDs
 * \{
 */

#define DS_HK_TLM_MID   0x0811 /**< \brief DS Hk Telemetry Message ID ****/
#define DS_DIAG_TLM_MID 0x0812 /**< \brief DS File Info Telemetry Message ID ****/
#define DS_COMP_TLM_MID 0x0813 /**< \brief DS Completed File Info Telemetry Message ID ****/

/**\}*/

#endif
