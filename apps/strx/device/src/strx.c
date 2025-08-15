#include <csp/csp.h>
#include <csp/csp_endian.h> // CSP_PRIO_HIGH
#include <gs/param/rparam.h>

#include "cfe.h"

#include "../inc/strx.h"
#include "../inc/strx_conf.h"
#include "../../fsw/inc/strx_app_eventids.h"



//Process Command Function /////////////////////////////////////////////////////////////


int32_t STRX_GndwdtClear(void)
{
    int status;
    // status = csp_transaction_w_opts(CSP_PRIO_NORM, CSP_NODE_STRX, AX2150_PORT_GNDWDT_RESET, 1000, NULL, 0, NULL, 0,CSP_O_CRC32);
    status = CFE_SRL_ApiTransactionCSP(CSP_NODE_STRX, AX2150_PORT_GNDWDT_RESET, NULL, 0, NULL, 0);

    if (status <= 0) {
        CFE_ES_WriteToSysLog("%s: Failed to clear Ground Watchdog! status=%d\n", __func__, status);
        return CFE_SRL_TRANSACTION_ERR;  
    }

    return CFE_SUCCESS; 
}

int32_t STRX_Reboot(void)
{
    int32_t status = DEVICE_ERROR;
    uint32_t magic_word = csp_hton32(CSP_REBOOT_MAGIC);

    // status = csp_transaction_w_opts(CSP_PRIO_NORM,CSP_NODE_STRX,CSP_REBOOT,1000, &magic_word, sizeof(magic_word), NULL,0,CSP_O_CRC32);
    status = CFE_SRL_ApiTransactionCSP(CSP_NODE_STRX, CSP_REBOOT, &magic_word, sizeof(magic_word), NULL, 0);
      if (status <= 0) {
        CFE_ES_WriteToSysLog("%s: Reboot command failed! status=%d\n", __func__, status);
        return CFE_SRL_TRANSACTION_ERR;
    }
    
    
    return status;
}

