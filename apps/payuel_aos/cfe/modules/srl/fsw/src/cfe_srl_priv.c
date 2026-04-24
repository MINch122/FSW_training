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
 * Private function definition
 */
#define CFE_SRL_HANDLE_PSP_SERIAL_ERR(psp_status) \
    CFE_SRL_TRxErrHandling(__func__, psp_status)
#define CFE_SRL_HANDLE_PSP_GPIO_ERR(psp_status) \
    CFE_SRL_GpioErrHandling(__func__, psp_status)

/**
 * Global data
 */
/* Serial Handle for each srl comm. */
extern CFE_SRL_IO_Handle_t *Handles[CFE_SRL_GNRL_DEVICE_NUM];

/* GPIO Handle for each gpio */
// extern CFE_SRL_GPIO_Handle_t GPIO[CFE_SRL_TOT_GPIO_NUM];

/**
 * Private Sleep function
 */
// void Sleep_us(uint32_t Delay_us) {
//     struct timespec Req;

//     Req.tv_sec = Delay_us / 1000000;
//     Req.tv_nsec = (Delay_us % 1000000) * 1000;
    
//     while(nanosleep(&Req, &Req) == -1 && errno == EINTR);

//     return;
// }


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

// CFE_SRL_GPIO_Handle_t *CFE_SRL_GetGpioHandle(CFE_SRL_GPIO_Indexer_t Index) {
//     return &GPIO[Index];
// }

/** 
 * \brief Private Error Handling function 
 * \param PspStatus [in] PSP StatusCode returned by iodriver linux-serial
 * \return SRL Status code correponed to PSP StatusCode
 * \warning Do not call this function directily.
 *  Use `CFE_SRL_HANDLE_PSP_SERIAL_ERR` macro instead.
 */
static int32 CFE_SRL_TRxErrHandling(const char *func, int32 PspStatus) {
    int32 Status;
    switch (PspStatus)
    {
    case CFE_PSP_IODriver_SERIAL_WRITE_ERROR:
        Status = CFE_SRL_WRITE_ERR;
        break;
    case CFE_PSP_IODriver_SERIAL_PARTIAL_WRITE_ERROR:
        Status = CFE_SRL_PARTIAL_WRITE_ERR;
        break;
    case CFE_PSP_IODriver_SERIAL_IOCTL_ERROR:
        Status = CFE_SRL_IOCTL_ERR;
        break;
    case CFE_PSP_IODriver_SERIAL_READ_ERROR:
        Status = CFE_SRL_READ_ERR;
        break;
    case CFE_PSP_IODriver_SERIAL_PARTIAL_READ_ERROR:
        Status = CFE_SRL_PARTIAL_READ_ERR;
        break;
    case CFE_PSP_IODriver_SERIAL_TIMEOUT_ERROR:
        Status = CFE_SRL_TIMEOUT;
        break;
    case CFE_PSP_IODriver_SERIAL_ERROR:
        Status = CFE_SRL_ERR;
        break;
    case CFE_PSP_IODriver_SERIAL_INVALID_TYPE_ERROR:
        Status = CFE_SRL_BAD_ARGUMENT;
        break;
    case CFE_PSP_IODriver_SERIAL_CLOSE_ERROR:
        Status = CFE_SRL_CLOSE_ERR;
        break;
    case CFE_PSP_IODriver_SERIAL_OPEN_ERROR:
        Status = CFE_SRL_OPEN_ERR;
        break;
    case CFE_PSP_IODriver_SERIAL_SOCKET_ERROR:
        Status = CFE_SRL_CAN_OPEN_SOCKET_ERR;
        break;
    case CFE_PSP_IODriver_SERIAL_BIND_ERROR:
        Status = CFE_SRL_CAN_BIND_ERR;
        break;
    default:
        Status = CFE_SRL_ERR;
        break;
    }

    if (Status != CFE_SUCCESS)
        CFE_ES_WriteToSysLog("%s: Operation not succeed. PSP RC = %d\n", func, PspStatus);
    
    return Status;
}

/** 
 * \brief Private Error Handling function.
 * \param PspStatus [in] PSP StatusCode returned by iodriver linux-gpio
 * \return SRL Status code correponed to PSP StatusCode
 * \warning Do not call this function directily.
 *  Use `CFE_SRL_HANDLE_PSP_GPIO_ERR` macro instead.
 */
