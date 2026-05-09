#ifndef LTRX_DISPATCH_H
#define LTRX_DISPATCH_H

#include "cfe.h"

void LTRX_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr);
void LTRX_DispatchCommand(const CFE_SB_Buffer_t *SBBufPtr);

#endif /* LTRX_DISPATCH_H */