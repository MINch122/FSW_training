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
 * \file
 *   This file contains the source code for the Sample App utility functions
 */

/*
** Include Files:
*/
#include "sant_app.h"
#include "sant_eventids.h"
#include "sant_utils.h"

void SANT_HandleReport(int32 Status, uint8_t CC, void *ReadData, uint16_t ReadSize) {
    CFE_SB_Buffer_t *BufPtr = CFE_SB_AllocateMessageBuffer(sizeof(SANT_ReportTlm_t));
	if (BufPtr == NULL) return;

	SANT_ReportTlm_t *Report = (SANT_ReportTlm_t *)BufPtr;
	if (CFE_MSG_Init(CFE_MSG_PTR(Report->TelemetryHeader), CFE_SB_ValueToMsgId(SANT_REPORT_TLM_MID), sizeof(SANT_ReportTlm_t)) != CFE_SUCCESS) {
		CFE_SB_ReleaseMessageBuffer(BufPtr);
		return;
	}

	Report->Report.MsgID = SANT_CMD_MID;
	Report->Report.CommandCode = CC;
	Report->Report.ReturnType = (Status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_HW;
	Report->Report.ReturnCode = Status;
	Report->Report.ReturnDataSize = ReadSize;
	if (ReadSize && ReadData) {
		memcpy(Report->Report.ReturnValue, ReadData, ReadSize);
	}

	CFE_SB_TimeStampMsg((CFE_MSG_PTR(Report->TelemetryHeader)));
	if (CFE_SB_TransmitBuffer(BufPtr, true) != CFE_SUCCESS) {
		CFE_SB_ReleaseMessageBuffer(BufPtr);
		return;
	}
	return;
}