static int32 CFE_SRL_GpioErrHandling(const char *func, int32 PspStatus) {
    int32 Status;
    switch (PspStatus)
    {
    case CFE_PSP_IODriver_DISCRETE_IO_TABLE_FULL_ERROR:
        Status = CFE_SRL_FULL_ERR;
        break;
    case CFE_PSP_IODriver_DISCRETE_IO_CHIP_OPEN_ERROR:
        Status = CFE_SRL_OPEN_ERR;
        break;
    case CFE_PSP_IODriver_DISCRETE_IO_GET_LINE_ERROR:
    case CFE_PSP_IODriver_DISCRETE_IO_REQUEST_OUTPUT_ERROR:
    case CFE_PSP_IODriver_DISCRETE_IO_REQUEST_INPUT_ERROR:
        Status = CFE_SRL_GPIO_CONFIG_FAIL_ERR;
        break;
    case CFE_PSP_IODriver_DISCRETE_IO_NOT_OPEN_ERROR:
        Status = CFE_SRL_NOT_OPEN_ERR;
        break;
    case CFE_PSP_IODriver_DISCRETE_IO_INVALID_DIRECTION_ERROR:
    case CFE_PSP_IODriver_DISCRETE_IO_INVALID_HANDLE:
        Status = CFE_SRL_BAD_ARGUMENT;
        break;
    case CFE_PSP_IODriver_DISCRETE_IO_SET_ERROR:
        Status = CFE_SRL_GPIO_SET_VALUE_ERR;
        break;
    case CFE_PSP_IODriver_DISCRETE_IO_GET_ERROR:
        Status = CFE_SRL_GPIO_GET_VALUE_ERR;
        break;
    
    default:
        Status = CFE_SRL_ERR;
        break;
    }

    if (Status != CFE_SUCCESS)
        CFE_ES_WriteToSysLog("%s: Operation not succeed. PSP RC = %d\n", func, PspStatus);
    
    return Status;
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
int32 CFE_SRL_WriteI2C(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    int32 Status;
    CFE_SRL_DevType_t DevType;
    CFE_PSP_IODriver_Location_t Location;
    CFE_PSP_IODriver_SerialXfer_t Xfer = {0};

    if (Handle == NULL || Params == NULL) return CFE_SRL_BAD_ARGUMENT;
    // if (Params->Addr > 128) return CFE_SRL_I2C_ADDR_ERR;

    // Check dev type
    DevType = CFE_SRL_GetHandleDevType(Handle);
    if (DevType != SRL_DEVTYPE_I2C) return CFE_SRL_INVALID_TYPE;

    // Mutex Lock
    Status = CFE_SRL_MutexLock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    Location.PspModuleId = CFE_SRL_Global.IOdriverSerialModuleId;
    Location.SubsystemId = CFE_PSP_IODriver_WRITE_SUBSYSTEM;
    Location.SubchannelId = CFE_PSP_IODriver_SERIAL_I2C_SUBCH;
    
    Xfer.FD = Handle->FD;
    Xfer.Params = *Params;

    Status = CFE_PSP_IODriver_Command(&Location, 0, CFE_PSP_IODriver_VPARG(&Xfer));
    if (Status < 0) {
        Status = CFE_SRL_HANDLE_PSP_SERIAL_ERR(Status);
        goto error;
    }

    // Mutex Unlock
    Status = CFE_SRL_MutexUnlock(Handle);
    if (Status != CFE_SUCCESS) goto error;

    return CFE_SUCCESS;

error:
    CFE_SRL_MutexUnlock(Handle);
    return Status;
}

int32 CFE_SRL_WriteGenericI2C(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    return CFE_SRL_WriteI2C(Handle, Params);
}

/*----------------------------------------------------------------
 *
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_WriteUART(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    int32 Status;
    CFE_SRL_DevType_t DevType;
    CFE_PSP_IODriver_Location_t Location;
    CFE_PSP_IODriver_SerialXfer_t Xfer = {0};

    if (Handle == NULL || Params == NULL) return CFE_SRL_BAD_ARGUMENT;

    DevType = CFE_SRL_GetHandleDevType(Handle);
    if (DevType != SRL_DEVTYPE_UART && DevType != SRL_DEVTYPE_RS422) return CFE_SRL_INVALID_TYPE;

    // Mutex Lock
    Status = CFE_SRL_MutexLock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    Location.PspModuleId = CFE_SRL_Global.IOdriverSerialModuleId;
    Location.SubsystemId = CFE_PSP_IODriver_WRITE_SUBSYSTEM;
    Location.SubchannelId = CFE_PSP_IODriver_SERIAL_UART_SUBCH;

    Xfer.FD = Handle->FD;
    Xfer.Params = *Params;
    Status = CFE_PSP_IODriver_Command(&Location, 0, CFE_PSP_IODriver_VPARG(&Xfer));
    if (Status < 0) {
        Status = CFE_SRL_HANDLE_PSP_SERIAL_ERR(Status);
        goto error;
    }
    OS_printf("Write Success.\n");

    // Mutex Unlock
    Status = CFE_SRL_MutexUnlock(Handle);
    if (Status != CFE_SUCCESS) goto error;

    return CFE_SUCCESS;

error:
    CFE_SRL_MutexUnlock(Handle);
    return Status;
}

int32 CFE_SRL_WriteGenericUART(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    return CFE_SRL_WriteUART(Handle, Params);
}

/*----------------------------------------------------------------
 *
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_WriteCAN(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    int32 Status;
    CFE_SRL_DevType_t DevType;
    CFE_PSP_IODriver_Location_t Location;
    CFE_PSP_IODriver_SerialXfer_t Xfer = {0};

    if (Handle == NULL || Params == NULL) return CFE_SRL_BAD_ARGUMENT;

    DevType = CFE_SRL_GetHandleDevType(Handle);
    if (DevType != SRL_DEVTYPE_CAN) return CFE_SRL_INVALID_TYPE;

    // Mutex Lock
    Status = CFE_SRL_MutexLock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    Location.PspModuleId = CFE_SRL_Global.IOdriverSerialModuleId;
    Location.SubsystemId = CFE_PSP_IODriver_WRITE_SUBSYSTEM;
    Location.SubchannelId = CFE_PSP_IODriver_SERIAL_CAN_SUBCH;

    Xfer.FD = Handle->FD;
    Xfer.Params = *Params;

    Status = CFE_PSP_IODriver_Command(&Location, 0, CFE_PSP_IODriver_VPARG(&Xfer));
    if (Status < 0) {
        Status = CFE_SRL_HANDLE_PSP_SERIAL_ERR(Status);
        goto error;
    }

    // Mutex Unlock
    Status = CFE_SRL_MutexUnlock(Handle);
    if (Status != CFE_SUCCESS) goto error;

    return CFE_SUCCESS;

error:
    CFE_SRL_MutexUnlock(Handle);
    return Status;
}

int32 CFE_SRL_WriteGenericCAN(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    return CFE_SRL_WriteCAN(Handle, Params);
}

/*----------------------------------------------------------------
 *
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_WriteSPI(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    int32 Status;
    CFE_SRL_DevType_t DevType;
    CFE_PSP_IODriver_Location_t Location;
    CFE_PSP_IODriver_SerialXfer_t Xfer = {0};

    if (Handle == NULL || Params == NULL) return CFE_SRL_BAD_ARGUMENT;

    DevType = CFE_SRL_GetHandleDevType(Handle);
    if (DevType != SRL_DEVTYPE_SPI) return CFE_SRL_INVALID_TYPE;

    //Mutex Lock
    Status = CFE_SRL_MutexLock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    Location.PspModuleId = CFE_SRL_Global.IOdriverSerialModuleId;
    Location.SubsystemId = CFE_PSP_IODriver_WRITE_SUBSYSTEM;
    Location.SubchannelId = CFE_PSP_IODriver_SERIAL_SPI_SUBCH;

    Xfer.FD = Handle->FD;
    Xfer.Params = *Params;

    Status = CFE_PSP_IODriver_Command(&Location, 0, CFE_PSP_IODriver_VPARG(&Xfer));
    if (Status < 0) {
        Status = CFE_SRL_HANDLE_PSP_SERIAL_ERR(Status);
        goto error;
    }

    // Mutex Unlock
    Status = CFE_SRL_MutexUnlock(Handle);
    if (Status != CFE_SUCCESS) goto error;

    return CFE_SUCCESS;

error:
    CFE_SRL_MutexUnlock(Handle);
    return Status;
}
int32 CFE_SRL_WriteGenericSPI(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    return CFE_SRL_WriteSPI(Handle, Params);
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
int32 CFE_SRL_ReadI2C(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    int32 Status;
    CFE_SRL_DevType_t DevType;
    CFE_PSP_IODriver_Location_t Location;
    CFE_PSP_IODriver_SerialXfer_t Xfer = {0};

    if (Handle == NULL || Params == NULL) return CFE_SRL_BAD_ARGUMENT;

    DevType = CFE_SRL_GetHandleDevType(Handle);
    if (DevType != SRL_DEVTYPE_I2C) return CFE_SRL_INVALID_TYPE;

    // Mutex Lock
    Status = CFE_SRL_MutexLock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    Location.PspModuleId = CFE_SRL_Global.IOdriverSerialModuleId;
    Location.SubsystemId = CFE_PSP_IODriver_READ_SUBSYSTEM;
    Location.SubchannelId = CFE_PSP_IODriver_SERIAL_I2C_SUBCH;

    Xfer.FD = Handle->FD;
    Xfer.Params = *Params;

    Status = CFE_PSP_IODriver_Command(&Location, 0, CFE_PSP_IODriver_VPARG(&Xfer));
    if (Status < 0) {
        Status = CFE_SRL_HANDLE_PSP_SERIAL_ERR(Status);
        goto error;
    }

    // Mutex Unlock
    Status = CFE_SRL_MutexUnlock(Handle);
    if (Status != CFE_SUCCESS) goto error;

    return CFE_SUCCESS;

error:
    CFE_SRL_MutexUnlock(Handle);
    return Status;
}

int32 CFE_SRL_ReadGenericI2C(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    return CFE_SRL_ReadI2C(Handle, Params);
}

/*----------------------------------------------------------------
 *
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_ReadUART(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    // write -> poll read
    int Status;
    CFE_SRL_DevType_t DevType;
    CFE_PSP_IODriver_Location_t Location;
    CFE_PSP_IODriver_SerialXfer_t Xfer = {0};

    if (Handle == NULL || Params == NULL) return CFE_SRL_BAD_ARGUMENT;

    DevType = CFE_SRL_GetHandleDevType(Handle);
    if (DevType != SRL_DEVTYPE_UART && DevType != SRL_DEVTYPE_RS422) return CFE_SRL_INVALID_TYPE;

    // Mutex Lock
    Status = CFE_SRL_MutexLock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    Location.PspModuleId = CFE_SRL_Global.IOdriverSerialModuleId;
    Location.SubsystemId = CFE_PSP_IODriver_READ_SUBSYSTEM;
    Location.SubchannelId = CFE_PSP_IODriver_SERIAL_UART_SUBCH;

    Xfer.FD = Handle->FD;
    Xfer.Params = *Params;

    Status = CFE_PSP_IODriver_Command(&Location, 0, CFE_PSP_IODriver_VPARG(&Xfer));
    if (Status < 0) {
        Status = CFE_SRL_HANDLE_PSP_SERIAL_ERR(Status);
        goto error;
    }

    // Mutex Unlock
    Status = CFE_SRL_MutexUnlock(Handle);
    if (Status != CFE_SUCCESS) goto error;

    return CFE_SUCCESS;

error:
    CFE_SRL_MutexUnlock(Handle);
    return Status;
}

int32 CFE_SRL_ReadGenericUART(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    return CFE_SRL_ReadUART(Handle, Params);
}

/*----------------------------------------------------------------
 *
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_ReadCAN(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    int32 Status;
    CFE_SRL_DevType_t DevType;
    CFE_PSP_IODriver_Location_t Location;
    CFE_PSP_IODriver_SerialXfer_t Xfer = {0};

    if (Handle == NULL || Params == NULL) return CFE_SRL_BAD_ARGUMENT;

    DevType = CFE_SRL_GetHandleDevType(Handle);
    if(DevType != SRL_DEVTYPE_CAN) return CFE_SRL_INVALID_TYPE;

    // Mutex Lock
    Status = CFE_SRL_MutexLock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    Location.PspModuleId = CFE_SRL_Global.IOdriverSerialModuleId;
    Location.SubsystemId = CFE_PSP_IODriver_READ_SUBSYSTEM;
    Location.SubchannelId = CFE_PSP_IODriver_SERIAL_CAN_SUBCH;

    Xfer.FD = Handle->FD;
    Xfer.Params = *Params;

    Status = CFE_PSP_IODriver_Command(&Location, 0, CFE_PSP_IODriver_VPARG(&Xfer));
    if (Status < 0) {
        Status = CFE_SRL_HANDLE_PSP_SERIAL_ERR(Status);
        goto error;
    }

    // Mutex Unlock
    Status = CFE_SRL_MutexUnlock(Handle);
    if (Status != CFE_SUCCESS) goto error;

    return CFE_SUCCESS;

error:
    CFE_SRL_MutexUnlock(Handle);
    return Status;
}

int32 CFE_SRL_ReadGenericCAN(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    return CFE_SRL_ReadCAN(Handle, Params);
}

/*----------------------------------------------------------------
 *
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_ReadSPI(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    int32 Status;
    CFE_SRL_DevType_t DevType;
    CFE_PSP_IODriver_Location_t Location;
    CFE_PSP_IODriver_SerialXfer_t Xfer = {0};

    if (Handle == NULL || Params == NULL) return CFE_SRL_BAD_ARGUMENT;

    DevType = CFE_SRL_GetHandleDevType(Handle);
    if (DevType != SRL_DEVTYPE_SPI) return CFE_SRL_INVALID_TYPE;

    Status = CFE_SRL_MutexLock(Handle);
    if (Status != CFE_SUCCESS) return Status;

    Location.PspModuleId = CFE_SRL_Global.IOdriverSerialModuleId;
    Location.SubsystemId = CFE_PSP_IODriver_READ_SUBSYSTEM;
    Location.SubchannelId = CFE_PSP_IODriver_SERIAL_SPI_SUBCH;

    Xfer.FD = Handle->FD;
    Xfer.Params = *Params;

    Status = CFE_PSP_IODriver_Command(&Location, 0, CFE_PSP_IODriver_VPARG(&Xfer));
    if (Status < 0) {
        Status = CFE_SRL_HANDLE_PSP_SERIAL_ERR(Status);
        goto error;
    }

    Status = CFE_SRL_MutexUnlock(Handle);
    if (Status != CFE_SUCCESS) goto error;

    return CFE_SUCCESS;

error:
    CFE_SRL_MutexUnlock(Handle);
    return Status;
}

int32 CFE_SRL_ReadGenericSPI(CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Params) {
    return CFE_SRL_ReadSPI(Handle, Params);
}

/* GPIO private function */
/*----------------------------------------------------------------
 *
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_GpioSetValue(CFE_SRL_GPIO_Handle_t *Handle, bool Value) {
    int32 Status;
    CFE_PSP_IODriver_Location_t Location;
    CFE_PSP_IODriver_GpioVal_t Val = {0,};

    if (Handle == NULL) return CFE_SRL_BAD_ARGUMENT;

    Location.PspModuleId = CFE_SRL_Global.IOdriverGpioModuleId;
    Location.SubsystemId = CFE_PSP_IODriver_WRITE_SUBSYSTEM;
    
    Val.handle = Handle->Handle;
    Val.level = Value ? 1 : 0;

    Status = CFE_PSP_IODriver_Command(&Location, 0, CFE_PSP_IODriver_VPARG(&Val));
    if (Status < 0) {
        Status = CFE_SRL_HANDLE_PSP_GPIO_ERR(Status);
        return Status;
    }

    return CFE_SUCCESS;
    
}
/*----------------------------------------------------------------
 *
 * Implemented per public API
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 CFE_SRL_GpioGetValue(CFE_SRL_GPIO_Handle_t *Handle, bool *Value) {
    int32 Status;
    CFE_PSP_IODriver_Location_t Location;
    CFE_PSP_IODriver_GpioVal_t Val = {0};

    if (Handle == NULL) return CFE_SRL_BAD_ARGUMENT;

    Location.PspModuleId = CFE_SRL_Global.IOdriverGpioModuleId;
    Location.SubsystemId = CFE_PSP_IODriver_READ_SUBSYSTEM;

    Val.handle = Handle->Handle;

    Status = CFE_PSP_IODriver_Command(&Location, 0, CFE_PSP_IODriver_VPARG(&Val));
    if (Status < 0) {
        Status = CFE_SRL_HANDLE_PSP_GPIO_ERR(Status);
        return Status;
    }

    *Value = Val.level ? true : false;

    return CFE_SUCCESS;
}

/* Handle counter function */
int32 CFE_SRL_UpdateHandleCounters(CFE_SRL_IO_Handle_t *Handle) {
    int32 Status;
    CFE_PSP_IODriver_Location_t Location;
    Location.PspModuleId = CFE_SRL_Global.IOdriverSerialModuleId;
    Location.SubsystemId = CFE_PSP_IODriver_CONFIG_SUBSYSTEM;
    Location.SubchannelId = 0; // meaningless

    Status = CFE_PSP_IODriver_Command(&Location, CFE_PSP_IODriver_SERIAL_IO_GET_CNTS, CFE_PSP_IODriver_VPARG(Handle));
    if (Status != CFE_PSP_SUCCESS) return CFE_SRL_ERR;
    else return CFE_SUCCESS;
}