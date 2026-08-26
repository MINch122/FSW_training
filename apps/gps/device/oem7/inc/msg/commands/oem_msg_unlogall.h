#ifndef _OEM_MSG_UNLOGALL_H_
#define _OEM_MSG_UNLOGALL_H_

#include "msg/oem_msg_common.h"

#define OEM_ID_CMD_UNLOGALL 38

/**
 * @struct oem_cmd_unlog_all
 *
 * @brief Removes all logs from logging control. MID = 38.
 */
typedef struct {
#ifdef OEM_MSG_INCLUDE_BIN_HEADER
    oem_binary_header_t header;
#endif

    /**
     * @brief   Port to clear.
     */
    oem_enum    port;

    /**
     * @brief If set, also removes logs with the HOLD parameter.
     */
    oem_enum    held;

} oem_cmd_unlog_all;

#endif