#ifndef SCH_LAB_MSGSTRUCT_H
#define SCH_LAB_MSGSTRUCT_H

#include "common_types.h"
#include "cfe_msg_api_typedefs.h"
#include "cfe_msg_hdr.h"
#include "sch_lab_interface_cfg.h"

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

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint16                  EntryIndex;
    CFE_MSG_FcnCode_t       FcnCode;
    uint32                  MessageID;
    uint32                  PacketRate;
    uint16                  PayloadLength;
    uint16                  Enabled;
    uint8                   MessageBuffer[SCH_LAB_MAX_ARGS_PER_ENTRY * sizeof(uint16)];
} SCH_LAB_AddEntryCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint16                  EntryIndex;
} SCH_LAB_DeleteEntryCmd_t;

#endif
