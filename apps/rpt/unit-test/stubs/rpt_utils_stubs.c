/**
 * @file
 *
 * Main header file for the RPT util function
 */

/**
 * @file
 *
 * Auto-Generated stub implementations for functions defined in rpt_utils header
 */

#include "rpt_utils.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_CalculateCRC()
 * ----------------------------------------------------
 */
uint32 RPT_CalculateCRC(const void *Data, size_t Size)
{
    UT_GenStub_SetupReturnBuffer(RPT_CalculateCRC, uint32);

    UT_GenStub_AddParam(RPT_CalculateCRC, const void *, Data);
    UT_GenStub_AddParam(RPT_CalculateCRC, size_t, Size);

    UT_GenStub_Execute(RPT_CalculateCRC, Basic, NULL);

    return UT_GenStub_GetReturnValue(RPT_CalculateCRC, uint32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_CalculateResetCause()
 * ----------------------------------------------------
 */
uint8 RPT_CalculateResetCause(uint8 ResetType, uint8 ResetSubType)
{
    UT_GenStub_SetupReturnBuffer(RPT_CalculateResetCause, uint8);

    UT_GenStub_AddParam(RPT_CalculateResetCause, uint8, ResetType);
    UT_GenStub_AddParam(RPT_CalculateResetCause, uint8, ResetSubType);

    UT_GenStub_Execute(RPT_CalculateResetCause, Basic, NULL);

    return UT_GenStub_GetReturnValue(RPT_CalculateResetCause, uint8);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_CloseFile()
 * ----------------------------------------------------
 */
int32 RPT_CloseFile(osal_id_t FD)
{
    UT_GenStub_SetupReturnBuffer(RPT_CloseFile, int32);

    UT_GenStub_AddParam(RPT_CloseFile, osal_id_t, FD);

    UT_GenStub_Execute(RPT_CloseFile, Basic, NULL);

    return UT_GenStub_GetReturnValue(RPT_CloseFile, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_Enqueue()
 * ----------------------------------------------------
 */
void RPT_Enqueue(const RPT_Report_t *Report, bool IsCritical)
{
    UT_GenStub_AddParam(RPT_Enqueue, const RPT_Report_t *, Report);
    UT_GenStub_AddParam(RPT_Enqueue, bool, IsCritical);

    UT_GenStub_Execute(RPT_Enqueue, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_MultipleCritical()
 * ----------------------------------------------------
 */
int32 RPT_MultipleCritical(uint8_t StartIdx, uint8_t TotNum)
{
    UT_GenStub_SetupReturnBuffer(RPT_MultipleCritical, int32);

    UT_GenStub_AddParam(RPT_MultipleCritical, uint8_t, StartIdx);
    UT_GenStub_AddParam(RPT_MultipleCritical, uint8_t, TotNum);

    UT_GenStub_Execute(RPT_MultipleCritical, Basic, NULL);

    return UT_GenStub_GetReturnValue(RPT_MultipleCritical, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_MultipleReport()
 * ----------------------------------------------------
 */
int32 RPT_MultipleReport(uint8_t StartIdx, uint8_t TotNum)
{
    UT_GenStub_SetupReturnBuffer(RPT_MultipleReport, int32);

    UT_GenStub_AddParam(RPT_MultipleReport, uint8_t, StartIdx);
    UT_GenStub_AddParam(RPT_MultipleReport, uint8_t, TotNum);

    UT_GenStub_Execute(RPT_MultipleReport, Basic, NULL);

    return UT_GenStub_GetReturnValue(RPT_MultipleReport, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_OpenCriticalFile()
 * ----------------------------------------------------
 */
osal_id_t RPT_OpenCriticalFile(void)
{
    UT_GenStub_SetupReturnBuffer(RPT_OpenCriticalFile, osal_id_t);

    UT_GenStub_Execute(RPT_OpenCriticalFile, Basic, NULL);

    return UT_GenStub_GetReturnValue(RPT_OpenCriticalFile, osal_id_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_OpenOpsFile()
 * ----------------------------------------------------
 */
osal_id_t RPT_OpenOpsFile(void)
{
    UT_GenStub_SetupReturnBuffer(RPT_OpenOpsFile, osal_id_t);

    UT_GenStub_Execute(RPT_OpenOpsFile, Basic, NULL);

    return UT_GenStub_GetReturnValue(RPT_OpenOpsFile, osal_id_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_OpenOpsBackupFile()
 * ----------------------------------------------------
 */
osal_id_t RPT_OpenOpsBackupFile(uint32 Timestamp)
{
    UT_GenStub_SetupReturnBuffer(RPT_OpenOpsBackupFile, osal_id_t);

    UT_GenStub_AddParam(RPT_OpenOpsBackupFile, uint32, Timestamp);

    UT_GenStub_Execute(RPT_OpenOpsBackupFile, Basic, NULL);

    return UT_GenStub_GetReturnValue(RPT_OpenOpsBackupFile, osal_id_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_ReadFromFile()
 * ----------------------------------------------------
 */
int32 RPT_ReadFromFile(osal_id_t FD, void *Data, size_t Size)
{
    UT_GenStub_SetupReturnBuffer(RPT_ReadFromFile, int32);

    UT_GenStub_AddParam(RPT_ReadFromFile, osal_id_t, FD);
    UT_GenStub_AddParam(RPT_ReadFromFile, void *, Data);
    UT_GenStub_AddParam(RPT_ReadFromFile, size_t, Size);

    UT_GenStub_Execute(RPT_ReadFromFile, Basic, NULL);

    return UT_GenStub_GetReturnValue(RPT_ReadFromFile, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_Report()
 * ----------------------------------------------------
 */
int32 RPT_Report(const RPT_Report_t *Report, bool IsCritical)
{
    UT_GenStub_SetupReturnBuffer(RPT_Report, int32);

    UT_GenStub_AddParam(RPT_Report, const RPT_Report_t *, Report);
    UT_GenStub_AddParam(RPT_Report, bool, IsCritical);

    UT_GenStub_Execute(RPT_Report, Basic, NULL);

    return UT_GenStub_GetReturnValue(RPT_Report, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_Subscribe()
 * ----------------------------------------------------
 */
void RPT_Subscribe(void)
{

    UT_GenStub_Execute(RPT_Subscribe, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_VerifyReportLength()
 * ----------------------------------------------------
 */
bool RPT_VerifyReportLength(const CFE_MSG_Message_t *MsgPtr)
{
    UT_GenStub_SetupReturnBuffer(RPT_VerifyReportLength, bool);

    UT_GenStub_AddParam(RPT_VerifyReportLength, const CFE_MSG_Message_t *, MsgPtr);

    UT_GenStub_Execute(RPT_VerifyReportLength, Basic, NULL);

    return UT_GenStub_GetReturnValue(RPT_VerifyReportLength, bool);
}

/*
 * ----------------------------------------------------
 * Generated stub function for RPT_WriteToFile()
 * ----------------------------------------------------
 */
int32 RPT_WriteToFile(osal_id_t FD, const void *Data, size_t Size)
{
    UT_GenStub_SetupReturnBuffer(RPT_WriteToFile, int32);

    UT_GenStub_AddParam(RPT_WriteToFile, osal_id_t, FD);
    UT_GenStub_AddParam(RPT_WriteToFile, const void *, Data);
    UT_GenStub_AddParam(RPT_WriteToFile, size_t, Size);

    UT_GenStub_Execute(RPT_WriteToFile, Basic, NULL);

    return UT_GenStub_GetReturnValue(RPT_WriteToFile, int32);
}
