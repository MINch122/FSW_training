/**
 * @file
 *
 * Main header file for the RPT application
 */

/**
 * @file
 *
 * Auto-Generated stub implementations for functions defined in rpt_task header
 */

#include "rpt_task.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_ForwardCommand()
 * ----------------------------------------------------
 */
void RPT_ForwardCommand(void)
{

    UT_GenStub_Execute(RPT_ForwardCommand, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_ForwardReport()
 * ----------------------------------------------------
 */
void RPT_ForwardReport(void)
{

    UT_GenStub_Execute(RPT_ForwardReport, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_FowardCritical()
 * ----------------------------------------------------
 */
void RPT_FowardCritical(void)
{

    UT_GenStub_Execute(RPT_FowardCritical, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_Init()
 * ----------------------------------------------------
 */
CFE_Status_t RPT_Init(void)
{
    UT_GenStub_SetupReturnBuffer(RPT_Init, CFE_Status_t);

    UT_GenStub_Execute(RPT_Init, Basic, NULL);

    return UT_GenStub_GetReturnValue(RPT_Init, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_Main()
 * ----------------------------------------------------
 */
void RPT_Main(void)
{

    UT_GenStub_Execute(RPT_Main, Basic, NULL);
}