int32_t STRX_RXCONF_SetFreq(uint32_t FreqRxconf)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_RXCONF;
    uint16_t addr = STRX_ADDR_RXCONF_FREQ;
    if ((status = CFE_SRL_ApiSetRparamCSP(GS_PARAM_UINT32, CSP_NODE_STRX, table_id, addr, &FreqRxconf)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TXCONF_SET_BAUD_ERR_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t STRX_RXCONF_GetFreq(uint32_t *FreqRxconf)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_RXCONF;
    uint16_t addr = STRX_ADDR_RXCONF_FREQ;
    if ((status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT32, CSP_NODE_STRX, table_id, addr, FreqRxconf)) < 0)
    {
        CFE_EVS_SendEvent(STRX_RXCONF_GET_FREQ_ERR_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    

    return status;
}

int32_t STRX_RXCONF_GetBaud(uint32_t *BaudRxconf)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_RXCONF;
    uint16_t addr = STRX_ADDR_RXCONF_BAUD;
    if ((status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT32, CSP_NODE_STRX, table_id, addr, BaudRxconf)) < 0)
    {
        CFE_EVS_SendEvent(STRX_RXCONF_GET_BAUD_ERR_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    

    return status;
}

int32_t STRX_RXCONF_SetBaud(uint32_t BaudRxconf)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_RXCONF;
    uint16_t addr = STRX_ADDR_RXCONF_BAUD;

    if ((status = CFE_SRL_ApiSetRparamCSP(GS_PARAM_UINT32, CSP_NODE_STRX, table_id, addr, &BaudRxconf)) < 0)
    {
        CFE_EVS_SendEvent(STRX_RXCONF_SET_BAUD_ERR_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }


    return status;
}

int32_t STRX_TXCONF_SetFreq(uint32_t FreqTxconf)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_TXCONF;
    uint16_t addr = STRX_ADDR_TXCONF_FREQ;

    if ((status = CFE_SRL_ApiSetRparamCSP(GS_PARAM_UINT32, CSP_NODE_STRX, table_id, addr, &FreqTxconf)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TXCONF_SET_BAUD_ERR_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }


    return status;
}

int32_t STRX_TXCONF_GetFreq(uint32_t *FreqTxconf)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_TXCONF;
    uint16_t addr = STRX_ADDR_TXCONF_FREQ; 
    if ((status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT32, CSP_NODE_STRX, table_id, addr, FreqTxconf)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TXCONF_GET_FREQ_ERR_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t STRX_TXCONF_SetBaud(uint32_t BaudTxconf)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_TXCONF;
    uint16_t addr = STRX_ADDR_TXCONF_BAUD;
    if ((status = CFE_SRL_ApiSetRparamCSP(GS_PARAM_UINT32, CSP_NODE_STRX, table_id, addr, &BaudTxconf)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TXCONF_SET_BAUD_ERR_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t STRX_SetDefaultBaud(void)
{
    int32_t status = DEVICE_SUCCESS;
    if (STRX_RXCONF_SetBaud(STRX_DEFUALT_BAUDRATE) != DEVICE_SUCCESS) { status = DEVICE_ERROR; }
    if (STRX_TXCONF_SetBaud(STRX_DEFUALT_BAUDRATE) != DEVICE_SUCCESS) { status = DEVICE_ERROR; }

    return status;
}

int32_t STRX_RparamSave0(void)      
{
    int32_t status = DEVICE_SUCCESS;

    status = CFE_SRL_ApiRparamSaveCSP(CSP_NODE_STRX, 1000, STRX_TABLE_SYSCONF, STRX_TABLE_SYSCONF);
    if (status < 0) {
        CFE_EVS_SendEvent(STRX_RPARAM_SAVE_0_ERR_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d)", status, STRX_TABLE_SYSCONF);
    }

    return status;
}

int32_t STRX_RparamSave1(void)       
{
    int32_t status = DEVICE_SUCCESS;

    status = CFE_SRL_ApiRparamSaveCSP(CSP_NODE_STRX, 1000, STRX_TABLE_RXCONF, STRX_TABLE_RXCONF);
    if (status < 0) {
        CFE_EVS_SendEvent(STRX_RPARAM_SAVE_1_ERR_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d)", status, STRX_TABLE_RXCONF);
    }

    return status;
}

int32_t STRX_RparamSave4(void)       
{
    int32_t status = DEVICE_SUCCESS;

    status = CFE_SRL_ApiRparamSaveCSP(CSP_NODE_STRX, 1000, STRX_TABLE_TLM, STRX_TABLE_TLM);
    if (status < 0) {
        CFE_EVS_SendEvent(STRX_RPARAM_SAVE_4_ERR_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d)", status, STRX_TABLE_TLM);
    }

    return status;
}

int32_t STRX_RparamSave5(void)
{
    int32_t status = DEVICE_SUCCESS;

    status = CFE_SRL_ApiRparamSaveCSP(CSP_NODE_STRX, 1000, STRX_TABLE_TXCONF, STRX_TABLE_TXCONF);
    if (status < 0) {
        CFE_EVS_SendEvent(STRX_RPARAM_SAVE_5_ERR_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d)", status, STRX_TABLE_TXCONF);
    }

    return status;
}

int32_t STRX_RparamSaveAll(void)
{
    int32_t status = DEVICE_SUCCESS;
if (STRX_RparamSave0() != DEVICE_SUCCESS) { status = DEVICE_ERROR; }
if (STRX_RparamSave1() != DEVICE_SUCCESS) { status = DEVICE_ERROR; }
if (STRX_RparamSave4() != DEVICE_SUCCESS) { status = DEVICE_ERROR; }
if (STRX_RparamSave5() != DEVICE_SUCCESS) { status = DEVICE_ERROR; }

    return status;
}

/////////////////////////////////////////////////////////////////////////////////
//Request Telemetery/////////////////////////////////////////////////////////////

int32_t STRX_GetStatusConfiguration(STRX_ConfBitTable_t *StatusConfiguration)
{
    int32_t status = DEVICE_SUCCESS;

    status |= STRX_RXCONF_GetBaud(&(StatusConfiguration->BaudRxconf));
    status |= STRX_RXCONF_GetGuard(&(StatusConfiguration->GuardRxconf));
    status |= STRX_TXCONF_GetBaud(&(StatusConfiguration->BaudTxconf));

    return status;
}

int32_t STRX_RXCONF_GetGuard(uint16_t *GuardRxconf)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_RXCONF;
    uint16_t addr = STRX_ADDR_RXCONF_GUARD;

    if ((status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT16,CSP_NODE_STRX, table_id, addr, GuardRxconf)) < 0)
    {
        CFE_EVS_SendEvent(STRX_RXCONF_GET_GUARD_ERR_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t STRX_TXCONF_GetGuard(uint16_t *GuardTxconf)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_TXCONF;
    uint16_t addr = STRX_ADDR_TXCONF_GUARD;

    if ((status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT16,CSP_NODE_STRX, table_id, addr, GuardTxconf)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TXCONF_GET_GUARD_ERR_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t STRX_TXCONF_GetBaud(uint32_t *BaudTxconf)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_TXCONF;
    uint16_t addr = STRX_ADDR_TXCONF_BAUD;
    
    if ((status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT32, CSP_NODE_STRX, table_id, addr, BaudTxconf)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TXCONF_GET_BAUD_ERR_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t STRX_TLM_GetTempBrd(int16_t *TempBrd)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_TLM;
    uint16_t addr = STRX_ADDR_TLM_TEMP_BRD;
   
    if ((status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_INT16, CSP_NODE_STRX, table_id, addr, TempBrd)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TLM_GET_TEMP_BRD_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t STRX_TLM_GetLastRssi(int16_t *LastRssi)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_TLM;
    uint16_t addr = STRX_ADDR_TLM_LAST_RSSI;
   
    if ((status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_INT16, CSP_NODE_STRX, table_id, addr, LastRssi)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TLM_GET_LAST_RSSI_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t STRX_TLM_GetRssibusy(int16_t *Rassibusy)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_TXCONF;
    uint16_t addr = STRX_ADDR_TXCONF_RSSIBUSY;
   
    if ((status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_INT16, CSP_NODE_STRX, table_id, addr, Rassibusy)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TLM_GetRssibusy_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t STRX_TLM_GetLastRferr(int16_t *LastRferr)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_TLM;
    uint16_t addr = STRX_ADDR_TLM_LAST_RFERR;

    if ((status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_INT16, CSP_NODE_STRX, table_id, addr, LastRferr)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TLM_GET_LAST_RFERR_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t STRX_TLM_GetBootCount(uint16_t *BootCount)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_TLM;
    uint16_t addr = STRX_ADDR_TLM_BOOT_COUNT;

    if ((status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT16, CSP_NODE_STRX, table_id, addr, BootCount)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TLM_GET_BOOT_COUNT_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t STRX_TLM_GetBootCause(uint32_t *BootCause)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_TLM;
    uint16_t addr = STRX_ADDR_TLM_BOOT_CAUSE;

    if ((status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT32, CSP_NODE_STRX, table_id, addr, BootCause)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TLM_GET_BOOT_CAUSE_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t STRX_TLM_GetLastContact(uint32_t *LastContact)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_TLM;
    uint16_t addr = STRX_ADDR_TLM_LAST_CONTACT;

    if ((status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT32, CSP_NODE_STRX, table_id, addr, LastContact)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TLM_GET_LAST_CONTACT_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t STRX_TLM_GetTotTxBytes(uint32_t *TotTxBytes)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_TLM;
    uint16_t addr = STRX_ADDR_TLM_TOT_TX_BYTES;

    if ((status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT32, CSP_NODE_STRX, table_id, addr, TotTxBytes)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TLM_GET_TOT_TX_BYTES_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t STRX_TLM_GetTotRxBytes(uint32_t *TotRxBytes)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_TLM;
    uint16_t addr = STRX_ADDR_TLM_TOT_RX_BYTES;

    if ((status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT32, CSP_NODE_STRX, table_id, addr, TotRxBytes)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TLM_GET_TOT_RX_BYTES_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t STRX_TLM_GET_HWDET(int8_t *HWDET)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_TLM;
    uint16_t addr = STRX_ADDR_TLM_HW_DET;

    if ((status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_INT8, CSP_NODE_STRX, table_id, addr, HWDET)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TLM_GET_HWDET_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t STRX_TLM_GET_RXMODE(uint8_t *Rxmode)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_TLM;
    uint16_t addr = STRX_ADDR_TLM_RX_MODE;

    if ((status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT8, CSP_NODE_STRX, table_id, addr, Rxmode)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TLM_GET_RXMODE_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t STRX_TLM_GET_GND_WDT_CNT(uint16_t *Wdt_cnt)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_TLM;
    uint16_t addr = STRX_ADDR_TLM_GND_WDT_CNT;

    if ((status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT16, CSP_NODE_STRX, table_id, addr, Wdt_cnt)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TLM_GET_GND_WDT_CNT_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t STRX_TLM_GET_GND_WDT_LEFT(uint32_t *Wdt_left)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_TLM;
    uint16_t addr = STRX_ADDR_TLM_GND_WDT_LEFT;

    if ((status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT32, CSP_NODE_STRX, table_id, addr, Wdt_left)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TLM_GET_GND_WDT_LEFT_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t STRX_TLM_GET_KISS_USART(int8_t *Kiss)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_SYSCONF;
    uint16_t addr = STRX_ADDR_KISS_USART;

    if ((status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_INT8, CSP_NODE_STRX, table_id, addr, Kiss)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TLM_GET_KISS_USART_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t STRX_TLM_SET_KISS_USART(int8_t Kiss)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_SYSCONF;
    uint16_t addr = STRX_ADDR_KISS_USART;
    if ((status = CFE_SRL_ApiSetRparamCSP(GS_PARAM_INT8, CSP_NODE_STRX, table_id, addr, &Kiss)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TLM_SET_KISS_USART_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t STRX_TLM_GET_GOSH_USART(uint8_t *Gosh)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_SYSCONF;
    uint16_t addr = STRX_ADDR_GOSH_USART;

    if ((status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT8, CSP_NODE_STRX, table_id, addr, Gosh)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TLM_GET_GOSH_USART_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t STRX_TLM_SET_GOSH_USART(uint8_t Gosh)
{
    int32_t status = DEVICE_SUCCESS;
    gs_param_table_id_t table_id = STRX_TABLE_SYSCONF;
    uint16_t addr = STRX_ADDR_GOSH_USART;
    if ((status = CFE_SRL_ApiSetRparamCSP(GS_PARAM_UINT8, CSP_NODE_STRX, table_id, addr, &Gosh)) < 0)
    {
        CFE_EVS_SendEvent(STRX_TLM_SET_GOSH_USART_EID, CFE_EVS_EventType_ERROR, "STRX: rparam Error (Error code : %d, Table ID : %d, Address : 0x%02X)", status, table_id, addr);
		return status;
    }

    return status;
}

int32_t csp_checkstate_ping(uint8_t node)
{   

    int rtt = CFE_SRL_ApiPingCSP(node, 1000, 4, CSP_O_CRC32);

    if (rtt > 0) {
        OS_printf("[CSP] Ping to node %d successful, RTT = %d ms\n", node, rtt);
        return IFC_OK;
    } 
    else {
        OS_printf("[CSP] Ping to node %d failed\n", node);
        return IFC_ERROR;
    }
}