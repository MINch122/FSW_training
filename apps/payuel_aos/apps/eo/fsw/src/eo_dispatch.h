/**
 * @file
 *
 * Main header file for the EO dispatch
 */

#ifndef EO_DISPATCH_H
#define EO_DISPATCH_H

#include "cfe.h"
#include "common_types.h"

bool EO_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);
void EO_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr);
void EO_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr);

#endif