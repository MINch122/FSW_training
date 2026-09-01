/**
 * @file oem_cb_bestpos.c
 * @brief BESTPOS telemetry packet sender callback.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2026.
 */
#include <string.h>

#include "oem_cb.h"
#include "msg/logs/oem_msg_bestpos.h"

#include "cfe.h"
#include "gps_msgids.h"

 /* original 88 -> lossless compact 75 B */
typedef struct __attribute__((packed)) {
    OEM_Log_Head_t Head;
    uint8_t        solStatus;
    uint8_t        posType;
    double         lat;
    double         lon;
    double         hgt;
    float          undulation;
    uint8_t        datumId;
    float          latStd;
    float          lonStd;
    float          hgtStd;
    char           stnId[4];
    float          diffAge;
    float          solAge;
    uint8_t        numSats;
    uint8_t        numSolnSats;
    uint8_t        numSolnL1Sats;
    uint8_t        numSolnMultiSats;
    uint8_t        extSolStat;
    uint8_t        galileoAndBeidouSigMask;
    uint8_t        gpsAndGlonassSigMask;
} OEM_Log_BestPos_Payload_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    OEM_Log_BestPos_Payload_t Payload;
} OEM_Log_BestPos_Tlm_t;

static OEM_Log_BestPos_Tlm_t BestPosTlm;
static bool                  BestPosTlmReady = false;

int oem_callback_BESTPOS(void* msg)
{
    const oem_binary_header_t* hdr;
    const oem_log_bestpos*     bestpos;

    if (!msg)
        return OEM_ERR_NULL;

    hdr = msg;
    if (hdr->messageID != OEM_ID_LOG_BESTPOS)
        return OEM_ERR_INVALID;

    if (hdr->messageLength != sizeof(oem_log_bestpos))
        return OEM_ERR_LOG_BODY_SIZE;

    bestpos = (const oem_log_bestpos*) (hdr + 1);

    if (!BestPosTlmReady) {
        CFE_MSG_Init(CFE_MSG_PTR(BestPosTlm.TelemetryHeader),
                     CFE_SB_ValueToMsgId(GPS_OEM_BESTPOS_TLM_MID), sizeof(BestPosTlm));
        BestPosTlmReady = true;
    }

    OEM_CB_FillHead(&BestPosTlm.Payload.Head, hdr);
    BestPosTlm.Payload.solStatus               = OEM_CB_U8Sat(bestpos->solStatus);
    BestPosTlm.Payload.posType                 = OEM_CB_U8Sat(bestpos->posType);
    BestPosTlm.Payload.lat                     = bestpos->lat;
    BestPosTlm.Payload.lon                     = bestpos->lon;
    BestPosTlm.Payload.hgt                     = bestpos->hgt;
    BestPosTlm.Payload.undulation              = bestpos->undulation;
    BestPosTlm.Payload.datumId                 = OEM_CB_U8Sat(bestpos->datumId);
    BestPosTlm.Payload.latStd                  = bestpos->latStd;
    BestPosTlm.Payload.lonStd                  = bestpos->lonStd;
    BestPosTlm.Payload.hgtStd                  = bestpos->hgtStd;
    BestPosTlm.Payload.diffAge                 = bestpos->diffAge;
    BestPosTlm.Payload.solAge                  = bestpos->solAge;
    BestPosTlm.Payload.numSats                 = bestpos->numSats;
    BestPosTlm.Payload.numSolnSats             = bestpos->numSolnSats;
    BestPosTlm.Payload.numSolnL1Sats           = bestpos->numSolnL1Sats;
    BestPosTlm.Payload.numSolnMultiSats        = bestpos->numSolnMultiSats;
    BestPosTlm.Payload.extSolStat              = bestpos->extSolStat;
    BestPosTlm.Payload.galileoAndBeidouSigMask = bestpos->galileoAndBeidouSigMask;
    BestPosTlm.Payload.gpsAndGlonassSigMask    = bestpos->gpsAndGlonassSigMask;
    memcpy(BestPosTlm.Payload.stnId, bestpos->stnId, sizeof(BestPosTlm.Payload.stnId));

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(BestPosTlm.TelemetryHeader));
    if (CFE_SB_TransmitMsg(CFE_MSG_PTR(BestPosTlm.TelemetryHeader), true) != CFE_SUCCESS)
        return OEM_ERR_IO_WRITE;

    return OEM_OK;
}
