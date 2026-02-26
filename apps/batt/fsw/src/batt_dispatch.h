/**
 * @file
 *
 * Dispatch header for the BATT application
 */

#ifndef BATT_DISPATCH_H
#define BATT_DISPATCH_H

/*
** Required header files.
*/
#include "cfe.h"
#include "batt_msg.h"

void BATT_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr);
void BATT_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr);
bool BATT_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);

#endif /* BATT_DISPATCH_H */
