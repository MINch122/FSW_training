#ifndef PAYUZUT_DISPATCH_H
#define PAYUZUT_DISPATCH_H

/*
** Required header files.
*/
#include "cfe.h"
#include "payuzut_msg.h"

bool PAYUZUT_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);
void PAYUZUT_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr);
void PAYUZUT_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr);

#endif
