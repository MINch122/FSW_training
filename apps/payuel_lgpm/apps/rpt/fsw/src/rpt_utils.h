/**
 * @file
 *
 * Main header file for the RPT util function
 */

#ifndef RPT_UTILS_H
#define RPT_UTILS_H

#include "common_types.h"
#include "rpt_mission_cfg.h"


typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Payload;
} RPT_ReportTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Critical_t Payload;
} RPT_CriticalTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;

    RPT_Report_t Payload[];
} RPT_MultipleReportTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;

    RPT_Critical_t Payload[];
} RPT_MultipleCriticalTlm_t;



void RPT_Subscribe(void);
void RPT_Enqueue(const RPT_Report_t *Report, bool IsCritical);

int32 RPT_Report(const RPT_Report_t *Report, bool IsCritical);

/// @brief Get multiple report from Report Queue
/// @param StartIdx Offset from oldest
/// @param TotNum Total Report number
/// @return Status
int32 RPT_MultipleReport(uint8_t StartIdx, uint8_t TotNum);

/// @brief Get multiple report from Critical Queue
/// @param StartIdx Offset from oldest
/// @param TotNum Total Report number
/// @return Status
int32 RPT_MultipleCritical(uint8_t StartIdx, uint8_t TotNum);

bool RPT_VerifyReportLength(const CFE_MSG_Message_t *MsgPtr);

osal_id_t RPT_OpenOpsFile(uint8_t IsBackup);
int32 RPT_WriteToFile(osal_id_t FD, const void *Data, size_t Size);
int32 RPT_ReadFromFile(osal_id_t FD, void *Data, size_t Size);
int32 RPT_CloseFile(osal_id_t FD);

osal_id_t RPT_OpenCriticalFile(void);

uint32 RPT_CalculateCRC(const void *Data, size_t Size);

/// @brief Calculate Reset value via bit operation
/// @param ResetType Reset Type earned by `CFE_ES_GetResetType`
/// @param ResetSubType Reset SubType earned by `CFE_ES_GetResetType`
/// @return Calculated U8 value
uint8 RPT_CalculateResetCause(uint8 ResetType, uint8 ResetSubType);

#endif