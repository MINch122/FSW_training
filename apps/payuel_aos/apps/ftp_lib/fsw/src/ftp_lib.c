#include "common_types.h"
#include <stdio.h>

int32 io_csp_ftp_init_server(void);

void IO_FTP_LibInit(void) {
    io_csp_ftp_init_server();
    printf("FTP LIB initialized.\n");
}
