#include "utrx_cmds_ax100.h"
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include "cfe_msg.h"
#include "cfe_sb.h"
#include "rpt_interface_cfg.h"
#include "utrx_app_msgids.h"

/* ---------- 공통 헬퍼 ---------- */

static inline void UTRX_RptBegin(void)
{
    /* 전체 패킷 0 초기화 후 헤더 초기화 */
    memset(&UTRX_APP_Data.RptPkt, 0, sizeof(UTRX_APP_Data.RptPkt));

    CFE_MSG_Init(CFE_MSG_PTR(UTRX_APP_Data.RptPkt.TelemetryHeader),
                 CFE_SB_ValueToMsgId(UTRX_APP_RPT_TLM_MID),  /* <- 앱에서 정의한 TLM MID 사용 */
                 sizeof(UTRX_APP_Data.RptPkt));             /* 통째로 전송 */
}

static inline void UTRX_RptSetStatusAuto(uint8_t cc, int32_t status)
{
    UTRX_APP_Data.RptPkt.Report.CommandCode = cc;

    if (status == DEVICE_SUCCESS) {
        UTRX_APP_Data.RptPkt.Report.ReturnType = RPT_RETTYPE_SUCCESS;
        UTRX_APP_Data.RptPkt.Report.ReturnCode = DEVICE_SUCCESS;
    } else {
        UTRX_APP_Data.RptPkt.Report.ReturnType = RPT_RETTYPE_HW;
        UTRX_APP_Data.RptPkt.Report.ReturnCode = status;
    }
}

/* 스칼라/가변 모두 사용 가능: 길이 상한 및 ReturnDataSize 설정 포함 */
static inline void UTRX_RptCopy(const void *src, size_t len)
{
    size_t n = len;

    /* 필수: ReturnValue 버퍼 크기 이하로 자르기 (오버런 방지) */
    size_t maxbuf = sizeof(UTRX_APP_Data.RptPkt.Report.ReturnValue);
    if (n > maxbuf) n = maxbuf;

    memcpy(UTRX_APP_Data.RptPkt.Report.ReturnValue, src, n);
    UTRX_APP_Data.RptPkt.Report.ReturnDataSize = (uint16_t)n;
}


static inline void UTRX_RptEnd(void)
{
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(UTRX_APP_Data.RptPkt.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(UTRX_APP_Data.RptPkt.TelemetryHeader), true);
}

/******************** SET *********************************************************/

void UTRX_AX100_GndwdtClearCmd(const UTRX_AX100_GndwdtClear_t *Msg)
{
    (void)Msg;
    UTRX_RptBegin();

    int32_t status = UTRX_GndwdtClear();
    UTRX_RptSetStatusAuto(UTRX_GNDWDT_CLEAR_CC, status);
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed, Status=%" PRId32,
                          UTRX_GNDWDT_CLEAR_CC, status);
    }
    UTRX_RptEnd();
}

void UTRX_AX100_UTRX_RebootCmd(const UTRX_AX100_UTRX_Reboot_t *Msg)
{
    (void)Msg;
    UTRX_RptBegin();

    int32_t status = UTRX_Reboot();
    UTRX_RptSetStatusAuto(UTRX_REBOOT_CC, status);
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed, Status=%" PRId32,
                          UTRX_REBOOT_CC, status);
    }
    UTRX_RptEnd();
}

void UTRX_AX100_RXCONF_SetBaudCmd(const UTRX_AX100_RXCONF_SetBaudCmd_t *Msg)
{
    UTRX_RptBegin();

    int32_t status = UTRX_RXCONF_SetBaud(Msg->arg);
    UTRX_RptSetStatusAuto(UTRX_RXCONF_SET_BAUD_CC, status);
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed (arg=%u), Status=%" PRId32,
                          UTRX_RXCONF_SET_BAUD_CC, (unsigned)Msg->arg, status);
    }
    UTRX_RptEnd();
}

void UTRX_AX100_TXCONF_SetBaudCmd(const UTRX_AX100_TXCONF_SetBaudCmd_t *Msg)
{
    UTRX_RptBegin();

    int32_t status = UTRX_TXCONF_SetBaud(Msg->arg);
    UTRX_RptSetStatusAuto(UTRX_TXCONF_SET_BAUD_CC, status);
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed (arg=%u), Status=%" PRId32,
                          UTRX_TXCONF_SET_BAUD_CC, (unsigned)Msg->arg, status);
    }
    UTRX_RptEnd();
}

