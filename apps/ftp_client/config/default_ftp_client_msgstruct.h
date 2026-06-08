#ifndef FTP_CLIENT_MSGSTRUCT_H
#define FTP_CLIENT_MSGSTRUCT_H

#include "ftp_client_mission_cfg.h"
#include "ftp_client_msgdefs.h"
#include "cfe_msg_hdr.h"
#include "rpt_interface_cfg.h"

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} FTP_CLIENT_NoArgCmd_t;

typedef FTP_CLIENT_NoArgCmd_t FTP_CLIENT_NoopCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    FTP_CLIENT_Transfer_Payload_t Payload;
} FTP_CLIENT_UploadCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    FTP_CLIENT_Transfer_Payload_t Payload;
} FTP_CLIENT_DownloadCmd_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Payload;
} FTP_CLIENT_ReportTlm_t;

#endif
