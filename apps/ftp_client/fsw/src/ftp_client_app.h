#ifndef FTP_CLIENT_APP_H
#define FTP_CLIENT_APP_H

#include "cfe.h"

#include "ftp_client_internal_cfg.h"
#include "ftp_client_msgids.h"
#include "ftp_client_msg.h"
#include "ftp_client_perfids.h"

typedef struct {
    uint8 CmdCounter;
    uint8 ErrCounter;
    FTP_CLIENT_ReportTlm_t ReportTlm;
    uint32 RunStatus;
    CFE_SB_PipeId_t CommandPipe;
    char PipeName[CFE_MISSION_MAX_API_LEN];
    uint16 PipeDepth;
} FTP_CLIENT_AppData_t;

extern FTP_CLIENT_AppData_t FTP_CLIENT_AppData;

void FTP_CLIENT_Main(void);
CFE_Status_t FTP_CLIENT_Init(void);
void FTP_CLIENT_SendReport(const CFE_MSG_Message_t *CmdMsg,
                           const void *Data,
                           uint16 DataSize,
                           int32 ReturnCode,
                           uint8 ReturnType);

#endif
