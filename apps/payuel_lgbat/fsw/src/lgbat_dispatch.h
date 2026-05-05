#ifndef LGBAT_DISPATCH_H
#define LGBAT_DISPATCH_H

#include "cfe.h"

bool LGBAT_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);
void LGBAT_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr);
void LGBAT_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr);

#endif /* LGBAT_DISPATCH_H */
