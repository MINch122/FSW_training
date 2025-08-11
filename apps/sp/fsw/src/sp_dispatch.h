#ifndef SP_APP_DISPATCH_H
#define SP_APP_DISPATCH_H

#include "cfe.h"
#include "sp_msg.h"

void SP_APP_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr);
void SP_APP_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr);
bool SP_APP_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);

#endif