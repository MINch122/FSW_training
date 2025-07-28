#ifndef RPT_DISPATCH_H
#define RPT_DISPATCH_H

#include "common_types.h"

void RPT_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr);
void RPT_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr);

#endif