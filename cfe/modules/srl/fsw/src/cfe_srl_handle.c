#include "cfe_srl_module_all.h"

#include <string.h>

#include <fcntl.h>

static CFE_SRL_Global_Handle_t GlobalHandle[CFE_SRL_GLOBAL_HANDLE_NUM] = {0};

int CFE_SRL_PriorInit(void) {
    int Status;

    /* Clear SRL Global */
    /* Should "NOT" cleared in task init */
    memset(&CFE_SRL_Global, 0, sizeof(CFE_SRL_Global));

    if (CFE_PSP_IODriver_FindByName(CFE_SRL_SERIAL_DRIVER, &CFE_SRL_Global.IOdriverSerialModuleId) != CFE_PSP_SUCCESS) {
        CFE_ES_WriteToSysLog("%s: PSP serial driver unavailable.\n", __func__);
        return CFE_SRL_PRIOR_INIT_ERR;
    }

    /* Temporarily deprecate the gpio module */
    // if (CFE_PSP_IODriver_FindByName(CFE_SRL_DISCRETE_DRIVER, &CFE_SRL_Global.IOdriverGpioModuleId) != CFE_PSP_SUCCESS) {
    //     CFE_ES_WriteToSysLog("%s: PSP gpio driver unavailable.\n", __func__);
    //     return CFE_SRL_PRIOR_INIT_ERR;
    // }
    Status = CFE_SRL_GlobalHandleMutexInit();
    if (Status != CFE_SUCCESS) {
        CFE_ES_WriteToSysLog("%s: Global Handle Mutex create failed.\n", __func__);
        return CFE_SRL_PRIOR_INIT_ERR;
    }

    return CFE_SUCCESS;
}

bool CFE_SRL_QueryStatus(const CFE_SRL_Global_Handle_t *Entry, CFE_SRL_Handle_Status_t Query) {
    return (Entry->Status & Query) == Query;
}

int CFE_SRL_SetHandleStatus(CFE_SRL_IO_Handle_t *Handle, uint8_t Label, bool Set) {
    if (Handle == NULL) return CFE_SRL_BAD_ARGUMENT;
    CFE_SRL_Global_Handle_t *Entry = (CFE_SRL_Global_Handle_t *)Handle;

    if (Set == true) Entry->Status |= Label;
    else Entry->Status &= ~Label;

    return CFE_SUCCESS;
}

CFE_SRL_DevType_t CFE_SRL_GetHandleDevType(CFE_SRL_IO_Handle_t *Handle) {
    if (Handle == NULL) return CFE_SRL_BAD_ARGUMENT;

    CFE_SRL_Global_Handle_t *Entry = (CFE_SRL_Global_Handle_t *)Handle;
    
    return Entry->DevType;
}


static int CFE_SRL_GlobalHandleInit(CFE_SRL_IO_Handle_t **Handle, const char *Name, const char *DevName, uint8_t DevType) {
    CFE_SRL_Global_Handle_t *Entry;
    int Status;
    
    // Mutex Lock
    Status = CFE_SRL_GlobalHandleMutexLock();
    if (Status != CFE_SUCCESS) return Status;

    // If Not Found, Do Main process
    for (uint8_t i = 0; i < CFE_SRL_GLOBAL_HANDLE_NUM; i++) {
        Entry = &GlobalHandle[i];
        if (!CFE_SRL_QueryStatus(Entry, CFE_SRL_HANDLE_STATUS_ALLOCATE)) {
            // memset(Entry, 0, sizeof(CFE_SRL_Global_Handle_t));
            
            strcpy(Entry->Name, Name);
            // Entry->Name[sizeof(Entry->Name)-1] = '\0';
            
            Entry->DevType = DevType;
            
            strcpy(Entry->DevName, DevName);
            // Entry->DevName[sizeof(Entry->DevName)-1] = '\0';
            
            Entry->Status = CFE_SRL_HANDLE_STATUS_ALLOCATE;
            CFE_SRL_SetHandleStatus(&Entry->Handle, CFE_SRL_HANDLE_STATUS_ALLOCATE, true);
            
            *Handle = &Entry->Handle;

            //Mutex Unlock
            CFE_SRL_GlobalHandleMutexUnlock();
            return CFE_SUCCESS;
        }
    }
    // Mutex Unlock
    CFE_SRL_GlobalHandleMutexUnlock();
    return CFE_SRL_FULL_ERR;
}


