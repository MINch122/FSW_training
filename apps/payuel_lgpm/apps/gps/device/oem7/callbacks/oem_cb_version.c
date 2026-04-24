#include "msg/logs/oem_msg_version.h"

#include <stdio.h>

int OEM_Callback_VERSION(void* msg)
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

#if OEM_DEBUG
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
#else
    (void) version;
#endif

    return OEM_OK;
}
