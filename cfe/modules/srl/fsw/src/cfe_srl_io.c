#include "cfe_srl_io.h"



#include <errno.h>
extern CFE_SRL_Global_Handle_t GlobalHandle[CFE_SRL_GLOBAL_HANDLE_NUM];

int CFE_SRL_Open(CFE_SRL_IO_Handle_t *Handle, const char *DevName, int Option) {
    int Status;

    if (Handle == NULL) {
        return CFE_SRL_BAD_ARGUMENT; // Revise to `NULL_ERROR`
    }
    Status = CFE_SRL_BasicOpen(DevName, Option);
    if (Status < 0) {
        Handle->__errno = errno;
        return Status; // Revise to `OPEN_ERR`
    }

    Handle->FD = Status;

    Status = CFE_SRL_SetHandleStatus(Handle, CFE_SRL_HANDLE_STATUS_FD_INIT, true);
    if (Status != CFE_SUCCESS) return Status;

    return CFE_SUCCESS; // Revise to `OK`
}

int CFE_SRL_Write(CFE_SRL_IO_Handle_t *Handle, const void *Data, size_t Size) {
    // int Status;
    ssize_t WriteBytes;

    if (Handle == NULL || Data == NULL) return CFE_SRL_BAD_ARGUMENT;

    if(!CFE_SRL_QueryStatus((const CFE_SRL_Global_Handle_t *)Handle, CFE_SRL_HANDLE_STATUS_FD_INIT)) {
        return CFE_SRL_NOT_OPEN_ERR;
    }
    OS_printf("FD = %d || Data = %p || Size = %u\n", Handle->FD, Data, (uint32_t)Size);
    WriteBytes = CFE_SRL_BasicWrite(Handle->FD, Data, Size);
    if (WriteBytes < 0) {
        Handle->TxErrCnt++;
        Handle->__errno = errno;
        return CFE_SRL_WRITE_ERR;
    }

    Handle->TxCount += WriteBytes;

    if (WriteBytes != Size) {
        Handle->TxErrCnt++;
        Handle->__errno = errno;
        return CFE_SRL_PARTIAL_WRITE_ERR;
    }

    return CFE_SUCCESS;
}

int CFE_SRL_Read(CFE_SRL_IO_Handle_t *Handle, void *Data, size_t Size, uint32_t Timeout) {
    // int Status;
    ssize_t ReadBytes;

    if (Handle == NULL || Data == NULL) return CFE_SRL_BAD_ARGUMENT;
    
    if (!CFE_SRL_QueryStatus((CFE_SRL_Global_Handle_t *)Handle, CFE_SRL_HANDLE_STATUS_FD_INIT)) {
        return CFE_SRL_NOT_OPEN_ERR;        
    }
    
    ReadBytes = CFE_SRL_BasicPollRead(Handle->FD, Data, Size, Timeout);

    if (ReadBytes == CFE_SRL_TIMEOUT) {
        Handle->RxErrCnt ++;
        Handle->__errno = errno;
        return CFE_SRL_TIMEOUT;
    }
    else if (ReadBytes == CFE_SRL_ERR) {
        Handle->RxErrCnt ++;
        Handle->__errno = errno;
        return CFE_SRL_READ_ERR;
    }

    Handle->RxCount += ReadBytes;

    if (ReadBytes != Size) {
        Handle->RxErrCnt ++;
        Handle->__errno = errno;
        return CFE_SRL_PARTIAL_READ_ERR;
    }
    
    return CFE_SUCCESS;
}

int CFE_SRL_TransactionI2C(CFE_SRL_IO_Handle_t *Handle, const void *TxData, size_t TxSize, void *RxData, size_t RxSize, uint32_t Addr) {
    int Status;
    struct i2c_rdwr_ioctl_data Packet = {0,};
    struct i2c_msg MsgI2C[2] = {0,};

    // First Message - Write
    MsgI2C[0].addr = (uint16_t)Addr;
    MsgI2C[0].flags = 0;    // Write flag
    MsgI2C[0].len = TxSize;
    MsgI2C[0].buf = (uint8_t *)TxData;

    Packet.nmsgs ++;

    // Second Message - Read
    MsgI2C[1].addr = (uint16_t)Addr;
    MsgI2C[1].flags = I2C_M_RD; // Read flag
    MsgI2C[1].len = RxSize;
    MsgI2C[1].buf = RxData;

    Packet.nmsgs ++;
    
    /**
     * Configure Transaction Packet
     */
    Packet.msgs = MsgI2C;

    /**
     * Do transaction
     */
    Status = ioctl(Handle->FD, I2C_RDWR, &Packet);
    if (Status < 0) {
        Handle->__errno = errno;
        return CFE_SRL_READ_ERR;
    }
    
    return CFE_SUCCESS;
}

