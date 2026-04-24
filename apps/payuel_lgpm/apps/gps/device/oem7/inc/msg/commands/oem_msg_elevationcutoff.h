#ifndef _OEM_MSG_ELEVATIONCUTOFF_H_
#define _OEM_MSG_ELEVATIONCUTOFF_H_

#include "msg/oem_msg_common.h"

#define OEM_ID_CMD_ELEVATIONCUTOFF 1735

/**
 * @struct oem_cmd_elevation_cutoff
 *
 * @brief Sets the elevation cut-off angle for tracked satellites. MID = 1735.
 */
typedef struct {
#ifdef OEM_MSG_INCLUDE_BIN_HEADER
    oem_binary_header_t header;
#endif
    oem_enum            constellation;
    // 0 for GPS, 1 for GLONASS and 32 for all.
    oem_float           cutoff;
    // +- 90 degrees.
    oem_ulong           reserved;
} OEM_PACK oem_cmd_elevation_cutoff;

#endif