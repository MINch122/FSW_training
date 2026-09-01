/**
 * @file oem_cb_satvis2.c
 * @brief SATVIS2 telemetry packet sender callback.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2026.
 */
#include <stddef.h>

#include "oem_cb.h"
#include "msg/logs/oem_msg_satvis2.h"

#include "cfe.h"
#include "gps_msgids.h"

/** Fixed part of the body: satSystem, satVis, compAlm, numSat. */
#define OEM_CB_SATVIS2_BASE (3 * sizeof(oem_enum) + sizeof(oem_ulong))

/**
 * Max satellites carried in the outgoing telemetry.
 */
#define OEM_CB_SATVIS2_MAX_SAT 50

 /* original 40 -> lossless compact 35 B */
typedef struct __attribute__((packed)) {
    uint8_t satId;
    int8_t  gloFreq;
    uint8_t health;
    double  elev;
    double  az;
    double  trueDop;
    double  appDop;
} OEM_Log_SatVis2_Comp_t;

 /* original 2032 -> lossless compact 1767 B at cap */
typedef struct __attribute__((packed)) {
    OEM_Log_Head_t         Head;
    uint8_t                satSystem;
    uint8_t                satVis;
    uint8_t                compAlm;
    uint8_t                numSat;
    OEM_Log_SatVis2_Comp_t comp[OEM_CB_SATVIS2_MAX_SAT];
} OEM_Log_SatVis2_Payload_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    OEM_Log_SatVis2_Payload_t Payload;
} OEM_Log_SatVis2_Tlm_t;

static OEM_Log_SatVis2_Tlm_t SatVis2Tlm;
static bool                  SatVis2TlmReady = false;

int oem_callback_SATVIS2(void* msg)
{
    const oem_binary_header_t* hdr;
    const oem_log_satvis2*     satvis2;
    oem_ulong                  Count;
    oem_ulong                  i;
    size_t                     Size;

    if (!msg)
        return OEM_ERR_NULL;

    hdr = msg;
    if (hdr->messageID != OEM_ID_LOG_SATVIS2)
        return OEM_ERR_INVALID;

    if (hdr->messageLength < OEM_CB_SATVIS2_BASE)
        return OEM_ERR_LOG_BODY_SIZE;

    satvis2 = (const oem_log_satvis2*) (hdr + 1);
    Count   = satvis2->numSat;

    if (Count > (hdr->messageLength - OEM_CB_SATVIS2_BASE) / sizeof(oem_log_satvis2_comp))
        return OEM_ERR_LOG_BODY_SIZE;

    if (Count > OEM_CB_SATVIS2_MAX_SAT)
        return OEM_ERR_RANGE;

    if (!SatVis2TlmReady) {
        CFE_MSG_Init(CFE_MSG_PTR(SatVis2Tlm.TelemetryHeader),
                     CFE_SB_ValueToMsgId(GPS_OEM_SATVIS2_TLM_MID), sizeof(SatVis2Tlm));
        SatVis2TlmReady = true;
    }

    OEM_CB_FillHead(&SatVis2Tlm.Payload.Head, hdr);
    SatVis2Tlm.Payload.satSystem = OEM_CB_U8Sat(satvis2->satSystem);
    SatVis2Tlm.Payload.satVis    = (satvis2->satVis != 0);
    SatVis2Tlm.Payload.compAlm   = (satvis2->compAlm != 0);
    SatVis2Tlm.Payload.numSat    = OEM_CB_U8Sat(Count);

    for (i = 0; i < Count; ++i) {
        oem_ulong Id = satvis2->comp[i].satId;

        SatVis2Tlm.Payload.comp[i].satId   = OEM_CB_U8Sat(Id & 0xFFFFU);
        SatVis2Tlm.Payload.comp[i].gloFreq = (int8_t)(int16_t)(Id >> 16);
        SatVis2Tlm.Payload.comp[i].health  = OEM_CB_U8Sat(satvis2->comp[i].health);
        SatVis2Tlm.Payload.comp[i].elev    = satvis2->comp[i].elev;
        SatVis2Tlm.Payload.comp[i].az      = satvis2->comp[i].az;
        SatVis2Tlm.Payload.comp[i].trueDop = satvis2->comp[i].trueDop;
        SatVis2Tlm.Payload.comp[i].appDop  = satvis2->comp[i].appDop;
    }

    Size = sizeof(CFE_MSG_TelemetryHeader_t) + offsetof(OEM_Log_SatVis2_Payload_t, comp)
           + (size_t) Count * sizeof(OEM_Log_SatVis2_Comp_t);

    CFE_MSG_SetSize(CFE_MSG_PTR(SatVis2Tlm.TelemetryHeader), Size);
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(SatVis2Tlm.TelemetryHeader));
    if (CFE_SB_TransmitMsg(CFE_MSG_PTR(SatVis2Tlm.TelemetryHeader), true) != CFE_SUCCESS)
        return OEM_ERR_IO_WRITE;

    return OEM_OK;
}
