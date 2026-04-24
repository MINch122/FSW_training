#include "msg/logs/oem_msg_hwmonitor.h"

#include <stdio.h>

int OEM_Callback_HWMONITOR(void* msg)
{
    const oem_binary_header_t* hdr;
    const oem_log_hwmonitor* hwmonitor;
    const oem_log_hwmonitor_comp* comp;

    if (!msg) {
        return OEM_ERR_NULL;
    }

    hdr = msg;
    hwmonitor = (const oem_log_hwmonitor*) (hdr + 1);

    if (hdr->messageLength < sizeof(hwmonitor->numMeasurements) ||
        hdr->messageLength != 
            sizeof(hwmonitor->numMeasurements) 
                + hwmonitor->numMeasurements*sizeof(*comp))
        return OEM_ERR_LEN_MSG;

#if OEM_DEBUG
    printf("HWMONITOR (%d meas):\n", hwmonitor->numMeasurements);
#endif
    for (oem_ulong i = 0; i < hwmonitor->numMeasurements; ++i) {
        comp = &hwmonitor->comp[i];
#if OEM_DEBUG
        printf("\tComp #%d:\n", i);
        printf("\t\tReading: %f\n", comp->reading);
        printf("\t\tStatus: %d (0x%X)\n", comp->status, comp->status);
#endif
    }
#if OEM_DEBUG
    printf("\n");
#endif
    return OEM_OK;


}
