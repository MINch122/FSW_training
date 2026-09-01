/**
 * @file oem_cb_range.c
 * @brief RANGE telemetry packet sender callback.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2026.
 */
#include <stddef.h>

#include "oem_cb.h"
#include "msg/logs/oem_msg_range.h"

#include "cfe.h"
#include "gps_msgids.h"

/**
 * Max observations carried in the outgoing telemetry.
 */
#define OEM_CB_RANGE_MAX_OBS 45

 /* original 44 -> lossless compact 42 B */
typedef struct __attribute__((packed)) {
    uint8_t  prnSlot;
    uint8_t  gloFreq;
    double   psr;
    float    psrStd;
    double   adr;
    float    adrStd;
    float    dopp;
    float    CN0;
    float    locktime;
    uint32_t chTrStatus;
} OEM_Log_Range_Comp_t;

 /* original 1980 -> lossless compact 1904 B at cap */
typedef struct __attribute__((packed)) {
    OEM_Log_Head_t       Head;
    uint8_t              numObs;
    OEM_Log_Range_Comp_t comp[OEM_CB_RANGE_MAX_OBS];
} OEM_Log_Range_Payload_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    OEM_Log_Range_Payload_t   Payload;
} OEM_Log_Range_Tlm_t;

static OEM_Log_Range_Tlm_t RangeTlm;
static bool                RangeTlmReady = false;

int oem_callback_RANGE(void* msg)
{
    const oem_binary_header_t* hdr;
    const oem_log_range*       range;
    oem_ulong                  Count;
    size_t                     Size;

    if (!msg)
        return OEM_ERR_NULL;

    hdr = msg;
    if (hdr->messageID != OEM_ID_LOG_RANGE)
        return OEM_ERR_INVALID;

    if (hdr->messageLength < sizeof(range->numObs))
        return OEM_ERR_LOG_BODY_SIZE;

    range = (const oem_log_range*) (hdr + 1);
    Count = range->numObs;

    if (Count > (hdr->messageLength - sizeof(range->numObs)) / sizeof(oem_log_range_comp))
        return OEM_ERR_LOG_BODY_SIZE;

    if (Count > OEM_CB_RANGE_MAX_OBS)
        return OEM_ERR_RANGE;

    if (!RangeTlmReady) {
        CFE_MSG_Init(CFE_MSG_PTR(RangeTlm.TelemetryHeader),
                     CFE_SB_ValueToMsgId(GPS_OEM_RANGE_TLM_MID), sizeof(RangeTlm));
        RangeTlmReady = true;
    }

    OEM_CB_FillHead(&RangeTlm.Payload.Head, hdr);
    RangeTlm.Payload.numObs = OEM_CB_U8Sat(Count);
    for (oem_ulong i = 0; i < Count; ++i) {
        RangeTlm.Payload.comp[i].prnSlot     = OEM_CB_U8Sat(range->comp[i].prnSlot);
        RangeTlm.Payload.comp[i].gloFreq     = OEM_CB_U8Sat(range->comp[i].gloFreq);
        RangeTlm.Payload.comp[i].psr         = range->comp[i].psr;
        RangeTlm.Payload.comp[i].psrStd      = range->comp[i].psrStd;
        RangeTlm.Payload.comp[i].adr         = range->comp[i].adr;
        RangeTlm.Payload.comp[i].adrStd      = range->comp[i].adrStd;
        RangeTlm.Payload.comp[i].dopp        = range->comp[i].dopp;
        RangeTlm.Payload.comp[i].CN0         = range->comp[i].CN0;
        RangeTlm.Payload.comp[i].locktime    = range->comp[i].locktime;
        RangeTlm.Payload.comp[i].chTrStatus  = range->comp[i].chTrStatus;
    }

    Size = sizeof(CFE_MSG_TelemetryHeader_t) + offsetof(OEM_Log_Range_Payload_t, comp)
           + (size_t) Count * sizeof(OEM_Log_Range_Comp_t);

    CFE_MSG_SetSize(CFE_MSG_PTR(RangeTlm.TelemetryHeader), Size);
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(RangeTlm.TelemetryHeader));
    if (CFE_SB_TransmitMsg(CFE_MSG_PTR(RangeTlm.TelemetryHeader), true) != CFE_SUCCESS)
        return OEM_ERR_IO_WRITE;

    return OEM_OK;
}