static void CFE_SRL_SetTRxFunction(CFE_SRL_IO_Handle_t *Handle) {
    const CFE_SRL_Global_Handle_t *Entry = (CFE_SRL_Global_Handle_t *)Handle;

    switch (Entry->DevType)
    {
    case SRL_DEVTYPE_I2C:
        Handle->Func.TxFunc = CFE_SRL_WriteGenericI2C;
        Handle->Func.RxFunc = CFE_SRL_ReadGenericI2C;
        break;
    case SRL_DEVTYPE_SPI:
        Handle->Func.TxFunc = CFE_SRL_WriteGenericSPI;
        Handle->Func.RxFunc = CFE_SRL_ReadGenericSPI;
        break;
    case SRL_DEVTYPE_CAN:
        Handle->Func.TxFunc = CFE_SRL_WriteGenericCAN;
        Handle->Func.RxFunc = CFE_SRL_ReadGenericCAN;
        break;
    case SRL_DEVTYPE_UART:
    case SRL_DEVTYPE_RS422:
        Handle->Func.TxFunc = CFE_SRL_WriteGenericUART;
        Handle->Func.RxFunc = CFE_SRL_ReadGenericUART;
        break;
    default:
        Handle->Func.TxFunc = NULL;
        Handle->Func.RxFunc = NULL;
        break;
    }
    
    return;
}

/* `Config` Parameter should correctly include the FD value */
int CFE_SRL_ConfigHandle(uint8_t DevType, CFE_PSP_IODriver_Serial_cfg_t *Config) {
    int32 Status;
    CFE_PSP_IODriver_Location_t Location;

    Location.PspModuleId = CFE_SRL_Global.IOdriverSerialModuleId;
    Location.SubsystemId = CFE_PSP_IODriver_CONFIG_SUBSYSTEM;
    switch (DevType)
    {
        case SRL_DEVTYPE_I2C:
            Location.SubchannelId = CFE_PSP_IODriver_SERIAL_I2C_SUBCH;
            break;
        case SRL_DEVTYPE_SPI:
            Location.SubchannelId = CFE_PSP_IODriver_SERIAL_SPI_SUBCH;
            break;
        case SRL_DEVTYPE_CAN:
            Location.SubchannelId = CFE_PSP_IODriver_SERIAL_CAN_SUBCH;
            break;
        case SRL_DEVTYPE_UART:
            Location.SubchannelId = CFE_PSP_IODriver_SERIAL_UART_SUBCH;
            break;
        default:
            Location.SubchannelId = CFE_PSP_IODriver_SERIAL_SUBCH_MAX;
            break;
    }

    Status = CFE_PSP_IODriver_Command(&Location, CFE_PSP_IODriver_SET_CONFIGURATION,
                                        CFE_PSP_IODriver_VPARG(Config));

    /* Error Handling */
    if (Status < 0) {
        CFE_ES_WriteToSysLog("%s: DevType %u config failed. PSP RC =%d\n", __func__, DevType, Status);
        switch (Location.SubchannelId)
        {
            case CFE_PSP_IODriver_SERIAL_I2C_SUBCH:
                return CFE_SRL_I2C_CONFIG_FAIL_ERR;
            case CFE_PSP_IODriver_SERIAL_SPI_SUBCH:
                return CFE_SRL_SPI_CONFIG_FAIL_ERR;
            case CFE_PSP_IODriver_SERIAL_UART_SUBCH:
                return CFE_SRL_UART_CONFIG_FAIL_ERR;
            case CFE_PSP_IODriver_SERIAL_CAN_SUBCH:
                return CFE_SRL_CAN_CONFIG_FAIL_ERR;
            default:
                return CFE_SRL_BAD_ARGUMENT;
        }
    }

    return CFE_SUCCESS;
}