int CFE_SRL_Close(CFE_SRL_IO_Handle_t *Handle) {
    int Status;

    if (Handle == NULL) return CFE_SRL_BAD_ARGUMENT;

    if (!CFE_SRL_QueryStatus((CFE_SRL_Global_Handle_t *)Handle, CFE_SRL_HANDLE_STATUS_FD_INIT)) {
        return CFE_SRL_NOT_OPEN_ERR;
    }
    
    Status = CFE_SRL_BasicClose(Handle->FD);
    if (Status < 0) {
        Handle->__errno = errno;    
        return CFE_SRL_CLOSE_ERR;
    }
    return CFE_SUCCESS;
}


// For CAN
int CFE_SRL_OpenSocket(CFE_SRL_IO_Handle_t *Handle, const char *DevName) {
    int Status;
    int Socket; // Equivalent to File Descriptor
    struct ifreq Ifr;
    struct sockaddr_can Addr = {0};

    Socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (Socket < 0) {
        Handle->__errno = errno;
        return CFE_SRL_CAN_OPEN_SOCKET_ERR;
    }

    strncpy(Ifr.ifr_name, DevName, IFNAMSIZ-1);
    Status = ioctl(Socket, SIOCGIFINDEX, &Ifr); // Get interface index number
    if (Status < 0) {
        Handle->__errno = errno;
        CFE_SRL_BasicClose(Socket);
        return CFE_SRL_CAN_SIOCGIFINDEX_ERR;
    }

    Addr.can_family = AF_CAN;
    Addr.can_ifindex = Ifr.ifr_ifindex;

    Status = bind(Socket, (struct sockaddr *)&Addr, sizeof(Addr));
    if (Status < 0) {
        Handle->__errno = errno;
        CFE_SRL_BasicClose(Socket);
        return CFE_SRL_CAN_BIND_ERR;
    }

    Handle->FD = Socket;
    
    Status = CFE_SRL_SetHandleStatus(Handle, CFE_SRL_HANDLE_STATUS_FD_INIT, true);
    if (Status != CFE_SUCCESS) return Status;

    return CFE_SUCCESS;
}

/*************************************************************
 * 
 *  IO GPIO Function
 * 
 *************************************************************/
int CFE_SRL_GpioInit(CFE_SRL_GPIO_Handle_t *Handle, const char *Path, unsigned int Line, const char *Name, bool Default) {
    int Status;
    OS_printf("GPIO Handle address: %p\n",Handle);
    if (Handle == NULL || Path == NULL) return CFE_SRL_BAD_ARGUMENT;

    Status = CFE_SRL_BasicGpioOpen(Handle, Path);
    if (Status != CFE_SUCCESS) return Status;

    Status = CFE_SRL_BasicGpioGetLine(Handle, Line);
    if (Status != CFE_SUCCESS) return Status;

    Status = CFE_SRL_BasicGpioSetOutput(Handle, Name, Default);
    if (Status != CFE_SUCCESS) return Status;

    return CFE_SUCCESS;
}

/*************************************************************
 * 
 *  SPI setting Function
 * 
 *************************************************************/
int32 CFE_SRL_SetModeSPI(CFE_SRL_IO_Handle_t *Handle, uint8_t Mode) {
    int32 Status;

    if (Handle == NULL) return CFE_SRL_BAD_ARGUMENT;

    Status = ioctl(Handle->FD, SPI_IOC_WR_MODE, &Mode);
    if (Status < 0) {
        Handle->__errno = errno;
        return CFE_SRL_IOCTL_ERR;
    }

    return CFE_SUCCESS;
}

int32 CFE_SRL_SetSpeedSPI(CFE_SRL_IO_Handle_t *Handle, uint32_t Speed) {
    int32 Status;

    if (Handle == NULL) return CFE_SRL_BAD_ARGUMENT;

    Status = ioctl(Handle->FD, SPI_IOC_WR_MAX_SPEED_HZ, &Speed);
    if (Status < 0) {
        Handle->__errno = errno;
        return CFE_SRL_IOCTL_ERR;
    }

    return CFE_SUCCESS;
}

int32 CFE_SRL_SetBitPerWordSPI(CFE_SRL_IO_Handle_t *Handle, uint8_t BitPerWord) {
    int32 Status;

    if (Handle == NULL) return CFE_SRL_BAD_ARGUMENT;

    Status = ioctl(Handle->FD, SPI_IOC_WR_BITS_PER_WORD, &BitPerWord);
    if (Status < 0) {
        Handle->__errno = errno;
        return CFE_SRL_IOCTL_ERR;
    }

    return CFE_SUCCESS;
}

int32 CFE_SRL_SetSPI(CFE_SRL_IO_Handle_t *Handle, uint8_t Mode, uint32_t Speed, uint8_t BitPerWord) {
    int32 Status;

    if (Handle == NULL ) return CFE_SRL_BAD_ARGUMENT;

    Status = CFE_SRL_SetModeSPI(Handle, Mode);
    if (Status != CFE_SUCCESS) return Status;

    Status = CFE_SRL_SetSpeedSPI(Handle, Speed);
    if (Status != CFE_SUCCESS) return Status;

    Status = CFE_SRL_SetBitPerWordSPI(Handle, BitPerWord);
    if (Status != CFE_SUCCESS) return Status;

    return CFE_SUCCESS;
}