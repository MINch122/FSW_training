#ifndef FTP_CLIENT_DISPATCH_H
#define FTP_CLIENT_DISPATCH_H

#include "cfe.h"

bool FTP_CLIENT_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);
void FTP_CLIENT_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr);
void FTP_CLIENT_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr);

#endif
