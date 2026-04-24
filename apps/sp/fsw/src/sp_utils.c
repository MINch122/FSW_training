#include "sp_task.h"
#include "sp_eventids.h"
#include "sp_utils.h"

void SP_HandleReport(int32 Status, uint8 CC, const void *ReadData, uint16 ReadSize) {
    CFE_SB_Buffer_t *BufPtr = CFE_SB_AllocateMessageBuffer(sizeof(SP_ReportTlm_t));
	if (BufPtr == NULL) return;

	SP_ReportTlm_t *Report = (SP_ReportTlm_t *)BufPtr;
	if (CFE_MSG_Init(CFE_MSG_PTR(Report->TelemetryHeader), CFE_SB_ValueToMsgId(SP_REPORT_TLM_MID), sizeof(SP_ReportTlm_t)) != CFE_SUCCESS) {
		CFE_SB_ReleaseMessageBuffer(BufPtr);
		return;
	}

	Report->Report.MsgID = SP_CMD_MID;
	Report->Report.CommandCode = CC;
	Report->Report.ReturnType = (Status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
	Report->Report.ReturnCode = Status; // `cfe_error.h`
	Report->Report.ReturnDataSize = (ReadSize > RPT_RET_VALUE_BUF_SIZE) ? RPT_RET_VALUE_BUF_SIZE : ReadSize;
	if (ReadSize && ReadData) {
		memcpy(Report->Report.ReturnValue, ReadData, Report->Report.ReturnDataSize);
	}

	CFE_SB_TimeStampMsg((CFE_MSG_PTR(Report->TelemetryHeader)));
	if (CFE_SB_TransmitBuffer(BufPtr, true) != CFE_SUCCESS) {
		CFE_SB_ReleaseMessageBuffer(BufPtr);
		return;
	}
	return;
}
