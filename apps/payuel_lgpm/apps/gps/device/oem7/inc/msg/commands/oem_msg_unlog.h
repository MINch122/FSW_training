#ifndef _OEM_MSG_UNLOG_H_
#define _OEM_MSG_UNLOG_H_

#include "msg/oem_msg_common.h"

#define OEM_ID_CMD_UNLOG 36

/**
 * @struct oem_cmd_unlog
 *
 * @brief Removes a log from logging control. MID = 36.
 */
typedef struct {
#ifdef OEM_MSG_INCLUDE_BIN_HEADER
    oem_binary_header_t header;
#endif

    /**
     * @brief   Port to which the log is being sent.
     */
    oem_enum    port;

    /**
     * @brief   Message ID.
     */
    oem_ushort  message;

    /**
     * @brief   Message type of the log. See oem_msg_type.
     */
    oem_char    messageType;

    /** @brief Unused. */
    oem_char    reserved;

} oem_cmd_unlog;

#endif