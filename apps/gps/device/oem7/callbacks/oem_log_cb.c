#include "oem.h"

#include <stdio.h>

// int OEM_Log_Callback_TIME(void* msg) {

//     return OEM_OK;
// }

int OEM_Log_Callback_LOGLIST(void* msg)
{
    const oem_binary_header_t* hdr;
    const oem_log_loglist* loglist;
    const oem_log_loglist_comp* comp;

    if (!msg) {
        return OEM_ERR_NULL;
    }

    hdr = msg;
    loglist = (const oem_log_loglist*) (hdr + 1);

    if (hdr->messageLength < sizeof(loglist->numlogs) ||
        hdr->messageLength != 
            sizeof(loglist->numlogs) + loglist->numlogs*sizeof(*comp))
        return OEM_ERR_LEN_MSG;

#if OEM_DEBUG
    printf("LOGLIST (%d logs):\n", loglist->numlogs);
#endif
    for (oem_ulong i = 0; i < loglist->numlogs; ++i) {
        comp = &loglist->comp[i];
#if OEM_DEBUG
        printf("\tLog #%d:\n", i);
        printf("\t\tMessage: %d\n", comp->message);
        printf("\t\tPort: %d (0x%X)\n", comp->port, comp->port);
        printf("\t\tTrigger: %d\n", comp->trigger);
        printf("\t\tPeriod: %f\n", comp->period);
        printf("\t\tOffset: %f\n", comp->offset);
        printf("\n");
#endif
    }

    return OEM_OK;
}

int OEM_Log_Callback_HWMONITOR(void* msg)
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

#if OEM_DEBUG
int OEM_Log_Callback_VERSION(void* msg)
{
    const oem_binary_header_t* hdr;
    const oem_log_version* version;
    if (!msg) {
        printf("! NULL VERSION LOG \n!");
        return -1;
    }
    hdr = msg;
    version = (const oem_log_version*) (hdr + 1);
    if (hdr->messageID != OEM_ID_LOG_VERSION) {
        printf("invalid MID for version log: expected %d, got %d\n",
               OEM_ID_LOG_VERSION,
               hdr->messageID);
        return OEM_ERR_INVALID;
    }
    printf("OEM VERSION LOG\n");
    for (int i = 0; i < version->numComp; i++) {
        printf("\tComponent %d:\n",        i);
        printf("\t\tmodel: %.16s\n",       version->comp[i].model);
        printf("\t\tpsn: %.16s\n",         version->comp[i].psn);
        printf("\t\thwVersion: %.16s\n",   version->comp[i].hwVersion);
        printf("\t\tswVersion: %.16s\n",   version->comp[i].swVersion);
        printf("\t\tbootVersion: %.16s\n", version->comp[i].bootVersion);
        printf("\t\tcompDate: %.16s\n",    version->comp[i].compDate);
        printf("\t\tcompTime: %.16s\n",    version->comp[i].compTime);
    }
    return OEM_OK;
}
#endif
