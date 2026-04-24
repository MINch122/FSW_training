#include "oem_cb.h"
#include "msg/logs/oem_msg_bestxyz.h"

#include <stdio.h>

static FILE* binfd = NULL;

int OEM_Callback_BESTXYZ_Bin(void* msg)
{
    oem_binary_header_t* hdr;
    oem_log_bestxyz* bestxyz;

    if (!msg) {
        return OEM_ERR_NULL;
    }

    if (binfd == NULL) {
        binfd = fopen(OEM_CALLBACK_LOG_SAVE_PATH_ROOT "bestxyz.bin", "a");
        if (binfd == NULL) {
            return OEM_ERR_FILE_OPEN;
        }
    }

    hdr = msg;
    bestxyz = (oem_log_bestxyz*) (hdr + 1);

    if (hdr->messageLength < sizeof(*bestxyz))
        return OEM_ERR_LEN_MSG;

    if (bestxyz->pSolStatus != OEM_BESTXYZ_SOLSTAT_SOL_COMPUTED ||
        bestxyz->vSolStatus != OEM_BESTXYZ_SOLSTAT_SOL_COMPUTED) {
        fwrite(msg, 1, sizeof(oem_binary_header_t), binfd);
        fwrite(&bestxyz->pSolStatus, 1, sizeof(bestxyz->pSolStatus), binfd);
        fwrite(&bestxyz->vSolStatus, 1, sizeof(bestxyz->vSolStatus), binfd);
        return OEM_ERR_NO_SOLUTION;
    }

    fwrite(msg, 1, sizeof(oem_binary_header_t) + hdr->messageLength, binfd);

    return OEM_OK;
}
