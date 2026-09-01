/**
 * @file oem_cb_bestxyz.c
 * @brief BESTXYZ telemetry packet sender callback.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2026.
 */
#include "oem_cb.h"
#include "msg/logs/oem_msg_bestxyz.h"

#include "cfe.h"
#include "gps_msgids.h"

 /* original 102 -> lossless compact 99 B */
typedef struct __attribute__((packed)) {
    OEM_Log_Head_t Head;
    uint8_t        pSolStatus;
    uint8_t        posType;
    double         pX;
    double         pY;
    double         pZ;
    float          pXstd;
    float          pYstd;
    float          pZstd;
    uint8_t        vSolStatus;
    uint8_t        velType;
    double         vX;
    double         vY;
    double         vZ;
    float          vXstd;
    float          vYstd;
    float          vZstd;
    float          vLatancy;
    float          diffAge;
    uint8_t        numSats;
    uint8_t        numSolnSats;
} OEM_Log_BestXyz_Payload_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    OEM_Log_BestXyz_Payload_t Payload;
} OEM_Log_BestXyz_Tlm_t;

static OEM_Log_BestXyz_Tlm_t BestXyzTlm;
static bool                  BestXyzTlmReady = false;

int oem_callback_BESTXYZ(void* msg)
{
    const oem_binary_header_t* hdr;
    const oem_log_bestxyz*     bestxyz;

    if (!msg)
        return OEM_ERR_NULL;

    hdr = msg;
    if (hdr->messageID != OEM_ID_LOG_BESTXYZ)
        return OEM_ERR_INVALID;

    if (hdr->messageLength != sizeof(*bestxyz))
        return OEM_ERR_LOG_BODY_SIZE;

    bestxyz = (const oem_log_bestxyz*) (hdr + 1);

    if (bestxyz->pSolStatus != OEM_BESTXYZ_SOLSTAT_SOL_COMPUTED &&
        bestxyz->vSolStatus != OEM_BESTXYZ_SOLSTAT_SOL_COMPUTED) {
        return OEM_OK;
    }

    /* Init once: CFE_MSG_Init restarts the sequence count, which the ground
     * needs in order to see dropped packets. */
    if (!BestXyzTlmReady) {
        CFE_MSG_Init(CFE_MSG_PTR(BestXyzTlm.TelemetryHeader),
                     CFE_SB_ValueToMsgId(GPS_OEM_BESTXYZ_TLM_MID), sizeof(BestXyzTlm));
        BestXyzTlmReady = true;
    }

    OEM_CB_FillHead(&BestXyzTlm.Payload.Head, hdr);

    BestXyzTlm.Payload.pSolStatus   = OEM_CB_U8Sat(bestxyz->pSolStatus);
    BestXyzTlm.Payload.posType      = OEM_CB_U8Sat(bestxyz->posType);
    BestXyzTlm.Payload.pX           = bestxyz->pX;
    BestXyzTlm.Payload.pY           = bestxyz->pY;
    BestXyzTlm.Payload.pZ           = bestxyz->pZ;
    BestXyzTlm.Payload.pXstd        = bestxyz->pXstd;
    BestXyzTlm.Payload.pYstd        = bestxyz->pYstd;
    BestXyzTlm.Payload.pZstd        = bestxyz->pZstd;
    BestXyzTlm.Payload.vSolStatus   = OEM_CB_U8Sat(bestxyz->vSolStatus);
    BestXyzTlm.Payload.velType      = OEM_CB_U8Sat(bestxyz->velType);
    BestXyzTlm.Payload.vX           = bestxyz->vX;
    BestXyzTlm.Payload.vY           = bestxyz->vY;
    BestXyzTlm.Payload.vZ           = bestxyz->vZ;
    BestXyzTlm.Payload.vXstd        = bestxyz->vXstd;
    BestXyzTlm.Payload.vYstd        = bestxyz->vYstd;
    BestXyzTlm.Payload.vZstd        = bestxyz->vZstd;
    BestXyzTlm.Payload.vLatancy     = bestxyz->vLatancy;
    BestXyzTlm.Payload.diffAge      = bestxyz->diffAge;
    BestXyzTlm.Payload.numSats      = bestxyz->numSats;
    BestXyzTlm.Payload.numSolnSats  = bestxyz->numSolnSats;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(BestXyzTlm.TelemetryHeader));
    if (CFE_SB_TransmitMsg(CFE_MSG_PTR(BestXyzTlm.TelemetryHeader), true) != CFE_SUCCESS)
        return OEM_ERR_IO_WRITE;

    return OEM_OK;
}