void UTRX_AX100_RXCONF_SetFreqCmd(const UTRX_AX100_RXCONF_SetFreqCmd_t *Msg)
{
    UTRX_RptBegin();

    int32_t status = UTRX_RXCONF_SetFreq(Msg->arg);
    UTRX_RptSetStatusAuto(UTRX_RXCONF_SET_FREQ_CC, status);
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed (arg=%u), Status=%" PRId32,
                          UTRX_RXCONF_SET_FREQ_CC, (unsigned)Msg->arg, status);
    }
    UTRX_RptEnd();
}

void UTRX_AX100_TXCONF_SetFreqCmd(const UTRX_AX100_TXCONF_SetFreqCmd_t *Msg)
{
    UTRX_RptBegin();

    int32_t status = UTRX_TXCONF_SetFreq(Msg->arg);
    UTRX_RptSetStatusAuto(UTRX_TXCONF_SET_FREQ_CC, status);
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed (arg=%u), Status=%" PRId32,
                          UTRX_TXCONF_SET_FREQ_CC, (unsigned)Msg->arg, status);
    }
    UTRX_RptEnd();
}

void UTRX_AX100_SetDefaultBaudCmd(const UTRX_AX100_SetDefaultBaudCmd_t *Msg)
{
    (void)Msg;
    UTRX_RptBegin();

    int32_t status = UTRX_SetDefaultBaud();
    UTRX_RptSetStatusAuto(UTRX_SET_DEFAULT_BAUD_CC, status);
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed, Status=%" PRId32,
                          UTRX_SET_DEFAULT_BAUD_CC, status);
    }
    UTRX_RptEnd();
}

void UTRX_AX100_RparamSave1Cmd(const UTRX_AX100_RparamSave1Cmd_t *Msg)
{
    (void)Msg;
    UTRX_RptBegin();

    int32_t status = UTRX_RparamSave1();
    UTRX_RptSetStatusAuto(UTRX_RPARAM_SAVE_1_CC, status);
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed, Status=%" PRId32,
                          UTRX_RPARAM_SAVE_1_CC, status);
    }
    UTRX_RptEnd();
}

void UTRX_AX100_RparamSave5Cmd(const UTRX_AX100_RparamSave5Cmd_t *Msg)
{
    (void)Msg;
    UTRX_RptBegin();

    int32_t status = UTRX_RparamSave5();
    UTRX_RptSetStatusAuto(UTRX_RPARAM_SAVE_5_CC, status);
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed, Status=%" PRId32,
                          UTRX_RPARAM_SAVE_5_CC, status);
    }
    UTRX_RptEnd();
}

void UTRX_AX100_RparamSaveAllCmd(const UTRX_AX100_RparamSaveAllCmd_t *Msg)
{
    (void)Msg;
    UTRX_RptBegin();

    int32_t status = UTRX_RparamSaveAll();
    UTRX_RptSetStatusAuto(UTRX_RPARAM_SAVE_ALL_CC, status);
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed, Status=%" PRId32,
                          UTRX_RPARAM_SAVE_ALL_CC, status);
    }
    UTRX_RptEnd();
}

void UTRX_AX100_CheckStatePingCmd(const UTRX_AX100_CheckStatePingCmd_t *Msg)
{
    (void)Msg;
    UTRX_RptBegin();

    int32_t status = csp_checkstate_ping(CSP_NODE_UTRX);
    UTRX_RptSetStatusAuto(UTRX_CHECK_STATE_PING_CC, status);
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed, Status=%" PRId32,
                          UTRX_CHECK_STATE_PING_CC, status);
    }
    UTRX_RptEnd();
}

/************************ GET *****************************************************/

void UTRX_AX100_RXCONF_GetBaudCmd(const UTRX_AX100_GetRxBaudCmd_t *Msg)
{
    (void)Msg;
    UTRX_RptBegin();

    uint32  baud = 0;
    int32_t status = UTRX_RXCONF_GetBaud(&baud);

    UTRX_RptSetStatusAuto(UTRX_RXCONF_GET_BAUD_CC, status);
    if (status == DEVICE_SUCCESS) UTRX_RptCopy(&baud, sizeof(baud));
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed, Status=%" PRId32,
                          UTRX_RXCONF_GET_BAUD_CC, status);
    } else {
        OS_printf("[UTRX] RX Baud: %u\n", (unsigned)baud);
    }
    UTRX_RptEnd();
}

void UTRX_AX100_RXCONF_GetGuardCmd(const UTRX_AX100_GetRxGuardCmd_t *Msg)
{
    (void)Msg;
    UTRX_RptBegin();

    uint16  val = 0;
    int32_t status = UTRX_RXCONF_GetGuard(&val);

    UTRX_RptSetStatusAuto(UTRX_RXCONF_GET_GUARD_CC, status);
    if (status == DEVICE_SUCCESS) UTRX_RptCopy(&val, sizeof(val));
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed, Status=%" PRId32,
                          UTRX_RXCONF_GET_GUARD_CC, status);
    } else {
        OS_printf("[UTRX] RX GUARD: %u\n", (unsigned)val);
    }
    UTRX_RptEnd();
}

