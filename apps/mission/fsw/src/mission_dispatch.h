/**
 * @file
 *
 * Main header file for the MISSION dispatch
 */

#ifndef MISSION_DISPATCH_H
#define MISSION_DISPATCH_H

#include "cfe.h"
#include "common_types.h"

bool MISSION_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);
void MISSION_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr);
void MISSION_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr);

#endif