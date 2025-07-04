/**
 * @file
 *
 * Main header file for the PAY UZURO CAM application
 */
#ifndef PAYUZUC_DISPATCH_H
#define PAYUZUC_DISPATCH_H

/*
** Required header files.
*/
#include "cfe.h"
#include "payuzuc_msg.h"

bool PAYUZUC_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);
void PAYUZUC_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr);
void PAYUZUC_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr);

#endif /* PAYUZUC_DISPATCH_H */