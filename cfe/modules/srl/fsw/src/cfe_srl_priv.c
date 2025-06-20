/************************************************************************
 * Author : Hyeokjin Kweon
 * 
 * Last Modified : 2025 - 04 - 27
 * 
 * Purpose : Serial Comm. Core Module's API Initialization
 ************************************************************************/

/**
 * Required header files
*/
#include "cfe_srl_module_all.h"

/**
 * Global data
 */
/* Serial Handle for each srl comm. */
extern CFE_SRL_IO_Handle_t *Handles[CFE_SRL_GNRL_DEVICE_NUM];

/* GPIO Handle for each gpio */
extern CFE_SRL_GPIO_Handle_t GPIO[CFE_SRL_TOT_GPIO_NUM];
/**
 * Private Get Handle function
 */
/*----------------------------------------------------------------
 *
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
CFE_SRL_IO_Handle_t *CFE_SRL_GetHandle(CFE_SRL_Handle_Indexer_t Index) {
    return Handles[Index];
}

CFE_SRL_GPIO_Handle_t *CFE_SRL_GetGpioHandle(CFE_SRL_GPIO_Indexer_t Index) {
    return &GPIO[Index];
}

/**
 * Private Write function
 */

/*----------------------------------------------------------------
 *
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_WriteI2C(CFE_SRL_IO_Handle_t *Handle, const void *Data, size_t Size, uint8_t Addr) {
    int32 Status;
    CFE_SRL_DevType_t DevType;

    if (Handle == NULL || Data == NULL) return CFE_SRL_BAD_ARGUMENT;
    if (Addr > 128) return CFE_SRL_I2C_ADDR_ERR;

    // Check dev type
    DevType = CFE_SRL_GetHandleDevType(Handle);
    if (DevType != SRL_DEVTYPE_I2C) return CFE_SRL_INVALID_TYPE;

    // Mutex Lock
    Status = CFE_SRL_MutexLock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    // Set Slave Addr
    Status = CFE_SRL_BasicIOCTL(Handle->FD, I2C_SLAVE, &Addr);
    if (Status < 0) {
        Handle->__errno = errno;
        return CFE_SRL_IOCTL_ERR;
    }

    // Write
    Status = CFE_SRL_Write(Handle, Data, Size);
    if (Status != CFE_SUCCESS) return Status;

    // Mutex Unlock
    Status = CFE_SRL_MutexUnlock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    return CFE_SUCCESS;
}

int32 CFE_SRL_WriteGenericI2C(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    return CFE_SRL_WriteI2C(Handle, Params->TxData, Params->TxSize, Params->Addr);
}

/*----------------------------------------------------------------
 *
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_WriteUART(CFE_SRL_IO_Handle_t *Handle, const void *Data, size_t Size) {
    int32 Status;
    CFE_SRL_DevType_t DevType;
    if (Handle == NULL || Data == NULL) return CFE_SRL_BAD_ARGUMENT;

    DevType = CFE_SRL_GetHandleDevType(Handle);
    if (DevType != SRL_DEVTYPE_UART && DevType != SRL_DEVTYPE_RS422) return CFE_SRL_INVALID_TYPE;

    // Mutex Lock
    Status = CFE_SRL_MutexLock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    // Write
    Status = CFE_SRL_Write(Handle, Data, Size);
    if (Status != CFE_SUCCESS) return Status;

    // Mutex Unlock
    Status = CFE_SRL_MutexUnlock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    return CFE_SUCCESS;
}

int32 CFE_SRL_WriteGenericUART(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    return CFE_SRL_WriteUART(Handle, Params->TxData, Params->TxSize);
}

/*----------------------------------------------------------------
 *
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_WriteCAN(CFE_SRL_IO_Handle_t *Handle, const void *Data, size_t Size, uint32_t Addr) {
    int32 Status;
    CFE_SRL_DevType_t DevType;
    struct can_frame Frame;

    if (Handle == NULL || Data == NULL) return CFE_SRL_BAD_ARGUMENT;

    DevType = CFE_SRL_GetHandleDevType(Handle);
    if (DevType != SRL_DEVTYPE_CAN) return CFE_SRL_INVALID_TYPE;

    // if (Addr > 128) return -1; // Revise to `CAN_ADDR_ERR`, 128 to 29 bits max num


    // Mutex Lock
    Status = CFE_SRL_MutexLock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    size_t TotBytes = 0; // Total Tx bytes till now
    size_t WrBytes; // Write bytes at this very time
    while (TotBytes < Size) {

        // Configure Frame
        WrBytes = (Size - TotBytes >= CAN_MAX_DLEN) ? CAN_MAX_DLEN : (Size - TotBytes);
        Frame.can_id = Addr | CAN_EFF_FLAG;
        Frame.can_dlc = WrBytes;
        memcpy(Frame.data, ((uint8_t *)Data) + TotBytes, WrBytes);

        // Write
        Status = CFE_SRL_Write(Handle, &Frame, sizeof(Frame));
        if (Status != CFE_SUCCESS) return Status;

        TotBytes += WrBytes;
    }

    // Mutex Unlock
    Status = CFE_SRL_MutexUnlock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    return CFE_SUCCESS;
}

int32 CFE_SRL_WriteGenericCAN(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    return CFE_SRL_WriteCAN(Handle, Params->TxData, Params->TxSize, Params->Addr);
}

/*----------------------------------------------------------------
 *
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_WriteSPI(CFE_SRL_IO_Handle_t *Handle, const void *Data, size_t Size) {
    int32 Status;
    CFE_SRL_DevType_t DevType;
    struct spi_ioc_transfer Xfer[1];
    memset(Xfer, 0, sizeof(Xfer));

    if (Handle == NULL || Data == NULL) return CFE_SRL_BAD_ARGUMENT;

    DevType = CFE_SRL_GetHandleDevType(Handle);
    if (DevType != SRL_DEVTYPE_SPI) return CFE_SRL_INVALID_TYPE;

    //Mutex Lock
    Status = CFE_SRL_MutexLock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    Xfer[0].tx_buf = (uint64_t)(uintptr_t)Data;
    Xfer[0].len = Size;

    Status = ioctl(Handle->FD, SPI_IOC_MESSAGE(1), Xfer);
    if (Status < 0) {
        Handle->__errno = errno;
        return CFE_SRL_IOCTL_ERR;
    }

    // Mutex Unlock
    Status = CFE_SRL_MutexUnlock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    return CFE_SUCCESS;
}
int32 CFE_SRL_WriteGenericSPI(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    return CFE_SRL_WriteSPI(Handle, Params->TxData, Params->TxSize);
}


/**
 * Private Read function
 */

