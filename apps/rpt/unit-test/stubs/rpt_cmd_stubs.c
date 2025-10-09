/**
 * @file
 *
 * Main header file for the RPT Command
 */

/**
 * @file
 *
 * Auto-Generated stub implementations for functions defined in rpt_cmd header
 */

#include "rpt_cmd.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_ClearQueueCmd()
 * ----------------------------------------------------
 */
CFE_Status_t RPT_ClearQueueCmd(const RPT_ClearQueueCmd_t *Msg)
{
    UT_GenStub_SetupReturnBuffer(RPT_ClearQueueCmd, CFE_Status_t);

    UT_GenStub_AddParam(RPT_ClearQueueCmd, const RPT_ClearQueueCmd_t *, Msg);

    UT_GenStub_Execute(RPT_ClearQueueCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(RPT_ClearQueueCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_NoopCmd()
 * ----------------------------------------------------
 */
CFE_Status_t RPT_NoopCmd(const RPT_NoopCmd_t *Msg)
{
    UT_GenStub_SetupReturnBuffer(RPT_NoopCmd, CFE_Status_t);

    UT_GenStub_AddParam(RPT_NoopCmd, const RPT_NoopCmd_t *, Msg);

    UT_GenStub_Execute(RPT_NoopCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(RPT_NoopCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_ReportCmd()
 * ----------------------------------------------------
 */
CFE_Status_t RPT_ReportCmd(const RPT_ReportCmd_t *Msg)
{
    UT_GenStub_SetupReturnBuffer(RPT_ReportCmd, CFE_Status_t);

    UT_GenStub_AddParam(RPT_ReportCmd, const RPT_ReportCmd_t *, Msg);

    UT_GenStub_Execute(RPT_ReportCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(RPT_ReportCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_ResetCounterCmd()
 * ----------------------------------------------------
 */
CFE_Status_t RPT_ResetCounterCmd(const RPT_ResetCounterCmd_t *Msg)
{
    UT_GenStub_SetupReturnBuffer(RPT_ResetCounterCmd, CFE_Status_t);

    UT_GenStub_AddParam(RPT_ResetCounterCmd, const RPT_ResetCounterCmd_t *, Msg);

    UT_GenStub_Execute(RPT_ResetCounterCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(RPT_ResetCounterCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_SendBeaconCmd()
 * ----------------------------------------------------
 */
CFE_Status_t RPT_SendBeaconCmd(void)
{
    UT_GenStub_SetupReturnBuffer(RPT_SendBeaconCmd, CFE_Status_t);

    UT_GenStub_Execute(RPT_SendBeaconCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(RPT_SendBeaconCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_SendHKCmd()
 * ----------------------------------------------------
 */
CFE_Status_t RPT_SendHKCmd(void)
{
    UT_GenStub_SetupReturnBuffer(RPT_SendHKCmd, CFE_Status_t);

    UT_GenStub_Execute(RPT_SendHKCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(RPT_SendHKCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_UpdateOperationData()
 * ----------------------------------------------------
 */
void RPT_UpdateOperationData(void)
{

    UT_GenStub_Execute(RPT_UpdateOperationData, Basic, NULL);
}