int CFE_SRL_HandleInit(CFE_SRL_IO_Handle_t **Handle, const char *Name, const char *Devname, uint8_t DevType, uint8_t MutexIdx, CFE_PSP_IODriver_Serial_cfg_t *Config) {
    int Status;
    CFE_SRL_IO_Handle_t *TempHandle;
    CFE_PSP_IODriver_Location_t Location;

    // GlobalHandle Init
    Status = CFE_SRL_GlobalHandleInit(&TempHandle, Name, Devname, DevType);
    if (Status != CFE_SUCCESS) return Status;     // Revise `1` to `OK`

    if (*Handle != NULL) { // Which means, 'already initialized'
        CFE_ES_WriteToSysLog("%s: %s Handle alreay initialized.", 
            __func__, ((CFE_SRL_Global_Handle_t *)*Handle)->DevName);
        return CFE_SUCCESS;
    }
    else {
        Location.PspModuleId = CFE_SRL_Global.IOdriverSerialModuleId;
        Location.SubsystemId = CFE_PSP_IODriver_OPEN_SUBSYSTEM;
        switch (DevType)
        {
            case SRL_DEVTYPE_I2C:
                Location.SubchannelId = CFE_PSP_IODriver_SERIAL_I2C_SUBCH;
                break;
            case SRL_DEVTYPE_SPI:
                Location.SubchannelId = CFE_PSP_IODriver_SERIAL_SPI_SUBCH;
                break;
            case SRL_DEVTYPE_CAN:
                Location.SubchannelId = CFE_PSP_IODriver_SERIAL_CAN_SUBCH;
                break;
            case SRL_DEVTYPE_UART:
                Location.SubchannelId = CFE_PSP_IODriver_SERIAL_UART_SUBCH;
                break;
            default:
                Location.SubchannelId = CFE_PSP_IODriver_SERIAL_SUBCH_MAX;
                break;
        }

        Status = CFE_PSP_IODriver_Command(&Location, 0, CFE_PSP_IODriver_CONST_STR(Devname));
        /* Error Handling */
        if (Status < 0) {
            CFE_ES_WriteToSysLog("%s: %s open failed. PSP RC =%d\n", __func__, Devname, Status);
            switch (Location.SubchannelId)
            {
                case CFE_PSP_IODriver_SERIAL_I2C_SUBCH:
                    return CFE_SRL_OPEN_ERR;
                case CFE_PSP_IODriver_SERIAL_SPI_SUBCH:
                    return CFE_SRL_OPEN_ERR;
                case CFE_PSP_IODriver_SERIAL_UART_SUBCH:
                    return CFE_SRL_OPEN_ERR;
                case CFE_PSP_IODriver_SERIAL_CAN_SUBCH:
                    return CFE_SRL_CAN_OPEN_SOCKET_ERR;
                default:
                    return CFE_SRL_BAD_ARGUMENT;
            }
        }
        TempHandle->FD = Status;
    }
    
    /* Config comm. specification for each protocol */
    /* Set FD to Config object */
    if (Config) {
        Config->FD = TempHandle->FD;
        Status = CFE_SRL_ConfigHandle(DevType, Config);
    }
    
    /* If open & config successfully done, set handle status to `FD_INIT` */
    Status = CFE_SRL_SetHandleStatus(TempHandle, CFE_SRL_HANDLE_STATUS_FD_INIT, true);
    if (Status != CFE_SUCCESS) return Status;

    /* IO Handle Mutex Init */
    Status = CFE_SRL_HandleMutexInit(TempHandle, MutexIdx, Name);
    if (Status != CFE_SUCCESS) return Status;

    // Allocate the result Handle
    *Handle = TempHandle;

    CFE_SRL_SetTRxFunction(*Handle);

    return CFE_SUCCESS;
}


