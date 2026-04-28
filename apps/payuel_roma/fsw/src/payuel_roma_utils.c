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
#include "payuel_roma.h"
#include "payuel_roma_eventids.h"
#include "payuel_roma_tbl.h"
#include "payuel_roma_utils.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Verify contents of First Example Table buffer contents                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t PAYUEL_ROMA_TblValidationFunc(void *TblData)
{
    CFE_Status_t               ReturnCode = CFE_SUCCESS;
    PAYUEL_ROMA_ExampleTable_t *TblDataPtr = (PAYUEL_ROMA_ExampleTable_t *)TblData;

    /*
    ** Sample Example Table Validation
    */
    if (TblDataPtr->Int1 > PAYUEL_ROMA_TBL_ELEMENT_1_MAX)
    {
        /* First element is out of range, return an appropriate error code */
        ReturnCode = PAYUEL_ROMA_TABLE_OUT_OF_RANGE_ERR_CODE;
    }

    return ReturnCode;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Output CRC                                                      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void PAYUEL_ROMA_GetCrc(const char *TableName)
{
    CFE_Status_t   status;
    uint32         Crc;
    CFE_TBL_Info_t TblInfoPtr;

    status = CFE_TBL_GetInfo(&TblInfoPtr, TableName);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Roma-SP: Error Getting Example Table Info");
    }
    else
    {
        Crc = TblInfoPtr.Crc;
        CFE_ES_WriteToSysLog("Roma-SP: CRC: 0x%08lX\n\n", (unsigned long)Crc);
    }
}

/***********************************************
 * 
 * Report util function
 * 
 **********************************************/
void PAYUEL_ROMA_HandleReport(int32 Status, uint8_t CC, void *ReadData, uint16_t ReadSize)
{
	CFE_SB_Buffer_t *BufPtr = CFE_SB_AllocateMessageBuffer(sizeof(PAYUEL_ROMA_ReportTlm_t));
	if (BufPtr == NULL) return;

	PAYUEL_ROMA_ReportTlm_t *Report = (PAYUEL_ROMA_ReportTlm_t *)BufPtr;
	if (CFE_MSG_Init(CFE_MSG_PTR(Report->TelemetryHeader), CFE_SB_ValueToMsgId(PAYUEL_ROMA_REPORT_TLM_MID),
         sizeof(PAYUEL_ROMA_ReportTlm_t)) != CFE_SUCCESS)
    {
		CFE_SB_ReleaseMessageBuffer(BufPtr);
		return;
	}

	Report->Report.MsgID = PAYUEL_ROMA_CMD_MID;
	Report->Report.CommandCode = CC;
	Report->Report.ReturnType = (Status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_HW;
	Report->Report.ReturnCode = Status; // `device/s5lab.h`
	Report->Report.ReturnDataSize = ReadSize;
	if (ReadSize && ReadData)
    {
		memcpy(Report->Report.ReturnValue, ReadData, ReadSize);
	}

	CFE_SB_TimeStampMsg((CFE_MSG_PTR(Report->TelemetryHeader)));
	if (CFE_SB_TransmitBuffer(BufPtr, true) != CFE_SUCCESS)
    {
		CFE_SB_ReleaseMessageBuffer(BufPtr);
		return;
	}
	return;
}
