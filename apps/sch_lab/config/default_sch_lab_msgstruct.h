#ifndef SCH_LAB_MSGSTRUCT_H
#define SCH_LAB_MSGSTRUCT_H

#include <stdint.h>

#include "cfe_msg_hdr.h"

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} SCH_LAB_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
} SCH_LAB_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint16                  EntryIndex;
    uint16                  Enabled;
} SCH_LAB_SetEntryStateCmd_t;

#endif
