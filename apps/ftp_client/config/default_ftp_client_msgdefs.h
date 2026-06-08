#ifndef FTP_CLIENT_MSGDEFS_H
#define FTP_CLIENT_MSGDEFS_H

#include "ftp_client_internal_cfg.h"

typedef struct {
    uint8 Host;
    uint8 Port;
    uint16 Reserved;
    uint32 Timeout;
    uint32 ChunkSize;
    char LocalUrl[FTP_CLIENT_PATH_LEN];
    char RemoteUrl[FTP_CLIENT_PATH_LEN];
} FTP_CLIENT_Transfer_Payload_t;

typedef enum {
    FTP_CLIENT_REPORT_OP_NOOP     = 0,
    FTP_CLIENT_REPORT_OP_UPLOAD   = 1,
    FTP_CLIENT_REPORT_OP_DOWNLOAD = 2
} FTP_CLIENT_ReportOp_t;

typedef enum {
    FTP_CLIENT_REPORT_PHASE_STARTED  = 1,
    FTP_CLIENT_REPORT_PHASE_COMPLETE = 2,
    FTP_CLIENT_REPORT_PHASE_ERROR    = 3
} FTP_CLIENT_ReportPhase_t;

typedef struct {
    uint8 Operation;
    uint8 Phase;
    uint8 Host;
    uint8 Port;
    int32 Status;
    char LocalUrl[FTP_CLIENT_PATH_LEN];
    char RemoteUrl[FTP_CLIENT_PATH_LEN];
} FTP_CLIENT_TransferReportData_t;

#endif
