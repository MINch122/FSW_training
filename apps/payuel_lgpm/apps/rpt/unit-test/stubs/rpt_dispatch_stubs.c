/**
 * @file
 *
 * Main header file for the RPT dispatch
 */

/**
 * @file
 *
 * Auto-Generated stub implementations for functions defined in rpt_dispatch header
 */

#include "rpt_dispatch.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_ProcessGroundCommand()
 * ----------------------------------------------------
 */
void RPT_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    UT_GenStub_AddParam(RPT_ProcessGroundCommand, const CFE_SB_Buffer_t *, SBBufPtr);

    UT_GenStub_Execute(RPT_ProcessGroundCommand, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_TaskPipe()
 * ----------------------------------------------------
 */
void RPT_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    UT_GenStub_AddParam(RPT_TaskPipe, const CFE_SB_Buffer_t *, SBBufPtr);

    UT_GenStub_Execute(RPT_TaskPipe, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_VerifyCmdLength()
 * ----------------------------------------------------
 */
bool RPT_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    UT_GenStub_SetupReturnBuffer(RPT_VerifyCmdLength, bool);

    UT_GenStub_AddParam(RPT_VerifyCmdLength, const CFE_MSG_Message_t *, MsgPtr);
    UT_GenStub_AddParam(RPT_VerifyCmdLength, size_t, ExpectedLength);

    UT_GenStub_Execute(RPT_VerifyCmdLength, Basic, NULL);

    return UT_GenStub_GetReturnValue(RPT_VerifyCmdLength, bool);
}
