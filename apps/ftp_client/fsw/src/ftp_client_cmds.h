#ifndef FTP_CLIENT_CMDS_H
#define FTP_CLIENT_CMDS_H

#include "cfe_error.h"
#include "ftp_client_msg.h"

CFE_Status_t FTP_CLIENT_NoopCmd(const FTP_CLIENT_NoopCmd_t *Msg);
CFE_Status_t FTP_CLIENT_UploadCmd(const FTP_CLIENT_UploadCmd_t *Msg);
CFE_Status_t FTP_CLIENT_DownloadCmd(const FTP_CLIENT_DownloadCmd_t *Msg);

#endif
