#include "cfe_srl_handle.h"

#include <pthread.h>
#include <string.h>

#include <fcntl.h>

#include "cfe_srl_priv.h"
static CFE_SRL_Global_Handle_t GlobalHandle[CFE_SRL_GLOBAL_HANDLE_NUM] = {0};


int CFE_SRL_PriorInit(void) {
    int Status;

    Status = CFE_SRL_GlobalHandleMutexInit();
    if (Status != CFE_SUCCESS) {
        return CFE_SRL_PRIOR_INIT_ERR;
    }

    return CFE_SUCCESS;
}

int CFE_SRL_GetOpenOption(CFE_SRL_DevType_t Devtype) {
    switch (Devtype) {
        case SRL_DEVTYPE_I2C:
        case SRL_DEVTYPE_SPI:
        case SRL_DEVTYPE_CAN: // Meaningless
            return O_RDWR;
        case SRL_DEVTYPE_UART:
        case SRL_DEVTYPE_RS422:
            return O_RDWR | O_NOCTTY;
            // return O_RDWR | O_NOCTTY | O_NONBLOCK;
        default:
            break;
    }
    return CFE_SRL_TYPE_UNSUPPORTED;
}


int CFE_SRL_GlobalHandleInit(CFE_SRL_IO_Handle_t **Handle, const char *Name, const char *DevName, uint8_t DevType) {
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
    //Mutex Unlock
    CFE_SRL_GlobalHandleMutexUnlock();
    return CFE_SRL_FULL_ERR;
}


void CFE_SRL_SetTRxFunction(CFE_SRL_IO_Handle_t *Handle) {
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


int CFE_SRL_HandleInit(CFE_SRL_IO_Handle_t **Handle, const char *Name, const char *Devname, uint8_t DevType, uint8_t MutexID, uint32_t BaudRate, uint8_t SPIMode) {
    int Status;
    CFE_SRL_IO_Handle_t *TempHandle;
    int OpenOption;

    // Mutex value check


    // Get Open Option
    OpenOption = CFE_SRL_GetOpenOption(DevType);
    if (OpenOption == CFE_SRL_TYPE_UNSUPPORTED) return CFE_SRL_HANDLE_INIT_ERR;

    // GlobalHandle Init
    Status = CFE_SRL_GlobalHandleInit(&TempHandle, Name, Devname, DevType);
    if (Status != CFE_SUCCESS) return Status;     // Revise `1` to `OK`

    if (*Handle != NULL) { // Which means, 'already initialized'
        CFE_ES_WriteToSysLog("%s: %s Handle alreay initialized!", 
            __func__, ((CFE_SRL_Global_Handle_t *)*Handle)->DevName);
        return CFE_SUCCESS;
    }
    else {
        if (DevType == SRL_DEVTYPE_CAN) {
            Status = CFE_SRL_OpenSocket(TempHandle, Devname); // Do socket()
            if (Status != CFE_SUCCESS) return Status;
        }
        else {
            Status = CFE_SRL_Open(TempHandle, Devname, OpenOption); // Do open()
            if (Status != CFE_SUCCESS) return Status;
        }
    
        // Status = CFE_SRL_DevOpenInsert(Devname, TempHandle->FD); // Add to `DeviceTable`
        if (Status != CFE_SUCCESS) return Status;
    }
    
    // IO Handle Mutex Init
    Status = CFE_SRL_HandleMutexInit(TempHandle, MutexID);
    if (Status != CFE_SUCCESS) return Status;

    // Allocate the result Handle
    *Handle = TempHandle;
    
    if (DevType == SRL_DEVTYPE_UART || DevType == SRL_DEVTYPE_RS422) {
        Status = CFE_SRL_BasicSetUART(*Handle, BaudRate);

        if (Status != CFE_SUCCESS) return CFE_SRL_UART_SET_ERR;
    }

    if (DevType == SRL_DEVTYPE_SPI) {
        Status = CFE_SRL_SetSPI(*Handle, SPIMode, BaudRate, 8);
    }

    CFE_SRL_SetTRxFunction(*Handle);

    return CFE_SUCCESS;
}


int CFE_SRL_HandleClose(CFE_SRL_IO_Handle_t *Handle) {
    int Status;
    CFE_SRL_Global_Handle_t *Entry;

    if (Handle == NULL) return CFE_SRL_BAD_ARGUMENT;

    Entry = (CFE_SRL_Global_Handle_t *)Handle;

    Status = CFE_SRL_Close(Handle);
    if (Status != CFE_SUCCESS) return Status;

    Status = CFE_SRL_MutexDestroy(Handle);
    if (Status != CFE_SUCCESS) return Status;

    Entry->Status = CFE_SRL_HANDLE_STATUS_NONE;
    memset(Entry, 0, sizeof(CFE_SRL_Global_Handle_t));
    
    return CFE_SUCCESS;
}