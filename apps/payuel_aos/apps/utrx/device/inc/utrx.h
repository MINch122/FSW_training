#ifndef _UTRX_H_
#define _UTRX_H_

#include "cfe.h"
// #include <utrx_app/utrx_tlm.h>
#include "utrx_conf.h"
#include "utrx_app.h"
#define AX100_PORT_RPARAM			7    // Controls AX100 with parameter system
#define AX100_PORT_GNDWDT_RESET		9    // Resets the AX100 ground WDT


int32 UTRX_GndwdtClear(void);
int32 UTRX_Reboot(void);

int32 UTRX_RXCONF_GetBaud(uint32 *BaudRxconf);
int32 UTRX_RXCONF_GetFreq(uint32 *FreqRxconf);
int32 UTRX_RXCONF_GetGuard(uint16 *GuardRxconf);
int32 UTRX_TXCONF_GetBaud(uint32 *BaudTxconf);
int32 UTRX_TXCONF_GetFreq(uint32 *FreqTxconf);
int32 UTRX_TLM_GetTempBrd(int16 *TempBrd);
int32 UTRX_TLM_GetLastRssi(int16 *LastRssi);
int32 UTRX_TLM_GetLastRferr(int16 *LastRferr);
int32 UTRX_TLM_GetActiveConf(uint8 *ActiveConf);
int32 UTRX_TLM_GetBootCount(uint16 *BootCount);
int32 UTRX_TLM_GetBootCause(uint32 *BootCause);
int32 UTRX_TLM_GetLastContact(uint32 *LastContact);
int32 UTRX_TLM_GetTotTxBytes(uint32 *TotTxBytes);
int32 UTRX_TLM_GetTotRxBytes(uint32 *TotRxBytes);

int32 UTRX_RXCONF_SetBaud(uint32 BaudRxconf);
int32 UTRX_TXCONF_SetBaud(uint32 BaudTxconf);
int32 UTRX_RXCONF_SetFreq(uint32 FreqRxconf);
int32 UTRX_TXCONF_SetFreq(uint32 FreqTxconf);

// int32 UTRX_GetStatusConfiguration(UTRX_ConfBitTable_t *StatusConfiguration);
int32 UTRX_SetDefaultBaud(void);
int32 UTRX_RparamSave1(void);
int32 UTRX_RparamSave5(void);
int32 UTRX_RparamSaveAll(void);

int32 csp_checkstate_ping(uint8_t node);


// int32 UTRX_RequestHK(UTRX_HK_t *UTRX_HK);
// int32 UTRX_RequestBCN(UTRX_BCN_t *UTRX_BCN);

// int32 UTRX_AddDataToSlot(AppDataSlot_t *DataSlot, uint16 CommandCode, uint16 DataLen, void* Data);
typedef enum {
    CMD_RETCODE_TYPE_CFE,
    CMD_RETCODE_TYPE_APP,
    CMD_RETCODE_TYPE_DRIVER,
    
}UTRX_AppCmdRetcode_Type_n;



typedef enum {
    DEVICE_SUCCESS   = 0,
    DEVICE_ERROR     = -1,
    DEVICE_ERR_NULL  = -2,  
} UTRX_DeviceRetcode_t; //define error code


 
 #define CSP_DEFAULT_TIMEOUT_LOCAL 1000

#endif