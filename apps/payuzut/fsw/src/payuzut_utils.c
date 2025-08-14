#include "payuzut_utils.h"
#include "payuzut_internal_cfg.h"
#include "payuzut_task.h"

#include <fcntl.h>
#include <unistd.h>

int PAYUZUT_OpenFile(void) {
    int FD;

    FD = open(PAYUZUT_TEMPERATURE_FILE, O_CREAT | O_RDWR, 0666);
    OS_printf("FD: %d\n", FD);
    return FD;
}

int32 PAYUZUT_WriteToFile(int FD, void *Data, size_t Size) {
    int32 Status;

    Status = write(FD, Data, Size);
    if (Status != Size) return -1;
    Status = fsync(FD);
    if (Status != 0) return -2;
    return CFE_SUCCESS;
}

int32 PAYUZUT_CloseFile(int FD) {
    return close(FD);
}