#ifndef _OEM_MSG_INTERFACEMODE_H_
#define _OEM_MSG_INTERFACEMODE_H_

#include "msg/oem_msg_common.h"

#define OEM_ID_CMD_INTERFACEMODE 3

/**
 * @struct oem_cmd_interface_mode
 *
 * @brief Sets receive or transmit modes for ports. MID = 3.
 */
typedef struct {
#ifdef OEM_MSG_INCLUDE_BIN_HEADER
    oem_binary_header_t header;
#endif

    /**
     * @brief 
     */
    oem_enum port;

    /**
     * @brief 
     */
    oem_enum rxType;

    /**
     * @brief 
     */
    oem_enum txType;

    /**
     * @brief 
     */
    oem_enum responses;

} oem_cmd_interface_mode;

#endif