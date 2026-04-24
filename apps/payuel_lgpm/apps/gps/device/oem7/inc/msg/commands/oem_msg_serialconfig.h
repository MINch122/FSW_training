#ifndef _OEM_MSG_SERIALCONFIG_H_
#define _OEM_MSG_SERIALCONFIG_H_

#include "msg/oem_msg_common.h"

#define OEM_ID_CMD_SERIALCONFIG 1246

/**
 * @struct oem_cmd_serial_config
 *
 * @brief Configures serial port settings. MID = 1246.
 */
typedef struct {
#ifdef OEM_MSG_INCLUDE_BIN_HEADER
    oem_binary_header_t header;
#endif

    oem_enum    port;

    /**
     * @brief   Serial baud rate (2400, 4800, 9600, 19200, 38400,
     *          57600, 115200, 230400 or 460800).
     */
    oem_ulong   baud;

    /**
     * @brief   Parity bit. 0 for no paraity (default), 1 for even, 2 for odd.
     */
    oem_enum    parity;

    /**
     * @brief   Number of data bits. Default 8.
     */
    oem_ulong   databits;

    /**
     * @brief   Number of stop bits. Default 1.
     */
    oem_ulong   stopbits;

    /**
     * @brief   
     */
    oem_enum    handshake;

    /**
     * @brief   Boolean enabling break detection. Set by default.
     */
    oem_enum _break;

} OEM_PACK oem_cmd_serial_config;

#endif