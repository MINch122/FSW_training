/**
 * @file
 *
 * Main header file for the RPT dispatch
 */

#ifndef RPT_DISPATCH_H
#define RPT_DISPATCH_H

#include "cfe.h"
#include "common_types.h"

bool RPT_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);
void RPT_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr);
void RPT_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr);

#endif