#include "ftp_client_app.h"
#include "ftp_client_cmds.h"
#include "ftp_client_eventids.h"

#include <gs/ftp/client.h>

static void FTP_CLIENT_GetSettings(const FTP_CLIENT_Transfer_Payload_t *Payload,
                                   gs_ftp_settings_t *Settings)
{
    gs_ftp_default_settings(Settings);
    Settings->host = (Payload->Host != 0) ? Payload->Host : FTP_CLIENT_DEFAULT_HOST;
    Settings->port = Payload->Port;
    Settings->timeout = Payload->Timeout;
    Settings->chunk_size = Payload->ChunkSize;
}

static void FTP_CLIENT_CopyUrl(char *Dst, size_t DstSize, const char *Src, size_t SrcSize)
{
    size_t CopySize = (DstSize - 1 < SrcSize) ? DstSize - 1 : SrcSize;

    memcpy(Dst, Src, CopySize);
    Dst[CopySize] = 0;
}

static void FTP_CLIENT_FillTransferReport(FTP_CLIENT_TransferReportData_t *Report,
                                          uint8 Operation,
                                          uint8 Phase,
                                          const gs_ftp_settings_t *Settings,
                                          const char *LocalUrl,
                                          const char *RemoteUrl,
                                          int32 Status)
{
    memset(Report, 0, sizeof(*Report));

    Report->Operation = Operation;
    Report->Phase = Phase;
    Report->Host = Settings->host;
    Report->Port = Settings->port;
    Report->Status = Status;
    FTP_CLIENT_CopyUrl(Report->LocalUrl, sizeof(Report->LocalUrl), LocalUrl, strlen(LocalUrl));
    FTP_CLIENT_CopyUrl(Report->RemoteUrl, sizeof(Report->RemoteUrl), RemoteUrl, strlen(RemoteUrl));
}

static void FTP_CLIENT_SendTransferReport(const CFE_MSG_Message_t *CmdMsg,
                                          uint8 Operation,
                                          uint8 Phase,
                                          const gs_ftp_settings_t *Settings,
                                          const char *LocalUrl,
                                          const char *RemoteUrl,
                                          int32 Status,
                                          uint8 ReturnType)
{
    FTP_CLIENT_TransferReportData_t Report;

    FTP_CLIENT_FillTransferReport(&Report, Operation, Phase, Settings, LocalUrl, RemoteUrl, Status);
    FTP_CLIENT_SendReport(CmdMsg, &Report, sizeof(Report), Status, ReturnType);
}

CFE_Status_t FTP_CLIENT_NoopCmd(const FTP_CLIENT_NoopCmd_t *Msg)
{
    FTP_CLIENT_AppData.CmdCounter++;
    CFE_EVS_SendEvent(FTP_CLIENT_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "FTP_CLIENT: NOOP command");
    FTP_CLIENT_SendReport((const CFE_MSG_Message_t *)Msg, NULL, 0, CFE_SUCCESS, RPT_RETTYPE_SUCCESS);

    return CFE_SUCCESS;
}

