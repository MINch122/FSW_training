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
#include "stx.h"
#include "stx_eventids.h"
#include "stx_tbl.h"
#include "stx_utils.h"
#include "enduro_stx.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Verify contents of First Example Table buffer contents                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t STX_TblValidationFunc(void *TblData)
{
    CFE_Status_t               ReturnCode = CFE_SUCCESS;
    STX_ExampleTable_t *TblDataPtr = (STX_ExampleTable_t *)TblData;

    /*
    ** Sample Example Table Validation
    */
    if (TblDataPtr->Int1 > STX_TBL_ELEMENT_1_MAX)
    {
        /* First element is out of range, return an appropriate error code */
        ReturnCode = STX_TABLE_OUT_OF_RANGE_ERR_CODE;
    }

    return ReturnCode;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Output CRC                                                      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void STX_GetCrc(const char *TableName)
{
    CFE_Status_t   status;
    uint32         Crc;
    CFE_TBL_Info_t TblInfoPtr;

    status = CFE_TBL_GetInfo(&TblInfoPtr, TableName);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Sample App: Error Getting Example Table Info");
    }
    else
    {
        Crc = TblInfoPtr.Crc;
        CFE_ES_WriteToSysLog("Sample App: CRC: 0x%08lX\n\n", (unsigned long)Crc);
    }
}

static const char* engine_ret_str(esup_ret_t ret)
{
    switch (ESUP_RET_ENGINE(ret)) {
    case ESUP_OK:                   return NULL;
    case ESUP_SESSION_EXEC_ERROR:   return "device execution error";
    case ESUP_SESSION_REJECTED:     return "rejected (NOT_ACK)";
    case ESUP_SESSION_EXPIRED:      return "result expired or never enqueued";
    case ESUP_SESSION_BUSY_TIMEOUT: return "device stayed busy";
    case ESUP_SESSION_UNREACHABLE:  return "no reply (link or device fault)";
    case ESUP_SESSION_IO_ERROR:     return "host I/O error";
    case ESUP_SESSION_ABORTED:      return "aborted or engine not ready";
    case ESUP_SESSION_DESYNC:       return "reply desync";
    case ESUP_SESSION_PENDING:      return "pending (timed out)";
    case ESUP_SESSION_FULL:         return "no free container cell";
    case ESUP_SESSION_DUP:          return "duplicate selector active";
    case ESUP_SESSION_ACK_FAILED:
        return "result received, result ACK failed";
    case ESUP_SESSION_RESULT_TOO_LARGE:
        return "result exceeded caller buffer";
    default:
        return "engine error";
    }
}

static const char* device_ret_str(esup_ret_t ret)
{
    switch (ESUP_RET_DEVICE_LOCAL(ret)) {
    case ESUP_OK:      return NULL;
    case STX_ERR_ARG:   return "bad argument";
    case STX_ERR_RANGE: return "value out of range";
    case STX_ERR_REPLY: return "short or invalid reply";
    default:            return "device error";
    }
}

void print_status(const char* op, esup_ret_t ret)
{
    if (ret == ESUP_OK) {
        OS_printf("%s: ok\n", op);
        return;
    }

    const char* dev = device_ret_str(ret);
    const char* eng = engine_ret_str(ret);
    uint8_t exec = ESUP_RET_EXEC_STATUS(ret);
    bool printed = false;

    OS_printf("%s: error: ", op);

    if (dev != NULL) {
        OS_printf("%s", dev);
        printed = true;
    }
    if (exec != ESUP_EXEC_OK) {
        if (printed)
            OS_printf(" / ");
        OS_printf("execution status 0x%02X", exec);
        printed = true;
    }
    if (eng != NULL) {
        if (printed)
            OS_printf(" / ");
        OS_printf("%s", eng);
        printed = true;
    }
    if (!printed)
        OS_printf("transport fault");

    OS_printf(" (0x%08X)\n", (unsigned)ret);
}
