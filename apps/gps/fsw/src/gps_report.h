/************************************************************************
 * NASA Docket No. GSC-19,200-1, and identified as "cFS Draco"
 *
 * Copyright (c) 2023 United States Government as represented by the
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
 * @brief  GPS command-outcome reporting, RPT app contract
 *
 * The packet is RPT_Report_t and goes out on GPS_REPORT_TLM_MID, which the RPT
 * table must carry as a subscribed entry. See apps/rpt/README.md.
 */
#ifndef GPS_REPORT_H
#define GPS_REPORT_H

#include "cfe.h"

/**
 * Transmit the GPS_ReportTlm_t packet.
 *
 * @param cmd      the invoking command's message header
 * @param data     retrieved payload, or NULL
 * @param dataSize bytes of retrieved data (clamped to the packet capacity)
 * @param retCode  the command's return value
 * @param retType  GPS_MISSION_REPORT_RETTYPE_* bucket for the return value;
 *                 a successful outcome is normalized to RPT_RETTYPE_SUCCESS
 */
void GPS_SendReport(const void* cmd, const void* data, uint16 dataSize, int32 retCode, uint8 retType);

#endif /* GPS_REPORT_H */
