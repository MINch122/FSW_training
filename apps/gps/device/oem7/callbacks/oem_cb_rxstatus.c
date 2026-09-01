/**
 * @file oem_cb_rxstatus.c
 * @brief RXSTATUS telemetry packet sender callback.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2026.
 */
#include <stddef.h>

#include "oem_cb.h"
#include "msg/logs/oem_msg_rxstatus.h"

#include "cfe.h"
#include "gps_msgids.h"

/** Fixed part of the body: error, numStats. */
#define OEM_CB_RXSTATUS_BASE (2 * sizeof(oem_ulong))

/**
 * Max status codes carried in the outgoing telemetry.
 */
#define OEM_CB_RXSTATUS_MAX_STATS 8

 /* original 16 -> lossless compact 16 B, all bit fields */
typedef struct __attribute__((packed)) {
    uint32_t status;
    uint32_t priority;
    uint32_t eventSet;
    uint32_t eventClear;
} OEM_Log_RxStatus_Comp_t;

 /* original 152 -> lossless compact 146 B at cap */
typedef struct __attribute__((packed)) {
    OEM_Log_Head_t          Head;
    uint32_t                error;
    uint8_t                 numStats;
    OEM_Log_RxStatus_Comp_t comp[OEM_CB_RXSTATUS_MAX_STATS];
} OEM_Log_RxStatus_Payload_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t  TelemetryHeader;
    OEM_Log_RxStatus_Payload_t Payload;
} OEM_Log_RxStatus_Tlm_t;

static OEM_Log_RxStatus_Tlm_t RxStatusTlm;
static bool                   RxStatusTlmReady = false;

int oem_callback_RXSTATUS(void* msg)
{
    const oem_binary_header_t* hdr;
    const oem_log_rxstatus*    rxstatus;
    oem_ulong                  Count;
    oem_ulong                  i;
    size_t                     Size;

    if (!msg)
        return OEM_ERR_NULL;

    hdr = msg;
    if (hdr->messageID != OEM_ID_LOG_RXSTATUS)
        return OEM_ERR_INVALID;

    if (hdr->messageLength < OEM_CB_RXSTATUS_BASE)
        return OEM_ERR_LOG_BODY_SIZE;

    rxstatus = (const oem_log_rxstatus*) (hdr + 1);
    Count    = rxstatus->numStats;

    if (Count > (hdr->messageLength - OEM_CB_RXSTATUS_BASE) / sizeof(oem_log_rxstatus_comp))
        return OEM_ERR_LOG_BODY_SIZE;

    if (Count > OEM_CB_RXSTATUS_MAX_STATS)
        return OEM_ERR_RANGE;

    if (!RxStatusTlmReady) {
        CFE_MSG_Init(CFE_MSG_PTR(RxStatusTlm.TelemetryHeader),
                     CFE_SB_ValueToMsgId(GPS_OEM_RXSTATUS_TLM_MID), sizeof(RxStatusTlm));
        RxStatusTlmReady = true;
    }

    OEM_CB_FillHead(&RxStatusTlm.Payload.Head, hdr);
    RxStatusTlm.Payload.error    = rxstatus->error;
    RxStatusTlm.Payload.numStats = OEM_CB_U8Sat(Count);

    for (i = 0; i < Count; ++i) {
        RxStatusTlm.Payload.comp[i].status     = rxstatus->comp[i].status;
        RxStatusTlm.Payload.comp[i].priority   = rxstatus->comp[i].priority;
        RxStatusTlm.Payload.comp[i].eventSet   = rxstatus->comp[i].eventSet;
        RxStatusTlm.Payload.comp[i].eventClear = rxstatus->comp[i].eventClear;
    }

    Size = sizeof(CFE_MSG_TelemetryHeader_t) + offsetof(OEM_Log_RxStatus_Payload_t, comp)
           + (size_t) Count * sizeof(OEM_Log_RxStatus_Comp_t);

    CFE_MSG_SetSize(CFE_MSG_PTR(RxStatusTlm.TelemetryHeader), Size);
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(RxStatusTlm.TelemetryHeader));
    if (CFE_SB_TransmitMsg(CFE_MSG_PTR(RxStatusTlm.TelemetryHeader), true) != CFE_SUCCESS)
        return OEM_ERR_IO_WRITE;

    return OEM_OK;
}
