#include "cfe.h"

#include <stdio.h>

int32 io_csp_ftp_init_server(void);

int32 IO_FTP_LibInit(CFE_ES_LibId_t LibId)
{
    int32 status;

    (void)LibId;

    status = io_csp_ftp_init_server();
    if (status != 0)
    {
        printf("FTP LIB initialization failed: rc=%ld\n", (long)status);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    printf("FTP LIB initialized.\n");
    return CFE_SUCCESS;
}
