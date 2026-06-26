#include "cfe.h"

#include <stdio.h>

int32 io_csp_ftp_server_start(uint32 *RunStatus);

void FTP_ORIGIN_Main(void)
{
    uint32 RunStatus = CFE_ES_RunStatus_APP_RUN;
    int32  status;

    status = io_csp_ftp_server_start(&RunStatus);

    if (status != 0)
    {
        printf("FTP ORIGIN server failed: rc=%ld\n", (long)status);
        RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    CFE_ES_ExitApp(RunStatus);
}
