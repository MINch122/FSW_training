#include "ftp_task.h"
#include "ftp_utils.h"
#include "rpt_interface_cfg.h"

void FTP_LocalTimerCallback(osal_id_t object_id, void *arg) {
    FTP_Data.RunFlag = false;
    return;
}

void FTP_HandleReport(int32 Status, uint8_t ReturnType, uint8_t CC, void *Data, size_t Size) {
    FTP_Data.Report.Report.MsgID = FTP_CMD_MID;
    FTP_Data.Report.Report.CommandCode = CC;
    FTP_Data.Report.Report.ReturnType = ReturnType;
    FTP_Data.Report.Report.ReturnCode = Status;
    FTP_Data.Report.Report.ReturnDataSize = Size;
    memcpy(FTP_Data.Report.Report.ReturnValue, Data, Size);

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(FTP_Data.Report.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(FTP_Data.Report.TelemetryHeader), true);

    memset(FTP_Data.Report.Report.ReturnValue, 0, sizeof(FTP_Data.Report.Report.ReturnValue));
    return;
}