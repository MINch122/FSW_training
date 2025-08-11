#include "sp_task.h"
#include "sp_eventids.h"
#include "sp_tbl.h"
#include "sp_utils.h"

CFE_Status_t SP_APP_TBLValidationFunc(void *TblData){
    CFE_Status_t ReturnCode = CFE_SUCCESS;
    SP_APP_ExampleTable_t *TblDataPtr = (SP_APP_ExampleTable_t *)TblData;

    if (TblDataPtr->Int1 > SP_APP_TBL_ELEMENT_1_MAX){
        ReturnCode = SP_APP_TABLE_OUT_OF_RANGE_ERR_CODE;
    }

    return ReturnCode;
}

void SP_APP_GetCrc(const char *TableName){
    CFE_Status_t status;
    uint32 Crc;
    CFE_TBL_Info_t TblInfoPtr;

    status = CFE_TBL_GetInfo(&TblInfoPtr, TableName);
    if (status != CFE_SUCCESS){
        CFE_ES_WriteToSysLog("SP: Error Getting Example Table Info");
    }
    else{
        Crc = TblInfoPtr.Crc;
        CFE_ES_WriteToSysLog("SP: CRC: 0x%08lx\n\n", (unsigned long)Crc);
    }
}