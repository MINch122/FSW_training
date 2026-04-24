/**
 * @file
 *   Report (EO) Init function
 */
#include "eo_task.h"
#include "common_types.h"
#include "eo_mission_cfg.h"
#include "eo_msgids.h"
#include "eo_utils.h"
#include "eo_mission_cfg.h"
#include "eo_eventids.h"
#include "cfe_msgids.h"


CFE_Status_t EO_StepInit(void) {
    int32 OsStatus;
    
    OsStatus = OS_OpenCreate(&EO_Data.CurrentStepHandle, EO_CURRENT_DATA_PATH, OS_FILE_FLAG_CREATE, OS_READ_WRITE);
    OS_printf("EO ops FD: %d\n", EO_Data.CurrentStepHandle);

    if (OsStatus != OS_SUCCESS) return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;

    OsStatus = OS_read(EO_Data.CurrentStepHandle, &EO_Data.CurrentStep, sizeof(EO_Data.CurrentStep));
    if (OsStatus == 0) return CFE_SUCCESS;

    else if (OsStatus != sizeof(EO_CurrentStep_t)) { // It's empty, because file is created
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OsStatus = OS_close(EO_Data.CurrentStepHandle);
    if (OsStatus != OS_SUCCESS) return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;

    return CFE_SUCCESS;
}

CFE_Status_t EO_WriteStep(void) {
    int32 OsStatus;
    
    OsStatus = OS_OpenCreate(&EO_Data.CurrentStepHandle, EO_CURRENT_DATA_PATH, OS_FILE_FLAG_CREATE, OS_READ_WRITE);
    OS_printf("EO ops FD: %d\n", EO_Data.CurrentStepHandle);

    if (OsStatus != OS_SUCCESS) return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;

    OS_MutSemTake(EO_Data.EOMutex);
    OsStatus = OS_write(EO_Data.CurrentStepHandle, &EO_Data.CurrentStep, sizeof(EO_Data.CurrentStep));
    OS_MutSemGive(EO_Data.EOMutex);
    
    if (OsStatus != 0 && OsStatus != sizeof(EO_CurrentStep_t)) { // It's empty, because file is created
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OsStatus = OS_close(EO_Data.CurrentStepHandle);
    if (OsStatus != OS_SUCCESS) return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;

    return CFE_SUCCESS;
}