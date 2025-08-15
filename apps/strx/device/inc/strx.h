#ifndef _STRX_H_
#define _STRX_H_

#include <cfe.h>
// #include <strx_app/strx_tlm.h>
#include "strx_conf.h"
#include "strx_app.h"

// #define CSP_CMP                     0    //Control Port
// #define CSP_PING                    1    // Returns a copy of the packet received
// #define CSP_PS                      2    // Returns processs list
// #define CSP_MEMFREE                 3    // Returns memory free
// #define CSP_PORT_REBOOT             4    // Reboots subsystem
// #define CSP_BUF_FREE                5    // Returns number of free buffers
// #define CSP_UPTIME                  6    // Returns subsystem uptime
#define AX2150_PORT_RPARAM			    7    // Controls AX100 with parameter system
#define AX2150_PORT_GNDWDT_RESET		9    // Resets the AX100 ground WDT

int32 STRX_GndwdtClear(void);
int32 STRX_Reboot(void);

int32 STRX_RXCONF_GetBaud(uint32 *BaudRxconf);
int32 STRX_RXCONF_GetFreq(uint32 *FreqRxconf);
int32 STRX_RXCONF_GetGuard(uint16 *GuardRxconf);
int32 STRX_TXCONF_GetBaud(uint32 *BaudTxconf);
int32 STRX_TXCONF_GetFreq(uint32 *FreqTxconf);
int32_t STRX_TXCONF_GetGuard(uint16_t *GuardTxconf);
int32 STRX_TLM_GetTempBrd(int16 *TempBrd);
int32 STRX_TLM_GetLastRssi(int16 *LastRssi);
int32 STRX_TLM_GetLastRferr(int16 *LastRferr);
int32 STRX_TLM_GetBootCount(uint16 *BootCount);
int32 STRX_TLM_GetBootCause(uint32 *BootCause);
int32 STRX_TLM_GetLastContact(uint32 *LastContact);
int32 STRX_TLM_GetTotTxBytes(uint32 *TotTxBytes);
int32 STRX_TLM_GetTotRxBytes(uint32 *TotRxBytes);
int32_t STRX_TLM_GET_HWDET(int8_t *HWDET);
int32_t STRX_TLM_GET_RXMODE(uint8_t *Rxmode);
int32_t STRX_TLM_GET_GND_WDT_CNT(uint16_t *Wdt_cnt);
int32_t STRX_TLM_GET_GND_WDT_LEFT(uint32_t *Wdt_left);
int32_t STRX_TLM_GetRssibusy(int16_t *Rassibusy);

int32_t STRX_TLM_GET_KISS_USART(int8_t *Kiss);
int32_t STRX_TLM_SET_KISS_USART(int8_t Kiss);
int32_t STRX_TLM_GET_GOSH_USART(uint8_t *Gosh);
int32_t STRX_TLM_SET_GOSH_USART(uint8_t Gosh);

int32 STRX_RXCONF_SetBaud(uint32 BaudRxconf);
int32 STRX_TXCONF_SetBaud(uint32 BaudTxconf);
int32 STRX_RXCONF_SetFreq(uint32 FreqRxconf);
int32 STRX_TXCONF_SetFreq(uint32 FreqTxconf);

int32 STRX_GetStatusConfiguration(STRX_ConfBitTable_t *StatusConfiguration);
int32 STRX_SetDefaultBaud(void);
// int32 STRX_SetDefaultGuard(void);
int32 STRX_RparamSave0(void);
int32 STRX_RparamSave1(void);
int32 STRX_RparamSave4(void);
int32 STRX_RparamSave5(void);
int32 STRX_RparamSaveAll(void);


// int32 STRX_RequestHK(STRX_HK_t *STRX_HK);
// int32 STRX_RequestBCN(STRX_BCN_t *STRX_BCN);

// int32 STRX_AddDataToSlot(AppDataSlot_t *DataSlot, uint16 CommandCode, uint16 DataLen, void* Data);
typedef enum {
    CMD_RETCODE_TYPE_OK,
    CMD_RETCODE_TYPE_UNKNOWN,
    CMD_RETCODE_TYPE_CFE,
    CMD_RETCODE_TYPE_FSW_IMPL,
    CMD_RETCODE_TYPE_FSW_TBL,
    CMD_RETCODE_TYPE_OSAL,
    CMD_RETCODE_TYPE_PSP,
    CMD_RETCODE_TYPE_APP,
    CMD_RETCODE_TYPE_LIB,
    CMD_RETCODE_TYPE_DRIVER,
    CMD_RETCODE_TYPE_USER,
    CMD_RETCODE_TYPE_BADCMD,
    CMD_RETCODE_TYPE_PARLEN,
    CMD_RETCODE_TYPE_APPDATA,
    CMD_RETCODE_TYPE_APPUTILS,
}STRX_AppCmdRetcode_Type_n;



typedef enum {
    DEVICE_SUCCESS   = 0,
    DEVICE_ERROR     = -1,
    DEVICE_ERR_NULL  = -2,
    DEVICE_ERR_RANGE = -3,
    DEVICE_ERR_READ  = -4,
    DEVICE_ERR_WRITE = -5
} STRX_DeviceRetcode_t; //define error code

#define COMMAND_CASE_HANDLING_NOARG(status_type, mtype, func) \
    if (VerifyCmdLength(MsgPtr, sizeof(mtype))) { \
        status = func(); \
        statusType = status_type; \
    }

#define COMMAND_CASE_HANDLING_ARG(status_type, mtype, func, arg) \
    if (VerifyCmdLength(MsgPtr, sizeof(mtype))) { \
        status = func(((mtype*)MsgPtr)->arg); \
        statusType = status_type; \
    }


 typedef enum {
    /**
     * Generic errors.
     */
    IFC_INFO_EMERGENCY_PKT  = 1,

    IFC_OK                  =  0,  /* operation successful. */
    IFC_ERROR               = -1,  /* generic error. */
    IFC_ERR_NULL            = -2,  /* null pointer(s) passed. */
    IFC_ERR_NOMEM           = -3,  /* malloc() failed. */
    IFC_ERR_RANGE           = -4,  /* argument is out of range. */
    IFC_ERR_INVALID         = -5,  /* invalid operation or arguments. */
    IFC_ERR_EXISTS          = -6,  /* target already exists. */
    IFC_ERR_NOT_FOUND       = -7,  /* target item not found. */
    IFC_ERR_EMPTY           = -8,  /* target is empty or not initialized. */
    IFC_ERR_NAME_LENGTH     = -9,  /* name too long. */
    IFC_ERR_FULL            = -10, /* no empty slot. */
 } ifc_ret_t;

 #define CSP_DEFAULT_TIMEOUT_LOCAL 1000

int32 csp_checkstate_ping(uint8_t node);


#endif