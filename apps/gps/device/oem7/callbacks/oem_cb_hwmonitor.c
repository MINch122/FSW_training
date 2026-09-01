/**
 * @file oem_cb_hwmonitor.c
 * @brief HWMONITOR telemetry packet sender callback.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2026.
 */
#include <stdio.h>
#include <stddef.h>

#include "oem_cb.h"
#include "msg/logs/oem_msg_hwmonitor.h"
#include "oem_utils.h"

#include "cfe.h"
#include "gps_msgids.h"

/**
 * @brief Test-only console dump of the HWMONITOR log.
 */
int oem_callback_HWMONITOR_print(void* msg)
{
    const oem_binary_header_t*    hdr;
    const oem_log_hwmonitor*      hwmonitor;
    const oem_log_hwmonitor_comp* comp;
    size_t                        max_comp;

    if (!msg)
        return OEM_ERR_NULL;

    hdr = msg;
    hwmonitor = (const oem_log_hwmonitor*) (hdr + 1);

    if (hdr->messageLength < sizeof(hwmonitor->numMeasurements))
        return OEM_ERR_LOG_BODY_SIZE;

    max_comp = (hdr->messageLength - sizeof(hwmonitor->numMeasurements))
               / sizeof(oem_log_hwmonitor_comp);

    if (hwmonitor->numMeasurements > max_comp) {
        oem_debug_error("hwmonitor log claims %u measurements, body holds %u\n",
                        (unsigned int) hwmonitor->numMeasurements,
                        (unsigned int) max_comp);
        return OEM_ERR_LOG_BODY_SIZE;
    }

    printf("HWMONITOR (%u meas):\n", (unsigned int) hwmonitor->numMeasurements);
    for (oem_ulong i = 0; i < hwmonitor->numMeasurements; ++i) {
        comp = &hwmonitor->comp[i];
        printf("\tComp #%u:\n", (unsigned int) i);
        printf("\t\tReading: %f\n", comp->reading);
        printf("\t\tStatus: %u (0x%X)\n",
               (unsigned int) comp->status, (unsigned int) comp->status);
    }
    printf("\n");
    return OEM_OK;
}

/**
 * Max measurements carried in the outgoing telemetry.
 */
#define OEM_CB_HWMONITOR_MAX_MEAS 16

 /* original 8 -> lossless compact 6 B */
typedef struct __attribute__((packed)) {
    float   reading;
    uint8_t boundary;
    uint8_t readingType;
} OEM_Log_HwMonitor_Comp_t;

 /* original 148 -> lossless compact 110 B at cap */
typedef struct __attribute__((packed)) {
    OEM_Log_Head_t           Head;
    uint8_t                  numMeasurements;
    OEM_Log_HwMonitor_Comp_t comp[OEM_CB_HWMONITOR_MAX_MEAS];
} OEM_Log_HwMonitor_Payload_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t   TelemetryHeader;
    OEM_Log_HwMonitor_Payload_t Payload;
} OEM_Log_HwMonitor_Tlm_t;

static OEM_Log_HwMonitor_Tlm_t HwMonitorTlm;
static bool                    HwMonitorTlmReady = false;

int oem_callback_HWMONITOR(void* msg)
{
    const oem_binary_header_t* hdr;
    const oem_log_hwmonitor*   hwmonitor;
    oem_ulong                  Count;
    oem_ulong                  i;
    size_t                     Size;

    if (!msg)
        return OEM_ERR_NULL;

    hdr = msg;
    if (hdr->messageID != OEM_ID_LOG_HWMONITOR)
        return OEM_ERR_INVALID;

    if (hdr->messageLength < sizeof(hwmonitor->numMeasurements))
        return OEM_ERR_LOG_BODY_SIZE;

    hwmonitor = (const oem_log_hwmonitor*) (hdr + 1);
    Count     = hwmonitor->numMeasurements;

    if (Count > (hdr->messageLength - sizeof(hwmonitor->numMeasurements))
                / sizeof(oem_log_hwmonitor_comp))
        return OEM_ERR_LOG_BODY_SIZE;

    if (Count > OEM_CB_HWMONITOR_MAX_MEAS)
        return OEM_ERR_RANGE;

    if (!HwMonitorTlmReady) {
        CFE_MSG_Init(CFE_MSG_PTR(HwMonitorTlm.TelemetryHeader),
                     CFE_SB_ValueToMsgId(GPS_OEM_HWMONITOR_TLM_MID), sizeof(HwMonitorTlm));
        HwMonitorTlmReady = true;
    }

    OEM_CB_FillHead(&HwMonitorTlm.Payload.Head, hdr);
    HwMonitorTlm.Payload.numMeasurements = OEM_CB_U8Sat(Count);

    for (i = 0; i < Count; ++i) {
        HwMonitorTlm.Payload.comp[i].reading     = hwmonitor->comp[i].reading;
        HwMonitorTlm.Payload.comp[i].boundary    = hwmonitor->comp[i].status & 0xFFU;
        HwMonitorTlm.Payload.comp[i].readingType = (hwmonitor->comp[i].status >> 8) & 0xFFU;
    }

    Size = sizeof(CFE_MSG_TelemetryHeader_t) + offsetof(OEM_Log_HwMonitor_Payload_t, comp)
           + (size_t) Count * sizeof(OEM_Log_HwMonitor_Comp_t);

    CFE_MSG_SetSize(CFE_MSG_PTR(HwMonitorTlm.TelemetryHeader), Size);
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(HwMonitorTlm.TelemetryHeader));
    if (CFE_SB_TransmitMsg(CFE_MSG_PTR(HwMonitorTlm.TelemetryHeader), true) != CFE_SUCCESS)
        return OEM_ERR_IO_WRITE;

    return OEM_OK;
}
