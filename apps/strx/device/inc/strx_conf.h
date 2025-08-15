#ifndef _STRX_CONF_H_
#define _STRX_CONF_H_

// Default Settings
#define STRX_DEFUALT_BAUDRATE       4800
#define STRX_DEFAULT_GURAD          0

//Table Configuration
#define STRX_TABLE_SYSCONF                  0
#define STRX_TABLE_RXCONF                   1
#define STRX_TABLE_CALIBRATION              2
#define STRX_TABLE_TLM                      4
#define STRX_TABLE_TXCONF                   5

//Address Configuration

//Table 0
#define STRX_ADDR_KISS_USART           0x001f
#define STRX_ADDR_GOSH_USART           0x0020

//Table 1
#define STRX_ADDR_RXCONF_FREQ          0x0000
#define STRX_ADDR_RXCONF_BAUD          0x0008
#define STRX_ADDR_RXCONF_GUARD         0x000C
#define STRX_ADDR_RXCONF_CSP_HMAC      0x000F
#define STRX_ADDR_RXCONF_CSP_RS        0x0010
#define STRX_ADDR_RXCONF_CSP_CRC       0x0011
#define STRX_ADDR_RXCONF_CSP_RAND      0x0012
#define STRX_ADDR_RXCONF_CSP_HMAC_KEY  0x0013
#define STRX_ADDR_RXCONF_MIX_CURSET    0x0024
#define STRX_ADDR_RXCONF_BW            0x0028
#define STRX_ADDR_RXCONF_AFCRANGE      0x002C

//Table 2
#define STRX_ADDR_TABLE2_RSSI_OFFSET   0x0000  
#define STRX_ADDR_TABLE2_TCXO_OFFSET   0x0002 

//Table 4
#define STRX_ADDR_TLM_TEMP_BRD         0x0000
#define STRX_ADDR_TLM_LAST_RSSI        0x0004
#define STRX_ADDR_TLM_LAST_RFERR       0x0006
#define STRX_ADDR_TLM_BOOT_COUNT       0x0018
#define STRX_ADDR_TLM_BOOT_CAUSE       0x001c
// #define STRX_ADDR_TLM_ACTIVE_CONF      0x00FF // 수정필요
#define STRX_ADDR_TLM_LAST_CONTACT     0x0020 
#define STRX_ADDR_TLM_TOT_TX_BYTES     0x0034
#define STRX_ADDR_TLM_TOT_RX_BYTES     0x0038
#define STRX_ADDR_TLM_HW_DET           0x0044
#define STRX_ADDR_TLM_RX_MODE          0x0045
#define STRX_ADDR_TLM_GND_WDT_CNT      0x0046
#define STRX_ADDR_TLM_GND_WDT_LEFT     0x0048

//Table 5
#define STRX_ADDR_TXCONF_FREQ          0x0000
#define STRX_ADDR_TXCONF_BAUD          0x0008
#define STRX_ADDR_TXCONF_GUARD         0x000C
#define STRX_ADDR_TXCONF_CSP_HMAC      0x000F
#define STRX_ADDR_TXCONF_CSP_RS        0x0010
#define STRX_ADDR_TXCONF_CSP_CRC       0x0011
#define STRX_ADDR_TXCONF_CSP_RAND      0x0012
#define STRX_ADDR_TXCONF_CSP_HMAC_KEY  0x0013
#define STRX_ADDR_TXCONF_MIX_CURSET    0x0024
#define STRX_ADDR_TXCONF_RSSIBUSY      0x002C

#endif