#ifndef SP_APP_UTILS_H
#define SP_APP_UTILS_H

#include "sp_task.h"

CFE_Status_t SP_APP_TBLValidationFunc(void *TblData);
void SP_APP_GetCrc(const char *TableName);

#endif