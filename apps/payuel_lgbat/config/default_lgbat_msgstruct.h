#ifndef LGBAT_MSGSTRUCT_H
#define LGBAT_MSGSTRUCT_H

#include "lgbat_mission_cfg.h"
#include "lgbat_msgdefs.h"
#include "cfe.h"
#include "rpt_interface_cfg.h"

typedef struct {
    CFE_MSG_TelemetryHeader_t  TelemetryHeader;
    LGBAT_HkTlm_Payload_t      Payload;
} LGBAT_HkTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t  TelemetryHeader;
    RPT_Report_t                Payload;
} LGBAT_ReportTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t  TelemetryHeader;
    LGBAT_BcnTlm_Payload_t     Payload;
} LGBAT_BcnTlm_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t      TelemetryHeader;
    LGBAT_CriticalTlm_Payload_t    Payload;
} LGBAT_CriticalTlm_t;


typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} LGBAT_NoopCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} LGBAT_ResetCounterCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} LGBAT_SendBcnCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8_t DataID;
    uint8_t Spare[3];
} LGBAT_RequestDataCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} LGBAT_RequestAllDataCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8_t PowerOn;
    uint8_t Spare[3];
} LGBAT_SetPowerCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} LGBAT_ResetBmsCmd_t;

#endif /* LGBAT_MSGSTRUCT_H */
