#ifndef CFE_SRL_IO_H
#define CFE_SRL_IO_H

#include "cfe_srl_basic.h"

/**
 * Header for CAN via socket
 */
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/if.h>

#include <linux/spi/spidev.h>

int CFE_SRL_Open(CFE_SRL_IO_Handle_t *Handle, const char *DevName, int Option);
int CFE_SRL_Write(CFE_SRL_IO_Handle_t *Handle, const void *Data, size_t Size);
int CFE_SRL_Read(CFE_SRL_IO_Handle_t *Handle, void *Data, size_t Size, uint32_t Timeout, ssize_t *ReadBytes);
int CFE_SRL_TransactionI2C(CFE_SRL_IO_Handle_t *Handle, const void *TxData, size_t TxSize, void *RxData, size_t RxSize, uint32_t Addr);
int CFE_SRL_Close(CFE_SRL_IO_Handle_t *Handle);

int CFE_SRL_OpenSocket(CFE_SRL_IO_Handle_t *Handle, const char *DevName);

int CFE_SRL_GpioInit(CFE_SRL_GPIO_Handle_t *Handle, const char *Path, unsigned int Line, const char *Name, bool Default, bool IsOut);

int32 CFE_SRL_SetModeSPI(CFE_SRL_IO_Handle_t *Handle, uint8_t Mode);
int32 CFE_SRL_SetSpeedSPI(CFE_SRL_IO_Handle_t *Handle, uint32_t Speed);
int32 CFE_SRL_SetBitPerWordSPI(CFE_SRL_IO_Handle_t *Handle, uint8_t BitPerWord);
int32 CFE_SRL_SetSPI(CFE_SRL_IO_Handle_t *Handle, uint8_t Mode, uint32_t Speed, uint8_t BitPerWord);

#endif /* CFE_SRL_IO_H */