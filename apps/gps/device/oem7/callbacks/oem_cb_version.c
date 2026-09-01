/**
 * @file oem_cb_version.c
 * @brief VERSION log print callback. Test purpose.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2026.
 */
#include "msg/logs/oem_msg_version.h"
#include "oem_utils.h"

#include <stdio.h>

/**
 * @brief Test-only console dump of the VERSION log.
 */
int oem_callback_VERSION_print(void* msg)
{
    const oem_binary_header_t* hdr;
    const oem_log_version*     version;
    size_t                     max_comp;

    if (!msg)
        return OEM_ERR_NULL;

    hdr = msg;
    if (hdr->messageID != OEM_ID_LOG_VERSION) {
        oem_debug_error("invalid MID for version log: expected %d, got %d\n",
                        OEM_ID_LOG_VERSION,
                        hdr->messageID);
        return OEM_ERR_INVALID;
    }

    version = (const oem_log_version*) (hdr + 1);

    if (hdr->messageLength < sizeof(version->numComp))
        return OEM_ERR_LOG_BODY_SIZE;

    max_comp = (hdr->messageLength - sizeof(version->numComp))
               / sizeof(oem_log_version_comp);

    if (version->numComp < 0 || (oem_ulong) version->numComp > max_comp) {
        oem_debug_error("version log claims %d components, body holds %u\n",
                        version->numComp, (unsigned int) max_comp);
        return OEM_ERR_LOG_BODY_SIZE;
    }

    printf("OEM VERSION LOG\n");
    for (int i = 0; i < (int) version->numComp; i++) {
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
