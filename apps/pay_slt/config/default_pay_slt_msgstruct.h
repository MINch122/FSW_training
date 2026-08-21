/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 ************************************************************************/

#ifndef PAY_SLT_MSGSTRUCT_H
#define PAY_SLT_MSGSTRUCT_H

#include "pay_slt_mission_cfg.h"
#include "pay_slt_msgdefs.h"
#include "cfe_msg_hdr.h"
#include "rpt_interface_cfg.h"

typedef struct __attribute__((__packed__))
{
    CFE_MSG_CommandHeader_t CommandHeader;
} PAY_SLT_NoArgsCmd_t;

typedef struct __attribute__((__packed__))
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8 Arg;
} PAY_SLT_U8Cmd_t;

typedef struct __attribute__((__packed__))
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint16 Arg;
} PAY_SLT_U16Cmd_t;

typedef struct __attribute__((__packed__))
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint32 Arg;
} PAY_SLT_U32Cmd_t;


typedef PAY_SLT_NoArgsCmd_t PAY_SLT_NoopCmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_ResetCountersCmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_SendHkCmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_SendBcnCmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_RS422PingCmd_t;

typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_CSP_CMP_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_CSP_PING_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_CSP_PS_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_CSP_MEM_FREE_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_CSP_REBOOT_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_CSP_BUF_FREE_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_CSP_UPTIME_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_CSP_GNDWDT_Cmd_t;


typedef struct __attribute__((__packed__))
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    PAY_SLT_HkTlm_Payload_t Payload;
} PAY_SLT_HkTlm_t;

typedef struct __attribute__((__packed__))
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    PAY_SLT_BcnTlm_Payload_t Payload;
} PAY_SLT_BcnTlm_t;

typedef struct __attribute__((__packed__))
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Report;
} PAY_SLT_RPT_t;


/* 만들어 보아요 */
 
typedef struct __attribute__((__packed__)) {
    CFE_MSG_CommandHeader_t       CommandHeader;
    PAY_SLT_OutputEnabled_Payload_t    Payload;
} PAY_SLT_OutputEnabledCmd_t;

typedef struct __attribute__((__packed__))
{
    CFE_MSG_CommandHeader_t       CommandHeader;
    PAY_SLT_ParGet_Payload_t      Payload;
} PAY_SLT_ParGetCmd_t;

typedef struct __attribute__((__packed__))
{
    CFE_MSG_CommandHeader_t       CommandHeader;
    PAY_SLT_ParSet_Payload_t      Payload;
} PAY_SLT_ParSetCmd_t;

typedef struct __attribute__((__packed__))
{
    CFE_MSG_CommandHeader_t           CommandHeader;
    PAY_SLT_ParSetArray_Payload_t     Payload;
} PAY_SLT_ParSetArrayCmd_t;

typedef struct __attribute__((__packed__))
{
    CFE_MSG_CommandHeader_t       CommandHeader;
    PAY_SLT_ScanFiles_Payload_t   Payload;
} PAY_SLT_ScanFilesCmd_t;

typedef struct __attribute__((__packed__))
{
    CFE_MSG_CommandHeader_t        CommandHeader;
    PAY_SLT_DownloadFile_Payload_t Payload;
} PAY_SLT_DownloadFileCmd_t;


typedef struct __attribute__((__packed__))
{
    CFE_MSG_CommandHeader_t        CommandHeader;
    PAY_SLT_GetFullTable_Payload_t Payload;
} PAY_SLT_GetFullTableCmd_t;

#endif
