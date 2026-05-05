#ifndef LGBAT_MSGSTRUCT_H
#define LGBAT_MSGSTRUCT_H

#include "lgbat_mission_cfg.h"
#include "lgbat_msgdefs.h"
#include "cfe.h"
#include "rpt_interface_cfg.h"



// Telemetry messages (CCSDS TelemetryHeader + payload)


// HK Telemetry (0x08C6): sent on every SEND_HK_MID trigger
typedef struct {
    CFE_MSG_TelemetryHeader_t  TelemetryHeader;
    LGBAT_HkTlm_Payload_t      Payload;
} LGBAT_HkTlm_t;

// Report Telemetry (0x08C7): sent after every ground command
typedef struct {
    CFE_MSG_TelemetryHeader_t  TelemetryHeader;
    RPT_Report_t                Payload;
} LGBAT_ReportTlm_t;

// Beacon Telemetry (0x08C8): key BMS data for ground downlink
typedef struct {
    CFE_MSG_TelemetryHeader_t  TelemetryHeader;
    LGBAT_BcnTlm_Payload_t     Payload;
} LGBAT_BcnTlm_t;

// Critical Alert Telemetry: sent when a BMS fault or warning is detected
typedef struct {
    CFE_MSG_TelemetryHeader_t      TelemetryHeader;
    LGBAT_CriticalTlm_Payload_t    Payload;
} LGBAT_CriticalTlm_t;




// Command messages (CCSDS CommandHeader + parameters)


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
    uint8_t DataID;   // BMS Data ID to read (0x01 to 0x0C)
    uint8_t Spare[3]; // Alignment padding
} LGBAT_RequestDataCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} LGBAT_RequestAllDataCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8_t PowerOn;  // 1 = apply 3.3V, 0 = remove 3.3V
    uint8_t Spare[3]; // Alignment padding
} LGBAT_SetPowerCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} LGBAT_ResetBmsCmd_t;

#endif // LGBAT_MSGSTRUCT_H
