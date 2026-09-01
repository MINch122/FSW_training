/**
 * @file oem_cb_time.c
 * @brief TIME telemetry packet sender callback.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2026.
 */
#include "oem_cb.h"
#include "msg/logs/oem_msg_time.h"

#include "cfe.h"
#include "gps_msgids.h"

 /* original 60 -> lossless compact 47 B */
typedef struct __attribute__((packed)) {
    OEM_Log_Head_t Head;
    uint8_t        clockStatus;
    double         Offset;
    double         Offsetstd;
    double         UtcOffset;
    uint16_t       UtcYear;
    uint8_t        UtcMonth;
    uint8_t        UtcDay;
    uint8_t        UtcHour;
    uint8_t        UtcMin;
    uint16_t       UtcMs;
    uint8_t        UtcStatus;
} OEM_Log_Time_Payload_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    OEM_Log_Time_Payload_t    Payload;
} OEM_Log_Time_Tlm_t;

static OEM_Log_Time_Tlm_t TimeTlm;
static bool               TimeTlmReady = false;

int oem_callback_TIME(void* msg)
{
    const oem_binary_header_t* hdr;
    const oem_log_time*        time;

    if (!msg)
        return OEM_ERR_NULL;

    hdr = msg;
    if (hdr->messageID != OEM_ID_LOG_TIME)
        return OEM_ERR_INVALID;

    if (hdr->messageLength != sizeof(oem_log_time))
        return OEM_ERR_LOG_BODY_SIZE;

    time = (const oem_log_time*) (hdr + 1);

    if (!TimeTlmReady) {
        CFE_MSG_Init(CFE_MSG_PTR(TimeTlm.TelemetryHeader),
                     CFE_SB_ValueToMsgId(GPS_OEM_TIME_TLM_MID), sizeof(TimeTlm));
        TimeTlmReady = true;
    }

    OEM_CB_FillHead(&TimeTlm.Payload.Head, hdr);
    TimeTlm.Payload.clockStatus = OEM_CB_U8Sat(time->clockStatus);
    TimeTlm.Payload.Offset      = time->Offset;
    TimeTlm.Payload.Offsetstd   = time->Offsetstd;
    TimeTlm.Payload.UtcOffset   = time->UtcOffset;
    TimeTlm.Payload.UtcYear     = OEM_CB_U16Sat(time->UtcYear);
    TimeTlm.Payload.UtcMonth    = time->UtcMonth;
    TimeTlm.Payload.UtcDay      = time->UtcDay;
    TimeTlm.Payload.UtcHour     = time->UtcHour;
    TimeTlm.Payload.UtcMin      = time->UtcMin;
    TimeTlm.Payload.UtcMs       = OEM_CB_U16Sat(time->UtcMs);
    TimeTlm.Payload.UtcStatus   = OEM_CB_U8Sat(time->UtcStatus);

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(TimeTlm.TelemetryHeader));
    if (CFE_SB_TransmitMsg(CFE_MSG_PTR(TimeTlm.TelemetryHeader), true) != CFE_SUCCESS)
        return OEM_ERR_IO_WRITE;

    return OEM_OK;
}