void UTRX_AX100_RXCONF_GetFreqCmd(const UTRX_AX100_GetRxFreqCmd_t *Msg)
{
    (void)Msg;
    UTRX_RptBegin();

    uint32  val = 0;
    int32_t status = UTRX_RXCONF_GetFreq(&val);

    UTRX_RptSetStatusAuto(UTRX_RXCONF_GET_FREQ_CC, status);
    if (status == DEVICE_SUCCESS) UTRX_RptCopy(&val, sizeof(val));
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed, Status=%" PRId32,
                          UTRX_RXCONF_GET_FREQ_CC, status);
    } else {
        OS_printf("[UTRX] RX Freq: %u\n", (unsigned)val);
    }
    UTRX_RptEnd();
}

/* --- TXCONF --- */

void UTRX_AX100_TXCONF_GetBaudCmd(const UTRX_AX100_GetTxBaudCmd_t *Msg)
{
    (void)Msg;
    UTRX_RptBegin();

    uint32  val = 0;
    int32_t status = UTRX_TXCONF_GetBaud(&val);

    UTRX_RptSetStatusAuto(UTRX_TXCONF_GET_BAUD_CC, status);
    if (status == DEVICE_SUCCESS) UTRX_RptCopy(&val, sizeof(val));
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed, Status=%" PRId32,
                          UTRX_TXCONF_GET_BAUD_CC, status);
    } else {
        OS_printf("[UTRX] TX Baud: %u\n", (unsigned)val);
    }
    UTRX_RptEnd();
}

void UTRX_AX100_TXCONF_GetFreqCmd(const UTRX_AX100_GetTxFreqCmd_t *Msg)
{
    (void)Msg;
    UTRX_RptBegin();

    uint32  val = 0;
    int32_t status = UTRX_TXCONF_GetFreq(&val);

    UTRX_RptSetStatusAuto(UTRX_TXCONF_GET_FREQ_CC, status);
    if (status == DEVICE_SUCCESS) UTRX_RptCopy(&val, sizeof(val));
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed, Status=%" PRId32,
                          UTRX_TXCONF_GET_FREQ_CC, status);
    } else {
        OS_printf("[UTRX] TX Freq: %u\n", (unsigned)val);
    }
    UTRX_RptEnd();
}

/* --- TLM (int16 항목 주의) --- */

void UTRX_AX100_TLM_GetTempBrdCmd(const UTRX_AX100_GetTempBrdCmd_t *Msg)
{
    (void)Msg;
    UTRX_RptBegin();

    int16_t val = 0;
    int32_t status = UTRX_TLM_GetTempBrd(&val);

    UTRX_RptSetStatusAuto(UTRX_TLM_GET_TEMP_BRD_CC, status);
    if (status == DEVICE_SUCCESS) UTRX_RptCopy(&val, sizeof(val));
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed, Status=%" PRId32,
                          UTRX_TLM_GET_TEMP_BRD_CC, status);
    } else {
        OS_printf("[UTRX] TEMPERATURE: %d\n", (int)val);
    }
    UTRX_RptEnd();
}

void UTRX_AX100_TLM_GetLastRssiCmd(const UTRX_AX100_GetLastRssiCmd_t *Msg)
{
    (void)Msg;
    UTRX_RptBegin();

    int16_t val = 0;
    int32_t status = UTRX_TLM_GetLastRssi(&val);

    UTRX_RptSetStatusAuto(UTRX_TLM_GET_LAST_RSSI_CC, status);
    if (status == DEVICE_SUCCESS) UTRX_RptCopy(&val, sizeof(val));
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed, Status=%" PRId32,
                          UTRX_TLM_GET_LAST_RSSI_CC, status);
    } else {
        OS_printf("[UTRX] LAST RSSI: %d\n", (int)val);
    }
    UTRX_RptEnd();
}

void UTRX_AX100_TLM_GetLastRferrCmd(const UTRX_AX100_GetLastRferrCmd_t *Msg)
{
    (void)Msg;
    UTRX_RptBegin();

    int16_t val = 0;
    int32_t status = UTRX_TLM_GetLastRferr(&val);

    UTRX_RptSetStatusAuto(UTRX_TLM_GET_LAST_RFERR_CC, status);
    if (status == DEVICE_SUCCESS) UTRX_RptCopy(&val, sizeof(val));
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed, Status=%" PRId32,
                          UTRX_TLM_GET_LAST_RFERR_CC, status);
    } else {
        OS_printf("[UTRX] LAST RFERR: %d\n", (int)val);
    }
    UTRX_RptEnd();
}

