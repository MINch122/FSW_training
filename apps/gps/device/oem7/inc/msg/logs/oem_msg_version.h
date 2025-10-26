#ifndef _OEM_MSG_VERSION_H_
#define _OEM_MSG_VERSION_H_

#include "msg/oem_msg_common.h"

#define OEM_ID_LOG_VERSION 37

/**
 * @brief VERSION log component. See oem_log_version.
 */
typedef struct {
    oem_ulong type;
    oem_char model[16];
    oem_char psn[16];
    oem_char hwVersion[16];
    oem_char swVersion[16];
    oem_char bootVersion[16];
    oem_char compDate[12];
    oem_char compTime[12];
} OEM_PACK oem_log_version_comp;

/**
 * @struct oem_log_version
 *
 * @brief Version information. Polled log. MID = 37.
 */
typedef struct {
#ifdef OEM_MSG_INCLUDE_BIN_HEADER
    oem_binary_header_t header;
#endif
    oem_long             numComp;
    oem_log_version_comp comp[];
    /* 32-bit CRC at the end */
} OEM_PACK oem_log_version;

#endif