#ifndef LGBAT_DISPATCH_H
#define LGBAT_DISPATCH_H

#include "cfe.h"

// Verify that the received command message is the expected length.
// Returns true on success, sends an EVS error and RPT on failure.
bool LGBAT_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);

// Dispatch a ground command by reading the function code and calling the handler.
void LGBAT_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr);

// Main message pipe dispatcher: routes incoming MIDs to the correct handler.
void LGBAT_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr);

#endif // LGBAT_DISPATCH_H
