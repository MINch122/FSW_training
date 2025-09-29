/**
 * \file
 *
 * Main header file for the FTP application
 */
#ifndef FTP_DISPATCH_H
#define FTP_DISPATCH_H

/*
** Required header files.
*/
#include "cfe.h"
#include "ftp_msg.h"

bool FTP_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);
void FTP_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr);
void FTP_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr);

#endif