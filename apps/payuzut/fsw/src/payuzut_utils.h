#ifndef PAYUZUT_UTILS_H
#define PAYUZUT_UTILS_H

#include "common_types.h"

int PAYUZUT_OpenFile(void);
int32 PAYUZUT_WriteToFile(int FD, void *Data, size_t Size);
int32 PAYUZUT_CloseFile(int FD);

#endif
