#ifndef THRUST_DISPATCH_H
#define THRUST_DISPATCH_H

#include "cfe.h"           // CFE_SB_Buffer_t, CFE_MSG_Message_t
#include "thrust_msg.h"    // 메시지 구조체

void THRUST_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr);
void THRUST_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr);
bool THRUST_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);

#endif /* THRUST_DISPATCH_H */
