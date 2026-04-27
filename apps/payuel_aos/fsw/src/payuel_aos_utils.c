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
 *   This file contains the source code for the Payuel Aos utility functions
 */

/*
** Include Files:
*/
#include "payuel_aos.h"
#include "payuel_aos_eventids.h"
#include "payuel_aos_tbl.h"
#include "payuel_aos_utils.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Verify contents of First Example Table buffer contents                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t PAYUEL_AOS_TblValidationFunc(void *TblData)
{
    CFE_Status_t               ReturnCode = CFE_SUCCESS;
    PAYUEL_AOS_ExampleTable_t *TblDataPtr = (PAYUEL_AOS_ExampleTable_t *)TblData;

    /*
    ** PayuelAos Example Table Validation
    */
    if (TblDataPtr->Int1 > PAYUEL_AOS_TBL_ELEMENT_1_MAX)
    {
        /* First element is out of range, return an appropriate error code */
        ReturnCode = PAYUEL_AOS_TABLE_OUT_OF_RANGE_ERR_CODE;
    }

    return ReturnCode;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Output CRC                                                      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void PAYUEL_AOS_GetCrc(const char *TableName)
{
    CFE_Status_t   status;
    uint32         Crc;
    CFE_TBL_Info_t TblInfoPtr;

    status = CFE_TBL_GetInfo(&TblInfoPtr, TableName);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Payuel Aos: Error Getting Example Table Info");
    }
    else
    {
        Crc = TblInfoPtr.Crc;
        CFE_ES_WriteToSysLog("Payuel Aos: CRC: 0x%08lX\n\n", (unsigned long)Crc);
    }
}
