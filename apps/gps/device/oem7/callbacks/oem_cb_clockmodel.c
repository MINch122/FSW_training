/**
 * @file oem_cb_clockmodel.c
 * @brief CLOCKMODEL telemetry packet sender callback.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2026.
 */
#include <string.h>

#include "oem_cb.h"
#include "msg/logs/oem_msg_clockmodel.h"

#include "cfe.h"
#include "gps_msgids.h"

 /* original 148 -> lossless compact 138 B */
typedef struct __attribute__((packed)) {
    OEM_Log_Head_t Head;
    uint8_t        clockStatus;
    uint32_t       reject;
    int32_t        noiseTime;
    int32_t        updateTime;
    double         parameters[3];
    double         covData[9];
    double         rangeBias;
    double         rangeBiasRate;
} OEM_Log_ClockModel_Payload_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t    TelemetryHeader;
    OEM_Log_ClockModel_Payload_t Payload;
} OEM_Log_ClockModel_Tlm_t;

static OEM_Log_ClockModel_Tlm_t ClockModelTlm;
static bool                     ClockModelTlmReady = false;

int oem_callback_CLOCKMODEL(void* msg)
{
    const oem_binary_header_t* hdr;
    const oem_log_clockmodel*  clockmodel;

    if (!msg)
        return OEM_ERR_NULL;

    hdr = msg;
    if (hdr->messageID != OEM_ID_LOG_CLOCKMODEL)
        return OEM_ERR_INVALID;

    if (hdr->messageLength != sizeof(oem_log_clockmodel))
        return OEM_ERR_LOG_BODY_SIZE;

    clockmodel = (const oem_log_clockmodel*) (hdr + 1);

    if (!ClockModelTlmReady) {
        CFE_MSG_Init(CFE_MSG_PTR(ClockModelTlm.TelemetryHeader),
                     CFE_SB_ValueToMsgId(GPS_OEM_CLOCKMODEL_TLM_MID), sizeof(ClockModelTlm));
        ClockModelTlmReady = true;
    }

    OEM_CB_FillHead(&ClockModelTlm.Payload.Head, hdr);
    ClockModelTlm.Payload.clockStatus   = OEM_CB_U8Sat(clockmodel->clockStatus);
    ClockModelTlm.Payload.reject        = clockmodel->reject;
    ClockModelTlm.Payload.noiseTime     = clockmodel->noiseTime;
    ClockModelTlm.Payload.updateTime    = clockmodel->updateTime;
    ClockModelTlm.Payload.rangeBias     = clockmodel->rangeBias;
    ClockModelTlm.Payload.rangeBiasRate = clockmodel->rangeBiasRate;
    memcpy(ClockModelTlm.Payload.parameters, clockmodel->parameters,
           sizeof(ClockModelTlm.Payload.parameters));
    memcpy(ClockModelTlm.Payload.covData, clockmodel->covData,
           sizeof(ClockModelTlm.Payload.covData));

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(ClockModelTlm.TelemetryHeader));
    if (CFE_SB_TransmitMsg(CFE_MSG_PTR(ClockModelTlm.TelemetryHeader), true) != CFE_SUCCESS)
        return OEM_ERR_IO_WRITE;

    return OEM_OK;
}
