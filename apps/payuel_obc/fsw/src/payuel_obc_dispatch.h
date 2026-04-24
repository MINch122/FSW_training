#ifndef PAYUEL_OBC_DISPATCH_H
#define PAYUEL_OBC_DISPATCH_H

#include "cfe.h"
#include "payuel_obc_msg.h"

void PAYUEL_OBC_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr);
void PAYUEL_OBC_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr);

#endif /* PAYUEL_OBC_DISPATCH_H */
