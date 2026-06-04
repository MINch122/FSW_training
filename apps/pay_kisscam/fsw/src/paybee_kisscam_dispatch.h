/**
 * @file
 *
 * Main header file for the PAY UZURO CAM application
 */
#ifndef paybee_kisscam_DISPATCH_H
#define paybee_kisscam_DISPATCH_H

/*
** Required header files.
*/
#include "cfe.h"
#include "paybee_kisscam_msg.h"

bool paybee_kisscam_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);
void paybee_kisscam_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr);
void paybee_kisscam_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr);

#endif /* paybee_kisscam_DISPATCH_H */