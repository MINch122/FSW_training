#ifndef _UTRX_CONF_H_
#define _UTRX_CONF_H_

// Default Settings
#define UTRX_DEFUALT_BAUDRATE       4800
#define UTRX_DEFAULT_GURAD          0

//Table Configuration
#define UTRX_TABLE_SYSCONF                  0
#define UTRX_TABLE_RXCONF                   1
#define UTRX_TABLE_TLM                      4
#define UTRX_TABLE_TXCONF                   5

//Address Configuration
//Table 1
#define UTRX_ADDR_RXCONF_BAUD          0x0004
#define UTRX_ADDR_RXCONF_GUARD         0x000C
#define UTRX_ADDR_RXCONF_FREQ          0X0000

//Table 4
#define UTRX_ADDR_TLM_TEMP_BRD         0x0000
#define UTRX_ADDR_TLM_LAST_RSSI        0x0004
#define UTRX_ADDR_TLM_LAST_RFERR       0x0006
#define UTRX_ADDR_TLM_ACTIVE_CONF      0x0018
#define UTRX_ADDR_TLM_BOOT_COUNT       0x0020
#define UTRX_ADDR_TLM_BOOT_CAUSE       0x0024
#define UTRX_ADDR_TLM_LAST_CONTACT     0x0028
#define UTRX_ADDR_TLM_TOT_TX_BYTES     0x0038
#define UTRX_ADDR_TLM_TOT_RX_BYTES     0x003C

//Table 5
#define UTRX_ADDR_TXCONF_BAUD          0x0004


#endif