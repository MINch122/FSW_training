#ifndef SCH_LAB_DISPATCH_H
#define SCH_LAB_DISPATCH_H

#include <stdbool.h>
#include <stddef.h>

#include "cfe.h"

bool SCH_LAB_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);
void SCH_LAB_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr);
bool SCH_LAB_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr);

#endif
