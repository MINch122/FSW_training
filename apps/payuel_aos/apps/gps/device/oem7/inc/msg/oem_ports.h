/**
 * @file oem_ports.h
 * @brief OEM log/cmd message port number definition.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2025.
 */
#ifndef _OEM_PORTS_H_

/**
 * @brief Port identifiers.
 * 
 *    OEM719/729 receivers support only three physical UART ports: COM1, COM2
 *    and COM3. Use the SERIALCONFIG Command to configure the receiver-side
 *    COM port settings.
 * 
 *    Logically, for input messages (Commands), the receiver is expected to
 *    ignore the port address field in the message header. This behavior is
 *    explicitly stated for ASCII commands, but the manual does not clarify
 *    this for binary messages. As a best practice, use `THISPORT` (192) for
 *    input Command headers.
 */
typedef enum {
    /**
     * COM port this message is delivered through.
     */
    OEM_PORT_THIS           = 0xC0, /* 192 */

    /**
     * UART. LVCMOS.
     */
    OEM_PORT_COM1           = 0x20, /* 32  */

    /**
     * UART. LVCMOS.
     */
    OEM_PORT_COM2           = 0x40, /* 64  */

    /**
     * UART. LVCMOS.
     */
    OEM_PORT_COM3           = 0x60, /* 96  */

    /**
     * Internal file logging.
     * 
     */
    OEM_PORT_FILE           = 0xE0,

    /**
     * CAN COM1.
     */
    OEM_PORT_CCOM1          = 0x1EA0,

    /**
     * CAN COM2.
     */
    OEM_PORT_CCOM2          = 0x1FA0,

    /**
     * External USB.
     */
    OEM_PORT_USB1           = 0x05A0,

    /**
     * "ALL" ports below are only valid for the UNLOGALL command.
     */
    OEM_PORT_COM1_ALL       = 0x01,
    OEM_PORT_COM2_ALL       = 0x02,
    OEM_PORT_COM3_ALL       = 0x03,
    OEM_PORT_THISPORT_ALL   = 0x06,
    OEM_PORT_FILE_ALL       = 0x07,
    OEM_PORT_ALL_PORTS      = 0x08,

} oem_port_t;


#endif