CFE_Status_t FTP_CLIENT_UploadCmd(const FTP_CLIENT_UploadCmd_t *Msg)
{
    gs_ftp_settings_t Settings;
    char LocalUrl[FTP_CLIENT_PATH_LEN + 1];
    char RemoteUrl[FTP_CLIENT_PATH_LEN + 1];
    gs_error_t Status;

    FTP_CLIENT_GetSettings(&Msg->Payload, &Settings);
    FTP_CLIENT_CopyUrl(LocalUrl, sizeof(LocalUrl), Msg->Payload.LocalUrl, sizeof(Msg->Payload.LocalUrl));
    FTP_CLIENT_CopyUrl(RemoteUrl, sizeof(RemoteUrl), Msg->Payload.RemoteUrl, sizeof(Msg->Payload.RemoteUrl));

    FTP_CLIENT_SendTransferReport((const CFE_MSG_Message_t *)Msg,
                                  FTP_CLIENT_REPORT_OP_UPLOAD,
                                  FTP_CLIENT_REPORT_PHASE_STARTED,
                                  &Settings,
                                  LocalUrl,
                                  RemoteUrl,
                                  CFE_SUCCESS,
                                  RPT_RETTYPE_SUCCESS);

    Status = gs_ftp_upload(&Settings, LocalUrl, RemoteUrl, NULL, NULL);
    if (Status == GS_OK) {
        FTP_CLIENT_AppData.CmdCounter++;
        FTP_CLIENT_SendTransferReport((const CFE_MSG_Message_t *)Msg,
                                      FTP_CLIENT_REPORT_OP_UPLOAD,
                                      FTP_CLIENT_REPORT_PHASE_COMPLETE,
                                      &Settings,
                                      LocalUrl,
                                      RemoteUrl,
                                      CFE_SUCCESS,
                                      RPT_RETTYPE_SUCCESS);
        CFE_EVS_SendEvent(FTP_CLIENT_UPLOAD_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "FTP_CLIENT: upload complete, host=%u local=%s remote=%s",
                          (unsigned int)Settings.host, LocalUrl, RemoteUrl);
        return CFE_SUCCESS;
    }

    FTP_CLIENT_AppData.ErrCounter++;
    FTP_CLIENT_SendTransferReport((const CFE_MSG_Message_t *)Msg,
                                  FTP_CLIENT_REPORT_OP_UPLOAD,
                                  FTP_CLIENT_REPORT_PHASE_ERROR,
                                  &Settings,
                                  LocalUrl,
                                  RemoteUrl,
                                  (int32)Status,
                                  RPT_RETTYPE_LIB);
    CFE_EVS_SendEvent(FTP_CLIENT_TRANSFER_ERR_EID, CFE_EVS_EventType_ERROR,
                      "FTP_CLIENT: upload failed, host=%u status=%d local=%s remote=%s",
                      (unsigned int)Settings.host, (int)Status, LocalUrl, RemoteUrl);
    return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
}

CFE_Status_t FTP_CLIENT_DownloadCmd(const FTP_CLIENT_DownloadCmd_t *Msg)
{
    gs_ftp_settings_t Settings;
    char LocalUrl[FTP_CLIENT_PATH_LEN + 1];
    char RemoteUrl[FTP_CLIENT_PATH_LEN + 1];
    gs_error_t Status;

    FTP_CLIENT_GetSettings(&Msg->Payload, &Settings);
    FTP_CLIENT_CopyUrl(LocalUrl, sizeof(LocalUrl), Msg->Payload.LocalUrl, sizeof(Msg->Payload.LocalUrl));
    FTP_CLIENT_CopyUrl(RemoteUrl, sizeof(RemoteUrl), Msg->Payload.RemoteUrl, sizeof(Msg->Payload.RemoteUrl));

    FTP_CLIENT_SendTransferReport((const CFE_MSG_Message_t *)Msg,
                                  FTP_CLIENT_REPORT_OP_DOWNLOAD,
                                  FTP_CLIENT_REPORT_PHASE_STARTED,
                                  &Settings,
                                  LocalUrl,
                                  RemoteUrl,
                                  CFE_SUCCESS,
                                  RPT_RETTYPE_SUCCESS);

    Status = gs_ftp_download(&Settings, LocalUrl, RemoteUrl, NULL, NULL);
    if (Status == GS_OK) {
        FTP_CLIENT_AppData.CmdCounter++;
        FTP_CLIENT_SendTransferReport((const CFE_MSG_Message_t *)Msg,
                                      FTP_CLIENT_REPORT_OP_DOWNLOAD,
                                      FTP_CLIENT_REPORT_PHASE_COMPLETE,
                                      &Settings,
                                      LocalUrl,
                                      RemoteUrl,
                                      CFE_SUCCESS,
                                      RPT_RETTYPE_SUCCESS);
        CFE_EVS_SendEvent(FTP_CLIENT_DOWNLOAD_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "FTP_CLIENT: download complete, host=%u local=%s remote=%s",
                          (unsigned int)Settings.host, LocalUrl, RemoteUrl);
        return CFE_SUCCESS;
    }

    FTP_CLIENT_AppData.ErrCounter++;
    FTP_CLIENT_SendTransferReport((const CFE_MSG_Message_t *)Msg,
                                  FTP_CLIENT_REPORT_OP_DOWNLOAD,
                                  FTP_CLIENT_REPORT_PHASE_ERROR,
                                  &Settings,
                                  LocalUrl,
                                  RemoteUrl,
                                  (int32)Status,
                                  RPT_RETTYPE_LIB);
    CFE_EVS_SendEvent(FTP_CLIENT_TRANSFER_ERR_EID, CFE_EVS_EventType_ERROR,
                      "FTP_CLIENT: download failed, host=%u status=%d local=%s remote=%s",
                      (unsigned int)Settings.host, (int)Status, LocalUrl, RemoteUrl);
    return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
}
