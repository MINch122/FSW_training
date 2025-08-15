#include "payuzut_utils.h"
#include "payuzut_internal_cfg.h"
#include "payuzut_task.h"

#include <fcntl.h>
#include <unistd.h>

int PAYUZUT_OpenFile(uint8_t Idx) {
    int FD;

    if (!Idx) FD = open(PAYUZUT_TEMPERATURE_FILE_1, O_CREAT | O_RDWR, 0666);
    else FD = open(PAYUZUT_TEMPERATURE_FILE_2, O_CREAT | O_RDWR, 0666);
    
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

int32 PAYUZUT_ConfigADC(uint8 Addr) {
    int32 Status;
    CFE_SRL_IO_Param_t Params = {0,};

    uint8_t TxBuf[3] = {PAYUZUT_ADC_CONF_REG_ADDR, PAYUZUT_CONF_MSB, PAYUZUT_CONF_LSB};
    Params.TxData = TxBuf;
    Params.TxSize = sizeof(TxBuf);
    Params.Addr = Addr;
    Status = CFE_SRL_ApiWrite(PAYUZUT_Data.Handle, &Params);

    return Status;
}

int32 PAYUZUT_ReadADC(uint8 Addr, uint16 *Value) {
    int32 Status;
    CFE_SRL_IO_Param_t Params = {0,};

    uint8_t TxBuf = {PAYUZUT_ADC_CONV_REG_ADDR};
    uint8_t RxBuf[2] = {0,};

    Params.TxData = &TxBuf;
    Params.TxSize = sizeof(TxBuf);
    Params.RxData = RxBuf;
    Params.RxSize = sizeof(RxBuf);
    Params.Addr = Addr;

    Status = CFE_SRL_ApiRead(PAYUZUT_Data.Handle, &Params);
    if (Status != CFE_SUCCESS) return Status;

    *Value = (((uint16_t)RxBuf[0] << 8) | RxBuf[1]) >> 4;

    return Status;
}

int32 PAYUZUT_GetADCValue(uint8 Addr, uint16 *Value) {
    int32 Status;

    Status = PAYUZUT_ConfigADC(Addr);
    if (Status == CFE_SUCCESS) {
        OS_TaskDelay(PAYUZUT_ADC_POLL_MSEC);
        Status = PAYUZUT_ReadADC(Addr, Value);
    }

    return Status;
}