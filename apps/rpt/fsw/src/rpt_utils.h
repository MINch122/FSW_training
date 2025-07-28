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


int32 RPT_PriorInit(void);
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

int RPT_OpenOpsFile(uint8_t IsBackup);
int32 RPT_WriteToFile(int FD, const void *Data, size_t Size);
int32 RPT_ReadFromFile(int FD, void *Data, size_t Size);
int32 RPT_CloseFile(int FD);

int RPT_OpenCriticalFile(void);

uint32 RPT_CalculateCRC(const void *Data, size_t Size);

#endif