void UTRX_AX100_TLM_GetActiveConfCmd(const UTRX_AX100_GetActiveConfCmd_t *Msg)
{
    (void)Msg;
    UTRX_RptBegin();

    uint8   val = 0;
    int32_t status = UTRX_TLM_GetActiveConf(&val);

    UTRX_RptSetStatusAuto(UTRX_TLM_GET_ACTIVE_CONF_CC, status);
    if (status == DEVICE_SUCCESS) UTRX_RptCopy(&val, sizeof(val));
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed, Status=%" PRId32,
                          UTRX_TLM_GET_ACTIVE_CONF_CC, status);
    } else {
        OS_printf("[UTRX] ACTIVE CONF: %u\n", (unsigned)val);
    }
    UTRX_RptEnd();
}

void UTRX_AX100_TLM_GetBootCountCmd(const UTRX_AX100_GetBootCountCmd_t *Msg)
{
    (void)Msg;
    UTRX_RptBegin();

    uint16  val = 0;
    int32_t status = UTRX_TLM_GetBootCount(&val);

    UTRX_RptSetStatusAuto(UTRX_TLM_GET_BOOT_COUNT_CC, status);
    if (status == DEVICE_SUCCESS) UTRX_RptCopy(&val, sizeof(val));
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed, Status=%" PRId32,
                          UTRX_TLM_GET_BOOT_COUNT_CC, status);
    } else {
        OS_printf("[UTRX] BOOT COUNT: %u\n", (unsigned)val);
    }
    UTRX_RptEnd();
}

void UTRX_AX100_TLM_GetBootCauseCmd(const UTRX_AX100_GetBootCauseCmd_t *Msg)
{
    (void)Msg;
    UTRX_RptBegin();

    uint32  val = 0;
    int32_t status = UTRX_TLM_GetBootCause(&val);

    UTRX_RptSetStatusAuto(UTRX_TLM_GET_BOOT_CAUSE_CC, status);
    if (status == DEVICE_SUCCESS) UTRX_RptCopy(&val, sizeof(val));
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed, Status=%" PRId32,
                          UTRX_TLM_GET_BOOT_CAUSE_CC, status);
    } else {
        OS_printf("[UTRX] BOOT CAUSE: %u\n", (unsigned)val);
    }
    UTRX_RptEnd();
}

void UTRX_AX100_TLM_GetLastContactCmd(const UTRX_AX100_GetLastContactCmd_t *Msg)
{
    (void)Msg;
    UTRX_RptBegin();

    uint32  val = 0;
    int32_t status = UTRX_TLM_GetLastContact(&val);

    UTRX_RptSetStatusAuto(UTRX_TLM_GET_LAST_CONTACT_CC, status);
    if (status == DEVICE_SUCCESS) UTRX_RptCopy(&val, sizeof(val));
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed, Status=%" PRId32,
                          UTRX_TLM_GET_LAST_CONTACT_CC, status);
    } else {
        OS_printf("[UTRX] LAST CONTACT: %u\n", (unsigned)val);
    }
    UTRX_RptEnd();
}

void UTRX_AX100_TLM_GetTotTxBytesCmd(const UTRX_AX100_GetTotTxBytesCmd_t *Msg)
{
    (void)Msg;
    UTRX_RptBegin();

    uint32  val = 0;
    int32_t status = UTRX_TLM_GetTotTxBytes(&val);

    UTRX_RptSetStatusAuto(UTRX_TLM_GET_TOT_TX_BYTES_CC, status);
    if (status == DEVICE_SUCCESS) UTRX_RptCopy(&val, sizeof(val));
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed, Status=%" PRId32,
                          UTRX_TLM_GET_TOT_TX_BYTES_CC, status);
    } else {
        OS_printf("[UTRX] TOTAL TX BYTES: %u\n", (unsigned)val);
    }
    UTRX_RptEnd();
}

void UTRX_AX100_TLM_GetTotRxBytesCmd(const UTRX_AX100_GetTotRxBytesCmd_t *Msg)
{
    (void)Msg;
    UTRX_RptBegin();

    uint32  val = 0;
    int32_t status = UTRX_TLM_GetTotRxBytes(&val);

    UTRX_RptSetStatusAuto(UTRX_TLM_GET_TOT_RX_BYTES_CC, status);
    if (status == DEVICE_SUCCESS) UTRX_RptCopy(&val, sizeof(val));
    UTRX_CountFromReport();

    if (status != DEVICE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX AX100: CC=%u failed, Status=%" PRId32,
                          UTRX_TLM_GET_TOT_RX_BYTES_CC, status);
    } else {
        OS_printf("[UTRX] TOTAL RX BYTES: %u\n", (unsigned)val);
    }
    UTRX_RptEnd();
}