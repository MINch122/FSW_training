/**
 * @file
 *   MEOW Application Message ID values
 */
#ifndef DEFAULT_MEOW_MSGID_VALUES_H
#define DEFAULT_MEOW_MSGID_VALUES_H

#include "cfe_core_api_base_msgids.h"
#include "meow_topicids.h"

#define MEOW_CMD_PLATFORM_MIDVAL(x) CFE_PLATFORM_CMD_TOPICID_TO_MIDV(MEOW_MISSION_##x##_TOPICID)
#define MEOW_TLM_PLATFORM_MIDVAL(x) CFE_PLATFORM_TLM_TOPICID_TO_MIDV(MEOW_MISSION_##x##_TOPICID)

#endif /* DEFAULT_MEOW_MSGID_VALUES_H */