int CFE_SRL_HandleClose(CFE_SRL_IO_Handle_t **Handle) {
    int Status;
    CFE_SRL_Global_Handle_t *Entry;
    CFE_PSP_IODriver_Location_t Location = {0};
    CFE_PSP_IODriver_SerialXfer_t Xfer = {0};
    const char *Name = NULL;

    if (*Handle == NULL) return CFE_SRL_BAD_ARGUMENT;

    Entry = (CFE_SRL_Global_Handle_t *)*Handle;
    Name = Entry->DevName;

    if (!CFE_SRL_QueryStatus((CFE_SRL_Global_Handle_t *)*Handle, CFE_SRL_HANDLE_STATUS_FD_INIT)) {
        return CFE_SRL_NOT_OPEN_ERR;
    }

    Location.PspModuleId = CFE_SRL_Global.IOdriverSerialModuleId;
    Location.SubsystemId = CFE_PSP_IODriver_CLOSE_SUBSYSTEM;
    /* `.SubchannelId` is not needed */
    
    Xfer.FD = (*Handle)->FD;

    Status = CFE_PSP_IODriver_Command(&Location, 0, CFE_PSP_IODriver_VPARG(&Xfer));
    if (Status < 0) {
        CFE_ES_WriteToSysLog("%s: %s close failed. PSP RC =%d\n", __func__, Name, Status);
        Status = CFE_SRL_CLOSE_ERR;
    }

    Status = CFE_SRL_MutexDestroy(*Handle);
    if (Status != CFE_SUCCESS) Status = CFE_SRL_MUTEX_ERR;

    Entry->Status = CFE_SRL_HANDLE_STATUS_NONE;
    memset(Entry, 0, sizeof(CFE_SRL_Global_Handle_t));
    
    *Handle = NULL;

    return Status;
}

/*************************************************************
 * 
 *  GPIO Initialization Function
 * 
 *************************************************************/
int CFE_SRL_GpioInit(CFE_SRL_GPIO_Handle_t *Handle, const char *Path, unsigned int Line, const char *Name, bool Default, bool IsOut) {
    if (Handle == NULL || Path == NULL) return CFE_SRL_BAD_ARGUMENT;

    int32_t Status;
    CFE_PSP_IODriver_Location_t Location = (CFE_PSP_IODriver_Location_t) {.PspModuleId = CFE_SRL_Global.IOdriverGpioModuleId,
                                                                          .SubsystemId = CFE_PSP_IODriver_OPEN_SUBSYSTEM,
                                                                          .SubchannelId = 0};
    CFE_PSP_IODriver_Gpio_init_t Config = (CFE_PSP_IODriver_Gpio_init_t) {.path = Path,
                                                                          .line = Line,
                                                                          .name = Name,
                                                                          .default_val = Default,
                                                                          .isout = IsOut};

    Status = CFE_PSP_IODriver_Command(&Location, 0, CFE_PSP_IODriver_VPARG(&Config));
    if (Status < 0) {
        CFE_ES_WriteToSysLog("%s: %s Gpio Init failed. PSP RC = %d\n", __func__, Path, Status);
        return CFE_SRL_GPIO_CONFIG_FAIL_ERR;
    }

    /* Allocate Handle. this value is similar to FD */
    Handle->Handle = Status;

    return CFE_SUCCESS;

}

int CFE_SRL_GpioClose(CFE_SRL_GPIO_Handle_t *Handle) {
    if (Handle == NULL || Handle->Handle < 0) return CFE_SRL_BAD_ARGUMENT;

    int32 Status;
    CFE_PSP_IODriver_Location_t Location = (CFE_PSP_IODriver_Location_t) {.PspModuleId = CFE_SRL_Global.IOdriverGpioModuleId,
                                                                          .SubsystemId = CFE_PSP_IODriver_CLOSE_SUBSYSTEM,
                                                                          .SubchannelId = 0};

    Status = CFE_PSP_IODriver_Command(&Location, 0, CFE_PSP_IODriver_U32ARG((uint32_t)Handle->Handle));
    if(Status < 0) {
        CFE_ES_WriteToSysLog("%s: Gpio close failed. PSP RC = %d\n", __func__, Status);
        return CFE_SRL_CLOSE_ERR;
    }

    return CFE_SUCCESS;
}