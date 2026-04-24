#ifndef _OEM_MSG_LOGLIST_H_
#define _OEM_MSG_LOGLIST_H_

#include "msg/oem_msg_common.h"

#define OEM_ID_LOG_LOGLIST 5

typedef struct {
    oem_enum port;
    oem_ushort message;
    oem_char messageType;
    oem_char reserved;
    oem_enum trigger;
    oem_double period;
    oem_double offset;
    oem_enum hold;
} OEM_PACK oem_log_loglist_comp;

typedef struct {
#ifdef OEM_MSG_INCLUDE_BIN_HEADER
    oem_binary_header_t header;
#endif
    oem_ulong numlogs;
    oem_log_loglist_comp comp[];
} OEM_PACK oem_log_loglist;

#endif