/*----------------------------------------------------------------
 *
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_ReadI2C(CFE_SRL_IO_Handle_t *Handle, const void *TxData, size_t TxSize, void *RxData, size_t RxSize, uint32_t Addr) {
    int32 Status;
    CFE_SRL_DevType_t DevType;

    if (Handle == NULL || RxData == NULL || TxData == NULL) return CFE_SRL_BAD_ARGUMENT;

    DevType = CFE_SRL_GetHandleDevType(Handle);
    if (DevType != SRL_DEVTYPE_I2C) return CFE_SRL_INVALID_TYPE;

    // Mutex Lock
    Status = CFE_SRL_MutexLock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    // Transaction
    Status = CFE_SRL_TransactionI2C(Handle, TxData, TxSize, RxData, RxSize, Addr);
    if (Status != CFE_SUCCESS) return Status;

    // Mutex Unlock
    Status = CFE_SRL_MutexUnlock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    return CFE_SUCCESS;
}

int32 CFE_SRL_ReadGenericI2C(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    return CFE_SRL_ReadI2C(Handle, Params->TxData, Params->TxSize, Params->RxData, Params->RxSize, Params->Addr);
}

/*----------------------------------------------------------------
 *
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_ReadUART(CFE_SRL_IO_Handle_t *Handle, const void *TxData, size_t TxSize, void *RxData, size_t RxSize, uint32_t Timeout) {
    // write -> poll read
    int Status;
    CFE_SRL_DevType_t DevType;
    if (Handle == NULL || TxData == NULL || RxData == NULL) return CFE_SRL_BAD_ARGUMENT;

    DevType = CFE_SRL_GetHandleDevType(Handle);
    if (DevType != SRL_DEVTYPE_UART && DevType != SRL_DEVTYPE_RS422) return CFE_SRL_INVALID_TYPE;

    // Mutex Lock
    Status = CFE_SRL_MutexLock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    // Write
    Status = CFE_SRL_Write(Handle, TxData, TxSize);
    if (Status != CFE_SUCCESS) return Status;
    
    // Poll Read
    Status = CFE_SRL_Read(Handle, RxData, RxSize, Timeout);
    if (Status != CFE_SUCCESS) return Status;

    // Mutex Unlock
    Status = CFE_SRL_MutexUnlock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    return CFE_SUCCESS;
}

int32 CFE_SRL_ReadGenericUART(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    return CFE_SRL_ReadUART(Handle, Params->TxData, Params->TxSize, Params->RxData, Params->RxSize, Params->Timeout);
}

/*----------------------------------------------------------------
 *
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_ReadCAN(CFE_SRL_IO_Handle_t *Handle, const void *TxData, size_t TxSize, void *RxData, size_t RxSize, uint32_t Timeout, uint32_t Addr) {
    int32 Status;
    CFE_SRL_DevType_t DevType;
    struct can_frame Frame = {0,};

    if (Handle == NULL || TxData == NULL || RxData == NULL) return CFE_SRL_BAD_ARGUMENT;

    DevType = CFE_SRL_GetHandleDevType(Handle);
    if(DevType != SRL_DEVTYPE_CAN) return CFE_SRL_INVALID_TYPE;

    // Write
    Status = CFE_SRL_WriteCAN(Handle, TxData, TxSize, Addr);
    if (Status != CFE_SUCCESS) return Status;

    // Mutex Lock
    Status = CFE_SRL_MutexLock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    size_t TotBytes = 0; // Total Rx bytes till now
    size_t RdBytes; // Read bytes at this very time
    while (TotBytes < RxSize) {
        RdBytes = (RxSize - TotBytes >= CAN_MAX_DLEN) ? CAN_MAX_DLEN : (RxSize - TotBytes);
        // Poll Read
        Status = CFE_SRL_Read(Handle, &Frame, sizeof(struct can_frame), Timeout);
        if (Status != CFE_SUCCESS) return Status;

        uint32_t RxID = 0;
        if (Frame.can_id & CAN_EFF_FLAG) RxID = Frame.can_id & CAN_EFF_MASK;
        else RxID = Frame.can_id & CAN_SFF_MASK;
        OS_printf("InComing CAN Frame ID: %u\n", RxID);

        if (RdBytes != Frame.can_dlc) {
            OS_printf("%s: CAN read length NOT matched!\n", __func__);
        }
        memcpy((uint8_t *)RxData + TotBytes, Frame.data, RdBytes);
        TotBytes += RdBytes;
    }

    // Mutex Unlock
    Status = CFE_SRL_MutexUnlock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    return CFE_SUCCESS;
}

int32 CFE_SRL_ReadGenericCAN(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    return CFE_SRL_ReadCAN(Handle, Params->TxData, Params->TxSize, Params->RxData, Params->RxSize, Params->Timeout, Params->Addr);
}

/*----------------------------------------------------------------
 *
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_ReadSPI(CFE_SRL_IO_Handle_t *Handle, const void *TxData, size_t TxSize, void *RxData, size_t RxSize) {
    int32 Status;
    CFE_SRL_DevType_t DevType;
    struct spi_ioc_transfer Xfer[2];
    memset(Xfer, 0, sizeof(Xfer));

    if (Handle == NULL || TxData == NULL || RxData == NULL) return CFE_SRL_BAD_ARGUMENT;

    DevType = CFE_SRL_GetHandleDevType(Handle);
    if (DevType != SRL_DEVTYPE_SPI) return CFE_SRL_INVALID_TYPE;

    Status = CFE_SRL_MutexLock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    Xfer[0].tx_buf = (uint64_t)(uintptr_t)TxData;
    Xfer[0].len = TxSize;

    Xfer[1].rx_buf = (uint64_t)(uintptr_t)RxData;
    Xfer[1].len = RxSize;

    Status = ioctl(Handle->FD, SPI_IOC_MESSAGE(2), Xfer);
    if (Status < 0) {
        Handle->__errno = errno;
        return CFE_SRL_IOCTL_ERR;
    }

    Status = CFE_SRL_MutexUnlock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    return CFE_SUCCESS;
}

int32 CFE_SRL_ReadGenericSPI(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    return CFE_SRL_ReadSPI(Handle, Params->TxData, Params->TxSize, Params->RxData, Params->RxSize);
}